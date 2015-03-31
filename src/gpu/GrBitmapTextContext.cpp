/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrBitmapTextContext.h"

#include "GrAtlas.h"
#include "GrBatch.h"
#include "GrBatchFontCache.h"
#include "GrBatchTarget.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawTarget.h"
#include "GrFontCache.h"
#include "GrFontScaler.h"
#include "GrIndexBuffer.h"
#include "GrStrokeInfo.h"
#include "GrTexturePriv.h"

#include "SkAutoKern.h"
#include "SkColorPriv.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkGr.h"
#include "SkPath.h"
#include "SkRTConf.h"
#include "SkStrokeRec.h"
#include "SkTextBlob.h"
#include "SkTextMapStateProc.h"

#include "effects/GrBitmapTextGeoProc.h"
#include "effects/GrSimpleTextureEffect.h"

SK_CONF_DECLARE(bool, c_DumpFontCache, "gpu.dumpFontCache", false,
                "Dump the contents of the font cache before every purge.");

namespace {
static const size_t kLCDTextVASize = sizeof(SkPoint) + sizeof(SkIPoint16);

// position + local coord
static const size_t kColorTextVASize = sizeof(SkPoint) + sizeof(SkIPoint16);

static const size_t kGrayTextVASize = sizeof(SkPoint) + sizeof(GrColor) + sizeof(SkIPoint16);

static const int kVerticesPerGlyph = 4;
static const int kIndicesPerGlyph = 6;
};

// TODO
// More tests
// move to SkCache
// handle textblobs where the whole run is larger than the cache size
// TODO implement micro speedy hash map for fast refing of glyphs

GrBitmapTextContextB::GrBitmapTextContextB(GrContext* context,
                                         SkGpuDevice* gpuDevice,
                                         const SkDeviceProperties& properties)
                                       : INHERITED(context, gpuDevice, properties) {
    fCurrStrike = NULL;
}

void GrBitmapTextContextB::ClearCacheEntry(uint32_t key, BitmapTextBlob** blob) {
    (*blob)->unref();
}

GrBitmapTextContextB::~GrBitmapTextContextB() {
    fCache.foreach(&GrBitmapTextContextB::ClearCacheEntry);
}

GrBitmapTextContextB* GrBitmapTextContextB::Create(GrContext* context,
                                                 SkGpuDevice* gpuDevice,
                                                 const SkDeviceProperties& props) {
    return SkNEW_ARGS(GrBitmapTextContextB, (context, gpuDevice, props));
}

bool GrBitmapTextContextB::canDraw(const GrRenderTarget*,
                                  const GrClip&,
                                  const GrPaint&,
                                  const SkPaint& skPaint,
                                  const SkMatrix& viewMatrix) {
    return !SkDraw::ShouldDrawTextAsPaths(skPaint, viewMatrix);
}

inline void GrBitmapTextContextB::init(GrRenderTarget* rt, const GrClip& clip,
                                      const GrPaint& paint, const SkPaint& skPaint,
                                      const SkIRect& regionClipBounds) {
    INHERITED::init(rt, clip, paint, skPaint, regionClipBounds);

    fCurrStrike = NULL;
}

bool GrBitmapTextContextB::MustRegenerateBlob(const BitmapTextBlob& blob, const SkPaint& paint,
                                             const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {
    // We always regenerate blobs with patheffects or mask filters we could cache these
    // TODO find some way to cache the maskfilter / patheffects on the textblob
    return !blob.fViewMatrix.cheapEqualTo(viewMatrix) || blob.fX != x || blob.fY != y ||
            paint.getMaskFilter() || paint.getPathEffect() || paint.getStyle() != blob.fStyle;
}

void GrBitmapTextContextB::drawTextBlob(GrRenderTarget* rt, const GrClip& clip,
                                        const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                        const SkTextBlob* blob, SkScalar x, SkScalar y,
                                        SkDrawFilter* drawFilter, const SkIRect& clipBounds) {
    BitmapTextBlob* cacheBlob;
    BitmapTextBlob** foundBlob = fCache.find(blob->uniqueID());

    SkIRect clipRect;
    clip.getConservativeBounds(rt->width(), rt->height(), &clipRect);

    if (foundBlob) {
        cacheBlob = *foundBlob;
        if (MustRegenerateBlob(*cacheBlob, skPaint, viewMatrix, x, y)) {
            // We can get away with reusing the blob if there are no outstanding refs on it.
            // However, we still have to reset all of the runs.
            if (!cacheBlob->unique()) {
                cacheBlob->unref();
                cacheBlob = SkNEW(BitmapTextBlob);
                fCache.set(blob->uniqueID(), cacheBlob);
            }
            this->regenerateTextBlob(cacheBlob, skPaint, viewMatrix, blob, x, y, drawFilter,
                                     clipRect);
        }
    } else {
        cacheBlob = SkNEW(BitmapTextBlob);
        fCache.set(blob->uniqueID(), cacheBlob);
        this->regenerateTextBlob(cacheBlob, skPaint, viewMatrix, blob, x, y, drawFilter, clipRect);
    }

    // Though for the time being runs in the textblob can override the paint, they only touch font
    // info.
    GrPaint grPaint;
    SkPaint2GrPaintShader(fContext, rt, skPaint, viewMatrix, true, &grPaint);

    this->flush(fContext->getTextTarget(), cacheBlob, rt, grPaint, clip, viewMatrix,
                fSkPaint.getAlpha());
}

void GrBitmapTextContextB::regenerateTextBlob(BitmapTextBlob* cacheBlob,
                                              const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                              const SkTextBlob* blob, SkScalar x, SkScalar y,
                                              SkDrawFilter* drawFilter, const SkIRect& clipRect) {
    cacheBlob->fViewMatrix = viewMatrix;
    cacheBlob->fX = x;
    cacheBlob->fY = y;
    cacheBlob->fStyle = skPaint.getStyle();
    cacheBlob->fRuns.reset(blob->fRunCount);

    // Regenerate textblob
    SkPaint runPaint = skPaint;
    SkTextBlob::RunIterator it(blob);
    for (int run = 0; !it.done(); it.next(), run++) {
        size_t textLen = it.glyphCount() * sizeof(uint16_t);
        const SkPoint& offset = it.offset();
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);

        if (drawFilter && !drawFilter->filter(&runPaint, SkDrawFilter::kText_Type)) {
            // A false return from filter() means we should abort the current draw.
            runPaint = skPaint;
            continue;
        }

        runPaint.setFlags(fGpuDevice->filterTextFlags(runPaint));

        switch (it.positioning()) {
            case SkTextBlob::kDefault_Positioning:
                this->internalDrawText(cacheBlob, run, runPaint, viewMatrix,
                                       (const char *)it.glyphs(), textLen,
                                       x + offset.x(), y + offset.y(), clipRect);
                break;
            case SkTextBlob::kHorizontal_Positioning:
                this->internalDrawPosText(cacheBlob, run, runPaint, viewMatrix,
                                          (const char*)it.glyphs(), textLen, it.pos(), 1,
                                          SkPoint::Make(x, y + offset.y()), clipRect);
                break;
            case SkTextBlob::kFull_Positioning:
                this->internalDrawPosText(cacheBlob, run, runPaint, viewMatrix,
                                          (const char*)it.glyphs(), textLen, it.pos(), 2,
                                          SkPoint::Make(x, y), clipRect);
                break;
        }

        if (drawFilter) {
            // A draw filter may change the paint arbitrarily, so we must re-seed in this case.
            runPaint = skPaint;
        }
    }
}

