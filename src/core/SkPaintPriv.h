/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintPriv_DEFINED
#define SkPaintPriv_DEFINED

#include "SkTypes.h"

class SkBitmap;
class SkImage;
class SkPaint;

class SkPaintPriv {
public:
    /**
     *  Returns true if drawing with this paint (or NULL) will ovewrite all affected pixels.
     *
     *  Note: returns conservative true, meaning it may return false even though the paint might
     *        in fact overwrite its pixels.
     */
    static bool Overwrites(const SkPaint& paint);

    /**
     *  Returns true if drawing this bitmap with this paint (or NULL) will ovewrite all affected
     *  pixels.
     */
    static bool Overwrites(const SkBitmap&, const SkPaint* paint);

    /**
     *  Returns true if drawing this image with this paint (or NULL) will ovewrite all affected
     *  pixels.
     */
    static bool Overwrites(const SkImage*, const SkPaint* paint);
};

#endif
