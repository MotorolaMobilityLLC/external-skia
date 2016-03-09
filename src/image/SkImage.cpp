/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapCache.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageEncoder.h"
#include "SkImageGenerator.h"
#include "SkImagePriv.h"
#include "SkImageShader.h"
#include "SkImage_Base.h"
#include "SkNextID.h"
#include "SkPixelRef.h"
#include "SkPixelSerializer.h"
#include "SkReadPixelsRec.h"
#include "SkString.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
#include "GrTexture.h"
#include "GrContext.h"
#include "SkImage_Gpu.h"
#endif

SkImage::SkImage(int width, int height, uint32_t uniqueID)
    : fWidth(width)
    , fHeight(height)
    , fUniqueID(kNeedNewImageUniqueID == uniqueID ? SkNextID::ImageID() : uniqueID)
{
    SkASSERT(width > 0);
    SkASSERT(height > 0);
}

bool SkImage::peekPixels(SkPixmap* pm) const {
    SkPixmap tmp;
    if (!pm) {
        pm = &tmp;
    }
    return as_IB(this)->onPeekPixels(pm);
}

#ifdef SK_SUPPORT_LEGACY_PEEKPIXELS_PARMS
const void* SkImage::peekPixels(SkImageInfo* info, size_t* rowBytes) const {
    SkPixmap pm;
    if (this->peekPixels(&pm)) {
        if (info) {
            *info = pm.info();
        }
        if (rowBytes) {
            *rowBytes = pm.rowBytes();
        }
        return pm.addr();
    }
    return nullptr;
}
#endif

bool SkImage::readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                           int srcX, int srcY, CachingHint chint) const {
    SkReadPixelsRec rec(dstInfo, dstPixels, dstRowBytes, srcX, srcY);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }
    return as_IB(this)->onReadPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY, chint);
}

bool SkImage::scalePixels(const SkPixmap& dst, SkFilterQuality quality, CachingHint chint) const {
    if (this->width() == dst.width() && this->height() == dst.height()) {
        return this->readPixels(dst, 0, 0, chint);
    }

    // Idea: If/when SkImageGenerator supports a native-scaling API (where the generator itself
    //       can scale more efficiently) we should take advantage of it here.
    //
    SkBitmap bm;
    if (as_IB(this)->getROPixels(&bm, chint)) {
        bm.lockPixels();
        SkPixmap pmap;
        // Note: By calling the pixmap scaler, we never cache the final result, so the chint
        //       is (currently) only being applied to the getROPixels. If we get a request to
        //       also attempt to cache the final (scaled) result, we would add that logic here.
        //
        return bm.peekPixels(&pmap) && pmap.scalePixels(dst, quality);
    }
    return false;
}

