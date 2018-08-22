/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBlurUtils.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrFixedClip.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrShape.h"
#include "GrSoftwarePathRenderer.h"
#include "GrStyle.h"
#include "GrTextureProxy.h"
#include "effects/GrSimpleTextureEffect.h"

#include "SkDraw.h"
#include "SkGr.h"
#include "SkMaskFilterBase.h"
#include "SkPaint.h"
#include "SkTLazy.h"

static bool clip_bounds_quick_reject(const SkIRect& clipBounds, const SkIRect& rect) {
    return clipBounds.isEmpty() || rect.isEmpty() || !SkIRect::Intersects(clipBounds, rect);
}

// Draw a mask using the supplied paint. Since the coverage/geometry
// is already burnt into the mask this boils down to a rect draw.
// Return true if the mask was successfully drawn.
static bool draw_mask(GrRenderTargetContext* renderTargetContext,
                      const GrClip& clip,
                      const SkMatrix& viewMatrix,
                      const SkIRect& maskRect,
                      GrPaint&& paint,
                      sk_sp<GrTextureProxy> mask) {
    SkMatrix inverse;
    if (!viewMatrix.invert(&inverse)) {
        return false;
    }

    SkMatrix matrix = SkMatrix::MakeTrans(-SkIntToScalar(maskRect.fLeft),
                                          -SkIntToScalar(maskRect.fTop));
    matrix.preConcat(viewMatrix);
    paint.addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(std::move(mask), matrix));

    renderTargetContext->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                                 SkRect::Make(maskRect), inverse);
    return true;
}

static void mask_release_proc(void* addr, void* /*context*/) {
    SkMask::FreeImage(addr);
}

static bool sw_draw_with_mask_filter(GrContext* context,
                                     GrRenderTargetContext* renderTargetContext,
                                     const GrClip& clipData,
                                     const SkMatrix& viewMatrix,
                                     const GrShape& shape,
                                     const SkMaskFilter* filter,
                                     const SkIRect& clipBounds,
                                     GrPaint&& paint,
                                     const GrUniqueKey& key) {
    SkASSERT(filter);
    SkASSERT(!shape.style().applies());

    auto proxyProvider = context->contextPriv().proxyProvider();

    sk_sp<GrTextureProxy> maskProxy;

    SkStrokeRec::InitStyle fillOrHairline = shape.style().isSimpleHairline()
                                                    ? SkStrokeRec::kHairline_InitStyle
                                                    : SkStrokeRec::kFill_InitStyle;

    if (key.isValid()) {
        // TODO: this cache look up is duplicated in draw_shape_with_mask_filter for gpu
        maskProxy = proxyProvider->findProxyByUniqueKey(key, kTopLeft_GrSurfaceOrigin);
    }

    SkIRect drawRect;
    if (maskProxy) {
        SkRect devBounds = shape.bounds();
        viewMatrix.mapRect(&devBounds);

        // Here we need to recompute the destination bounds in order to draw the mask correctly
        SkMask srcM, dstM;
        if (!SkDraw::ComputeMaskBounds(devBounds, &clipBounds, filter, &viewMatrix,
                                       &srcM.fBounds)) {
            return false;
        }

        srcM.fFormat = SkMask::kA8_Format;

        if (!as_MFB(filter)->filterMask(&dstM, srcM, viewMatrix, nullptr)) {
            return false;
        }

        SkASSERT(dstM.fBounds.width() == maskProxy->width());
        SkASSERT(dstM.fBounds.height() == maskProxy->height());
        drawRect = dstM.fBounds;
    } else {
        // TODO: it seems like we could create an SkDraw here and set its fMatrix field rather
        // than explicitly transforming the path to device space.
        SkPath devPath;

        shape.asPath(&devPath);

        devPath.transform(viewMatrix);

        SkMask srcM, dstM;
        if (!SkDraw::DrawToMask(devPath, &clipBounds, filter, &viewMatrix, &srcM,
                                SkMask::kComputeBoundsAndRenderImage_CreateMode, fillOrHairline)) {
            return false;
        }
        SkAutoMaskFreeImage autoSrc(srcM.fImage);

        SkASSERT(SkMask::kA8_Format == srcM.fFormat);

        if (!as_MFB(filter)->filterMask(&dstM, srcM, viewMatrix, nullptr)) {
            return false;
        }
        // this will free-up dstM when we're done (allocated in filterMask())
        SkAutoMaskFreeImage autoDst(dstM.fImage);

        if (clip_bounds_quick_reject(clipBounds, dstM.fBounds)) {
            return false;
        }

        // we now have a device-aligned 8bit mask in dstM, ready to be drawn using
        // the current clip (and identity matrix) and GrPaint settings
        SkBitmap bm;
        if (!bm.installPixels(SkImageInfo::MakeA8(dstM.fBounds.width(), dstM.fBounds.height()),
                              autoDst.release(), dstM.fRowBytes, mask_release_proc, nullptr)) {
            return false;
        }
        bm.setImmutable();

        sk_sp<SkImage> image = SkImage::MakeFromBitmap(bm);
        if (!image) {
            return false;
        }

        maskProxy = proxyProvider->createTextureProxy(std::move(image),
                                                      kNone_GrSurfaceFlags,
                                                      1, SkBudgeted::kYes,
                                                      SkBackingFit::kApprox);
        if (!maskProxy) {
            return false;
        }

        drawRect = dstM.fBounds;

        if (key.isValid()) {
            proxyProvider->assignUniqueKeyToProxy(key, maskProxy.get());
        }
    }

    return draw_mask(renderTargetContext, clipData, viewMatrix, drawRect,
                     std::move(paint), std::move(maskProxy));
}

