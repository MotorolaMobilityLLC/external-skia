/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSurfaceDrawContext.h"

#include "include/core/SkDrawable.h"
#include "include/core/SkVertices.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/GrImageContext.h"
#include "include/private/SkShadowFlags.h"
#include "include/utils/SkShadowUtils.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkDrawProcs.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrAttachment.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/geometry/GrQuadUtils.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrAtlasTextOp.h"
#include "src/gpu/ops/GrClearOp.h"
#include "src/gpu/ops/GrDrawAtlasOp.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrDrawVerticesOp.h"
#include "src/gpu/ops/GrDrawableOp.h"
#include "src/gpu/ops/GrFillRRectOp.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/ops/GrLatticeOp.h"
#include "src/gpu/ops/GrOp.h"
#include "src/gpu/ops/GrOvalOpFactory.h"
#include "src/gpu/ops/GrRegionOp.h"
#include "src/gpu/ops/GrShadowRRectOp.h"
#include "src/gpu/ops/GrStencilPathOp.h"
#include "src/gpu/ops/GrStrokeRectOp.h"
#include "src/gpu/ops/GrTextureOp.h"
#include "src/gpu/text/GrSDFTControl.h"
#include "src/gpu/text/GrTextBlobCache.h"

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this->drawingManager()->getContext())
#define ASSERT_SINGLE_OWNER        GR_ASSERT_SINGLE_OWNER(this->singleOwner())
#define RETURN_IF_ABANDONED        if (fContext->abandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED  if (fContext->abandoned()) { return false; }

//////////////////////////////////////////////////////////////////////////////

using DoSimplify = GrStyledShape::DoSimplify;

class AutoCheckFlush {
public:
    AutoCheckFlush(GrDrawingManager* drawingManager) : fDrawingManager(drawingManager) {
        SkASSERT(fDrawingManager);
    }
    ~AutoCheckFlush() { fDrawingManager->flushIfNecessary(); }

private:
    GrDrawingManager* fDrawingManager;
};

std::unique_ptr<GrSurfaceDrawContext> GrSurfaceDrawContext::Make(GrRecordingContext* context,
                                                                 GrColorType colorType,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 sk_sp<GrSurfaceProxy> proxy,
                                                                 GrSurfaceOrigin origin,
                                                                 const SkSurfaceProps* surfaceProps,
                                                                 bool flushTimeOpsTask) {
    if (!proxy) {
        return nullptr;
    }

    const GrBackendFormat& format = proxy->backendFormat();
    GrSwizzle readSwizzle, writeSwizzle;
    if (colorType != GrColorType::kUnknown) {
        readSwizzle = context->priv().caps()->getReadSwizzle(format, colorType);
        writeSwizzle = context->priv().caps()->getWriteSwizzle(format, colorType);
    }

    GrSurfaceProxyView readView (           proxy, origin,  readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

    return std::make_unique<GrSurfaceDrawContext>(context,
                                                  std::move(readView),
                                                  std::move(writeView),
                                                  colorType,
                                                  std::move(colorSpace),
                                                  surfaceProps,
                                                  flushTimeOpsTask);
}

std::unique_ptr<GrSurfaceDrawContext> GrSurfaceDrawContext::Make(
        GrRecordingContext* context,
        sk_sp<SkColorSpace> colorSpace,
        SkBackingFit fit,
        SkISize dimensions,
        const GrBackendFormat& format,
        int sampleCnt,
        GrMipmapped mipMapped,
        GrProtected isProtected,
        GrSwizzle readSwizzle,
        GrSwizzle writeSwizzle,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted,
        const SkSurfaceProps* surfaceProps) {
    // It is probably not necessary to check if the context is abandoned here since uses of the
    // GrSurfaceDrawContext which need the context will mostly likely fail later on without an
    // issue. However having this hear adds some reassurance in case there is a path doesn't handle
    // an abandoned context correctly. It also lets us early out of some extra work.
    if (context->abandoned()) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = context->priv().proxyProvider()->createProxy(
            format,
            dimensions,
            GrRenderable::kYes,
            sampleCnt,
            mipMapped,
            fit,
            budgeted,
            isProtected);
    if (!proxy) {
        return nullptr;
    }

    GrSurfaceProxyView readView (           proxy, origin,  readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

    auto rtc = std::make_unique<GrSurfaceDrawContext>(context,
                                                      std::move(readView),
                                                      std::move(writeView),
                                                      GrColorType::kUnknown,
                                                      std::move(colorSpace),
                                                      surfaceProps);
    rtc->discard();
    return rtc;
}

std::unique_ptr<GrSurfaceDrawContext> GrSurfaceDrawContext::Make(
        GrRecordingContext* context,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        SkBackingFit fit,
        SkISize dimensions,
        int sampleCnt,
        GrMipmapped mipMapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted,
        const SkSurfaceProps* surfaceProps) {
    auto format = context->priv().caps()->getDefaultBackendFormat(colorType, GrRenderable::kYes);
    if (!format.isValid()) {
        return nullptr;
    }
    sk_sp<GrTextureProxy> proxy = context->priv().proxyProvider()->createProxy(format,
                                                                               dimensions,
                                                                               GrRenderable::kYes,
                                                                               sampleCnt,
                                                                               mipMapped,
                                                                               fit,
                                                                               budgeted,
                                                                               isProtected);
    if (!proxy) {
        return nullptr;
    }

    return GrSurfaceDrawContext::Make(context,
                                      colorType,
                                      std::move(colorSpace),
                                      std::move(proxy),
                                      origin,
                                      surfaceProps);
}

std::unique_ptr<GrSurfaceDrawContext> GrSurfaceDrawContext::MakeWithFallback(
        GrRecordingContext* context,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        SkBackingFit fit,
        SkISize dimensions,
        int sampleCnt,
        GrMipmapped mipMapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted,
        const SkSurfaceProps* surfaceProps) {
    auto [ct, format] = GetFallbackColorTypeAndFormat(context, colorType, sampleCnt);
    if (ct == GrColorType::kUnknown) {
        return nullptr;
    }
    return GrSurfaceDrawContext::Make(context, ct, colorSpace, fit, dimensions, sampleCnt,
                                      mipMapped, isProtected, origin, budgeted, surfaceProps);
}

std::unique_ptr<GrSurfaceDrawContext> GrSurfaceDrawContext::MakeFromBackendTexture(
        GrRecordingContext* context,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        const GrBackendTexture& tex,
        int sampleCnt,
        GrSurfaceOrigin origin,
        const SkSurfaceProps* surfaceProps,
        sk_sp<GrRefCntedCallback> releaseHelper) {
    SkASSERT(sampleCnt > 0);
    sk_sp<GrTextureProxy> proxy(context->priv().proxyProvider()->wrapRenderableBackendTexture(
            tex, sampleCnt, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
            std::move(releaseHelper)));
    if (!proxy) {
        return nullptr;
    }

    return GrSurfaceDrawContext::Make(context, colorType, std::move(colorSpace), std::move(proxy),
                                      origin, surfaceProps);
}

std::unique_ptr<GrSurfaceDrawContext> GrSurfaceDrawContext::MakeFromBackendRenderTarget(
        GrRecordingContext* context,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        const GrBackendRenderTarget& rt,
        GrSurfaceOrigin origin,
        const SkSurfaceProps* surfaceProps,
        sk_sp<GrRefCntedCallback> releaseHelper) {
    sk_sp<GrSurfaceProxy> proxy(
            context->priv().proxyProvider()->wrapBackendRenderTarget(rt, std::move(releaseHelper)));
    if (!proxy) {
        return nullptr;
    }

    return GrSurfaceDrawContext::Make(context, colorType, std::move(colorSpace), std::move(proxy),
                                      origin, surfaceProps);
}

std::unique_ptr<GrSurfaceDrawContext> GrSurfaceDrawContext::MakeFromVulkanSecondaryCB(
        GrRecordingContext* context,
        const SkImageInfo& imageInfo,
        const GrVkDrawableInfo& vkInfo,
        const SkSurfaceProps* props) {
    sk_sp<GrSurfaceProxy> proxy(
            context->priv().proxyProvider()->wrapVulkanSecondaryCBAsRenderTarget(imageInfo,
                                                                                 vkInfo));
    if (!proxy) {
        return nullptr;
    }

    return GrSurfaceDrawContext::Make(context, SkColorTypeToGrColorType(imageInfo.colorType()),
                                      imageInfo.refColorSpace(), std::move(proxy),
                                      kTopLeft_GrSurfaceOrigin, props);
}

// In MDB mode the reffing of the 'getLastOpsTask' call's result allows in-progress
// GrOpsTask to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpsTask, a new one will be allocated
// when the surfaceDrawContext attempts to use it (via getOpsTask).
GrSurfaceDrawContext::GrSurfaceDrawContext(GrRecordingContext* context,
                                           GrSurfaceProxyView readView,
                                           GrSurfaceProxyView writeView,
                                           GrColorType colorType,
                                           sk_sp<SkColorSpace> colorSpace,
                                           const SkSurfaceProps* surfaceProps,
                                           bool flushTimeOpsTask)
        : GrSurfaceFillContext(context,
                               std::move(readView),
                               std::move(writeView),
                               {colorType, kPremul_SkAlphaType, std::move(colorSpace)},
                               flushTimeOpsTask)
        , fSurfaceProps(SkSurfacePropsCopyOrDefault(surfaceProps))
        , fGlyphPainter(*this) {
    SkDEBUGCODE(this->validate();)
}

GrSurfaceDrawContext::~GrSurfaceDrawContext() {
    ASSERT_SINGLE_OWNER
}

inline GrAAType GrSurfaceDrawContext::chooseAAType(GrAA aa) {
    if (GrAA::kNo == aa) {
        // On some devices we cannot disable MSAA if it is enabled so we make the AA type reflect
        // that.
        if (this->numSamples() > 1 && !this->caps()->multisampleDisableSupport()) {
            return GrAAType::kMSAA;
        }
        return GrAAType::kNone;
    }
    return (this->numSamples() > 1) ? GrAAType::kMSAA : GrAAType::kCoverage;
}

GrMipmapped GrSurfaceDrawContext::mipmapped() const {
    if (const GrTextureProxy* proxy = this->asTextureProxy()) {
        return proxy->mipmapped();
    }
    return GrMipmapped::kNo;
}

void GrSurfaceDrawContext::drawGlyphRunListNoCache(const GrClip* clip,
                                                   const SkMatrixProvider& viewMatrix,
                                                   const SkGlyphRunList& glyphRunList) {
    GrSDFTControl control =
            fContext->priv().getSDFTControl(fSurfaceProps.isUseDeviceIndependentFonts());
    const SkPoint drawOrigin = glyphRunList.origin();
    SkMatrix drawMatrix = viewMatrix.localToDevice();
    drawMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());
    GrSubRunAllocator* const alloc = this->recordingContext()->priv().recordTimeSubRunAllocator();

    for (auto& glyphRun : glyphRunList) {
        GrSubRunNoCachePainter painter{this, alloc, clip, viewMatrix, glyphRunList};

        // Make and add the text ops.
        fGlyphPainter.processGlyphRun(glyphRun,
                                      drawMatrix,
                                      glyphRunList.paint(),
                                      control,
                                      &painter);
    }
}

void GrSurfaceDrawContext::drawGlyphRunListWithCache(const GrClip* clip,
                                                     const SkMatrixProvider& viewMatrix,
                                                     const SkGlyphRunList& glyphRunList) {
    SkMatrix drawMatrix(viewMatrix.localToDevice());
    drawMatrix.preTranslate(glyphRunList.origin().x(), glyphRunList.origin().y());

    GrSDFTControl control =
            this->recordingContext()->priv().getSDFTControl(
                    this->surfaceProps().isUseDeviceIndependentFonts());

    auto [canCache, key] = GrTextBlob::Key::Make(glyphRunList,
                                                 fSurfaceProps,
                                                 this->colorInfo(),
                                                 drawMatrix,
                                                 control);

    sk_sp<GrTextBlob> blob;
    GrTextBlobCache* textBlobCache = fContext->priv().getTextBlobCache();
    if (canCache) {
        blob = textBlobCache->find(key);
    }

    if (blob == nullptr || !blob->canReuse(glyphRunList.paint(), drawMatrix)) {
        if (blob != nullptr) {
            SkASSERT(!drawMatrix.hasPerspective());
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away with reuse most of the time if the pointer is unique,
            //      but we'd have to clear the SubRun information
            textBlobCache->remove(blob.get());
        }

        blob = GrTextBlob::Make(glyphRunList, drawMatrix, control, &fGlyphPainter);

        if (canCache) {
            blob->addKey(key);
            // The blob may already have been created on a different thread. Use the first one
            // that was there.
            blob = textBlobCache->addOrReturnExisting(glyphRunList, blob);
        }
    }

    for (const GrSubRun& subRun : blob->subRunList()) {
        subRun.draw(clip, viewMatrix, glyphRunList, this);
    }
}

// choose to use the GrTextBlob cache or not.
bool gGrDrawTextNoCache = false;
void GrSurfaceDrawContext::drawGlyphRunList(const GrClip* clip,
                                            const SkMatrixProvider& viewMatrix,
                                            const SkGlyphRunList& glyphRunList) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawGlyphRunList", fContext);

    // Drawing text can cause us to do inline uploads. This is not supported for wrapped vulkan
    // secondary command buffers because it would require stopping and starting a render pass which
    // we don't have access to.
    if (this->wrapsVkSecondaryCB()) {
        return;
    }

    if (gGrDrawTextNoCache) {
        // drawGlyphRunListNoCache lives in GrTextBlob.cpp to share sub run implementation code.
        this->drawGlyphRunListNoCache(clip, viewMatrix, glyphRunList);
    } else {
        this->drawGlyphRunListWithCache(clip, viewMatrix, glyphRunList);
    }
}