void GrBitmapTextContextB::onDrawText(GrRenderTarget* rt, const GrClip& clip,
                                     const GrPaint& paint, const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix,
                                     const char text[], size_t byteLength,
                                     SkScalar x, SkScalar y, const SkIRect& regionClipBounds) {
    SkAutoTUnref<BitmapTextBlob> blob(SkNEW(BitmapTextBlob));
    blob->fViewMatrix = viewMatrix;
    blob->fX = x;
    blob->fY = y;
    blob->fStyle = skPaint.getStyle();
    blob->fRuns.push_back();

    SkIRect clipRect;
    clip.getConservativeBounds(rt->width(), rt->height(), &clipRect);
    this->internalDrawText(blob, 0, skPaint, viewMatrix, text, byteLength, x, y, clipRect);
    this->flush(fContext->getTextTarget(), blob, rt, paint, clip, viewMatrix, skPaint.getAlpha());
}

void GrBitmapTextContextB::internalDrawText(BitmapTextBlob* blob, int runIndex,
                                            const SkPaint& skPaint,
                                           const SkMatrix& viewMatrix,
                                           const char text[], size_t byteLength,
                                           SkScalar x, SkScalar y, const SkIRect& clipRect) {
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw
    if (text == NULL || byteLength == 0) {
        return;
    }

    fCurrStrike = NULL;
    SkDrawCacheProc glyphCacheProc = skPaint.getDrawCacheProc();

    // Get GrFontScaler from cache
    BitmapTextBlob::Run& run = blob->fRuns[runIndex];
    run.fDescriptor.reset(skPaint.getScalerContextDescriptor(&fDeviceProperties, &viewMatrix,
                                                              false));
    run.fTypeface.reset(SkSafeRef(skPaint.getTypeface()));
    const SkDescriptor* desc = reinterpret_cast<const SkDescriptor*>(run.fDescriptor->data());
    SkGlyphCache* cache = SkGlyphCache::DetachCache(run.fTypeface, desc);
    GrFontScaler* fontScaler = GetGrFontScaler(cache);

    // transform our starting point
    {
        SkPoint loc;
        viewMatrix.mapXY(x, y, &loc);
        x = loc.fX;
        y = loc.fY;
    }

    // need to measure first
    if (skPaint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector    stopVector;
        MeasureText(cache, glyphCacheProc, text, byteLength, &stopVector);

        SkScalar    stopX = stopVector.fX;
        SkScalar    stopY = stopVector.fY;

        if (skPaint.getTextAlign() == SkPaint::kCenter_Align) {
            stopX = SkScalarHalf(stopX);
            stopY = SkScalarHalf(stopY);
        }
        x -= stopX;
        y -= stopY;
    }

    const char* stop = text + byteLength;

    SkAutoKern autokern;

    SkFixed fxMask = ~0;
    SkFixed fyMask = ~0;
    SkScalar halfSampleX, halfSampleY;
    if (cache->isSubpixel()) {
        halfSampleX = halfSampleY = SkFixedToScalar(SkGlyph::kSubpixelRound);
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(viewMatrix);
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
            halfSampleY = SK_ScalarHalf;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
            halfSampleX = SK_ScalarHalf;
        }
    } else {
        halfSampleX = halfSampleY = SK_ScalarHalf;
    }

    Sk48Dot16 fx = SkScalarTo48Dot16(x + halfSampleX);
    Sk48Dot16 fy = SkScalarTo48Dot16(y + halfSampleY);

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text, fx & fxMask, fy & fyMask);

        fx += autokern.adjust(glyph);

        if (glyph.fWidth) {
            this->appendGlyph(blob,
                              runIndex,
                              GrGlyph::Pack(glyph.getGlyphID(),
                                            glyph.getSubXFixed(),
                                            glyph.getSubYFixed(),
                                            GrGlyph::kCoverage_MaskStyle),
                              Sk48Dot16FloorToInt(fx),
                              Sk48Dot16FloorToInt(fy),
                              fontScaler,
                              clipRect);
        }

        fx += glyph.fAdvanceX;
        fy += glyph.fAdvanceY;
    }

    SkGlyphCache::AttachCache(cache);
}

void GrBitmapTextContextB::onDrawPosText(GrRenderTarget* rt, const GrClip& clip,
                                        const GrPaint& paint, const SkPaint& skPaint,
                                        const SkMatrix& viewMatrix,
                                        const char text[], size_t byteLength,
                                        const SkScalar pos[], int scalarsPerPosition,
                                        const SkPoint& offset, const SkIRect& regionClipBounds) {
    SkAutoTUnref<BitmapTextBlob> blob(SkNEW(BitmapTextBlob));
    blob->fStyle = skPaint.getStyle();
    blob->fRuns.push_back();
    blob->fViewMatrix = viewMatrix;

    SkIRect clipRect;
    clip.getConservativeBounds(rt->width(), rt->height(), &clipRect);
    this->internalDrawPosText(blob, 0, skPaint, viewMatrix, text, byteLength, pos,
                              scalarsPerPosition, offset, clipRect);
    this->flush(fContext->getTextTarget(), blob, rt, paint, clip, viewMatrix, fSkPaint.getAlpha());
}

