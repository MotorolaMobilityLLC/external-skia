/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkBlurImageFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkStream.h"
#include "SkTypeface.h"

/*
 * Spits out a dummy gradient to test blur with shader on paint
 */
static SkShader* MakeLinear() {
    static const SkPoint     kPts[] = { { 0, 0 }, { 32, 32 } };
    static const SkScalar    kPos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    static const SkColor kColors[] = {0x80F00080, 0xF0F08000, 0x800080F0 };
    return SkGradientShader::CreateLinear(kPts, kColors, kPos,
                                          SK_ARRAY_COUNT(kColors), SkShader::kClamp_TileMode);
}

static SkImageFilter* make_grayscale(SkImageFilter* input = NULL) {
    SkScalar matrix[20];
    memset(matrix, 0, 20 * sizeof(SkScalar));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    SkAutoTUnref<SkColorFilter> filter(SkColorMatrixFilter::Create(matrix));
    return SkColorFilterImageFilter::Create(filter, input);
}

static SkImageFilter* make_blur(float amount, SkImageFilter* input = NULL) {
    return SkBlurImageFilter::Create(amount, amount, input);
}

namespace skiagm {

class ColorEmojiGM : public GM {
public:
    ColorEmojiGM() {
        fTypeface = NULL;
    }

    ~ColorEmojiGM() {
        SkSafeUnref(fTypeface);
    }
protected:
    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        SkString filename = GetResourcePath("/Funkster.ttf");
        SkAutoTUnref<SkFILEStream> stream(new SkFILEStream(filename.c_str()));
        if (!stream->isValid()) {
            SkDebugf("Could not find Funkster.ttf, please set --resourcePath correctly.\n");
            return;
        }

        fTypeface = SkTypeface::CreateFromStream(stream);
    }

    virtual SkString onShortName() {
        return SkString("coloremoji");
    }

    virtual SkISize onISize() {
        return SkISize::Make(650, 480);
    }

    virtual void onDraw(SkCanvas* canvas) {

        canvas->drawColor(SK_ColorGRAY);

        SkPaint paint;
        paint.setTypeface(fTypeface);

        const char* text = "hamburgerfons";

        // draw text at different point sizes
        const int textSize[] = { 10, 30, 50, };
        const int textYOffset[] = { 10, 40, 100, };
        SkASSERT(sizeof(textSize) == sizeof(textYOffset));
        size_t y_offset = 0;
        for (size_t y = 0; y < sizeof(textSize) / sizeof(int); y++) {
            paint.setTextSize(SkIntToScalar(textSize[y]));
            canvas->drawText(text, strlen(text), 10, SkIntToScalar(textYOffset[y]), paint);
            y_offset += textYOffset[y];
        }

        // draw with shaders and image filters
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    SkPaint shaderPaint;
                    shaderPaint.setTypeface(fTypeface);
                    if (SkToBool(i)) {
                        shaderPaint.setShader(MakeLinear());
                    }

                    if (SkToBool(j) && SkToBool(k)) {
                        SkAutoTUnref<SkImageFilter> grayScale(make_grayscale(NULL));
                        SkAutoTUnref<SkImageFilter> blur(make_blur(3.0f, grayScale));
                        shaderPaint.setImageFilter(blur);
                    } else if (SkToBool(j)) {
                        SkAutoTUnref<SkImageFilter> blur(make_blur(3.0f, NULL));
                        shaderPaint.setImageFilter(blur);
                    } else if (SkToBool(k)) {
                        SkAutoTUnref<SkImageFilter> grayScale(make_grayscale(NULL));
                        shaderPaint.setImageFilter(grayScale);
                    }
                    shaderPaint.setTextSize(30);
                    canvas->drawText(text, strlen(text), 380, SkIntToScalar(y_offset), shaderPaint);
                    y_offset += 32;
                }
            }
        }

        // setup work needed to draw text with different clips
        canvas->translate(10, 160);
        paint.setTextSize(40);

        // compute the bounds of the text
        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);

        const SkScalar boundsHalfWidth = bounds.width() * SK_ScalarHalf;
        const SkScalar boundsHalfHeight = bounds.height() * SK_ScalarHalf;
        const SkScalar boundsQuarterWidth = boundsHalfWidth * SK_ScalarHalf;
        const SkScalar boundsQuarterHeight = boundsHalfHeight * SK_ScalarHalf;

        SkRect upperLeftClip = SkRect::MakeXYWH(bounds.left(), bounds.top(),
                                                boundsHalfWidth, boundsHalfHeight);
        SkRect lowerRightClip = SkRect::MakeXYWH(bounds.centerX(), bounds.centerY(),
                                                 boundsHalfWidth, boundsHalfHeight);
        SkRect interiorClip = bounds;
        interiorClip.inset(boundsQuarterWidth, boundsQuarterHeight);

        const SkRect clipRects[] = { bounds, upperLeftClip, lowerRightClip, interiorClip };

        SkPaint clipHairline;
        clipHairline.setColor(SK_ColorWHITE);
        clipHairline.setStyle(SkPaint::kStroke_Style);

        for (size_t x = 0; x < sizeof(clipRects) / sizeof(SkRect); ++x) {
            canvas->save();
            canvas->drawRect(clipRects[x], clipHairline);
            paint.setAlpha(0x20);
            canvas->drawText(text, strlen(text), 0, 0, paint);
            canvas->clipRect(clipRects[x]);
            paint.setAlpha(0xFF);
            canvas->drawText(text, strlen(text), 0, 0, paint);
            canvas->restore();
            canvas->translate(0, bounds.height() + SkIntToScalar(25));
        }
    }

private:
    SkTypeface* fTypeface;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ColorEmojiGM; }
static GMRegistry reg(MyFactory);

}
