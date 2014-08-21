/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkColorFilter.h"
#include "SkMultiPictureDraw.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"

static const SkScalar kRoot3Over2 = 0.86602545f;  // sin(60)

static const int kHexSide = 30;
static const int kNumHexX = 6;
static const int kNumHexY = 6;
static const int kPicWidth = kNumHexX * kHexSide;
static const int kPicHeight = SkScalarCeilToInt((kNumHexY - 0.5f) * 2 * kHexSide * kRoot3Over2);
static const SkScalar kInset = 20.0f;

// Create a hexagon centered at (originX, originY)
static SkPath make_hex_path(SkScalar originX, SkScalar originY) {
    SkPath hex;
    hex.moveTo(originX-kHexSide, originY);
    hex.rLineTo(SkScalarHalf(kHexSide), kRoot3Over2 * kHexSide);
    hex.rLineTo(SkIntToScalar(kHexSide), 0);
    hex.rLineTo(SkScalarHalf(kHexSide), -kHexSide * kRoot3Over2);
    hex.rLineTo(-SkScalarHalf(kHexSide), -kHexSide * kRoot3Over2);
    hex.rLineTo(-SkIntToScalar(kHexSide), 0);
    hex.close();
    return hex;
}

// Make a picture that is a tiling of the plane with stroked hexagons where
// each hexagon is in its own layer. The layers are to exercise Ganesh's
// layer hoisting.
static const SkPicture* make_picture(SkColor fillColor) {

    // Create a hexagon with its center at the origin
    SkPath hex = make_hex_path(0, 0);

    SkPaint fill;
    fill.setStyle(SkPaint::kFill_Style);
    fill.setColor(fillColor);

    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(3);

    SkPictureRecorder recorder;

    SkCanvas* canvas = recorder.beginRecording(kPicWidth, kPicHeight);

    SkScalar xPos, yPos = 0;

    for (int y = 0; y < kNumHexY; ++y) {
        xPos = 0;

        for (int x = 0; x < kNumHexX; ++x) {
            canvas->saveLayer(NULL, NULL);
            canvas->translate(xPos, yPos + ((x % 2) ? kRoot3Over2 * kHexSide : 0));
            canvas->drawPath(hex, fill);
            canvas->drawPath(hex, stroke);
            canvas->restore();

            xPos += 1.5f * kHexSide;
        }

        yPos += 2 * kHexSide * kRoot3Over2;
    }

    return recorder.endRecording();
}

static SkSurface* create_compat_surface(SkCanvas* canvas, int width, int height) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);

    SkSurface* surface = canvas->newSurface(info);
    if (NULL == surface) {
        // picture canvas returns NULL so fall back to raster
        surface = SkSurface::NewRaster(info);
    }

    return surface;
}

// This class stores the information required to compose all the result
// fragments potentially generated by the MultiPictureDraw object
class ComposeStep {
public:
    ComposeStep() : fSurf(NULL), fX(0.0f), fY(0.0f), fPaint(NULL) { }
    ~ComposeStep() { SkSafeUnref(fSurf);  SkDELETE(fPaint); }

    SkSurface* fSurf;
    SkScalar   fX;
    SkScalar   fY;
    SkPaint*   fPaint;
};

typedef void (*PFContentMtd)(SkCanvas* canvas, const SkPicture* pictures[2]);

// Just a single picture with no clip
static void no_clip(SkCanvas* canvas, const SkPicture* pictures[2]) {
    canvas->drawPicture(pictures[0]);
}

// Two pictures with a rect clip on the second one
static void rect_clip(SkCanvas* canvas, const SkPicture* pictures[2]) {
    canvas->drawPicture(pictures[0]);

    SkRect rect = SkRect::MakeWH(SkIntToScalar(kPicWidth), SkIntToScalar(kPicHeight));
    rect.inset(kInset, kInset);

    canvas->clipRect(rect);

    canvas->drawPicture(pictures[1]);
}

// Two pictures with a round rect clip on the second one
static void rrect_clip(SkCanvas* canvas, const SkPicture* pictures[2]) {
    canvas->drawPicture(pictures[0]);

    SkRect rect = SkRect::MakeWH(SkIntToScalar(kPicWidth), SkIntToScalar(kPicHeight));
    rect.inset(kInset, kInset);
        
    SkRRect rrect;
    rrect.setRectXY(rect, kInset, kInset);

    canvas->clipRRect(rrect);

    canvas->drawPicture(pictures[1]);
}