void GrBitmapTextContextB::internalDrawPosText(BitmapTextBlob* blob, int runIndex,
                                               const SkPaint& skPaint,
                                              const SkMatrix& viewMatrix,
                                              const char text[], size_t byteLength,
                                              const SkScalar pos[], int scalarsPerPosition,
                                              const SkPoint& offset, const SkIRect& clipRect) {
    SkASSERT(byteLength == 0 || text != NULL);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == NULL || byteLength == 0) {
        return;
    }

    fCurrStrike = NULL;
    SkDrawCacheProc glyphCacheProc = skPaint.getDrawCacheProc();

    // Get GrFontScaler from cache
    BitmapTextBlob::Run& run = blob->fRuns[runIndex];
    run.fDescriptor.reset(skPaint.getScalerContextDescriptor(&fDeviceProperties, &viewMatrix,
                                                              false));
    run.fTypeface.reset(SkSafeRef(skPaint.getTypeface()));
    const SkDescriptor* desc = reinterpret_cast<const SkDescriptor*>(run.fDescriptor->data());
    SkGlyphCache* cache = SkGlyphCache::DetachCache(run.fTypeface, desc);
    GrFontScaler* fontScaler = GetGrFontScaler(cache);

    const char*        stop = text + byteLength;
    SkTextAlignProc    alignProc(skPaint.getTextAlign());
    SkTextMapStateProc tmsProc(viewMatrix, offset, scalarsPerPosition);
    SkScalar halfSampleX = 0, halfSampleY = 0;

    if (cache->isSubpixel()) {
        // maybe we should skip the rounding if linearText is set
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(viewMatrix);

        SkFixed fxMask = ~0;
        SkFixed fyMask = ~0;
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
            halfSampleY = SK_ScalarHalf;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
            halfSampleX = SK_ScalarHalf;
        }

        if (SkPaint::kLeft_Align == skPaint.getTextAlign()) {
            while (text < stop) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);
                Sk48Dot16 fx = SkScalarTo48Dot16(tmsLoc.fX + halfSampleX);
                Sk48Dot16 fy = SkScalarTo48Dot16(tmsLoc.fY + halfSampleY);

                const SkGlyph& glyph = glyphCacheProc(cache, &text,
                                                      fx & fxMask, fy & fyMask);

                if (glyph.fWidth) {
                    this->appendGlyph(blob,
                                      runIndex,
                                      GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed(),
                                                    GrGlyph::kCoverage_MaskStyle),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
                                      fontScaler,
                                      clipRect);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                const char* currentText = text;
                const SkGlyph& metricGlyph = glyphCacheProc(cache, &text, 0, 0);

                if (metricGlyph.fWidth) {
                    SkDEBUGCODE(SkFixed prevAdvX = metricGlyph.fAdvanceX;)
                    SkDEBUGCODE(SkFixed prevAdvY = metricGlyph.fAdvanceY;)
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);
                    SkPoint alignLoc;
                    alignProc(tmsLoc, metricGlyph, &alignLoc);

                    Sk48Dot16 fx = SkScalarTo48Dot16(alignLoc.fX + halfSampleX);
                    Sk48Dot16 fy = SkScalarTo48Dot16(alignLoc.fY + halfSampleY);

                    // have to call again, now that we've been "aligned"
                    const SkGlyph& glyph = glyphCacheProc(cache, &currentText,
                                                          fx & fxMask, fy & fyMask);
                    // the assumption is that the metrics haven't changed
                    SkASSERT(prevAdvX == glyph.fAdvanceX);
                    SkASSERT(prevAdvY == glyph.fAdvanceY);
                    SkASSERT(glyph.fWidth);

                    this->appendGlyph(blob,
                                      runIndex,
                                      GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed(),
                                                    GrGlyph::kCoverage_MaskStyle),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
                                      fontScaler,
                                      clipRect);
                }
                pos += scalarsPerPosition;
            }
        }
    } else {    // not subpixel

        if (SkPaint::kLeft_Align == skPaint.getTextAlign()) {
            while (text < stop) {
                // the last 2 parameters are ignored
                const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

                if (glyph.fWidth) {
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    Sk48Dot16 fx = SkScalarTo48Dot16(tmsLoc.fX + SK_ScalarHalf); //halfSampleX;
                    Sk48Dot16 fy = SkScalarTo48Dot16(tmsLoc.fY + SK_ScalarHalf); //halfSampleY;
                    this->appendGlyph(blob,
                                      runIndex,
                                      GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed(),
                                                    GrGlyph::kCoverage_MaskStyle),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
                                      fontScaler,
                                      clipRect);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                // the last 2 parameters are ignored
                const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

                if (glyph.fWidth) {
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    SkPoint alignLoc;
                    alignProc(tmsLoc, glyph, &alignLoc);

                    Sk48Dot16 fx = SkScalarTo48Dot16(alignLoc.fX + SK_ScalarHalf); //halfSampleX;
                    Sk48Dot16 fy = SkScalarTo48Dot16(alignLoc.fY + SK_ScalarHalf); //halfSampleY;
                    this->appendGlyph(blob,
                                      runIndex,
                                      GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed(),
                                                    GrGlyph::kCoverage_MaskStyle),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
                                      fontScaler,
                                      clipRect);
                }
                pos += scalarsPerPosition;
            }
        }
    }
    SkGlyphCache::AttachCache(cache);
}

static size_t get_vertex_stride(GrMaskFormat maskFormat) {
    switch (maskFormat) {
        case kA8_GrMaskFormat:
            return kGrayTextVASize;
        case kARGB_GrMaskFormat:
            return kColorTextVASize;
        default:
            return kLCDTextVASize;
    }
}

void GrBitmapTextContextB::appendGlyph(BitmapTextBlob* blob, int runIndex, GrGlyph::PackedID packed,
                                      int vx, int vy, GrFontScaler* scaler,
                                      const SkIRect& clipRect) {
    if (NULL == fCurrStrike) {
        fCurrStrike = fContext->getBatchFontCache()->getStrike(scaler);
    }

    GrGlyph* glyph = fCurrStrike->getGlyph(packed, scaler);
    if (NULL == glyph || glyph->fBounds.isEmpty()) {
        return;
    }

    int x = vx + glyph->fBounds.fLeft;
    int y = vy + glyph->fBounds.fTop;

    // keep them as ints until we've done the clip-test
    int width = glyph->fBounds.width();
    int height = glyph->fBounds.height();

    // check if we clipped out
    if (clipRect.quickReject(x, y, x + width, y + height)) {
        return;
    }

    // If the glyph is too large we fall back to paths
    if (fCurrStrike->glyphTooLargeForAtlas(glyph)) {
        if (NULL == glyph->fPath) {
            SkPath* path = SkNEW(SkPath);
            if (!scaler->getGlyphPath(glyph->glyphID(), path)) {
                // flag the glyph as being dead?
                SkDELETE(path);
                return;
            }
            glyph->fPath = path;
        }
        SkASSERT(glyph->fPath);
        blob->fBigGlyphs.push_back(BitmapTextBlob::BigGlyph(*glyph->fPath, vx, vy));
        return;
    }
    GrMaskFormat format = glyph->fMaskFormat;
    size_t vertexStride = get_vertex_stride(format);

    BitmapTextBlob::Run& run = blob->fRuns[runIndex];
    int glyphIdx = run.fInfos[format].fGlyphIDs.count();
    *run.fInfos[format].fGlyphIDs.append() = packed;
    run.fInfos[format].fVertices.append(static_cast<int>(vertexStride * kVerticesPerGlyph));

    SkRect r;
    r.fLeft = SkIntToScalar(x);
    r.fTop = SkIntToScalar(y);
    r.fRight = r.fLeft + SkIntToScalar(width);
    r.fBottom = r.fTop + SkIntToScalar(height);

    run.fVertexBounds.joinNonEmptyArg(r);
    GrColor color = fPaint.getColor();
    run.fColor = color;

    intptr_t vertex = reinterpret_cast<intptr_t>(run.fInfos[format].fVertices.begin());
    vertex += vertexStride * glyphIdx * kVerticesPerGlyph;

    // V0
    SkPoint* position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fLeft, r.fTop);
    if (kA8_GrMaskFormat == format) {
        SkColor* colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
    }
    vertex += vertexStride;

    // V1
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fLeft, r.fBottom);
    if (kA8_GrMaskFormat == format) {
        SkColor* colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
    }
    vertex += vertexStride;

    // V2
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fRight, r.fBottom);
    if (kA8_GrMaskFormat == format) {
        SkColor* colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
    }
    vertex += vertexStride;

    // V3
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fRight, r.fTop);
    if (kA8_GrMaskFormat == format) {
        SkColor* colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
    }
}

