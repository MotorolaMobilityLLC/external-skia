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
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrTexture.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLRectBlurEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLRectBlurEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrRectBlurEffect& _outer = args.fFp.cast<GrRectBlurEffect>();
        (void)_outer;
        auto rect = _outer.rect();
        (void)rect;
        auto sigma = _outer.sigma();
        (void)sigma;
        highPrecision = ((((abs(rect.left()) > 16000.0 || abs(rect.top()) > 16000.0) ||
                           abs(rect.right()) > 16000.0) ||
                          abs(rect.bottom()) > 16000.0) ||
                         abs(rect.right() - rect.left()) > 16000.0) ||
                        abs(rect.bottom() - rect.top()) > 16000.0;
        fRectVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kFloat4_GrSLType,
                                                    kDefault_GrSLPrecision, "rect");
        if (!highPrecision) {
            fProxyRectHalfVar =
                    args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType,
                                                     kDefault_GrSLPrecision, "proxyRectHalf");
        }
        if (highPrecision) {
            fProxyRectFloatVar =
                    args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kFloat4_GrSLType,
                                                     kDefault_GrSLPrecision, "proxyRectFloat");
        }
        fProfileSizeVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                           kDefault_GrSLPrecision, "profileSize");
        fragBuilder->codeAppendf(
                "bool highPrecision = %s;\n@if (highPrecision) {\n    float2 translatedPos = "
                "sk_FragCoord.xy - %s.xy;\n    float width = %s.z - %s.x;\n    float height = %s.w "
                "- %s.y;\n    float2 smallDims = float2(width - float(%s), height - float(%s));\n  "
                "  float center = 2.0 * floor(float(float(%s / 2.0) + 0.25)) - 1.0;\n    float2 wh "
                "= smallDims - float2(center, center);\n    half hcoord = "
                "half((abs(translatedPos.x - 0.5 * width) - 0.5 * wh.x) / float(%s));\n    half "
                "hlookup = texture(%s, float2(float(hcoord), 0.5)).%s.w",
                (highPrecision ? "true" : "false"), args.fUniformHandler->getUniformCStr(fRectVar),
                args.fUniformHandler->getUniformCStr(fRectVar),
                args.fUniformHandler->getUniformCStr(fRectVar),
                args.fUniformHandler->getUniformCStr(fRectVar),
                args.fUniformHandler->getUniformCStr(fRectVar),
                args.fUniformHandler->getUniformCStr(fProfileSizeVar),
                args.fUniformHandler->getUniformCStr(fProfileSizeVar),
                args.fUniformHandler->getUniformCStr(fProfileSizeVar),
                args.fUniformHandler->getUniformCStr(fProfileSizeVar),
                fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]).c_str(),
                fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str());
        fragBuilder->codeAppendf(
                ";\n    half vcoord = half((abs(translatedPos.y - 0.5 * height) - 0.5 * wh.y) / "
                "float(%s));\n    half vlookup = texture(%s, float2(float(vcoord), 0.5)).%s.w;\n   "
                " %s = (%s * hlookup) * vlookup;\n} else {\n    half2 translatedPos = "
                "half2(sk_FragCoord.xy - %s.xy);\n    half width = half(%s.z - %s.x);\n    half "
                "height = half(%s.w - %s.y);\n    half2 smallDims = half2(width - %s, height - "
                "%s);\n    half center = half(2.0 * floor(float(float(%s / 2.0) + 0.25)) - 1.0);\n "
                "   half2 wh = smallDims - half2(float2(floa",
                args.fUniformHandler->getUniformCStr(fProfileSizeVar),
                fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]).c_str(),
                fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str(),
                args.fOutputColor, args.fInputColor ? args.fInputColor : "half4(1)",
                args.fUniformHandler->getUniformCStr(fRectVar),
                args.fUniformHandler->getUniformCStr(fRectVar),
                args.fUniformHandler->getUniformCStr(fRectVar),
                args.fUniformHandler->getUniformCStr(fRectVar),
                args.fUniformHandler->getUniformCStr(fRectVar),
                args.fUniformHandler->getUniformCStr(fProfileSizeVar),
                args.fUniformHandler->getUniformCStr(fProfileSizeVar),
                args.fUniformHandler->getUniformCStr(fProfileSizeVar));
        fragBuilder->codeAppendf(
                "t(center), float(center)));\n    half hcoord = "
                "half((abs(float(float(translatedPos.x) - 0.5 * float(width))) - 0.5 * "
                "float(wh.x)) / float(%s));\n    half hlookup = texture(%s, float2(float(hcoord), "
                "0.5)).%s.w;\n    half vcoord = half((abs(float(float(translatedPos.y) - 0.5 * "
                "float(height))) - 0.5 * float(wh.y)) / float(%s));\n    half vlookup = "
                "texture(%s, float2(float(vcoord), 0.5)).%s.w;\n    %s = (%s * hlookup) * "
                "vlookup;\n}\n",
                args.fUniformHandler->getUniformCStr(fProfileSizeVar),
                fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]).c_str(),
                fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str(),
                args.fUniformHandler->getUniformCStr(fProfileSizeVar),
                fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]).c_str(),
                fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str(),
                args.fOutputColor, args.fInputColor ? args.fInputColor : "half4(1)");
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrRectBlurEffect& _outer = _proc.cast<GrRectBlurEffect>();
        {
            const SkRect rectValue = _outer.rect();
            pdman.set4fv(fRectVar, 1, (float*)&rectValue);
        }
        UniformHandle& rect = fRectVar;
        (void)rect;
        auto sigma = _outer.sigma();
        (void)sigma;
        GrSurfaceProxy& blurProfileProxy = *_outer.textureSampler(0).proxy();
        GrTexture& blurProfile = *blurProfileProxy.priv().peekTexture();
        (void)blurProfile;
        UniformHandle& proxyRectHalf = fProxyRectHalfVar;
        (void)proxyRectHalf;
        UniformHandle& proxyRectFloat = fProxyRectFloatVar;
        (void)proxyRectFloat;
        UniformHandle& profileSize = fProfileSizeVar;
        (void)profileSize;

        pdman.set1f(profileSize, SkScalarCeilToScalar(6 * sigma));
    }
    bool highPrecision = false;
    UniformHandle fProxyRectHalfVar;
    UniformHandle fProxyRectFloatVar;
    UniformHandle fProfileSizeVar;
    UniformHandle fRectVar;
};
GrGLSLFragmentProcessor* GrRectBlurEffect::onCreateGLSLInstance() const {
    return new GrGLSLRectBlurEffect();
}
void GrRectBlurEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                             GrProcessorKeyBuilder* b) const {}
bool GrRectBlurEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrRectBlurEffect& that = other.cast<GrRectBlurEffect>();
    (void)that;
    if (fRect != that.fRect) return false;
    if (fSigma != that.fSigma) return false;
    if (fBlurProfile != that.fBlurProfile) return false;
    return true;
}
GrRectBlurEffect::GrRectBlurEffect(const GrRectBlurEffect& src)
        : INHERITED(kGrRectBlurEffect_ClassID, src.optimizationFlags())
        , fRect(src.fRect)
        , fSigma(src.fSigma)
        , fBlurProfile(src.fBlurProfile) {
    this->addTextureSampler(&fBlurProfile);
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
    return GrRectBlurEffect::Make(data->proxyProvider(), SkRect::MakeWH(width, height), sigma);
}
#endif
