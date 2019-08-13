/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrRectBlurEffect.fp; do not modify.
 **************************************************************************************************/
#ifndef GrRectBlurEffect_DEFINED
#define GrRectBlurEffect_DEFINED
#include "include/core/SkTypes.h"

#include "include/core/SkScalar.h"
#include "src/core/SkBlurMask.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrShaderCaps.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"
class GrRectBlurEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrTextureProxy> CreateBlurProfileTexture(GrProxyProvider* proxyProvider,
                                                          float sigma) {
        unsigned int profileSize = SkScalarCeilToInt(6 * sigma);

        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        GrUniqueKey::Builder builder(&key, kDomain, 1, "Rect Blur Mask");
        builder[0] = profileSize;
        builder.finish();

        sk_sp<GrTextureProxy> blurProfile(proxyProvider->findOrCreateProxyByUniqueKey(
                key, GrColorType::kAlpha_8, kTopLeft_GrSurfaceOrigin));
        if (!blurProfile) {
            SkImageInfo ii = SkImageInfo::MakeA8(profileSize, 1);

            SkBitmap bitmap;
            if (!bitmap.tryAllocPixels(ii)) {
                return nullptr;
            }

            SkBlurMask::ComputeBlurProfile(bitmap.getAddr8(0, 0), profileSize, sigma);
            bitmap.setImmutable();

            sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
            if (!image) {
                return nullptr;
            }

            blurProfile = proxyProvider->createTextureProxy(std::move(image), 1, SkBudgeted::kYes,
                                                            SkBackingFit::kExact);
            if (!blurProfile) {
                return nullptr;
            }

            SkASSERT(blurProfile->origin() == kTopLeft_GrSurfaceOrigin);
            proxyProvider->assignUniqueKeyToProxy(key, blurProfile.get());
        }

        return blurProfile;
    }

    static std::unique_ptr<GrFragmentProcessor> Make(GrProxyProvider* proxyProvider,
                                                     const GrShaderCaps& caps, const SkRect& rect,
                                                     float sigma) {
        if (!caps.floatIs32Bits()) {
            // We promote the rect uniform from half to float when it has large values for
            // precision. If we don't have full float then fail.
            if (SkScalarAbs(rect.fLeft) > 16000.f || SkScalarAbs(rect.fTop) > 16000.f ||
                SkScalarAbs(rect.fRight) > 16000.f || SkScalarAbs(rect.fBottom) > 16000.f ||
                SkScalarAbs(rect.width()) > 16000.f || SkScalarAbs(rect.height()) > 16000.f) {
                return nullptr;
            }
        }
        int doubleProfileSize = SkScalarCeilToInt(12 * sigma);

        if (doubleProfileSize >= rect.width() || doubleProfileSize >= rect.height()) {
            // if the blur sigma is too large so the gaussian overlaps the whole
            // rect in either direction, fall back to CPU path for now.
            return nullptr;
        }

        sk_sp<GrTextureProxy> blurProfile(CreateBlurProfileTexture(proxyProvider, sigma));
        if (!blurProfile) {
            return nullptr;
        }

        return std::unique_ptr<GrFragmentProcessor>(new GrRectBlurEffect(
                rect, sigma, std::move(blurProfile),
                GrSamplerState(GrSamplerState::WrapMode::kClamp, GrSamplerState::Filter::kBilerp)));
    }
    GrRectBlurEffect(const GrRectBlurEffect& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "RectBlurEffect"; }
    SkRect rect;
    float sigma;
    TextureSampler blurProfile;

private:
    GrRectBlurEffect(SkRect rect, float sigma, sk_sp<GrTextureProxy> blurProfile,
                     GrSamplerState samplerParams)
            : INHERITED(kGrRectBlurEffect_ClassID,
                        (OptimizationFlags)kCompatibleWithCoverageAsAlpha_OptimizationFlag)
            , rect(rect)
            , sigma(sigma)
            , blurProfile(std::move(blurProfile), samplerParams) {
        this->setTextureSamplerCnt(1);
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    const TextureSampler& onTextureSampler(int) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    typedef GrFragmentProcessor INHERITED;
};
#endif
