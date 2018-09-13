/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrSweepGradientLayout.fp; do not modify.
 **************************************************************************************************/
#include "GrSweepGradientLayout.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrTexture.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLSweepGradientLayout : public GrGLSLFragmentProcessor {
public:
    GrGLSLSweepGradientLayout() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrSweepGradientLayout& _outer = args.fFp.cast<GrSweepGradientLayout>();
        (void)_outer;
        auto gradientMatrix = _outer.gradientMatrix();
        (void)gradientMatrix;
        auto bias = _outer.bias();
        (void)bias;
        auto scale = _outer.scale();
        (void)scale;
        fBiasVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                    kDefault_GrSLPrecision, "bias");
        fScaleVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                     kDefault_GrSLPrecision, "scale");
        SkString sk_TransformedCoords2D_0 = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
        fragBuilder->codeAppendf(
                "half angle;\nif (sk_Caps.atan2ImplementedAsAtanYOverX) {\n    angle = half(2.0 * "
                "atan(-%s.y, length(%s) - %s.x));\n} else {\n    angle = half(atan(-%s.y, "
                "-%s.x));\n}\n%s = half4(((float(float(angle) * 0.15915494309180001) + 0.5) + %s) "
                "* %s);\n",
                sk_TransformedCoords2D_0.c_str(), sk_TransformedCoords2D_0.c_str(),
                sk_TransformedCoords2D_0.c_str(), sk_TransformedCoords2D_0.c_str(),
                sk_TransformedCoords2D_0.c_str(), args.fOutputColor,
                args.fUniformHandler->getUniformCStr(fBiasVar),
                args.fUniformHandler->getUniformCStr(fScaleVar));
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrSweepGradientLayout& _outer = _proc.cast<GrSweepGradientLayout>();
        {
            float biasValue = _outer.bias();
            if (fBiasPrev != biasValue) {
                fBiasPrev = biasValue;
                pdman.set1f(fBiasVar, biasValue);
            }
            float scaleValue = _outer.scale();
            if (fScalePrev != scaleValue) {
                fScalePrev = scaleValue;
                pdman.set1f(fScaleVar, scaleValue);
            }
        }
    }
    float fBiasPrev = SK_FloatNaN;
    float fScalePrev = SK_FloatNaN;
    UniformHandle fBiasVar;
    UniformHandle fScaleVar;
};
GrGLSLFragmentProcessor* GrSweepGradientLayout::onCreateGLSLInstance() const {
    return new GrGLSLSweepGradientLayout();
}
void GrSweepGradientLayout::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {}
bool GrSweepGradientLayout::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSweepGradientLayout& that = other.cast<GrSweepGradientLayout>();
    (void)that;
    if (fGradientMatrix != that.fGradientMatrix) return false;
    if (fBias != that.fBias) return false;
    if (fScale != that.fScale) return false;
    return true;
}
GrSweepGradientLayout::GrSweepGradientLayout(const GrSweepGradientLayout& src)
        : INHERITED(kGrSweepGradientLayout_ClassID, src.optimizationFlags())
        , fGradientMatrix(src.fGradientMatrix)
        , fBias(src.fBias)
        , fScale(src.fScale)
        , fCoordTransform0(src.fCoordTransform0) {
    this->addCoordTransform(&fCoordTransform0);
}
std::unique_ptr<GrFragmentProcessor> GrSweepGradientLayout::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrSweepGradientLayout(*this));
}

std::unique_ptr<GrFragmentProcessor> GrSweepGradientLayout::Make(const SkSweepGradient& grad,
                                                                 const GrFPArgs& args) {
    SkMatrix matrix;
    if (!grad.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    matrix.postConcat(grad.getGradientMatrix());
    return std::unique_ptr<GrFragmentProcessor>(
            new GrSweepGradientLayout(matrix, grad.getTBias(), grad.getTScale()));
}
