/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDecodingImageGenerator.h"
#include "SkSurface.h"

class SkImage_Raster : public SkImage_Base {
public:
    static bool ValidArgs(const Info& info, size_t rowBytes) {
        const int maxDimension = SK_MaxS32 >> 2;
        const size_t kMaxPixelByteSize = SK_MaxS32;

        if (info.width() < 0 || info.height() < 0) {
            return false;
        }
        if (info.width() > maxDimension || info.height() > maxDimension) {
            return false;
        }
        if ((unsigned)info.colorType() > (unsigned)kLastEnum_SkColorType) {
            return false;
        }
        if ((unsigned)info.alphaType() > (unsigned)kLastEnum_SkAlphaType) {
            return false;
        }

        if (kUnknown_SkColorType == info.colorType()) {
            return false;
        }

        // TODO: check colorspace

        if (rowBytes < SkImageMinRowBytes(info)) {
            return false;
        }

        int64_t size = (int64_t)info.height() * rowBytes;
        if (size > (int64_t)kMaxPixelByteSize) {
            return false;
        }
        return true;
    }

    static SkImage* NewEmpty();

    SkImage_Raster(const SkImageInfo&, SkData*, size_t rb, const SkSurfaceProps*);
    virtual ~SkImage_Raster();

    void onDraw(SkCanvas*, SkScalar, SkScalar, const SkPaint*) const SK_OVERRIDE;
    void onDrawRect(SkCanvas*, const SkRect*, const SkRect&, const SkPaint*) const SK_OVERRIDE;
    SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) const SK_OVERRIDE;
    bool onReadPixels(SkBitmap*, const SkIRect&) const SK_OVERRIDE;
    const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const SK_OVERRIDE;
    bool getROPixels(SkBitmap*) const SK_OVERRIDE;

    // exposed for SkSurface_Raster via SkNewImageFromPixelRef
    SkImage_Raster(const SkImageInfo&, SkPixelRef*, size_t rowBytes, const SkSurfaceProps*);

    SkPixelRef* getPixelRef() const { return fBitmap.pixelRef(); }

    virtual SkShader* onNewShader(SkShader::TileMode,
                                  SkShader::TileMode,
                                  const SkMatrix* localMatrix) const SK_OVERRIDE;

    virtual bool isOpaque() const SK_OVERRIDE;

    SkImage_Raster(const SkBitmap& bm, const SkSurfaceProps* props)
        : INHERITED(bm.width(), bm.height(), props)
        , fBitmap(bm) {}

private:
    SkImage_Raster() : INHERITED(0, 0, NULL) {}

    SkBitmap    fBitmap;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage_Raster::NewEmpty() {
    // Returns lazily created singleton
    static SkImage* gEmpty;
    if (NULL == gEmpty) {
        gEmpty = SkNEW(SkImage_Raster);
    }
    gEmpty->ref();
    return gEmpty;
}

static void release_data(void* addr, void* context) {
    SkData* data = static_cast<SkData*>(context);
    data->unref();
}

SkImage_Raster::SkImage_Raster(const Info& info, SkData* data, size_t rowBytes,
                               const SkSurfaceProps* props)
    : INHERITED(info.width(), info.height(), props)
{
    data->ref();
    void* addr = const_cast<void*>(data->data());
    SkColorTable* ctable = NULL;

    fBitmap.installPixels(info, addr, rowBytes, ctable, release_data, data);
    fBitmap.setImmutable();
    fBitmap.lockPixels();
}

SkImage_Raster::SkImage_Raster(const Info& info, SkPixelRef* pr, size_t rowBytes,
                               const SkSurfaceProps* props)
    : INHERITED(info.width(), info.height(), props)
{
    fBitmap.setInfo(info, rowBytes);
    fBitmap.setPixelRef(pr);
    fBitmap.lockPixels();
}

SkImage_Raster::~SkImage_Raster() {}

SkShader* SkImage_Raster::onNewShader(SkShader::TileMode tileX, SkShader::TileMode tileY,
                                      const SkMatrix* localMatrix) const {
    return SkShader::CreateBitmapShader(fBitmap, tileX, tileY, localMatrix);
}

void SkImage_Raster::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

void SkImage_Raster::onDrawRect(SkCanvas* canvas, const SkRect* src, const SkRect& dst,
                                      const SkPaint* paint) const {
    canvas->drawBitmapRectToRect(fBitmap, src, dst, paint);
}

SkSurface* SkImage_Raster::onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) const {
    return SkSurface::NewRaster(info, &props);
}

bool SkImage_Raster::onReadPixels(SkBitmap* dst, const SkIRect& subset) const {
    if (dst->pixelRef()) {
        return this->INHERITED::onReadPixels(dst, subset);
    } else {
        SkBitmap src;
        if (!fBitmap.extractSubset(&src, subset)) {
            return false;
        }
        return src.copyTo(dst, src.colorType());
    }
}

const void* SkImage_Raster::onPeekPixels(SkImageInfo* infoPtr, size_t* rowBytesPtr) const {
    const SkImageInfo info = fBitmap.info();
    if ((kUnknown_SkColorType == info.colorType()) || !fBitmap.getPixels()) {
        return NULL;
    }
    *infoPtr = info;
    *rowBytesPtr = fBitmap.rowBytes();
    return fBitmap.getPixels();
}

bool SkImage_Raster::getROPixels(SkBitmap* dst) const {
    *dst = fBitmap;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewRasterCopy(const SkImageInfo& info, const void* pixels, size_t rowBytes) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes)) {
        return NULL;
    }
    if (0 == info.width() && 0 == info.height()) {
        return SkImage_Raster::NewEmpty();
    }
    // check this after empty-check
    if (NULL == pixels) {
        return NULL;
    }

    // Here we actually make a copy of the caller's pixel data
    SkAutoDataUnref data(SkData::NewWithCopy(pixels, info.height() * rowBytes));
    return SkNEW_ARGS(SkImage_Raster, (info, data, rowBytes, NULL));
}


SkImage* SkImage::NewRasterData(const SkImageInfo& info, SkData* data, size_t rowBytes) {
    if (!SkImage_Raster::ValidArgs(info, rowBytes)) {
        return NULL;
    }
    if (0 == info.width() && 0 == info.height()) {
        return SkImage_Raster::NewEmpty();
    }
    // check this after empty-check
    if (NULL == data) {
        return NULL;
    }

    // did they give us enough data?
    size_t size = info.height() * rowBytes;
    if (data->size() < size) {
        return NULL;
    }

    return SkNEW_ARGS(SkImage_Raster, (info, data, rowBytes, NULL));
}

SkImage* SkImage::NewFromGenerator(SkImageGenerator* generator) {
    SkBitmap bitmap;
    if (!SkInstallDiscardablePixelRef(generator, &bitmap)) {
        return NULL;
    }
    return SkNEW_ARGS(SkImage_Raster, (bitmap, NULL));
}

SkImage* SkNewImageFromPixelRef(const SkImageInfo& info, SkPixelRef* pr, size_t rowBytes,
                                const SkSurfaceProps* props) {
    return SkNEW_ARGS(SkImage_Raster, (info, pr, rowBytes, props));
}

const SkPixelRef* SkBitmapImageGetPixelRef(const SkImage* image) {
    return ((const SkImage_Raster*)image)->getPixelRef();
}

bool SkImage_Raster::isOpaque() const {
    return fBitmap.isOpaque();
}
