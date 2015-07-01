/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMiniRecorder_DEFINED
#define SkMiniRecorder_DEFINED

#include "SkRecords.h"
#include "SkScalar.h"
#include "SkTypes.h"
class SkCanvas;

// Records small pictures, but only a limited subset of the canvas API, and may fail.
class SkMiniRecorder : SkNoncopyable {
public:
    SkMiniRecorder();
    ~SkMiniRecorder();

    // Try to record an op.  Returns false on failure.
    bool drawBitmapRectToRect(const SkBitmap&, const SkRect* src, const SkRect& dst,
                              const SkPaint*, SkCanvas::DrawBitmapRectFlags);
    bool drawPath(const SkPath&, const SkPaint&);
    bool drawRect(const SkRect&, const SkPaint&);
    bool drawTextBlob(const SkTextBlob*, SkScalar x, SkScalar y, const SkPaint&);

    // Detach anything we've recorded as a picture, resetting this SkMiniRecorder.
    SkPicture* detachAsPicture(const SkRect& cull);

    // Flush anything we've recorded to the canvas, resetting this SkMiniRecorder.
    // This is logically the same as but rather more efficient than:
    //    SkAutoTUnref<SkPicture> pic(this->detachAsPicture(SkRect::MakeEmpty()));
    //    pic->playback(canvas);
    void flushAndReset(SkCanvas*);

private:
    enum class State {
        kEmpty,
        kDrawBitmapRectToRectFixedSize,
        kDrawPath,
        kDrawRect,
        kDrawTextBlob,
    };

    State fState;

    template <size_t A, size_t B>
    struct Max { static const size_t val = A > B ? A : B; };

    static const size_t kInlineStorage =
        Max<sizeof(SkRecords::DrawBitmapRectToRectFixedSize),
        Max<sizeof(SkRecords::DrawPath),
        Max<sizeof(SkRecords::DrawRect),
            sizeof(SkRecords::DrawTextBlob)>::val>::val>::val;
    SkAlignedSStorage<kInlineStorage> fBuffer;
};

#endif//SkMiniRecorder_DEFINED
