/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkTypes.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkMiniRecorder.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecordDraw.h"
#include "src/core/SkRecordOpts.h"
#include "src/core/SkRecordedDrawable.h"
#include "src/core/SkRecorder.h"

SkPictureRecorder::SkPictureRecorder() {
    fActivelyRecording = false;
    fMiniRecorder.reset(new SkMiniRecorder);
    fRecorder.reset(new SkRecorder(nullptr, SkRect::MakeEmpty(), fMiniRecorder.get()));
}

SkPictureRecorder::~SkPictureRecorder() {}

SkCanvas* SkPictureRecorder::beginRecording(const SkRect& userCullRect,
                                            sk_sp<SkBBoxHierarchy> bbh,
                                            uint32_t recordFlags /* = 0 */) {
    const SkRect cullRect = userCullRect.isEmpty() ? SkRect::MakeEmpty() : userCullRect;

    fCullRect = cullRect;
    fFlags = recordFlags;
    fBBH = std::move(bbh);

    if (!fRecord) {
        fRecord.reset(new SkRecord);
    }
    SkRecorder::DrawPictureMode dpm = (recordFlags & kPlaybackDrawPicture_RecordFlag)
        ? SkRecorder::Playback_DrawPictureMode
        : SkRecorder::Record_DrawPictureMode;
    fRecorder->reset(fRecord.get(), cullRect, dpm, fMiniRecorder.get());
    fActivelyRecording = true;
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::beginRecording(const SkRect& bounds,
                                            SkBBHFactory* factory,
                                            uint32_t flags) {
    return this->beginRecording(bounds, factory ? (*factory)() : nullptr, flags);
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    return fActivelyRecording ? fRecorder.get() : nullptr;
}

sk_sp<SkPicture> SkPictureRecorder::finishRecordingAsPicture(uint32_t finishFlags) {
    fActivelyRecording = false;
    fRecorder->restoreToCount(1);  // If we were missing any restores, add them now.

    if (fRecord->count() == 0) {
        auto pic = fMiniRecorder->detachAsPicture(fBBH ? nullptr : &fCullRect);
        if (auto bbh = (SkBBoxHierarchy_Base*)fBBH.get()) {
            SkRect bounds = pic->cullRect();  // actually the computed bounds, not fCullRect.
            bbh->insert(&bounds, 1);
        }
        fBBH.reset(nullptr);
        return pic;
    }

    // TODO: delay as much of this work until just before first playback?
    SkRecordOptimize(fRecord.get());

    SkDrawableList* drawableList = fRecorder->getDrawableList();
    std::unique_ptr<SkBigPicture::SnapshotArray> pictList{
        drawableList ? drawableList->newDrawableSnapshot() : nullptr
    };

    if (fBBH.get()) {
        SkAutoTMalloc<SkRect> bounds(fRecord->count());
        SkRecordFillBounds(fCullRect, *fRecord, bounds);

        auto bbh = static_cast<SkBBoxHierarchy_Base*>(fBBH.get());
        bbh->insert(bounds, fRecord->count());

        // Now that we've calculated content bounds, we can update fCullRect, often trimming it.
        SkRect bbhBound = SkRect::MakeEmpty();
        for (int i = 0; i < fRecord->count(); i++) {
            bbhBound.join(bounds[i]);
        }
        SkASSERT((bbhBound.isEmpty() || fCullRect.contains(bbhBound))
              || (bbhBound.isEmpty() && fCullRect.isEmpty()));
        fCullRect = bbhBound;
    }

    size_t subPictureBytes = fRecorder->approxBytesUsedBySubPictures();
    for (int i = 0; pictList && i < pictList->count(); i++) {
        subPictureBytes += pictList->begin()[i]->approximateBytesUsed();
    }
    return sk_make_sp<SkBigPicture>(fCullRect,
                                    std::move(fRecord),
                                    std::move(pictList),
                                    std::move(fBBH),
                                    subPictureBytes);
}

sk_sp<SkPicture> SkPictureRecorder::finishRecordingAsPictureWithCull(const SkRect& cullRect,
                                                                     uint32_t finishFlags) {
    fCullRect = cullRect;
    return this->finishRecordingAsPicture(finishFlags);
}


void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (nullptr == canvas) {
        return;
    }

    int drawableCount = 0;
    SkDrawable* const* drawables = nullptr;
    SkDrawableList* drawableList = fRecorder->getDrawableList();
    if (drawableList) {
        drawableCount = drawableList->count();
        drawables = drawableList->begin();
    }
    SkRecordDraw(*fRecord, canvas, nullptr, drawables, drawableCount, nullptr/*bbh*/, nullptr/*callback*/);
}

sk_sp<SkDrawable> SkPictureRecorder::finishRecordingAsDrawable(uint32_t finishFlags) {
    fActivelyRecording = false;
    fRecorder->flushMiniRecorder();
    fRecorder->restoreToCount(1);  // If we were missing any restores, add them now.

    SkRecordOptimize(fRecord.get());

    if (fBBH.get()) {
        SkAutoTMalloc<SkRect> bounds(fRecord->count());
        SkRecordFillBounds(fCullRect, *fRecord, bounds);
        static_cast<SkBBoxHierarchy_Base*>(fBBH.get())->insert(bounds, fRecord->count());
    }

    sk_sp<SkDrawable> drawable =
         sk_make_sp<SkRecordedDrawable>(std::move(fRecord), std::move(fBBH),
                                        fRecorder->detachDrawableList(), fCullRect);

    return drawable;
}
