/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilAndCoverTextContext.h"
#include "GrAtlasTextContext.h"
#include "GrDrawContext.h"
#include "GrDrawTarget.h"
#include "GrPath.h"
#include "GrPathRange.h"
#include "GrResourceProvider.h"
#include "SkAutoKern.h"
#include "SkDraw.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkPath.h"
#include "SkTextMapStateProc.h"
#include "SkTextFormatParams.h"

#include "batches/GrDrawPathBatch.h"

GrStencilAndCoverTextContext::GrStencilAndCoverTextContext(GrContext* context,
                                                           const SkSurfaceProps& surfaceProps)
    : INHERITED(context, surfaceProps) {
}

GrStencilAndCoverTextContext*
GrStencilAndCoverTextContext::Create(GrContext* context, const SkSurfaceProps& surfaceProps) {
    GrStencilAndCoverTextContext* textContext = 
        new GrStencilAndCoverTextContext(context, surfaceProps);
    textContext->fFallbackTextContext = GrAtlasTextContext::Create(context, surfaceProps);

    return textContext;
}

GrStencilAndCoverTextContext::~GrStencilAndCoverTextContext() {
}

bool GrStencilAndCoverTextContext::canDraw(const GrRenderTarget* rt,
                                           const GrClip& clip,
                                           const GrPaint& paint,
                                           const SkPaint& skPaint,
                                           const SkMatrix& viewMatrix) {
    if (skPaint.getRasterizer()) {
        return false;
    }
    if (skPaint.getMaskFilter()) {
        return false;
    }
    if (SkPathEffect* pe = skPaint.getPathEffect()) {
        if (pe->asADash(nullptr) != SkPathEffect::kDash_DashType) {
            return false;
        }
    }
    // No hairlines. They would require new paths with customized strokes for every new draw matrix.
    return SkPaint::kStroke_Style != skPaint.getStyle() || 0 != skPaint.getStrokeWidth();
}

void GrStencilAndCoverTextContext::onDrawText(GrDrawContext* dc, GrRenderTarget* rt,
                                              const GrClip& clip,
                                              const GrPaint& paint,
                                              const SkPaint& skPaint,
                                              const SkMatrix& viewMatrix,
                                              const char text[],
                                              size_t byteLength,
                                              SkScalar x, SkScalar y,
                                              const SkIRect& regionClipBounds) {
    TextRun run(skPaint);
    run.setText(text, byteLength, x, y, fContext, &fSurfaceProps);
    run.draw(dc, rt, clip, paint, viewMatrix, regionClipBounds, fFallbackTextContext, skPaint);
}

