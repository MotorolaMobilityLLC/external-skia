/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrRectBlurEffect.fp; do not modify.
 **************************************************************************************************/
#include "GrRectBlurEffect.h"

#include "src/core/SkUtils.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLRectBlurEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLRectBlurEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrRectBlurEffect& _outer = args.fFp.cast<GrRectBlurEffect>();
        (void)_outer;
        auto rect = _outer.rect;
        (void)rect;
        auto isFast = _outer.isFast;
        (void)isFast;
        highp = ((abs(rect.left()) > 16000.0 || abs(rect.top()) > 16000.0) ||
                 abs(rect.right()) > 16000.0) ||
                abs(rect.bottom()) > 16000.0;
        if (highp) {
            rectFVar = args.fUniformHandler->addUniform(&_outer, kFragment_GrShaderFlag,
                                                        kFloat4_GrSLType, "rectF");
        }
        if (!highp) {
            rectHVar = args.fUniformHandler->addUniform(&_outer, kFragment_GrShaderFlag,
                                                        kHalf4_GrSLType, "rectH");
        }
        fragBuilder->codeAppendf(
                R"SkSL(/* key */ bool highp = %s;
half xCoverage, yCoverage;
@if (%s) {
    half2 xy;
    @if (highp) {
        xy = max(half2(%s.xy - sk_FragCoord.xy), half2(sk_FragCoord.xy - %s.zw));
    } else {
        xy = max(half2(float2(%s.xy) - sk_FragCoord.xy), half2(sk_FragCoord.xy - float2(%s.zw)));
    })SkSL",
                (highp ? "true" : "false"), (_outer.isFast ? "true" : "false"),
                rectFVar.isValid() ? args.fUniformHandler->getUniformCStr(rectFVar) : "float4(0)",
                rectFVar.isValid() ? args.fUniformHandler->getUniformCStr(rectFVar) : "float4(0)",
                rectHVar.isValid() ? args.fUniformHandler->getUniformCStr(rectHVar) : "half4(0)",
                rectHVar.isValid() ? args.fUniformHandler->getUniformCStr(rectHVar) : "half4(0)");
        SkString _coords7176("float2(half2(xy.x, 0.5))");
        SkString _sample7176 = this->invokeChild(1, args, _coords7176.c_str());
        fragBuilder->codeAppendf(
                R"SkSL(
    xCoverage = %s.w;)SkSL",
                _sample7176.c_str());
        SkString _coords7234("float2(half2(xy.y, 0.5))");
        SkString _sample7234 = this->invokeChild(1, args, _coords7234.c_str());
        fragBuilder->codeAppendf(
                R"SkSL(
    yCoverage = %s.w;
} else {
    half4 rect;
    @if (highp) {
        rect.xy = half2(%s.xy - sk_FragCoord.xy);
        rect.zw = half2(sk_FragCoord.xy - %s.zw);
    } else {
        rect.xy = half2(float2(%s.xy) - sk_FragCoord.xy);
        rect.zw = half2(sk_FragCoord.xy - float2(%s.zw));
    })SkSL",
                _sample7234.c_str(),
                rectFVar.isValid() ? args.fUniformHandler->getUniformCStr(rectFVar) : "float4(0)",
                rectFVar.isValid() ? args.fUniformHandler->getUniformCStr(rectFVar) : "float4(0)",
                rectHVar.isValid() ? args.fUniformHandler->getUniformCStr(rectHVar) : "half4(0)",
                rectHVar.isValid() ? args.fUniformHandler->getUniformCStr(rectHVar) : "half4(0)");
        SkString _coords8601("float2(half2(rect.x, 0.5))");
        SkString _sample8601 = this->invokeChild(1, args, _coords8601.c_str());
        SkString _coords8664("float2(half2(rect.z, 0.5))");
        SkString _sample8664 = this->invokeChild(1, args, _coords8664.c_str());
        fragBuilder->codeAppendf(
                R"SkSL(
    xCoverage = (1.0 - %s.w) - %s.w;)SkSL",
                _sample8601.c_str(), _sample8664.c_str());
        SkString _coords8728("float2(half2(rect.y, 0.5))");
        SkString _sample8728 = this->invokeChild(1, args, _coords8728.c_str());
        SkString _coords8791("float2(half2(rect.w, 0.5))");
        SkString _sample8791 = this->invokeChild(1, args, _coords8791.c_str());
        fragBuilder->codeAppendf(
                R"SkSL(
    yCoverage = (1.0 - %s.w) - %s.w;
})SkSL",
                _sample8728.c_str(), _sample8791.c_str());
        SkString _sample8860 = this->invokeChild(0, args);
        fragBuilder->codeAppendf(
                R"SkSL(
half4 inputColor = %s;
%s = (inputColor * xCoverage) * yCoverage;
)SkSL",
                _sample8860.c_str(), args.fOutputColor);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrRectBlurEffect& _outer = _proc.cast<GrRectBlurEffect>();
        auto rect = _outer.rect;
        (void)rect;
        UniformHandle& rectF = rectFVar;
        (void)rectF;
        UniformHandle& rectH = rectHVar;
        (void)rectH;
        auto isFast = _outer.isFast;
        (void)isFast;

        float r[]{rect.fLeft, rect.fTop, rect.fRight, rect.fBottom};
        pdman.set4fv(highp ? rectF : rectH, 1, r);
    }
    bool highp = false;
    UniformHandle rectFVar;
    UniformHandle rectHVar;
};
GrGLSLFragmentProcessor* GrRectBlurEffect::onCreateGLSLInstance() const {
    return new GrGLSLRectBlurEffect();
}
void GrRectBlurEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                             GrProcessorKeyBuilder* b) const {
    bool highp = ((abs(rect.left()) > 16000.0 || abs(rect.top()) > 16000.0) ||
                  abs(rect.right()) > 16000.0) ||
                 abs(rect.bottom()) > 16000.0;
    b->add32((uint32_t)highp);
    b->add32((uint32_t)isFast);
}
bool GrRectBlurEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrRectBlurEffect& that = other.cast<GrRectBlurEffect>();
    (void)that;
    if (rect != that.rect) return false;
    if (isFast != that.isFast) return false;
    return true;
}
GrRectBlurEffect::GrRectBlurEffect(const GrRectBlurEffect& src)
        : INHERITED(kGrRectBlurEffect_ClassID, src.optimizationFlags())
        , rect(src.rect)
        , isFast(src.isFast) {
    this->cloneAndRegisterAllChildProcessors(src);
}
std::unique_ptr<GrFragmentProcessor> GrRectBlurEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrRectBlurEffect(*this));
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrRectBlurEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrRectBlurEffect::TestCreate(GrProcessorTestData* data) {
    float sigma = data->fRandom->nextRangeF(3, 8);
    float width = data->fRandom->nextRangeF(200, 300);
    float height = data->fRandom->nextRangeF(200, 300);
    return GrRectBlurEffect::Make(data->inputFP(), data->context(), *data->caps()->shaderCaps(),
                                  SkRect::MakeWH(width, height), sigma);
}
#endif
