/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkData.h"
#if !defined(GOOGLE3)
#include "SkJpegCodec.h"
#endif
#include "SkRawCodec.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkSwizzler.h"
#include "SkTaskGroup.h"
#include "SkTemplates.h"
#include "SkTypes.h"

#include "dng_area_task.h"
#include "dng_color_space.h"
#include "dng_exceptions.h"
#include "dng_host.h"
#include "dng_info.h"
#include "dng_memory.h"
#include "dng_render.h"
#include "dng_stream.h"

#include "src/piex.h"

#include <cmath>  // for std::round,floor,ceil
#include <limits>

namespace {

// Caluclates the number of tiles of tile_size that fit into the area in vertical and horizontal
// directions.
dng_point num_tiles_in_area(const dng_point &areaSize,
                            const dng_point_real64 &tileSize) {
  // FIXME: Add a ceil_div() helper in SkCodecPriv.h
  return dng_point(static_cast<int32>((areaSize.v + tileSize.v - 1) / tileSize.v),
                   static_cast<int32>((areaSize.h + tileSize.h - 1) / tileSize.h));
}

int num_tasks_required(const dng_point& tilesInTask,
                         const dng_point& tilesInArea) {
  return ((tilesInArea.v + tilesInTask.v - 1) / tilesInTask.v) *
         ((tilesInArea.h + tilesInTask.h - 1) / tilesInTask.h);
}

// Calculate the number of tiles to process per task, taking into account the maximum number of
// tasks. It prefers to increase horizontally for better locality of reference.
dng_point num_tiles_per_task(const int maxTasks,
                             const dng_point &tilesInArea) {
  dng_point tilesInTask = {1, 1};
  while (num_tasks_required(tilesInTask, tilesInArea) > maxTasks) {
      if (tilesInTask.h < tilesInArea.h) {
          ++tilesInTask.h;
      } else if (tilesInTask.v < tilesInArea.v) {
          ++tilesInTask.v;
      } else {
          ThrowProgramError("num_tiles_per_task calculation is wrong.");
      }
  }
  return tilesInTask;
}

std::vector<dng_rect> compute_task_areas(const int maxTasks, const dng_rect& area,
                                         const dng_point& tileSize) {
  std::vector<dng_rect> taskAreas;
  const dng_point tilesInArea = num_tiles_in_area(area.Size(), tileSize);
  const dng_point tilesPerTask = num_tiles_per_task(maxTasks, tilesInArea);
  const dng_point taskAreaSize = {tilesPerTask.v * tileSize.v,
                                    tilesPerTask.h * tileSize.h};
  for (int v = 0; v < tilesInArea.v; v += tilesPerTask.v) {
    for (int h = 0; h < tilesInArea.h; h += tilesPerTask.h) {
      dng_rect taskArea;
      taskArea.t = area.t + v * tileSize.v;
      taskArea.l = area.l + h * tileSize.h;
      taskArea.b = Min_int32(taskArea.t + taskAreaSize.v, area.b);
      taskArea.r = Min_int32(taskArea.l + taskAreaSize.h, area.r);

      taskAreas.push_back(taskArea);
    }
  }
  return taskAreas;
}

class SkDngHost : public dng_host {
public:
    explicit SkDngHost(dng_memory_allocator* allocater) : dng_host(allocater) {}

    void PerformAreaTask(dng_area_task& task, const dng_rect& area) override {
        // The area task gets split up into max_tasks sub-tasks. The max_tasks is defined by the
        // dng-sdks default implementation of dng_area_task::MaxThreads() which returns 8 or 32
        // sub-tasks depending on the architecture.
        const int maxTasks = static_cast<int>(task.MaxThreads());

        SkTaskGroup taskGroup;

        // tileSize is typically 256x256
        const dng_point tileSize(task.FindTileSize(area));
        const std::vector<dng_rect> taskAreas = compute_task_areas(maxTasks, area, tileSize);
        const int numTasks = static_cast<int>(taskAreas.size());

        task.Start(numTasks, tileSize, &Allocator(), Sniffer());
        for (int taskIndex = 0; taskIndex < numTasks; ++taskIndex) {
            taskGroup.add([&task, this, taskIndex, taskAreas, tileSize] {
                task.ProcessOnThread(taskIndex, taskAreas[taskIndex], tileSize, this->Sniffer());
            });
        }

        taskGroup.wait();
        task.Finish(numTasks);
    }

