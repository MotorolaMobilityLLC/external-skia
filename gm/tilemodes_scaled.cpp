
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTypeface.h"
#include "SkBlurMask.h"

// effects
#include "SkGradientShader.h"
#include "SkBlurDrawLooper.h"

static void makebm(SkBitmap* bm, SkColorType ct, int w, int h) {
    bm->allocPixels(SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType));
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas    canvas(*bm);
    SkPoint     pts[] = { { 0, 0 }, { SkIntToScalar(w), SkIntToScalar(h)} };
    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;

    paint.setDither(true);
    paint.setShader(SkGradientShader::CreateLinear(pts, colors, pos,
                SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode))->unref();
    canvas.drawPaint(paint);
}

static void setup(SkPaint* paint, const SkBitmap& bm, SkPaint::FilterLevel filter_level,
                  SkShader::TileMode tmx, SkShader::TileMode tmy) {
    SkShader* shader = SkShader::CreateBitmapShader(bm, tmx, tmy);
    paint->setShader(shader)->unref();
    paint->setFilterLevel(filter_level);
}

static const SkColorType gColorTypes[] = {
    kN32_SkColorType,
    kRGB_565_SkColorType,
};

class ScaledTilingGM : public skiagm::GM {
    SkAutoTUnref<SkBlurDrawLooper> fLooper;
public:
    ScaledTilingGM(bool powerOfTwoSize)
            : fLooper(SkBlurDrawLooper::Create(0x88000000,
                                               SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(1)),
                                               SkIntToScalar(2), SkIntToScalar(2)))
            , fPowerOfTwoSize(powerOfTwoSize) {
    }

    SkBitmap    fTexture[SK_ARRAY_COUNT(gColorTypes)];

protected:
    enum {
        kPOTSize = 4,
        kNPOTSize = 3,
    };

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        if (!fPowerOfTwoSize) {
            return kSkipTiled_Flag;  // Only for 565.  8888 is fine.
        }
        return 0;
    }

    SkString onShortName() {
        SkString name("scaled_tilemodes");
        if (!fPowerOfTwoSize) {
            name.append("_npot");
        }
        return name;
    }

    SkISize onISize() { return SkISize::Make(880, 760); }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        int size = fPowerOfTwoSize ? kPOTSize : kNPOTSize;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gColorTypes); i++) {
            makebm(&fTexture[i], gColorTypes[i], size, size);
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        float scale = 32.f/kPOTSize;

        int size = fPowerOfTwoSize ? kPOTSize : kNPOTSize;

        SkRect r = { 0, 0, SkIntToScalar(size*2), SkIntToScalar(size*2) };

        static const char* gColorTypeNames[] = { "8888" , "565", "4444" };

        static const SkPaint::FilterLevel           gFilterLevels[] =
            { SkPaint::kNone_FilterLevel,
              SkPaint::kLow_FilterLevel,
              SkPaint::kMedium_FilterLevel,
              SkPaint::kHigh_FilterLevel };
        static const char*          gFilterNames[] = { "None", "Low", "Medium", "High" };

        static const SkShader::TileMode gModes[] = { SkShader::kClamp_TileMode, SkShader::kRepeat_TileMode, SkShader::kMirror_TileMode };
        static const char*          gModeNames[] = {    "C",                    "R",                   "M" };

        SkScalar y = SkIntToScalar(24);
        SkScalar x = SkIntToScalar(10)/scale;

        for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
            for (size_t ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
                SkPaint p;
                SkString str;
                p.setAntiAlias(true);
                p.setDither(true);
                p.setLooper(fLooper);
                str.printf("[%s,%s]", gModeNames[kx], gModeNames[ky]);

                p.setTextAlign(SkPaint::kCenter_Align);
                canvas->drawText(str.c_str(), str.size(), scale*(x + r.width()/2), y, p);

                x += r.width() * 4 / 3;
            }
        }

        y = SkIntToScalar(40) / scale;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gColorTypes); i++) {
            for (size_t j = 0; j < SK_ARRAY_COUNT(gFilterLevels); j++) {
                x = SkIntToScalar(10)/scale;
                for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
                    for (size_t ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
                        SkPaint paint;
#if 1 // Temporary change to regen bitmap before each draw. This may help tracking down an issue
      // on SGX where resizing NPOT textures to POT textures exhibits a driver bug.
                        if (!fPowerOfTwoSize) {
                            makebm(&fTexture[i], gColorTypes[i], size, size);
                        }
#endif
                        setup(&paint, fTexture[i], gFilterLevels[j], gModes[kx], gModes[ky]);
                        paint.setDither(true);

                        canvas->save();
                        canvas->scale(scale,scale);
                        canvas->translate(x, y);
                        canvas->drawRect(r, paint);
                        canvas->restore();

                        x += r.width() * 4 / 3;
                    }
                }
                {
                    SkPaint p;
                    SkString str;
                    p.setAntiAlias(true);
                    p.setLooper(fLooper);
                    str.printf("%s, %s", gColorTypeNames[i], gFilterNames[j]);
                    canvas->drawText(str.c_str(), str.size(), scale*x, scale*(y + r.height() * 2 / 3), p);
                }

                y += r.height() * 4 / 3;
            }
        }
    }

