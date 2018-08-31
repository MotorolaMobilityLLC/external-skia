/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrCircleBlurFragmentProcessor.fp; do not modify.
 **************************************************************************************************/
#ifndef GrCircleBlurFragmentProcessor_DEFINED
#define GrCircleBlurFragmentProcessor_DEFINED
#include "SkTypes.h"
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
class GrCircleBlurFragmentProcessor : public GrFragmentProcessor {
public:
    const SkRect& circleRect() const { return fCircleRect; }
    float textureRadius() const { return fTextureRadius; }
    float solidRadius() const { return fSolidRadius; }

    static std::unique_ptr<GrFragmentProcessor> Make(GrProxyProvider*, const SkRect& circle,
                                                     float sigma);
    GrCircleBlurFragmentProcessor(const GrCircleBlurFragmentProcessor& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "CircleBlurFragmentProcessor"; }

private:
    GrCircleBlurFragmentProcessor(SkRect circleRect, float textureRadius, float solidRadius,
                                  sk_sp<GrTextureProxy> blurProfileSampler)
            : INHERITED(kGrCircleBlurFragmentProcessor_ClassID,
                        (OptimizationFlags)kCompatibleWithCoverageAsAlpha_OptimizationFlag)
            , fCircleRect(circleRect)
            , fTextureRadius(textureRadius)
            , fSolidRadius(solidRadius)
            , fBlurProfileSampler(std::move(blurProfileSampler)) {
        this->setTextureSamplerCnt(1);
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    const TextureSampler& onTextureSampler(int) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    SkRect fCircleRect;
    float fTextureRadius;
    float fSolidRadius;
    TextureSampler fBlurProfileSampler;
    typedef GrFragmentProcessor INHERITED;
};
#endif
