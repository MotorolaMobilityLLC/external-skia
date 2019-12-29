/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrSimpleTextureEffect.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLSimpleTextureEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLSimpleTextureEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrSimpleTextureEffect& _outer = args.fFp.cast<GrSimpleTextureEffect>();
        (void)_outer;
        auto matrix = _outer.matrix;
        (void)matrix;
        SkString sk_TransformedCoords2D_0 =
                fragBuilder->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint);
        fragBuilder->codeAppendf(
                "%s = %s * sample(%s, %s).%s;\n", args.fOutputColor, args.fInputColor,
                fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]),
                sk_TransformedCoords2D_0.c_str(),
                fragBuilder->getProgramBuilder()
                        ->samplerSwizzle(args.fTexSamplers[0])
                        .asString()
                        .c_str());
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
    if (image != that.image) return false;
    if (matrix != that.matrix) return false;
    return true;
}
GrSimpleTextureEffect::GrSimpleTextureEffect(const GrSimpleTextureEffect& src)
        : INHERITED(kGrSimpleTextureEffect_ClassID, src.optimizationFlags())
        , imageCoordTransform(src.imageCoordTransform)
        , image(src.image)
        , matrix(src.matrix) {
    this->setTextureSamplerCnt(1);
    this->addCoordTransform(&imageCoordTransform);
}
std::unique_ptr<GrFragmentProcessor> GrSimpleTextureEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrSimpleTextureEffect(*this));
}
const GrFragmentProcessor::TextureSampler& GrSimpleTextureEffect::onTextureSampler(
        int index) const {
    return IthTextureSampler(index, image);
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSimpleTextureEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrSimpleTextureEffect::TestCreate(
        GrProcessorTestData* testData) {
    auto[proxy, ct, at] = testData->randomProxy();
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
    return GrSimpleTextureEffect::Make(std::move(proxy), at, matrix, params);
}
#endif
