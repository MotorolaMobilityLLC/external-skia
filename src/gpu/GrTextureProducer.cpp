/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrRecordingContext.h"
#include "src/core/SkMipMap.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureProducer.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrTextureDomain.h"
#include "src/gpu/effects/generated/GrSimpleTextureEffect.h"

sk_sp<GrTextureProxy> GrTextureProducer::CopyOnGpu(GrRecordingContext* context,
                                                   sk_sp<GrTextureProxy> inputProxy,
                                                   const CopyParams& copyParams,
                                                   bool dstWillRequireMipMaps) {
    SkASSERT(context);

    GrPixelConfig config = GrMakePixelConfigUncompressed(inputProxy->config());

    const SkRect dstRect = SkRect::MakeIWH(copyParams.fWidth, copyParams.fHeight);
    GrMipMapped mipMapped = dstWillRequireMipMaps ? GrMipMapped::kYes : GrMipMapped::kNo;

    SkRect localRect = SkRect::MakeWH(inputProxy->width(), inputProxy->height());

    bool needsDomain = false;
    bool resizing = false;
    if (copyParams.fFilter != GrSamplerState::Filter::kNearest) {
        bool resizing = localRect.width()  != dstRect.width() ||
                        localRect.height() != dstRect.height();
        needsDomain = resizing && !GrProxyProvider::IsFunctionallyExact(inputProxy.get());
    }

    if (copyParams.fFilter == GrSamplerState::Filter::kNearest && !needsDomain && !resizing &&
        dstWillRequireMipMaps) {
        sk_sp<GrTextureProxy> proxy = GrCopyBaseMipMapToTextureProxy(context, inputProxy.get());
        if (proxy) {
            return proxy;
        }
    }

    GrBackendFormat format = inputProxy->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> copyRTC =
        context->priv().makeDeferredRenderTargetContextWithFallback(
            format, SkBackingFit::kExact, dstRect.width(), dstRect.height(),
            config, nullptr, 1, mipMapped, inputProxy->origin());
    if (!copyRTC) {
        return nullptr;
    }

    GrPaint paint;

    if (needsDomain) {
        const SkRect domain = localRect.makeInset(0.5f, 0.5f);
        // This would cause us to read values from outside the subset. Surely, the caller knows
        // better!
        SkASSERT(copyParams.fFilter != GrSamplerState::Filter::kMipMap);
        paint.addColorFragmentProcessor(
            GrTextureDomainEffect::Make(std::move(inputProxy), SkMatrix::I(), domain,
                                        GrTextureDomain::kClamp_Mode, copyParams.fFilter));
    } else {
        GrSamplerState samplerState(GrSamplerState::WrapMode::kClamp, copyParams.fFilter);
        paint.addColorTextureProcessor(std::move(inputProxy), SkMatrix::I(), samplerState);
    }
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    copyRTC->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), dstRect,
                            localRect);
    return copyRTC->asTextureProxyRef();
}

/** Determines whether a texture domain is necessary and if so what domain to use. There are two
 *  rectangles to consider:
 *  - The first is the content area specified by the texture adjuster (i.e., textureContentArea).
 *    We can *never* allow filtering to cause bleed of pixels outside this rectangle.
 *  - The second rectangle is the constraint rectangle (i.e., constraintRect), which is known to
 *    be contained by the content area. The filterConstraint specifies whether we are allowed to
 *    bleed across this rect.
 *
 *  We want to avoid using a domain if possible. We consider the above rectangles, the filter type,
 *  and whether the coords generated by the draw would all fall within the constraint rect. If the
 *  latter is true we only need to consider whether the filter would extend beyond the rects.
 */
