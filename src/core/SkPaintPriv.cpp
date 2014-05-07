/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaintPriv.h"

#include "SkBitmap.h"
#include "SkColorFilter.h"
#include "SkPaint.h"
#include "SkShader.h"

bool isPaintOpaque(const SkPaint* paint,
                   const SkBitmap* bmpReplacesShader) {
    // TODO: SkXfermode should have a virtual isOpaque method, which would
    // make it possible to test modes that do not have a Coeff representation.

    if (!paint) {
        return bmpReplacesShader ? bmpReplacesShader->isOpaque() : true;
    }

    SkXfermode::Coeff srcCoeff, dstCoeff;
    if (SkXfermode::AsCoeff(paint->getXfermode(), &srcCoeff, &dstCoeff)){
        if (SkXfermode::kDA_Coeff == srcCoeff || SkXfermode::kDC_Coeff == srcCoeff ||
            SkXfermode::kIDA_Coeff == srcCoeff || SkXfermode::kIDC_Coeff == srcCoeff) {
            return false;
        }
        switch (dstCoeff) {
        case SkXfermode::kZero_Coeff:
            return true;
        case SkXfermode::kISA_Coeff:
            if (paint->getAlpha() != 255) {
                break;
            }
            if (bmpReplacesShader) {
                if (!bmpReplacesShader->isOpaque()) {
                    break;
                }
            } else if (paint->getShader() && !paint->getShader()->isOpaque()) {
                break;
            }
            if (paint->getColorFilter() &&
                ((paint->getColorFilter()->getFlags() &
                SkColorFilter::kAlphaUnchanged_Flag) == 0)) {
                break;
            }
            return true;
        case SkXfermode::kSA_Coeff:
            if (paint->getAlpha() != 0) {
                break;
            }
            if (paint->getColorFilter() &&
                ((paint->getColorFilter()->getFlags() &
                SkColorFilter::kAlphaUnchanged_Flag) == 0)) {
                break;
            }
            return true;
        case SkXfermode::kSC_Coeff:
            if (paint->getColor() != 0) { // all components must be 0
                break;
            }
            if (bmpReplacesShader || paint->getShader()) {
                break;
            }
            if (paint->getColorFilter() && (
                (paint->getColorFilter()->getFlags() &
                SkColorFilter::kAlphaUnchanged_Flag) == 0)) {
                break;
            }
            return true;
        default:
            break;
        }
    }
    return false;
}

bool NeedsDeepCopy(const SkPaint& paint) {
    /*
     *  These fields are known to be immutable, and so can be shallow-copied
     *
     *  getTypeface()
     *  getAnnotation()
     *  paint.getColorFilter()
     *  getXfermode()
     *  getPathEffect()
     *  getMaskFilter()
     */

    return paint.getShader() ||
#ifdef SK_SUPPORT_LEGACY_LAYERRASTERIZER_API
           paint.getRasterizer() ||
#endif
           paint.getLooper() || // needs to hide its addLayer...
           paint.getImageFilter();
}
