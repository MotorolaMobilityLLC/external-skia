/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureParamsAdjuster.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrGpu.h"
#include "GrGpuResourcePriv.h"
#include "GrResourceKey.h"
#include "GrTexture.h"
#include "GrTextureParams.h"
#include "GrTextureProvider.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkGrPriv.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrTextureDomain.h"

typedef GrTextureProducer::CopyParams CopyParams;

//////////////////////////////////////////////////////////////////////////////

static GrTexture* copy_on_gpu(GrTexture* inputTexture, const SkIRect* subset,
                              const CopyParams& copyParams) {
    SkASSERT(!subset || !subset->isEmpty());
    GrContext* context = inputTexture->getContext();
    SkASSERT(context);
    const GrCaps* caps = context->caps();

    // Either it's a cache miss or the original wasn't cached to begin with.
    GrSurfaceDesc rtDesc = inputTexture->desc();
    rtDesc.fFlags = rtDesc.fFlags | kRenderTarget_GrSurfaceFlag;
    rtDesc.fWidth = copyParams.fWidth;
    rtDesc.fHeight = copyParams.fHeight;
    rtDesc.fConfig = GrMakePixelConfigUncompressed(rtDesc.fConfig);

    // If the config isn't renderable try converting to either A8 or an 32 bit config. Otherwise,
    // fail.
    if (!caps->isConfigRenderable(rtDesc.fConfig, false)) {
        if (GrPixelConfigIsAlphaOnly(rtDesc.fConfig)) {
            if (caps->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
                rtDesc.fConfig = kAlpha_8_GrPixelConfig;
            } else if (caps->isConfigRenderable(kSkia8888_GrPixelConfig, false)) {
                rtDesc.fConfig = kSkia8888_GrPixelConfig;
            } else {
                return nullptr;
            }
        } else if (kRGB_GrColorComponentFlags ==
                   (kRGB_GrColorComponentFlags & GrPixelConfigComponentMask(rtDesc.fConfig))) {
            if (caps->isConfigRenderable(kSkia8888_GrPixelConfig, false)) {
                rtDesc.fConfig = kSkia8888_GrPixelConfig;
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    SkAutoTUnref<GrTexture> copy(context->textureProvider()->createTexture(rtDesc, true));
    if (!copy) {
        return nullptr;
    }

    // TODO: If no scaling is being performed then use copySurface.

    GrPaint paint;

    SkScalar sx;
    SkScalar sy;
    if (subset) {
        sx = 1.f / inputTexture->width();
        sy = 1.f / inputTexture->height();
    }

    if (copyParams.fFilter != GrTextureParams::kNone_FilterMode && subset &&
        (subset->width() != copyParams.fWidth || subset->height() != copyParams.fHeight)) {
        SkRect domain;
        domain.fLeft = (subset->fLeft + 0.5f) * sx;
        domain.fTop = (subset->fTop + 0.5f)* sy;
        domain.fRight = (subset->fRight - 0.5f) * sx;
        domain.fBottom = (subset->fBottom - 0.5f) * sy;
        // This would cause us to read values from outside the subset. Surely, the caller knows
        // better!
        SkASSERT(copyParams.fFilter != GrTextureParams::kMipMap_FilterMode);
        paint.addColorFragmentProcessor(
            GrTextureDomainEffect::Create(inputTexture, SkMatrix::I(), domain,
                                          GrTextureDomain::kClamp_Mode,
                                          copyParams.fFilter))->unref();
    } else {
        GrTextureParams params(SkShader::kClamp_TileMode, copyParams.fFilter);
        paint.addColorTextureProcessor(inputTexture, SkMatrix::I(), params);
    }

    SkRect localRect;
    if (subset) {
        localRect = SkRect::Make(*subset);
        localRect.fLeft *= sx;
        localRect.fTop *= sy;
        localRect.fRight *= sx;
        localRect.fBottom *= sy;
    } else {
        localRect = SkRect::MakeWH(1.f, 1.f);
    }

    SkAutoTUnref<GrDrawContext> drawContext(context->drawContext(copy->asRenderTarget()));
    if (!drawContext) {
        return nullptr;
    }

    SkRect dstRect = SkRect::MakeWH(SkIntToScalar(rtDesc.fWidth), SkIntToScalar(rtDesc.fHeight));
    drawContext->fillRectToRect(GrClip::WideOpen(), paint, SkMatrix::I(), dstRect, localRect);
    return copy.detach();
}

GrTextureAdjuster::GrTextureAdjuster(GrTexture* original, const SkIRect& contentArea)
    : fOriginal(original) {
    if (contentArea.fLeft > 0 || contentArea.fTop > 0 ||
        contentArea.fRight < original->width() || contentArea.fBottom < original->height()) {
        fContentArea.set(contentArea);
    }
}

GrTexture* GrTextureAdjuster::refTextureSafeForParams(const GrTextureParams& params,
                                                      SkIPoint* outOffset) {
    GrTexture* texture = this->originalTexture();
    GrContext* context = texture->getContext();
    CopyParams copyParams;
    const SkIRect* contentArea = this->contentAreaOrNull();

    if (contentArea && GrTextureParams::kMipMap_FilterMode == params.filterMode()) {
        // If we generate a MIP chain for texture it will read pixel values from outside the content
        // area.
        copyParams.fWidth = contentArea->width();
        copyParams.fHeight = contentArea->height();
        copyParams.fFilter = GrTextureParams::kBilerp_FilterMode;
    } else if (!context->getGpu()->makeCopyForTextureParams(texture->width(), texture->height(),
                                                            params, &copyParams)) {
        if (outOffset) {
            if (contentArea) {
                outOffset->set(contentArea->fLeft, contentArea->fRight);
            } else {
                outOffset->set(0, 0);
            }
        }
        return SkRef(texture);
    }
    GrUniqueKey key;
    this->makeCopyKey(copyParams, &key);
    if (key.isValid()) {
        GrTexture* result = context->textureProvider()->findAndRefTextureByUniqueKey(key);
        if (result) {
            return result;
        }
    }
    GrTexture* result = copy_on_gpu(texture, contentArea, copyParams);
    if (result) {
        if (key.isValid()) {
            result->resourcePriv().setUniqueKey(key);
            this->didCacheCopy(key);
        }
        if (outOffset) {
            outOffset->set(0, 0);
        }
    }
    return result;
}

enum DomainMode {
    kNoDomain_DomainMode,
    kDomain_DomainMode,
    kTightCopy_DomainMode
};

/** Determines whether a texture domain is necessary and if so what domain to use. There are two
 *  rectangles to consider:
 *  - The first is the content area specified by the texture adjuster. We can *never* allow
 *    filtering to cause bleed of pixels outside this rectangle.
 *  - The second rectangle is the constraint rectangle, which is known to be contained by the
 *    content area. The filterConstraint specifies whether we are allowed to bleed across this
 *    rect.
 *
 *  We want to avoid using a domain if possible. We consider the above rectangles, the filter type,
 *  and whether the coords generated by the draw would all fall within the constraint rect. If the
 *  latter is true we only need to consider whether the filter would extend beyond the rects.
 */
static DomainMode determine_domain_mode(
                                    const SkRect& constraintRect,
                                    GrTextureAdjuster::FilterConstraint filterConstraint,
                                    bool coordsLimitedToConstraintRect,
                                    int texW, int texH,
                                    const SkIRect* textureContentArea,
                                    const GrTextureParams::FilterMode* filterModeOrNullForBicubic,
                                    SkRect* domainRect) {

    SkASSERT(SkRect::MakeIWH(texW, texH).contains(constraintRect));
    // We only expect a content area rect if there is some non-content area.
    SkASSERT(!textureContentArea ||
             (!textureContentArea->contains(SkIRect::MakeWH(texW, texH)) &&
              SkRect::Make(*textureContentArea).contains(constraintRect)));

    SkRect textureBounds = SkRect::MakeIWH(texW, texH);
    // If the src rectangle contains the whole texture then no need for a domain.
    if (constraintRect.contains(textureBounds)) {
        return kNoDomain_DomainMode;
    }

    bool restrictFilterToRect = (filterConstraint == GrTextureAdjuster::kYes_FilterConstraint);

    // If we can filter outside the constraint rect, and there is no non-content area of the
    // texture, and we aren't going to generate sample coords outside the constraint rect then we
    // don't need a domain.
    if (!restrictFilterToRect && !textureContentArea && coordsLimitedToConstraintRect) {
        return kNoDomain_DomainMode;
    }

    // Get the domain inset based on sampling mode (or bail if mipped)
    SkScalar filterHalfWidth = 0.f;
    if (filterModeOrNullForBicubic) {
        switch (*filterModeOrNullForBicubic) {
            case GrTextureParams::kNone_FilterMode:
                if (coordsLimitedToConstraintRect) {
                    return kNoDomain_DomainMode;
                } else {
                    filterHalfWidth = 0.f;
                }
                break;
            case GrTextureParams::kBilerp_FilterMode:
                filterHalfWidth = .5f;
                break;
            case GrTextureParams::kMipMap_FilterMode:
                // No domain can save use here.
                return kTightCopy_DomainMode;
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
        domainRect->fLeft = constraintRect.fLeft + kDomainInset;
        domainRect->fTop = constraintRect.fTop + kDomainInset;
        domainRect->fRight = constraintRect.fRight - kDomainInset;
        domainRect->fBottom = constraintRect.fBottom - kDomainInset;
    } else if (textureContentArea) {
        // If we got here then: there is a textureContentArea, the coords are limited to the
        // constraint rect, and we're allowed to filter across the constraint rect boundary. So
        // we check whether the filter would reach across the edge of the content area.
        // We will only set the sides that are required.

        domainRect->setLargest();
        if (coordsLimitedToConstraintRect) {
            // We may be able to use the fact that the texture coords are limited to the constraint
            // rect in order to avoid having to add a domain.
            bool needContentAreaConstraint = false;
            if (textureContentArea->fLeft > 0 &&
                textureContentArea->fLeft + filterHalfWidth > constraintRect.fLeft) {
                domainRect->fLeft = textureContentArea->fLeft + kDomainInset;
                needContentAreaConstraint = true;
            }
            if (textureContentArea->fTop > 0 &&
                textureContentArea->fTop + filterHalfWidth > constraintRect.fTop) {
                domainRect->fTop = textureContentArea->fTop + kDomainInset;
                needContentAreaConstraint = true;
            }
            if (textureContentArea->fRight < texW &&
                textureContentArea->fRight - filterHalfWidth < constraintRect.fRight) {
                domainRect->fRight = textureContentArea->fRight - kDomainInset;
                needContentAreaConstraint = true;
            }
            if (textureContentArea->fBottom < texH &&
                textureContentArea->fBottom - filterHalfWidth < constraintRect.fBottom) {
                domainRect->fBottom = textureContentArea->fBottom - kDomainInset;
                needContentAreaConstraint = true;
            }
            if (!needContentAreaConstraint) {
                return kNoDomain_DomainMode;
            }
        } else {
            // Our sample coords for the texture are allowed to be outside the constraintRect so we
            // don't consider it when computing the domain.
            if (textureContentArea->fLeft != 0) {
                domainRect->fLeft = textureContentArea->fLeft + kDomainInset;
            }
            if (textureContentArea->fTop != 0) {
                domainRect->fTop = textureContentArea->fTop + kDomainInset;
            }
            if (textureContentArea->fRight != texW) {
                domainRect->fRight = textureContentArea->fRight - kDomainInset;
            }
            if (textureContentArea->fBottom != texH) {
                domainRect->fBottom = textureContentArea->fBottom - kDomainInset;
            }
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
    domainRect->fLeft /= texW;
    domainRect->fTop /= texH;
    domainRect->fRight /= texW;
    domainRect->fBottom /= texH;
    return kDomain_DomainMode;
}

const GrFragmentProcessor* GrTextureAdjuster::createFragmentProcessor(
                                        const SkMatrix& textureMatrix,
                                        const SkRect& constraintRect,
                                        FilterConstraint filterConstraint,
                                        bool coordsLimitedToConstraintRect,
                                        const GrTextureParams::FilterMode* filterOrNullForBicubic) {

    const SkIRect* contentArea = this->contentAreaOrNull();

    SkRect domain;
    GrTexture* texture = this->originalTexture();
    DomainMode domainMode =
        determine_domain_mode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                              texture->width(), texture->height(),
                              contentArea, filterOrNullForBicubic,
                              &domain);
    if (kTightCopy_DomainMode == domainMode) {
        // TODO: Copy the texture and adjust the texture matrix (both parts need to consider
        // non-int constraint rect)
        // For now: treat as bilerp and ignore what goes on above level 0.

        // We only expect MIP maps to require a tight copy.
        SkASSERT(filterOrNullForBicubic &&
                 GrTextureParams::kMipMap_FilterMode == *filterOrNullForBicubic);
        static const GrTextureParams::FilterMode kBilerp = GrTextureParams::kBilerp_FilterMode;
        domainMode =
            determine_domain_mode(constraintRect, filterConstraint, coordsLimitedToConstraintRect,
                                  texture->width(), texture->height(),
                                  contentArea, &kBilerp, &domain);
        SkASSERT(kTightCopy_DomainMode != domainMode);
        filterOrNullForBicubic = &kBilerp;
    }
    SkASSERT(kNoDomain_DomainMode == domainMode ||
             (domain.fLeft <= domain.fRight && domain.fTop <= domain.fBottom));
    if (filterOrNullForBicubic) {
        if (kDomain_DomainMode == domainMode) {
            SkASSERT(*filterOrNullForBicubic != GrTextureParams::kMipMap_FilterMode);
            return GrTextureDomainEffect::Create(texture, textureMatrix, domain,
                                                 GrTextureDomain::kClamp_Mode,
                                                 *filterOrNullForBicubic);
        } else {
            GrTextureParams params(SkShader::kClamp_TileMode, *filterOrNullForBicubic);
            return GrSimpleTextureEffect::Create(texture, textureMatrix, params);
        }
    } else {
        if (kDomain_DomainMode == domainMode) {
            return GrBicubicEffect::Create(texture, textureMatrix, domain);
        } else {
            static const SkShader::TileMode kClampClamp[] =
            { SkShader::kClamp_TileMode, SkShader::kClamp_TileMode };
            return GrBicubicEffect::Create(texture, textureMatrix, kClampClamp);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

GrTexture* GrTextureMaker::refTextureForParams(GrContext* ctx, const GrTextureParams& params) {
    CopyParams copyParams;
    if (!ctx->getGpu()->makeCopyForTextureParams(this->width(), this->height(), params,
                                                 &copyParams)) {
        return this->refOriginalTexture(ctx);
    }
    GrUniqueKey copyKey;
    this->makeCopyKey(copyParams, &copyKey);
    if (copyKey.isValid()) {
        GrTexture* result = ctx->textureProvider()->findAndRefTextureByUniqueKey(copyKey);
        if (result) {
            return result;
        }
    }

    GrTexture* result = this->generateTextureForParams(ctx, copyParams);
    if (!result) {
        return nullptr;
    }

    if (copyKey.isValid()) {
        ctx->textureProvider()->assignUniqueKeyToTexture(copyKey, result);
        this->didCacheCopy(copyKey);
    }
    return result;
}

GrTexture* GrTextureMaker::generateTextureForParams(GrContext* ctx, const CopyParams& copyParams) {
    SkAutoTUnref<GrTexture> original(this->refOriginalTexture(ctx));
    if (!original) {
        return nullptr;
    }
    return copy_on_gpu(original, nullptr, copyParams);
}
