/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This file was autogenerated from GrAlphaThresholdFragmentProcessor.fp; do not modify.
 */
#include "GrAlphaThresholdFragmentProcessor.h"
#if SK_SUPPORT_GPU

    inline GrFragmentProcessor::OptimizationFlags GrAlphaThresholdFragmentProcessor::optFlags(
                                                                             float outerThreshold) {
        if (outerThreshold >= 1.0) {
            return kPreservesOpaqueInput_OptimizationFlag |
                   kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        } else {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        }
    }
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLAlphaThresholdFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    GrGLSLAlphaThresholdFragmentProcessor() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrAlphaThresholdFragmentProcessor& _outer = args.fFp.cast<GrAlphaThresholdFragmentProcessor>();
        (void) _outer;
        fColorSpaceHelper.emitCode(args.fUniformHandler, _outer.colorXform().get());
        fInnerThresholdVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kFloat_GrSLType, kDefault_GrSLPrecision, "innerThreshold");
        fOuterThresholdVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kFloat_GrSLType, kDefault_GrSLPrecision, "outerThreshold");
        SkSL::String sk_TransformedCoords2D_0 = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
        SkSL::String sk_TransformedCoords2D_1 = fragBuilder->ensureCoords2D(args.fTransformedCoords[1]);
        fragBuilder->codeAppendf("vec4 _tmp0;\nvec4 color = (_tmp0 = texture(%s, %s).%s , %s != mat4(1.0) ? vec4(clamp((%s * vec4(_tmp0.xyz, 1.0)).xyz, 0.0, _tmp0.w), _tmp0.w) : _tmp0);\nvec4 mask_color = texture(%s, %s).%s;\nif (mask_color.w < 0.5) {\n    if (color.w > %s) {\n        float scale = %s / color.w;\n        color.xyz *= scale;\n        color.w = %s;\n    }\n} else if (color.w < %s) {\n    float scale = %s / max(0.001, color.w);\n    color.xyz *= scale;\n    color.w = %s;\n}\n%s = color;\n", fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]).c_str(), sk_TransformedCoords2D_0.c_str(), fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str(), fColorSpaceHelper.isValid() ? args.fUniformHandler->getUniformCStr(fColorSpaceHelper.gamutXformUniform()) : "mat4(1.0)", fColorSpaceHelper.isValid() ? args.fUniformHandler->getUniformCStr(fColorSpaceHelper.gamutXformUniform()) : "mat4(1.0)", fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[1]).c_str(), sk_TransformedCoords2D_1.c_str(), fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[1]).c_str(), args.fUniformHandler->getUniformCStr(fOuterThresholdVar), args.fUniformHandler->getUniformCStr(fOuterThresholdVar), args.fUniformHandler->getUniformCStr(fOuterThresholdVar), args.fUniformHandler->getUniformCStr(fInnerThresholdVar), args.fUniformHandler->getUniformCStr(fInnerThresholdVar), args.fUniformHandler->getUniformCStr(fInnerThresholdVar), args.fOutputColor);
    }
private:
    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& _proc) override {
        const GrAlphaThresholdFragmentProcessor& _outer = _proc.cast<GrAlphaThresholdFragmentProcessor>();
        {
        if (fColorSpaceHelper.isValid()) {
            fColorSpaceHelper.setData(pdman, _outer.colorXform().get());
        }
        pdman.set1f(fInnerThresholdVar, _outer.innerThreshold());
        pdman.set1f(fOuterThresholdVar, _outer.outerThreshold());
        }
    }
    UniformHandle fImageVar;
    UniformHandle fMaskVar;
    UniformHandle fInnerThresholdVar;
    UniformHandle fOuterThresholdVar;
    GrGLSLColorSpaceXformHelper fColorSpaceHelper;
};
GrGLSLFragmentProcessor* GrAlphaThresholdFragmentProcessor::onCreateGLSLInstance() const {
    return new GrGLSLAlphaThresholdFragmentProcessor();
}
void GrAlphaThresholdFragmentProcessor::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
    b->add32(GrColorSpaceXform::XformKey(fColorXform.get()));
}
bool GrAlphaThresholdFragmentProcessor::onIsEqual(const GrFragmentProcessor& other) const {
    const GrAlphaThresholdFragmentProcessor& that = other.cast<GrAlphaThresholdFragmentProcessor>();
    (void) that;
    if (fImage != that.fImage) return false;
    if (fColorXform != that.fColorXform) return false;
    if (fMask != that.fMask) return false;
    if (fInnerThreshold != that.fInnerThreshold) return false;
    if (fOuterThreshold != that.fOuterThreshold) return false;
    return true;
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrAlphaThresholdFragmentProcessor);
#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrAlphaThresholdFragmentProcessor::TestCreate(GrProcessorTestData* testData) {

    sk_sp<GrTextureProxy> bmpProxy = testData->textureProxy(GrProcessorUnitTest::kSkiaPMTextureIdx);
    sk_sp<GrTextureProxy> maskProxy = testData->textureProxy(GrProcessorUnitTest::kAlphaTextureIdx);
    
    float innerThresh = testData->fRandom->nextUScalar1() * .99f + 0.005f;
    float outerThresh = testData->fRandom->nextUScalar1() * .99f + 0.005f;
    const int kMaxWidth = 1000;
    const int kMaxHeight = 1000;
    uint32_t width = testData->fRandom->nextULessThan(kMaxWidth);
    uint32_t height = testData->fRandom->nextULessThan(kMaxHeight);
    uint32_t x = testData->fRandom->nextULessThan(kMaxWidth - width);
    uint32_t y = testData->fRandom->nextULessThan(kMaxHeight - height);
    SkIRect bounds = SkIRect::MakeXYWH(x, y, width, height);
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrTest::TestColorXform(testData->fRandom);
    return GrAlphaThresholdFragmentProcessor::Make(
                                std::move(bmpProxy),
                                colorSpaceXform,
                                std::move(maskProxy),
                                innerThresh, outerThresh,
                                bounds);
}
#endif
#endif
