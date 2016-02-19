 /*
  * Copyright 2016 Google Inc.
  *
  * Use of this source code is governed by a BSD-style license that can be
  * found in the LICENSE file.
  */

#include "Test.h"

#include "SkBitmap.h"
#include "SkImage.h"
#include "SkImageFilter.h"
#include "SkImageFilterCacheKey.h"
#include "SkMatrix.h"
#include "SkSpecialImage.h"

static const int kSmallerSize = 10;
static const int kPad = 3;
static const int kFullSize = kSmallerSize + 2 * kPad;

static SkBitmap create_bm() {
    SkBitmap bm;
    bm.allocN32Pixels(kFullSize, kFullSize, true);
    bm.eraseColor(SK_ColorTRANSPARENT);
    return bm;
}

// Ensure the cache can return a cached image
static void test_find_existing(skiatest::Reporter* reporter,
                               SkSpecialImage* image,
                               SkSpecialImage* subset) {
    static const size_t kCacheSize = 1000000;
    SkAutoTUnref<SkImageFilter::Cache> cache(SkImageFilter::Cache::Create(kCacheSize));

    SkIRect clip = SkIRect::MakeWH(100, 100);
    SkImageFilter::Cache::Key key1(0, SkMatrix::I(), clip, image->uniqueID(), image->subset());
    SkImageFilter::Cache::Key key2(0, SkMatrix::I(), clip, subset->uniqueID(), subset->subset());

    SkIPoint offset = SkIPoint::Make(3, 4);
    cache->set(key1, image, offset);

    SkIPoint foundOffset;

    SkSpecialImage* foundImage = cache->get(key1, &foundOffset);
    REPORTER_ASSERT(reporter, foundImage);
    REPORTER_ASSERT(reporter, offset == foundOffset);

    REPORTER_ASSERT(reporter, !cache->get(key2, &foundOffset));
}

// If either id is different or the clip or the matrix are different the
// cached image won't be found. Even if it is caching the same bitmap.
static void test_dont_find_if_diff_key(skiatest::Reporter* reporter,
                                       SkSpecialImage* image,
                                       SkSpecialImage* subset) {
    static const size_t kCacheSize = 1000000;
    SkAutoTUnref<SkImageFilter::Cache> cache(SkImageFilter::Cache::Create(kCacheSize));

    SkIRect clip1 = SkIRect::MakeWH(100, 100);
    SkIRect clip2 = SkIRect::MakeWH(200, 200);
    SkImageFilter::Cache::Key key0(0, SkMatrix::I(), clip1, image->uniqueID(), image->subset());
    SkImageFilter::Cache::Key key1(1, SkMatrix::I(), clip1, image->uniqueID(), image->subset());
    SkImageFilter::Cache::Key key2(0, SkMatrix::MakeTrans(5, 5), clip1,
                                   image->uniqueID(), image->subset());
    SkImageFilter::Cache::Key key3(0, SkMatrix::I(), clip2, image->uniqueID(), image->subset());
    SkImageFilter::Cache::Key key4(0, SkMatrix::I(), clip1, subset->uniqueID(), subset->subset());

    SkIPoint offset = SkIPoint::Make(3, 4);
    cache->set(key0, image, offset);

    SkIPoint foundOffset;
    REPORTER_ASSERT(reporter, !cache->get(key1, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key2, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key3, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key4, &foundOffset));
}

// Test purging when the max cache size is exceeded
static void test_internal_purge(skiatest::Reporter* reporter, SkSpecialImage* image) {
    SkASSERT(image->getSize());
    static const size_t kCacheSize = image->getSize() + 10;
    SkAutoTUnref<SkImageFilter::Cache> cache(SkImageFilter::Cache::Create(kCacheSize));

    SkIRect clip = SkIRect::MakeWH(100, 100);
    SkImageFilter::Cache::Key key1(0, SkMatrix::I(), clip, image->uniqueID(), image->subset());
    SkImageFilter::Cache::Key key2(1, SkMatrix::I(), clip, image->uniqueID(), image->subset());

    SkIPoint offset = SkIPoint::Make(3, 4);
    cache->set(key1, image, offset);

    SkIPoint foundOffset;

    REPORTER_ASSERT(reporter, cache->get(key1, &foundOffset));

    // This should knock the first one out of the cache
    cache->set(key2, image, offset);

    REPORTER_ASSERT(reporter, cache->get(key2, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key1, &foundOffset));
}