// Two pictures with a clip path on the second one
static void path_clip(SkCanvas* canvas, const SkPicture* pictures[2]) {
    canvas->drawPicture(pictures[0]);

    // Create a hexagon centered on the middle of the hex grid
    SkPath hex = make_hex_path((kNumHexX / 2.0f) * kHexSide, kNumHexY * kHexSide * kRoot3Over2);

    canvas->clipPath(hex);

    canvas->drawPicture(pictures[1]);
}

// Two pictures with an inverse clip path on the second one
static void invpath_clip(SkCanvas* canvas, const SkPicture* pictures[2]) {
    canvas->drawPicture(pictures[0]);

    // Create a hexagon centered on the middle of the hex grid
    SkPath hex = make_hex_path((kNumHexX / 2.0f) * kHexSide, kNumHexY * kHexSide * kRoot3Over2);
    hex.setFillType(SkPath::kInverseEvenOdd_FillType);

    canvas->clipPath(hex);

    canvas->drawPicture(pictures[1]);
}

static const PFContentMtd gContentMthds[] = {
    no_clip,
    rect_clip,
    rrect_clip,
    path_clip,
    invpath_clip
};

static void create_content(SkMultiPictureDraw* mpd, PFContentMtd pfGen,
                           const SkPicture* pictures[2],
                           SkCanvas* dest, const SkMatrix& xform) {
    SkAutoTUnref<SkPicture> composite;

    {
        SkPictureRecorder recorder;

        SkCanvas* pictureCanvas = recorder.beginRecording(kPicWidth, kPicHeight);

        (*pfGen)(pictureCanvas, pictures);

        composite.reset(recorder.endRecording());
    }

    mpd->add(dest, composite, &xform);
}

typedef void(*PFLayoutMtd)(SkCanvas* finalCanvas, SkMultiPictureDraw* mpd, 
                           PFContentMtd pfGen, const SkPicture* pictures[2],
                           SkTArray<ComposeStep>* composeSteps);

// Draw the content into a single canvas
static void simple(SkCanvas* finalCanvas, SkMultiPictureDraw* mpd, 
                   PFContentMtd pfGen, 
                   const SkPicture* pictures[2],
                   SkTArray<ComposeStep> *composeSteps) {

    ComposeStep& step = composeSteps->push_back();

    step.fSurf = create_compat_surface(finalCanvas, kPicWidth, kPicHeight);

    SkCanvas* subCanvas = step.fSurf->getCanvas();

    create_content(mpd, pfGen, pictures, subCanvas, SkMatrix::I());
}

// Draw the content into multiple canvases/tiles
static void tiled(SkCanvas* finalCanvas, SkMultiPictureDraw* mpd,
                  PFContentMtd pfGen, 
                  const SkPicture* pictures[2],
                  SkTArray<ComposeStep> *composeSteps) {
    static const int kNumTilesX = 2;
    static const int kNumTilesY = 2;
    static const int kTileWidth = kPicWidth / kNumTilesX;
    static const int kTileHeight = kPicHeight / kNumTilesY;

    SkASSERT(kPicWidth == kNumTilesX * kTileWidth);
    SkASSERT(kPicHeight == kNumTilesY * kTileHeight);

    static const SkColor colors[kNumTilesX][kNumTilesY] = {
        { SK_ColorCYAN,   SK_ColorMAGENTA }, 
        { SK_ColorYELLOW, SK_ColorGREEN   }
    };

    for (int y = 0; y < kNumTilesY; ++y) {
        for (int x = 0; x < kNumTilesX; ++x) {
            ComposeStep& step = composeSteps->push_back();

            step.fX = SkIntToScalar(x*kTileWidth);
            step.fY = SkIntToScalar(y*kTileHeight);
            step.fPaint = SkNEW(SkPaint);
            step.fPaint->setColorFilter(
                SkColorFilter::CreateModeFilter(colors[x][y], SkXfermode::kModulate_Mode))->unref();

            step.fSurf = create_compat_surface(finalCanvas, kTileWidth, kTileHeight);

            SkCanvas* subCanvas = step.fSurf->getCanvas();

            SkMatrix trans;
            trans.setTranslate(-SkIntToScalar(x*kTileWidth), -SkIntToScalar(y*kTileHeight));

            create_content(mpd, pfGen, pictures, subCanvas, trans);
        }
    }
}

