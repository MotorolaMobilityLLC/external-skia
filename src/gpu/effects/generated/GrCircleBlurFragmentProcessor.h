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
#include "include/core/SkTypes.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"
class GrCircleBlurFragmentProcessor : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(GrProxyProvider*, const SkRect& circle,
                                                     float sigma);
    GrCircleBlurFragmentProcessor(const GrCircleBlurFragmentProcessor& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "CircleBlurFragmentProcessor"; }
    SkRect circleRect;
    float textureRadius;
    float solidRadius;
    TextureSampler blurProfileSampler;

private:
    GrCircleBlurFragmentProcessor(SkRect circleRect, float textureRadius, float solidRadius,
                                  sk_sp<GrSurfaceProxy> blurProfileSampler)
            : INHERITED(kGrCircleBlurFragmentProcessor_ClassID,
                        (OptimizationFlags)kCompatibleWithCoverageAsAlpha_OptimizationFlag)
            , circleRect(circleRect)
            , textureRadius(textureRadius)
            , solidRadius(solidRadius)
            , blurProfileSampler(std::move(blurProfileSampler)) {
        this->setTextureSamplerCnt(1);
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    const TextureSampler& onTextureSampler(int) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    typedef GrFragmentProcessor INHERITED;
};
#endif