GrTextureProducer::DomainMode GrTextureProducer::DetermineDomainMode(
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        GrTextureProxy* proxy,
        const GrSamplerState::Filter* filterModeOrNullForBicubic,
        SkRect* domainRect) {
    const SkIRect proxyBounds = SkIRect::MakeWH(proxy->width(), proxy->height());

    SkASSERT(proxyBounds.contains(constraintRect));

    const bool proxyIsExact = GrProxyProvider::IsFunctionallyExact(proxy);

    // If the constraint rectangle contains the whole proxy then no need for a domain.
    if (constraintRect.contains(proxyBounds) && proxyIsExact) {
        return kNoDomain_DomainMode;
    }

    bool restrictFilterToRect = (filterConstraint == GrTextureProducer::kYes_FilterConstraint);

    // If we can filter outside the constraint rect, and there is no non-content area of the
    // proxy, and we aren't going to generate sample coords outside the constraint rect then we
    // don't need a domain.
    if (!restrictFilterToRect && proxyIsExact && coordsLimitedToConstraintRect) {
        return kNoDomain_DomainMode;
    }

    // Get the domain inset based on sampling mode (or bail if mipped)
    SkScalar filterHalfWidth = 0.f;
    if (filterModeOrNullForBicubic) {
        switch (*filterModeOrNullForBicubic) {
            case GrSamplerState::Filter::kNearest:
                if (coordsLimitedToConstraintRect) {
                    return kNoDomain_DomainMode;
                } else {
                    filterHalfWidth = 0.f;
                }
                break;
            case GrSamplerState::Filter::kBilerp:
                filterHalfWidth = .5f;
                break;
            case GrSamplerState::Filter::kMipMap:
                if (restrictFilterToRect || !proxyIsExact) {
                    // No domain can save us here.
                    return kTightCopy_DomainMode;
                }
                return kNoDomain_DomainMode;
        }
    } else {
        // bicubic does nearest filtering internally.
        filterHalfWidth = 1.5f;
    }

    // Both bilerp and bicubic use bilinear filtering and so need to be clamped to the center
    // of the edge texel. Pinning to the texel center has no impact on nearest mode and MIP-maps

    static const SkScalar kDomainInset = 0.5f;
    // Figure out the limits of pixels we're allowed to sample from.
    // Unless we know the amount of outset and the texture matrix we have to conservatively enforce
    // the domain.
    if (restrictFilterToRect) {
        *domainRect = constraintRect.makeInset(kDomainInset, kDomainInset);
    } else if (!proxyIsExact) {
        // If we got here then: proxy is not exact, the coords are limited to the
        // constraint rect, and we're allowed to filter across the constraint rect boundary. So
        // we check whether the filter would reach across the edge of the proxy.
        // We will only set the sides that are required.

        *domainRect = SkRectPriv::MakeLargest();
        if (coordsLimitedToConstraintRect) {
            // We may be able to use the fact that the texture coords are limited to the constraint
            // rect in order to avoid having to add a domain.
            bool needContentAreaConstraint = false;
            if (proxyBounds.fRight - filterHalfWidth < constraintRect.fRight) {
                domainRect->fRight = proxyBounds.fRight - kDomainInset;
                needContentAreaConstraint = true;
            }
            if (proxyBounds.fBottom - filterHalfWidth < constraintRect.fBottom) {
                domainRect->fBottom = proxyBounds.fBottom - kDomainInset;
                needContentAreaConstraint = true;
            }
            if (!needContentAreaConstraint) {
                return kNoDomain_DomainMode;
            }
        } else {
            // Our sample coords for the texture are allowed to be outside the constraintRect so we
            // don't consider it when computing the domain.
            domainRect->fRight = proxyBounds.fRight - kDomainInset;
            domainRect->fBottom = proxyBounds.fBottom - kDomainInset;
        }
    } else {
        return kNoDomain_DomainMode;
    }

    if (domainRect->fLeft > domainRect->fRight) {
        domainRect->fLeft = domainRect->fRight = SkScalarAve(domainRect->fLeft, domainRect->fRight);
    }
    if (domainRect->fTop > domainRect->fBottom) {
        domainRect->fTop = domainRect->fBottom = SkScalarAve(domainRect->fTop, domainRect->fBottom);
    }
    return kDomain_DomainMode;
}

std::unique_ptr<GrFragmentProcessor> GrTextureProducer::createFragmentProcessorForDomainAndFilter(
        sk_sp<GrTextureProxy> proxy,
        const SkMatrix& textureMatrix,
        DomainMode domainMode,
        const SkRect& domain,
        const GrSamplerState::Filter* filterOrNullForBicubic) {
    SkASSERT(kTightCopy_DomainMode != domainMode);
    bool clampToBorderSupport = fContext->priv().caps()->clampToBorderSupport();
    if (filterOrNullForBicubic) {
        if (kDomain_DomainMode == domainMode || (fDomainNeedsDecal && !clampToBorderSupport)) {
            GrTextureDomain::Mode wrapMode = fDomainNeedsDecal ? GrTextureDomain::kDecal_Mode
                                                               : GrTextureDomain::kClamp_Mode;
            return GrTextureDomainEffect::Make(std::move(proxy), textureMatrix, domain,
                                               wrapMode, *filterOrNullForBicubic);
        } else {
            GrSamplerState::WrapMode wrapMode =
                    fDomainNeedsDecal ? GrSamplerState::WrapMode::kClampToBorder
                                      : GrSamplerState::WrapMode::kClamp;
            GrSamplerState samplerState(wrapMode, *filterOrNullForBicubic);
            return GrSimpleTextureEffect::Make(std::move(proxy), textureMatrix, samplerState);
        }
    } else {
        static const GrSamplerState::WrapMode kClampClamp[] = {
                GrSamplerState::WrapMode::kClamp, GrSamplerState::WrapMode::kClamp};
        static const GrSamplerState::WrapMode kDecalDecal[] = {
                GrSamplerState::WrapMode::kClampToBorder, GrSamplerState::WrapMode::kClampToBorder};

        static constexpr auto kDir = GrBicubicEffect::Direction::kXY;
        if (kDomain_DomainMode == domainMode || (fDomainNeedsDecal && !clampToBorderSupport)) {
            GrTextureDomain::Mode wrapMode = fDomainNeedsDecal ? GrTextureDomain::kDecal_Mode
                                         : GrTextureDomain::kClamp_Mode;
            return GrBicubicEffect::Make(std::move(proxy), textureMatrix, kClampClamp, wrapMode,
                                         wrapMode, kDir, this->alphaType(),
                                         kDomain_DomainMode == domainMode ? &domain : nullptr);
        } else {
            return GrBicubicEffect::Make(std::move(proxy), textureMatrix,
                                         fDomainNeedsDecal ? kDecalDecal : kClampClamp, kDir,
                                         this->alphaType());
        }
    }
}

