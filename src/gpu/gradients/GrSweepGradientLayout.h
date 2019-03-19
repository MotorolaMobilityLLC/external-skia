/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrSweepGradientLayout.fp; do not modify.
 **************************************************************************************************/
#ifndef GrSweepGradientLayout_DEFINED
#define GrSweepGradientLayout_DEFINED
#include "SkTypes.h"

#include "SkSweepGradient.h"
#include "GrGradientShader.h"
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
class GrSweepGradientLayout : public GrFragmentProcessor {
public:
    const SkMatrix44& gradientMatrix() const { return fGradientMatrix; }
    float             bias() const { return fBias; }
    float             scale() const { return fScale; }

    static std::unique_ptr<GrFragmentProcessor> Make(const SkSweepGradient& gradient,
                                                     const GrFPArgs&        args);
    GrSweepGradientLayout(const GrSweepGradientLayout& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char*                          name() const override { return "SweepGradientLayout"; }

private:
    GrSweepGradientLayout(SkMatrix44 gradientMatrix, float bias, float scale)
            : INHERITED(kGrSweepGradientLayout_ClassID,
                        (OptimizationFlags)kPreservesOpaqueInput_OptimizationFlag)
            , fGradientMatrix(gradientMatrix)
            , fBias(bias)
            , fScale(scale)
            , fCoordTransform0(gradientMatrix) {
        this->addCoordTransform(&fCoordTransform0);
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    SkMatrix44                  fGradientMatrix;
    float                       fBias;
    float                       fScale;
    GrCoordTransform            fCoordTransform0;
    typedef GrFragmentProcessor INHERITED;
};
#endif
