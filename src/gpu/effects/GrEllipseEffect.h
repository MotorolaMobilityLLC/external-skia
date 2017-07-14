/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This file was autogenerated from GrEllipseEffect.fp; do not modify.
 */
#ifndef GrEllipseEffect_DEFINED
#define GrEllipseEffect_DEFINED
#include "SkTypes.h"
#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
#include "GrColorSpaceXform.h"
#include "effects/GrProxyMove.h"
class GrEllipseEffect : public GrFragmentProcessor {
public:
    int edgeType() const { return fEdgeType; }
    SkPoint center() const { return fCenter; }
    SkPoint radii() const { return fRadii; }
    static sk_sp<GrFragmentProcessor> Make(int edgeType, SkPoint center, SkPoint radii) {
        return sk_sp<GrFragmentProcessor>(new GrEllipseEffect(edgeType, center, radii));
    }
    const char* name() const override { return "EllipseEffect"; }
private:
    GrEllipseEffect(int edgeType, SkPoint center, SkPoint radii)
    : INHERITED((OptimizationFlags)  kCompatibleWithCoverageAsAlpha_OptimizationFlag )
    , fEdgeType(edgeType)
    , fCenter(center)
    , fRadii(radii) {
        this->initClassID<GrEllipseEffect>();
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&,GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    int fEdgeType;
    SkPoint fCenter;
    SkPoint fRadii;
    typedef GrFragmentProcessor INHERITED;
};
#endif
#endif
