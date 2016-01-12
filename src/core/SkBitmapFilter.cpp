/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapFilter.h"
#include "SkRTConf.h"
#include "SkTypes.h"

#include <string.h>

// These are the per-scanline callbacks that are used when we must resort to
// resampling an image as it is blitted.  Typically these are used only when
// the image is rotated or has some other complex transformation applied.
// Scaled images will usually be rescaled directly before rasterization.

SK_CONF_DECLARE(const char *, c_bitmapFilter, "bitmap.filter", "mitchell", "Which scanline bitmap filter to use [mitchell, lanczos, hamming, gaussian, triangle, box]");

SkBitmapFilter *SkBitmapFilter::Allocate() {
    if (!strcmp(c_bitmapFilter, "mitchell")) {
        return new SkMitchellFilter;
    } else if (!strcmp(c_bitmapFilter, "lanczos")) {
        return new SkLanczosFilter;
    } else if (!strcmp(c_bitmapFilter, "hamming")) {
        return new SkHammingFilter;
    } else if (!strcmp(c_bitmapFilter, "gaussian")) {
        return new SkGaussianFilter(2);
    } else if (!strcmp(c_bitmapFilter, "triangle")) {
        return new SkTriangleFilter;
    } else if (!strcmp(c_bitmapFilter, "box")) {
        return new SkBoxFilter;
    } else {
        SkDEBUGFAIL("Unknown filter type");
    }

    return nullptr;
}