// Exercise the purgeByKeys and purge methods
static void test_explicit_purging(skiatest::Reporter* reporter,
                                  SkSpecialImage* image,
                                  SkSpecialImage* subset) {
    static const size_t kCacheSize = 1000000;
    SkAutoTUnref<SkImageFilter::Cache> cache(SkImageFilter::Cache::Create(kCacheSize));

    SkIRect clip = SkIRect::MakeWH(100, 100);
    SkImageFilter::Cache::Key key1(0, SkMatrix::I(), clip, image->uniqueID(), image->subset());
    SkImageFilter::Cache::Key key2(1, SkMatrix::I(), clip, subset->uniqueID(), image->subset());

    SkIPoint offset = SkIPoint::Make(3, 4);
    cache->set(key1, image, offset);
    cache->set(key2, image, offset);

    SkIPoint foundOffset;

    REPORTER_ASSERT(reporter, cache->get(key1, &foundOffset));
    REPORTER_ASSERT(reporter, cache->get(key2, &foundOffset));

    cache->purgeByKeys(&key1, 1);

    REPORTER_ASSERT(reporter, !cache->get(key1, &foundOffset));
    REPORTER_ASSERT(reporter, cache->get(key2, &foundOffset));

    cache->purge();

    REPORTER_ASSERT(reporter, !cache->get(key1, &foundOffset));
    REPORTER_ASSERT(reporter, !cache->get(key2, &foundOffset));
}

DEF_TEST(ImageFilterCache_RasterBacked, reporter) {
    SkBitmap srcBM = create_bm();

    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    SkAutoTUnref<SkSpecialImage> fullImg(SkSpecialImage::NewFromRaster(nullptr, full, srcBM));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    SkAutoTUnref<SkSpecialImage> subsetImg(SkSpecialImage::NewFromRaster(nullptr, subset, srcBM));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}

DEF_TEST(ImageFilterCache_ImageBacked, reporter) {
    SkBitmap srcBM = create_bm();

    SkAutoTUnref<SkImage> srcImage(SkImage::NewFromBitmap(srcBM));

    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    SkAutoTUnref<SkSpecialImage> fullImg(SkSpecialImage::NewFromImage(full, srcImage));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    SkAutoTUnref<SkSpecialImage> subsetImg(SkSpecialImage::NewFromImage(subset, srcImage));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}

#if SK_SUPPORT_GPU
#include "GrContext.h"

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageFilterCache_GPUBacked, reporter, context) {
    SkBitmap srcBM = create_bm();

    GrSurfaceDesc desc;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fFlags  = kNone_GrSurfaceFlags;
    desc.fWidth  = kFullSize;
    desc.fHeight = kFullSize;

    SkAutoTUnref<GrTexture> srcTexture(context->textureProvider()->createTexture(desc, false,
                                                                                 srcBM.getPixels(),
                                                                                 0));
    if (!srcTexture) {
        return;
    }

    const SkIRect& full = SkIRect::MakeWH(kFullSize, kFullSize);

    SkAutoTUnref<SkSpecialImage> fullImg(SkSpecialImage::NewFromGpu(
                                                                nullptr, full, 
                                                                kNeedNewImageUniqueID_SpecialImage,
                                                                srcTexture));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    SkAutoTUnref<SkSpecialImage> subsetImg(SkSpecialImage::NewFromGpu(
                                                                nullptr, subset, 
                                                                kNeedNewImageUniqueID_SpecialImage,
                                                                srcTexture));

    test_find_existing(reporter, fullImg, subsetImg);
    test_dont_find_if_diff_key(reporter, fullImg, subsetImg);
    test_internal_purge(reporter, fullImg);
    test_explicit_purging(reporter, fullImg, subsetImg);
}
#endif