// Create a mask of 'shape' and place the result in 'mask'.
static sk_sp<GrTextureProxy> create_mask_GPU(GrContext* context,
                                             const SkIRect& maskRect,
                                             const SkMatrix& origViewMatrix,
                                             const GrShape& shape,
                                             GrAA aa,
                                             int sampleCnt) {
    if (GrAA::kNo == aa) {
        // Don't need MSAA if mask isn't AA
        sampleCnt = 1;
    }

    sk_sp<GrRenderTargetContext> rtContext(
        context->contextPriv().makeDeferredRenderTargetContextWithFallback(
            SkBackingFit::kApprox, maskRect.width(), maskRect.height(), kAlpha_8_GrPixelConfig,
            nullptr, sampleCnt));
    if (!rtContext) {
        return nullptr;
    }

    rtContext->priv().absClear(nullptr, 0x0);

    GrPaint maskPaint;
    maskPaint.setCoverageSetOpXPFactory(SkRegion::kReplace_Op);

    // setup new clip
    const SkIRect clipRect = SkIRect::MakeWH(maskRect.width(), maskRect.height());
    GrFixedClip clip(clipRect);

    // Draw the mask into maskTexture with the path's integerized top-left at
    // the origin using maskPaint.
    SkMatrix viewMatrix = origViewMatrix;
    viewMatrix.postTranslate(-SkIntToScalar(maskRect.fLeft), -SkIntToScalar(maskRect.fTop));
    rtContext->drawShape(clip, std::move(maskPaint), aa, viewMatrix, shape);
    return rtContext->asTextureProxyRef();
}

static bool get_unclipped_shape_dev_bounds(const GrShape& shape, const SkMatrix& matrix,
                                           SkIRect* devBounds) {
    SkRect shapeBounds = shape.styledBounds();
    if (shapeBounds.isEmpty()) {
        return false;
    }
    SkRect shapeDevBounds;
    matrix.mapRect(&shapeDevBounds, shapeBounds);
    // Even though these are "unclipped" bounds we still clip to the int32_t range.
    // This is the largest int32_t that is representable exactly as a float. The next 63 larger ints
    // would round down to this value when cast to a float, but who really cares.
    // INT32_MIN is exactly representable.
    static constexpr int32_t kMaxInt = 2147483520;
    if (!shapeDevBounds.intersect(SkRect::MakeLTRB(INT32_MIN, INT32_MIN, kMaxInt, kMaxInt))) {
        return false;
    }
    // Make sure that the resulting SkIRect can have representable width and height
    if (SkScalarRoundToInt(shapeDevBounds.width()) > kMaxInt ||
        SkScalarRoundToInt(shapeDevBounds.height()) > kMaxInt) {
        return false;
    }
    shapeDevBounds.roundOut(devBounds);
    return true;
}

