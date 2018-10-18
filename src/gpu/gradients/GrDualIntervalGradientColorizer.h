/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrDualIntervalGradientColorizer.fp; do not modify.
 **************************************************************************************************/
#ifndef GrDualIntervalGradientColorizer_DEFINED
#define GrDualIntervalGradientColorizer_DEFINED
#include "SkTypes.h"
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
class GrDualIntervalGradientColorizer : public GrFragmentProcessor {
public:
    const SkPMColor4f& scale01() const { return fScale01; }
    const SkPMColor4f& bias01() const { return fBias01; }
    const SkPMColor4f& scale23() const { return fScale23; }
    const SkPMColor4f& bias23() const { return fBias23; }
    float threshold() const { return fThreshold; }

    static std::unique_ptr<GrFragmentProcessor> Make(const SkPMColor4f& c0, const SkPMColor4f& c1,
                                                     const SkPMColor4f& c2, const SkPMColor4f& c3,
                                                     float threshold);
    GrDualIntervalGradientColorizer(const GrDualIntervalGradientColorizer& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "DualIntervalGradientColorizer"; }

private:
    GrDualIntervalGradientColorizer(SkPMColor4f scale01, SkPMColor4f bias01, SkPMColor4f scale23,
                                    SkPMColor4f bias23, float threshold)
            : INHERITED(kGrDualIntervalGradientColorizer_ClassID, kNone_OptimizationFlags)
            , fScale01(scale01)
            , fBias01(bias01)
            , fScale23(scale23)
            , fBias23(bias23)
            , fThreshold(threshold) {}
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    SkPMColor4f fScale01;
    SkPMColor4f fBias01;
    SkPMColor4f fScale23;
    SkPMColor4f fBias23;
    float fThreshold;
    typedef GrFragmentProcessor INHERITED;
};
#endif