    uint32 PerformAreaTaskThreads() override {
        // FIXME: Need to get the real amount of available threads used in the SkTaskGroup.
        return kMaxMPThreads;
    }

private:
    typedef dng_host INHERITED;
};

// T must be unsigned type.
template <class T>
bool safe_add_to_size_t(T arg1, T arg2, size_t* result) {
    SkASSERT(arg1 >= 0);
    SkASSERT(arg2 >= 0);
    if (arg1 >= 0 && arg2 <= std::numeric_limits<T>::max() - arg1) {
        T sum = arg1 + arg2;
        if (sum <= std::numeric_limits<size_t>::max()) {
            *result = static_cast<size_t>(sum);
            return true;
        }
    }
    return false;
}

class SkDngMemoryAllocator : public dng_memory_allocator {
public:
    ~SkDngMemoryAllocator() override {}

    dng_memory_block* Allocate(uint32 size) override {
        // To avoid arbitary allocation requests which might lead to out-of-memory, limit the
        // amount of memory that can be allocated at once. The memory limit is based on experiments
        // and supposed to be sufficient for all valid DNG images.
        if (size > 300 * 1024 * 1024) {  // 300 MB
            ThrowMemoryFull();
        }
        return dng_memory_allocator::Allocate(size);
    }
};

}  // namespace

// Note: this class could throw exception if it is used as dng_stream.
class SkRawStream : public ::piex::StreamInterface {
public:
    // Note that this call will take the ownership of stream.
    explicit SkRawStream(SkStream* stream)
        : fStream(stream), fWholeStreamRead(false) {}

    ~SkRawStream() override {}

    /*
     * Creates an SkMemoryStream from the offset with size.
     * Note: for performance reason, this function is destructive to the SkRawStream. One should
     *       abandon current object after the function call.
     */
    SkMemoryStream* transferBuffer(size_t offset, size_t size) {
        SkAutoTUnref<SkData> data(SkData::NewUninitialized(size));
        if (offset > fStreamBuffer.bytesWritten()) {
            // If the offset is not buffered, read from fStream directly and skip the buffering.
            const size_t skipLength = offset - fStreamBuffer.bytesWritten();
            if (fStream->skip(skipLength) != skipLength) {
                return nullptr;
            }
            const size_t bytesRead = fStream->read(data->writable_data(), size);
            if (bytesRead < size) {
                data.reset(SkData::NewSubset(data.get(), 0, bytesRead));
            }
        } else {
            const size_t alreadyBuffered = SkTMin(fStreamBuffer.bytesWritten() - offset, size);
            if (alreadyBuffered > 0 &&
                !fStreamBuffer.read(data->writable_data(), offset, alreadyBuffered)) {
                return nullptr;
            }

            const size_t remaining = size - alreadyBuffered;
            if (remaining) {
                auto* dst = static_cast<uint8_t*>(data->writable_data()) + alreadyBuffered;
                const size_t bytesRead = fStream->read(dst, remaining);
                size_t newSize;
                if (bytesRead < remaining) {
                    if (!safe_add_to_size_t(alreadyBuffered, bytesRead, &newSize)) {
                        return nullptr;
                    }
                    data.reset(SkData::NewSubset(data.get(), 0, newSize));
                }
            }
        }
        return new SkMemoryStream(data);
    }

    // For PIEX
    ::piex::Error GetData(const size_t offset, const size_t length,
                          uint8* data) override {
        if (offset == 0 && length == 0) {
            return ::piex::Error::kOk;
        }
        size_t sum;
        if (!safe_add_to_size_t(offset, length, &sum) || !this->bufferMoreData(sum)) {
            return ::piex::Error::kFail;
        }
        if (!fStreamBuffer.read(data, offset, length)) {
            return ::piex::Error::kFail;
        }
        return ::piex::Error::kOk;
    }

    // For dng_stream
    uint64 getLength() {
        if (!this->bufferMoreData(kReadToEnd)) {  // read whole stream
            ThrowReadFile();
        }
        return fStreamBuffer.bytesWritten();
    }

