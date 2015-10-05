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
#include "SkGrPriv.h"
#include "SkPath.h"
#include "SkTextMapStateProc.h"
#include "SkTextFormatParams.h"

#include "batches/GrDrawPathBatch.h"

template<typename Key, typename Val> static void delete_hash_map_entry(const Key&, Val* val) {
    SkASSERT(*val);
    delete *val;
}

template<typename T> static void delete_hash_table_entry(T* val) {
    SkASSERT(*val);
    delete *val;
}

GrStencilAndCoverTextContext::GrStencilAndCoverTextContext(GrContext* context,
                                                           const SkSurfaceProps& surfaceProps)
    : INHERITED(context, surfaceProps),
      fCacheSize(0) {
}

GrStencilAndCoverTextContext*
GrStencilAndCoverTextContext::Create(GrContext* context, const SkSurfaceProps& surfaceProps) {
    GrStencilAndCoverTextContext* textContext = 
        new GrStencilAndCoverTextContext(context, surfaceProps);
    textContext->fFallbackTextContext = GrAtlasTextContext::Create(context, surfaceProps);

    return textContext;
}

GrStencilAndCoverTextContext::~GrStencilAndCoverTextContext() {
    fBlobIdCache.foreach(delete_hash_map_entry<uint32_t, TextBlob*>);
    fBlobKeyCache.foreach(delete_hash_table_entry<TextBlob*>);
}

bool GrStencilAndCoverTextContext::internalCanDraw(const SkPaint& skPaint) {
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
                                              const SkIRect& clipBounds) {
    TextRun run(skPaint);
    GrPipelineBuilder pipelineBuilder(paint, rt, clip);
    run.setText(text, byteLength, x, y, fContext, &fSurfaceProps);
    run.draw(dc, &pipelineBuilder, paint.getColor(), viewMatrix, 0, 0, clipBounds,
             fFallbackTextContext, skPaint);
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
                                                 const SkIRect& clipBounds) {
    TextRun run(skPaint);
    GrPipelineBuilder pipelineBuilder(paint, rt, clip);
    run.setPosText(text, byteLength, pos, scalarsPerPosition, offset, fContext, &fSurfaceProps);
    run.draw(dc, &pipelineBuilder, paint.getColor(), viewMatrix, 0, 0, clipBounds,
             fFallbackTextContext, skPaint);
}

void GrStencilAndCoverTextContext::drawTextBlob(GrDrawContext* dc, GrRenderTarget* rt,
                                                const GrClip& clip, const SkPaint& skPaint,
                                                const SkMatrix& viewMatrix,
                                                const SkTextBlob* skBlob, SkScalar x, SkScalar y,
                                                SkDrawFilter* drawFilter,
                                                const SkIRect& clipBounds) {
    if (!this->internalCanDraw(skPaint)) {
        fFallbackTextContext->drawTextBlob(dc, rt, clip, skPaint, viewMatrix, skBlob, x, y,
                                           drawFilter, clipBounds);
        return;
    }

    if (drawFilter || skPaint.getPathEffect()) {
        // This draw can't be cached.
        INHERITED::drawTextBlob(dc, rt, clip, skPaint, viewMatrix, skBlob, x, y, drawFilter,
                                clipBounds);
        return;
    }

    if (fContext->abandoned()) {
        return;
    }

    GrPaint paint;
    if (!SkPaintToGrPaint(fContext, skPaint, viewMatrix, &paint)) {
        return;
    }

    const TextBlob& blob = this->findOrCreateTextBlob(skBlob, skPaint);
    GrPipelineBuilder pipelineBuilder(paint, rt, clip);

    TextBlob::Iter iter(blob);
    for (TextRun* run = iter.get(); run; run = iter.next()) {
        run->draw(dc, &pipelineBuilder, paint.getColor(), viewMatrix, x, y, clipBounds,
                  fFallbackTextContext, skPaint);
    }
}

