#ifndef DMUtil_DEFINED
#define DMUtil_DEFINED

#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "gm_expectations.h"

class SkBBHFactory;

// Small free functions used in more than one place in DM.

namespace DM {

// UnderJoin("a", "b") -> "a_b"
SkString UnderJoin(const char* a, const char* b);

// Draw gm to picture.  Passes recordFlags to SkPictureRecorder::beginRecording().
SkPicture* RecordPicture(skiagm::GM* gm,
                         uint32_t recordFlags = 0,
                         SkBBHFactory* factory = NULL);

// Prepare bitmap to have gm, bench or picture draw into it with this config.
// TODO(mtklein): make SkBenchmark::getSize()/GM::getISize() const.
void SetupBitmap(SkColorType, skiagm::GM* gm, SkBitmap* bitmap);
void SetupBitmap(SkColorType, SkBenchmark* bench, SkBitmap* bitmap);
void SetupBitmap(SkColorType, const SkPicture& picture, SkBitmap* bitmap);

// Draw picture to bitmap.
void DrawPicture(SkPicture* picture, SkBitmap* bitmap);

// Are these identical bitmaps?
bool BitmapsEqual(const SkBitmap& a, const SkBitmap& b);

}  // namespace DM

#endif  // DMUtil_DEFINED