class BitmapTextBatch : public GrBatch {
public:
    typedef GrBitmapTextContextB::BitmapTextBlob Blob;
    typedef Blob::Run Run;
    typedef Run::PerFormatInfo TextInfo;
    struct Geometry {
        Geometry() {}
        Geometry(const Geometry& geometry)
            : fBlob(SkRef(geometry.fBlob.get()))
            , fRun(geometry.fRun)
            , fColor(geometry.fColor) {}
        SkAutoTUnref<Blob> fBlob;
        int fRun;
        GrColor fColor;
    };

    static GrBatch* Create(const Geometry& geometry, GrColor color, GrMaskFormat maskFormat,
                           GrBatchFontCache* fontCache) {
        return SkNEW_ARGS(BitmapTextBatch, (geometry, color, maskFormat, fontCache));
    }

    const char* name() const override { return "BitmapTextBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        if (kARGB_GrMaskFormat == fMaskFormat) {
            out->setUnknownFourComponents();
        } else {
            out->setKnownFourComponents(fBatch.fColor);
        }
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        if (kARGB_GrMaskFormat != fMaskFormat) {
            if (GrPixelConfigIsAlphaOnly(fPixelConfig)) {
                out->setUnknownSingleComponent();
            } else if (GrPixelConfigIsOpaque(fPixelConfig)) {
                out->setUnknownOpaqueFourComponents();
                out->setUsingLCDCoverage();
            } else {
                out->setUnknownFourComponents();
                out->setUsingLCDCoverage();
            }
        } else {
            out->setKnownSingleComponent(0xff);
        }
    }

    void initBatchTracker(const GrPipelineInfo& init) override {
        // Handle any color overrides
        if (init.fColorIgnored) {
            fBatch.fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fBatch.fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) override {
        // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix.
        // TODO actually only invert if we don't have RGBA
        SkMatrix localMatrix;
        if (this->usesLocalCoords() && !this->viewMatrix().invert(&localMatrix)) {
            SkDebugf("Cannot invert viewmatrix\n");
            return;
        }

        GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kNone_FilterMode);
        // This will be ignored in the non A8 case
        bool opaqueVertexColors = GrColorIsOpaque(this->color());
        SkAutoTUnref<const GrGeometryProcessor> gp(
                GrBitmapTextGeoProc::Create(this->color(),
                                            fFontCache->getTexture(fMaskFormat),
                                            params,
                                            fMaskFormat,
                                            opaqueVertexColors,
                                            localMatrix));

        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == get_vertex_stride(fMaskFormat));

        this->initDraw(batchTarget, gp, pipeline);

        int glyphCount = this->numGlyphs();
        int instanceCount = fGeoData.count();
        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void* vertices = batchTarget->vertexPool()->makeSpace(vertexStride,
                                                              glyphCount * kVerticesPerGlyph,
                                                              &vertexBuffer,
                                                              &firstVertex);
        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        unsigned char* currVertex = reinterpret_cast<unsigned char*>(vertices);

        // setup drawinfo
        const GrIndexBuffer* quadIndexBuffer = batchTarget->quadIndexBuffer();
        int maxInstancesPerDraw = quadIndexBuffer->maxQuads();

        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(kTriangles_GrPrimitiveType);
        drawInfo.setStartVertex(0);
        drawInfo.setStartIndex(0);
        drawInfo.setVerticesPerInstance(kVerticesPerGlyph);
        drawInfo.setIndicesPerInstance(kIndicesPerGlyph);
        drawInfo.adjustStartVertex(firstVertex);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setIndexBuffer(quadIndexBuffer);

        int instancesToFlush = 0;
        for (int i = 0; i < instanceCount; i++) {
            Geometry& args = fGeoData[i];
            Blob* blob = args.fBlob;
            Run& run = blob->fRuns[args.fRun];
            TextInfo& info = run.fInfos[fMaskFormat];

            uint64_t currentAtlasGen = fFontCache->atlasGeneration(fMaskFormat);
            bool regenerateTextureCoords = info.fAtlasGeneration != currentAtlasGen;
            bool regenerateColors = kA8_GrMaskFormat == fMaskFormat && run.fColor != args.fColor;
            int glyphCount = info.fGlyphIDs.count();

            // We regenerate both texture coords and colors in the blob itself, and update the
            // atlas generation.  If we don't end up purging any unused plots, we can avoid
            // regenerating the coords.  We could take a finer grained approach to updating texture
            // coords but its not clear if the extra bookkeeping would offset any gains.
            // To avoid looping over the glyphs twice, we do one loop and conditionally update color
            // or coords as needed.  One final note, if we have to break a run for an atlas eviction
            // then we can't really trust the atlas has all of the correct data.  Atlas evictions
            // should be pretty rare, so we just always regenerate in those cases
            if (regenerateTextureCoords || regenerateColors) {
                // first regenerate texture coordinates / colors if need be
                const SkDescriptor* desc = NULL;
                SkGlyphCache* cache = NULL;
                GrFontScaler* scaler = NULL;
                GrBatchTextStrike* strike = NULL;
                bool brokenRun = false;
                if (regenerateTextureCoords) {
                    desc = reinterpret_cast<const SkDescriptor*>(run.fDescriptor->data());
                    cache = SkGlyphCache::DetachCache(run.fTypeface, desc);
                    scaler = GrTextContext::GetGrFontScaler(cache);
                    strike = fFontCache->getStrike(scaler);
                }
                for (int glyphIdx = 0; glyphIdx < glyphCount; glyphIdx++) {
                    GrGlyph::PackedID glyphID = info.fGlyphIDs[glyphIdx];

                    if (regenerateTextureCoords) {
                        // Upload the glyph only if needed
                        GrGlyph* glyph = strike->getGlyph(glyphID, scaler);
                        SkASSERT(glyph);

                        if (!fFontCache->hasGlyph(glyph) &&
                            !strike->addGlyphToAtlas(batchTarget, glyph, scaler)) {
                            this->flush(batchTarget, &drawInfo, instancesToFlush,
                                        maxInstancesPerDraw);
                            this->initDraw(batchTarget, gp, pipeline);
                            instancesToFlush = 0;
                            brokenRun = glyphIdx > 0;

                            SkDEBUGCODE(bool success =) strike->addGlyphToAtlas(batchTarget, glyph,
                                                                                scaler);
                            SkASSERT(success);
                        }

                        fFontCache->setGlyphRefToken(glyph, batchTarget->currentToken());

                        // Texture coords are the last vertex attribute so we get a pointer to the
                        // first one and then map with stride in regenerateTextureCoords
                        intptr_t vertex = reinterpret_cast<intptr_t>(info.fVertices.begin());
                        vertex += vertexStride * glyphIdx * kVerticesPerGlyph;
                        vertex += vertexStride - sizeof(SkIPoint16);

                        this->regenerateTextureCoords(glyph, vertex, vertexStride);
                    }

                    if (regenerateColors) {
                        intptr_t vertex = reinterpret_cast<intptr_t>(info.fVertices.begin());
                        vertex += vertexStride * glyphIdx * kVerticesPerGlyph + sizeof(SkPoint);
                        this->regenerateColors(vertex, vertexStride, args.fColor);
                    }

                    instancesToFlush++;
                }

                if (regenerateTextureCoords) {
                    SkGlyphCache::AttachCache(cache);
                    info.fAtlasGeneration = brokenRun ? GrBatchAtlas::kInvalidAtlasGeneration :
                                                        fFontCache->atlasGeneration(fMaskFormat);
                }
            } else {
                instancesToFlush += glyphCount;
            }

            // now copy all vertices
            int byteCount = info.fVertices.count();
            memcpy(currVertex, info.fVertices.begin(), byteCount);

            currVertex += byteCount;
        }

        this->flush(batchTarget, &drawInfo, instancesToFlush, maxInstancesPerDraw);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    BitmapTextBatch(const Geometry& geometry, GrColor color, GrMaskFormat maskFormat,
                    GrBatchFontCache* fontCache)
            : fMaskFormat(maskFormat)
            , fPixelConfig(fontCache->getPixelConfig(maskFormat))
            , fFontCache(fontCache) {
        this->initClassID<BitmapTextBatch>();
        fGeoData.push_back(geometry);
        fBatch.fColor = color;
        fBatch.fViewMatrix = geometry.fBlob->fViewMatrix;
        int numGlyphs = geometry.fBlob->fRuns[geometry.fRun].fInfos[maskFormat].fGlyphIDs.count();
        fBatch.fNumGlyphs = numGlyphs;
    }

    void regenerateTextureCoords(GrGlyph* glyph, intptr_t vertex, size_t vertexStride) {
        int width = glyph->fBounds.width();
        int height = glyph->fBounds.height();
        int u0 = glyph->fAtlasLocation.fX;
        int v0 = glyph->fAtlasLocation.fY;
        int u1 = u0 + width;
        int v1 = v0 + height;

        // we assume texture coords are the last vertex attribute, this is a bit fragile.
        // TODO pass in this offset or something
        SkIPoint16* textureCoords;
        // V0
        textureCoords = reinterpret_cast<SkIPoint16*>(vertex);
        textureCoords->set(u0, v0);
        vertex += vertexStride;

        // V1
        textureCoords = reinterpret_cast<SkIPoint16*>(vertex);
        textureCoords->set(u0, v1);
        vertex += vertexStride;

        // V2
        textureCoords = reinterpret_cast<SkIPoint16*>(vertex);
        textureCoords->set(u1, v1);
        vertex += vertexStride;

        // V3
        textureCoords = reinterpret_cast<SkIPoint16*>(vertex);
        textureCoords->set(u1, v0);
    }

    void regenerateColors(intptr_t vertex, size_t vertexStride, GrColor color) {
        for (int i = 0; i < kVerticesPerGlyph; i++) {
            SkColor* vcolor = reinterpret_cast<SkColor*>(vertex);
            *vcolor = color;
            vertex += vertexStride;
        }
    }

    void initDraw(GrBatchTarget* batchTarget,
                  const GrGeometryProcessor* gp,
                  const GrPipeline* pipeline) {
        batchTarget->initDraw(gp, pipeline);

        // TODO remove this when batch is everywhere
        GrPipelineInfo init;
        init.fColorIgnored = fBatch.fColorIgnored;
        init.fOverrideColor = GrColor_ILLEGAL;
        init.fCoverageIgnored = fBatch.fCoverageIgnored;
        init.fUsesLocalCoords = this->usesLocalCoords();
        gp->initBatchTracker(batchTarget->currentBatchTracker(), init);
    }

    void flush(GrBatchTarget* batchTarget,
               GrDrawTarget::DrawInfo* drawInfo,
               int instanceCount,
               int maxInstancesPerDraw) {
        while (instanceCount) {
            drawInfo->setInstanceCount(SkTMin(instanceCount, maxInstancesPerDraw));
            drawInfo->setVertexCount(drawInfo->instanceCount() * drawInfo->verticesPerInstance());
            drawInfo->setIndexCount(drawInfo->instanceCount() * drawInfo->indicesPerInstance());

            batchTarget->draw(*drawInfo);

            drawInfo->setStartVertex(drawInfo->startVertex() + drawInfo->vertexCount());
            instanceCount -= drawInfo->instanceCount();
       }
    }

    GrColor color() const { return fBatch.fColor; }
    const SkMatrix& viewMatrix() const { return fBatch.fViewMatrix; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    int numGlyphs() const { return fBatch.fNumGlyphs; }

    bool onCombineIfPossible(GrBatch* t) override {
        BitmapTextBatch* that = t->cast<BitmapTextBatch>();

        if (this->fMaskFormat != that->fMaskFormat) {
            return false;
        }

        if (this->fMaskFormat != kA8_GrMaskFormat && this->color() != that->color()) {
            return false;
        }

        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fBatch.fNumGlyphs += that->numGlyphs();
        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        return true;
    }

    struct BatchTracker {
        GrColor fColor;
        SkMatrix fViewMatrix;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        int fNumGlyphs;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
    GrMaskFormat fMaskFormat;
    GrPixelConfig fPixelConfig;
    GrBatchFontCache* fFontCache;
};

void GrBitmapTextContextB::flushSubRun(GrDrawTarget* target, BitmapTextBlob* blob, int i,
                                       GrPipelineBuilder* pipelineBuilder, GrMaskFormat format,
                                       GrColor color, int paintAlpha) {
    if (0 == blob->fRuns[i].fInfos[format].fGlyphIDs.count()) {
        return;
    }

    if (kARGB_GrMaskFormat == format) {
        color = SkColorSetARGB(paintAlpha, paintAlpha, paintAlpha, paintAlpha);
    }

    BitmapTextBatch::Geometry geometry;
    geometry.fBlob.reset(SkRef(blob));
    geometry.fRun = i;
    geometry.fColor = color;
    SkAutoTUnref<GrBatch> batch(BitmapTextBatch::Create(geometry, color, format,
                                                        fContext->getBatchFontCache()));

    target->drawBatch(pipelineBuilder, batch, &blob->fRuns[i].fVertexBounds);
}

void GrBitmapTextContextB::flush(GrDrawTarget* target, BitmapTextBlob* blob, GrRenderTarget* rt,
                                 const GrPaint& paint, const GrClip& clip,
                                 const SkMatrix& viewMatrix, int paintAlpha) {
    GrPipelineBuilder pipelineBuilder;
    pipelineBuilder.setFromPaint(paint, rt, clip);

    GrColor color = paint.getColor();
    for (int i = 0; i < blob->fRuns.count(); i++) {
        this->flushSubRun(target, blob, i, &pipelineBuilder, kA8_GrMaskFormat, color, paintAlpha);
        this->flushSubRun(target, blob, i, &pipelineBuilder, kA565_GrMaskFormat, color, paintAlpha);
        this->flushSubRun(target, blob, i, &pipelineBuilder, kARGB_GrMaskFormat, color, paintAlpha);
    }

    // Now flush big glyphs
    for (int i = 0; i < blob->fBigGlyphs.count(); i++) {
        BitmapTextBlob::BigGlyph& bigGlyph = blob->fBigGlyphs[i];
        SkMatrix translate;
        translate.setTranslate(SkIntToScalar(bigGlyph.fVx), SkIntToScalar(bigGlyph.fVy));
        SkPath tmpPath(bigGlyph.fPath);
        tmpPath.transform(translate);
        GrStrokeInfo strokeInfo(SkStrokeRec::kFill_InitStyle);
        fContext->drawPath(rt, clip, paint, SkMatrix::I(), tmpPath, strokeInfo);
    }
}

GrBitmapTextContext::GrBitmapTextContext(GrContext* context,
                                         SkGpuDevice* gpuDevice,
                                         const SkDeviceProperties& properties)
    : GrTextContext(context, gpuDevice, properties) {
    fStrike = NULL;

    fCurrTexture = NULL;
    fEffectTextureUniqueID = SK_InvalidUniqueID;

    fVertices = NULL;
    fCurrVertex = 0;
    fAllocVertexCount = 0;
    fTotalVertexCount = 0;

    fVertexBounds.setLargestInverted();
}

GrBitmapTextContext* GrBitmapTextContext::Create(GrContext* context,
                                                 SkGpuDevice* gpuDevice,
                                                 const SkDeviceProperties& props) {
    return SkNEW_ARGS(GrBitmapTextContext, (context, gpuDevice, props));
}

bool GrBitmapTextContext::canDraw(const GrRenderTarget* rt,
                                  const GrClip& clip,
                                  const GrPaint& paint,
                                  const SkPaint& skPaint,
                                  const SkMatrix& viewMatrix) {
    return !SkDraw::ShouldDrawTextAsPaths(skPaint, viewMatrix);
}

inline void GrBitmapTextContext::init(GrRenderTarget* rt, const GrClip& clip,
                                      const GrPaint& paint, const SkPaint& skPaint,
                                      const SkIRect& regionClipBounds) {
    GrTextContext::init(rt, clip, paint, skPaint, regionClipBounds);

    fStrike = NULL;

    fCurrTexture = NULL;
    fCurrVertex = 0;

    fVertices = NULL;
    fAllocVertexCount = 0;
    fTotalVertexCount = 0;
}

void GrBitmapTextContext::onDrawText(GrRenderTarget* rt, const GrClip& clip,
                                     const GrPaint& paint, const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix,
                                     const char text[], size_t byteLength,
                                     SkScalar x, SkScalar y, const SkIRect& regionClipBounds) {
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw
    if (text == NULL || byteLength == 0 /*|| fRC->isEmpty()*/) {
        return;
    }

    this->init(rt, clip, paint, skPaint, regionClipBounds);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCache    autoCache(fSkPaint, &fDeviceProperties, &viewMatrix);
    SkGlyphCache*       cache = autoCache.getCache();
    GrFontScaler*       fontScaler = GetGrFontScaler(cache);

    // transform our starting point
    {
        SkPoint loc;
        viewMatrix.mapXY(x, y, &loc);
        x = loc.fX;
        y = loc.fY;
    }

    // need to measure first
    int numGlyphs;
    if (fSkPaint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector    stopVector;
        numGlyphs = MeasureText(cache, glyphCacheProc, text, byteLength, &stopVector);

        SkScalar    stopX = stopVector.fX;
        SkScalar    stopY = stopVector.fY;

        if (fSkPaint.getTextAlign() == SkPaint::kCenter_Align) {
            stopX = SkScalarHalf(stopX);
            stopY = SkScalarHalf(stopY);
        }
        x -= stopX;
        y -= stopY;
    } else {
        numGlyphs = fSkPaint.textToGlyphs(text, byteLength, NULL);
    }
    fTotalVertexCount = kVerticesPerGlyph*numGlyphs;

    const char* stop = text + byteLength;

    SkAutoKern autokern;

    SkFixed fxMask = ~0;
    SkFixed fyMask = ~0;
    SkScalar halfSampleX, halfSampleY;
    if (cache->isSubpixel()) {
        halfSampleX = halfSampleY = SkFixedToScalar(SkGlyph::kSubpixelRound);
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(viewMatrix);
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
            halfSampleY = SK_ScalarHalf;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
            halfSampleX = SK_ScalarHalf;
        }
    } else {
        halfSampleX = halfSampleY = SK_ScalarHalf;
    }

    Sk48Dot16 fx = SkScalarTo48Dot16(x + halfSampleX);
    Sk48Dot16 fy = SkScalarTo48Dot16(y + halfSampleY);

    // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix, but for
    // performance reasons we just invert here instead
    if (!viewMatrix.invert(&fLocalMatrix)) {
        SkDebugf("Cannot invert viewmatrix\n");
        return;
    }

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text, fx & fxMask, fy & fyMask);

        fx += autokern.adjust(glyph);

        if (glyph.fWidth) {
            this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                            glyph.getSubXFixed(),
                                            glyph.getSubYFixed(),
                                            GrGlyph::kCoverage_MaskStyle),
                              Sk48Dot16FloorToInt(fx),
                              Sk48Dot16FloorToInt(fy),
                              fontScaler);
        }

        fx += glyph.fAdvanceX;
        fy += glyph.fAdvanceY;
    }

    this->finish();
}