const GrStencilAndCoverTextContext::TextBlob&
GrStencilAndCoverTextContext::findOrCreateTextBlob(const SkTextBlob* skBlob,
                                                   const SkPaint& skPaint) {
    // The font-related parameters are baked into the text blob and will override this skPaint, so
    // the only remaining properties that can affect a TextBlob are the ones related to stroke.
    if (SkPaint::kFill_Style == skPaint.getStyle()) { // Fast path.
        if (TextBlob** found = fBlobIdCache.find(skBlob->uniqueID())) {
            fLRUList.remove(*found);
            fLRUList.addToTail(*found);
            return **found;
        }
        TextBlob* blob = new TextBlob(skBlob->uniqueID(), skBlob, skPaint, fContext,
                                      &fSurfaceProps);
        this->purgeToFit(*blob);
        fBlobIdCache.set(skBlob->uniqueID(), blob);
        fLRUList.addToTail(blob);
        fCacheSize += blob->cpuMemorySize();
        return *blob;
    } else {
        GrStrokeInfo stroke(skPaint);
        SkSTArray<4, uint32_t, true> key;
        key.reset(1 + stroke.computeUniqueKeyFragmentData32Cnt());
        key[0] = skBlob->uniqueID();
        stroke.asUniqueKeyFragment(&key[1]);
        if (TextBlob** found = fBlobKeyCache.find(key)) {
            fLRUList.remove(*found);
            fLRUList.addToTail(*found);
            return **found;
        }
        TextBlob* blob = new TextBlob(key, skBlob, skPaint, fContext, &fSurfaceProps);
        this->purgeToFit(*blob);
        fBlobKeyCache.set(blob);
        fLRUList.addToTail(blob);
        fCacheSize += blob->cpuMemorySize();
        return *blob;
    }
}

