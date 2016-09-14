/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCanvas.h"
#include "SkPipe.h"
#include "SkPaint.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "Test.h"

#include "SkNullCanvas.h"
#include "SkAutoPixmapStorage.h"

static void drain(SkPipeDeserializer* deserial, SkDynamicMemoryWStream* stream) {
    std::unique_ptr<SkCanvas> canvas(SkCreateNullCanvas());
    sk_sp<SkData> data = stream->detachAsData();
    deserial->playback(data->data(), data->size(), canvas.get());
}

static sk_sp<SkImage> drain_as_image(SkPipeDeserializer* deserial, SkDynamicMemoryWStream* stream) {
    sk_sp<SkData> data = stream->detachAsData();
    return deserial->readImage(data->data(), data->size());
}

static bool deep_equal(SkImage* a, SkImage* b) {
    if (a->width() != b->width() || a->height() != b->height()) {
        return false;
    }

    const SkImageInfo info = SkImageInfo::MakeN32Premul(a->width(), a->height());
    SkAutoPixmapStorage pmapA, pmapB;
    pmapA.alloc(info);
    pmapB.alloc(info);

    if (!a->readPixels(pmapA, 0, 0) || !b->readPixels(pmapB, 0, 0)) {
        return false;
    }

    for (int y = 0; y < info.height(); ++y) {
        if (memcmp(pmapA.addr32(0, y), pmapB.addr32(0, y), info.width() * sizeof(SkPMColor))) {
            return false;
        }
    }
    return true;
}

DEF_TEST(Pipe_image_draw_first, reporter) {
    sk_sp<SkImage> img = GetResourceAsImage("mandrill_128.png");
    SkASSERT(img.get());

    SkPipeSerializer serializer;
    SkPipeDeserializer deserializer;

    SkDynamicMemoryWStream stream;
    SkCanvas* wc = serializer.beginWrite(SkRect::MakeWH(100, 100), &stream);
    wc->drawImage(img, 0, 0, nullptr);
    serializer.endWrite();
    size_t offset0 = stream.bytesWritten();
    REPORTER_ASSERT(reporter, offset0 > 100);   // the raw image must be sorta big
    drain(&deserializer, &stream);

    // try drawing the same image again -- it should be much smaller
    wc = serializer.beginWrite(SkRect::MakeWH(100, 100), &stream);
    wc->drawImage(img, 0, 0, nullptr);
    size_t offset1 = stream.bytesWritten();
    serializer.endWrite();
    REPORTER_ASSERT(reporter, offset1 <= 32);
    drain(&deserializer, &stream);

    // try serializing the same image directly, again it should be small
    serializer.write(img.get(), &stream);
    size_t offset2 = stream.bytesWritten();
    REPORTER_ASSERT(reporter, offset2 <= 32);
    auto img1 = drain_as_image(&deserializer, &stream);
    REPORTER_ASSERT(reporter, deep_equal(img.get(), img1.get()));

    // try serializing the same image directly (again), check that it is the same!
    serializer.write(img.get(), &stream);
    size_t offset3 = stream.bytesWritten();
    REPORTER_ASSERT(reporter, offset3 <= 32);
    auto img2 = drain_as_image(&deserializer, &stream);
    REPORTER_ASSERT(reporter, img1.get() == img2.get());
}

DEF_TEST(Pipe_image_draw_second, reporter) {
    sk_sp<SkImage> img = GetResourceAsImage("mandrill_128.png");
    SkASSERT(img.get());

    SkPipeSerializer serializer;
    SkPipeDeserializer deserializer;
    SkDynamicMemoryWStream stream;

    serializer.write(img.get(), &stream);
    size_t offset0 = stream.bytesWritten();
    REPORTER_ASSERT(reporter, offset0 > 100);   // the raw image must be sorta big
    drain_as_image(&deserializer, &stream);

    // The 2nd image should be nice and small
    serializer.write(img.get(), &stream);
    size_t offset1 = stream.bytesWritten();
    REPORTER_ASSERT(reporter, offset1 <= 32);
    drain_as_image(&deserializer, &stream);

    // Now try drawing the image, it should also be small
    SkCanvas* wc = serializer.beginWrite(SkRect::MakeWH(100, 100), &stream);
    wc->drawImage(img, 0, 0, nullptr);
    serializer.endWrite();
    size_t offset2 = stream.bytesWritten();
    REPORTER_ASSERT(reporter, offset2 <= 32);
}