    // For dng_stream
    void read(void* data, uint32 count, uint64 offset) {
        if (count == 0 && offset == 0) {
            return;
        }
        size_t sum;
        if (!safe_add_to_size_t(static_cast<uint64>(count), offset, &sum) ||
            !this->bufferMoreData(sum)) {
            ThrowReadFile();
        }

        if (!fStreamBuffer.read(data, static_cast<size_t>(offset), count)) {
            ThrowReadFile();
        }
    }

private:
    // Note: if the newSize == kReadToEnd (0), this function will read to the end of stream.
    bool bufferMoreData(size_t newSize) {
        if (newSize == kReadToEnd) {
            if (fWholeStreamRead) {  // already read-to-end.
                return true;
            }

            // TODO: optimize for the special case when the input is SkMemoryStream.
            return SkStreamCopy(&fStreamBuffer, fStream.get());
        }

        if (newSize <= fStreamBuffer.bytesWritten()) {  // already buffered to newSize
            return true;
        }
        if (fWholeStreamRead) {  // newSize is larger than the whole stream.
            return false;
        }

        const size_t sizeToRead = newSize - fStreamBuffer.bytesWritten();
        SkAutoTMalloc<uint8> tempBuffer(sizeToRead);
        const size_t bytesRead = fStream->read(tempBuffer.get(), sizeToRead);
        if (bytesRead != sizeToRead) {
            return false;
        }
        return fStreamBuffer.write(tempBuffer.get(), bytesRead);
    }

    SkAutoTDelete<SkStream> fStream;
    bool fWholeStreamRead;

    SkDynamicMemoryWStream fStreamBuffer;

    const size_t kReadToEnd = 0;
};

class SkDngStream : public dng_stream {
public:
    SkDngStream(SkRawStream* rawStream) : fRawStream(rawStream) {}

    uint64 DoGetLength() override { return fRawStream->getLength(); }

    void DoRead(void* data, uint32 count, uint64 offset) override {
        fRawStream->read(data, count, offset);
    }

private:
    SkRawStream* fRawStream;
};

class SkDngImage {
public:
    static SkDngImage* NewFromStream(SkRawStream* stream) {
        SkAutoTDelete<SkDngImage> dngImage(new SkDngImage(stream));
        if (!dngImage->readDng()) {
            return nullptr;
        }

        SkASSERT(dngImage->fNegative);
        return dngImage.release();
    }

    /*
     * Renders the DNG image to the size. The DNG SDK only allows scaling close to integer factors
     * down to 80 pixels on the short edge. The rendered image will be close to the specified size,
     * but there is no guarantee that any of the edges will match the requested size. E.g.
     *   100% size:              4000 x 3000
     *   requested size:         1600 x 1200
     *   returned size could be: 2000 x 1500
     */
    dng_image* render(int width, int height) {
        if (!fHost || !fInfo || !fNegative || !fDngStream) {
            if (!this->readDng()) {
                return nullptr;
            }
        }

        // render() takes ownership of fHost, fInfo, fNegative and fDngStream when available.
        SkAutoTDelete<dng_host> host(fHost.release());
        SkAutoTDelete<dng_info> info(fInfo.release());
        SkAutoTDelete<dng_negative> negative(fNegative.release());
        SkAutoTDelete<dng_stream> dngStream(fDngStream.release());

        // DNG SDK preserves the aspect ratio, so it only needs to know the longer dimension.
        const int preferredSize = SkTMax(width, height);
        try {
            host->SetPreferredSize(preferredSize);
            host->ValidateSizes();

            negative->ReadStage1Image(*host, *dngStream, *info);

            if (info->fMaskIndex != -1) {
                negative->ReadTransparencyMask(*host, *dngStream, *info);
            }

            negative->ValidateRawImageDigest(*host);
            if (negative->IsDamaged()) {
                return nullptr;
            }

            const int32 kMosaicPlane = -1;
            negative->BuildStage2Image(*host);
            negative->BuildStage3Image(*host, kMosaicPlane);

            dng_render render(*host, *negative);
            render.SetFinalSpace(dng_space_sRGB::Get());
            render.SetFinalPixelType(ttByte);

            dng_point stage3_size = negative->Stage3Image()->Size();
            render.SetMaximumSize(SkTMax(stage3_size.h, stage3_size.v));

            return render.Render();
        } catch (...) {
            return nullptr;
        }
    }

