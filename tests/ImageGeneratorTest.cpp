/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageGenerator.h"
#include "Test.h"

DEF_TEST(ImageGenerator, reporter) {
    SkImageGenerator ig;
    SkISize sizes[3];
    sizes[0] = SkISize::Make(200, 200);
    sizes[1] = SkISize::Make(100, 100);
    sizes[2] = SkISize::Make( 50,  50);
    void*   planes[3] = { NULL };
    size_t  rowBytes[3] = { 0 };

    // Check that the YUV decoding API does not cause any crashes
    ig.getYUV8Planes(sizes, NULL, NULL);
    ig.getYUV8Planes(sizes, planes, NULL);
    ig.getYUV8Planes(sizes, NULL, rowBytes);
    ig.getYUV8Planes(sizes, planes, rowBytes);

    int dummy;
    planes[0] = planes[1] = planes[2] = &dummy;
    rowBytes[0] = rowBytes[1] = rowBytes[2] = 250;

    ig.getYUV8Planes(sizes, planes, rowBytes);
}