void GrSurfaceDrawContext::drawPaint(const GrClip* clip,
                                     GrPaint&& paint,
                                     const SkMatrix& viewMatrix) {
    // Start with the render target, since that is the maximum content we could possibly fill.
    // drawFilledQuad() will automatically restrict it to clip bounds for us if possible.
    if (!paint.numTotalFragmentProcessors()) {
        // The paint is trivial so we won't need to use local coordinates, so skip calculating the
        // inverse view matrix.
        SkRect r = this->asSurfaceProxy()->getBoundsRect();
        this->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(), r, r);
    } else {
        // Use the inverse view matrix to arrive at appropriate local coordinates for the paint.
        SkMatrix localMatrix;
        if (!viewMatrix.invert(&localMatrix)) {
            return;
        }
        SkIRect bounds = SkIRect::MakeSize(this->asSurfaceProxy()->dimensions());
        this->fillPixelsWithLocalMatrix(clip, std::move(paint), bounds, localMatrix);
    }
}

enum class GrSurfaceDrawContext::QuadOptimization {
    // The rect to draw doesn't intersect clip or render target, so no draw op should be added
    kDiscarded,
    // The rect to draw was converted to some other op and appended to the oplist, so no additional
    // op is necessary. Currently this can convert it to a clear op or a rrect op. Only valid if
    // a constColor is provided.
    kSubmitted,
    // The clip was folded into the device quad, with updated edge flags and local coords, and
    // caller is responsible for adding an appropriate op.
    kClipApplied,
    // No change to clip, but quad updated to better fit clip/render target, and caller is
    // responsible for adding an appropriate op.
    kCropped
};

GrSurfaceDrawContext::QuadOptimization GrSurfaceDrawContext::attemptQuadOptimization(
        const GrClip* clip, const SkPMColor4f* constColor,
        const GrUserStencilSettings* stencilSettings, GrAA* aa, DrawQuad* quad) {
    // Optimization requirements:
    // 1. kDiscard applies when clip bounds and quad bounds do not intersect
    // 2a. kSubmitted applies when constColor and final geom is pixel aligned rect;
    //       pixel aligned rect requires rect clip and (rect quad or quad covers clip) OR
    // 2b. kSubmitted applies when constColor and rrect clip and quad covers clip
    // 4. kClipApplied applies when rect clip and (rect quad or quad covers clip)
    // 5. kCropped in all other scenarios (although a crop may be a no-op)

    // Save the old AA flags since CropToRect will modify 'quad' and if kCropped is returned, it's
    // better to just keep the old flags instead of introducing mixed edge flags.
    GrQuadAAFlags oldFlags = quad->fEdgeFlags;

    // Use the logical size of the render target, which allows for "fullscreen" clears even if
    // the render target has an approximate backing fit
    SkRect rtRect = this->asSurfaceProxy()->getBoundsRect();

    SkRect drawBounds = quad->fDevice.bounds();
    if (!quad->fDevice.isFinite() || drawBounds.isEmpty() ||
        GrClip::IsOutsideClip(rtRect, drawBounds)) {
        return QuadOptimization::kDiscarded;
    }
    auto conservativeCrop = [&]() {
        static constexpr int kLargeDrawLimit = 15000;
        // Crop the quad to the render target. This doesn't change the visual results of drawing but
        // is meant to help numerical stability for excessively large draws.
        if (drawBounds.width() > kLargeDrawLimit || drawBounds.height() > kLargeDrawLimit) {
            GrQuadUtils::CropToRect(rtRect, *aa, quad, /* compute local */ !constColor);
            SkASSERT(quad->fEdgeFlags == oldFlags);
        }
    };

    bool simpleColor = !stencilSettings && constColor;
    GrClip::PreClipResult result = clip ? clip->preApply(drawBounds, *aa)
                                        : GrClip::PreClipResult(GrClip::Effect::kUnclipped);
    switch(result.fEffect) {
        case GrClip::Effect::kClippedOut:
            return QuadOptimization::kDiscarded;
        case GrClip::Effect::kUnclipped:
            if (!simpleColor) {
                conservativeCrop();
                return QuadOptimization::kClipApplied;
            } else {
                // Update result to store the render target bounds in order and then fall
                // through to attempt the draw->native clear optimization
                result = GrClip::PreClipResult(SkRRect::MakeRect(rtRect), *aa);
            }
            break;
        case GrClip::Effect::kClipped:
            if (!result.fIsRRect || (stencilSettings && result.fAA != *aa) ||
                (!result.fRRect.isRect() && !simpleColor)) {
                // The clip and draw state are too complicated to try and reduce
                conservativeCrop();
                return QuadOptimization::kCropped;
            } // Else fall through to attempt to combine the draw and clip geometry together
            break;
        default:
            SkUNREACHABLE;
    }

    // If we reached here, we know we're an axis-aligned clip that is either a rect or a round rect,
    // so we can potentially combine it with the draw geometry so that no clipping is needed.
    SkASSERT(result.fEffect == GrClip::Effect::kClipped && result.fIsRRect);
    SkRect clippedBounds = result.fRRect.getBounds();
    clippedBounds.intersect(rtRect);
    if (result.fRRect.isRect()) {
        // No rounded corners, so we might be able to become a native clear or we might be able to
        // modify geometry and edge flags to represent intersected shape of clip and draw.
        if (GrQuadUtils::CropToRect(clippedBounds, result.fAA, quad,
                                    /*compute local*/ !constColor)) {
            if (simpleColor && quad->fDevice.quadType() == GrQuad::Type::kAxisAligned) {
                // Clear optimization is possible
                drawBounds = quad->fDevice.bounds();
                if (drawBounds.contains(rtRect)) {
                    // Fullscreen clear
                    this->clear(*constColor);
                    return QuadOptimization::kSubmitted;
                } else if (GrClip::IsPixelAligned(drawBounds) &&
                           drawBounds.width() > 256 && drawBounds.height() > 256) {
                    // Scissor + clear (round shouldn't do anything since we are pixel aligned)
                    SkIRect scissorRect;
                    drawBounds.round(&scissorRect);
                    this->clear(scissorRect, *constColor);
                    return QuadOptimization::kSubmitted;
                }
            }

            // else the draw and clip were combined so just update the AA to reflect combination
            if (*aa == GrAA::kNo && result.fAA == GrAA::kYes &&
                quad->fEdgeFlags != GrQuadAAFlags::kNone) {
                // The clip was anti-aliased and now the draw needs to be upgraded to AA to
                // properly reflect the smooth edge of the clip.
                *aa = GrAA::kYes;
            }
            // We intentionally do not downgrade AA here because we don't know if we need to
            // preserve MSAA (see GrQuadAAFlags docs). But later in the pipeline, the ops can
            // use GrResolveAATypeForQuad() to turn off coverage AA when all flags are off.
            // deviceQuad is exactly the intersection of original quad and clip, so it can be
            // drawn with no clip (submitted by caller)
            return QuadOptimization::kClipApplied;
        }
    } else {
        // Rounded corners and constant filled color (limit ourselves to solid colors because
        // there is no way to use custom local coordinates with drawRRect).
        SkASSERT(simpleColor);
        if (GrQuadUtils::CropToRect(clippedBounds, result.fAA, quad,
                                    /* compute local */ false) &&
            quad->fDevice.quadType() == GrQuad::Type::kAxisAligned &&
            quad->fDevice.bounds().contains(clippedBounds)) {
            // Since the cropped quad became a rectangle which covered the bounds of the rrect,
            // we can draw the rrect directly and ignore the edge flags
            GrPaint paint;
            ClearToGrPaint(constColor->array(), &paint);
            this->drawRRect(nullptr, std::move(paint), result.fAA, SkMatrix::I(), result.fRRect,
                            GrStyle::SimpleFill());
            return QuadOptimization::kSubmitted;
        }
    }

    // The quads have been updated to better fit the clip bounds, but can't get rid of
    // the clip entirely
    quad->fEdgeFlags = oldFlags;
    return QuadOptimization::kCropped;
}