    const SkImageInfo& getImageInfo() const {
        return fImageInfo;
    }

    bool isScalable() const {
        return fIsScalable;
    }

    bool isXtransImage() const {
        return fIsXtransImage;
    }

private:
    bool readDng() {
        // Due to the limit of DNG SDK, we need to reset host and info.
        fHost.reset(new SkDngHost(&fAllocator));
        fInfo.reset(new dng_info);
        fDngStream.reset(new SkDngStream(fStream));
        try {
            fHost->ValidateSizes();
            fInfo->Parse(*fHost, *fDngStream);
            fInfo->PostParse(*fHost);
            if (!fInfo->IsValidDNG()) {
                return false;
            }

            fNegative.reset(fHost->Make_dng_negative());
            fNegative->Parse(*fHost, *fDngStream, *fInfo);
            fNegative->PostParse(*fHost, *fDngStream, *fInfo);
            fNegative->SynchronizeMetadata();

            fImageInfo = SkImageInfo::Make(
                static_cast<int>(fNegative->DefaultCropSizeH().As_real64()),
                static_cast<int>(fNegative->DefaultCropSizeV().As_real64()),
                kN32_SkColorType, kOpaque_SkAlphaType);

            // The DNG SDK scales only for at demosaicing, so only when a mosaic info
            // is available also scale is available.
            fIsScalable = fNegative->GetMosaicInfo() != nullptr;
            fIsXtransImage = fIsScalable
                ? (fNegative->GetMosaicInfo()->fCFAPatternSize.v == 6
                   && fNegative->GetMosaicInfo()->fCFAPatternSize.h == 6)
                : false;
            return true;
        } catch (...) {
            fNegative.reset(nullptr);
            return false;
        }
    }

    SkDngImage(SkRawStream* stream)
        : fStream(stream) {}

    SkDngMemoryAllocator fAllocator;
    SkAutoTDelete<SkRawStream> fStream;
    SkAutoTDelete<dng_host> fHost;
    SkAutoTDelete<dng_info> fInfo;
    SkAutoTDelete<dng_negative> fNegative;
    SkAutoTDelete<dng_stream> fDngStream;

    SkImageInfo fImageInfo;
    bool fIsScalable;
    bool fIsXtransImage;
};

/*
 * Tries to handle the image with PIEX. If PIEX returns kOk and finds the preview image, create a
 * SkJpegCodec. If PIEX returns kFail, then the file is invalid, return nullptr. In other cases,
 * fallback to create SkRawCodec for DNG images.
 */
SkCodec* SkRawCodec::NewFromStream(SkStream* stream) {
    SkAutoTDelete<SkRawStream> rawStream(new SkRawStream(stream));
    ::piex::PreviewImageData imageData;
    // FIXME: ::piex::GetPreviewImageData() calls GetData() frequently with small amounts,
    // resulting in many calls to bufferMoreData(). Could we make this more efficient by grouping
    // smaller requests together?
    if (::piex::IsRaw(rawStream.get())) {
        ::piex::Error error = ::piex::GetPreviewImageData(rawStream.get(), &imageData);

        if (error == ::piex::Error::kOk && imageData.preview_length > 0) {
#if !defined(GOOGLE3)
            // transferBuffer() is destructive to the rawStream. Abandon the rawStream after this
            // function call.
            // FIXME: one may avoid the copy of memoryStream and use the buffered rawStream.
            SkMemoryStream* memoryStream =
                    rawStream->transferBuffer(imageData.preview_offset, imageData.preview_length);
            return memoryStream ? SkJpegCodec::NewFromStream(memoryStream) : nullptr;
#else
            return nullptr;
#endif
        } else if (error == ::piex::Error::kFail) {
            return nullptr;
        }
    }

    SkAutoTDelete<SkDngImage> dngImage(SkDngImage::NewFromStream(rawStream.release()));
    if (!dngImage) {
        return nullptr;
    }

    return new SkRawCodec(dngImage.release());
}

