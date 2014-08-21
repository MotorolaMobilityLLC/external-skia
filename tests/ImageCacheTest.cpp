 /*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDiscardableMemory.h"
#include "SkScaledImageCache.h"
#include "Test.h"

namespace {
static void* gGlobalAddress;
struct TestingKey : public SkScaledImageCache::Key {
    void*       fPtr;
    intptr_t    fValue;

    TestingKey(intptr_t value) : fPtr(&gGlobalAddress), fValue(value) {
        this->init(sizeof(fPtr) + sizeof(fValue));
    }
};
}

static void make_bm(SkBitmap* bm, int w, int h) {
    bm->allocN32Pixels(w, h);
}

static const int COUNT = 10;
static const int DIM = 256;

static void test_cache(skiatest::Reporter* reporter, SkScaledImageCache& cache,
                       bool testPurge) {
    SkScaledImageCache::ID* id;

    SkBitmap bm[COUNT];

    for (int i = 0; i < COUNT; ++i) {
        make_bm(&bm[i], DIM, DIM);
    }

    for (int i = 0; i < COUNT; ++i) {
        TestingKey key(bm[i].getGenerationID());
        SkBitmap tmp;

        SkScaledImageCache::ID* id = cache.findAndLock(key, &tmp);
        REPORTER_ASSERT(reporter, NULL == id);

        make_bm(&tmp, DIM, DIM);
        id = cache.addAndLock(key, tmp);
        REPORTER_ASSERT(reporter, NULL != id);

        SkBitmap tmp2;
        SkScaledImageCache::ID* id2 = cache.findAndLock(key, &tmp2);
        REPORTER_ASSERT(reporter, id == id2);
        REPORTER_ASSERT(reporter, tmp.pixelRef() == tmp2.pixelRef());
        REPORTER_ASSERT(reporter, tmp.width() == tmp2.width());
        REPORTER_ASSERT(reporter, tmp.height() == tmp2.height());
        cache.unlock(id2);

        cache.unlock(id);
    }

    if (testPurge) {
        // stress test, should trigger purges
        for (size_t i = 0; i < COUNT * 100; ++i) {
            TestingKey key(i);
            SkBitmap tmp;
            make_bm(&tmp, DIM, DIM);

            SkScaledImageCache::ID* id = cache.addAndLock(key, tmp);
            REPORTER_ASSERT(reporter, NULL != id);
            cache.unlock(id);
        }
    }

    // test the originals after all that purging
    for (int i = 0; i < COUNT; ++i) {
        TestingKey key(bm[i].getGenerationID());
        SkBitmap tmp;
        id = cache.findAndLock(key, &tmp);
        if (id) {
            cache.unlock(id);
        }
    }

    cache.setTotalByteLimit(0);
}

#include "SkDiscardableMemoryPool.h"

static SkDiscardableMemoryPool* gPool;
static SkDiscardableMemory* pool_factory(size_t bytes) {
    SkASSERT(gPool);
    return gPool->create(bytes);
}

DEF_TEST(ImageCache, reporter) {
    static const size_t defLimit = DIM * DIM * 4 * COUNT + 1024;    // 1K slop

    {
        SkScaledImageCache cache(defLimit);
        test_cache(reporter, cache, true);
    }
    {
        SkAutoTUnref<SkDiscardableMemoryPool> pool(
                SkDiscardableMemoryPool::Create(defLimit, NULL));
        gPool = pool.get();
        SkScaledImageCache cache(pool_factory);
        test_cache(reporter, cache, true);
    }
    {
        SkScaledImageCache cache(SkDiscardableMemory::Create);
        test_cache(reporter, cache, false);
    }
}

DEF_TEST(ImageCache_doubleAdd, r) {
    // Adding the same key twice should be safe.
    SkScaledImageCache cache(4096);

    SkBitmap original;
    original.allocN32Pixels(40, 40);

    SkBitmap scaled1;
    scaled1.allocN32Pixels(20, 20);

    SkBitmap scaled2;
    scaled2.allocN32Pixels(20, 20);

    TestingKey key(original.getGenerationID());

    SkScaledImageCache::ID* id1 = cache.addAndLock(key, scaled1);
    SkScaledImageCache::ID* id2 = cache.addAndLock(key, scaled2);
    // We don't really care if id1 == id2 as long as unlocking both works.
    cache.unlock(id1);
    cache.unlock(id2);

    SkBitmap tmp;
    // Lookup should return the value that was added last.
    SkScaledImageCache::ID* id = cache.findAndLock(key, &tmp);
    REPORTER_ASSERT(r, NULL != id);
    REPORTER_ASSERT(r, tmp.getGenerationID() == scaled2.getGenerationID());
    cache.unlock(id);
}
