#include "Test.h"

#include "SkDebugCanvas.h"
#include "SkRecord.h"
#include "SkRecordCulling.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkRecords.h"

static const int W = 1920, H = 1080;

DEF_TEST(RecordDraw_Culling, r) {
    // Record these 7 drawing commands verbatim.
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, W, H);

    recorder.pushCull(SkRect::MakeWH(100, 100));
        recorder.drawRect(SkRect::MakeWH(10, 10), SkPaint());
        recorder.drawRect(SkRect::MakeWH(30, 30), SkPaint());
        recorder.pushCull(SkRect::MakeWH(5, 5));
            recorder.drawRect(SkRect::MakeWH(1, 1), SkPaint());
        recorder.popCull();
    recorder.popCull();

    // Take a pass over to match up pushCulls and popCulls.
    SkRecordAnnotateCullingPairs(&record);

    // Rerecord into another SkRecord using full SkCanvas semantics,
    // tracking clips and allowing SkRecordDraw's quickReject() calls to work.
    SkRecord rerecord;
    SkRecorder rerecorder(SkRecorder::kReadWrite_Mode, &rerecord, W, H);
    // This clip intersects the outer cull, but allows us to quick reject the inner one.
    rerecorder.clipRect(SkRect::MakeLTRB(20, 20, 200, 200));

    SkRecordDraw(record, &rerecorder);

    // We'll keep the clipRect call from above, and the outer two drawRects, and the push/pop pair.
    // If culling weren't working, we'd see 8 commands recorded here.
    REPORTER_ASSERT(r, 5 == rerecord.count());
}

DEF_TEST(RecordDraw_Clipping, r) {
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, W, H);

    // 8 draw commands.
    // The inner clipRect makes the clip empty, so the inner drawRect does nothing.
    recorder.save();
        recorder.clipRect(SkRect::MakeLTRB(0, 0, 100, 100));
        recorder.drawRect(SkRect::MakeLTRB(20, 20, 40, 40), SkPaint());
        recorder.save();
            recorder.clipRect(SkRect::MakeLTRB(200, 200, 300, 300));
            recorder.drawRect(SkRect::MakeLTRB(220, 220, 240, 240), SkPaint());
        recorder.restore();
    recorder.restore();

    // Same deal as above: we need full SkCanvas semantics for clip skipping to work.
    SkRecord rerecord;
    SkRecorder rerecorder(SkRecorder::kReadWrite_Mode, &rerecord, W, H);
    SkRecordDraw(record, &rerecorder);

    // All commands except the drawRect will be preserved.
    REPORTER_ASSERT(r, 7 == rerecord.count());
}
