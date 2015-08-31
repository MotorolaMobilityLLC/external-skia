/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapController.h"
#include "SkMatrix.h"
#include "SkTemplates.h"

// RESIZE_LANCZOS3 is another good option, but chrome prefers mitchell at the moment
#define kHQ_RESIZE_METHOD   SkBitmapScaler::RESIZE_MITCHELL

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool valid_for_drawing(const SkBitmap& bm) {
    if (0 == bm.width() || 0 == bm.height()) {
        return false;   // nothing to draw
    }
    if (nullptr == bm.pixelRef()) {
        return false;   // no pixels to read
    }
    if (bm.getTexture()) {
        // we can handle texture (ugh) since lockPixels will perform a read-back
        return true;
    }
    if (kIndex_8_SkColorType == bm.colorType()) {
        SkAutoLockPixels alp(bm); // but we need to call it before getColorTable() is safe.
        if (!bm.getColorTable()) {
            return false;
        }
    }
    return true;
}

SkBitmapController::State* SkBitmapController::requestBitmap(const SkBitmap& bm,
                                                             const SkMatrix& inv,
                                                             SkFilterQuality quality,
                                                             void* storage, size_t storageSize) {
    
    if (!valid_for_drawing(bm)) {
        return nullptr;
    }

    State* state = this->onRequestBitmap(bm, inv, quality, storage, storageSize);
    if (state) {
        if (nullptr == state->fPixmap.addr()) {
            SkInPlaceDeleteCheck(state, storage);
            state = nullptr;
        }
    }
    return state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkBitmapCache.h"
#include "SkBitmapScaler.h"
#include "SkMipMap.h"
#include "SkResourceCache.h"

class SkDefaultBitmapControllerState : public SkBitmapController::State {
public:
    SkDefaultBitmapControllerState(const SkBitmap& src, const SkMatrix& inv, SkFilterQuality qual);

private:
    SkBitmap                     fResultBitmap;
    SkAutoTUnref<const SkMipMap> fCurrMip;
    
    bool processHQRequest(const SkBitmap& orig);
    bool processMediumRequest(const SkBitmap& orig);
};

// Check to see that the size of the bitmap that would be produced by
// scaling by the given inverted matrix is less than the maximum allowed.
static inline bool cache_size_okay(const SkBitmap& bm, const SkMatrix& invMat) {
    size_t maximumAllocation = SkResourceCache::GetEffectiveSingleAllocationByteLimit();
    if (0 == maximumAllocation) {
        return true;
    }
    // float matrixScaleFactor = 1.0 / (invMat.scaleX * invMat.scaleY);
    // return ((origBitmapSize * matrixScaleFactor) < maximumAllocationSize);
    // Skip the division step:
    const size_t size = bm.info().getSafeSize(bm.info().minRowBytes());
    return size < (maximumAllocation * invMat.getScaleX() * invMat.getScaleY());
}

/*
 *  High quality is implemented by performing up-right scale-only filtering and then
 *  using bilerp for any remaining transformations.
 */
bool SkDefaultBitmapControllerState::processHQRequest(const SkBitmap& origBitmap) {
    if (fQuality != kHigh_SkFilterQuality) {
        return false;
    }
    
    // Our default return state is to downgrade the request to Medium, w/ or w/o setting fBitmap
    // to a valid bitmap. If we succeed, we will set this to Low instead.
    fQuality = kMedium_SkFilterQuality;
    
    if (kN32_SkColorType != origBitmap.colorType() || !cache_size_okay(origBitmap, fInvMatrix) ||
        fInvMatrix.hasPerspective())
    {
        return false; // can't handle the reqeust
    }
    
    SkScalar invScaleX = fInvMatrix.getScaleX();
    SkScalar invScaleY = fInvMatrix.getScaleY();
    if (fInvMatrix.getType() & SkMatrix::kAffine_Mask) {
        SkSize scale;
        if (!fInvMatrix.decomposeScale(&scale)) {
            return false;
        }
        invScaleX = scale.width();
        invScaleY = scale.height();
    }
    if (SkScalarNearlyEqual(invScaleX, 1) && SkScalarNearlyEqual(invScaleY, 1)) {
        return false; // no need for HQ
    }
    
    const int dstW = SkScalarRoundToScalar(origBitmap.width() / invScaleX);
    const int dstH = SkScalarRoundToScalar(origBitmap.height() / invScaleY);
    
    if (!SkBitmapCache::FindWH(origBitmap, dstW, dstH, &fResultBitmap)) {
        SkAutoPixmapUnlock src;
        if (!origBitmap.requestLock(&src)) {
            return false;
        }
        if (!SkBitmapScaler::Resize(&fResultBitmap, src.pixmap(), kHQ_RESIZE_METHOD,
                                    dstW, dstH, SkResourceCache::GetAllocator())) {
            return false; // we failed to create fScaledBitmap
        }
        
        SkASSERT(fResultBitmap.getPixels());
        fResultBitmap.setImmutable();
        SkBitmapCache::AddWH(origBitmap, dstW, dstH, fResultBitmap);
    }
    
    SkASSERT(fResultBitmap.getPixels());
    
    fInvMatrix.postScale(SkIntToScalar(dstW) / origBitmap.width(),
                         SkIntToScalar(dstH) / origBitmap.height());
    fQuality = kLow_SkFilterQuality;
    return true;
}

/*
 *  Modulo internal errors, this should always succeed *if* the matrix is downscaling
 *  (in this case, we have the inverse, so it succeeds if fInvMatrix is upscaling)
 */
bool SkDefaultBitmapControllerState::processMediumRequest(const SkBitmap& origBitmap) {
    SkASSERT(fQuality <= kMedium_SkFilterQuality);
    if (fQuality != kMedium_SkFilterQuality) {
        return false;
    }
    
    // Our default return state is to downgrade the request to Low, w/ or w/o setting fBitmap
    // to a valid bitmap.
    fQuality = kLow_SkFilterQuality;
    
    SkSize invScaleSize;
    if (!fInvMatrix.decomposeScale(&invScaleSize, nullptr)) {
        return false;
    }
    SkScalar invScale = SkScalarSqrt(invScaleSize.width() * invScaleSize.height());
    
    if (invScale > SK_Scalar1) {
        fCurrMip.reset(SkMipMapCache::FindAndRef(origBitmap));
        if (nullptr == fCurrMip.get()) {
            fCurrMip.reset(SkMipMapCache::AddAndRef(origBitmap));
            if (nullptr == fCurrMip.get()) {
                return false;
            }
        }
        // diagnostic for a crasher...
        if (nullptr == fCurrMip->data()) {
            sk_throw();
        }
        
        SkScalar levelScale = SkScalarInvert(invScale);
        SkMipMap::Level level;
        if (fCurrMip->extractLevel(levelScale, &level)) {
            SkScalar invScaleFixup = level.fScale;
            fInvMatrix.postScale(invScaleFixup, invScaleFixup);
            
            const SkImageInfo info = origBitmap.info().makeWH(level.fWidth, level.fHeight);
            // todo: if we could wrap the fCurrMip in a pixelref, then we could just install
            //       that here, and not need to explicitly track it ourselves.
            return fResultBitmap.installPixels(info, level.fPixels, level.fRowBytes);
        } else {
            // failed to extract, so release the mipmap
            fCurrMip.reset(nullptr);
        }
    }
    return false;
}

SkDefaultBitmapControllerState::SkDefaultBitmapControllerState(const SkBitmap& src,
                                                               const SkMatrix& inv,
                                                               SkFilterQuality qual) {
    fInvMatrix = inv;
    fQuality = qual;

    if (this->processHQRequest(src) || this->processMediumRequest(src)) {
        SkASSERT(fResultBitmap.getPixels());
    } else {
        fResultBitmap = src;
        fResultBitmap.lockPixels();
        // lock may fail to give us pixels
    }
    SkASSERT(fQuality <= kLow_SkFilterQuality);

    // fResultBitmap.getPixels() may be null, but our caller knows to check fPixmap.addr()
    // and will destroy us if it is nullptr.
    fPixmap.reset(fResultBitmap.info(), fResultBitmap.getPixels(), fResultBitmap.rowBytes(),
                  fResultBitmap.getColorTable());
}

SkBitmapController::State* SkDefaultBitmapController::onRequestBitmap(const SkBitmap& bm,
                                                                      const SkMatrix& inverse,
                                                                      SkFilterQuality quality,
                                                                      void* storage, size_t size) {
    return SkInPlaceNewCheck<SkDefaultBitmapControllerState>(storage, size, bm, inverse, quality);
}