void GrStencilAndCoverTextContext::onDrawPosText(GrDrawContext* dc, GrRenderTarget* rt,
                                                 const GrClip& clip,
                                                 const GrPaint& paint,
                                                 const SkPaint& skPaint,
                                                 const SkMatrix& viewMatrix,
                                                 const char text[],
                                                 size_t byteLength,
                                                 const SkScalar pos[],
                                                 int scalarsPerPosition,
                                                 const SkPoint& offset,
                                                 const SkIRect& regionClipBounds) {
    TextRun run(skPaint);
    run.setPosText(text, byteLength, pos, scalarsPerPosition, offset, fContext, &fSurfaceProps);
    run.draw(dc, rt, clip, paint, viewMatrix, regionClipBounds, fFallbackTextContext, skPaint);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GrStencilAndCoverTextContext::TextRun::TextRun(const SkPaint& fontAndStroke)
    : fStroke(fontAndStroke),
      fFont(fontAndStroke) {
    SkASSERT(!fStroke.isHairlineStyle()); // Hairlines are not supported.

    // Setting to "fill" ensures that no strokes get baked into font outlines. (We use the GPU path
    // rendering API for stroking).
    fFont.setStyle(SkPaint::kFill_Style);

    if (fFont.isFakeBoldText() && SkStrokeRec::kStroke_Style != fStroke.getStyle()) {
        // Instead of letting fake bold get baked into the glyph outlines, do it with GPU stroke.
        SkScalar fakeBoldScale = SkScalarInterpFunc(fFont.getTextSize(),
                                                    kStdFakeBoldInterpKeys,
                                                    kStdFakeBoldInterpValues,
                                                    kStdFakeBoldInterpLength);
        SkScalar extra = SkScalarMul(fFont.getTextSize(), fakeBoldScale);
        fStroke.setStrokeStyle(fStroke.needToApply() ? fStroke.getWidth() + extra : extra,
                               true /*strokeAndFill*/);

        fFont.setFakeBoldText(false);
    }

    if (!fFont.getPathEffect() && !fStroke.isDashed()) {
        // We can draw the glyphs from canonically sized paths.
        fTextRatio = fFont.getTextSize() / SkPaint::kCanonicalTextSizeForPaths;
        fTextInverseRatio = SkPaint::kCanonicalTextSizeForPaths / fFont.getTextSize();

        // Compensate for the glyphs being scaled by fTextRatio.
        if (!fStroke.isFillStyle()) {
            fStroke.setStrokeStyle(fStroke.getWidth() / fTextRatio,
                                   SkStrokeRec::kStrokeAndFill_Style == fStroke.getStyle());
        }

        fFont.setLinearText(true);
        fFont.setLCDRenderText(false);
        fFont.setAutohinted(false);
        fFont.setHinting(SkPaint::kNo_Hinting);
        fFont.setSubpixelText(true);
        fFont.setTextSize(SkIntToScalar(SkPaint::kCanonicalTextSizeForPaths));

        fUsingRawGlyphPaths = SK_Scalar1 == fFont.getTextScaleX() &&
                              0 == fFont.getTextSkewX() &&
                              !fFont.isFakeBoldText() &&
                              !fFont.isVerticalText();
    } else {
        fTextRatio = fTextInverseRatio = 1.0f;
        fUsingRawGlyphPaths = false;
    }

    // When drawing from canonically sized paths, the actual local coords are fTextRatio * coords.
    fLocalMatrix.setScale(fTextRatio, fTextRatio);
}

GrStencilAndCoverTextContext::TextRun::~TextRun() {
}

void GrStencilAndCoverTextContext::TextRun::setText(const char text[], size_t byteLength,
                                                    SkScalar x, SkScalar y, GrContext* ctx,
                                                    const SkSurfaceProps* surfaceProps) {
    SkASSERT(byteLength == 0 || text != nullptr);

    SkAutoGlyphCacheNoGamma autoGlyphCache(fFont, surfaceProps, nullptr);
    SkGlyphCache* glyphCache = autoGlyphCache.getCache();

    fDraw.reset(GrPathRangeDraw::Create(this->createGlyphs(ctx, glyphCache),
                                        GrPathRendering::kTranslate_PathTransformType,
                                        fFont.countText(text, byteLength)));

    SkDrawCacheProc glyphCacheProc = fFont.getDrawCacheProc();

    const char* stop = text + byteLength;

    // Measure first if needed.
    if (fFont.getTextAlign() != SkPaint::kLeft_Align) {
        SkFixed    stopX = 0;
        SkFixed    stopY = 0;

        const char* textPtr = text;
        while (textPtr < stop) {
            // We don't need x, y here, since all subpixel variants will have the
            // same advance.
            const SkGlyph& glyph = glyphCacheProc(glyphCache, &textPtr, 0, 0);

            stopX += glyph.fAdvanceX;
            stopY += glyph.fAdvanceY;
        }
        SkASSERT(textPtr == stop);

        SkScalar alignX = SkFixedToScalar(stopX) * fTextRatio;
        SkScalar alignY = SkFixedToScalar(stopY) * fTextRatio;

        if (fFont.getTextAlign() == SkPaint::kCenter_Align) {
            alignX = SkScalarHalf(alignX);
            alignY = SkScalarHalf(alignY);
        }

        x -= alignX;
        y -= alignY;
    }

    SkAutoKern autokern;

    SkFixed fixedSizeRatio = SkScalarToFixed(fTextRatio);

    SkFixed fx = SkScalarToFixed(x);
    SkFixed fy = SkScalarToFixed(y);
    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(glyphCache, &text, 0, 0);
        fx += SkFixedMul(autokern.adjust(glyph), fixedSizeRatio);
        if (glyph.fWidth) {
            this->appendGlyph(glyph, SkPoint::Make(SkFixedToScalar(fx), SkFixedToScalar(fy)));
        }

        fx += SkFixedMul(glyph.fAdvanceX, fixedSizeRatio);
        fy += SkFixedMul(glyph.fAdvanceY, fixedSizeRatio);
    }
}

void GrStencilAndCoverTextContext::TextRun::setPosText(const char text[], size_t byteLength,
                                                       const SkScalar pos[], int scalarsPerPosition,
                                                       const SkPoint& offset, GrContext* ctx,
                                                       const SkSurfaceProps* surfaceProps) {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    SkAutoGlyphCacheNoGamma autoGlyphCache(fFont, surfaceProps, nullptr);
    SkGlyphCache* glyphCache = autoGlyphCache.getCache();

    fDraw.reset(GrPathRangeDraw::Create(this->createGlyphs(ctx, glyphCache),
                                        GrPathRendering::kTranslate_PathTransformType,
                                        fFont.countText(text, byteLength)));

    SkDrawCacheProc glyphCacheProc = fFont.getDrawCacheProc();

    const char* stop = text + byteLength;

    SkTextMapStateProc tmsProc(SkMatrix::I(), offset, scalarsPerPosition);
    SkTextAlignProc alignProc(fFont.getTextAlign());
    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(glyphCache, &text, 0, 0);
        if (glyph.fWidth) {
            SkPoint tmsLoc;
            tmsProc(pos, &tmsLoc);
            SkPoint loc;
            alignProc(tmsLoc, glyph, &loc);

            this->appendGlyph(glyph, loc);
        }
        pos += scalarsPerPosition;
    }
}

