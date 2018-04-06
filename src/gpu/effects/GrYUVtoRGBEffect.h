/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrYUVtoRGBEffect.fp; do not modify.
 **************************************************************************************************/
#ifndef GrYUVtoRGBEffect_DEFINED
#define GrYUVtoRGBEffect_DEFINED
#include "SkTypes.h"
#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
class GrYUVtoRGBEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> yProxy,
                                                     sk_sp<GrTextureProxy> uProxy,
                                                     sk_sp<GrTextureProxy> vProxy,
                                                     const SkISize sizes[3],
                                                     SkYUVColorSpace colorSpace, bool nv12);
    SkMatrix44 ySamplerTransform() const { return fYSamplerTransform; }
    SkMatrix44 uSamplerTransform() const { return fUSamplerTransform; }
    SkMatrix44 vSamplerTransform() const { return fVSamplerTransform; }
    SkMatrix44 colorSpaceMatrix() const { return fColorSpaceMatrix; }
    bool nv12() const { return fNv12; }
    static std::unique_ptr<GrFragmentProcessor> Make(
            sk_sp<GrTextureProxy> ySampler, SkMatrix44 ySamplerTransform,
            sk_sp<GrTextureProxy> uSampler, SkMatrix44 uSamplerTransform,
            sk_sp<GrTextureProxy> vSampler, SkMatrix44 vSamplerTransform,
            SkMatrix44 colorSpaceMatrix, bool nv12, GrSamplerState uvSamplerParams) {
        return std::unique_ptr<GrFragmentProcessor>(new GrYUVtoRGBEffect(
                ySampler, ySamplerTransform, uSampler, uSamplerTransform, vSampler,
                vSamplerTransform, colorSpaceMatrix, nv12, uvSamplerParams));
    }
    GrYUVtoRGBEffect(const GrYUVtoRGBEffect& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "YUVtoRGBEffect"; }

private:
    GrYUVtoRGBEffect(sk_sp<GrTextureProxy> ySampler, SkMatrix44 ySamplerTransform,
                     sk_sp<GrTextureProxy> uSampler, SkMatrix44 uSamplerTransform,
                     sk_sp<GrTextureProxy> vSampler, SkMatrix44 vSamplerTransform,
                     SkMatrix44 colorSpaceMatrix, bool nv12, GrSamplerState uvSamplerParams)
            : INHERITED(kGrYUVtoRGBEffect_ClassID, kNone_OptimizationFlags)
            , fYSampler(std::move(ySampler))
            , fYSamplerTransform(ySamplerTransform)
            , fUSampler(std::move(uSampler), uvSamplerParams)
            , fUSamplerTransform(uSamplerTransform)
            , fVSampler(std::move(vSampler), uvSamplerParams)
            , fVSamplerTransform(vSamplerTransform)
            , fColorSpaceMatrix(colorSpaceMatrix)
            , fNv12(nv12)
            , fYSamplerCoordTransform(ySamplerTransform, fYSampler.proxy())
            , fUSamplerCoordTransform(uSamplerTransform, fUSampler.proxy())
            , fVSamplerCoordTransform(vSamplerTransform, fVSampler.proxy()) {
        this->addTextureSampler(&fYSampler);
        this->addTextureSampler(&fUSampler);
        this->addTextureSampler(&fVSampler);
        this->addCoordTransform(&fYSamplerCoordTransform);
        this->addCoordTransform(&fUSamplerCoordTransform);
        this->addCoordTransform(&fVSamplerCoordTransform);
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    TextureSampler fYSampler;
    SkMatrix44 fYSamplerTransform;
    TextureSampler fUSampler;
    SkMatrix44 fUSamplerTransform;
    TextureSampler fVSampler;
    SkMatrix44 fVSamplerTransform;
    SkMatrix44 fColorSpaceMatrix;
    bool fNv12;
    GrCoordTransform fYSamplerCoordTransform;
    GrCoordTransform fUSamplerCoordTransform;
    GrCoordTransform fVSamplerCoordTransform;
    typedef GrFragmentProcessor INHERITED;
};
#endif
