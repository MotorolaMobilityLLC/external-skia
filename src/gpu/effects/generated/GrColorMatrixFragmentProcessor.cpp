/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrColorMatrixFragmentProcessor.fp; do not modify.
 **************************************************************************************************/
#include "GrColorMatrixFragmentProcessor.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLColorMatrixFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    GrGLSLColorMatrixFragmentProcessor() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrColorMatrixFragmentProcessor& _outer =
                args.fFp.cast<GrColorMatrixFragmentProcessor>();
        (void)_outer;
        auto m = _outer.m;
        (void)m;
        auto v = _outer.v;
        (void)v;
        auto unpremulInput = _outer.unpremulInput;
        (void)unpremulInput;
        auto clampRGBOutput = _outer.clampRGBOutput;
        (void)clampRGBOutput;
        auto premulOutput = _outer.premulOutput;
        (void)premulOutput;
        mVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4x4_GrSLType, "m");
        vVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType, "v");
        fragBuilder->codeAppendf(
                "half4 inputColor = %s;\n@if (%s) {\n    half nonZeroAlpha = max(inputColor.w, "
                "9.9999997473787516e-05);\n    inputColor = half4(inputColor.xyz / nonZeroAlpha, "
                "inputColor.w);\n}\n%s = %s * inputColor + %s;\n@if (%s) {\n    %s = clamp(%s, "
                "0.0, 1.0);\n} else {\n    %s.w = clamp(%s.w, 0.0, 1.0);\n}\n@if (%s) {\n    "
                "%s.xyz *= %s.w;\n}\n",
                args.fInputColor, (_outer.unpremulInput ? "true" : "false"), args.fOutputColor,
                args.fUniformHandler->getUniformCStr(mVar),
                args.fUniformHandler->getUniformCStr(vVar),
                (_outer.clampRGBOutput ? "true" : "false"), args.fOutputColor, args.fOutputColor,
                args.fOutputColor, args.fOutputColor, (_outer.premulOutput ? "true" : "false"),
                args.fOutputColor, args.fOutputColor);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrColorMatrixFragmentProcessor& _outer = _proc.cast<GrColorMatrixFragmentProcessor>();
        {
            const SkM44& mValue = _outer.m;
            if (mPrev != (mValue)) {
                mPrev = mValue;
                pdman.setSkM44(mVar, mValue);
            }
            const SkV4& vValue = _outer.v;
            if (vPrev != (vValue)) {
                vPrev = vValue;
                pdman.set4fv(vVar, 1, vValue.ptr());
            }
        }
    }
    SkM44 mPrev = SkM44(SkM44::kNaN_Constructor);
    SkV4 vPrev = SkV4{SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN};
    UniformHandle mVar;
    UniformHandle vVar;
};
GrGLSLFragmentProcessor* GrColorMatrixFragmentProcessor::onCreateGLSLInstance() const {
    return new GrGLSLColorMatrixFragmentProcessor();
}
void GrColorMatrixFragmentProcessor::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                           GrProcessorKeyBuilder* b) const {
    b->add32((int32_t)unpremulInput);
    b->add32((int32_t)clampRGBOutput);
    b->add32((int32_t)premulOutput);
}
bool GrColorMatrixFragmentProcessor::onIsEqual(const GrFragmentProcessor& other) const {
    const GrColorMatrixFragmentProcessor& that = other.cast<GrColorMatrixFragmentProcessor>();
    (void)that;
    if (m != that.m) return false;
    if (v != that.v) return false;
    if (unpremulInput != that.unpremulInput) return false;
    if (clampRGBOutput != that.clampRGBOutput) return false;
    if (premulOutput != that.premulOutput) return false;
    return true;
}
GrColorMatrixFragmentProcessor::GrColorMatrixFragmentProcessor(
        const GrColorMatrixFragmentProcessor& src)
        : INHERITED(kGrColorMatrixFragmentProcessor_ClassID, src.optimizationFlags())
        , m(src.m)
        , v(src.v)
        , unpremulInput(src.unpremulInput)
        , clampRGBOutput(src.clampRGBOutput)
        , premulOutput(src.premulOutput) {}
std::unique_ptr<GrFragmentProcessor> GrColorMatrixFragmentProcessor::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrColorMatrixFragmentProcessor(*this));
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrColorMatrixFragmentProcessor);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrColorMatrixFragmentProcessor::TestCreate(
        GrProcessorTestData* d) {
    float m[20];
    for (int i = 0; i < 20; ++i) {
        m[i] = d->fRandom->nextRangeScalar(-10.f, 10.f);
    }
    bool unpremul = d->fRandom->nextBool();
    bool clampRGB = d->fRandom->nextBool();
    bool premul = d->fRandom->nextBool();
    return Make(m, unpremul, clampRGB, premul);
}
#endif
