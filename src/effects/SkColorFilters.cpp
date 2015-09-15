/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkModeColorFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkUtils.h"
#include "SkString.h"
#include "SkValidationUtils.h"
#include "SkColorMatrixFilter.h"

bool SkModeColorFilter::asColorMode(SkColor* color, SkXfermode::Mode* mode) const {
    if (color) {
        *color = fColor;
    }
    if (mode) {
        *mode = fMode;
    }
    return true;
}

uint32_t SkModeColorFilter::getFlags() const {
    switch (fMode) {
        case SkXfermode::kDst_Mode:      //!< [Da, Dc]
        case SkXfermode::kSrcATop_Mode:  //!< [Da, Sc * Da + (1 - Sa) * Dc]
            return kAlphaUnchanged_Flag;
        default:
            break;
    }
    return 0;
}

void SkModeColorFilter::filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const {
    SkPMColor       color = fPMColor;
    SkXfermodeProc  proc = fProc;

    for (int i = 0; i < count; i++) {
        result[i] = proc(color, shader[i]);
    }
}

void SkModeColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fColor);
    buffer.writeUInt(fMode);
}

void SkModeColorFilter::updateCache() {
    fPMColor = SkPreMultiplyColor(fColor);
    fProc = SkXfermode::GetProc(fMode);
}

SkFlattenable* SkModeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkColor color = buffer.readColor();
    SkXfermode::Mode mode = (SkXfermode::Mode)buffer.readUInt();
    return SkColorFilter::CreateModeFilter(color, mode);
}

///////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "GrBlend.h"
#include "GrInvariantOutput.h"
#include "effects/GrXfermodeFragmentProcessor.h"
#include "effects/GrConstColorProcessor.h"
#include "SkGr.h"

bool SkModeColorFilter::asFragmentProcessors(GrContext*, GrProcessorDataManager*,
                                             SkTDArray<const GrFragmentProcessor*>* array) const {
    if (SkXfermode::kDst_Mode != fMode) {
        SkAutoTUnref<const GrFragmentProcessor> constFP(
            GrConstColorProcessor::Create(SkColor2GrColor(fColor),
                                          GrConstColorProcessor::kIgnore_InputMode));
        const GrFragmentProcessor* fp =
            GrXfermodeFragmentProcessor::CreateFromSrcProcessor(constFP, fMode);
        if (fp) {
#ifdef SK_DEBUG
            // With a solid color input this should always be able to compute the blended color
            // (at least for coeff modes)
            if (fMode <= SkXfermode::kLastCoeffMode) {
                static SkRandom gRand;
                GrInvariantOutput io(GrPremulColor(gRand.nextU()), kRGBA_GrColorComponentFlags,
                                     false);
                fp->computeInvariantOutput(&io);
                SkASSERT(io.validFlags() == kRGBA_GrColorComponentFlags);
            }
#endif
            if (array) {
                *array->append() = fp;
            } else {
                fp->unref();
                SkDEBUGCODE(fp = nullptr;)
            }
            return true;
        }
    }
    return false;
}

#endif

///////////////////////////////////////////////////////////////////////////////

class Src_SkModeColorFilter : public SkModeColorFilter {
public:
    Src_SkModeColorFilter(SkColor color) : INHERITED(color, SkXfermode::kSrc_Mode) {}

    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override {
        sk_memset32(result, this->getPMColor(), count);
    }

private:
    typedef SkModeColorFilter INHERITED;
};

class SrcOver_SkModeColorFilter : public SkModeColorFilter {
public:
    SrcOver_SkModeColorFilter(SkColor color) : INHERITED(color, SkXfermode::kSrcOver_Mode) { }

    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override {
        SkBlitRow::Color32(result, shader, count, this->getPMColor());
    }

private:
    typedef SkModeColorFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkColorFilter* SkColorFilter::CreateModeFilter(SkColor color, SkXfermode::Mode mode) {
    if (!SkIsValidMode(mode)) {
        return nullptr;
    }

    unsigned alpha = SkColorGetA(color);

    // first collaps some modes if possible

    if (SkXfermode::kClear_Mode == mode) {
        color = 0;
        mode = SkXfermode::kSrc_Mode;
    } else if (SkXfermode::kSrcOver_Mode == mode) {
        if (0 == alpha) {
            mode = SkXfermode::kDst_Mode;
        } else if (255 == alpha) {
            mode = SkXfermode::kSrc_Mode;
        }
        // else just stay srcover
    }

    // weed out combinations that are noops, and just return null
    if (SkXfermode::kDst_Mode == mode ||
        (0 == alpha && (SkXfermode::kSrcOver_Mode == mode ||
                        SkXfermode::kDstOver_Mode == mode ||
                        SkXfermode::kDstOut_Mode == mode ||
                        SkXfermode::kSrcATop_Mode == mode ||
                        SkXfermode::kXor_Mode == mode ||
                        SkXfermode::kDarken_Mode == mode)) ||
            (0xFF == alpha && SkXfermode::kDstIn_Mode == mode)) {
        return nullptr;
    }

    switch (mode) {
        case SkXfermode::kSrc_Mode:
            return new Src_SkModeColorFilter(color);
        case SkXfermode::kSrcOver_Mode:
            return new SrcOver_SkModeColorFilter(color);
        default:
            return new SkModeColorFilter(color, mode);
    }
}

///////////////////////////////////////////////////////////////////////////////

static SkScalar byte_to_scale(U8CPU byte) {
    if (0xFF == byte) {
        // want to get this exact
        return 1;
    } else {
        return byte * 0.00392156862745f;
    }
}

SkColorFilter* SkColorFilter::CreateLightingFilter(SkColor mul, SkColor add) {
    SkColorMatrix matrix;
    matrix.setScale(byte_to_scale(SkColorGetR(mul)),
                    byte_to_scale(SkColorGetG(mul)),
                    byte_to_scale(SkColorGetB(mul)),
                    1);
    matrix.postTranslate(SkIntToScalar(SkColorGetR(add)),
                         SkIntToScalar(SkColorGetG(add)),
                         SkIntToScalar(SkColorGetB(add)),
                         0);
    return SkColorMatrixFilter::Create(matrix);
}

