/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkMipMap.h"
#include "SkRandom.h"
#include "Test.h"

static void make_bitmap(SkBitmap* bm, SkRandom& rand) {
    // for now, Build needs a min size of 2, otherwise it will return nullptr.
    // should fix that to support 1 X N, where N > 1 to return non-null.
    int w = 2 + rand.nextU() % 1000;
    int h = 2 + rand.nextU() % 1000;
    bm->allocN32Pixels(w, h);
    bm->eraseColor(SK_ColorWHITE);
}

DEF_TEST(MipMap, reporter) {
    SkBitmap bm;
    SkRandom rand;

    for (int i = 0; i < 500; ++i) {
        make_bitmap(&bm, rand);
        SkAutoTUnref<SkMipMap> mm(SkMipMap::Build(bm, nullptr));

        REPORTER_ASSERT(reporter, !mm->extractLevel(SK_Scalar1, nullptr));
        REPORTER_ASSERT(reporter, !mm->extractLevel(SK_Scalar1 * 2, nullptr));

        SkMipMap::Level prevLevel;
        sk_bzero(&prevLevel, sizeof(prevLevel));

        SkScalar scale = SK_Scalar1;
        for (int j = 0; j < 30; ++j) {
            scale = scale * 2 / 3;

            SkMipMap::Level level;
            if (mm->extractLevel(scale, &level)) {
                REPORTER_ASSERT(reporter, level.fPixels);
                REPORTER_ASSERT(reporter, level.fWidth > 0);
                REPORTER_ASSERT(reporter, level.fHeight > 0);
                REPORTER_ASSERT(reporter, level.fRowBytes >= level.fWidth * 4);

                if (prevLevel.fPixels) {
                    REPORTER_ASSERT(reporter, level.fWidth <= prevLevel.fWidth);
                    REPORTER_ASSERT(reporter, level.fHeight <= prevLevel.fHeight);
                }
                prevLevel = level;
            }
        }
    }
}
