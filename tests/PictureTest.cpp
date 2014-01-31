/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkDecodingImageGenerator.h"
#include "SkError.h"
#include "SkImageEncoder.h"
#include "SkImageGenerator.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureUtils.h"
#include "SkRRect.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkStream.h"
#include "Test.h"

static const int gColorScale = 30;
static const int gColorOffset = 60;

static void make_bm(SkBitmap* bm, int w, int h, SkColor color, bool immutable) {
    bm->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bm->allocPixels();
    bm->eraseColor(color);
    if (immutable) {
        bm->setImmutable();
    }
}

static void make_checkerboard(SkBitmap* bm, int w, int h, bool immutable) {
    SkASSERT(w % 2 == 0);
    SkASSERT(h % 2 == 0);
    bm->setConfig(SkBitmap::kA8_Config, w, h);
    bm->allocPixels();
    SkAutoLockPixels lock(*bm);
    for (int y = 0; y < h; y += 2) {
        uint8_t* s = bm->getAddr8(0, y);
        for (int x = 0; x < w; x += 2) {
            *s++ = 0xFF;
            *s++ = 0x00;
        }
        s = bm->getAddr8(0, y + 1);
        for (int x = 0; x < w; x += 2) {
            *s++ = 0x00;
            *s++ = 0xFF;
        }
    }
    if (immutable) {
        bm->setImmutable();
    }
}

static void init_paint(SkPaint* paint, const SkBitmap &bm) {
    SkShader* shader = SkShader::CreateBitmapShader(bm,
                                                    SkShader::kClamp_TileMode,
                                                    SkShader::kClamp_TileMode);
    paint->setShader(shader)->unref();
}

typedef void (*DrawBitmapProc)(SkCanvas*, const SkBitmap&,
                               const SkBitmap&, const SkPoint&,
                               SkTDArray<SkPixelRef*>* usedPixRefs);