void GrStencilAndCoverTextContext::purgeToFit(const TextBlob& blob) {
    static const int maxCacheSize = 4 * 1024 * 1024; // Allow up to 4 MB for caching text blobs.

    int maxSizeForNewBlob = maxCacheSize - blob.cpuMemorySize();
    while (fCacheSize && fCacheSize > maxSizeForNewBlob) {
        TextBlob* lru = fLRUList.head();
        if (1 == lru->key().count()) {
            // 1-length keys are unterstood to be the blob id.
            fBlobIdCache.remove(lru->key()[0]);
        } else {
            fBlobKeyCache.remove(lru->key());
        }
        fLRUList.remove(lru);
        fCacheSize -= lru->cpuMemorySize();
        delete lru;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GrStencilAndCoverTextContext::TextBlob::init(const SkTextBlob* skBlob, const SkPaint& skPaint,
                                                  GrContext* ctx, const SkSurfaceProps* props) {
    fCpuMemorySize = sizeof(TextBlob);
    SkPaint runPaint(skPaint);
    for (SkTextBlob::RunIterator iter(skBlob); !iter.done(); iter.next()) {
        iter.applyFontToPaint(&runPaint); // No need to re-seed the paint.
        TextRun* run = SkNEW_INSERT_AT_LLIST_TAIL(this, TextRun, (runPaint));

        const char* text = reinterpret_cast<const char*>(iter.glyphs());
        size_t byteLength = sizeof(uint16_t) * iter.glyphCount();
        const SkPoint& runOffset = iter.offset();

        switch (iter.positioning()) {
            case SkTextBlob::kDefault_Positioning:
                run->setText(text, byteLength, runOffset.fX, runOffset.fY, ctx, props);
                break;
            case SkTextBlob::kHorizontal_Positioning:
                run->setPosText(text, byteLength, iter.pos(), 1, SkPoint::Make(0, runOffset.fY),
                                ctx, props);
                break;
            case SkTextBlob::kFull_Positioning:
                run->setPosText(text, byteLength, iter.pos(), 2, SkPoint::Make(0, 0), ctx, props);
                break;
        }

        fCpuMemorySize += run->cpuMemorySize();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class GrStencilAndCoverTextContext::FallbackBlobBuilder {
public:
    FallbackBlobBuilder() : fBuffIdx(0) {}

    bool isInitialized() const { return SkToBool(fBuilder); }

    void init(const SkPaint& font, SkScalar textRatio);

    void appendGlyph(uint16_t glyphId, const SkPoint& pos);

    const SkTextBlob* buildIfInitialized();

private:
    enum { kWriteBufferSize = 1024 };

    void flush();

    SkAutoTDelete<SkTextBlobBuilder>   fBuilder;
    SkPaint                            fFont;
    int                                fBuffIdx;
    uint16_t                           fGlyphIds[kWriteBufferSize];
    SkPoint                            fPositions[kWriteBufferSize];
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GrStencilAndCoverTextContext::TextRun::TextRun(const SkPaint& fontAndStroke)
    : fStroke(fontAndStroke),
      fFont(fontAndStroke),
      fTotalGlyphCount(0) {
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
    fLocalMatrixTemplate.setScale(fTextRatio, fTextRatio);
}

GrStencilAndCoverTextContext::TextRun::~TextRun() {
}

void GrStencilAndCoverTextContext::TextRun::setText(const char text[], size_t byteLength,
                                                    SkScalar x, SkScalar y, GrContext* ctx,
                                                    const SkSurfaceProps* surfaceProps) {
    SkASSERT(byteLength == 0 || text != nullptr);

    SkAutoGlyphCacheNoGamma autoGlyphCache(fFont, surfaceProps, nullptr);
    SkGlyphCache* glyphCache = autoGlyphCache.getCache();

    fTotalGlyphCount = fFont.countText(text, byteLength);
    fDraw.reset(GrPathRangeDraw::Create(this->createGlyphs(ctx, glyphCache),
                                        GrPathRendering::kTranslate_PathTransformType,
                                        fTotalGlyphCount));

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
    FallbackBlobBuilder fallback;
    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(glyphCache, &text, 0, 0);
        fx += SkFixedMul(autokern.adjust(glyph), fixedSizeRatio);
        if (glyph.fWidth) {
            this->appendGlyph(glyph, SkPoint::Make(SkFixedToScalar(fx), SkFixedToScalar(fy)),
                              &fallback);
        }

        fx += SkFixedMul(glyph.fAdvanceX, fixedSizeRatio);
        fy += SkFixedMul(glyph.fAdvanceY, fixedSizeRatio);
    }

    fDraw->loadGlyphPathsIfNeeded();

    fFallbackTextBlob.reset(fallback.buildIfInitialized());
}

void GrStencilAndCoverTextContext::TextRun::setPosText(const char text[], size_t byteLength,
                                                       const SkScalar pos[], int scalarsPerPosition,
                                                       const SkPoint& offset, GrContext* ctx,
                                                       const SkSurfaceProps* surfaceProps) {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    SkAutoGlyphCacheNoGamma autoGlyphCache(fFont, surfaceProps, nullptr);
    SkGlyphCache* glyphCache = autoGlyphCache.getCache();

    fTotalGlyphCount = fFont.countText(text, byteLength);
    fDraw.reset(GrPathRangeDraw::Create(this->createGlyphs(ctx, glyphCache),
                                        GrPathRendering::kTranslate_PathTransformType,
                                        fTotalGlyphCount));

    SkDrawCacheProc glyphCacheProc = fFont.getDrawCacheProc();

    const char* stop = text + byteLength;

    SkTextMapStateProc tmsProc(SkMatrix::I(), offset, scalarsPerPosition);
    SkTextAlignProc alignProc(fFont.getTextAlign());
    FallbackBlobBuilder fallback;
    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(glyphCache, &text, 0, 0);
        if (glyph.fWidth) {
            SkPoint tmsLoc;
            tmsProc(pos, &tmsLoc);
            SkPoint loc;
            alignProc(tmsLoc, glyph, &loc);

            this->appendGlyph(glyph, loc, &fallback);
        }
        pos += scalarsPerPosition;
    }

    fDraw->loadGlyphPathsIfNeeded();

    fFallbackTextBlob.reset(fallback.buildIfInitialized());
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
                                                               const SkPoint& pos,
                                                               FallbackBlobBuilder* fallback) {
    // Stick the glyphs we can't draw into the fallback text blob.
    if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
        if (!fallback->isInitialized()) {
            fallback->init(fFont, fTextRatio);
        }
        fallback->appendGlyph(glyph.getGlyphID(), pos);
    } else {
        float translate[] = { fTextInverseRatio * pos.x(), fTextInverseRatio * pos.y() };
        fDraw->append(glyph.getGlyphID(), translate);
    }
}

void GrStencilAndCoverTextContext::TextRun::draw(GrDrawContext* dc,
                                                 GrPipelineBuilder* pipelineBuilder,
                                                 GrColor color,
                                                 const SkMatrix& viewMatrix,
                                                 SkScalar x, SkScalar y,
                                                 const SkIRect& clipBounds,
                                                 GrTextContext* fallbackTextContext,
                                                 const SkPaint& originalSkPaint) const {
    SkASSERT(fDraw);
    SkASSERT(pipelineBuilder->getRenderTarget()->isStencilBufferMultisampled() ||
             !fFont.isAntiAlias());

    if (fDraw->count()) {
        pipelineBuilder->setState(GrPipelineBuilder::kHWAntialias_Flag, fFont.isAntiAlias());

        GR_STATIC_CONST_SAME_STENCIL(kStencilPass,
                                     kZero_StencilOp,
                                     kKeep_StencilOp,
                                     kNotEqual_StencilFunc,
                                     0xffff,
                                     0x0000,
                                     0xffff);

        *pipelineBuilder->stencil() = kStencilPass;

        SkMatrix drawMatrix(viewMatrix);
        drawMatrix.preTranslate(x, y);
        drawMatrix.preScale(fTextRatio, fTextRatio);

        SkMatrix& localMatrix = fLocalMatrixTemplate;
        localMatrix.setTranslateX(x);
        localMatrix.setTranslateY(y);

        dc->drawPathsFromRange(pipelineBuilder, drawMatrix, localMatrix, color, fDraw,
                               GrPathRendering::kWinding_FillType);
    }

    if (fFallbackTextBlob) {
        SkPaint fallbackSkPaint(originalSkPaint);
        fStroke.applyToPaint(&fallbackSkPaint);
        if (!fStroke.isFillStyle()) {
            fallbackSkPaint.setStrokeWidth(fStroke.getWidth() * fTextRatio);
        }

        fallbackTextContext->drawTextBlob(dc, pipelineBuilder->getRenderTarget(),
                                          pipelineBuilder->clip(), fallbackSkPaint, viewMatrix,
                                          fFallbackTextBlob, x, y, nullptr, clipBounds);
    }
}

int GrStencilAndCoverTextContext::TextRun::cpuMemorySize() const {
    int size = sizeof(TextRun) + fTotalGlyphCount * (sizeof(uint16_t) + 2 * sizeof(float));
    if (fDraw) {
        size += sizeof(GrPathRangeDraw);
    }
    if (fFallbackTextBlob) {
        size += sizeof(SkTextBlob);
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GrStencilAndCoverTextContext::FallbackBlobBuilder::init(const SkPaint& font,
                                                             SkScalar textRatio) {
    SkASSERT(!this->isInitialized());
    fBuilder.reset(new SkTextBlobBuilder);
    fFont = font;
    fFont.setTextAlign(SkPaint::kLeft_Align); // The glyph positions will already account for align.
    fFont.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    // No need for subpixel positioning with bitmap glyphs. TODO: revisit if non-bitmap color glyphs
    // show up and https://code.google.com/p/skia/issues/detail?id=4408 gets resolved.
    fFont.setSubpixelText(false);
    fFont.setTextSize(fFont.getTextSize() * textRatio);
    fBuffIdx = 0;
}

void GrStencilAndCoverTextContext::FallbackBlobBuilder::appendGlyph(uint16_t glyphId,
                                                                    const SkPoint& pos) {
    SkASSERT(this->isInitialized());
    if (fBuffIdx >= kWriteBufferSize) {
        this->flush();
    }
    fGlyphIds[fBuffIdx] = glyphId;
    fPositions[fBuffIdx] = pos;
    fBuffIdx++;
}

void GrStencilAndCoverTextContext::FallbackBlobBuilder::flush() {
    SkASSERT(this->isInitialized());
    SkASSERT(fBuffIdx <= kWriteBufferSize);
    if (!fBuffIdx) {
        return;
    }
    // This will automatically merge with previous runs since we use the same font.
    const SkTextBlobBuilder::RunBuffer& buff = fBuilder->allocRunPos(fFont, fBuffIdx);
    memcpy(buff.glyphs, fGlyphIds, fBuffIdx * sizeof(uint16_t));
    memcpy(buff.pos, fPositions[0].asScalars(), fBuffIdx * 2 * sizeof(SkScalar));
    fBuffIdx = 0;
}

const SkTextBlob* GrStencilAndCoverTextContext::FallbackBlobBuilder::buildIfInitialized() {
    if (!this->isInitialized()) {
        return nullptr;
    }
    this->flush();
    return fBuilder->build();
}
