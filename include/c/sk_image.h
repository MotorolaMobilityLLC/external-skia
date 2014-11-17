/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_image_DEFINED
#define sk_image_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/**
 *  Return a new image that has made a copy of the provided pixels, or NULL on failure.
 *  Balance with a call to sk_image_unref().
 */
sk_image_t* sk_image_new_raster_copy(const sk_imageinfo_t*, const void* pixels, size_t rowBytes);
void sk_image_ref(const sk_image_t*);
void sk_image_unref(const sk_image_t*);
int sk_image_get_width(const sk_image_t*);
int sk_image_get_height(const sk_image_t*);
uint32_t sk_image_get_unique_id(const sk_image_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