void GrSurfaceDrawContext::drawFilledQuad(const GrClip* clip,
                                          GrPaint&& paint,
                                          GrAA aa,
                                          DrawQuad* quad,
                                          const GrUserStencilSettings* ss) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawFilledQuad", fContext);

    AutoCheckFlush acf(this->drawingManager());

    SkPMColor4f* constColor = nullptr;
    SkPMColor4f paintColor;
    if (!ss && !paint.hasCoverageFragmentProcessor() && paint.isConstantBlendedColor(&paintColor)) {
        // Only consider clears/rrects when it's easy to guarantee 100% fill with single color
        constColor = &paintColor;
    }

    QuadOptimization opt = this->attemptQuadOptimization(clip, constColor, ss, &aa, quad);
    if (opt >= QuadOptimization::kClipApplied) {
        // These optimizations require caller to add an op themselves
        const GrClip* finalClip = opt == QuadOptimization::kClipApplied ? nullptr : clip;
        GrAAType aaType = ss ? (aa == GrAA::kYes ? GrAAType::kMSAA : GrAAType::kNone)
                             : this->chooseAAType(aa);
        this->addDrawOp(finalClip, GrFillRectOp::Make(fContext, std::move(paint), aaType,
                                                      quad, ss));
    }
    // All other optimization levels were completely handled inside attempt(), so no extra op needed
}

void GrSurfaceDrawContext::drawTexturedQuad(const GrClip* clip,
                                            GrSurfaceProxyView proxyView,
                                            SkAlphaType srcAlphaType,
                                            sk_sp<GrColorSpaceXform> textureXform,
                                            GrSamplerState::Filter filter,
                                            GrSamplerState::MipmapMode mm,
                                            const SkPMColor4f& color,
                                            SkBlendMode blendMode,
                                            GrAA aa,
                                            DrawQuad* quad,
                                            const SkRect* subset) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    SkASSERT(proxyView.asTextureProxy());
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawTexturedQuad", fContext);

    AutoCheckFlush acf(this->drawingManager());

    // Functionally this is very similar to drawFilledQuad except that there's no constColor to
    // enable the kSubmitted optimizations, no stencil settings support, and its a GrTextureOp.
    QuadOptimization opt = this->attemptQuadOptimization(clip, nullptr, nullptr, &aa, quad);

    SkASSERT(opt != QuadOptimization::kSubmitted);
    if (opt != QuadOptimization::kDiscarded) {
        // And the texture op if not discarded
        const GrClip* finalClip = opt == QuadOptimization::kClipApplied ? nullptr : clip;
        GrAAType aaType = this->chooseAAType(aa);
        auto clampType = GrColorTypeClampType(this->colorInfo().colorType());
        auto saturate = clampType == GrClampType::kManual ? GrTextureOp::Saturate::kYes
                                                          : GrTextureOp::Saturate::kNo;
        // Use the provided subset, although hypothetically we could detect that the cropped local
        // quad is sufficiently inside the subset and the constraint could be dropped.
        this->addDrawOp(finalClip,
                        GrTextureOp::Make(fContext, std::move(proxyView), srcAlphaType,
                                          std::move(textureXform), filter, mm, color, saturate,
                                          blendMode, aaType, quad, subset));
    }
}

void GrSurfaceDrawContext::drawRect(const GrClip* clip,
                                    GrPaint&& paint,
                                    GrAA aa,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const GrStyle* style) {
    if (!style) {
        style = &GrStyle::SimpleFill();
    }
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawRect", fContext);

    // Path effects should've been devolved to a path in SkGpuDevice
    SkASSERT(!style->pathEffect());

    AutoCheckFlush acf(this->drawingManager());

    const SkStrokeRec& stroke = style->strokeRec();
    if (stroke.getStyle() == SkStrokeRec::kFill_Style) {
        // Fills the rect, using rect as its own local coordinates
        this->fillRectToRect(clip, std::move(paint), aa, viewMatrix, rect, rect);
        return;
    } else if ((stroke.getStyle() == SkStrokeRec::kStroke_Style ||
                stroke.getStyle() == SkStrokeRec::kHairline_Style) &&
               (rect.width() && rect.height())) {
        // Only use the StrokeRectOp for non-empty rectangles. Empty rectangles will be processed by
        // GrStyledShape to handle stroke caps and dashing properly.
        GrAAType aaType = this->chooseAAType(aa);
        GrOp::Owner op = GrStrokeRectOp::Make(
                fContext, std::move(paint), aaType, viewMatrix, rect, stroke);
        // op may be null if the stroke is not supported or if using coverage aa and the view matrix
        // does not preserve rectangles.
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return;
        }
    }
    assert_alive(paint);
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix,
                                     GrStyledShape(rect, *style, DoSimplify::kNo));
}

void GrSurfaceDrawContext::drawQuadSet(const GrClip* clip,
                                       GrPaint&& paint,
                                       GrAA aa,
                                       const SkMatrix& viewMatrix,
                                       const QuadSetEntry quads[],
                                       int cnt) {
    GrAAType aaType = this->chooseAAType(aa);

    GrFillRectOp::AddFillRectOps(this, clip, fContext, std::move(paint), aaType, viewMatrix,
                                 quads, cnt);
}

int GrSurfaceDrawContext::maxWindowRectangles() const {
    return this->asRenderTargetProxy()->maxWindowRectangles(*this->caps());
}

GrOpsTask::CanDiscardPreviousOps GrSurfaceDrawContext::canDiscardPreviousOpsOnFullClear() const {
#if GR_TEST_UTILS
    if (fPreserveOpsOnFullClear_TestingOnly) {
        return GrOpsTask::CanDiscardPreviousOps::kNo;
    }
#endif
    // Regardless of how the clear is implemented (native clear or a fullscreen quad), all prior ops
    // would normally be overwritten. The one exception is if the render target context is marked as
    // needing a stencil buffer then there may be a prior op that writes to the stencil buffer.
    // Although the clear will ignore the stencil buffer, following draw ops may not so we can't get
    // rid of all the preceding ops. Beware! If we ever add any ops that have a side effect beyond
    // modifying the stencil buffer we will need a more elaborate tracking system (skbug.com/7002).
    return GrOpsTask::CanDiscardPreviousOps(!fNumStencilSamples);
}

