/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkBBoxHierarchy.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"

#include "Test.h"

class PictureBBHTestBase {
public:
    PictureBBHTestBase(int playbackWidth, int playbackHeight,
        int recordWidth, int recordHeight) {

        fResultBitmap.allocN32Pixels(playbackWidth, playbackHeight);
        fPictureWidth = recordWidth;
        fPictureHeight = recordHeight;
    }

    virtual ~PictureBBHTestBase() { }

    virtual void doTest(SkCanvas& playbackCanvas, SkCanvas& recordingCanvas) = 0;

    void run(skiatest::Reporter* reporter) {
        // No BBH
        this->run(NULL, reporter);

        // With a Tile Grid
        SkTileGridFactory::TileGridInfo gridInfo;
        gridInfo.fMargin.setEmpty();
        gridInfo.fOffset.setZero();
        gridInfo.fTileInterval.set(1, 1);
        SkTileGridFactory gridFactory(gridInfo);
        this->run(&gridFactory, reporter);

        // With an R-Tree
        SkRTreeFactory RTreeFactory;
        this->run(&RTreeFactory, reporter);
    }

private:
    void run(SkBBHFactory* factory, skiatest::Reporter* reporter) {
        SkCanvas playbackCanvas(fResultBitmap);
        playbackCanvas.clear(SK_ColorGREEN);
        SkPictureRecorder recorder;
        SkCanvas* recordCanvas = recorder.beginRecording(SkIntToScalar(fPictureWidth),
                                                         SkIntToScalar(fPictureHeight),
                                                         factory);
        this->doTest(playbackCanvas, *recordCanvas);
        SkAutoTUnref<SkPicture> picture(recorder.endRecording());
        playbackCanvas.drawPicture(picture);
        REPORTER_ASSERT(reporter, SK_ColorGREEN == fResultBitmap.getColor(0, 0));
    }

    SkBitmap fResultBitmap;
    int fPictureWidth, fPictureHeight;
};

// Test to verify the playback of an empty picture
// 
class DrawEmptyPictureBBHTest : public PictureBBHTestBase {
public:
    DrawEmptyPictureBBHTest()
        : PictureBBHTestBase(2, 2, 1, 1) { }
    virtual ~DrawEmptyPictureBBHTest() { }

    virtual void doTest(SkCanvas&, SkCanvas&) SK_OVERRIDE { }
};

// Test to verify the playback of a picture into a canvas that has
// an empty clip.
//
class EmptyClipPictureBBHTest : public PictureBBHTestBase {
public:
    EmptyClipPictureBBHTest() 
        : PictureBBHTestBase(2, 2, 3, 3) { }

    virtual void doTest(SkCanvas& playbackCanvas, SkCanvas& recordingCanvas) SK_OVERRIDE {
        // intersect with out of bounds rect -> empty clip.
        playbackCanvas.clipRect(SkRect::MakeXYWH(SkIntToScalar(10), SkIntToScalar(10),
            SkIntToScalar(1), SkIntToScalar(1)), SkRegion::kIntersect_Op);
        SkPaint paint;
        recordingCanvas.drawRect(SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
            SkIntToScalar(3), SkIntToScalar(3)), paint);
    }

    virtual ~EmptyClipPictureBBHTest() { }
};

DEF_TEST(PictureBBH, reporter) {

    DrawEmptyPictureBBHTest emptyPictureTest;
    emptyPictureTest.run(reporter);

    EmptyClipPictureBBHTest emptyClipPictureTest;
    emptyClipPictureTest.run(reporter);
}

static void test_clear(skiatest::Reporter* r, SkBBHFactory* factory) {
    // SkPicture should always call clear()s on the target canvas, even if its clip is empty.
    SkPictureRecorder src, dst;

    // A picture that's just clear().
    src.beginRecording(1,1, factory)
        ->clear(SK_ColorGREEN);
    SkAutoTDelete<SkPicture> srcPic(src.endRecording());

    // A target canvas with an empty clip.
    SkCanvas* c = dst.beginRecording(1,1, NULL);
        c->clipRect(SkRect::MakeEmpty());
        srcPic->playback(c);
    SkAutoTDelete<SkPicture> dstPic(dst.endRecording());

    // Should be Clip - Save - Clear - Restore.
    // Buggy implementations might return 1 (just Clip) or 3 (Clip - Save - Restore).
    REPORTER_ASSERT(r, dstPic->approximateOpCount() == 4);
}

DEF_TEST(PictureBBH_Clear, r) {
    test_clear(r, NULL);

    SkTileGridFactory::TileGridInfo grid = { {1,1}, {0,0}, {0,0} };
    SkTileGridFactory tilegrid(grid);
    test_clear(r, &tilegrid);

    SkRTreeFactory rtree;
    test_clear(r, &rtree);
}
