/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrClampedGradientEffect.fp; do not modify.
 **************************************************************************************************/
#ifndef GrClampedGradientEffect_DEFINED
#define GrClampedGradientEffect_DEFINED
#include "SkTypes.h"
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
class GrClampedGradientEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(
            std::unique_ptr<GrFragmentProcessor> colorizer,
            std::unique_ptr<GrFragmentProcessor> gradLayout, SkPMColor4f leftBorderColor,
            SkPMColor4f rightBorderColor, bool makePremul, bool colorsAreOpaque) {
        return std::unique_ptr<GrFragmentProcessor>(new GrClampedGradientEffect(
                std::move(colorizer), std::move(gradLayout), leftBorderColor, rightBorderColor,
                makePremul, colorsAreOpaque));
    }
    GrClampedGradientEffect(const GrClampedGradientEffect& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "ClampedGradientEffect"; }
    int colorizer_index = -1;
    int gradLayout_index = -1;
    SkPMColor4f leftBorderColor;
    SkPMColor4f rightBorderColor;
    bool makePremul;
    bool colorsAreOpaque;

private:
    GrClampedGradientEffect(std::unique_ptr<GrFragmentProcessor> colorizer,
                            std::unique_ptr<GrFragmentProcessor> gradLayout,
                            SkPMColor4f leftBorderColor, SkPMColor4f rightBorderColor,
                            bool makePremul, bool colorsAreOpaque)
            : INHERITED(kGrClampedGradientEffect_ClassID,
                        (OptimizationFlags)kCompatibleWithCoverageAsAlpha_OptimizationFlag |
                                (colorsAreOpaque && gradLayout->preservesOpaqueInput()
                                         ? kPreservesOpaqueInput_OptimizationFlag
                                         : kNone_OptimizationFlags))
            , leftBorderColor(leftBorderColor)
            , rightBorderColor(rightBorderColor)
            , makePremul(makePremul)
            , colorsAreOpaque(colorsAreOpaque) {
        SkASSERT(colorizer);
        colorizer_index = this->numChildProcessors();
        this->registerChildProcessor(std::move(colorizer));
        SkASSERT(gradLayout);
        gradLayout_index = this->numChildProcessors();
        this->registerChildProcessor(std::move(gradLayout));
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    typedef GrFragmentProcessor INHERITED;
};
#endif