void GrSurfaceDrawContext::setNeedsStencil(bool useMixedSamplesIfNotMSAA) {
    // Don't clear stencil until after we've changed fNumStencilSamples. This ensures we don't loop
    // forever in the event that there are driver bugs and we need to clear as a draw.
    bool hasInitializedStencil = fNumStencilSamples > 0;

    int numRequiredSamples = this->numSamples();
    if (useMixedSamplesIfNotMSAA && 1 == numRequiredSamples) {
        SkASSERT(this->asRenderTargetProxy()->canUseMixedSamples(*this->caps()));
        numRequiredSamples =
                this->caps()->internalMultisampleCount(this->asSurfaceProxy()->backendFormat());
    }
    SkASSERT(numRequiredSamples > 0);

    if (numRequiredSamples > fNumStencilSamples) {
        fNumStencilSamples = numRequiredSamples;
        this->asRenderTargetProxy()->setNeedsStencil(fNumStencilSamples);
    }

    if (!hasInitializedStencil) {
        if (this->caps()->performStencilClearsAsDraws()) {
            // There is a driver bug with clearing stencil. We must use an op to manually clear the
            // stencil buffer before the op that required 'setNeedsStencil'.
            this->internalStencilClear(nullptr, /* inside mask */ false);
        } else {
            this->getOpsTask()->setInitialStencilContent(
                    GrOpsTask::StencilContent::kUserBitsCleared);
        }
    }
}

void GrSurfaceDrawContext::internalStencilClear(const SkIRect* scissor, bool insideStencilMask) {
    this->setNeedsStencil(/* useMixedSamplesIfNotMSAA = */ false);

    GrScissorState scissorState(this->asSurfaceProxy()->backingStoreDimensions());
    if (scissor && !scissorState.set(*scissor)) {
        // The requested clear region is off screen, so nothing to do.
        return;
    }

    bool clearWithDraw = this->caps()->performStencilClearsAsDraws() ||
                         (scissorState.enabled() && this->caps()->performPartialClearsAsDraws());
    if (clearWithDraw) {
        const GrUserStencilSettings* ss = GrStencilSettings::SetClipBitSettings(insideStencilMask);

        // Configure the paint to have no impact on the color buffer
        GrPaint paint;
        paint.setXPFactory(GrDisableColorXPFactory::Get());
        this->addDrawOp(nullptr,
                        GrFillRectOp::MakeNonAARect(fContext, std::move(paint), SkMatrix::I(),
                                                    SkRect::Make(scissorState.rect()), ss));
    } else {
        this->addOp(GrClearOp::MakeStencilClip(fContext, scissorState, insideStencilMask));
    }
}

bool GrSurfaceDrawContext::stencilPath(const GrHardClip* clip,
                                       GrAA doStencilMSAA,
                                       const SkMatrix& viewMatrix,
                                       const SkPath& path) {
    SkIRect clipBounds = clip ? clip->getConservativeBounds()
                              : SkIRect::MakeSize(this->dimensions());
    GrStyledShape shape(path, GrStyledShape::DoSimplify::kNo);

    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = fContext->priv().caps();
    canDrawArgs.fProxy = this->asRenderTargetProxy();
    canDrawArgs.fClipConservativeBounds = &clipBounds;
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fPaint = nullptr;
    canDrawArgs.fAAType = (doStencilMSAA == GrAA::kYes) ? GrAAType::kMSAA : GrAAType::kNone;
    canDrawArgs.fHasUserStencilSettings = false;
    canDrawArgs.fTargetIsWrappedVkSecondaryCB = this->wrapsVkSecondaryCB();
    GrPathRenderer* pr = this->drawingManager()->getPathRenderer(
            canDrawArgs, false, GrPathRendererChain::DrawType::kStencil);
    if (!pr) {
        SkDebugf("WARNING: No path renderer to stencil path.\n");
        return false;
    }

    GrPathRenderer::StencilPathArgs args;
    args.fContext = fContext;
    args.fRenderTargetContext = this;
    args.fClip = clip;
    args.fClipConservativeBounds = &clipBounds;
    args.fViewMatrix = &viewMatrix;
    args.fShape = &shape;
    args.fDoStencilMSAA = doStencilMSAA;
    pr->stencilPath(args);
    return true;
}

void GrSurfaceDrawContext::stencilPath(const GrHardClip* clip,
                                       GrAA doStencilMSAA,
                                       const SkMatrix& viewMatrix,
                                       sk_sp<const GrPath> path) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "stencilPath", fContext);

    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    SkASSERT(path);
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());

    // FIXME: Use path bounds instead of this WAR once
    // https://bugs.chromium.org/p/skia/issues/detail?id=5640 is resolved.
    SkIRect bounds = SkIRect::MakeSize(this->dimensions());

    // Setup clip and reject offscreen paths; we do this explicitly instead of relying on addDrawOp
    // because GrStencilPathOp is not a draw op as its state depends directly on the choices made
    // during this clip application.
    GrAppliedHardClip appliedClip(this->dimensions(),
                                  this->asSurfaceProxy()->backingStoreDimensions());

    if (clip && GrClip::Effect::kClippedOut == clip->apply(&appliedClip, &bounds)) {
        return;
    }
    // else see FIXME above; we'd normally want to check path bounds with render target bounds,
    // but as it is, we're just using the full render target so intersecting the two bounds would
    // do nothing.

    GrOp::Owner op = GrStencilPathOp::Make(fContext,
                                           viewMatrix,
                                           doStencilMSAA == GrAA::kYes,
                                           appliedClip.hasStencilClip(),
                                           appliedClip.scissorState(),
                                           std::move(path));
    if (!op) {
        return;
    }
    op->setClippedBounds(SkRect::Make(bounds));

    this->setNeedsStencil(GrAA::kYes == doStencilMSAA);
    this->addOp(std::move(op));
}

void GrSurfaceDrawContext::drawTextureSet(const GrClip* clip,
                                          TextureSetEntry set[],
                                          int cnt,
                                          int proxyRunCnt,
                                          GrSamplerState::Filter filter,
                                          GrSamplerState::MipmapMode mm,
                                          SkBlendMode mode,
                                          GrAA aa,
                                          SkCanvas::SrcRectConstraint constraint,
                                          const SkMatrix& viewMatrix,
                                          sk_sp<GrColorSpaceXform> texXform) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawTextureSet", fContext);

    // Create the minimum number of GrTextureOps needed to draw this set. Individual
    // GrTextureOps can rebind the texture between draws thus avoiding GrPaint (re)creation.
    AutoCheckFlush acf(this->drawingManager());
    GrAAType aaType = this->chooseAAType(aa);
    auto clampType = GrColorTypeClampType(this->colorInfo().colorType());
    auto saturate = clampType == GrClampType::kManual ? GrTextureOp::Saturate::kYes
                                                      : GrTextureOp::Saturate::kNo;
    GrTextureOp::AddTextureSetOps(this, clip, fContext, set, cnt, proxyRunCnt, filter, mm, saturate,
                                  mode, aaType, constraint, viewMatrix, std::move(texXform));
}

void GrSurfaceDrawContext::drawVertices(const GrClip* clip,
                                        GrPaint&& paint,
                                        const SkMatrixProvider& matrixProvider,
                                        sk_sp<SkVertices> vertices,
                                        GrPrimitiveType* overridePrimType,
                                        const SkRuntimeEffect* effect) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawVertices", fContext);

    AutoCheckFlush acf(this->drawingManager());

    SkASSERT(vertices);
    GrAAType aaType = this->chooseAAType(GrAA::kNo);
    GrOp::Owner op =
            GrDrawVerticesOp::Make(fContext, std::move(paint), std::move(vertices), matrixProvider,
                                   aaType, this->colorInfo().refColorSpaceXformFromSRGB(),
                                   overridePrimType, effect);
    this->addDrawOp(clip, std::move(op));
}

///////////////////////////////////////////////////////////////////////////////

void GrSurfaceDrawContext::drawAtlas(const GrClip* clip,
                                     GrPaint&& paint,
                                     const SkMatrix& viewMatrix,
                                     int spriteCount,
                                     const SkRSXform xform[],
                                     const SkRect texRect[],
                                     const SkColor colors[]) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawAtlas", fContext);

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(GrAA::kNo);
    GrOp::Owner op = GrDrawAtlasOp::Make(fContext, std::move(paint), viewMatrix,
                                         aaType, spriteCount, xform, texRect, colors);
    this->addDrawOp(clip, std::move(op));
}

///////////////////////////////////////////////////////////////////////////////

