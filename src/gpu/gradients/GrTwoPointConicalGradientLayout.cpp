/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrTwoPointConicalGradientLayout.fp; do not modify.
 **************************************************************************************************/
#include "GrTwoPointConicalGradientLayout.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrTexture.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLTwoPointConicalGradientLayout : public GrGLSLFragmentProcessor {
public:
    GrGLSLTwoPointConicalGradientLayout() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrTwoPointConicalGradientLayout& _outer =
                args.fFp.cast<GrTwoPointConicalGradientLayout>();
        (void)_outer;
        auto gradientMatrix = _outer.gradientMatrix;
        (void)gradientMatrix;
        auto type = _outer.type;
        (void)type;
        auto isRadiusIncreasing = _outer.isRadiusIncreasing;
        (void)isRadiusIncreasing;
        auto isFocalOnCircle = _outer.isFocalOnCircle;
        (void)isFocalOnCircle;
        auto isWellBehaved = _outer.isWellBehaved;
        (void)isWellBehaved;
        auto isSwapped = _outer.isSwapped;
        (void)isSwapped;
        auto isNativelyFocal = _outer.isNativelyFocal;
        (void)isNativelyFocal;
        auto focalParams = _outer.focalParams;
        (void)focalParams;
        focalParamsVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                                          "focalParams");
        SkString sk_TransformedCoords2D_0 = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
        fragBuilder->codeAppendf(
                "float2 p = %s;\nfloat t = -1.0;\nhalf v = 1.0;\n@switch (%d) {\n    case 1:\n     "
                "   {\n            half r0_2 = %s.y;\n            t = float(r0_2) - p.y * p.y;\n   "
                "         if (t >= 0.0) {\n                t = p.x + sqrt(t);\n            } else "
                "{\n                v = -1.0;\n            }\n        }\n        break;\n    case "
                "0:\n        {\n            half r0 = %s.x;\n            @if (%s) {\n              "
                "  t = length(p) - float(r0);\n            } else {\n                t = "
                "-length(p) - float(r0);\n       ",
                sk_TransformedCoords2D_0.c_str(), (int)_outer.type,
                args.fUniformHandler->getUniformCStr(focalParamsVar),
                args.fUniformHandler->getUniformCStr(focalParamsVar),
                (_outer.isRadiusIncreasing ? "true" : "false"));
        fragBuilder->codeAppendf(
                "     }\n        }\n        break;\n    case 2:\n        {\n            half invR1 "
                "= %s.x;\n            half fx = %s.y;\n            float x_t = -1.0;\n            "
                "@if (%s) {\n                x_t = dot(p, p) / p.x;\n            } else if (%s) "
                "{\n                x_t = length(p) - p.x * float(invR1);\n            } else {\n  "
                "              float temp = p.x * p.x - p.y * p.y;\n                if (temp >= "
                "0.0) {\n                    @if (%s || !%s) {\n                        x_t = "
                "-sqrt(temp) - p.x * float(invR1)",
                args.fUniformHandler->getUniformCStr(focalParamsVar),
                args.fUniformHandler->getUniformCStr(focalParamsVar),
                (_outer.isFocalOnCircle ? "true" : "false"),
                (_outer.isWellBehaved ? "true" : "false"), (_outer.isSwapped ? "true" : "false"),
                (_outer.isRadiusIncreasing ? "true" : "false"));
        fragBuilder->codeAppendf(
                ";\n                    } else {\n                        x_t = sqrt(temp) - p.x * "
                "float(invR1);\n                    }\n                }\n            }\n          "
                "  @if (!%s) {\n                if (x_t <= 0.0) {\n                    v = -1.0;\n "
                "               }\n            }\n            @if (%s) {\n                @if (%s) "
                "{\n                    t = x_t;\n                } else {\n                    t "
                "= x_t + float(fx);\n                }\n            } else {\n                @if "
                "(%s) {\n              ",
                (_outer.isWellBehaved ? "true" : "false"),
                (_outer.isRadiusIncreasing ? "true" : "false"),
                (_outer.isNativelyFocal ? "true" : "false"),
                (_outer.isNativelyFocal ? "true" : "false"));
        fragBuilder->codeAppendf(
                "      t = -x_t;\n                } else {\n                    t = -x_t + "
                "float(fx);\n                }\n            }\n            @if (%s) {\n            "
                "    t = 1.0 - t;\n            }\n        }\n        break;\n}\n%s = "
                "half4(half(t), v, 0.0, 0.0);\n",
                (_outer.isSwapped ? "true" : "false"), args.fOutputColor);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrTwoPointConicalGradientLayout& _outer =
                _proc.cast<GrTwoPointConicalGradientLayout>();
        {
            const SkPoint& focalParamsValue = _outer.focalParams;
            if (focalParamsPrev != focalParamsValue) {
                focalParamsPrev = focalParamsValue;
                pdman.set2f(focalParamsVar, focalParamsValue.fX, focalParamsValue.fY);
            }
        }
    }
    SkPoint focalParamsPrev = SkPoint::Make(SK_FloatNaN, SK_FloatNaN);
    UniformHandle focalParamsVar;
};
GrGLSLFragmentProcessor* GrTwoPointConicalGradientLayout::onCreateGLSLInstance() const {
    return new GrGLSLTwoPointConicalGradientLayout();
}
void GrTwoPointConicalGradientLayout::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                            GrProcessorKeyBuilder* b) const {
    b->add32((int32_t)type);
    b->add32((int32_t)isRadiusIncreasing);
    b->add32((int32_t)isFocalOnCircle);
    b->add32((int32_t)isWellBehaved);
    b->add32((int32_t)isSwapped);
    b->add32((int32_t)isNativelyFocal);
}
bool GrTwoPointConicalGradientLayout::onIsEqual(const GrFragmentProcessor& other) const {
    const GrTwoPointConicalGradientLayout& that = other.cast<GrTwoPointConicalGradientLayout>();
    (void)that;
    if (gradientMatrix != that.gradientMatrix) return false;
    if (type != that.type) return false;
    if (isRadiusIncreasing != that.isRadiusIncreasing) return false;
    if (isFocalOnCircle != that.isFocalOnCircle) return false;
    if (isWellBehaved != that.isWellBehaved) return false;
    if (isSwapped != that.isSwapped) return false;
    if (isNativelyFocal != that.isNativelyFocal) return false;
    if (focalParams != that.focalParams) return false;
    return true;
}
GrTwoPointConicalGradientLayout::GrTwoPointConicalGradientLayout(
        const GrTwoPointConicalGradientLayout& src)
        : INHERITED(kGrTwoPointConicalGradientLayout_ClassID, src.optimizationFlags())
        , fCoordTransform0(src.fCoordTransform0)
        , gradientMatrix(src.gradientMatrix)
        , type(src.type)
        , isRadiusIncreasing(src.isRadiusIncreasing)
        , isFocalOnCircle(src.isFocalOnCircle)
        , isWellBehaved(src.isWellBehaved)
        , isSwapped(src.isSwapped)
        , isNativelyFocal(src.isNativelyFocal)
        , focalParams(src.focalParams) {
    this->addCoordTransform(&fCoordTransform0);
}
std::unique_ptr<GrFragmentProcessor> GrTwoPointConicalGradientLayout::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrTwoPointConicalGradientLayout(*this));
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrTwoPointConicalGradientLayout);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrTwoPointConicalGradientLayout::TestCreate(
        GrProcessorTestData* d) {
    SkScalar scale = GrGradientShader::RandomParams::kGradientScale;
    SkScalar offset = scale / 32.0f;

    SkPoint center1 = {d->fRandom->nextRangeScalar(0.0f, scale),
                       d->fRandom->nextRangeScalar(0.0f, scale)};
    SkPoint center2 = {d->fRandom->nextRangeScalar(0.0f, scale),
                       d->fRandom->nextRangeScalar(0.0f, scale)};
    SkScalar radius1 = d->fRandom->nextRangeScalar(0.0f, scale);
    SkScalar radius2 = d->fRandom->nextRangeScalar(0.0f, scale);

    constexpr int kTestTypeMask = (1 << 2) - 1, kTestNativelyFocalBit = (1 << 2),
                  kTestFocalOnCircleBit = (1 << 3), kTestSwappedBit = (1 << 4);
    // We won't treat isWellDefined and isRadiusIncreasing specially because they
    // should have high probability to be turned on and off as we're getting random
    // radii and centers.

    int mask = d->fRandom->nextU();
    int type = mask & kTestTypeMask;
    if (type == static_cast<int>(Type::kRadial)) {
        center2 = center1;
        // Make sure that the radii are different
        if (SkScalarNearlyZero(radius1 - radius2)) {
            radius2 += offset;
        }
    } else if (type == static_cast<int>(Type::kStrip)) {
        radius1 = SkTMax(radius1, .1f);  // Make sure that the radius is non-zero
        radius2 = radius1;
        // Make sure that the centers are different
        if (SkScalarNearlyZero(SkPoint::Distance(center1, center2))) {
            center2.fX += offset;
        }
    } else {  // kFocal_Type
        // Make sure that the centers are different
        if (SkScalarNearlyZero(SkPoint::Distance(center1, center2))) {
            center2.fX += offset;
        }

        if (kTestNativelyFocalBit & mask) {
            radius1 = 0;
        }
        if (kTestFocalOnCircleBit & mask) {
            radius2 = radius1 + SkPoint::Distance(center1, center2);
        }
        if (kTestSwappedBit & mask) {
            std::swap(radius1, radius2);
            radius2 = 0;
        }

        // Make sure that the radii are different
        if (SkScalarNearlyZero(radius1 - radius2)) {
            radius2 += offset;
        }
    }

    if (SkScalarNearlyZero(radius1 - radius2) &&
        SkScalarNearlyZero(SkPoint::Distance(center1, center2))) {
        radius2 += offset;  // make sure that we're not degenerated
    }

    GrGradientShader::RandomParams params(d->fRandom);
    auto shader = params.fUseColors4f
                          ? SkGradientShader::MakeTwoPointConical(
                                    center1, radius1, center2, radius2, params.fColors4f,
                                    params.fColorSpace, params.fStops, params.fColorCount,
                                    params.fTileMode)
                          : SkGradientShader::MakeTwoPointConical(
                                    center1, radius1, center2, radius2, params.fColors,
                                    params.fStops, params.fColorCount, params.fTileMode);
    GrTest::TestAsFPArgs asFPArgs(d);
    std::unique_ptr<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());

    GrAlwaysAssert(fp);
    return fp;
}
#endif

