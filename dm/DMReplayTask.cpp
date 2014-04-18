#include "DMReplayTask.h"
#include "DMWriteTask.h"
#include "DMUtil.h"

#include "SkBBHFactory.h"
#include "SkCommandLineFlags.h"
#include "SkPicture.h"

DEFINE_bool(replay, true, "If true, run picture replay tests.");
DEFINE_bool(rtree,  true, "If true, run picture replay tests with an rtree.");

namespace DM {

ReplayTask::ReplayTask(const Task& parent,
                       skiagm::GM* gm,
                       SkBitmap reference,
                       bool useRTree)
    : CpuTask(parent)
    , fName(UnderJoin(parent.name().c_str(), useRTree ? "rtree" : "replay"))
    , fGM(gm)
    , fReference(reference)
    , fUseRTree(useRTree)
    {}

void ReplayTask::draw() {
    SkAutoTDelete<SkBBHFactory> factory;
    if (fUseRTree) {
        factory.reset(SkNEW(SkRTreeFactory));
    }
    SkAutoTUnref<SkPicture> recorded(RecordPicture(fGM.get(), 0, factory.get()));

    SkBitmap bitmap;
    SetupBitmap(fReference.colorType(), fGM.get(), &bitmap);
    DrawPicture(recorded, &bitmap);
    if (!BitmapsEqual(bitmap, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
    }
}

bool ReplayTask::shouldSkip() const {
    if (fGM->getFlags() & skiagm::GM::kSkipPicture_Flag) {
        return true;
    }

    if (FLAGS_rtree && fUseRTree) {
        return (fGM->getFlags() & skiagm::GM::kSkipTiled_Flag) != 0;
    }
    if (FLAGS_replay && !fUseRTree) {
        return false;
    }
    return true;
}

}  // namespace DM
