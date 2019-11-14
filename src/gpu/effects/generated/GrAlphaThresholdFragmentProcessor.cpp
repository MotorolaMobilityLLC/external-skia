/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrAlphaThresholdFragmentProcessor.fp; do not modify.
 **************************************************************************************************/
#include "GrAlphaThresholdFragmentProcessor.h"

inline GrFragmentProcessor::OptimizationFlags GrAlphaThresholdFragmentProcessor::optFlags(
        float outerThreshold) {
    if (outerThreshold >= 1.0) {
        return kPreservesOpaqueInput_OptimizationFlag |
               kCompatibleWithCoverageAsAlpha_OptimizationFlag;
    } else {
        return kCompatibleWithCoverageAsAlpha_OptimizationFlag;
    }
}
#include "include/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLAlphaThresholdFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    GrGLSLAlphaThresholdFragmentProcessor() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrAlphaThresholdFragmentProcessor& _outer =
                args.fFp.cast<GrAlphaThresholdFragmentProcessor>();
        (void)_outer;
        auto innerThreshold = _outer.innerThreshold;
        (void)innerThreshold;
        auto outerThreshold = _outer.outerThreshold;
        (void)outerThreshold;
        innerThresholdVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                             "innerThreshold");
        outerThresholdVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                             "outerThreshold");
        SkString sk_TransformedCoords2D_0 =
                fragBuilder->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint);
        fragBuilder->codeAppendf(
                "half4 color = %s;\nhalf4 mask_color = sample(%s, %s).%s;\nif (mask_color.w < 0.5) "
                "{\n    if (color.w > %s) {\n        half scale = %s / color.w;\n        color.xyz "
                "*= scale;\n        color.w = %s;\n    }\n} else if (color.w < %s) {\n    half "
                "scale = %s / max(0.0010000000474974513, color.w);\n    color.xyz *= scale;\n    "
                "color.w = %s;\n}\n%s = color;\n",
                args.fInputColor,
                fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]),
                sk_TransformedCoords2D_0.c_str(),
                fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str(),
                args.fUniformHandler->getUniformCStr(outerThresholdVar),
                args.fUniformHandler->getUniformCStr(outerThresholdVar),
                args.fUniformHandler->getUniformCStr(outerThresholdVar),
                args.fUniformHandler->getUniformCStr(innerThresholdVar),
                args.fUniformHandler->getUniformCStr(innerThresholdVar),
                args.fUniformHandler->getUniformCStr(innerThresholdVar), args.fOutputColor);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrAlphaThresholdFragmentProcessor& _outer =
                _proc.cast<GrAlphaThresholdFragmentProcessor>();
        {
            pdman.set1f(innerThresholdVar, (_outer.innerThreshold));
            pdman.set1f(outerThresholdVar, (_outer.outerThreshold));
        }
    }
    UniformHandle innerThresholdVar;
    UniformHandle outerThresholdVar;
};
GrGLSLFragmentProcessor* GrAlphaThresholdFragmentProcessor::onCreateGLSLInstance() const {
    return new GrGLSLAlphaThresholdFragmentProcessor();
}
void GrAlphaThresholdFragmentProcessor::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                              GrProcessorKeyBuilder* b) const {}
bool GrAlphaThresholdFragmentProcessor::onIsEqual(const GrFragmentProcessor& other) const {
    const GrAlphaThresholdFragmentProcessor& that = other.cast<GrAlphaThresholdFragmentProcessor>();
    (void)that;
    if (mask != that.mask) return false;
    if (innerThreshold != that.innerThreshold) return false;
    if (outerThreshold != that.outerThreshold) return false;
    return true;
}
GrAlphaThresholdFragmentProcessor::GrAlphaThresholdFragmentProcessor(
        const GrAlphaThresholdFragmentProcessor& src)
        : INHERITED(kGrAlphaThresholdFragmentProcessor_ClassID, src.optimizationFlags())
        , maskCoordTransform(src.maskCoordTransform)
        , mask(src.mask)
        , innerThreshold(src.innerThreshold)
        , outerThreshold(src.outerThreshold) {
    this->setTextureSamplerCnt(1);
    this->addCoordTransform(&maskCoordTransform);
}
std::unique_ptr<GrFragmentProcessor> GrAlphaThresholdFragmentProcessor::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrAlphaThresholdFragmentProcessor(*this));
}
const GrFragmentProcessor::TextureSampler& GrAlphaThresholdFragmentProcessor::onTextureSampler(
        int index) const {
    return IthTextureSampler(index, mask);
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrAlphaThresholdFragmentProcessor);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrAlphaThresholdFragmentProcessor::TestCreate(
        GrProcessorTestData* testData) {
    sk_sp<GrTextureProxy> maskProxy = testData->textureProxy(GrProcessorUnitTest::kAlphaTextureIdx);
    // Make the inner and outer thresholds be in (0, 1) exclusive and be sorted correctly.
    float innerThresh = testData->fRandom->nextUScalar1() * .99f + 0.005f;
    float outerThresh = testData->fRandom->nextUScalar1() * .99f + 0.005f;
    const int kMaxWidth = 1000;
    const int kMaxHeight = 1000;
    uint32_t width = testData->fRandom->nextULessThan(kMaxWidth);
    uint32_t height = testData->fRandom->nextULessThan(kMaxHeight);
    uint32_t x = testData->fRandom->nextULessThan(kMaxWidth - width);
    uint32_t y = testData->fRandom->nextULessThan(kMaxHeight - height);
    SkIRect bounds = SkIRect::MakeXYWH(x, y, width, height);
    return GrAlphaThresholdFragmentProcessor::Make(std::move(maskProxy), innerThresh, outerThresh,
                                                   bounds);
}
#endif