static const PFLayoutMtd gLayoutMthds[] = { simple, tiled };

namespace skiagm {
    /**
     * This GM exercises the SkMultiPictureDraw object. It tests the
     * cross product of:
     *      tiled vs. all-at-once rendering (e.g., into many or just 1 canvas)
     *      different clips (e.g., none, rect, rrect)
     *      single vs. multiple pictures (e.g., normal vs. picture-pile-style content)
     */
    class MultiPictureDraw : public GM {
    public:
        enum Content {
            kNoClipSingle_Content,
            kRectClipMulti_Content,
            kRRectClipMulti_Content,
            kPathClipMulti_Content,
            kInvPathClipMulti_Content,

            kLast_Content = kInvPathClipMulti_Content
        };

        static const int kContentCnt = kLast_Content + 1;

        enum Layout {
            kSimple_Layout,
            kTiled_Layout,

            kLast_Layout = kTiled_Layout
        };

        static const int kLayoutCnt = kLast_Layout + 1;

        MultiPictureDraw(Content content, Layout layout) : fContent(content), fLayout(layout) {
            SkASSERT(SK_ARRAY_COUNT(gLayoutMthds) == kLayoutCnt);
            SkASSERT(SK_ARRAY_COUNT(gContentMthds) == kContentCnt);

            fPictures[0] = fPictures[1] = NULL;
        }

        virtual ~MultiPictureDraw() {
            SkSafeUnref(fPictures[0]);
            SkSafeUnref(fPictures[1]);
        }

    protected:
        Content          fContent;
        Layout           fLayout;
        const SkPicture* fPictures[2];

        virtual void onOnceBeforeDraw() SK_OVERRIDE {
            fPictures[0] = SkRef(make_picture(SK_ColorWHITE));
            fPictures[1] = SkRef(make_picture(SK_ColorGRAY));
        }

        virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE{
            SkMultiPictureDraw mpd;
            SkTArray<ComposeStep> composeSteps;

            // Fill up the MultiPictureDraw
            (*gLayoutMthds[fLayout])(canvas, &mpd, 
                                     gContentMthds[fContent], 
                                     fPictures, &composeSteps);

            mpd.draw();

            // Compose all the drawn canvases into the final canvas
            for (int i = 0; i < composeSteps.count(); ++i) {
                const ComposeStep& step = composeSteps[i];

                SkAutoTUnref<SkImage> image(step.fSurf->newImageSnapshot());

                image->draw(canvas, step.fX, step.fY, step.fPaint);
            }
        }

        virtual SkISize onISize() SK_OVERRIDE{ return SkISize::Make(kPicWidth, kPicHeight); }

        virtual SkString onShortName() SK_OVERRIDE{
            static const char* gContentNames[] = { 
                "noclip", "rectclip", "rrectclip", "pathclip", "invpathclip" 
            };
            static const char* gLayoutNames[] = { "simple", "tiled" };

            SkASSERT(SK_ARRAY_COUNT(gLayoutNames) == kLayoutCnt);
            SkASSERT(SK_ARRAY_COUNT(gContentNames) == kContentCnt);

            SkString name("multipicturedraw_");

            name.append(gContentNames[fContent]);
            name.append("_");
            name.append(gLayoutNames[fLayout]);
            return name;
        }

        virtual uint32_t onGetFlags() const SK_OVERRIDE { return kAsBench_Flag | kSkipTiled_Flag; }

    private:
        typedef GM INHERITED;
    };

    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kNoClipSingle_Content,     
                                                MultiPictureDraw::kSimple_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kRectClipMulti_Content,    
                                                MultiPictureDraw::kSimple_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kRRectClipMulti_Content,   
                                                MultiPictureDraw::kSimple_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kPathClipMulti_Content,    
                                                MultiPictureDraw::kSimple_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kInvPathClipMulti_Content, 
                                                MultiPictureDraw::kSimple_Layout));)

    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kNoClipSingle_Content,     
                                                MultiPictureDraw::kTiled_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kRectClipMulti_Content,    
                                                MultiPictureDraw::kTiled_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kRRectClipMulti_Content,   
                                                MultiPictureDraw::kTiled_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kPathClipMulti_Content,    
                                                MultiPictureDraw::kTiled_Layout));)
    DEF_GM(return SkNEW_ARGS(MultiPictureDraw, (MultiPictureDraw::kInvPathClipMulti_Content, 
                                                MultiPictureDraw::kTiled_Layout));)
}
