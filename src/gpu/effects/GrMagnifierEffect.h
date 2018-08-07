/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrMagnifierEffect.fp; do not modify.
 **************************************************************************************************/
#ifndef GrMagnifierEffect_DEFINED
#define GrMagnifierEffect_DEFINED
#include "SkTypes.h"
#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
class GrMagnifierEffect : public GrFragmentProcessor {
public:
    SkIRect bounds() const { return fBounds; }
    SkRect srcRect() const { return fSrcRect; }
    float xInvZoom() const { return fXInvZoom; }
    float yInvZoom() const { return fYInvZoom; }
    float xInvInset() const { return fXInvInset; }
    float yInvInset() const { return fYInvInset; }
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> src, SkIRect bounds,
                                                     SkRect srcRect, float xInvZoom, float yInvZoom,
                                                     float xInvInset, float yInvInset) {
        return std::unique_ptr<GrFragmentProcessor>(new GrMagnifierEffect(
                src, bounds, srcRect, xInvZoom, yInvZoom, xInvInset, yInvInset));
    }
    GrMagnifierEffect(const GrMagnifierEffect& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "MagnifierEffect"; }

private:
    GrMagnifierEffect(sk_sp<GrTextureProxy> src, SkIRect bounds, SkRect srcRect, float xInvZoom,
                      float yInvZoom, float xInvInset, float yInvInset)
            : INHERITED(kGrMagnifierEffect_ClassID, kNone_OptimizationFlags)
            , fSrc(std::move(src))
            , fBounds(bounds)
            , fSrcRect(srcRect)
            , fXInvZoom(xInvZoom)
            , fYInvZoom(yInvZoom)
            , fXInvInset(xInvInset)
            , fYInvInset(yInvInset)
            , fSrcCoordTransform(SkMatrix::I(), fSrc.proxy()) {
        this->addTextureSampler(&fSrc);
        this->addCoordTransform(&fSrcCoordTransform);
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    TextureSampler fSrc;
    SkIRect fBounds;
    SkRect fSrcRect;
    float fXInvZoom;
    float fYInvZoom;
    float fXInvInset;
    float fYInvInset;
    GrCoordTransform fSrcCoordTransform;
    typedef GrFragmentProcessor INHERITED;
};
#endif
#endif
