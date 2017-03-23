/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkBitmapCache.h"
#include "SkCanvas.h"
#include "SkDiscardableMemoryPool.h"
#include "SkGraphics.h"
#include "SkMipMap.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkResourceCache.h"
#include "SkSurface.h"

////////////////////////////////////////////////////////////////////////////////////////

enum LockedState {
    kNotLocked,
    kLocked,
};

enum CachedState {
    kNotInCache,
    kInCache,
};

static void check_data(skiatest::Reporter* reporter, const SkCachedData* data,
                       int refcnt, CachedState cacheState, LockedState lockedState) {
    REPORTER_ASSERT(reporter, data->testing_only_getRefCnt() == refcnt);
    REPORTER_ASSERT(reporter, data->testing_only_isInCache() == (kInCache == cacheState));
    bool isLocked = (data->data() != nullptr);
    REPORTER_ASSERT(reporter, isLocked == (lockedState == kLocked));
}

static void test_mipmapcache(skiatest::Reporter* reporter, SkResourceCache* cache) {
    cache->purgeAll();

    SkBitmap src;
    src.allocN32Pixels(5, 5);
    src.setImmutable();

    const SkDestinationSurfaceColorMode colorMode = SkDestinationSurfaceColorMode::kLegacy;

    const SkMipMap* mipmap = SkMipMapCache::FindAndRef(SkBitmapCacheDesc::Make(src), colorMode,
                                                       cache);
    REPORTER_ASSERT(reporter, nullptr == mipmap);

    mipmap = SkMipMapCache::AddAndRef(src, colorMode, cache);
    REPORTER_ASSERT(reporter, mipmap);

    {
        const SkMipMap* mm = SkMipMapCache::FindAndRef(SkBitmapCacheDesc::Make(src), colorMode,
                                                       cache);
        REPORTER_ASSERT(reporter, mm);
        REPORTER_ASSERT(reporter, mm == mipmap);
        mm->unref();
    }

    check_data(reporter, mipmap, 2, kInCache, kLocked);

    mipmap->unref();
    // tricky, since technically after this I'm no longer an owner, but since the cache is
    // local, I know it won't get purged behind my back
    check_data(reporter, mipmap, 1, kInCache, kNotLocked);

    // find us again
    mipmap = SkMipMapCache::FindAndRef(SkBitmapCacheDesc::Make(src), colorMode, cache);
    check_data(reporter, mipmap, 2, kInCache, kLocked);

    cache->purgeAll();
    check_data(reporter, mipmap, 1, kNotInCache, kLocked);

    mipmap->unref();
}

static void test_mipmap_notify(skiatest::Reporter* reporter, SkResourceCache* cache) {
    const SkDestinationSurfaceColorMode colorMode = SkDestinationSurfaceColorMode::kLegacy;
    const int N = 3;

    SkBitmap src[N];
    for (int i = 0; i < N; ++i) {
        src[i].allocN32Pixels(5, 5);
        src[i].setImmutable();
        SkMipMapCache::AddAndRef(src[i], colorMode, cache)->unref();
    }

    for (int i = 0; i < N; ++i) {
        const SkMipMap* mipmap = SkMipMapCache::FindAndRef(SkBitmapCacheDesc::Make(src[i]),
                                                           colorMode, cache);
        if (cache) {
            // if cache is null, we're working on the global cache, and other threads might purge
            // it, making this check fragile.
            REPORTER_ASSERT(reporter, mipmap);
        }
        SkSafeUnref(mipmap);

        src[i].reset(); // delete the underlying pixelref, which *should* remove us from the cache

        mipmap = SkMipMapCache::FindAndRef(SkBitmapCacheDesc::Make(src[i]), colorMode, cache);
        REPORTER_ASSERT(reporter, !mipmap);
    }
}

#include "SkDiscardableMemoryPool.h"

static SkDiscardableMemoryPool* gPool = 0;
static SkDiscardableMemory* pool_factory(size_t bytes) {
    SkASSERT(gPool);
    return gPool->create(bytes);
}

static void testBitmapCache_discarded_bitmap(skiatest::Reporter* reporter, SkResourceCache* cache,
                                             SkResourceCache::DiscardableFactory factory) {
    test_mipmapcache(reporter, cache);
    test_mipmap_notify(reporter, cache);
}

DEF_TEST(BitmapCache_discarded_bitmap, reporter) {
    const size_t byteLimit = 100 * 1024;
    {
        SkResourceCache cache(byteLimit);
        testBitmapCache_discarded_bitmap(reporter, &cache, nullptr);
    }
    {
        sk_sp<SkDiscardableMemoryPool> pool(SkDiscardableMemoryPool::Create(byteLimit, nullptr));
        gPool = pool.get();
        SkResourceCache::DiscardableFactory factory = pool_factory;
        SkResourceCache cache(factory);
        testBitmapCache_discarded_bitmap(reporter, &cache, factory);
    }
}

static void test_discarded_image(skiatest::Reporter* reporter, const SkMatrix& transform,
                                 sk_sp<SkImage> (*buildImage)()) {
    auto surface(SkSurface::MakeRasterN32Premul(10, 10));
    SkCanvas* canvas = surface->getCanvas();

    // SkBitmapCache is global, so other threads could be evicting our bitmaps.  Loop a few times
    // to mitigate this risk.
    const unsigned kRepeatCount = 42;
    for (unsigned i = 0; i < kRepeatCount; ++i) {
        SkAutoCanvasRestore acr(canvas, true);

        sk_sp<SkImage> image(buildImage());

        // always use high quality to ensure caching when scaled
        SkPaint paint;
        paint.setFilterQuality(kHigh_SkFilterQuality);

        // draw the image (with a transform, to tickle different code paths) to ensure
        // any associated resources get cached
        canvas->concat(transform);
        canvas->drawImage(image, 0, 0, &paint);

        auto imageId = image->uniqueID();

        // delete the image
        image.reset(nullptr);

        // all resources should have been purged
        SkBitmap result;
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(imageId, &result));
    }
}


// Verify that associated bitmap cache entries are purged on SkImage destruction.
DEF_TEST(BitmapCache_discarded_image, reporter) {
    // Cache entries associated with SkImages fall into two categories:
    //
    // 1) generated image bitmaps (managed by the image cacherator)
    // 2) scaled/resampled bitmaps (cached when HQ filters are used)
    //
    // To exercise the first cache type, we use generated/picture-backed SkImages.
    // To exercise the latter, we draw scaled bitmap images using HQ filters.

    const SkMatrix xforms[] = {
        SkMatrix::MakeScale(1, 1),
        SkMatrix::MakeScale(1.7f, 0.5f),
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(xforms); ++i) {
        test_discarded_image(reporter, xforms[i], []() {
            auto surface(SkSurface::MakeRasterN32Premul(10, 10));
            surface->getCanvas()->clear(SK_ColorCYAN);
            return surface->makeImageSnapshot();
        });

        test_discarded_image(reporter, xforms[i], []() {
            SkPictureRecorder recorder;
            SkCanvas* canvas = recorder.beginRecording(10, 10);
            canvas->clear(SK_ColorCYAN);
            return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(),
                                            SkISize::Make(10, 10), nullptr, nullptr,
                                            SkImage::BitDepth::kU8,
                                            SkColorSpace::MakeSRGB());
        });
    }
}
