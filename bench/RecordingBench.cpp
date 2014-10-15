/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "RecordingBench.h"

#include "SkBBHFactory.h"
#include "SkPictureRecorder.h"

static const int kTileSize = 256;

RecordingBench::RecordingBench(const char* name, const SkPicture* pic, bool useBBH)
    : fSrc(SkRef(pic))
    , fName(name)
    , fUseBBH(useBBH) {}

const char* RecordingBench::onGetName() {
    return fName.c_str();
}

bool RecordingBench::isSuitableFor(Backend backend) {
    return backend == kNonRendering_Backend;
}

SkIPoint RecordingBench::onGetSize() {
    return SkIPoint::Make(SkScalarCeilToInt(fSrc->cullRect().width()),
                          SkScalarCeilToInt(fSrc->cullRect().height()));
}

void RecordingBench::onDraw(const int loops, SkCanvas*) {
    SkTileGridFactory::TileGridInfo info;
    info.fTileInterval.set(kTileSize, kTileSize);
    info.fMargin.setEmpty();
    info.fOffset.setZero();
    SkTileGridFactory factory(info);

    const SkScalar w = fSrc->cullRect().width(),
                   h = fSrc->cullRect().height();

    for (int i = 0; i < loops; i++) {
        SkPictureRecorder recorder;
        fSrc->playback(recorder.beginRecording(w, h, fUseBBH ? &factory : NULL));
        SkDELETE(recorder.endRecording());
    }
}
