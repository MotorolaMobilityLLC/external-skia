/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Define NAME_WRAP(x) before including this header to perform name-wrapping
// E.g. for ARM NEON, defined it as 'x ## _neon' to ensure all important
// identifiers have a _neon suffix.
#ifndef NAME_WRAP
    #error "Please define NAME_WRAP() before including this file"
#endif

#define MAKENAME(suffix)        NAME_WRAP(S32_alpha_D32 ## suffix)
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(4 == state.fPixmap.info().bytesPerPixel()); \
                                SkASSERT(state.fAlphaScale <= 256)
#define PREAMBLE(state)         unsigned alphaScale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(src, alphaScale)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

#undef NAME_WRAP
