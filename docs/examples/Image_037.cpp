#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ecdbaff44a02c310ef672b7d393c6dea
REG_FIDDLE(Image_makeNonTextureImage, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    auto drawImage = [=](sk_sp<SkImage> image, const char* label) -> void {
        if (nullptr == image) {
            return;
        }
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_sp<SkImage> nonTexture(image->makeNonTextureImage());
        canvas->drawImage(nonTexture, 0, 0);
        canvas->drawString(label, 20, nonTexture->height() / 4, paint);
    };
    sk_sp<SkImage> bitmapImage(SkImage::MakeFromBitmap(source));
    sk_sp<SkImage> textureImage(SkImage::MakeFromTexture(canvas->getGrContext(), backEndTexture,
                                kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                kOpaque_SkAlphaType, nullptr));
    drawImage(image, "image");
    canvas->translate(image->width(), 0);
    drawImage(bitmapImage, "source");
    canvas->translate(-image->width(), image->height());
    drawImage(textureImage, "backEndTexture");
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