// Gets the shape bounds, the clip bounds, and the intersection (if any). Returns false if there
// is no intersection.
static bool get_shape_and_clip_bounds(GrRenderTargetContext* renderTargetContext,
                                      const GrClip& clip,
                                      const GrShape& shape,
                                      const SkMatrix& matrix,
                                      SkIRect* unclippedDevShapeBounds,
                                      SkIRect* devClipBounds) {
    // compute bounds as intersection of rt size, clip, and path
    clip.getConservativeBounds(renderTargetContext->width(),
                               renderTargetContext->height(),
                               devClipBounds);

    if (!get_unclipped_shape_dev_bounds(shape, matrix, unclippedDevShapeBounds)) {
        *unclippedDevShapeBounds = SkIRect::EmptyIRect();
        return false;
    }

    return true;
}

static void draw_shape_with_mask_filter(GrContext* context,
                                        GrRenderTargetContext* renderTargetContext,
                                        const GrClip& clip,
                                        GrPaint&& paint,
                                        GrAA aa,
                                        const SkMatrix& viewMatrix,
                                        const SkMaskFilterBase* maskFilter,
                                        const GrShape& origShape) {
    SkASSERT(maskFilter);

    const GrShape* shape = &origShape;
    SkTLazy<GrShape> tmpShape;

    if (origShape.style().applies()) {
        SkScalar styleScale =  GrStyle::MatrixToScaleFactor(viewMatrix);
        if (0 == styleScale) {
            return;
        }

        tmpShape.init(origShape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, styleScale));
        if (tmpShape.get()->isEmpty()) {
            return;
        }

        shape = tmpShape.get();
    }

    if (maskFilter->directFilterMaskGPU(context,
                                        renderTargetContext,
                                        std::move(paint),
                                        clip,
                                        viewMatrix,
                                        *shape)) {
        // the mask filter was able to draw itself directly, so there's nothing
        // left to do.
        return;
    }

    // If the path is hairline, ignore inverse fill.
    bool inverseFilled = shape->inverseFilled() &&
                         !GrPathRenderer::IsStrokeHairlineOrEquivalent(shape->style(),
                                                                       viewMatrix, nullptr);

    SkIRect unclippedDevShapeBounds, devClipBounds;
    if (!get_shape_and_clip_bounds(renderTargetContext, clip, *shape, viewMatrix,
                                   &unclippedDevShapeBounds,
                                   &devClipBounds)) {
        // TODO: just cons up an opaque mask here
        if (!inverseFilled) {
            return;
        }
    }

    // To prevent overloading the cache with entries during animations we limit the cache of masks
    // to cases where the matrix preserves axis alignment.
    bool useCache = !inverseFilled && viewMatrix.preservesAxisAlignment() &&
                    shape->hasUnstyledKey() && as_MFB(maskFilter)->asABlur(nullptr);

    const SkIRect* boundsForClip = &devClipBounds;
    if (useCache) {
        SkIRect clippedMaskRect, unClippedMaskRect;
        maskFilter->canFilterMaskGPU(*shape, unclippedDevShapeBounds, devClipBounds,
                                     viewMatrix, &clippedMaskRect);
        maskFilter->canFilterMaskGPU(*shape, unclippedDevShapeBounds, unclippedDevShapeBounds,
                                     viewMatrix, &unClippedMaskRect);
        if (clippedMaskRect.isEmpty()) {
            return;
        }

        // Use the cache only if >50% of the filtered mask is visible.
        int unclippedWidth = unClippedMaskRect.width();
        int unclippedHeight = unClippedMaskRect.height();
        int64_t unclippedArea = sk_64_mul(unclippedWidth, unclippedHeight);
        int64_t clippedArea = sk_64_mul(clippedMaskRect.width(), clippedMaskRect.height());
        int maxTextureSize = renderTargetContext->caps()->maxTextureSize();
        if (unclippedArea > 2 * clippedArea || unclippedWidth > maxTextureSize ||
            unclippedHeight > maxTextureSize) {
            useCache = false;
        } else {
            // Make the clip not affect the mask
            boundsForClip = &unclippedDevShapeBounds;
        }
    }

    GrUniqueKey maskKey;
    if (useCache) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&maskKey, kDomain, 5 + 2 + shape->unstyledKeySize(),
                                     "Mask Filtered Masks");

        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkScalar sx = viewMatrix.get(SkMatrix::kMScaleX);
        SkScalar sy = viewMatrix.get(SkMatrix::kMScaleY);
        SkScalar kx = viewMatrix.get(SkMatrix::kMSkewX);
        SkScalar ky = viewMatrix.get(SkMatrix::kMSkewY);
        SkScalar tx = viewMatrix.get(SkMatrix::kMTransX);
        SkScalar ty = viewMatrix.get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning.
        SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
        SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;

        builder[0] = SkFloat2Bits(sx);
        builder[1] = SkFloat2Bits(sy);
        builder[2] = SkFloat2Bits(kx);
        builder[3] = SkFloat2Bits(ky);
        // Distinguish between hairline and filled paths. For hairlines, we also need to include
        // the cap. (SW grows hairlines by 0.5 pixel with round and square caps). Note that
        // stroke-and-fill of hairlines is turned into pure fill by SkStrokeRec, so this covers
        // all cases we might see.
        uint32_t styleBits = shape->style().isSimpleHairline()
                                    ? ((shape->style().strokeRec().getCap() << 1) | 1)
                                    : 0;
        builder[4] = fracX | (fracY >> 8) | (styleBits << 16);

        SkMaskFilterBase::BlurRec rec;
        SkAssertResult(as_MFB(maskFilter)->asABlur(&rec));

        builder[5] = rec.fStyle;  // TODO: we could put this with the other style bits
        builder[6] = rec.fSigma;
        shape->writeUnstyledKey(&builder[7]);
    }

    SkIRect maskRect;
    if (maskFilter->canFilterMaskGPU(*shape,
                                     unclippedDevShapeBounds,
                                     *boundsForClip,
                                     viewMatrix,
                                     &maskRect)) {
        if (clip_bounds_quick_reject(*boundsForClip, maskRect)) {
            // clipped out
            return;
        }

        sk_sp<GrTextureProxy> filteredMask;

        GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

        if (maskKey.isValid()) {
            // TODO: this cache look up is duplicated in sw_draw_with_mask_filter for raster
            filteredMask = proxyProvider->findOrCreateProxyByUniqueKey(
                                                maskKey, renderTargetContext->origin());
        }

        if (!filteredMask) {
            sk_sp<GrTextureProxy> maskProxy(create_mask_GPU(
                                                        context,
                                                        maskRect,
                                                        viewMatrix,
                                                        *shape,
                                                        aa,
                                                        renderTargetContext->numColorSamples()));
            if (maskProxy) {
                filteredMask = maskFilter->filterMaskGPU(context,
                                                         std::move(maskProxy),
                                                         viewMatrix,
                                                         maskRect);
                if (filteredMask && maskKey.isValid()) {
                    proxyProvider->assignUniqueKeyToProxy(maskKey, filteredMask.get());
                }
            }
        }

        if (filteredMask) {
            if (draw_mask(renderTargetContext, clip, viewMatrix,
                          maskRect, std::move(paint), std::move(filteredMask))) {
                // This path is completely drawn
                return;
            }
        }
    }

    sw_draw_with_mask_filter(context, renderTargetContext, clip, viewMatrix, *shape,
                             maskFilter, *boundsForClip, std::move(paint), maskKey);
}

