/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrSimpleTextureEffect.fp; do not modify.
 **************************************************************************************************/
#include "GrSimpleTextureEffect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrTexture.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLSimpleTextureEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLSimpleTextureEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrSimpleTextureEffect& _outer = args.fFp.cast<GrSimpleTextureEffect>();
        (void)_outer;
        auto matrix = _outer.matrix();
        (void)matrix;
        SkString sk_TransformedCoords2D_0 = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
        fragBuilder->codeAppendf(
                "%s = %s * texture(%s, %s).%s;\n", args.fOutputColor,
                args.fInputColor ? args.fInputColor : "half4(1)",
                fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]).c_str(),
                sk_TransformedCoords2D_0.c_str(),
                fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str());
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {}
};
GrGLSLFragmentProcessor* GrSimpleTextureEffect::onCreateGLSLInstance() const {
    return new GrGLSLSimpleTextureEffect();
}
void GrSimpleTextureEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {}
bool GrSimpleTextureEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSimpleTextureEffect& that = other.cast<GrSimpleTextureEffect>();
    (void)that;
    if (fImage != that.fImage) return false;
    if (fMatrix != that.fMatrix) return false;
    return true;
}
GrSimpleTextureEffect::GrSimpleTextureEffect(const GrSimpleTextureEffect& src)
        : INHERITED(kGrSimpleTextureEffect_ClassID, src.optimizationFlags())
        , fImage(src.fImage)
        , fMatrix(src.fMatrix)
        , fImageCoordTransform(src.fImageCoordTransform) {
    this->setTextureSamplerCnt(1);
    this->addCoordTransform(&fImageCoordTransform);
}
std::unique_ptr<GrFragmentProcessor> GrSimpleTextureEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrSimpleTextureEffect(*this));
}
const GrFragmentProcessor::TextureSampler& GrSimpleTextureEffect::onTextureSampler(
        int index) const {
    return IthTextureSampler(index, fImage);
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSimpleTextureEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrSimpleTextureEffect::TestCreate(
        GrProcessorTestData* testData) {
    int texIdx = testData->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                               : GrProcessorUnitTest::kAlphaTextureIdx;
    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(testData->fRandom, wrapModes);
    if (!testData->caps()->npotTextureTileSupport()) {
        // Performing repeat sampling on npot textures will cause asserts on HW
        // that lacks support.
        wrapModes[0] = GrSamplerState::WrapMode::kClamp;
        wrapModes[1] = GrSamplerState::WrapMode::kClamp;
    }

    GrSamplerState params(wrapModes, testData->fRandom->nextBool()
                                             ? GrSamplerState::Filter::kBilerp
                                             : GrSamplerState::Filter::kNearest);

    const SkMatrix& matrix = GrTest::TestMatrix(testData->fRandom);
    return GrSimpleTextureEffect::Make(testData->textureProxy(texIdx), matrix, params);
}
#endif