// .fp files do not let you reference outside enum definitions, so we have to explicitly map
// between the two compatible enum defs
GrTwoPointConicalGradientLayout::Type convert_type(SkTwoPointConicalGradient::Type type) {
    switch (type) {
        case SkTwoPointConicalGradient::Type::kRadial:
            return GrTwoPointConicalGradientLayout::Type::kRadial;
        case SkTwoPointConicalGradient::Type::kStrip:
            return GrTwoPointConicalGradientLayout::Type::kStrip;
        case SkTwoPointConicalGradient::Type::kFocal:
            return GrTwoPointConicalGradientLayout::Type::kFocal;
    }
    SkDEBUGFAIL("Should not be reachable");
    return GrTwoPointConicalGradientLayout::Type::kRadial;
}

std::unique_ptr<GrFragmentProcessor> GrTwoPointConicalGradientLayout::Make(
        const SkTwoPointConicalGradient& grad, const GrFPArgs& args) {
    GrTwoPointConicalGradientLayout::Type grType = convert_type(grad.getType());

    // The focalData struct is only valid if isFocal is true
    const SkTwoPointConicalGradient::FocalData& focalData = grad.getFocalData();
    bool isFocal = grType == Type::kFocal;

    // Calculate optimization switches from gradient specification
    bool isFocalOnCircle = isFocal && focalData.isFocalOnCircle();
    bool isWellBehaved = isFocal && focalData.isWellBehaved();
    bool isSwapped = isFocal && focalData.isSwapped();
    bool isNativelyFocal = isFocal && focalData.isNativelyFocal();

    // Type-specific calculations: isRadiusIncreasing, focalParams, and the gradient matrix.
    // However, all types start with the total inverse local matrix calculated from the shader
    // and args
    bool isRadiusIncreasing;
    SkPoint focalParams;  // really just a 2D tuple
    SkMatrix matrix;

    // Initialize the base matrix
    if (!grad.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }

    if (isFocal) {
        isRadiusIncreasing = (1 - focalData.fFocalX) > 0;

        focalParams.set(1.0 / focalData.fR1, focalData.fFocalX);

        matrix.postConcat(grad.getGradientMatrix());
    } else if (grType == Type::kRadial) {
        SkScalar dr = grad.getDiffRadius();
        isRadiusIncreasing = dr >= 0;

        SkScalar r0 = grad.getStartRadius() / dr;
        focalParams.set(r0, r0 * r0);

        // GPU radial matrix is different from the original matrix, since we map the diff radius
        // to have |dr| = 1, so manually compute the final gradient matrix here.

        // Map center to (0, 0)
        matrix.postTranslate(-grad.getStartCenter().fX, -grad.getStartCenter().fY);

        // scale |diffRadius| to 1
        matrix.postScale(1 / dr, 1 / dr);
    } else {                         // kStrip
        isRadiusIncreasing = false;  // kStrip doesn't use this flag

        SkScalar r0 = grad.getStartRadius() / grad.getCenterX1();
        focalParams.set(r0, r0 * r0);

        matrix.postConcat(grad.getGradientMatrix());
    }

    return std::unique_ptr<GrFragmentProcessor>(new GrTwoPointConicalGradientLayout(
            matrix, grType, isRadiusIncreasing, isFocalOnCircle, isWellBehaved, isSwapped,
            isNativelyFocal, focalParams));
}