void SkImage::preroll(GrContext* ctx) const {
    // For now, and to maintain parity w/ previous pixelref behavior, we just force the image
    // to produce a cached raster-bitmap form, so that drawing to a raster canvas should be fast.
    //
    SkBitmap bm;
    if (as_IB(this)->getROPixels(&bm)) {
        bm.lockPixels();
        bm.unlockPixels();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkImage::makeShader(SkShader::TileMode tileX, SkShader::TileMode tileY,
                                    const SkMatrix* localMatrix) const {
    return SkImageShader::Make(this, tileX, tileY, localMatrix);
}

#ifdef SK_SUPPORT_LEGACY_CREATESHADER_PTR
SkShader* SkImage::newShader(SkShader::TileMode tileX, SkShader::TileMode tileY,
                             const SkMatrix* localMatrix) const {
    return this->makeShader(tileX, tileY, localMatrix).release();
}
#endif

SkData* SkImage::encode(SkImageEncoder::Type type, int quality) const {
    SkBitmap bm;
    if (as_IB(this)->getROPixels(&bm)) {
        return SkImageEncoder::EncodeData(bm, type, quality);
    }
    return nullptr;
}

SkData* SkImage::encode(SkPixelSerializer* serializer) const {
    SkAutoTUnref<SkPixelSerializer> defaultSerializer;
    SkPixelSerializer* effectiveSerializer = serializer;
    if (!effectiveSerializer) {
        defaultSerializer.reset(SkImageEncoder::CreatePixelSerializer());
        SkASSERT(defaultSerializer.get());
        effectiveSerializer = defaultSerializer.get();
    }
    SkAutoTUnref<SkData> encoded(this->refEncoded());
    if (encoded && effectiveSerializer->useEncodedData(encoded->data(), encoded->size())) {
        return encoded.detach();
    }

    SkBitmap bm;
    SkAutoPixmapUnlock apu;
    if (as_IB(this)->getROPixels(&bm) && bm.requestLock(&apu)) {
        return effectiveSerializer->encode(apu.pixmap());
    }

    return nullptr;
}

SkData* SkImage::refEncoded() const {
    GrContext* ctx = nullptr;   // should we allow the caller to pass in a ctx?
    return as_IB(this)->onRefEncoded(ctx);
}

SkImage* SkImage::NewFromEncoded(SkData* encoded, const SkIRect* subset) {
    if (nullptr == encoded || 0 == encoded->size()) {
        return nullptr;
    }
    SkImageGenerator* generator = SkImageGenerator::NewFromEncoded(encoded);
    return generator ? SkImage::NewFromGenerator(generator, subset) : nullptr;
}

const char* SkImage::toString(SkString* str) const {
    str->appendf("image: (id:%d (%d, %d) %s)", this->uniqueID(), this->width(), this->height(),
                 this->isOpaque() ? "opaque" : "");
    return str->c_str();
}

SkImage* SkImage::newSubset(const SkIRect& subset) const {
    if (subset.isEmpty()) {
        return nullptr;
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    if (!bounds.contains(subset)) {
        return nullptr;
    }

    // optimization : return self if the subset == our bounds
    if (bounds == subset) {
        return SkRef(const_cast<SkImage*>(this));
    }
    return as_IB(this)->onNewSubset(subset);
}

#if SK_SUPPORT_GPU

GrTexture* SkImage::getTexture() const {
    return as_IB(this)->peekTexture();
}

bool SkImage::isTextureBacked() const { return SkToBool(as_IB(this)->peekTexture()); }

GrBackendObject SkImage::getTextureHandle(bool flushPendingGrContextIO) const {
    GrTexture* texture = as_IB(this)->peekTexture();
    if (texture) {
        GrContext* context = texture->getContext();
        if (context) {            
            if (flushPendingGrContextIO) {
                context->prepareSurfaceForExternalIO(texture);
            }
        }
        return texture->getTextureHandle();
    }
    return 0;
}

#else

GrTexture* SkImage::getTexture() const { return nullptr; }

bool SkImage::isTextureBacked() const { return false; }

GrBackendObject SkImage::getTextureHandle(bool) const { return 0; }

#endif

///////////////////////////////////////////////////////////////////////////////

static bool raster_canvas_supports(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kN32_SkColorType:
            return kUnpremul_SkAlphaType != info.alphaType();
        case kRGB_565_SkColorType:
            return true;
        case kAlpha_8_SkColorType:
            return true;
        default:
            break;
    }
    return false;
}

SkImage_Base::SkImage_Base(int width, int height, uint32_t uniqueID)
    : INHERITED(width, height, uniqueID)
    , fAddedToCache(false)
{}

SkImage_Base::~SkImage_Base() {
    if (fAddedToCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

bool SkImage_Base::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                                int srcX, int srcY, CachingHint) const {
    if (!raster_canvas_supports(dstInfo)) {
        return false;
    }

    SkBitmap bm;
    bm.installPixels(dstInfo, dstPixels, dstRowBytes);
    SkCanvas canvas(bm);

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    canvas.drawImage(this, -SkIntToScalar(srcX), -SkIntToScalar(srcY), &paint);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage::readPixels(const SkPixmap& pmap, int srcX, int srcY, CachingHint chint) const {
    return this->readPixels(pmap.info(), pmap.writable_addr(), pmap.rowBytes(), srcX, srcY, chint);
}

#if SK_SUPPORT_GPU
#include "GrTextureToYUVPlanes.h"
#endif

#include "SkRGBAToYUV.h"

bool SkImage::readYUV8Planes(const SkISize sizes[3], void* const planes[3],
                             const size_t rowBytes[3], SkYUVColorSpace colorSpace) const {
#if SK_SUPPORT_GPU
    if (GrTexture* texture = as_IB(this)->peekTexture()) {
        if (GrTextureToYUVPlanes(texture, sizes, planes, rowBytes, colorSpace)) {
            return true;
        }
    }
#endif
    return SkRGBAToYUV(this, sizes, planes, rowBytes, colorSpace);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewFromBitmap(const SkBitmap& bm) {
    SkPixelRef* pr = bm.pixelRef();
    if (nullptr == pr) {
        return nullptr;
    }

#if SK_SUPPORT_GPU
    if (GrTexture* tex = pr->getTexture()) {
        SkAutoTUnref<GrTexture> unrefCopy;
        if (!bm.isImmutable()) {
            tex = GrDeepCopyTexture(tex, SkBudgeted::kNo);
            if (nullptr == tex) {
                return nullptr;
            }
            unrefCopy.reset(tex);
        }
        const SkImageInfo info = bm.info();
        return new SkImage_Gpu(info.width(), info.height(), bm.getGenerationID(), info.alphaType(),
                               tex, SkBudgeted::kNo);
    }
#endif

    // This will check for immutable (share or copy)
    return SkNewImageFromRasterBitmap(bm);
}

bool SkImage::asLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode mode) const {
    return as_IB(this)->onAsLegacyBitmap(bitmap, mode);
}

bool SkImage_Base::onAsLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode mode) const {
    // As the base-class, all we can do is make a copy (regardless of mode).
    // Subclasses that want to be more optimal should override.
    SkImageInfo info = SkImageInfo::MakeN32(this->width(), this->height(),
                                    this->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    if (!bitmap->tryAllocPixels(info)) {
        return false;
    }
    if (!this->readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), 0, 0)) {
        bitmap->reset();
        return false;
    }

    if (kRO_LegacyBitmapMode == mode) {
        bitmap->setImmutable();
    }
    return true;
}

SkImage* SkImage::NewFromPicture(const SkPicture* picture, const SkISize& dimensions,
                                 const SkMatrix* matrix, const SkPaint* paint) {
    if (!picture) {
        return nullptr;
    }
    return NewFromGenerator(SkImageGenerator::NewFromPicture(dimensions, picture, matrix, paint));
}

bool SkImage::isLazyGenerated() const {
    return as_IB(this)->onIsLazyGenerated();
}

//////////////////////////////////////////////////////////////////////////////////////

#if !SK_SUPPORT_GPU

SkImage* SkImage::NewFromTexture(GrContext*, const GrBackendTextureDesc&, SkAlphaType,
                                 TextureReleaseProc, ReleaseContext) {
    return nullptr;
}

SkImage* SkImage::NewFromAdoptedTexture(GrContext*, const GrBackendTextureDesc&, SkAlphaType) {
    return nullptr;
}

SkImage* SkImage::NewFromTextureCopy(GrContext*, const GrBackendTextureDesc&, SkAlphaType) {
    return nullptr;
}

SkImage* SkImage::newTextureImage(GrContext*) const {
    return nullptr;
}

#endif