SkCodec::Result SkRawCodec::onGetPixels(const SkImageInfo& requestedInfo, void* dst,
                                        size_t dstRowBytes, const Options& options,
                                        SkPMColor ctable[], int* ctableCount,
                                        int* rowsDecoded) {
    if (!conversion_possible(requestedInfo, this->getInfo())) {
        SkCodecPrintf("Error: cannot convert input type to output type.\n");
        return kInvalidConversion;
    }

    SkAutoTDelete<SkSwizzler> swizzler(SkSwizzler::CreateSwizzler(
            SkSwizzler::kRGB, nullptr, requestedInfo, options));
    SkASSERT(swizzler);

    const int width = requestedInfo.width();
    const int height = requestedInfo.height();
    SkAutoTDelete<dng_image> image(fDngImage->render(width, height));
    if (!image) {
        return kInvalidInput;
    }

    // Because the DNG SDK can not guarantee to render to requested size, we allow a small
    // difference. Only the overlapping region will be converted.
    const float maxDiffRatio = 1.03f;
    const dng_point& imageSize = image->Size();
    if (imageSize.h / width > maxDiffRatio || imageSize.h < width ||
        imageSize.v / height > maxDiffRatio || imageSize.v < height) {
        return SkCodec::kInvalidScale;
    }

    void* dstRow = dst;
    SkAutoTMalloc<uint8_t> srcRow(width * 3);

    dng_pixel_buffer buffer;
    buffer.fData = &srcRow[0];
    buffer.fPlane = 0;
    buffer.fPlanes = 3;
    buffer.fColStep = buffer.fPlanes;
    buffer.fPlaneStep = 1;
    buffer.fPixelType = ttByte;
    buffer.fPixelSize = sizeof(uint8_t);
    buffer.fRowStep = sizeof(srcRow);

    for (int i = 0; i < height; ++i) {
        buffer.fArea = dng_rect(i, 0, i + 1, width);

        try {
            image->Get(buffer, dng_image::edge_zero);
        } catch (...) {
            *rowsDecoded = i;
            return kIncompleteInput; 
        }

        swizzler->swizzle(dstRow, &srcRow[0]);
        dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
    }
    return kSuccess;
}

SkISize SkRawCodec::onGetScaledDimensions(float desiredScale) const {
    SkASSERT(desiredScale <= 1.f);
    const SkISize dim = this->getInfo().dimensions();
    if (!fDngImage->isScalable()) {
        return dim;
    }

    // Limits the minimum size to be 80 on the short edge.
    const float shortEdge = static_cast<float>(SkTMin(dim.fWidth, dim.fHeight));
    if (desiredScale < 80.f / shortEdge) {
        desiredScale = 80.f / shortEdge;
    }

    // For Xtrans images, the integer-factor scaling does not support the half-size scaling case
    // (stronger downscalings are fine). In this case, returns the factor "3" scaling instead.
    if (fDngImage->isXtransImage() && desiredScale > 1.f / 3.f && desiredScale < 1.f) {
        desiredScale = 1.f / 3.f;
    }

    // Round to integer-factors.
    const float finalScale = std::floor(1.f/ desiredScale);
    return SkISize::Make(static_cast<int32_t>(std::floor(dim.fWidth / finalScale)),
                         static_cast<int32_t>(std::floor(dim.fHeight / finalScale)));
}

bool SkRawCodec::onDimensionsSupported(const SkISize& dim) {
    const SkISize fullDim = this->getInfo().dimensions();
    const float fullShortEdge = static_cast<float>(SkTMin(fullDim.fWidth, fullDim.fHeight));
    const float shortEdge = static_cast<float>(SkTMin(dim.fWidth, dim.fHeight));

    SkISize sizeFloor = this->onGetScaledDimensions(1.f / std::floor(fullShortEdge / shortEdge));
    SkISize sizeCeil = this->onGetScaledDimensions(1.f / std::ceil(fullShortEdge / shortEdge));
    return sizeFloor == dim || sizeCeil == dim;
}

SkRawCodec::~SkRawCodec() {}

SkRawCodec::SkRawCodec(SkDngImage* dngImage)
    : INHERITED(dngImage->getImageInfo(), nullptr)
    , fDngImage(dngImage) {}
