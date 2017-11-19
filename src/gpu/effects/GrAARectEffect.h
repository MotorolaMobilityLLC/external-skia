/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This file was autogenerated from GrAARectEffect.fp; do not modify.
 */
#ifndef GrAARectEffect_DEFINED
#define GrAARectEffect_DEFINED
#include "SkTypes.h"
#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
class GrAARectEffect : public GrFragmentProcessor {
public:
    GrClipEdgeType edgeType() const { return fEdgeType; }
    SkRect rect() const { return fRect; }
    static std::unique_ptr<GrFragmentProcessor> Make(GrClipEdgeType edgeType, SkRect rect) {
        return std::unique_ptr<GrFragmentProcessor>(new GrAARectEffect(edgeType, rect));
    }
    GrAARectEffect(const GrAARectEffect& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "AARectEffect"; }

private:
    GrAARectEffect(GrClipEdgeType edgeType, SkRect rect)
            : INHERITED(kGrAARectEffect_ClassID,
                        (OptimizationFlags)kCompatibleWithCoverageAsAlpha_OptimizationFlag)
            , fEdgeType(edgeType)
            , fRect(rect) {}
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    GrClipEdgeType fEdgeType;
    SkRect fRect;
    typedef GrFragmentProcessor INHERITED;
};
#endif
#endif