static void drawpaint_proc(SkCanvas* canvas, const SkBitmap& bm,
                           const SkBitmap& altBM, const SkPoint& pos,
                           SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    canvas->drawPaint(paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawpoints_proc(SkCanvas* canvas, const SkBitmap& bm,
                            const SkBitmap& altBM, const SkPoint& pos,
                            SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    // draw a rect
    SkPoint points[5] = {
        { pos.fX, pos.fY },
        { pos.fX + bm.width() - 1, pos.fY },
        { pos.fX + bm.width() - 1, pos.fY + bm.height() - 1 },
        { pos.fX, pos.fY + bm.height() - 1 },
        { pos.fX, pos.fY },
    };

    canvas->drawPoints(SkCanvas::kPolygon_PointMode, 5, points, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawrect_proc(SkCanvas* canvas, const SkBitmap& bm,
                          const SkBitmap& altBM, const SkPoint& pos,
                          SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkRect r = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
    r.offset(pos.fX, pos.fY);

    canvas->drawRect(r, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawoval_proc(SkCanvas* canvas, const SkBitmap& bm,
                          const SkBitmap& altBM, const SkPoint& pos,
                          SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkRect r = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
    r.offset(pos.fX, pos.fY);

    canvas->drawOval(r, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawrrect_proc(SkCanvas* canvas, const SkBitmap& bm,
                           const SkBitmap& altBM, const SkPoint& pos,
                           SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkRect r = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
    r.offset(pos.fX, pos.fY);

    SkRRect rr;
    rr.setRectXY(r, SkIntToScalar(bm.width())/4, SkIntToScalar(bm.height())/4);
    canvas->drawRRect(rr, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawpath_proc(SkCanvas* canvas, const SkBitmap& bm,
                          const SkBitmap& altBM, const SkPoint& pos,
                          SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkPath path;
    path.lineTo(bm.width()/2.0f, SkIntToScalar(bm.height()));
    path.lineTo(SkIntToScalar(bm.width()), 0);
    path.close();
    path.offset(pos.fX, pos.fY);

    canvas->drawPath(path, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawbitmap_proc(SkCanvas* canvas, const SkBitmap& bm,
                            const SkBitmap& altBM, const SkPoint& pos,
                            SkTDArray<SkPixelRef*>* usedPixRefs) {
    canvas->drawBitmap(bm, pos.fX, pos.fY, NULL);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawbitmap_withshader_proc(SkCanvas* canvas, const SkBitmap& bm,
                                       const SkBitmap& altBM, const SkPoint& pos,
                                       SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    // The bitmap in the paint is ignored unless we're drawing an A8 bitmap
    canvas->drawBitmap(altBM, pos.fX, pos.fY, &paint);
    *usedPixRefs->append() = bm.pixelRef();
    *usedPixRefs->append() = altBM.pixelRef();
}

static void drawsprite_proc(SkCanvas* canvas, const SkBitmap& bm,
                            const SkBitmap& altBM, const SkPoint& pos,
                            SkTDArray<SkPixelRef*>* usedPixRefs) {
    const SkMatrix& ctm = canvas->getTotalMatrix();

    SkPoint p(pos);
    ctm.mapPoints(&p, 1);

    canvas->drawSprite(bm, (int)p.fX, (int)p.fY, NULL);
    *usedPixRefs->append() = bm.pixelRef();
}

#if 0
// Although specifiable, this case doesn't seem to make sense (i.e., the
// bitmap in the shader is never used).
static void drawsprite_withshader_proc(SkCanvas* canvas, const SkBitmap& bm,
                                       const SkBitmap& altBM, const SkPoint& pos,
                                       SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    const SkMatrix& ctm = canvas->getTotalMatrix();

    SkPoint p(pos);
    ctm.mapPoints(&p, 1);

    canvas->drawSprite(altBM, (int)p.fX, (int)p.fY, &paint);
    *usedPixRefs->append() = bm.pixelRef();
    *usedPixRefs->append() = altBM.pixelRef();
}
#endif

static void drawbitmaprect_proc(SkCanvas* canvas, const SkBitmap& bm,
                                const SkBitmap& altBM, const SkPoint& pos,
                                SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkRect r = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };

    r.offset(pos.fX, pos.fY);
    canvas->drawBitmapRectToRect(bm, NULL, r, NULL);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawbitmaprect_withshader_proc(SkCanvas* canvas,
                                           const SkBitmap& bm,
                                           const SkBitmap& altBM,
                                           const SkPoint& pos,
                                           SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkRect r = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
    r.offset(pos.fX, pos.fY);

    // The bitmap in the paint is ignored unless we're drawing an A8 bitmap
    canvas->drawBitmapRectToRect(altBM, NULL, r, &paint);
    *usedPixRefs->append() = bm.pixelRef();
    *usedPixRefs->append() = altBM.pixelRef();
}

static void drawtext_proc(SkCanvas* canvas, const SkBitmap& bm,
                          const SkBitmap& altBM, const SkPoint& pos,
                          SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);
    paint.setTextSize(SkIntToScalar(1.5*bm.width()));

    canvas->drawText("0", 1, pos.fX, pos.fY+bm.width(), paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawpostext_proc(SkCanvas* canvas, const SkBitmap& bm,
                             const SkBitmap& altBM, const SkPoint& pos,
                             SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);
    paint.setTextSize(SkIntToScalar(1.5*bm.width()));

    SkPoint point = { pos.fX, pos.fY + bm.height() };
    canvas->drawPosText("O", 1, &point, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawtextonpath_proc(SkCanvas* canvas, const SkBitmap& bm,
                                const SkBitmap& altBM, const SkPoint& pos,
                                SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;

    init_paint(&paint, bm);
    paint.setTextSize(SkIntToScalar(1.5*bm.width()));

    SkPath path;
    path.lineTo(SkIntToScalar(bm.width()), 0);
    path.offset(pos.fX, pos.fY+bm.height());

    canvas->drawTextOnPath("O", 1, path, NULL, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

static void drawverts_proc(SkCanvas* canvas, const SkBitmap& bm,
                           const SkBitmap& altBM, const SkPoint& pos,
                           SkTDArray<SkPixelRef*>* usedPixRefs) {
    SkPaint paint;
    init_paint(&paint, bm);

    SkPoint verts[4] = {
        { pos.fX, pos.fY },
        { pos.fX + bm.width(), pos.fY },
        { pos.fX + bm.width(), pos.fY + bm.height() },
        { pos.fX, pos.fY + bm.height() }
    };
    SkPoint texs[4] = { { 0, 0 },
                        { SkIntToScalar(bm.width()), 0 },
                        { SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) },
                        { 0, SkIntToScalar(bm.height()) } };
    uint16_t indices[6] = { 0, 1, 2, 0, 2, 3 };

    canvas->drawVertices(SkCanvas::kTriangles_VertexMode, 4, verts, texs, NULL, NULL,
                         indices, 6, paint);
    *usedPixRefs->append() = bm.pixelRef();
}

// Return a picture with the bitmaps drawn at the specified positions.
static SkPicture* record_bitmaps(const SkBitmap bm[],
                                 const SkPoint pos[],
                                 SkTDArray<SkPixelRef*> analytic[],
                                 int count,
                                 DrawBitmapProc proc) {
    SkPicture* pic = new SkPicture;
    SkCanvas* canvas = pic->beginRecording(1000, 1000);
    for (int i = 0; i < count; ++i) {
        analytic[i].rewind();
        canvas->save();
        SkRect clipRect = SkRect::MakeXYWH(pos[i].fX, pos[i].fY,
                                           SkIntToScalar(bm[i].width()),
                                           SkIntToScalar(bm[i].height()));
        canvas->clipRect(clipRect, SkRegion::kIntersect_Op);
        proc(canvas, bm[i], bm[count+i], pos[i], &analytic[i]);
        canvas->restore();
    }
    pic->endRecording();
    return pic;
}

static void rand_rect(SkRect* rect, SkRandom& rand, SkScalar W, SkScalar H) {
    rect->fLeft   = rand.nextRangeScalar(-W, 2*W);
    rect->fTop    = rand.nextRangeScalar(-H, 2*H);
    rect->fRight  = rect->fLeft + rand.nextRangeScalar(0, W);
    rect->fBottom = rect->fTop + rand.nextRangeScalar(0, H);

    // we integralize rect to make our tests more predictable, since Gather is
    // a little sloppy.
    SkIRect ir;
    rect->round(&ir);
    rect->set(ir);
}

static void draw(SkPicture* pic, int width, int height, SkBitmap* result) {
    make_bm(result, width, height, SK_ColorBLACK, false);

    SkCanvas canvas(*result);
    canvas.drawPicture(*pic);
}

template <typename T> int find_index(const T* array, T elem, int count) {
    for (int i = 0; i < count; ++i) {
        if (array[i] == elem) {
            return i;
        }
    }
    return -1;
}

// Return true if 'ref' is found in array[]
static bool find(SkPixelRef const * const * array, SkPixelRef const * ref, int count) {
    return find_index<const SkPixelRef*>(array, ref, count) >= 0;
}

// Look at each pixel that is inside 'subset', and if its color appears in
// colors[], find the corresponding value in refs[] and append that ref into
// array, skipping duplicates of the same value.
// Note that gathering pixelRefs from rendered colors suffers from the problem
// that multiple simultaneous textures (e.g., A8 for alpha and 8888 for color)
// isn't easy to reconstruct.
static void gather_from_image(const SkBitmap& bm, SkPixelRef* const refs[],
                              int count, SkTDArray<SkPixelRef*>* array,
                              const SkRect& subset) {
    SkIRect ir;
    subset.roundOut(&ir);

    if (!ir.intersect(0, 0, bm.width()-1, bm.height()-1)) {
        return;
    }

    // Since we only want to return unique values in array, when we scan we just
    // set a bit for each index'd color found. In practice we only have a few
    // distinct colors, so we just use an int's bits as our array. Hence the
    // assert that count <= number-of-bits-in-our-int.
    SkASSERT((unsigned)count <= 32);
    uint32_t bitarray = 0;

    SkAutoLockPixels alp(bm);

    for (int y = ir.fTop; y < ir.fBottom; ++y) {
        for (int x = ir.fLeft; x < ir.fRight; ++x) {
            SkPMColor pmc = *bm.getAddr32(x, y);
            // the only good case where the color is not found would be if
            // the color is transparent, meaning no bitmap was drawn in that
            // pixel.
            if (pmc) {
                uint32_t index = SkGetPackedR32(pmc);
                SkASSERT(SkGetPackedG32(pmc) == index);
                SkASSERT(SkGetPackedB32(pmc) == index);
                if (0 == index) {
                    continue;           // background color
                }
                SkASSERT(0 == (index - gColorOffset) % gColorScale);
                index = (index - gColorOffset) / gColorScale;
                SkASSERT(static_cast<int>(index) < count);
                bitarray |= 1 << index;
            }
        }
    }

    for (int i = 0; i < count; ++i) {
        if (bitarray & (1 << i)) {
            *array->append() = refs[i];
        }
    }
}

static void gather_from_analytic(const SkPoint pos[], SkScalar w, SkScalar h,
                                 const SkTDArray<SkPixelRef*> analytic[],
                                 int count,
                                 SkTDArray<SkPixelRef*>* result,
                                 const SkRect& subset) {
    for (int i = 0; i < count; ++i) {
        SkRect rect = SkRect::MakeXYWH(pos[i].fX, pos[i].fY, w, h);

        if (SkRect::Intersects(subset, rect)) {
            result->append(analytic[i].count(), analytic[i].begin());
        }
    }
}

static const DrawBitmapProc gProcs[] = {
        drawpaint_proc,
        drawpoints_proc,
        drawrect_proc,
        drawoval_proc,
        drawrrect_proc,
        drawpath_proc,
        drawbitmap_proc,
        drawbitmap_withshader_proc,
        drawsprite_proc,
#if 0
        drawsprite_withshader_proc,
#endif
        drawbitmaprect_proc,
        drawbitmaprect_withshader_proc,
        drawtext_proc,
        drawpostext_proc,
        drawtextonpath_proc,
        drawverts_proc,
};

static void create_textures(SkBitmap* bm, SkPixelRef** refs, int num, int w, int h) {
    // Our convention is that the color components contain an encoding of
    // the index of their corresponding bitmap/pixelref. (0,0,0,0) is
    // reserved for the background
    for (int i = 0; i < num; ++i) {
        make_bm(&bm[i], w, h,
                SkColorSetARGB(0xFF,
                               gColorScale*i+gColorOffset,
                               gColorScale*i+gColorOffset,
                               gColorScale*i+gColorOffset),
                true);
        refs[i] = bm[i].pixelRef();
    }

    // The A8 alternate bitmaps are all BW checkerboards
    for (int i = 0; i < num; ++i) {
        make_checkerboard(&bm[num+i], w, h, true);
        refs[num+i] = bm[num+i].pixelRef();
    }
}

static void test_gatherpixelrefs(skiatest::Reporter* reporter) {
    const int IW = 32;
    const int IH = IW;
    const SkScalar W = SkIntToScalar(IW);
    const SkScalar H = W;

    static const int N = 4;
    SkBitmap bm[2*N];
    SkPixelRef* refs[2*N];
    SkTDArray<SkPixelRef*> analytic[N];

    const SkPoint pos[N] = {
        { 0, 0 }, { W, 0 }, { 0, H }, { W, H }
    };

    create_textures(bm, refs, N, IW, IH);

    SkRandom rand;
    for (size_t k = 0; k < SK_ARRAY_COUNT(gProcs); ++k) {
        SkAutoTUnref<SkPicture> pic(record_bitmaps(bm, pos, analytic, N, gProcs[k]));

        REPORTER_ASSERT(reporter, pic->willPlayBackBitmaps() || N == 0);
        // quick check for a small piece of each quadrant, which should just
        // contain 1 or 2 bitmaps.
        for (size_t  i = 0; i < SK_ARRAY_COUNT(pos); ++i) {
            SkRect r;
            r.set(2, 2, W - 2, H - 2);
            r.offset(pos[i].fX, pos[i].fY);
            SkAutoDataUnref data(SkPictureUtils::GatherPixelRefs(pic, r));
            REPORTER_ASSERT(reporter, data);
            if (data) {
                SkPixelRef** gatheredRefs = (SkPixelRef**)data->data();
                int count = static_cast<int>(data->size() / sizeof(SkPixelRef*));
                REPORTER_ASSERT(reporter, 1 == count || 2 == count);
                if (1 == count) {
                    REPORTER_ASSERT(reporter, gatheredRefs[0] == refs[i]);
                } else if (2 == count) {
                    REPORTER_ASSERT(reporter,
                        (gatheredRefs[0] == refs[i] && gatheredRefs[1] == refs[i+N]) ||
                        (gatheredRefs[1] == refs[i] && gatheredRefs[0] == refs[i+N]));
                }
            }
        }

        SkBitmap image;
        draw(pic, 2*IW, 2*IH, &image);

        // Test a bunch of random (mostly) rects, and compare the gather results
        // with a deduced list of refs by looking at the colors drawn.
        for (int j = 0; j < 100; ++j) {
            SkRect r;
            rand_rect(&r, rand, 2*W, 2*H);

            SkTDArray<SkPixelRef*> fromImage;
            gather_from_image(image, refs, N, &fromImage, r);

            SkTDArray<SkPixelRef*> fromAnalytic;
            gather_from_analytic(pos, W, H, analytic, N, &fromAnalytic, r);

            SkData* data = SkPictureUtils::GatherPixelRefs(pic, r);
            size_t dataSize = data ? data->size() : 0;
            int gatherCount = static_cast<int>(dataSize / sizeof(SkPixelRef*));
            SkASSERT(gatherCount * sizeof(SkPixelRef*) == dataSize);
            SkPixelRef** gatherRefs = data ? (SkPixelRef**)(data->data()) : NULL;
            SkAutoDataUnref adu(data);

            // Everything that we saw drawn should appear in the analytic list
            // but the analytic list may contain some pixelRefs that were not
            // seen in the image (e.g., A8 textures used as masks)
            for (int i = 0; i < fromImage.count(); ++i) {
                REPORTER_ASSERT(reporter, -1 != fromAnalytic.find(fromImage[i]));
            }

            /*
             *  GatherPixelRefs is conservative, so it can return more bitmaps
             *  than are strictly required. Thus our check here is only that
             *  Gather didn't miss any that we actually needed. Even that isn't
             *  a strict requirement on Gather, which is meant to be quick and
             *  only mostly-correct, but at the moment this test should work.
             */
            for (int i = 0; i < fromAnalytic.count(); ++i) {
                bool found = find(gatherRefs, fromAnalytic[i], gatherCount);
                REPORTER_ASSERT(reporter, found);
#if 0
                // enable this block of code to debug failures, as it will rerun
                // the case that failed.
                if (!found) {
                    SkData* data = SkPictureUtils::GatherPixelRefs(pic, r);
                    size_t dataSize = data ? data->size() : 0;
                }
#endif
            }
        }
    }
}

static void test_gatherpixelrefsandrects(skiatest::Reporter* reporter) {
    const int IW = 32;
    const int IH = IW;
    const SkScalar W = SkIntToScalar(IW);
    const SkScalar H = W;

    static const int N = 4;
    SkBitmap bm[2*N];
    SkPixelRef* refs[2*N];
    SkTDArray<SkPixelRef*> analytic[N];

    const SkPoint pos[N] = {
        { 0, 0 }, { W, 0 }, { 0, H }, { W, H }
    };

    create_textures(bm, refs, N, IW, IH);

    SkRandom rand;
    for (size_t k = 0; k < SK_ARRAY_COUNT(gProcs); ++k) {
        SkAutoTUnref<SkPicture> pic(record_bitmaps(bm, pos, analytic, N, gProcs[k]));

        REPORTER_ASSERT(reporter, pic->willPlayBackBitmaps() || N == 0);

        SkAutoTUnref<SkPictureUtils::SkPixelRefContainer> prCont(
                                new SkPictureUtils::SkPixelRefsAndRectsList);

        SkPictureUtils::GatherPixelRefsAndRects(pic, prCont);

        // quick check for a small piece of each quadrant, which should just
        // contain 1 or 2 bitmaps.
        for (size_t  i = 0; i < SK_ARRAY_COUNT(pos); ++i) {
            SkRect r;
            r.set(2, 2, W - 2, H - 2);
            r.offset(pos[i].fX, pos[i].fY);

            SkTDArray<SkPixelRef*> gatheredRefs;
            prCont->query(r, &gatheredRefs);

            int count = gatheredRefs.count();
            REPORTER_ASSERT(reporter, 1 == count || 2 == count);
            if (1 == count) {
                REPORTER_ASSERT(reporter, gatheredRefs[0] == refs[i]);
            } else if (2 == count) {
                REPORTER_ASSERT(reporter,
                    (gatheredRefs[0] == refs[i] && gatheredRefs[1] == refs[i+N]) ||
                    (gatheredRefs[1] == refs[i] && gatheredRefs[0] == refs[i+N]));
            }
        }

        SkBitmap image;
        draw(pic, 2*IW, 2*IH, &image);

        // Test a bunch of random (mostly) rects, and compare the gather results
        // with the analytic results and the pixel refs seen in a rendering.
        for (int j = 0; j < 100; ++j) {
            SkRect r;
            rand_rect(&r, rand, 2*W, 2*H);

            SkTDArray<SkPixelRef*> fromImage;
            gather_from_image(image, refs, N, &fromImage, r);

            SkTDArray<SkPixelRef*> fromAnalytic;
            gather_from_analytic(pos, W, H, analytic, N, &fromAnalytic, r);

            SkTDArray<SkPixelRef*> gatheredRefs;
            prCont->query(r, &gatheredRefs);

            // Everything that we saw drawn should appear in the analytic list
            // but the analytic list may contain some pixelRefs that were not
            // seen in the image (e.g., A8 textures used as masks)
            for (int i = 0; i < fromImage.count(); ++i) {
                REPORTER_ASSERT(reporter, -1 != fromAnalytic.find(fromImage[i]));
            }

            // Everything in the analytic list should appear in the gathered
            // list.
            for (int i = 0; i < fromAnalytic.count(); ++i) {
                REPORTER_ASSERT(reporter, -1 != gatheredRefs.find(fromAnalytic[i]));
            }
        }
    }
}

#ifdef SK_DEBUG
// Ensure that deleting SkPicturePlayback does not assert. Asserts only fire in debug mode, so only
// run in debug mode.
static void test_deleting_empty_playback() {
    SkPicture picture;
    // Creates an SkPictureRecord
    picture.beginRecording(0, 0);
    // Turns that into an SkPicturePlayback
    picture.endRecording();
    // Deletes the old SkPicturePlayback, and creates a new SkPictureRecord
    picture.beginRecording(0, 0);
}

// Ensure that serializing an empty picture does not assert. Likewise only runs in debug mode.
static void test_serializing_empty_picture() {
    SkPicture picture;
    picture.beginRecording(0, 0);
    picture.endRecording();
    SkDynamicMemoryWStream stream;
    picture.serialize(&stream);
}
#endif

static void rand_op(SkCanvas* canvas, SkRandom& rand) {
    SkPaint paint;
    SkRect rect = SkRect::MakeWH(50, 50);

    SkScalar unit = rand.nextUScalar1();
    if (unit <= 0.3) {
//        SkDebugf("save\n");
        canvas->save();
    } else if (unit <= 0.6) {
//        SkDebugf("restore\n");
        canvas->restore();
    } else if (unit <= 0.9) {
//        SkDebugf("clip\n");
        canvas->clipRect(rect);
    } else {
//        SkDebugf("draw\n");
        canvas->drawPaint(paint);
    }
}

static void test_peephole() {
    SkRandom rand;

    for (int j = 0; j < 100; j++) {
        SkRandom rand2(rand); // remember the seed

        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(100, 100);

        for (int i = 0; i < 1000; ++i) {
            rand_op(canvas, rand);
        }
        picture.endRecording();

        rand = rand2;
    }

    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(100, 100);
        SkRect rect = SkRect::MakeWH(50, 50);

        for (int i = 0; i < 100; ++i) {
            canvas->save();
        }
        while (canvas->getSaveCount() > 1) {
            canvas->clipRect(rect);
            canvas->restore();
        }
        picture.endRecording();
    }
}

#ifndef SK_DEBUG
// Only test this is in release mode. We deliberately crash in debug mode, since a valid caller
// should never do this.
static void test_bad_bitmap() {
    // This bitmap has a width and height but no pixels. As a result, attempting to record it will
    // fail.
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    SkPicture picture;
    SkCanvas* recordingCanvas = picture.beginRecording(100, 100);
    recordingCanvas->drawBitmap(bm, 0, 0);
    picture.endRecording();

    SkCanvas canvas;
    canvas.drawPicture(picture);
}
#endif

static SkData* encode_bitmap_to_data(size_t*, const SkBitmap& bm) {
    return SkImageEncoder::EncodeData(bm, SkImageEncoder::kPNG_Type, 100);
}

static SkData* serialized_picture_from_bitmap(const SkBitmap& bitmap) {
    SkPicture picture;
    SkCanvas* canvas = picture.beginRecording(bitmap.width(), bitmap.height());
    canvas->drawBitmap(bitmap, 0, 0);
    SkDynamicMemoryWStream wStream;
    picture.serialize(&wStream, &encode_bitmap_to_data);
    return wStream.copyToData();
}

struct ErrorContext {
    int fErrors;
    skiatest::Reporter* fReporter;
};

static void assert_one_parse_error_cb(SkError error, void* context) {
    ErrorContext* errorContext = static_cast<ErrorContext*>(context);
    errorContext->fErrors++;
    // This test only expects one error, and that is a kParseError. If there are others,
    // there is some unknown problem.
    REPORTER_ASSERT_MESSAGE(errorContext->fReporter, 1 == errorContext->fErrors,
                            "This threw more errors than expected.");
    REPORTER_ASSERT_MESSAGE(errorContext->fReporter, kParseError_SkError == error,
                            SkGetLastErrorString());
}

static void test_bitmap_with_encoded_data(skiatest::Reporter* reporter) {
    // Create a bitmap that will be encoded.
    SkBitmap original;
    make_bm(&original, 100, 100, SK_ColorBLUE, true);
    SkDynamicMemoryWStream wStream;
    if (!SkImageEncoder::EncodeStream(&wStream, original, SkImageEncoder::kPNG_Type, 100)) {
        return;
    }
    SkAutoDataUnref data(wStream.copyToData());

    SkBitmap bm;
    bool installSuccess = SkInstallDiscardablePixelRef(
         SkDecodingImageGenerator::Create(data, SkDecodingImageGenerator::Options()), &bm, NULL);
    REPORTER_ASSERT(reporter, installSuccess);

    // Write both bitmaps to pictures, and ensure that the resulting data streams are the same.
    // Flattening original will follow the old path of performing an encode, while flattening bm
    // will use the already encoded data.
    SkAutoDataUnref picture1(serialized_picture_from_bitmap(original));
    SkAutoDataUnref picture2(serialized_picture_from_bitmap(bm));
    REPORTER_ASSERT(reporter, picture1->equals(picture2));
    // Now test that a parse error was generated when trying to create a new SkPicture without
    // providing a function to decode the bitmap.
    ErrorContext context;
    context.fErrors = 0;
    context.fReporter = reporter;
    SkSetErrorCallback(assert_one_parse_error_cb, &context);
    SkMemoryStream pictureStream(picture1);
    SkClearLastError();
    SkAutoUnref pictureFromStream(SkPicture::CreateFromStream(&pictureStream, NULL));
    REPORTER_ASSERT(reporter, pictureFromStream.get() != NULL);
    SkClearLastError();
    SkSetErrorCallback(NULL, NULL);
}

static void test_clone_empty(skiatest::Reporter* reporter) {
    // This is a regression test for crbug.com/172062
    // Before the fix, we used to crash accessing a null pointer when we
    // had a picture with no paints. This test passes by not crashing.
    {
        SkPicture picture;
        picture.beginRecording(1, 1);
        picture.endRecording();
        SkPicture* destPicture = picture.clone();
        REPORTER_ASSERT(reporter, NULL != destPicture);
        destPicture->unref();
    }
    {
        // Test without call to endRecording
        SkPicture picture;
        picture.beginRecording(1, 1);
        SkPicture* destPicture = picture.clone();
        REPORTER_ASSERT(reporter, NULL != destPicture);
        destPicture->unref();
    }
}

static void test_clip_bound_opt(skiatest::Reporter* reporter) {
    // Test for crbug.com/229011
    SkRect rect1 = SkRect::MakeXYWH(SkIntToScalar(4), SkIntToScalar(4),
                                    SkIntToScalar(2), SkIntToScalar(2));
    SkRect rect2 = SkRect::MakeXYWH(SkIntToScalar(7), SkIntToScalar(7),
                                    SkIntToScalar(1), SkIntToScalar(1));
    SkRect rect3 = SkRect::MakeXYWH(SkIntToScalar(6), SkIntToScalar(6),
                                    SkIntToScalar(1), SkIntToScalar(1));

    SkPath invPath;
    invPath.addOval(rect1);
    invPath.setFillType(SkPath::kInverseEvenOdd_FillType);
    SkPath path;
    path.addOval(rect2);
    SkPath path2;
    path2.addOval(rect3);
    SkIRect clipBounds;
    // Minimalist test set for 100% code coverage of
    // SkPictureRecord::updateClipConservativelyUsingBounds
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(invPath, SkRegion::kIntersect_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(path, SkRegion::kIntersect_Op);
        canvas->clipPath(invPath, SkRegion::kIntersect_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(path, SkRegion::kIntersect_Op);
        canvas->clipPath(invPath, SkRegion::kUnion_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(path, SkRegion::kDifference_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(path, SkRegion::kReverseDifference_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        // True clip is actually empty in this case, but the best
        // determination we can make using only bounds as input is that the
        // clip is included in the bounds of 'path'.
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
    {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(10, 10,
            SkPicture::kUsePathBoundsForClip_RecordingFlag);
        canvas->clipPath(path, SkRegion::kIntersect_Op);
        canvas->clipPath(path2, SkRegion::kXOR_Op);
        bool nonEmpty = canvas->getClipDeviceBounds(&clipBounds);
        REPORTER_ASSERT(reporter, true == nonEmpty);
        REPORTER_ASSERT(reporter, 6 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 6 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
}

/**
 * A canvas that records the number of clip commands.
 */
class ClipCountingCanvas : public SkCanvas {
public:
    explicit ClipCountingCanvas(int width, int height)
        : INHERITED(width, height)
        , fClipCount(0){
    }

    virtual bool clipRect(const SkRect& r, SkRegion::Op op, bool doAA)
        SK_OVERRIDE {
        fClipCount += 1;
        return this->INHERITED::clipRect(r, op, doAA);
    }

    virtual bool clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA)
        SK_OVERRIDE {
        fClipCount += 1;
        return this->INHERITED::clipRRect(rrect, op, doAA);
    }

    virtual bool clipPath(const SkPath& path, SkRegion::Op op, bool doAA)
        SK_OVERRIDE {
        fClipCount += 1;
        return this->INHERITED::clipPath(path, op, doAA);
    }

    unsigned getClipCount() const { return fClipCount; }

private:
    unsigned fClipCount;

    typedef SkCanvas INHERITED;
};

static void test_clip_expansion(skiatest::Reporter* reporter) {
    SkPicture picture;
    SkCanvas* canvas = picture.beginRecording(10, 10, 0);

    canvas->clipRect(SkRect::MakeEmpty(), SkRegion::kReplace_Op);
    // The following expanding clip should not be skipped.
    canvas->clipRect(SkRect::MakeXYWH(4, 4, 3, 3), SkRegion::kUnion_Op);
    // Draw something so the optimizer doesn't just fold the world.
    SkPaint p;
    p.setColor(SK_ColorBLUE);
    canvas->drawPaint(p);

    ClipCountingCanvas testCanvas(10, 10);
    picture.draw(&testCanvas);

    // Both clips should be present on playback.
    REPORTER_ASSERT(reporter, testCanvas.getClipCount() == 2);
}

static void test_hierarchical(skiatest::Reporter* reporter) {
    SkBitmap bm;
    make_bm(&bm, 10, 10, SK_ColorRED, true);

    SkCanvas* canvas;

    SkPicture childPlain;
    childPlain.beginRecording(10, 10);
    childPlain.endRecording();
    REPORTER_ASSERT(reporter, !childPlain.willPlayBackBitmaps()); // 0

    SkPicture childWithBitmap;
    childWithBitmap.beginRecording(10, 10)->drawBitmap(bm, 0, 0);
    childWithBitmap.endRecording();
    REPORTER_ASSERT(reporter, childWithBitmap.willPlayBackBitmaps()); // 1

    SkPicture parentPP;
    canvas = parentPP.beginRecording(10, 10);
    canvas->drawPicture(childPlain);
    parentPP.endRecording();
    REPORTER_ASSERT(reporter, !parentPP.willPlayBackBitmaps()); // 0

    SkPicture parentPWB;
    canvas = parentPWB.beginRecording(10, 10);
    canvas->drawPicture(childWithBitmap);
    parentPWB.endRecording();
    REPORTER_ASSERT(reporter, parentPWB.willPlayBackBitmaps()); // 1

    SkPicture parentWBP;
    canvas = parentWBP.beginRecording(10, 10);
    canvas->drawBitmap(bm, 0, 0);
    canvas->drawPicture(childPlain);
    parentWBP.endRecording();
    REPORTER_ASSERT(reporter, parentWBP.willPlayBackBitmaps()); // 1

    SkPicture parentWBWB;
    canvas = parentWBWB.beginRecording(10, 10);
    canvas->drawBitmap(bm, 0, 0);
    canvas->drawPicture(childWithBitmap);
    parentWBWB.endRecording();
    REPORTER_ASSERT(reporter, parentWBWB.willPlayBackBitmaps()); // 2
}

DEF_TEST(Picture, reporter) {
#ifdef SK_DEBUG
    test_deleting_empty_playback();
    test_serializing_empty_picture();
#else
    test_bad_bitmap();
#endif
    test_peephole();
    test_gatherpixelrefs(reporter);
    test_gatherpixelrefsandrects(reporter);
    test_bitmap_with_encoded_data(reporter);
    test_clone_empty(reporter);
    test_clip_bound_opt(reporter);
    test_clip_expansion(reporter);
    test_hierarchical(reporter);
}