private:
    bool fPowerOfTwoSize;
    typedef skiagm::GM INHERITED;
};

static const int gWidth = 32;
static const int gHeight = 32;

static SkShader* make_bm(SkShader::TileMode tx, SkShader::TileMode ty) {
    SkBitmap bm;
    makebm(&bm, kN32_SkColorType, gWidth, gHeight);
    return SkShader::CreateBitmapShader(bm, tx, ty);
}

static SkShader* make_grad(SkShader::TileMode tx, SkShader::TileMode ty) {
    SkPoint pts[] = { { 0, 0 }, { SkIntToScalar(gWidth), SkIntToScalar(gHeight)} };
    SkPoint center = { SkIntToScalar(gWidth)/2, SkIntToScalar(gHeight)/2 };
    SkScalar rad = SkIntToScalar(gWidth)/2;
    SkColor colors[] = { 0xFFFF0000, 0xFF0044FF };

    int index = (int)ty;
    switch (index % 3) {
        case 0:
            return SkGradientShader::CreateLinear(pts, colors, NULL, SK_ARRAY_COUNT(colors), tx);
        case 1:
            return SkGradientShader::CreateRadial(center, rad, colors, NULL, SK_ARRAY_COUNT(colors), tx);
        case 2:
            return SkGradientShader::CreateSweep(center.fX, center.fY, colors, NULL, SK_ARRAY_COUNT(colors));
    }

    return NULL;
}

typedef SkShader* (*ShaderProc)(SkShader::TileMode, SkShader::TileMode);

class ScaledTiling2GM : public skiagm::GM {
    ShaderProc fProc;
    SkString   fName;
public:
    ScaledTiling2GM(ShaderProc proc, const char name[]) : fProc(proc) {
        fName.printf("scaled_tilemode_%s", name);
    }

protected:
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return kSkipTiled_Flag;
    }

    SkString onShortName() {
        return fName;
    }

    SkISize onISize() { return SkISize::Make(880, 560); }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->scale(SkIntToScalar(3)/2, SkIntToScalar(3)/2);

        const SkScalar w = SkIntToScalar(gWidth);
        const SkScalar h = SkIntToScalar(gHeight);
        SkRect r = { -w, -h, w*2, h*2 };

        static const SkShader::TileMode gModes[] = {
            SkShader::kClamp_TileMode, SkShader::kRepeat_TileMode, SkShader::kMirror_TileMode
        };
        static const char* gModeNames[] = {
            "Clamp", "Repeat", "Mirror"
        };

        SkScalar y = SkIntToScalar(24);
        SkScalar x = SkIntToScalar(66);

        SkPaint p;
        p.setAntiAlias(true);
        p.setTextAlign(SkPaint::kCenter_Align);

        for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
            SkString str(gModeNames[kx]);
            canvas->drawText(str.c_str(), str.size(), x + r.width()/2, y, p);
            x += r.width() * 4 / 3;
        }

        y += SkIntToScalar(16) + h;
        p.setTextAlign(SkPaint::kRight_Align);

        for (size_t ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
            x = SkIntToScalar(16) + w;

            SkString str(gModeNames[ky]);
            canvas->drawText(str.c_str(), str.size(), x, y + h/2, p);

            x += SkIntToScalar(50);
            for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
                SkPaint paint;
                paint.setShader(fProc(gModes[kx], gModes[ky]))->unref();

                canvas->save();
                canvas->translate(x, y);
                canvas->drawRect(r, paint);
                canvas->restore();

                x += r.width() * 4 / 3;
            }
            y += r.height() * 4 / 3;
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ScaledTilingGM(true); )
DEF_GM( return new ScaledTilingGM(false); )
DEF_GM( return new ScaledTiling2GM(make_bm, "bitmap"); )
DEF_GM( return new ScaledTiling2GM(make_grad, "gradient"); )