void GrSurfaceDrawContext::drawRRect(const GrClip* origClip,
                                     GrPaint&& paint,
                                     GrAA aa,
                                     const SkMatrix& viewMatrix,
                                     const SkRRect& rrect,
                                     const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawRRect", fContext);

    SkASSERT(!style.pathEffect()); // this should've been devolved to a path in SkGpuDevice

    const SkStrokeRec& stroke = style.strokeRec();
    if (stroke.getStyle() == SkStrokeRec::kFill_Style && rrect.isEmpty()) {
       return;
    }

    const GrClip* clip = origClip;
    // It is not uncommon to clip to a round rect and then draw that same round rect. Since our
    // lower level clip code works from op bounds, which are SkRects, it doesn't detect that the
    // clip can be ignored. The following test attempts to mitigate the stencil clip cost but only
    // works for axis-aligned round rects. This also only works for filled rrects since the stroke
    // width outsets beyond the rrect itself.
    // TODO: skbug.com/10462 - There was mixed performance wins and regressions when this
    // optimization was turned on outside of Android Framework. I (michaelludwig) believe this is
    // do to the overhead in determining if an SkClipStack is just a rrect. Once that is improved,
    // re-enable this and see if we avoid the regressions.
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    SkRRect devRRect;
    if (clip && stroke.getStyle() == SkStrokeRec::kFill_Style &&
        rrect.transform(viewMatrix, &devRRect)) {
        GrClip::PreClipResult result = clip->preApply(devRRect.getBounds(), aa);
        switch(result.fEffect) {
            case GrClip::Effect::kClippedOut:
                return;
            case GrClip::Effect::kUnclipped:
                clip = nullptr;
                break;
            case GrClip::Effect::kClipped:
                // Currently there's no general-purpose rrect-to-rrect contains function, and if we
                // got here, we know the devRRect's bounds aren't fully contained by the clip.
                // Testing for equality between the two is a reasonable stop-gap for now.
                if (result.fIsRRect && result.fRRect == devRRect) {
                    // NOTE: On the android framework, we allow this optimization even when the clip
                    // is non-AA and the draw is AA.
                    if (result.fAA == aa || (result.fAA == GrAA::kNo && aa == GrAA::kYes)) {
                        clip = nullptr;
                    }
                }
                break;
            default:
                SkUNREACHABLE;
        }
    }
#endif

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(aa);

    GrOp::Owner op;
    if (GrAAType::kCoverage == aaType && rrect.isSimple() &&
        rrect.getSimpleRadii().fX == rrect.getSimpleRadii().fY &&
        viewMatrix.rectStaysRect() && viewMatrix.isSimilarity()) {
        // In coverage mode, we draw axis-aligned circular roundrects with the GrOvalOpFactory
        // to avoid perf regressions on some platforms.
        assert_alive(paint);
        op = GrOvalOpFactory::MakeCircularRRectOp(
                fContext, std::move(paint), viewMatrix, rrect, stroke, this->caps()->shaderCaps());
    }
    if (!op && style.isSimpleFill()) {
        assert_alive(paint);
        op = GrFillRRectOp::Make(fContext, std::move(paint), viewMatrix, rrect,
                                 GrAA(aaType != GrAAType::kNone));
    }
    if (!op && GrAAType::kCoverage == aaType) {
        assert_alive(paint);
        op = GrOvalOpFactory::MakeRRectOp(
                fContext, std::move(paint), viewMatrix, rrect, stroke, this->caps()->shaderCaps());
    }
    if (op) {
        this->addDrawOp(clip, std::move(op));
        return;
    }

    assert_alive(paint);
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix,
                                     GrStyledShape(rrect, style, DoSimplify::kNo));
}

///////////////////////////////////////////////////////////////////////////////

bool GrSurfaceDrawContext::drawFastShadow(const GrClip* clip,
                                          const SkMatrix& viewMatrix,
                                          const SkPath& path,
                                          const SkDrawShadowRec& rec) {
    ASSERT_SINGLE_OWNER
    if (fContext->abandoned()) {
        return true;
    }
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawFastShadow", fContext);

    // check z plane
    bool tiltZPlane = SkToBool(!SkScalarNearlyZero(rec.fZPlaneParams.fX) ||
                               !SkScalarNearlyZero(rec.fZPlaneParams.fY));
    bool skipAnalytic = SkToBool(rec.fFlags & SkShadowFlags::kGeometricOnly_ShadowFlag);
    if (tiltZPlane || skipAnalytic || !viewMatrix.rectStaysRect() || !viewMatrix.isSimilarity()) {
        return false;
    }

    SkRRect rrect;
    SkRect rect;
    // we can only handle rects, circles, and simple rrects with circular corners
    bool isRRect = path.isRRect(&rrect) && SkRRectPriv::IsNearlySimpleCircular(rrect) &&
                   rrect.getSimpleRadii().fX > SK_ScalarNearlyZero;
    if (!isRRect &&
        path.isOval(&rect) && SkScalarNearlyEqual(rect.width(), rect.height()) &&
        rect.width() > SK_ScalarNearlyZero) {
        rrect.setOval(rect);
        isRRect = true;
    }
    if (!isRRect && path.isRect(&rect)) {
        rrect.setRect(rect);
        isRRect = true;
    }

    if (!isRRect) {
        return false;
    }

    if (rrect.isEmpty()) {
        return true;
    }

    AutoCheckFlush acf(this->drawingManager());

    SkPoint3 devLightPos = rec.fLightPos;
    bool directional = SkToBool(rec.fFlags & kDirectionalLight_ShadowFlag);
    if (directional) {
        devLightPos.normalize();
    } else {
        // transform light
        viewMatrix.mapPoints((SkPoint*)&devLightPos.fX, 1);
    }

    // 1/scale
    SkScalar devToSrcScale = viewMatrix.isScaleTranslate() ?
        SkScalarInvert(SkScalarAbs(viewMatrix[SkMatrix::kMScaleX])) :
        sk_float_rsqrt(viewMatrix[SkMatrix::kMScaleX] * viewMatrix[SkMatrix::kMScaleX] +
                       viewMatrix[SkMatrix::kMSkewX] * viewMatrix[SkMatrix::kMSkewX]);

    SkScalar occluderHeight = rec.fZPlaneParams.fZ;
    bool transparent = SkToBool(rec.fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag);

    if (SkColorGetA(rec.fAmbientColor) > 0) {
        SkScalar devSpaceInsetWidth = SkDrawShadowMetrics::AmbientBlurRadius(occluderHeight);
        const SkScalar umbraRecipAlpha = SkDrawShadowMetrics::AmbientRecipAlpha(occluderHeight);
        const SkScalar devSpaceAmbientBlur = devSpaceInsetWidth * umbraRecipAlpha;

        // Outset the shadow rrect to the border of the penumbra
        SkScalar ambientPathOutset = devSpaceInsetWidth * devToSrcScale;
        SkRRect ambientRRect;
        SkRect outsetRect = rrect.rect().makeOutset(ambientPathOutset, ambientPathOutset);
        // If the rrect was an oval then its outset will also be one.
        // We set it explicitly to avoid errors.
        if (rrect.isOval()) {
            ambientRRect = SkRRect::MakeOval(outsetRect);
        } else {
            SkScalar outsetRad = SkRRectPriv::GetSimpleRadii(rrect).fX + ambientPathOutset;
            ambientRRect = SkRRect::MakeRectXY(outsetRect, outsetRad, outsetRad);
        }

        GrColor ambientColor = SkColorToPremulGrColor(rec.fAmbientColor);
        if (transparent) {
            // set a large inset to force a fill
            devSpaceInsetWidth = ambientRRect.width();
        }

        GrOp::Owner op = GrShadowRRectOp::Make(fContext,
                                               ambientColor,
                                               viewMatrix,
                                               ambientRRect,
                                               devSpaceAmbientBlur,
                                               devSpaceInsetWidth);
        if (op) {
            this->addDrawOp(clip, std::move(op));
        }
    }

    if (SkColorGetA(rec.fSpotColor) > 0) {
        SkScalar devSpaceSpotBlur;
        SkScalar spotScale;
        SkVector spotOffset;
        if (directional) {
            SkDrawShadowMetrics::GetDirectionalParams(occluderHeight, devLightPos.fX,
                                                      devLightPos.fY, devLightPos.fZ,
                                                      rec.fLightRadius, &devSpaceSpotBlur,
                                                      &spotScale, &spotOffset);
        } else {
            SkDrawShadowMetrics::GetSpotParams(occluderHeight, devLightPos.fX, devLightPos.fY,
                                               devLightPos.fZ, rec.fLightRadius,
                                               &devSpaceSpotBlur, &spotScale, &spotOffset);
        }
        // handle scale of radius due to CTM
        const SkScalar srcSpaceSpotBlur = devSpaceSpotBlur * devToSrcScale;

        // Adjust translate for the effect of the scale.
        spotOffset.fX += spotScale*viewMatrix[SkMatrix::kMTransX];
        spotOffset.fY += spotScale*viewMatrix[SkMatrix::kMTransY];
        // This offset is in dev space, need to transform it into source space.
        SkMatrix ctmInverse;
        if (viewMatrix.invert(&ctmInverse)) {
            ctmInverse.mapPoints(&spotOffset, 1);
        } else {
            // Since the matrix is a similarity, this should never happen, but just in case...
            SkDebugf("Matrix is degenerate. Will not render spot shadow correctly!\n");
            SkASSERT(false);
        }

        // Compute the transformed shadow rrect
        SkRRect spotShadowRRect;
        SkMatrix shadowTransform;
        shadowTransform.setScaleTranslate(spotScale, spotScale, spotOffset.fX, spotOffset.fY);
        rrect.transform(shadowTransform, &spotShadowRRect);
        SkScalar spotRadius = spotShadowRRect.getSimpleRadii().fX;

        // Compute the insetWidth
        SkScalar blurOutset = srcSpaceSpotBlur;
        SkScalar insetWidth = blurOutset;
        if (transparent) {
            // If transparent, just do a fill
            insetWidth += spotShadowRRect.width();
        } else {
            // For shadows, instead of using a stroke we specify an inset from the penumbra
            // border. We want to extend this inset area so that it meets up with the caster
            // geometry. The inset geometry will by default already be inset by the blur width.
            //
            // We compare the min and max corners inset by the radius between the original
            // rrect and the shadow rrect. The distance between the two plus the difference
            // between the scaled radius and the original radius gives the distance from the
            // transformed shadow shape to the original shape in that corner. The max
            // of these gives the maximum distance we need to cover.
            //
            // Since we are outsetting by 1/2 the blur distance, we just add the maxOffset to
            // that to get the full insetWidth.
            SkScalar maxOffset;
            if (rrect.isRect()) {
                // Manhattan distance works better for rects
                maxOffset = std::max(std::max(SkTAbs(spotShadowRRect.rect().fLeft -
                                                 rrect.rect().fLeft),
                                          SkTAbs(spotShadowRRect.rect().fTop -
                                                 rrect.rect().fTop)),
                                   std::max(SkTAbs(spotShadowRRect.rect().fRight -
                                                 rrect.rect().fRight),
                                          SkTAbs(spotShadowRRect.rect().fBottom -
                                                 rrect.rect().fBottom)));
            } else {
                SkScalar dr = spotRadius - SkRRectPriv::GetSimpleRadii(rrect).fX;
                SkPoint upperLeftOffset = SkPoint::Make(spotShadowRRect.rect().fLeft -
                                                        rrect.rect().fLeft + dr,
                                                        spotShadowRRect.rect().fTop -
                                                        rrect.rect().fTop + dr);
                SkPoint lowerRightOffset = SkPoint::Make(spotShadowRRect.rect().fRight -
                                                         rrect.rect().fRight - dr,
                                                         spotShadowRRect.rect().fBottom -
                                                         rrect.rect().fBottom - dr);
                maxOffset = SkScalarSqrt(std::max(SkPointPriv::LengthSqd(upperLeftOffset),
                                                  SkPointPriv::LengthSqd(lowerRightOffset))) + dr;
            }
            insetWidth += std::max(blurOutset, maxOffset);
        }

        // Outset the shadow rrect to the border of the penumbra
        SkRect outsetRect = spotShadowRRect.rect().makeOutset(blurOutset, blurOutset);
        if (spotShadowRRect.isOval()) {
            spotShadowRRect = SkRRect::MakeOval(outsetRect);
        } else {
            SkScalar outsetRad = spotRadius + blurOutset;
            spotShadowRRect = SkRRect::MakeRectXY(outsetRect, outsetRad, outsetRad);
        }

        GrColor spotColor = SkColorToPremulGrColor(rec.fSpotColor);

        GrOp::Owner op = GrShadowRRectOp::Make(fContext,
                                               spotColor,
                                               viewMatrix,
                                               spotShadowRRect,
                                               2.0f * devSpaceSpotBlur,
                                               insetWidth);
        if (op) {
            this->addDrawOp(clip, std::move(op));
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void GrSurfaceDrawContext::drawRegion(const GrClip* clip,
                                      GrPaint&& paint,
                                      GrAA aa,
                                      const SkMatrix& viewMatrix,
                                      const SkRegion& region,
                                      const GrStyle& style,
                                      const GrUserStencilSettings* ss) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawRegion", fContext);

    if (GrAA::kYes == aa) {
        // GrRegionOp performs no antialiasing but is much faster, so here we check the matrix
        // to see whether aa is really required.
        if (!SkToBool(viewMatrix.getType() & ~(SkMatrix::kTranslate_Mask)) &&
            SkScalarIsInt(viewMatrix.getTranslateX()) &&
            SkScalarIsInt(viewMatrix.getTranslateY())) {
            aa = GrAA::kNo;
        }
    }
    bool complexStyle = !style.isSimpleFill();
    if (complexStyle || GrAA::kYes == aa) {
        SkPath path;
        region.getBoundaryPath(&path);
        path.setIsVolatile(true);

        return this->drawPath(clip, std::move(paint), aa, viewMatrix, path, style);
    }

    GrAAType aaType = this->chooseAAType(GrAA::kNo);
    GrOp::Owner op = GrRegionOp::Make(fContext, std::move(paint), viewMatrix, region,
                                      aaType, ss);
    this->addDrawOp(clip, std::move(op));
}

void GrSurfaceDrawContext::drawOval(const GrClip* clip,
                                    GrPaint&& paint,
                                    GrAA aa,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& oval,
                                    const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawOval", fContext);

    const SkStrokeRec& stroke = style.strokeRec();

    if (oval.isEmpty() && !style.pathEffect()) {
        if (stroke.getStyle() == SkStrokeRec::kFill_Style) {
            return;
        }

        this->drawRect(clip, std::move(paint), aa, viewMatrix, oval, &style);
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(aa);

    GrOp::Owner op;
    if (GrAAType::kCoverage == aaType && oval.width() > SK_ScalarNearlyZero &&
        oval.width() == oval.height() && viewMatrix.isSimilarity()) {
        // We don't draw true circles as round rects in coverage mode, because it can
        // cause perf regressions on some platforms as compared to the dedicated circle Op.
        assert_alive(paint);
        op = GrOvalOpFactory::MakeCircleOp(fContext, std::move(paint), viewMatrix, oval, style,
                                           this->caps()->shaderCaps());
    }
    if (!op && style.isSimpleFill()) {
        // GrFillRRectOp has special geometry and a fragment-shader branch to conditionally evaluate
        // the arc equation. This same special geometry and fragment branch also turn out to be a
        // substantial optimization for drawing ovals (namely, by not evaluating the arc equation
        // inside the oval's inner diamond). Given these optimizations, it's a clear win to draw
        // ovals the exact same way we do round rects.
        assert_alive(paint);
        op = GrFillRRectOp::Make(fContext, std::move(paint), viewMatrix, SkRRect::MakeOval(oval),
                                 GrAA(aaType != GrAAType::kNone));
    }
    if (!op && GrAAType::kCoverage == aaType) {
        assert_alive(paint);
        op = GrOvalOpFactory::MakeOvalOp(fContext, std::move(paint), viewMatrix, oval, style,
                                         this->caps()->shaderCaps());
    }
    if (op) {
        this->addDrawOp(clip, std::move(op));
        return;
    }

    assert_alive(paint);
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix,
                                     GrStyledShape(SkRRect::MakeOval(oval), SkPathDirection::kCW, 2,
                                                   false, style, DoSimplify::kNo));
}

void GrSurfaceDrawContext::drawArc(const GrClip* clip,
                                   GrPaint&& paint,
                                   GrAA aa,
                                   const SkMatrix& viewMatrix,
                                   const SkRect& oval,
                                   SkScalar startAngle,
                                   SkScalar sweepAngle,
                                   bool useCenter,
                                   const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
            GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawArc", fContext);

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(aa);
    if (GrAAType::kCoverage == aaType) {
        const GrShaderCaps* shaderCaps = this->caps()->shaderCaps();
        GrOp::Owner op = GrOvalOpFactory::MakeArcOp(fContext,
                                                    std::move(paint),
                                                    viewMatrix,
                                                    oval,
                                                    startAngle,
                                                    sweepAngle,
                                                    useCenter,
                                                    style,
                                                    shaderCaps);
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return;
        }
        assert_alive(paint);
    }
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix,
                                     GrStyledShape::MakeArc(oval, startAngle, sweepAngle, useCenter,
                                                            style, DoSimplify::kNo));
}

