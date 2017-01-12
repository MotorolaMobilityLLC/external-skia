/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

#define WIDTH 400
#define HEIGHT 400

namespace {
// Test thin stroked rect (stroked "by hand", not by stroking).
void draw_thin_stroked_rect(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(10 + width, 10 + width);
    path.lineTo(40,         10 + width);
    path.lineTo(40,         20);
    path.lineTo(10 + width, 20);
    path.moveTo(10,         10);
    path.lineTo(10,         20 + width);
    path.lineTo(40 + width, 20 + width);
    path.lineTo(40 + width, 10);
    canvas->drawPath(path, paint);
}

};

class ThinConcavePathsGM : public skiagm::GM {
public:
    ThinConcavePathsGM() {}

protected:
    SkString onShortName() override {
        return SkString("thinconcavepaths");
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);

        canvas->save();
        for (SkScalar width = 1.0; width < 2.05; width += 0.25) {
            draw_thin_stroked_rect(canvas, paint, width);
            canvas->translate(0, 25);
        }
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new ThinConcavePathsGM; )
