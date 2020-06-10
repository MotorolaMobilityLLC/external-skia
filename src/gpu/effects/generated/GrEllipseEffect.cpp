/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrEllipseEffect.fp; do not modify.
 **************************************************************************************************/
#include "GrEllipseEffect.h"

#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLEllipseEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLEllipseEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrEllipseEffect& _outer = args.fFp.cast<GrEllipseEffect>();
        (void)_outer;
        auto edgeType = _outer.edgeType;
        (void)edgeType;
        auto center = _outer.center;
        (void)center;
        auto radii = _outer.radii;
        (void)radii;
        prevRadii = float2(-1.0);
        medPrecision = !sk_Caps.floatIs32Bits;
        ellipseVar = args.fUniformHandler->addUniform(&_outer, kFragment_GrShaderFlag,
                                                      kFloat4_GrSLType, "ellipse");
        if (medPrecision) {
            scaleVar = args.fUniformHandler->addUniform(&_outer, kFragment_GrShaderFlag,
                                                        kFloat2_GrSLType, "scale");
        }
        fragBuilder->codeAppendf(
                "float2 prevCenter;\nfloat2 prevRadii = float2(%f, %f);\nbool medPrecision = "
                "%s;\nfloat2 d = sk_FragCoord.xy - %s.xy;\n@if (medPrecision) {\n    d *= "
                "%s.y;\n}\nfloat2 Z = d * %s.zw;\nfloat implicit = dot(Z, d) - 1.0;\nfloat "
                "grad_dot = 4.0 * dot(Z, Z);\n@if (medPrecision) {\n    grad_dot = max(grad_dot, "
                "6.1036000261083245e-05);\n} else {\n    grad_dot = max(grad_dot, "
                "1.1754999560161448e-38);\n}\nfloat approx_dist = implicit * "
                "inversesqrt(grad_dot);\n@if (medPrecision) {\n    approx_dist *= %s.x;\n}\nhalf "
                "alph",
                prevRadii.fX, prevRadii.fY, (medPrecision ? "true" : "false"),
                args.fUniformHandler->getUniformCStr(ellipseVar),
                scaleVar.isValid() ? args.fUniformHandler->getUniformCStr(scaleVar) : "float2(0)",
                args.fUniformHandler->getUniformCStr(ellipseVar),
                scaleVar.isValid() ? args.fUniformHandler->getUniformCStr(scaleVar) : "float2(0)");
        fragBuilder->codeAppendf(
                "a;\n@switch (%d) {\n    case 0:\n        alpha = approx_dist > 0.0 ? 0.0 : 1.0;\n "
                "       break;\n    case 1:\n        alpha = clamp(0.5 - half(approx_dist), 0.0, "
                "1.0);\n        break;\n    case 2:\n        alpha = approx_dist > 0.0 ? 1.0 : "
                "0.0;\n        break;\n    case 3:\n        alpha = clamp(0.5 + half(approx_dist), "
                "0.0, 1.0);\n        break;\n    default:\n        discard;\n}",
                (int)_outer.edgeType);
        SkString _input4497 = SkStringPrintf("%s", args.fInputColor);
        SkString _sample4497;
        if (_outer.inputFP_index >= 0) {
            _sample4497 = this->invokeChild(_outer.inputFP_index, _input4497.c_str(), args);
        } else {
            _sample4497 = _input4497;
        }
        fragBuilder->codeAppendf("\nhalf4 inputColor = %s;\n%s = inputColor * alpha;\n",
                                 _sample4497.c_str(), args.fOutputColor);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrEllipseEffect& _outer = _proc.cast<GrEllipseEffect>();
        auto edgeType = _outer.edgeType;
        (void)edgeType;
        auto center = _outer.center;
        (void)center;
        auto radii = _outer.radii;
        (void)radii;
        UniformHandle& ellipse = ellipseVar;
        (void)ellipse;
        UniformHandle& scale = scaleVar;
        (void)scale;

        if (radii != prevRadii || center != prevCenter) {
            float invRXSqd;
            float invRYSqd;
            // If we're using a scale factor to work around precision issues, choose the larger
            // radius as the scale factor. The inv radii need to be pre-adjusted by the scale
            // factor.
            if (scale.isValid()) {
                if (radii.fX > radii.fY) {
                    invRXSqd = 1.f;
                    invRYSqd = (radii.fX * radii.fX) / (radii.fY * radii.fY);
                    pdman.set2f(scale, radii.fX, 1.f / radii.fX);
                } else {
                    invRXSqd = (radii.fY * radii.fY) / (radii.fX * radii.fX);
                    invRYSqd = 1.f;
                    pdman.set2f(scale, radii.fY, 1.f / radii.fY);
                }
            } else {
                invRXSqd = 1.f / (radii.fX * radii.fX);
                invRYSqd = 1.f / (radii.fY * radii.fY);
            }
            pdman.set4f(ellipse, center.fX, center.fY, invRXSqd, invRYSqd);
            prevCenter = center;
            prevRadii = radii;
        }
    }
    SkPoint prevCenter = float2(0);
    SkPoint prevRadii = float2(0);
    bool medPrecision = false;
    UniformHandle ellipseVar;
    UniformHandle scaleVar;
};
GrGLSLFragmentProcessor* GrEllipseEffect::onCreateGLSLInstance() const {
    return new GrGLSLEllipseEffect();
}
void GrEllipseEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    b->add32((int32_t)edgeType);
}
bool GrEllipseEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrEllipseEffect& that = other.cast<GrEllipseEffect>();
    (void)that;
    if (edgeType != that.edgeType) return false;
    if (center != that.center) return false;
    if (radii != that.radii) return false;
    return true;
}
GrEllipseEffect::GrEllipseEffect(const GrEllipseEffect& src)
        : INHERITED(kGrEllipseEffect_ClassID, src.optimizationFlags())
        , inputFP_index(src.inputFP_index)
        , edgeType(src.edgeType)
        , center(src.center)
        , radii(src.radii) {
    if (inputFP_index >= 0) {
        auto clone = src.childProcessor(inputFP_index).clone();
        if (src.childProcessor(inputFP_index).isSampledWithExplicitCoords()) {
            clone->setSampledWithExplicitCoords();
        }
        this->registerChildProcessor(std::move(clone));
    }
}
std::unique_ptr<GrFragmentProcessor> GrEllipseEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrEllipseEffect(*this));
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrEllipseEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrEllipseEffect::TestCreate(GrProcessorTestData* testData) {
    SkPoint center;
    center.fX = testData->fRandom->nextRangeScalar(0.f, 1000.f);
    center.fY = testData->fRandom->nextRangeScalar(0.f, 1000.f);
    SkScalar rx = testData->fRandom->nextRangeF(0.f, 1000.f);
    SkScalar ry = testData->fRandom->nextRangeF(0.f, 1000.f);
    GrClipEdgeType et;
    do {
        et = (GrClipEdgeType)testData->fRandom->nextULessThan(kGrClipEdgeTypeCnt);
    } while (GrClipEdgeType::kHairlineAA == et);
    return GrEllipseEffect::Make(/*inputFP=*/nullptr, et, center, SkPoint::Make(rx, ry),
                                 *testData->caps()->shaderCaps());
}
#endif
