/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkGradientShader.h"

DEF_SIMPLE_GM(gamma, canvas, 500, 200) {
    SkPaint p;
    const SkScalar sz = 50.0f;
    const int szInt = SkScalarTruncToInt(sz);
    const SkScalar tx = sz + 5.0f;
    const SkRect r = SkRect::MakeXYWH(0, 0, sz, sz);
    SkShader::TileMode rpt = SkShader::kRepeat_TileMode;

    SkBitmap ditherBmp;
    ditherBmp.allocN32Pixels(2, 2);
    SkPMColor* pixels = reinterpret_cast<SkPMColor*>(ditherBmp.getPixels());
    pixels[0] = pixels[3] = SkPackARGB32(0xFF, 0xFF, 0xFF, 0xFF);
    pixels[1] = pixels[2] = SkPackARGB32(0xFF, 0, 0, 0);

    SkBitmap linearGreyBmp;
    SkImageInfo linearGreyInfo = SkImageInfo::MakeN32(szInt, szInt, kOpaque_SkAlphaType,
                                                      kLinear_SkColorProfileType);
    linearGreyBmp.allocPixels(linearGreyInfo);
    linearGreyBmp.eraseARGB(0xFF, 0x7F, 0x7F, 0x7F);

    SkBitmap srgbGreyBmp;
    SkImageInfo srgbGreyInfo = SkImageInfo::MakeN32(szInt, szInt, kOpaque_SkAlphaType,
                                                    kSRGB_SkColorProfileType);
    srgbGreyBmp.allocPixels(srgbGreyInfo);
    srgbGreyBmp.eraseARGB(0xFF, 0xBC, 0xBC, 0xBC);

    SkPaint textPaint;
    textPaint.setColor(SK_ColorWHITE);

    // Helpers:
    auto advance = [&]() {
        canvas->translate(tx, 0);
        p.reset();
    };

    auto nextRect = [&](const char* label, const char* label2) {
        canvas->drawRect(r, p);
        canvas->drawText(label, strlen(label), 0, sz + textPaint.getFontSpacing(), textPaint);
        if (label2) {
            canvas->drawText(label2, strlen(label2), 0, sz + 2 * textPaint.getFontSpacing(),
                             textPaint);
        }
        advance();
    };

    auto nextBitmap = [&](const SkBitmap& bmp, const char* label) {
        canvas->drawBitmap(bmp, 0, 0);
        canvas->drawText(label, strlen(label), 0, sz + textPaint.getFontSpacing(), textPaint);
        advance();
    };

    auto nextXferRect = [&](SkColor srcColor, SkXfermode::Mode mode, SkColor dstColor) {
        p.setColor(dstColor);
        canvas->drawRect(r, p);
        p.setColor(srcColor);
        p.setXfermodeMode(mode);
        canvas->drawRect(r, p);

        SkString srcText = SkStringPrintf("%08X", srcColor);
        SkString dstText = SkStringPrintf("%08X", dstColor);
        canvas->drawText(srcText.c_str(), srcText.size(), 0, sz + textPaint.getFontSpacing(),
                         textPaint);
        const char* modeName = SkXfermode::ModeName(mode);
        canvas->drawText(modeName, strlen(modeName), 0, sz + 2 * textPaint.getFontSpacing(),
                         textPaint);
        canvas->drawText(dstText.c_str(), dstText.size(), 0, sz + 3 * textPaint.getFontSpacing(),
                         textPaint);
        advance();
    };

    // Necessary for certain Xfermodes to work:
    canvas->clear(SK_ColorTRANSPARENT);

    // *Everything* should be perceptually 50% grey. Only the first rectangle
    // is guaranteed to draw that way, though.
    canvas->save();

    // Black/white dither, pixel perfect. This is ground truth.
    p.setShader(SkShader::CreateBitmapShader(
        ditherBmp, rpt, rpt))->unref();
    p.setFilterQuality(SkFilterQuality::kNone_SkFilterQuality);
    nextRect("Dither", "Reference");

    // Black/white dither, sampled at half-texel offset. Tests bilerp.
    // NOTE: We need to apply a non-identity scale and/or rotation to trick
    // the raster pipeline into *not* snapping to nearest.
    SkMatrix offsetMatrix = SkMatrix::Concat(
        SkMatrix::MakeScale(-1.0f), SkMatrix::MakeTrans(0.5f, 0.0f));
    p.setShader(SkShader::CreateBitmapShader(
        ditherBmp, rpt, rpt, &offsetMatrix))->unref();
    p.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
    nextRect("Dither", "Bilerp");

    // Black/white dither, scaled down by 2x. Tests minification.
    SkMatrix scaleMatrix = SkMatrix::MakeScale(0.5f);
    p.setShader(SkShader::CreateBitmapShader(
        ditherBmp, rpt, rpt, &scaleMatrix))->unref();
    p.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
    nextRect("Dither", "Scale");

    // 50% grey via paint color.
    p.setColor(0xff7f7f7f);
    nextRect("Color", 0);

    // Black -> White gradient, scaled to sample just the middle.
    // Tests gradient interpolation.
    SkPoint points[2] = {
        SkPoint::Make(0 - (sz * 10), 0),
        SkPoint::Make(sz + (sz * 10), 0)
    };
    SkColor colors[2] = { SK_ColorBLACK, SK_ColorWHITE };
    p.setShader(SkGradientShader::CreateLinear(
        points, colors, nullptr, 2, SkShader::kClamp_TileMode))->unref();
    nextRect("Gradient", 0);

    // 50% grey from linear bitmap, with drawBitmap
    nextBitmap(linearGreyBmp, "Lnr BMP");

    // 50% grey from sRGB bitmap, with drawBitmap
    nextBitmap(srgbGreyBmp, "sRGB BMP");

    // Bitmap wrapped in a shader (linear):
    p.setShader(SkShader::CreateBitmapShader(linearGreyBmp, rpt, rpt))->unref();
    p.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
    nextRect("Lnr BMP", "Shader");

    // Bitmap wrapped in a shader (sRGB):
    p.setShader(SkShader::CreateBitmapShader(srgbGreyBmp, rpt, rpt))->unref();
    p.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
    nextRect("sRGB BMP", "Shader");

    // Carriage return.
    canvas->restore();
    canvas->translate(0, 2 * sz);

    const U8CPU sqrtHalf = 0xB4;
    const SkColor sqrtHalfAlpha = SkColorSetARGB(sqrtHalf, 0, 0, 0);
    const SkColor sqrtHalfWhite = SkColorSetARGB(0xFF, sqrtHalf, sqrtHalf, sqrtHalf);

    // Xfermode tests.
    nextXferRect(0x7fffffff, SkXfermode::kSrcOver_Mode, SK_ColorBLACK);
    nextXferRect(0x7f000000, SkXfermode::kSrcOver_Mode, SK_ColorWHITE);

    nextXferRect(SK_ColorBLACK, SkXfermode::kDstOver_Mode, 0x7fffffff);
    nextXferRect(SK_ColorWHITE, SkXfermode::kSrcIn_Mode, 0x7fff00ff);
    nextXferRect(0x7fff00ff, SkXfermode::kDstIn_Mode, SK_ColorWHITE);
    nextXferRect(sqrtHalfWhite, SkXfermode::kSrcIn_Mode, sqrtHalfAlpha);
    nextXferRect(sqrtHalfAlpha, SkXfermode::kDstIn_Mode, sqrtHalfWhite);

    nextXferRect(0xff3f3f3f, SkXfermode::kPlus_Mode, 0xff3f3f3f);
    nextXferRect(sqrtHalfWhite, SkXfermode::kModulate_Mode, sqrtHalfWhite);

    canvas->restore();
}
