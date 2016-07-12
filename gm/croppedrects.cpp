/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRRect.h"
#include "SkSurface.h"

namespace skiagm {

constexpr SkRect kSrcImageClip{75, 75, 275, 275};

/*
 * The purpose of this test is to exercise all three codepaths in GrDrawContext (drawFilledRect,
 * fillRectToRect, fillRectWithLocalMatrix) that pre-crop filled rects based on the clip.
 *
 * The test creates an image of a green square surrounded by red background, then draws this image
 * in various ways with the red clipped out. The test is successful if there is no visible red
 * background, scissor is never used, and ideally, all the rectangles draw in one batch.
 */
class CroppedRectsGM : public GM {
private:
    SkString onShortName() override final { return SkString("croppedrects"); }
    SkISize onISize() override { return SkISize::Make(500, 500); }

    void onOnceBeforeDraw() override {
        sk_sp<SkSurface> srcSurface = SkSurface::MakeRasterN32Premul(500, 500);
        SkCanvas* srcCanvas = srcSurface->getCanvas();

        srcCanvas->clear(SK_ColorRED);

        SkPaint paint;
        paint.setColor(0xff00ff00);
        srcCanvas->drawRect(kSrcImageClip, paint);

        constexpr SkScalar kStrokeWidth = 10;
        SkPaint stroke;
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setStrokeWidth(kStrokeWidth);
        stroke.setColor(0xff008800);
        srcCanvas->drawRect(kSrcImageClip.makeInset(kStrokeWidth / 2, kStrokeWidth / 2), stroke);

        fSrcImage = srcSurface->makeImageSnapshot(SkBudgeted::kYes, SkSurface::kNo_ForceUnique);
        fSrcImageShader = fSrcImage->makeShader(SkShader::kClamp_TileMode,
                                                SkShader::kClamp_TileMode);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);

        {
            // GrDrawContext::drawFilledRect.
            SkAutoCanvasRestore acr(canvas, true);
            SkPaint paint;
            paint.setShader(fSrcImageShader);
            paint.setFilterQuality(kNone_SkFilterQuality);
            canvas->clipRect(kSrcImageClip);
            canvas->drawPaint(paint);
        }

        {
            // GrDrawContext::fillRectToRect.
            SkAutoCanvasRestore acr(canvas, true);
            SkPaint paint;
            paint.setFilterQuality(kNone_SkFilterQuality);
            SkRect drawRect = SkRect::MakeXYWH(350, 100, 100, 300);
            canvas->clipRect(drawRect);
            canvas->drawImageRect(fSrcImage.get(),
                                  kSrcImageClip.makeOutset(0.5f * kSrcImageClip.width(),
                                                           kSrcImageClip.height()),
                                  drawRect.makeOutset(0.5f * drawRect.width(), drawRect.height()),
                                  &paint);
        }

        {
            // GrDrawContext::fillRectWithLocalMatrix.
            SkAutoCanvasRestore acr(canvas, true);
            SkPath path;
            path.moveTo(kSrcImageClip.fLeft - kSrcImageClip.width(), kSrcImageClip.centerY());
            path.lineTo(kSrcImageClip.fRight + 3 * kSrcImageClip.width(), kSrcImageClip.centerY());
            SkPaint paint;
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(2 * kSrcImageClip.height());
            paint.setShader(fSrcImageShader);
            paint.setFilterQuality(kNone_SkFilterQuality);
            canvas->translate(-90, 263);
            canvas->scale(300 / kSrcImageClip.width(), 100 / kSrcImageClip.height());
            canvas->clipRect(kSrcImageClip);
            canvas->drawPath(path, paint);
        }

        // TODO: assert the draw target only has one batch in the post-MDB world.
    }

    sk_sp<SkImage> fSrcImage;
    sk_sp<SkShader> fSrcImageShader;

    typedef GM INHERITED;
};

DEF_GM( return new CroppedRectsGM(); )

}
