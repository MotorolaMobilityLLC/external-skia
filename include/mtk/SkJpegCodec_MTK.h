/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegCodec_MTK_DEFINED
#define SkJpegCodec_MTK_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "src/codec/SkSwizzler.h"
#include "include/core/SkStream.h"
#include "include/private/SkTemplates.h"
#include "AppInsight.h"

#ifdef MTK_JPEG_HW_REGION_RESIZER
#include <sys/mman.h>
#include <linux/ion.h>
#include <ion/ion.h>
#include <BufferAllocator/BufferAllocatorWrapper.h>
#include <BufferAllocator/BufferAllocator.h>

#define ION_HEAP_MULTIMEDIA_MASK (1 << 10)
#define MAX_HEAP_COUNT  ION_HEAP_TYPE_CUSTOM
using namespace AppInsight;

class SkBufMalloc
{
public:
    SkBufMalloc(int ionClientHnd, BufferAllocator* bufferAllocator): fIonAllocHnd(-1), fIonClientHnd(ionClientHnd), fBufferAllocator(bufferAllocator), fAddr(NULL), fShareFD(-1), fSize(0), fStreamSize(0), fColorType(kRGBA_8888_SkColorType), fAlignment(0)
    {
    }

    ~SkBufMalloc() noexcept(false)
    {
        free();
    }
    void* reset(size_t size)
    {
        int ret;
        if(fAddr != NULL)
        {
            free();
        }
        fSize = size;
        if (fIonClientHnd >= 0)
        {
            ret = ion_alloc(fIonClientHnd, size, fAlignment, ION_HEAP_MULTIMEDIA_MASK, ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC, &fIonAllocHnd);
            if (ret)
            {
                SkDebugf("SkBufMalloc::ion_alloc failed (%d, %zu, %d)\n", fIonClientHnd, size, fIonAllocHnd);
                return NULL;
            }
            ret = ion_map(fIonClientHnd, fIonAllocHnd, size, PROT_READ | PROT_WRITE, MAP_SHARED, 0, &fAddr, &fShareFD);
            if (ret)
            {
                SkDebugf("SkBufMalloc::ion_mmap failed (%d, %zu, %d)\n", fIonClientHnd, size, fShareFD);
                ret = ion_free(fIonClientHnd, fIonAllocHnd);
                return NULL;
            }
        }
        else
        {
            fShareFD = DmabufHeapAlloc(fBufferAllocator, "mtk_mm", fSize, 0, 0);
            if (fShareFD < 0) {
                SkDebugf("SkBufMalloc DmabufHeapAlloc fail, bufferallocator %p fd: %d size: %zu\n", fBufferAllocator, fShareFD, size);
                fShareFD = -1;
                return NULL;
            }

            /* Create memory mapped buffer for the buffer fd */
            fAddr = (unsigned char *)mmap(NULL, fSize, PROT_READ|PROT_WRITE, MAP_SHARED, fShareFD, 0);
            if (fAddr == MAP_FAILED) {
                SkDebugf("SkBufMalloc mmap failed (%zu, %d)\n", size, fShareFD);
                close(fShareFD);
                return NULL;
            }
        }
        return fAddr;
    }
    void free()
    {
        if (fIonClientHnd >= 0)
        {
            if(fAddr != NULL)
            {
                int ret = munmap(fAddr, fSize);
                if (ret < 0)
                    SkDebugf("SkBufMalloc::ion_munmap failed (%d, %p, %zu)\n", fIonClientHnd, fAddr, fSize);
                else
                    fAddr = NULL;
            }
            if (fShareFD != -1)
            {
                if (close(fShareFD))
                {
                    SkDebugf("SkBufMalloc close failed (%d, %d)\n", fIonClientHnd, fShareFD);
                }
            }
            if (fIonAllocHnd != -1)
            {
                if (ion_free(fIonClientHnd, fIonAllocHnd))
                {
                    SkDebugf("SkBufMalloc::ion_free failed (%d %d)\n", fIonClientHnd, fIonAllocHnd);
                }
            }
        }
        else
        {
            if(fAddr != NULL)
            {
                int ret = munmap(fAddr, fSize);
                if (ret < 0)
                    SkDebugf("SkBufMalloc munmap failed (%p, %zu)\n", fAddr, fSize);
                else
                    fAddr = NULL;
            }
            if (fShareFD != -1)
            {
                if (close(fShareFD))
                {
                    SkDebugf("SkBufMalloc close failed (%d)\n", fShareFD);
                }
            }
        }
    }
    void setStreamSize(int streamSize) { fStreamSize = streamSize; }
    void setColor(SkColorType color) { fColorType = color; }
    void setAlignment(size_t align) { fAlignment = align; }
    void* getAddr() { return fAddr; }
    int getFD() { return fShareFD; }
    size_t getSize() { return fSize; }
    size_t getStreamSize() { return fStreamSize; }
    ion_user_handle_t getIonAllocHnd() { return fIonAllocHnd; }
    SkColorType getColor() { return fColorType; }
private:
    ion_user_handle_t fIonAllocHnd;
    int               fIonClientHnd;
    BufferAllocator*  fBufferAllocator;
    unsigned char*    fAddr;
    int               fShareFD;
    size_t            fSize;
    size_t            fStreamSize;
    SkColorType       fColorType;
    size_t            fAlignment;
};
#endif