GrPathRange* GrStencilAndCoverTextContext::TextRun::createGlyphs(GrContext* ctx,
                                                                 SkGlyphCache* glyphCache) {
    SkTypeface* typeface = fUsingRawGlyphPaths ? fFont.getTypeface()
                                               : glyphCache->getScalerContext()->getTypeface();
    const SkDescriptor* desc = fUsingRawGlyphPaths ? nullptr : &glyphCache->getDescriptor();

    static const GrUniqueKey::Domain kPathGlyphDomain = GrUniqueKey::GenerateDomain();
    int strokeDataCount = fStroke.computeUniqueKeyFragmentData32Cnt();
    GrUniqueKey glyphKey;
    GrUniqueKey::Builder builder(&glyphKey, kPathGlyphDomain, 2 + strokeDataCount);
    reinterpret_cast<uint32_t&>(builder[0]) = desc ? desc->getChecksum() : 0;
    reinterpret_cast<uint32_t&>(builder[1]) = typeface ? typeface->uniqueID() : 0;
    if (strokeDataCount > 0) {
        fStroke.asUniqueKeyFragment(&builder[2]);
    }
    builder.finish();

    SkAutoTUnref<GrPathRange> glyphs(
        static_cast<GrPathRange*>(
            ctx->resourceProvider()->findAndRefResourceByUniqueKey(glyphKey)));
    if (nullptr == glyphs) {
        glyphs.reset(ctx->resourceProvider()->createGlyphs(typeface, desc, fStroke));
        ctx->resourceProvider()->assignUniqueKeyToResource(glyphKey, glyphs);
    } else {
        SkASSERT(nullptr == desc || glyphs->isEqualTo(*desc));
    }

    return glyphs.detach();
}

inline void GrStencilAndCoverTextContext::TextRun::appendGlyph(const SkGlyph& glyph,
                                                               const SkPoint& pos) {
    // Stick the glyphs we can't draw into the fallback arrays.
    if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
        fFallbackIndices.push_back(glyph.getGlyphID());
        fFallbackPositions.push_back(pos);
    } else {
        float translate[] = { fTextInverseRatio * pos.x(), fTextInverseRatio * pos.y() };
        fDraw->append(glyph.getGlyphID(), translate);
    }
}

void GrStencilAndCoverTextContext::TextRun::draw(GrDrawContext* dc,
                                                 GrRenderTarget* rt,
                                                 const GrClip& clip,
                                                 const GrPaint& paint,
                                                 const SkMatrix& viewMatrix,
                                                 const SkIRect& regionClipBounds,
                                                 GrTextContext* fallbackTextContext,
                                                 const SkPaint& originalSkPaint) const {
    SkASSERT(fDraw);

    if (fDraw->count()) {
        GrPipelineBuilder pipelineBuilder(paint, rt, clip);
        SkASSERT(rt->isStencilBufferMultisampled() || !paint.isAntiAlias());
        pipelineBuilder.setState(GrPipelineBuilder::kHWAntialias_Flag, paint.isAntiAlias());

        GR_STATIC_CONST_SAME_STENCIL(kStencilPass,
                                     kZero_StencilOp,
                                     kKeep_StencilOp,
                                     kNotEqual_StencilFunc,
                                     0xffff,
                                     0x0000,
                                     0xffff);

        *pipelineBuilder.stencil() = kStencilPass;

        SkMatrix drawMatrix(viewMatrix);
        drawMatrix.preScale(fTextRatio, fTextRatio);

        dc->drawPathsFromRange(&pipelineBuilder, drawMatrix, fLocalMatrix, paint.getColor(), fDraw,
                               GrPathRendering::kWinding_FillType);
    }

    if (fFallbackIndices.count()) {
        SkASSERT(fFallbackPositions.count() == fFallbackIndices.count());

        enum { kPreservedFlags = SkPaint::kFakeBoldText_Flag | SkPaint::kLinearText_Flag |
                                 SkPaint::kLCDRenderText_Flag | SkPaint::kAutoHinting_Flag };

        SkPaint fallbackSkPaint(originalSkPaint);
        fStroke.applyToPaint(&fallbackSkPaint);
        if (!fStroke.isFillStyle()) {
            fallbackSkPaint.setStrokeWidth(fStroke.getWidth() * fTextRatio);
        }
        fallbackSkPaint.setTextAlign(SkPaint::kLeft_Align); // Align has already been accounted for.
        fallbackSkPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        fallbackSkPaint.setHinting(fFont.getHinting());
        fallbackSkPaint.setFlags((fFont.getFlags() & kPreservedFlags) |
                                 (originalSkPaint.getFlags() & ~kPreservedFlags));
        // No need for subpixel positioning with bitmap glyphs. TODO: revisit if non-bitmap color
        // glyphs show up and https://code.google.com/p/skia/issues/detail?id=4408 gets resolved.
        fallbackSkPaint.setSubpixelText(false);
        fallbackSkPaint.setTextSize(fFont.getTextSize() * fTextRatio);

        fallbackTextContext->drawPosText(dc, rt, clip, paint, fallbackSkPaint, viewMatrix,
                                         (char*)fFallbackIndices.begin(),
                                         sizeof(uint16_t) * fFallbackIndices.count(),
                                         fFallbackPositions[0].asScalars(), 2, SkPoint::Make(0, 0),
                                         regionClipBounds);
    }
}
