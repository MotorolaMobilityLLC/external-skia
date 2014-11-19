/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureRecorder_DEFINED
#define SkPictureRecorder_DEFINED

#include "SkBBHFactory.h"
#include "SkPicture.h"
#include "SkRefCnt.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
namespace android {
    class Picture;
};
#endif

class SkCanvas;
class SkPictureRecord;
class SkRecord;
class SkRecorder;

class SK_API SkPictureRecorder : SkNoncopyable {
public:
    SkPictureRecorder();
    ~SkPictureRecorder();

#ifdef SK_LEGACY_PICTURE_SIZE_API
    SkCanvas* beginRecording(int width, int height,
                             SkBBHFactory* bbhFactory = NULL,
                             uint32_t recordFlags = 0) {
        return this->beginRecording(SkIntToScalar(width), SkIntToScalar(height),
                                    bbhFactory, recordFlags);
    }
#endif

    enum RecordFlags {
        // This flag indicates that, if some BHH is being computed, saveLayer
        // information should also be extracted at the same time.
        kComputeSaveLayerInfo_RecordFlag = 0x01
    };

    /** Returns the canvas that records the drawing commands.
        @param bounds the cull rect used when recording this picture. Any drawing the falls outside
                      of this rect is undefined, and may be drawn or it may not.
        @param bbhFactory factory to create desired acceleration structure
        @param recordFlags optional flags that control recording.
        @return the canvas.
    */
    SkCanvas* beginRecording(const SkRect& bounds,
                             SkBBHFactory* bbhFactory = NULL,
                             uint32_t recordFlags = 0);

    SkCanvas* beginRecording(SkScalar width, SkScalar height,
                             SkBBHFactory* bbhFactory = NULL,
                             uint32_t recordFlags = 0) {
        return this->beginRecording(SkRect::MakeWH(width, height), bbhFactory, recordFlags);
    }

    /** Returns the recording canvas if one is active, or NULL if recording is
        not active. This does not alter the refcnt on the canvas (if present).
    */
    SkCanvas* getRecordingCanvas();

    /** Signal that the caller is done recording. This invalidates the canvas
        returned by beginRecording/getRecordingCanvas, and returns the
        created SkPicture. Note that the returned picture has its creation
        ref which the caller must take ownership of.
    */
    SkPicture* endRecording();

private:
    void reset();

    /** Replay the current (partially recorded) operation stream into
        canvas. This call doesn't close the current recording.
    */
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    friend class android::Picture;
#endif
    friend class SkPictureRecorderReplayTester; // for unit testing
    void partialReplay(SkCanvas* canvas) const;

    uint32_t                      fFlags;
    SkRect                        fCullRect;
    SkAutoTUnref<SkBBoxHierarchy> fBBH;
    SkAutoTUnref<SkRecorder>      fRecorder;
    SkAutoTDelete<SkRecord>       fRecord;

    typedef SkNoncopyable INHERITED;
};

#endif