class JpegDecoderMgr_MTK;

/*
 *
 * This class implements the decoding for jpeg images
 *
 */
class SkJpegCodec : public SkCodec {
public:
    static bool IsJpeg(const void*, size_t);

    /*
     * Assumes IsJpeg was called and returned true
     * Takes ownership of the stream
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

protected:

    /*
     * Recommend a set of destination dimensions given a requested scale
     */
    SkISize onGetScaledDimensions(float desiredScale) const override;

    /*
     * Initiates the jpeg decode
     */
    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes, const Options&,
            int*) override;

    bool onQueryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes&,
                         SkYUVAPixmapInfo*) const override;

    Result onGetYUVAPlanes(const SkYUVAPixmaps& yuvaPixmaps) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kJPEG;
    }

    bool onRewind() override;

    bool onDimensionsSupported(const SkISize&) override;

    bool conversionSupported(const SkImageInfo&, bool, bool) override;

private:
    /*
     * Allows SkRawCodec to communicate the color profile from the exif data.
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*,
            std::unique_ptr<SkEncodedInfo::ICCProfile> defaultColorProfile);

    /*
     * Read enough of the stream to initialize the SkJpegCodec.
     * Returns a bool representing success or failure.
     *
     * @param codecOut
     * If this returns true, and codecOut was not nullptr,
     * codecOut will be set to a new SkJpegCodec.
     *
     * @param decoderMgrOut
     * If this returns true, and codecOut was nullptr,
     * decoderMgrOut must be non-nullptr and decoderMgrOut will be set to a new
     * JpegDecoderMgr pointer.
     *
     * @param stream
     * Deleted on failure.
     * codecOut will take ownership of it in the case where we created a codec.
     * Ownership is unchanged when we set decoderMgrOut.
     *
     * @param defaultColorProfile
     * If the jpeg does not have an embedded color profile, the image data should
     * be tagged with this color profile.
     */
    static Result ReadHeader(SkStream* stream, SkCodec** codecOut,
            JpegDecoderMgr_MTK** decoderMgrOut,
            std::unique_ptr<SkEncodedInfo::ICCProfile> defaultColorProfile);

    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     *
     * @param info contains properties of the encoded data
     * @param stream the encoded image data
     * @param decoderMgr holds decompress struct, src manager, and error manager
     *                   takes ownership
     */
    SkJpegCodec(SkEncodedInfo&& info, std::unique_ptr<SkStream> stream,
            JpegDecoderMgr_MTK* decoderMgr, SkEncodedOrigin origin);

#ifdef MTK_JPEG_HW_REGION_RESIZER
    ~SkJpegCodec() override;
#endif
    void initializeSwizzler(const SkImageInfo& dstInfo, const Options& options,
                            bool needsCMYKToRGB);
    bool SK_WARN_UNUSED_RESULT allocateStorage(const SkImageInfo& dstInfo);
    int readRows(const SkImageInfo& dstInfo, void* dst, size_t rowBytes, int count, const Options&);

#ifdef MTK_JPEG_HW_REGION_RESIZER
    int readRows_MTK(const SkImageInfo& dstInfo, void* dst, size_t rowBytes, int count, const Options&);
#endif

    /*
     * Scanline decoding.
     */
    SkSampler* getSampler(bool createIfNecessary) override;
    Result onStartScanlineDecode(const SkImageInfo& dstInfo,
            const Options& options) override;
    int onGetScanlines(void* dst, int count, size_t rowBytes) override;
    bool onSkipScanlines(int count) override;

    std::unique_ptr<JpegDecoderMgr_MTK>    fDecoderMgr;

    // We will save the state of the decompress struct after reading the header.
    // This allows us to safely call onGetScaledDimensions() at any time.
    const int                          fReadyState;


    SkAutoTMalloc<uint8_t>             fStorage;
    uint8_t*                           fSwizzleSrcRow;
    uint32_t*                          fColorXformSrcRow;

    // libjpeg-turbo provides some subsetting.  In the case that libjpeg-turbo
    // cannot take the exact the subset that we need, we will use the swizzler
    // to further subset the output from libjpeg-turbo.
    SkIRect                            fSwizzlerSubset;

    std::unique_ptr<SkSwizzler>        fSwizzler;

    friend class SkRawCodec;

    using INHERITED = SkCodec;
#ifdef MTK_JPEG_HW_REGION_RESIZER
    int                    fIonClientHnd;
    BufferAllocator*       fBufferAllocator;
    int                    fISOSpeedRatings;
    SkBufMalloc*           fBufferStorage;
    SkBufMalloc*           fBitstreamBuf;
    SkStream*              fMemStream;
    bool                   fIsSampleDecode;
    unsigned int           fSampleDecodeY;

    bool                   fFirstTileDone;
    bool                   fUseHWResizer;
    bool                   fEnTdshp;
    unsigned int           fRegionHeight;
    InfoDump               fInfoDump;
#endif
};

#endif
