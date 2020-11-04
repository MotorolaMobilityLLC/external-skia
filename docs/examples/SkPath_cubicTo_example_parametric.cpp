// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkPath_cubicTo_example_parametric, 512, 512, false, 0) {
/*
        If the starting point is (x0, y0), then this curve is defined as the
        paramentric curve as `t` goes from 0 to 1:
          s := 1 - t
          x := (s * s * s * x0) +
               (3 * s * s * t * x1) +
               (3 * s * t * t * x2) +
               (t * t * t * x3)
          y := (s * s * s * y0) +
               (3 * s * s * t * y1) +
               (3 * s * t * t * y2) +
               (t * t * t * y3)

*/

SkPoint cubic(SkPoint p0, SkPoint p1, SkPoint p2, SkPoint p3, float t) {
    float s = 1 - t;
    return {(s * s * s * p0.x()) + (3 * s * s * t * p1.x()) + (3 * s * t * t * p2.x()) +
                    (t * t * t * p3.x()),
            (s * s * s * p0.y()) + (3 * s * s * t * p1.y()) + (3 * s * t * t * p2.y()) +
                    (t * t * t * p3.y())};
}

void draw(SkCanvas* canvas) {
    canvas->clear(SkColorSetARGB(255, 255, 255, 255));
    SkFont font(nullptr, 32);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(5);

    SkPoint a{136, 64};
    SkPoint b{448, 448};
    SkPoint c{64, 448};
    SkPoint d{376, 64};

    SkPath threeSegments;
    threeSegments.moveTo(a);
    threeSegments.lineTo(b);
    threeSegments.lineTo(c);
    threeSegments.lineTo(d);

    canvas->drawPath(threeSegments, paint);

    paint.setColor(SkColorSetARGB(255, 0, 0, 255));
    SkPath cubicCurve;
    cubicCurve.moveTo(a);
    cubicCurve.cubicTo(b, c, d);
    canvas->drawPath(cubicCurve, paint);

    SkPaint textPaint;
    textPaint.setColor(SkColorSetARGB(255, 0, 255, 0));
    textPaint.setAntiAlias(true);
    canvas->drawString("a", a.x(), a.y(), font, textPaint);
    canvas->drawString("b", b.x(), b.y(), font, textPaint);
    canvas->drawString("c", c.x() - 20, c.y(), font, textPaint);
    canvas->drawString("d", d.x(), d.y(), font, textPaint);

    SkPaint pointPaint;
    pointPaint.setAntiAlias(true);
    pointPaint.setStrokeWidth(8);
    pointPaint.setStrokeCap(SkPaint::kRound_Cap);
    pointPaint.setColor(SkColorSetARGB(255, 0, 255, 0));
    const int N = 16;
    for (int i = 0; i <= N; ++i) {
        SkPoint p = cubic(a, b, c, d, (float)i / N);
        canvas->drawPoint(p.x(), p.y(), pointPaint);
    }
    pointPaint.setColor(SkColorSetARGB(255, 255, 0, 0));
}
}  // END FIDDLE
