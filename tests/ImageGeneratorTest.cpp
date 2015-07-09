/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkGraphics.h"
#include "SkImageGenerator.h"
#include "Test.h"

static SkImageGenerator* my_factory(SkData* data) {
    int* ptr = *(int**)data->data();
    *ptr = 1;   // signal that we were called
    return NULL;
}

static void test_imagegenerator_factory(skiatest::Reporter* reporter) {
    int factoryHasBeenCalled = 0;
    int* sentinelPtr = &factoryHasBeenCalled;
    SkData* data = SkData::NewWithCopy(&sentinelPtr, sizeof(sentinelPtr));

    SkImageGenerator* gen;
    REPORTER_ASSERT(reporter, 0 == *sentinelPtr);

    gen = SkImageGenerator::NewFromEncoded(data);
    REPORTER_ASSERT(reporter, NULL == gen);
    REPORTER_ASSERT(reporter, 0 == *sentinelPtr);

    // Test is racy, in that it hopes no other thread is changing this global...
    SkGraphics::ImageGeneratorFromEncodedFactory prev =
                                                SkGraphics::GetImageGeneratorFromEncodedFactory();
    SkGraphics::SetImageGeneratorFromEncodedFactory(my_factory);
    gen = SkImageGenerator::NewFromEncoded(data);
    REPORTER_ASSERT(reporter, NULL == gen);
    REPORTER_ASSERT(reporter, 1 == *sentinelPtr);
    SkGraphics::SetImageGeneratorFromEncodedFactory(prev);
}

class MyImageGenerator : public SkImageGenerator {
public:
    MyImageGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(0, 0)) {}
};

DEF_TEST(ImageGenerator, reporter) {
    MyImageGenerator ig;
    SkISize sizes[3];
    sizes[0] = SkISize::Make(200, 200);
    sizes[1] = SkISize::Make(100, 100);
    sizes[2] = SkISize::Make( 50,  50);
    void*   planes[3] = { NULL };
    size_t  rowBytes[3] = { 0 };
    SkYUVColorSpace colorSpace;

    // Check that the YUV decoding API does not cause any crashes
    ig.getYUV8Planes(sizes, NULL, NULL, &colorSpace);
    ig.getYUV8Planes(sizes, NULL, NULL, NULL);
    ig.getYUV8Planes(sizes, planes, NULL, NULL);
    ig.getYUV8Planes(sizes, NULL, rowBytes, NULL);
    ig.getYUV8Planes(sizes, planes, rowBytes, NULL);
    ig.getYUV8Planes(sizes, planes, rowBytes, &colorSpace);

    int dummy;
    planes[0] = planes[1] = planes[2] = &dummy;
    rowBytes[0] = rowBytes[1] = rowBytes[2] = 250;

    ig.getYUV8Planes(sizes, planes, rowBytes, &colorSpace);

    test_imagegenerator_factory(reporter);
}