void GrSurfaceDrawContext::drawImageLattice(const GrClip* clip,
                                            GrPaint&& paint,
                                            const SkMatrix& viewMatrix,
                                            GrSurfaceProxyView view,
                                            SkAlphaType alphaType,
                                            sk_sp<GrColorSpaceXform> csxf,
                                            GrSamplerState::Filter filter,
                                            std::unique_ptr<SkLatticeIter> iter,
                                            const SkRect& dst) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawImageLattice", fContext);

    AutoCheckFlush acf(this->drawingManager());

    GrOp::Owner op =
            GrLatticeOp::MakeNonAA(fContext, std::move(paint), viewMatrix, std::move(view),
                                   alphaType, std::move(csxf), filter, std::move(iter), dst);
    this->addDrawOp(clip, std::move(op));
}

void GrSurfaceDrawContext::drawDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
                                         const SkRect& bounds) {
    GrOp::Owner op(GrDrawableOp::Make(fContext, std::move(drawable), bounds));
    SkASSERT(op);
    this->addOp(std::move(op));
}

bool GrSurfaceDrawContext::waitOnSemaphores(int numSemaphores,
                                            const GrBackendSemaphore waitSemaphores[],
                                            bool deleteSemaphoresAfterWait) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "waitOnSemaphores", fContext);

    AutoCheckFlush acf(this->drawingManager());

    if (numSemaphores && !this->caps()->semaphoreSupport()) {
        return false;
    }

    auto direct = fContext->asDirectContext();
    if (!direct) {
        return false;
    }

    auto resourceProvider = direct->priv().resourceProvider();

    GrWrapOwnership ownership =
            deleteSemaphoresAfterWait ? kAdopt_GrWrapOwnership : kBorrow_GrWrapOwnership;

    std::unique_ptr<std::unique_ptr<GrSemaphore>[]> grSemaphores(
            new std::unique_ptr<GrSemaphore>[numSemaphores]);
    for (int i = 0; i < numSemaphores; ++i) {
        grSemaphores[i] = resourceProvider->wrapBackendSemaphore(
                waitSemaphores[i], GrResourceProvider::SemaphoreWrapType::kWillWait, ownership);
    }
    this->drawingManager()->newWaitRenderTask(this->asSurfaceProxyRef(), std::move(grSemaphores),
                                              numSemaphores);
    return true;
}

void GrSurfaceDrawContext::drawPath(const GrClip* clip,
                                    GrPaint&& paint,
                                    GrAA aa,
                                    const SkMatrix& viewMatrix,
                                    const SkPath& path,
                                    const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawPath", fContext);

    GrStyledShape shape(path, style, DoSimplify::kNo);
    this->drawShape(clip, std::move(paint), aa, viewMatrix, std::move(shape));
}

