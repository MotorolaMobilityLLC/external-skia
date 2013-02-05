
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkShader.h"
#include "SkString.h"

static void create_gradient(SkBitmap* bm) {
    SkASSERT(1 == bm->width());
    const int height = bm->height();

    float deltaB = 255.0f / height;
    float blue = 255.0f;

    SkAutoLockPixels lock(*bm);
    for (int y = 0; y < height; y++) {
        *bm->getAddr32(0, y) = SkColorSetRGB(0, 0, (U8CPU) blue);
        blue -= deltaB;
    }
}

// Test out the special case of a tiled 1xN texture. Test out opacity,
// filtering and the different tiling modes
class ConstXTileBench : public SkBenchmark {
    SkPaint             fPaint;
    SkString            fName;
    bool                fDoFilter;
    bool                fDoTrans;
    bool                fDoScale;
    enum { N = SkBENCHLOOP(20) };
    static const int kWidth = 1;
    static const int kHeight = 300;

public:
    ConstXTileBench(void* param,
                    SkShader::TileMode xTile,
                    SkShader::TileMode yTile,
                    bool doFilter,
                    bool doTrans,
                    bool doScale)
        : INHERITED(param)
        , fDoFilter(doFilter)
        , fDoTrans(doTrans)
        , fDoScale(doScale) {
        SkBitmap bm;

        bm.setConfig(SkBitmap::kARGB_8888_Config, kWidth, kHeight);

        bm.allocPixels();
        bm.eraseColor(SK_ColorWHITE);
        bm.setIsOpaque(true);

        create_gradient(&bm);

        SkShader* s = SkShader::CreateBitmapShader(bm, xTile, yTile);
        fPaint.setShader(s)->unref();

        fName.printf("constXTile_");

        static const char* gTileModeStr[SkShader::kTileModeCount] = { "C", "R", "M" };
        fName.append(gTileModeStr[xTile]);
        fName.append(gTileModeStr[yTile]);

        if (doFilter) {
            fName.append("_filter");
        }

        if (doTrans) {
            fName.append("_trans");
        }

        if (doScale) {
            fName.append("_scale");
        }
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);
        paint.setFilterBitmap(fDoFilter);
        if (fDoTrans) {
            paint.setColor(SkColorSetARGBMacro(0x80, 0xFF, 0xFF, 0xFF));
        }

        SkRect r;

        if (fDoScale) {
            r = SkRect::MakeWH(SkIntToScalar(2 * 640), SkIntToScalar(2 * 480));
            canvas->scale(SK_ScalarHalf, SK_ScalarHalf);
        } else {
            r = SkRect::MakeWH(SkIntToScalar(640), SkIntToScalar(480));
        }

        SkPaint bgPaint;
        bgPaint.setColor(SK_ColorWHITE);

        for (int i = 0; i < N; i++) {
            if (fDoTrans) {
                canvas->drawRect(r, bgPaint);
            }

            canvas->drawRect(r, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

DEF_BENCH(return new ConstXTileBench(p, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode, false, false, true))
DEF_BENCH(return new ConstXTileBench(p, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, false, false, false))
DEF_BENCH(return new ConstXTileBench(p, SkShader::kMirror_TileMode, SkShader::kMirror_TileMode, false, false, true))

DEF_BENCH(return new ConstXTileBench(p, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode, true, false, false))
DEF_BENCH(return new ConstXTileBench(p, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, true, false, true))
DEF_BENCH(return new ConstXTileBench(p, SkShader::kMirror_TileMode, SkShader::kMirror_TileMode, true, false, false))

DEF_BENCH(return new ConstXTileBench(p, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode, false, true, true))
DEF_BENCH(return new ConstXTileBench(p, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, false, true, false))
DEF_BENCH(return new ConstXTileBench(p, SkShader::kMirror_TileMode, SkShader::kMirror_TileMode, false, true, true))

DEF_BENCH(return new ConstXTileBench(p, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode, true, true, false))
DEF_BENCH(return new ConstXTileBench(p, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, true, true, true))
DEF_BENCH(return new ConstXTileBench(p, SkShader::kMirror_TileMode, SkShader::kMirror_TileMode, true, true, false))
