#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b9026d7f39029756bd7cab9542c64f4e
REG_FIDDLE(ImageInfo_021, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul({18, 18}));
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(15);
    offscreen.drawString("\xF0\x9F\x98\xB9", 1, 15, paint);
    canvas->scale(6, 6);
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