void GrBitmapTextContext::onDrawPosText(GrRenderTarget* rt, const GrClip& clip,
                                        const GrPaint& paint, const SkPaint& skPaint,
                                        const SkMatrix& viewMatrix,
                                        const char text[], size_t byteLength,
                                        const SkScalar pos[], int scalarsPerPosition,
                                        const SkPoint& offset, const SkIRect& regionClipBounds) {
    SkASSERT(byteLength == 0 || text != NULL);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == NULL || byteLength == 0/* || fRC->isEmpty()*/) {
        return;
    }

    this->init(rt, clip, paint, skPaint, regionClipBounds);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCache    autoCache(fSkPaint, &fDeviceProperties, &viewMatrix);
    SkGlyphCache*       cache = autoCache.getCache();
    GrFontScaler*       fontScaler = GetGrFontScaler(cache);

    // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix, but for
    // performance reasons we just invert here instead
    if (!viewMatrix.invert(&fLocalMatrix)) {
        SkDebugf("Cannot invert viewmatrix\n");
        return;
    }

    int numGlyphs = fSkPaint.textToGlyphs(text, byteLength, NULL);
    fTotalVertexCount = kVerticesPerGlyph*numGlyphs;

    const char*        stop = text + byteLength;
    SkTextAlignProc    alignProc(fSkPaint.getTextAlign());
    SkTextMapStateProc tmsProc(viewMatrix, offset, scalarsPerPosition);
    SkScalar halfSampleX = 0, halfSampleY = 0;

    if (cache->isSubpixel()) {
        // maybe we should skip the rounding if linearText is set
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(viewMatrix);

        SkFixed fxMask = ~0;
        SkFixed fyMask = ~0;
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
            halfSampleY = SK_ScalarHalf;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
            halfSampleX = SK_ScalarHalf;
        }

        if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
            while (text < stop) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);
                Sk48Dot16 fx = SkScalarTo48Dot16(tmsLoc.fX + halfSampleX);
                Sk48Dot16 fy = SkScalarTo48Dot16(tmsLoc.fY + halfSampleY);

                const SkGlyph& glyph = glyphCacheProc(cache, &text,
                                                      fx & fxMask, fy & fyMask);

                if (glyph.fWidth) {
                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed(),
                                                    GrGlyph::kCoverage_MaskStyle),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
                                      fontScaler);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                const char* currentText = text;
                const SkGlyph& metricGlyph = glyphCacheProc(cache, &text, 0, 0);

                if (metricGlyph.fWidth) {
                    SkDEBUGCODE(SkFixed prevAdvX = metricGlyph.fAdvanceX;)
                    SkDEBUGCODE(SkFixed prevAdvY = metricGlyph.fAdvanceY;)
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);
                    SkPoint alignLoc;
                    alignProc(tmsLoc, metricGlyph, &alignLoc);

                    Sk48Dot16 fx = SkScalarTo48Dot16(alignLoc.fX + halfSampleX);
                    Sk48Dot16 fy = SkScalarTo48Dot16(alignLoc.fY + halfSampleY);

                    // have to call again, now that we've been "aligned"
                    const SkGlyph& glyph = glyphCacheProc(cache, &currentText,
                                                          fx & fxMask, fy & fyMask);
                    // the assumption is that the metrics haven't changed
                    SkASSERT(prevAdvX == glyph.fAdvanceX);
                    SkASSERT(prevAdvY == glyph.fAdvanceY);
                    SkASSERT(glyph.fWidth);

                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed(),
                                                    GrGlyph::kCoverage_MaskStyle),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
                                      fontScaler);
                }
                pos += scalarsPerPosition;
            }
        }
    } else {    // not subpixel

        if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
            while (text < stop) {
                // the last 2 parameters are ignored
                const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

                if (glyph.fWidth) {
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    Sk48Dot16 fx = SkScalarTo48Dot16(tmsLoc.fX + SK_ScalarHalf); //halfSampleX;
                    Sk48Dot16 fy = SkScalarTo48Dot16(tmsLoc.fY + SK_ScalarHalf); //halfSampleY;
                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed(),
                                                    GrGlyph::kCoverage_MaskStyle),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
                                      fontScaler);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                // the last 2 parameters are ignored
                const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

                if (glyph.fWidth) {
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    SkPoint alignLoc;
                    alignProc(tmsLoc, glyph, &alignLoc);

                    Sk48Dot16 fx = SkScalarTo48Dot16(alignLoc.fX + SK_ScalarHalf); //halfSampleX;
                    Sk48Dot16 fy = SkScalarTo48Dot16(alignLoc.fY + SK_ScalarHalf); //halfSampleY;
                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed(),
                                                    GrGlyph::kCoverage_MaskStyle),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
                                      fontScaler);
                }
                pos += scalarsPerPosition;
            }
        }
    }

    this->finish();
}

