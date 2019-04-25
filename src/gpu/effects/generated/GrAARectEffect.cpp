/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrAARectEffect.fp; do not modify.
 **************************************************************************************************/
#include "GrAARectEffect.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLAARectEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLAARectEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrAARectEffect& _outer = args.fFp.cast<GrAARectEffect>();
        (void)_outer;
        auto edgeType = _outer.edgeType;
        (void)edgeType;
        auto rect = _outer.rect;
        (void)rect;
        prevRect = float4(-1.0);
        rectUniformVar = args.fUniformHandler->addUniform(
                kFragment_GrShaderFlag, kFloat4_GrSLType, "rectUniform");
        fragBuilder->codeAppendf(
                "float4 prevRect = float4(%f, %f, %f, %f);\nhalf alpha;\n@switch (%d) {\n    case "
                "0:\n    case 2:\n        alpha = half(all(greaterThan(float4(sk_FragCoord.xy, "
                "%s.zw), float4(%s.xy, sk_FragCoord.xy))) ? 1 : 0);\n        break;\n    "
                "default:\n        half xSub, ySub;\n        xSub = min(half(sk_FragCoord.x - "
                "%s.x), 0.0);\n        xSub += min(half(%s.z - sk_FragCoord.x), 0.0);\n        "
                "ySub = min(half(sk_FragCoord.y - %s.y), 0.0);\n        ySub += min(half(%s.w - "
                "sk_FragCoord.y), 0.0);\n        alpha = (1.0 + ",
                prevRect.left(),
                prevRect.top(),
                prevRect.right(),
                prevRect.bottom(),
                (int)_outer.edgeType,
                args.fUniformHandler->getUniformCStr(rectUniformVar),
                args.fUniformHandler->getUniformCStr(rectUniformVar),
                args.fUniformHandler->getUniformCStr(rectUniformVar),
                args.fUniformHandler->getUniformCStr(rectUniformVar),
                args.fUniformHandler->getUniformCStr(rectUniformVar),
                args.fUniformHandler->getUniformCStr(rectUniformVar));
        fragBuilder->codeAppendf(
                "max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));\n}\n@if (%d == 2 || %d == 3) {\n    "
                "alpha = 1.0 - alpha;\n}\n%s = %s * alpha;\n",
                (int)_outer.edgeType,
                (int)_outer.edgeType,
                args.fOutputColor,
                args.fInputColor);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrAARectEffect& _outer = _proc.cast<GrAARectEffect>();
        auto edgeType = _outer.edgeType;
        (void)edgeType;
        auto rect = _outer.rect;
        (void)rect;
        UniformHandle& rectUniform = rectUniformVar;
        (void)rectUniform;

        const SkRect& newRect = GrProcessorEdgeTypeIsAA(edgeType) ? rect.makeInset(.5f, .5f) : rect;
        if (newRect != prevRect) {
            pdman.set4f(rectUniform, newRect.fLeft, newRect.fTop, newRect.fRight, newRect.fBottom);
            prevRect = newRect;
        }
    }
    SkRect prevRect = float4(0);
    UniformHandle rectUniformVar;
};
GrGLSLFragmentProcessor* GrAARectEffect::onCreateGLSLInstance() const {
    return new GrGLSLAARectEffect();
}
void GrAARectEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                           GrProcessorKeyBuilder* b) const {
    b->add32((int32_t)edgeType);
}
bool GrAARectEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrAARectEffect& that = other.cast<GrAARectEffect>();
    (void)that;
    if (edgeType != that.edgeType) return false;
    if (rect != that.rect) return false;
    return true;
}
GrAARectEffect::GrAARectEffect(const GrAARectEffect& src)
        : INHERITED(kGrAARectEffect_ClassID, src.optimizationFlags())
        , edgeType(src.edgeType)
        , rect(src.rect) {}
std::unique_ptr<GrFragmentProcessor> GrAARectEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrAARectEffect(*this));
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrAARectEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrAARectEffect::TestCreate(GrProcessorTestData* d) {
    SkRect rect = SkRect::MakeLTRB(d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1());
    std::unique_ptr<GrFragmentProcessor> fp;
    do {
        GrClipEdgeType edgeType =
                static_cast<GrClipEdgeType>(d->fRandom->nextULessThan(kGrClipEdgeTypeCnt));

        fp = GrAARectEffect::Make(edgeType, rect);
    } while (nullptr == fp);
    return fp;
}
#endif
