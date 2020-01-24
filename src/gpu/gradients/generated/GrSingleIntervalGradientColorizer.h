/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrSingleIntervalGradientColorizer.fp; do not modify.
 **************************************************************************************************/
#ifndef GrSingleIntervalGradientColorizer_DEFINED
#define GrSingleIntervalGradientColorizer_DEFINED
#include "include/core/SkTypes.h"
#include "include/private/SkM44.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"
class GrSingleIntervalGradientColorizer : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(SkPMColor4f start, SkPMColor4f end) {
        return std::unique_ptr<GrFragmentProcessor>(
                new GrSingleIntervalGradientColorizer(start, end));
    }
    GrSingleIntervalGradientColorizer(const GrSingleIntervalGradientColorizer& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "SingleIntervalGradientColorizer"; }
    SkPMColor4f start;
    SkPMColor4f end;

private:
    GrSingleIntervalGradientColorizer(SkPMColor4f start, SkPMColor4f end)
            : INHERITED(kGrSingleIntervalGradientColorizer_ClassID, kNone_OptimizationFlags)
            , start(start)
            , end(end) {}
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    typedef GrFragmentProcessor INHERITED;
};
#endif
