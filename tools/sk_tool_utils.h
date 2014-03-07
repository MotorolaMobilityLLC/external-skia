/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_tool_utils_DEFINED
#define sk_tool_utils_DEFINED

#include "SkCanvas.h"
#include "SkBitmap.h"

namespace sk_tool_utils {

    /**
     *  Return the colorType and alphaType that correspond to the specified Config8888
     */
    void config8888_to_imagetypes(SkCanvas::Config8888, SkColorType*, SkAlphaType*);

    /**
     *  Call canvas->writePixels() by using the pixels from bitmap, but with an info that claims
     *  the pixels are colorType + alphaType
     */
    void write_pixels(SkCanvas*, const SkBitmap&, int x, int y, SkColorType, SkAlphaType);
}

#endif
