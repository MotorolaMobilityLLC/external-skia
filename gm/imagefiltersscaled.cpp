/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColor.h"
#include "SkBitmapSource.h"
#include "SkBlurImageFilter.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkGradientShader.h"
#include "SkMorphologyImageFilter.h"
#include "SkScalar.h"

namespace skiagm {

class ImageFiltersScaledGM : public GM {
public:
    ImageFiltersScaledGM() : fInitialized(false) {
        this->setBGColor(0x00000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("imagefiltersscaled");
    }

    virtual SkISize onISize() {
        return make_isize(860, 500);
    }

    void make_checkerboard() {
        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config, 64, 64);
        fCheckerboard.allocPixels();
        SkBitmapDevice device(fCheckerboard);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF404040);
        SkPaint lightPaint;
        lightPaint.setColor(0xFFA0A0A0);
        for (int y = 0; y < 64; y += 16) {
          for (int x = 0; x < 64; x += 16) {
            canvas.save();
            canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas.drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
            canvas.restore();
          }
        }
    }

    void make_gradient_circle(int width, int height) {
        SkScalar x = SkIntToScalar(width / 2);
        SkScalar y = SkIntToScalar(height / 2);
        SkScalar radius = SkScalarMul(SkMinScalar(x, y), SkIntToScalar(4) / SkIntToScalar(5));
        fGradientCircle.setConfig(SkBitmap::kARGB_8888_Config, width, height);
        fGradientCircle.allocPixels();
        SkBitmapDevice device(fGradientCircle);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkColor colors[2];
        colors[0] = SK_ColorWHITE;
        colors[1] = SK_ColorBLACK;
        SkAutoTUnref<SkShader> shader(
            SkGradientShader::CreateRadial(SkPoint::Make(x, y), radius, colors, NULL, 2,
                                           SkShader::kClamp_TileMode)
        );
        SkPaint paint;
        paint.setShader(shader);
        canvas.drawCircle(x, y, radius, paint);
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            this->make_checkerboard();
            this->make_gradient_circle(64, 64);
            fInitialized = true;
        }
        canvas->clear(0x00000000);

        SkAutoTUnref<SkImageFilter> gradient(new SkBitmapSource(fGradientCircle));
        SkAutoTUnref<SkImageFilter> checkerboard(new SkBitmapSource(fCheckerboard));

        SkImageFilter* filters[] = {
            new SkBlurImageFilter(SkIntToScalar(4), SkIntToScalar(4)),
            new SkDropShadowImageFilter(SkIntToScalar(5), SkIntToScalar(10), SkIntToScalar(3),
                                        SK_ColorYELLOW),
            new SkDisplacementMapEffect(SkDisplacementMapEffect::kR_ChannelSelectorType,
                                        SkDisplacementMapEffect::kR_ChannelSelectorType,
                                        SkIntToScalar(12),
                                        gradient.get(),
                                        checkerboard.get()),
            new SkDilateImageFilter(1, 1, checkerboard.get()),
            new SkErodeImageFilter(1, 1, checkerboard.get()),
        };

        SkVector scales[] = {
            SkVector::Make(SkScalarInvert(2), SkScalarInvert(2)),
            SkVector::Make(SkIntToScalar(1), SkIntToScalar(1)),
            SkVector::Make(SkIntToScalar(1), SkIntToScalar(2)),
            SkVector::Make(SkIntToScalar(2), SkIntToScalar(1)),
            SkVector::Make(SkIntToScalar(2), SkIntToScalar(2)),
        };

        SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
        SkScalar margin = SkIntToScalar(16);
        SkRect bounds = r;
        bounds.outset(margin, margin);

        for (size_t j = 0; j < SK_ARRAY_COUNT(scales); ++j) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
                SkPaint paint;
                paint.setColor(SK_ColorBLUE);
                paint.setImageFilter(filters[i]);
                paint.setAntiAlias(true);
                canvas->save();
                canvas->scale(scales[j].fX, scales[j].fY);
                canvas->clipRect(bounds);
                canvas->drawCircle(r.centerX(), r.centerY(),
                                   SkScalarDiv(r.width()*2, SkIntToScalar(5)), paint);
                canvas->restore();
                canvas->translate(r.width() * scales[j].fX + margin, 0);
            }
            canvas->restore();
            canvas->translate(0, r.height() * scales[j].fY + margin);
        }

        for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
            filters[i]->unref();
        }
    }

private:
    bool fInitialized;
    SkBitmap fCheckerboard;
    SkBitmap fGradientCircle;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ImageFiltersScaledGM; }
static GMRegistry reg(MyFactory);

}