void GrSurfaceDrawContext::drawShape(const GrClip* clip,
                                     GrPaint&& paint,
                                     GrAA aa,
                                     const SkMatrix& viewMatrix,
                                     GrStyledShape&& shape) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawShape", fContext);

    if (shape.isEmpty()) {
        if (shape.inverseFilled()) {
            this->drawPaint(clip, std::move(paint), viewMatrix);
        }
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    // If we get here in drawShape(), we definitely need to use path rendering
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix, std::move(shape),
                                     /* attemptDrawSimple */ true);
}

static SkIRect get_clip_bounds(const GrSurfaceDrawContext* rtc, const GrClip* clip) {
    return clip ? clip->getConservativeBounds() : SkIRect::MakeWH(rtc->width(), rtc->height());
}

bool GrSurfaceDrawContext::drawAndStencilPath(const GrHardClip* clip,
                                              const GrUserStencilSettings* ss,
                                              SkRegion::Op op,
                                              bool invert,
                                              GrAA aa,
                                              const SkMatrix& viewMatrix,
                                              const SkPath& path) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "drawAndStencilPath", fContext);

    if (path.isEmpty() && path.isInverseFillType()) {
        GrPaint paint;
        paint.setCoverageSetOpXPFactory(op, invert);
        this->stencilRect(clip, ss, std::move(paint), GrAA::kNo, SkMatrix::I(),
                          SkRect::Make(this->dimensions()));
        return true;
    }

    AutoCheckFlush acf(this->drawingManager());

    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    GrAAType aaType = this->chooseAAType(aa);
    bool hasUserStencilSettings = !ss->isUnused();

    SkIRect clipConservativeBounds = get_clip_bounds(this, clip);

    GrPaint paint;
    paint.setCoverageSetOpXPFactory(op, invert);

    GrStyledShape shape(path, GrStyle::SimpleFill());
    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = this->caps();
    canDrawArgs.fProxy = this->asRenderTargetProxy();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fPaint = &paint;
    canDrawArgs.fClipConservativeBounds = &clipConservativeBounds;
    canDrawArgs.fAAType = aaType;
    SkASSERT(!this->wrapsVkSecondaryCB());
    canDrawArgs.fTargetIsWrappedVkSecondaryCB = false;
    canDrawArgs.fHasUserStencilSettings = hasUserStencilSettings;

    // Don't allow the SW renderer
    GrPathRenderer* pr = this->drawingManager()->getPathRenderer(
            canDrawArgs, false, GrPathRendererChain::DrawType::kStencilAndColor);
    if (!pr) {
        return false;
    }

    GrPathRenderer::DrawPathArgs args{this->drawingManager()->getContext(),
                                      std::move(paint),
                                      ss,
                                      this,
                                      clip,
                                      &clipConservativeBounds,
                                      &viewMatrix,
                                      &shape,
                                      aaType,
                                      this->colorInfo().isLinearlyBlended()};
    pr->drawPath(args);
    return true;
}

SkBudgeted GrSurfaceDrawContext::isBudgeted() const {
    ASSERT_SINGLE_OWNER

    if (fContext->abandoned()) {
        return SkBudgeted::kNo;
    }

    SkDEBUGCODE(this->validate();)

    return this->asSurfaceProxy()->isBudgeted();
}

void GrSurfaceDrawContext::drawStrokedLine(const GrClip* clip, GrPaint&& paint,
                                           GrAA aa, const SkMatrix& viewMatrix,
                                           const SkPoint points[2], const SkStrokeRec& stroke) {
    ASSERT_SINGLE_OWNER

    SkASSERT(stroke.getStyle() == SkStrokeRec::kStroke_Style);
    SkASSERT(stroke.getWidth() > 0);
    // Adding support for round capping would require a
    // GrSurfaceDrawContext::fillRRectWithLocalMatrix entry point
    SkASSERT(SkPaint::kRound_Cap != stroke.getCap());

    const SkScalar halfWidth = 0.5f * stroke.getWidth();
    if (halfWidth <= 0.f) {
        // Prevents underflow when stroke width is epsilon > 0 (so technically not a hairline).
        // The CTM would need to have a scale near 1/epsilon in order for this to have meaningful
        // coverage (although that would likely overflow elsewhere and cause the draw to drop due
        // to non-finite bounds). At any other scale, this line is so thin, it's coverage is
        // negligible, so discarding the draw is visually equivalent.
        return;
    }

    SkVector parallel = points[1] - points[0];

    if (!SkPoint::Normalize(&parallel)) {
        parallel.fX = 1.0f;
        parallel.fY = 0.0f;
    }
    parallel *= halfWidth;

    SkVector ortho = { parallel.fY, -parallel.fX };
    if (SkPaint::kButt_Cap == stroke.getCap()) {
        // No extra extension for butt caps
        parallel = {0.f, 0.f};
    }
    // Order is TL, TR, BR, BL where arbitrarily "down" is p0 to p1 and "right" is positive
    SkPoint corners[4] = { points[0] - ortho - parallel,
                           points[0] + ortho - parallel,
                           points[1] + ortho + parallel,
                           points[1] - ortho + parallel };

    GrQuadAAFlags edgeAA = (aa == GrAA::kYes) ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;
    this->fillQuadWithEdgeAA(clip, std::move(paint), aa, edgeAA, viewMatrix, corners, nullptr);
}

bool GrSurfaceDrawContext::drawSimpleShape(const GrClip* clip, GrPaint* paint, GrAA aa,
                                           const SkMatrix& viewMatrix, const GrStyledShape& shape) {
    if (!shape.style().hasPathEffect()) {
        GrAAType aaType = this->chooseAAType(aa);
        SkPoint linePts[2];
        SkRRect rrect;
        // We can ignore the starting point and direction since there is no path effect.
        bool inverted;
        if (shape.asLine(linePts, &inverted) && !inverted &&
            shape.style().strokeRec().getStyle() == SkStrokeRec::kStroke_Style &&
            shape.style().strokeRec().getCap() != SkPaint::kRound_Cap) {
            // The stroked line is an oriented rectangle, which looks the same or better (if
            // perspective) compared to path rendering. The exception is subpixel/hairline lines
            // that are non-AA or MSAA, in which case the default path renderer achieves higher
            // quality.
            // FIXME(michaelludwig): If the fill rect op could take an external coverage, or checks
            // for and outsets thin non-aa rects to 1px, the path renderer could be skipped.
            SkScalar coverage;
            if (aaType == GrAAType::kCoverage ||
                !SkDrawTreatAAStrokeAsHairline(shape.style().strokeRec().getWidth(), viewMatrix,
                                               &coverage)) {
                this->drawStrokedLine(clip, std::move(*paint), aa, viewMatrix, linePts,
                                      shape.style().strokeRec());
                return true;
            }
        } else if (shape.asRRect(&rrect, nullptr, nullptr, &inverted) && !inverted) {
            if (rrect.isRect()) {
                this->drawRect(clip, std::move(*paint), aa, viewMatrix, rrect.rect(),
                               &shape.style());
                return true;
            } else if (rrect.isOval()) {
                this->drawOval(clip, std::move(*paint), aa, viewMatrix, rrect.rect(),
                               shape.style());
                return true;
            }
            this->drawRRect(clip, std::move(*paint), aa, viewMatrix, rrect, shape.style());
            return true;
        } else if (GrAAType::kCoverage == aaType && shape.style().isSimpleFill() &&
                   viewMatrix.rectStaysRect()) {
            // TODO: the rectStaysRect restriction could be lifted if we were willing to apply the
            // matrix to all the points individually rather than just to the rect
            SkRect rects[2];
            if (shape.asNestedRects(rects)) {
                // Concave AA paths are expensive - try to avoid them for special cases
                GrOp::Owner op = GrStrokeRectOp::MakeNested(
                                fContext, std::move(*paint), viewMatrix, rects);
                if (op) {
                    this->addDrawOp(clip, std::move(op));
                }
                // Returning here indicates that there is nothing to draw in this case.
                return true;
            }
        }
    }
    return false;
}