static void* alloc_vertices(GrDrawTarget* drawTarget,
                            int numVertices,
                            GrMaskFormat maskFormat) {
    if (numVertices <= 0) {
        return NULL;
    }

    // set up attributes
    void* vertices = NULL;
    bool success = drawTarget->reserveVertexAndIndexSpace(numVertices,
                                                          get_vertex_stride(maskFormat),
                                                          0,
                                                          &vertices,
                                                          NULL);
    GrAlwaysAssert(success);
    return vertices;
}

inline bool GrBitmapTextContext::uploadGlyph(GrGlyph* glyph, GrFontScaler* scaler) {
    if (!fStrike->glyphTooLargeForAtlas(glyph)) {
        if (fStrike->addGlyphToAtlas(glyph, scaler)) {
            return true;
        }

        // try to clear out an unused plot before we flush
        if (fContext->getFontCache()->freeUnusedPlot(fStrike, glyph) &&
            fStrike->addGlyphToAtlas(glyph, scaler)) {
            return true;
        }

        if (c_DumpFontCache) {
#ifdef SK_DEVELOPER
            fContext->getFontCache()->dump();
#endif
        }

        // before we purge the cache, we must flush any accumulated draws
        this->flush();
        fContext->flush();

        // we should have an unused plot now
        if (fContext->getFontCache()->freeUnusedPlot(fStrike, glyph) &&
            fStrike->addGlyphToAtlas(glyph, scaler)) {
            return true;
        }

        // we should never get here
        SkASSERT(false);
    }

    return false;
}