void GrBlurUtils::drawShapeWithMaskFilter(GrContext* context,
                                          GrRenderTargetContext* renderTargetContext,
                                          const GrClip& clip,
                                          const GrShape& shape,
                                          GrPaint&& paint,
                                          GrAA aa,
                                          const SkMatrix& viewMatrix,
                                          const SkMaskFilter* mf) {
    draw_shape_with_mask_filter(context, renderTargetContext, clip, std::move(paint), aa,
                                viewMatrix, as_MFB(mf), shape);
}

void GrBlurUtils::drawShapeWithMaskFilter(GrContext* context,
                                          GrRenderTargetContext* renderTargetContext,
                                          const GrClip& clip,
                                          const SkPaint& paint,
                                          const SkMatrix& viewMatrix,
                                          const GrShape& shape) {
    if (context->abandoned()) {
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(context, renderTargetContext->colorSpaceInfo(), paint, viewMatrix,
                          &grPaint)) {
        return;
    }

    GrAA aa = GrAA(paint.isAntiAlias());
    SkMaskFilterBase* mf = as_MFB(paint.getMaskFilter());
    if (mf && !mf->hasFragmentProcessor()) {
        // The MaskFilter wasn't already handled in SkPaintToGrPaint
        draw_shape_with_mask_filter(context, renderTargetContext, clip, std::move(grPaint), aa,
                                    viewMatrix, mf, shape);
    } else {
        renderTargetContext->drawShape(clip, std::move(grPaint), aa, viewMatrix, shape);
    }
}
