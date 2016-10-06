/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkRSXform.h"
#include "SkSurface.h"

class DrawAtlasGM : public skiagm::GM {
    static sk_sp<SkImage> MakeAtlas(SkCanvas* caller, const SkRect& target) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
        auto surface(caller->makeSurface(info));
        if (nullptr == surface) {
            surface = SkSurface::MakeRaster(info);
        }
        SkCanvas* canvas = surface->getCanvas();
        // draw red everywhere, but we don't expect to see it in the draw, testing the notion
        // that drawAtlas draws a subset-region of the atlas.
        canvas->clear(SK_ColorRED);

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kClear);
        SkRect r(target);
        r.inset(-1, -1);
        // zero out a place (with a 1-pixel border) to land our drawing.
        canvas->drawRect(r, paint);
        paint.setBlendMode(SkBlendMode::kSrcOver);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(true);
        canvas->drawOval(target, paint);
        return surface->makeImageSnapshot();
    }

public:
    DrawAtlasGM() {}

protected:

    SkString onShortName() override {
        return SkString("draw-atlas");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkRect target = { 50, 50, 80, 90 };
        auto atlas = MakeAtlas(canvas, target);

        const struct {
            SkScalar fScale;
            SkScalar fDegrees;
            SkScalar fTx;
            SkScalar fTy;

            void apply(SkRSXform* xform) const {
                const SkScalar rad = SkDegreesToRadians(fDegrees);
                xform->fSCos = fScale * SkScalarCos(rad);
                xform->fSSin = fScale * SkScalarSin(rad);
                xform->fTx   = fTx;
                xform->fTy   = fTy;
            }
        } rec[] = {
            { 1, 0, 10, 10 },       // just translate
            { 2, 0, 110, 10 },      // scale + translate
            { 1, 30, 210, 10 },     // rotate + translate
            { 2, -30, 310, 30 },    // scale + rotate + translate
        };

        const int N = SK_ARRAY_COUNT(rec);
        SkRSXform xform[N];
        SkRect tex[N];
        SkColor colors[N];

        for (int i = 0; i < N; ++i) {
            rec[i].apply(&xform[i]);
            tex[i] = target;
            colors[i] = 0x80FF0000 + (i * 40 * 256);
        }

        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);
        paint.setAntiAlias(true);

        canvas->drawAtlas(atlas.get(), xform, tex, N, nullptr, &paint);
        canvas->translate(0, 100);
        canvas->drawAtlas(atlas.get(), xform, tex, colors, N, SkXfermode::kSrcIn_Mode, nullptr, &paint);
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new DrawAtlasGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkPath.h"
#include "SkPathMeasure.h"

static void draw_text_on_path_rigid(SkCanvas* canvas, const void* text, size_t length,
                                    const SkPoint xy[], const SkPath& path, const SkPaint& paint) {
    SkPathMeasure meas(path, false);

    int count = paint.countText(text, length);
    size_t size = count * (sizeof(SkRSXform) + sizeof(SkScalar));
    SkAutoSMalloc<512> storage(size);
    SkRSXform* xform = (SkRSXform*)storage.get();
    SkScalar* widths = (SkScalar*)(xform + count);

    paint.getTextWidths(text, length, widths);

    for (int i = 0; i < count; ++i) {
        // we want to position each character on the center of its advance
        const SkScalar offset = SkScalarHalf(widths[i]);
        SkPoint pos;
        SkVector tan;
        if (!meas.getPosTan(xy[i].x() + offset, &pos, &tan)) {
            pos = xy[i];
            tan.set(1, 0);
        }
        xform[i].fSCos = tan.x();
        xform[i].fSSin = tan.y();
        xform[i].fTx   = pos.x() - tan.y() * xy[i].y() - tan.x() * offset;
        xform[i].fTy   = pos.y() + tan.x() * xy[i].y() - tan.y() * offset;
    }

    // Compute a conservative bounds so we can cull the draw
    const SkRect font = paint.getFontBounds();
    const SkScalar max = SkTMax(SkTMax(SkScalarAbs(font.fLeft), SkScalarAbs(font.fRight)),
                                SkTMax(SkScalarAbs(font.fTop), SkScalarAbs(font.fBottom)));
    const SkRect bounds = path.getBounds().makeOutset(max, max);

    canvas->drawTextRSXform(text, length, &xform[0], &bounds, paint);

    if (true) {
        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(bounds, p);
    }
}

DEF_SIMPLE_GM(drawTextRSXform, canvas, 860, 860) {
    const char text0[] = "ABCDFGHJKLMNOPQRSTUVWXYZ";
    const int N = sizeof(text0) - 1;
    SkPoint pos[N];

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(100);

    SkScalar x = 0;
    for (int i = 0; i < N; ++i) {
        pos[i].set(x, 0);
        x += paint.measureText(&text0[i], 1);
    }

    SkPath path;
    path.addOval(SkRect::MakeXYWH(160, 160, 540, 540));

    draw_text_on_path_rigid(canvas, text0, N, pos, path, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}


