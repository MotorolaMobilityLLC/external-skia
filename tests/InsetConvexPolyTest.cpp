/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkInsetConvexPolygon.h"

static bool is_convex(const SkTDArray<SkPoint>& poly) {
    if (poly.count() < 3) {
        return false;
    }

    SkVector v0 = poly[0] - poly[poly.count() - 1];
    SkVector v1 = poly[1] - poly[poly.count() - 1];
    SkScalar winding = v0.cross(v1);

    for (int i = 0; i < poly.count()-1; ++i) {
        int j = i + 1;
        int k = (i + 2) % poly.count();

        SkVector v0 = poly[j] - poly[i];
        SkVector v1 = poly[k] - poly[i];
        SkScalar perpDot = v0.cross(v1);
        if (winding*perpDot < 0) {
            return false;
        }
    }

    return true;
}

DEF_TEST(InsetConvexPoly, reporter) {
    SkTDArray<SkPoint> rrectPoly;

    // round rect
    *rrectPoly.push() = SkPoint::Make(-100, 55);
    *rrectPoly.push() = SkPoint::Make(100, 55);
    *rrectPoly.push() = SkPoint::Make(100 + 2.5f, 50 + 4.330127f);
    *rrectPoly.push() = SkPoint::Make(100 + 3.535534f, 50 + 3.535534f);
    *rrectPoly.push() = SkPoint::Make(100 + 4.330127f, 50 + 2.5f);
    *rrectPoly.push() = SkPoint::Make(105, 50);
    *rrectPoly.push() = SkPoint::Make(105, -50);
    *rrectPoly.push() = SkPoint::Make(100 + 4.330127f, -50 - 2.5f);
    *rrectPoly.push() = SkPoint::Make(100 + 3.535534f, -50 - 3.535534f);
    *rrectPoly.push() = SkPoint::Make(100 + 2.5f, -50 - 4.330127f);
    *rrectPoly.push() = SkPoint::Make(100, -55);
    *rrectPoly.push() = SkPoint::Make(-100, -55);
    *rrectPoly.push() = SkPoint::Make(-100 - 2.5f, -50 - 4.330127f);
    *rrectPoly.push() = SkPoint::Make(-100 - 3.535534f, -50 - 3.535534f);
    *rrectPoly.push() = SkPoint::Make(-100 - 4.330127f, -50 - 2.5f);
    *rrectPoly.push() = SkPoint::Make(-105, -50);
    *rrectPoly.push() = SkPoint::Make(-105, 50);
    *rrectPoly.push() = SkPoint::Make(-100 - 4.330127f, 50 + 2.5f);
    *rrectPoly.push() = SkPoint::Make(-100 - 3.535534f, 50 + 3.535534f);
    *rrectPoly.push() = SkPoint::Make(-100 - 2.5f, 50 + 4.330127f);
    REPORTER_ASSERT(reporter, is_convex(rrectPoly));

    // inset a little
    SkTDArray<SkPoint> insetPoly;
    bool result = SkInsetConvexPolygon(&rrectPoly[0], rrectPoly.count(), 3, &insetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, is_convex(insetPoly));

    // inset to rect
    result = SkInsetConvexPolygon(&rrectPoly[0], rrectPoly.count(), 10, &insetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, is_convex(insetPoly));
    REPORTER_ASSERT(reporter, insetPoly.count() == 4);
    if (insetPoly.count() == 4) {
        REPORTER_ASSERT(reporter, insetPoly[0].equals(-95, 45));
        REPORTER_ASSERT(reporter, insetPoly[1].equals(95, 45));
        REPORTER_ASSERT(reporter, insetPoly[2].equals(95, -45));
        REPORTER_ASSERT(reporter, insetPoly[3].equals(-95, -45));
    }

    // just to full inset
    // for shadows having a flat poly here is fine
    // may want to revisit for strokes
    result = SkInsetConvexPolygon(&rrectPoly[0], rrectPoly.count(), 55, &insetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, is_convex(insetPoly));
    REPORTER_ASSERT(reporter, insetPoly.count() == 4);
    if (insetPoly.count() == 4) {
        REPORTER_ASSERT(reporter, insetPoly[0].equals(-50, 0));
        REPORTER_ASSERT(reporter, insetPoly[1].equals(50, 0));
        REPORTER_ASSERT(reporter, insetPoly[2].equals(50, 0));
        REPORTER_ASSERT(reporter, insetPoly[3].equals(-50, 0));
    }

    // past full inset
    result = SkInsetConvexPolygon(&rrectPoly[0], rrectPoly.count(), 75, &insetPoly);
    REPORTER_ASSERT(reporter, !result);

    // troublesome case
    SkTDArray<SkPoint> clippedRRectPoly;
    *clippedRRectPoly.push() = SkPoint::Make(335.928101f, 428.219055f);
    *clippedRRectPoly.push() = SkPoint::Make(330.414459f, 423.034912f);
    *clippedRRectPoly.push() = SkPoint::Make(325.749084f, 417.395508f);
    *clippedRRectPoly.push() = SkPoint::Make(321.931946f, 411.300842f);
    *clippedRRectPoly.push() = SkPoint::Make(318.963074f, 404.750977f);
    *clippedRRectPoly.push() = SkPoint::Make(316.842468f, 397.745850f);
    *clippedRRectPoly.push() = SkPoint::Make(315.570068f, 390.285522f);
    *clippedRRectPoly.push() = SkPoint::Make(315.145966f, 382.369965f);
    *clippedRRectPoly.push() = SkPoint::Make(315.570068f, 374.454346f);
    *clippedRRectPoly.push() = SkPoint::Make(316.842468f, 366.994019f);
    *clippedRRectPoly.push() = SkPoint::Make(318.963074f, 359.988892f);
    *clippedRRectPoly.push() = SkPoint::Make(321.931946f, 353.439056f);
    *clippedRRectPoly.push() = SkPoint::Make(325.749084f, 347.344421f);
    *clippedRRectPoly.push() = SkPoint::Make(330.414459f, 341.705017f);
    *clippedRRectPoly.push() = SkPoint::Make(335.928101f, 336.520813f);
    *clippedRRectPoly.push() = SkPoint::Make(342.289948f, 331.791901f);
    *clippedRRectPoly.push() = SkPoint::Make(377.312134f, 331.791901f);
    *clippedRRectPoly.push() = SkPoint::Make(381.195313f, 332.532593f);
    *clippedRRectPoly.push() = SkPoint::Make(384.464935f, 334.754700f);
    *clippedRRectPoly.push() = SkPoint::Make(386.687042f, 338.024292f);
    *clippedRRectPoly.push() = SkPoint::Make(387.427765f, 341.907532f);
    *clippedRRectPoly.push() = SkPoint::Make(387.427765f, 422.832367f);
    *clippedRRectPoly.push() = SkPoint::Make(386.687042f, 426.715576f);
    *clippedRRectPoly.push() = SkPoint::Make(384.464935f, 429.985168f);
    *clippedRRectPoly.push() = SkPoint::Make(381.195313f, 432.207275f);
    *clippedRRectPoly.push() = SkPoint::Make(377.312134f, 432.947998f);
    *clippedRRectPoly.push() = SkPoint::Make(342.289948f, 432.947998f);
    REPORTER_ASSERT(reporter, is_convex(clippedRRectPoly));

    result = SkInsetConvexPolygon(&clippedRRectPoly[0], clippedRRectPoly.count(), 32.3699417f,
                                  &insetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, is_convex(insetPoly));
}