void GrSurfaceDrawContext::drawShapeUsingPathRenderer(const GrClip* clip,
                                                      GrPaint&& paint,
                                                      GrAA aa,
                                                      const SkMatrix& viewMatrix,
                                                      GrStyledShape&& shape,
                                                      bool attemptDrawSimple) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "internalDrawPath", fContext);

    if (!viewMatrix.isFinite() || !shape.bounds().isFinite()) {
        return;
    }

    SkIRect clipConservativeBounds = get_clip_bounds(this, clip);

    GrAAType aaType = this->chooseAAType(aa);

    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = this->caps();
    canDrawArgs.fProxy = this->asRenderTargetProxy();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fPaint = &paint;
    canDrawArgs.fClipConservativeBounds = &clipConservativeBounds;
    canDrawArgs.fTargetIsWrappedVkSecondaryCB = this->wrapsVkSecondaryCB();
    canDrawArgs.fHasUserStencilSettings = false;
    canDrawArgs.fAAType = aaType;

    constexpr static bool kDisallowSWPathRenderer = false;
    constexpr static bool kAllowSWPathRenderer = true;
    using DrawType = GrPathRendererChain::DrawType;

    GrPathRenderer* pr = nullptr;

    if (!shape.style().strokeRec().isFillStyle() && !shape.isEmpty()) {
        // Give the tessellation path renderer a chance to claim this stroke before we simplify it.
        GrPathRenderer* tess = this->drawingManager()->getTessellationPathRenderer();
        if (tess && tess->canDrawPath(canDrawArgs) == GrPathRenderer::CanDrawPath::kYes) {
            pr = tess;
        }
    }

    if (!pr) {
        // The shape isn't a stroke that can be drawn directly. Simplify if possible.
        shape.simplify();

        if (shape.isEmpty() && !shape.inverseFilled()) {
            return;
        }

        if (attemptDrawSimple || shape.simplified()) {
            // Usually we enter drawShapeUsingPathRenderer() because the shape+style was too complex
            // for dedicated draw ops. However, if GrStyledShape was able to reduce something we
            // ought to try again instead of going right to path rendering.
            if (this->drawSimpleShape(clip, &paint, aa, viewMatrix, shape)) {
                return;
            }
        }

        // Try a 1st time without applying any of the style to the geometry (and barring sw)
        pr = this->drawingManager()->getPathRenderer(canDrawArgs, kDisallowSWPathRenderer,
                                                     DrawType::kColor);
    }

    SkScalar styleScale =  GrStyle::MatrixToScaleFactor(viewMatrix);
    if (styleScale == 0.0f) {
        return;
    }

    if (!pr && shape.style().pathEffect()) {
        // It didn't work above, so try again with the path effect applied.
        shape = shape.applyStyle(GrStyle::Apply::kPathEffectOnly, styleScale);
        if (shape.isEmpty()) {
            return;
        }
        pr = this->drawingManager()->getPathRenderer(canDrawArgs, kDisallowSWPathRenderer,
                                                     DrawType::kColor);
    }
    if (!pr) {
        if (shape.style().applies()) {
            shape = shape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, styleScale);
            if (shape.isEmpty()) {
                return;
            }
            // This time, allow SW renderer
            pr = this->drawingManager()->getPathRenderer(canDrawArgs, kAllowSWPathRenderer,
                                                         DrawType::kColor);
        } else {
            pr = this->drawingManager()->getSoftwarePathRenderer();
#if GR_PATH_RENDERER_SPEW
            SkDebugf("falling back to: %s\n", pr->name());
#endif
        }
    }

    if (!pr) {
#ifdef SK_DEBUG
        SkDebugf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    GrPathRenderer::DrawPathArgs args{this->drawingManager()->getContext(),
                                      std::move(paint),
                                      &GrUserStencilSettings::kUnused,
                                      this,
                                      clip,
                                      &clipConservativeBounds,
                                      &viewMatrix,
                                      canDrawArgs.fShape,
                                      aaType,
                                      this->colorInfo().isLinearlyBlended()};
    pr->drawPath(args);
}

static void op_bounds(SkRect* bounds, const GrOp* op) {
    *bounds = op->bounds();
    if (op->hasZeroArea()) {
        if (op->hasAABloat()) {
            bounds->outset(0.5f, 0.5f);
        } else {
            // We don't know which way the particular GPU will snap lines or points at integer
            // coords. So we ensure that the bounds is large enough for either snap.
            SkRect before = *bounds;
            bounds->roundOut(bounds);
            if (bounds->fLeft == before.fLeft) {
                bounds->fLeft -= 1;
            }
            if (bounds->fTop == before.fTop) {
                bounds->fTop -= 1;
            }
            if (bounds->fRight == before.fRight) {
                bounds->fRight += 1;
            }
            if (bounds->fBottom == before.fBottom) {
                bounds->fBottom += 1;
            }
        }
    }
}

void GrSurfaceDrawContext::addDrawOp(const GrClip* clip,
                                     GrOp::Owner op,
                                     const std::function<WillAddOpFn>& willAddFn) {
    ASSERT_SINGLE_OWNER
    if (fContext->abandoned()) {
        return;
    }
    GrDrawOp* drawOp = (GrDrawOp*)op.get();
    SkDEBUGCODE(this->validate();)
    SkDEBUGCODE(drawOp->fAddDrawOpCalled = true;)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrSurfaceDrawContext", "addDrawOp", fContext);

    // Setup clip
    SkRect bounds;
    op_bounds(&bounds, op.get());
    GrAppliedClip appliedClip(this->dimensions(), this->asSurfaceProxy()->backingStoreDimensions());
    GrDrawOp::FixedFunctionFlags fixedFunctionFlags = drawOp->fixedFunctionFlags();
    bool usesHWAA = fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesHWAA;
    bool usesUserStencilBits = fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesStencil;

    if (usesUserStencilBits) {  // Stencil clipping will call setNeedsStencil on its own, if needed.
        this->setNeedsStencil(usesHWAA);
    }

    bool skipDraw = false;
    if (clip) {
        // Have a complex clip, so defer to its early clip culling
        GrAAType aaType;
        if (usesHWAA) {
            aaType = GrAAType::kMSAA;
        } else {
            aaType = op->hasAABloat() ? GrAAType::kCoverage : GrAAType::kNone;
        }
        skipDraw = clip->apply(fContext, this, aaType, usesUserStencilBits,
                               &appliedClip, &bounds) == GrClip::Effect::kClippedOut;
    } else {
        // No clipping, so just clip the bounds against the logical render target dimensions
        skipDraw = !bounds.intersect(this->asSurfaceProxy()->getBoundsRect());
    }

    if (skipDraw) {
        return;
    }

    bool willUseStencil = usesUserStencilBits || appliedClip.hasStencilClip();
    SkASSERT(!willUseStencil || fNumStencilSamples > 0);

    // If stencil is enabled and the framebuffer is mixed sampled, then the graphics pipeline will
    // have mixed sampled coverage, regardless of whether HWAA is enabled. (e.g., a non-aa draw
    // that uses a stencil test when the stencil buffer is multisampled.)
    bool hasMixedSampledCoverage = (
            willUseStencil && fNumStencilSamples > this->numSamples());
    SkASSERT(!hasMixedSampledCoverage ||
             this->asRenderTargetProxy()->canUseMixedSamples(*this->caps()));

    GrClampType clampType = GrColorTypeClampType(this->colorInfo().colorType());
    GrProcessorSet::Analysis analysis = drawOp->finalize(
            *this->caps(), &appliedClip, hasMixedSampledCoverage, clampType);

    // Must be called before setDstProxyView so that it sees the final bounds of the op.
    op->setClippedBounds(bounds);

    GrXferProcessor::DstProxyView dstProxyView;
    if (analysis.requiresDstTexture()) {
        if (!this->setupDstProxyView(*op, &dstProxyView)) {
            return;
        }
    }

    auto opsTask = this->getOpsTask();
    if (willAddFn) {
        willAddFn(op.get(), opsTask->uniqueID());
    }
    opsTask->addDrawOp(this->drawingManager(), std::move(op), analysis, std::move(appliedClip),
                       dstProxyView, GrTextureResolveManager(this->drawingManager()),
                       *this->caps());
}

bool GrSurfaceDrawContext::setupDstProxyView(const GrOp& op,
                                             GrXferProcessor::DstProxyView* dstProxyView) {
    // If we are wrapping a vulkan secondary command buffer, we can't make a dst copy because we
    // don't actually have a VkImage to make a copy of. Additionally we don't have the power to
    // start and stop the render pass in order to make the copy.
    if (this->asRenderTargetProxy()->wrapsVkSecondaryCB()) {
        return false;
    }

    if (fDstSampleType == GrDstSampleType::kNone) {
        fDstSampleType = this->caps()->getDstSampleTypeForProxy(this->asRenderTargetProxy());
    }
    SkASSERT(fDstSampleType != GrDstSampleType::kNone);

    if (GrDstSampleTypeDirectlySamplesDst(fDstSampleType)) {
        // The render target is a texture or input attachment, so we can read from it directly in
        // the shader. The XP will be responsible to detect this situation and request a texture
        // barrier.
        dstProxyView->setProxyView(this->readSurfaceView());
        dstProxyView->setOffset(0, 0);
        dstProxyView->setDstSampleType(fDstSampleType);
        return true;
    }
    SkASSERT(fDstSampleType == GrDstSampleType::kAsTextureCopy);

    GrColorType colorType = this->colorInfo().colorType();
    // MSAA consideration: When there is support for reading MSAA samples in the shader we could
    // have per-sample dst values by making the copy multisampled.
    GrCaps::DstCopyRestrictions restrictions = this->caps()->getDstCopyRestrictions(
            this->asRenderTargetProxy(), colorType);

    SkIRect copyRect = SkIRect::MakeSize(this->asSurfaceProxy()->backingStoreDimensions());
    if (!restrictions.fMustCopyWholeSrc) {
        // If we don't need the whole source, restrict to the op's bounds. We add an extra pixel
        // of padding to account for AA bloat and the unpredictable rounding of coords near pixel
        // centers during rasterization.
        SkIRect conservativeDrawBounds = op.bounds().roundOut();
        conservativeDrawBounds.outset(1, 1);
        SkAssertResult(copyRect.intersect(conservativeDrawBounds));
    }

    SkIPoint dstOffset;
    SkBackingFit fit;
    if (restrictions.fRectsMustMatch == GrSurfaceProxy::RectsMustMatch::kYes) {
        dstOffset = {0, 0};
        fit = SkBackingFit::kExact;
    } else {
        dstOffset = {copyRect.fLeft, copyRect.fTop};
        fit = SkBackingFit::kApprox;
    }
    auto copy = GrSurfaceProxy::Copy(fContext,
                                     this->asSurfaceProxyRef(),
                                     this->origin(),
                                     GrMipmapped::kNo,
                                     copyRect,
                                     fit,
                                     SkBudgeted::kYes,
                                     restrictions.fRectsMustMatch);
    SkASSERT(copy);

    dstProxyView->setProxyView({std::move(copy), this->origin(), this->readSwizzle()});
    dstProxyView->setOffset(dstOffset);
    dstProxyView->setDstSampleType(fDstSampleType);
    return true;
}