sk_sp<GrTextureProxy> GrTextureProducer::refTextureProxyForParams(
        const GrSamplerState::Filter* filterOrNullForBicubic,
        SkScalar scaleAdjust[2]) {
    GrSamplerState sampler; // Default is nearest + clamp
    if (filterOrNullForBicubic) {
        sampler.setFilterMode(*filterOrNullForBicubic);
    }
    if (fDomainNeedsDecal) {
        // Assuming hardware support, switch to clamp-to-border instead of clamp
        if (fContext->priv().caps()->clampToBorderSupport()) {
            sampler.setWrapModeX(GrSamplerState::WrapMode::kClampToBorder);
            sampler.setWrapModeY(GrSamplerState::WrapMode::kClampToBorder);
        }
    }
    return this->refTextureProxyForParams(sampler, scaleAdjust);
}

sk_sp<GrTextureProxy> GrTextureProducer::refTextureProxyForParams(
        const GrSamplerState& sampler,
        SkScalar scaleAdjust[2]) {
    // Check that the caller pre-initialized scaleAdjust
    SkASSERT(!scaleAdjust || (scaleAdjust[0] == 1 && scaleAdjust[1] == 1));
    // Check that if the caller passed nullptr for scaleAdjust that we're in the case where there
    // can be no scaling.
    SkDEBUGCODE(bool expectNoScale = (sampler.filter() != GrSamplerState::Filter::kMipMap &&
                                      !sampler.isRepeated()));
    SkASSERT(scaleAdjust || expectNoScale);

    int mipCount = SkMipMap::ComputeLevelCount(this->width(), this->height());
    bool willBeMipped = GrSamplerState::Filter::kMipMap == sampler.filter() && mipCount &&
                        this->context()->priv().caps()->mipMapSupport();

    auto result = this->onRefTextureProxyForParams(sampler, willBeMipped, scaleAdjust);

    // Check to make sure that if we say the texture willBeMipped that the returned texture has mip
    // maps, unless the config is not copyable.
    SkASSERT(!result || !willBeMipped || result->mipMapped() == GrMipMapped::kYes ||
             !this->context()->priv().caps()->isConfigCopyable(result->config()));

    // Check that the "no scaling expected" case always returns a proxy of the same size as the
    // producer.
    SkASSERT(!result || !expectNoScale ||
             (result->width() == this->width() && result->height() == this->height()));
    return result;
}

sk_sp<GrTextureProxy> GrTextureProducer::refTextureProxy(GrMipMapped willNeedMips) {
    GrSamplerState::Filter filter =
            GrMipMapped::kNo == willNeedMips ? GrSamplerState::Filter::kNearest
                                             : GrSamplerState::Filter::kMipMap;
    GrSamplerState sampler(GrSamplerState::WrapMode::kClamp, filter);

    int mipCount = SkMipMap::ComputeLevelCount(this->width(), this->height());
    bool willBeMipped = GrSamplerState::Filter::kMipMap == sampler.filter() && mipCount &&
                        this->context()->priv().caps()->mipMapSupport();

    auto result = this->onRefTextureProxyForParams(sampler, willBeMipped, nullptr);

    // Check to make sure that if we say the texture willBeMipped that the returned texture has mip
    // maps, unless the config is not copyable.
    SkASSERT(!result || !willBeMipped || result->mipMapped() == GrMipMapped::kYes ||
             !this->context()->priv().caps()->isConfigCopyable(result->config()));

    // Check that no scaling occured and we returned a proxy of the same size as the producer.
    SkASSERT(!result || (result->width() == this->width() && result->height() == this->height()));
    return result;
}
