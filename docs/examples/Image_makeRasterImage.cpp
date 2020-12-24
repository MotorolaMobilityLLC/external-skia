// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=aed5f399915d40bb5d133ab586e5bac3
REG_FIDDLE(Image_makeRasterImage, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext) {
        return;
    }

    auto drawImage = [=](sk_sp<SkImage> image, const char* label) -> void {
        if (nullptr == image) {
            return;
        }
        SkPaint paint;
        paint.setAntiAlias(true);
        SkFont font;
        sk_sp<SkImage> raster(image->makeRasterImage());
        canvas->drawImage(raster, 0, 0);
        canvas->drawString(label, 20, raster->height() / 4, font, paint);
    };
    sk_sp<SkImage> bitmapImage(source.asImage());
    sk_sp<SkImage> textureImage(SkImage::MakeFromTexture(dContext, backEndTexture,
                                                         kTopLeft_GrSurfaceOrigin,
                                                         kRGBA_8888_SkColorType,
                                                         kOpaque_SkAlphaType, nullptr));
    drawImage(image, "image");
    canvas->translate(image->width(), 0);
    drawImage(bitmapImage, "source");
    canvas->translate(-image->width(), image->height());
    drawImage(textureImage, "backEndTexture");
}
}  // END FIDDLE
