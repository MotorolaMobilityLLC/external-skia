/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureRecord.h"
#include "SkPictureRecorder.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkTypes.h"

SkPictureRecorder::SkPictureRecorder() {}

SkPictureRecorder::~SkPictureRecorder() {}

SkCanvas* SkPictureRecorder::beginRecording(SkScalar width, SkScalar height,
                                            SkBBHFactory* bbhFactory /* = NULL */,
                                            uint32_t recordFlags /* = 0 */) {
    return EXPERIMENTAL_beginRecording(width, height, bbhFactory);
}

SkCanvas* SkPictureRecorder::DEPRECATED_beginRecording(SkScalar width, SkScalar height,
                                                       SkBBHFactory* bbhFactory /* = NULL */,
                                                       uint32_t recordFlags /* = 0 */) {
    SkASSERT(!bbhFactory);  // No longer suppported with this backend.

    fCullWidth = width;
    fCullHeight = height;
    fPictureRecord.reset(SkNEW_ARGS(SkPictureRecord, (SkISize::Make(width, height), recordFlags)));

    fPictureRecord->beginRecording();
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::EXPERIMENTAL_beginRecording(SkScalar width, SkScalar height,
                                                         SkBBHFactory* bbhFactory /* = NULL */) {
    fCullWidth = width;
    fCullHeight = height;

    if (bbhFactory) {
        fBBH.reset((*bbhFactory)(width, height));
        SkASSERT(fBBH.get());
    }

    fRecord.reset(SkNEW(SkRecord));
    fRecorder.reset(SkNEW_ARGS(SkRecorder, (fRecord.get(), width, height)));
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    if (fRecorder.get()) {
        return fRecorder.get();
    }
    return fPictureRecord.get();
}

SkPicture* SkPictureRecorder::endRecording() {
    SkPicture* picture = NULL;

    if (fRecord.get()) {
        picture = SkNEW_ARGS(SkPicture, (fCullWidth, fCullHeight,
                                         fRecord.detach(), fBBH.get()));
    }

    if (fPictureRecord.get()) {
        fPictureRecord->endRecording();
        const bool deepCopyOps = false;
        picture = SkNEW_ARGS(SkPicture, (fCullWidth, fCullHeight,
                                         *fPictureRecord.get(), deepCopyOps));
    }

    return picture;
}

void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (NULL == canvas) {
        return;
    }

    if (fRecord.get()) {
        SkRecordDraw(*fRecord, canvas, NULL/*bbh*/, NULL/*callback*/);
    }

    if (fPictureRecord.get()) {
        const bool deepCopyOps = true;
        SkPicture picture(fCullWidth, fCullHeight,
                          *fPictureRecord.get(), deepCopyOps);
        picture.playback(canvas);
    }
}
