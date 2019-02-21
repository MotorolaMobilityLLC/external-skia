/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkSurface.h"

#ifdef SK_BUILD_FOR_MAC

#import <ApplicationServices/ApplicationServices.h>

static void std_cg_setup(CGContextRef ctx) {
    CGContextSetAllowsFontSubpixelQuantization(ctx, false);
    CGContextSetShouldSubpixelQuantizeFonts(ctx, false);

    // Because CG always draws from the horizontal baseline,
    // if there is a non-integral translation from the horizontal origin to the vertical origin,
    // then CG cannot draw the glyph in the correct location without subpixel positioning.
    CGContextSetAllowsFontSubpixelPositioning(ctx, true);
    CGContextSetShouldSubpixelPositionFonts(ctx, true);

    CGContextSetAllowsFontSmoothing(ctx, true);
    CGContextSetShouldAntialias(ctx, true);

    CGContextSetTextDrawingMode(ctx, kCGTextFill);

    // Draw black on white to create mask. (Special path exists to speed this up in CG.)
    CGContextSetGrayFillColor(ctx, 0.0f, 1.0f);
}

static CGContextRef make_cg_ctx(const SkPixmap& pm) {
    CGBitmapInfo info;
    CGColorSpaceRef cs;

    switch (pm.colorType()) {
        case kRGBA_8888_SkColorType:
            info = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
            cs = CGColorSpaceCreateDeviceRGB();
            break;
        case kGray_8_SkColorType:
            info = kCGImageAlphaNone;
            cs = CGColorSpaceCreateDeviceGray();
            break;
        case kAlpha_8_SkColorType:
            info = kCGImageAlphaOnly;
            cs = nullptr;
            break;
        default:
            return nullptr;
    }
    auto ctx = CGBitmapContextCreate(pm.writable_addr(), pm.width(), pm.height(), 8, pm.rowBytes(),
                                     cs, info);
    std_cg_setup(ctx);
    return ctx;
}

static void test_mac_fonts(SkCanvas* canvas, SkScalar size, SkScalar xpos) {
    int w = 32;
    int h = 32;

    canvas->scale(10, 10);
    SkScalar y = 1;

    for (SkColorType ct : {kRGBA_8888_SkColorType, kGray_8_SkColorType, kAlpha_8_SkColorType}) {
        SkImageInfo ii = SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType);
        auto surf = SkSurface::MakeRaster(ii);
        SkPixmap pm;
        surf->peekPixels(&pm);
        CGContextRef ctx = make_cg_ctx(pm);
        CGContextSelectFont(ctx, "Helvetica", size, kCGEncodingMacRoman);

        SkScalar x = 1;
        for (bool smooth : {false, true}) {
            surf->getCanvas()->clear(ct == kGray_8_SkColorType ? 0xFFFFFFFF : 0);
            CGContextSetShouldSmoothFonts(ctx, smooth);
            CGContextShowTextAtPoint(ctx, 2 + xpos, 2, "A", 1);

            surf->draw(canvas, x, y, nullptr);
            x += pm.width();
        }
        y += pm.height();
    }
}

class MacAAFontsGM : public skiagm::GM {
    SkScalar fSize = 16;
    SkScalar fXPos = 0;

public:
    MacAAFontsGM() {}
    ~MacAAFontsGM() override {}

protected:
    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        test_mac_fonts(canvas, fSize, fXPos);

        return DrawResult::kOk;
    }

    SkISize onISize() override { return { 1024, 768 }; }

    SkString onShortName() override { return SkString("macaatest"); }

    bool onHandleKey(SkUnichar uni) override {
        switch (uni) {
            case 'i': fSize += 1; return true;
            case 'k': fSize -= 1; return true;
            case 'j': fXPos -= 1.0f/16; return true;
            case 'l': fXPos += 1.0f/16; return true;
            default: break;
        }
        return false;
    }
};

DEF_GM(return new MacAAFontsGM;)

#endif

