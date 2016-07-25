/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrImageIDTextureAdjuster.h"

#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "SkBitmap.h"
#include "SkGrPriv.h"
#include "SkImage_Base.h"
#include "SkImageCacherator.h"
#include "SkPixelRef.h"

static bool bmp_is_alpha_only(const SkBitmap& bm) { return kAlpha_8_SkColorType == bm.colorType(); }

// SkImage's don't have a way of communicating whether they're alpha-only. So we fallback to
// inspecting the texture.
static bool tex_image_is_alpha_only(const SkImage_Base& img) {
    return GrPixelConfigIsAlphaOnly(img.peekTexture()->config());
}

GrImageTextureAdjuster::GrImageTextureAdjuster(const SkImage_Base* img)
    : INHERITED(img->peekTexture(), SkIRect::MakeWH(img->width(), img->height()),
                tex_image_is_alpha_only(*img))
    , fImageBase(img) {}

void GrImageTextureAdjuster::makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey) {
    // By construction this texture adjuster always represents an entire SkImage, so use the
    // image's width and height for the key's rectangle.
    GrUniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, fImageBase->uniqueID(),
                         SkIRect::MakeWH(fImageBase->width(), fImageBase->height()));
    MakeCopyKeyFromOrigKey(baseKey, params, copyKey);
}

void GrImageTextureAdjuster::didCacheCopy(const GrUniqueKey& copyKey) {
    // We don't currently have a mechanism for notifications on Images!
}

SkColorSpace* GrImageTextureAdjuster::getColorSpace() {
    return fImageBase->onImageInfo().colorSpace();
}

//////////////////////////////////////////////////////////////////////////////

GrBitmapTextureMaker::GrBitmapTextureMaker(GrContext* context, const SkBitmap& bitmap)
    : INHERITED(context, bitmap.width(), bitmap.height(), bmp_is_alpha_only(bitmap))
    , fBitmap(bitmap)
{
    if (!bitmap.isVolatile()) {
        SkIPoint origin = bitmap.pixelRefOrigin();
        SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, bitmap.width(),
                                           bitmap.height());
        GrMakeKeyFromImageID(&fOriginalKey, bitmap.pixelRef()->getGenerationID(), subset);
    }
}

GrTexture* GrBitmapTextureMaker::refOriginalTexture(bool willBeMipped,
                                                    SkSourceGammaTreatment gammaTreatment) {
    GrTexture* tex = nullptr;

    if (fOriginalKey.isValid()) {
        tex = this->context()->textureProvider()->findAndRefTextureByUniqueKey(fOriginalKey);
        if (tex) {
            return tex;
        }
    }
    if (willBeMipped) {
        tex = GrGenerateMipMapsAndUploadToTexture(this->context(), fBitmap, gammaTreatment);
    }
    if (!tex) {
        tex = GrUploadBitmapToTexture(this->context(), fBitmap);
    }
    if (tex && fOriginalKey.isValid()) {
        tex->resourcePriv().setUniqueKey(fOriginalKey);
        GrInstallBitmapUniqueKeyInvalidator(fOriginalKey, fBitmap.pixelRef());
    }
    return tex;
}

void GrBitmapTextureMaker::makeCopyKey(const CopyParams& copyParams, GrUniqueKey* copyKey) {
    if (fOriginalKey.isValid()) {
        MakeCopyKeyFromOrigKey(fOriginalKey, copyParams, copyKey);
    }
}

void GrBitmapTextureMaker::didCacheCopy(const GrUniqueKey& copyKey) {
    GrInstallBitmapUniqueKeyInvalidator(copyKey, fBitmap.pixelRef());
}

SkColorSpace* GrBitmapTextureMaker::getColorSpace() {
    return fBitmap.colorSpace();
}

//////////////////////////////////////////////////////////////////////////////
static bool cacher_is_alpha_only(const SkImageCacherator& cacher) {
    return kAlpha_8_SkColorType == cacher.info().colorType();
}
GrImageTextureMaker::GrImageTextureMaker(GrContext* context, SkImageCacherator* cacher,
                                         const SkImage* client, SkImage::CachingHint chint)
    : INHERITED(context, cacher->info().width(), cacher->info().height(),
                cacher_is_alpha_only(*cacher))
    , fCacher(cacher)
    , fClient(client)
    , fCachingHint(chint) {
    if (client) {
        GrMakeKeyFromImageID(&fOriginalKey, client->uniqueID(),
                             SkIRect::MakeWH(this->width(), this->height()));
    }
}

GrTexture* GrImageTextureMaker::refOriginalTexture(bool willBeMipped,
                                                   SkSourceGammaTreatment gammaTreatment) {
    return fCacher->lockTexture(this->context(), fOriginalKey, fClient, fCachingHint, willBeMipped,
                                gammaTreatment);
}

void GrImageTextureMaker::makeCopyKey(const CopyParams& stretch, GrUniqueKey* paramsCopyKey) {
    if (fOriginalKey.isValid() && SkImage::kAllow_CachingHint == fCachingHint) {
        MakeCopyKeyFromOrigKey(fOriginalKey, stretch, paramsCopyKey);
    }
}

void GrImageTextureMaker::didCacheCopy(const GrUniqueKey& copyKey) {
    if (fClient) {
        as_IB(fClient)->notifyAddedToCache();
    }
}

SkColorSpace* GrImageTextureMaker::getColorSpace() {
    return fCacher->info().colorSpace();
}