void GrBitmapTextContext::appendGlyph(GrGlyph::PackedID packed,
                                      int vx, int vy,
                                      GrFontScaler* scaler) {
    if (NULL == fDrawTarget) {
        return;
    }

    if (NULL == fStrike) {
        fStrike = fContext->getFontCache()->getStrike(scaler);
    }

    GrGlyph* glyph = fStrike->getGlyph(packed, scaler);
    if (NULL == glyph || glyph->fBounds.isEmpty()) {
        return;
    }

    int x = vx + glyph->fBounds.fLeft;
    int y = vy + glyph->fBounds.fTop;

    // keep them as ints until we've done the clip-test
    int width = glyph->fBounds.width();
    int height = glyph->fBounds.height();

    // check if we clipped out
    if (fClipRect.quickReject(x, y, x + width, y + height)) {
        return;
    }

    // If the glyph is too large we fall back to paths
    if (NULL == glyph->fPlot && !uploadGlyph(glyph, scaler)) {
        if (NULL == glyph->fPath) {
            SkPath* path = SkNEW(SkPath);
            if (!scaler->getGlyphPath(glyph->glyphID(), path)) {
                // flag the glyph as being dead?
                delete path;
                return;
            }
            glyph->fPath = path;
        }

        // flush any accumulated draws before drawing this glyph as a path.
        this->flush();

        SkMatrix translate;
        translate.setTranslate(SkIntToScalar(vx), SkIntToScalar(vy));
        SkPath tmpPath(*glyph->fPath);
        tmpPath.transform(translate);
        GrStrokeInfo strokeInfo(SkStrokeRec::kFill_InitStyle);
        fContext->drawPath(fRenderTarget, fClip, fPaint, SkMatrix::I(), tmpPath, strokeInfo);

        // remove this glyph from the vertices we need to allocate
        fTotalVertexCount -= kVerticesPerGlyph;
        return;
    }

    SkASSERT(glyph->fPlot);
    GrDrawTarget::DrawToken drawToken = fDrawTarget->getCurrentDrawToken();
    glyph->fPlot->setDrawToken(drawToken);

    // the current texture/maskformat must match what the glyph needs
    GrTexture* texture = glyph->fPlot->texture();
    SkASSERT(texture);

    if (fCurrTexture != texture || fCurrVertex + kVerticesPerGlyph > fAllocVertexCount) {
        this->flush();
        fCurrTexture = texture;
        fCurrTexture->ref();
        fCurrMaskFormat = glyph->fMaskFormat;
    }

    if (NULL == fVertices) {
        int maxQuadVertices = kVerticesPerGlyph * fContext->getQuadIndexBuffer()->maxQuads();
        fAllocVertexCount = SkMin32(fTotalVertexCount, maxQuadVertices);
        fVertices = alloc_vertices(fDrawTarget, fAllocVertexCount, fCurrMaskFormat);
    }

    SkRect r;
    r.fLeft = SkIntToScalar(x);
    r.fTop = SkIntToScalar(y);
    r.fRight = r.fLeft + SkIntToScalar(width);
    r.fBottom = r.fTop + SkIntToScalar(height);

    fVertexBounds.joinNonEmptyArg(r);

    int u0 = glyph->fAtlasLocation.fX;
    int v0 = glyph->fAtlasLocation.fY;
    int u1 = u0 + width;
    int v1 = v0 + height;

    size_t vertSize = get_vertex_stride(fCurrMaskFormat);
    intptr_t vertex = reinterpret_cast<intptr_t>(fVertices) + vertSize * fCurrVertex;

    // V0
    SkPoint* position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fLeft, r.fTop);
    if (kA8_GrMaskFormat == fCurrMaskFormat) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    SkIPoint16* textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize -
                                                              sizeof(SkIPoint16));
    textureCoords->set(u0, v0);
    vertex += vertSize;

    // V1
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fLeft, r.fBottom);
    if (kA8_GrMaskFormat == fCurrMaskFormat) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize  - sizeof(SkIPoint16));
    textureCoords->set(u0, v1);
    vertex += vertSize;

    // V2
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fRight, r.fBottom);
    if (kA8_GrMaskFormat == fCurrMaskFormat) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize  - sizeof(SkIPoint16));
    textureCoords->set(u1, v1);
    vertex += vertSize;

    // V3
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fRight, r.fTop);
    if (kA8_GrMaskFormat == fCurrMaskFormat) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize  - sizeof(SkIPoint16));
    textureCoords->set(u1, v0);

    fCurrVertex += 4;
}

