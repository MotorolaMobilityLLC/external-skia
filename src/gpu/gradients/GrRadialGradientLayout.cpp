/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrRadialGradientLayout.fp; do not modify.
 **************************************************************************************************/
#include "GrRadialGradientLayout.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrTexture.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLRadialGradientLayout : public GrGLSLFragmentProcessor {
public:
    GrGLSLRadialGradientLayout() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrRadialGradientLayout& _outer = args.fFp.cast<GrRadialGradientLayout>();
        (void)_outer;
        auto gradientMatrix = _outer.gradientMatrix();
        (void)gradientMatrix;
        SkString sk_TransformedCoords2D_0 = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
        fragBuilder->codeAppendf("half t = half(length(%s));\n%s = half4(t, 1.0, 0.0, 0.0);\n",
                                 sk_TransformedCoords2D_0.c_str(), args.fOutputColor);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {}
};
GrGLSLFragmentProcessor* GrRadialGradientLayout::onCreateGLSLInstance() const {
    return new GrGLSLRadialGradientLayout();
}
void GrRadialGradientLayout::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                   GrProcessorKeyBuilder* b) const {}
bool GrRadialGradientLayout::onIsEqual(const GrFragmentProcessor& other) const {
    const GrRadialGradientLayout& that = other.cast<GrRadialGradientLayout>();
    (void)that;
    if (fGradientMatrix != that.fGradientMatrix) return false;
    return true;
}
GrRadialGradientLayout::GrRadialGradientLayout(const GrRadialGradientLayout& src)
        : INHERITED(kGrRadialGradientLayout_ClassID, src.optimizationFlags())
        , fGradientMatrix(src.fGradientMatrix)
        , fCoordTransform0(src.fCoordTransform0) {
    this->addCoordTransform(&fCoordTransform0);
}
std::unique_ptr<GrFragmentProcessor> GrRadialGradientLayout::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrRadialGradientLayout(*this));
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrRadialGradientLayout);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrRadialGradientLayout::TestCreate(GrProcessorTestData* d) {
    sk_sp<SkShader> shader;
    do {
        GrGradientShader::RandomParams params(d->fRandom);
        SkPoint center = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
        SkScalar radius = d->fRandom->nextUScalar1();
        shader = params.fUseColors4f
                         ? SkGradientShader::MakeRadial(center, radius, params.fColors4f,
                                                        params.fColorSpace, params.fStops,
                                                        params.fColorCount, params.fTileMode)
                         : SkGradientShader::MakeRadial(center, radius, params.fColors,
                                                        params.fStops, params.fColorCount,
                                                        params.fTileMode);
    } while (!shader);
    GrTest::TestAsFPArgs asFPArgs(d);
    std::unique_ptr<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());
    GrAlwaysAssert(fp);
    return fp;
}
#endif

std::unique_ptr<GrFragmentProcessor> GrRadialGradientLayout::Make(const SkRadialGradient& grad,
                                                                  const GrFPArgs& args) {
    SkMatrix matrix;
    if (!grad.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    matrix.postConcat(grad.getGradientMatrix());
    return std::unique_ptr<GrFragmentProcessor>(new GrRadialGradientLayout(matrix));
}
