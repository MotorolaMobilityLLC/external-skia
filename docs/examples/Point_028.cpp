// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8fe8572685eaa617f25a5a6767a874dc
REG_FIDDLE(Point_notequal_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint test[] = { {0, -0.f}, {-1, -2}, {SK_ScalarInfinity, 1}, {SK_ScalarNaN, -1} };
    for (const SkPoint& pt : test) {
        SkDebugf("pt: %g, %g  %c= pt\n", pt.fX, pt.fY, pt != pt ? '!' : '=');
    }
}
}  // END FIDDLE