void GrBitmapTextContext::flush() {
    if (NULL == fDrawTarget) {
        return;
    }

    if (fCurrVertex > 0) {
        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setFromPaint(fPaint, fRenderTarget, fClip);

        // setup our sampler state for our text texture/atlas
        SkASSERT(SkIsAlign4(fCurrVertex));
        SkASSERT(fCurrTexture);

        SkASSERT(fStrike);
        GrColor color = fPaint.getColor();
        switch (fCurrMaskFormat) {
                // Color bitmap text
            case kARGB_GrMaskFormat: {
                int a = fSkPaint.getAlpha();
                color = SkColorSetARGB(a, a, a, a);
                break;
            }
                // LCD text
            case kA565_GrMaskFormat: {
                // TODO: move supportsRGBCoverage check to setupCoverageEffect and only add LCD
                // processor if the xp can support it. For now we will simply assume that if
                // fUseLCDText is true, then we have a known color output.
                const GrXPFactory* xpFactory = pipelineBuilder.getXPFactory();
                if (!xpFactory->supportsRGBCoverage(0, kRGBA_GrColorComponentFlags)) {
                    SkDebugf("LCD Text will not draw correctly.\n");
                }
                break;
            }
                // Grayscale/BW text
            case kA8_GrMaskFormat:
                break;
            default:
                SkFAIL("Unexpected mask format.");
        }

        GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kNone_FilterMode);
        uint32_t textureUniqueID = fCurrTexture->getUniqueID();
        if (textureUniqueID != fEffectTextureUniqueID ||
            fCachedGeometryProcessor->color() != color ||
            !fCachedGeometryProcessor->localMatrix().cheapEqualTo(fLocalMatrix)) {
            // This will be ignored in the non A8 case
            bool opaqueVertexColors = GrColorIsOpaque(fPaint.getColor());
            fCachedGeometryProcessor.reset(GrBitmapTextGeoProc::Create(color,
                                                                       fCurrTexture,
                                                                       params,
                                                                       fCurrMaskFormat,
                                                                       opaqueVertexColors,
                                                                       fLocalMatrix));
            fEffectTextureUniqueID = textureUniqueID;
        }

        int nGlyphs = fCurrVertex / kVerticesPerGlyph;
        fDrawTarget->setIndexSourceToBuffer(fContext->getQuadIndexBuffer());
        fDrawTarget->drawIndexedInstances(&pipelineBuilder,
                                          fCachedGeometryProcessor.get(),
                                          kTriangles_GrPrimitiveType,
                                          nGlyphs,
                                          kVerticesPerGlyph,
                                          kIndicesPerGlyph,
                                          &fVertexBounds);

        fDrawTarget->resetVertexSource();
        fVertices = NULL;
        fAllocVertexCount = 0;
        // reset to be those that are left
        fTotalVertexCount -= fCurrVertex;
        fCurrVertex = 0;
        fVertexBounds.setLargestInverted();
        SkSafeSetNull(fCurrTexture);
    }
}

inline void GrBitmapTextContext::finish() {
    this->flush();
    fTotalVertexCount = 0;

    GrTextContext::finish();
}
