#include "SkPMFloat.h"
#include "Test.h"

DEF_TEST(SkPMFloat, r) {
    // Test SkPMColor <-> SkPMFloat
    SkPMColor c = SkPreMultiplyColor(0xFFCC9933);
    SkPMFloat pmf(c);
    REPORTER_ASSERT(r, SkScalarNearlyEqual(255.0f, pmf.a()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(204.0f, pmf.r()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(153.0f, pmf.g()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual( 51.0f, pmf.b()));
    REPORTER_ASSERT(r, c == pmf.get());

    // Test rounding.  (Don't bother testing .5... we don't care which way it goes.)
    pmf = SkPMFloat(254.6f, 204.3f, 153.1f, 50.8f);
    REPORTER_ASSERT(r, c == pmf.get());

    // Test clamping.
    SkPMFloat clamped(SkPMFloat(510.0f, 153.0f, 1.0f, -0.2f).clamped());
    REPORTER_ASSERT(r, SkScalarNearlyEqual(255.0f, clamped.a()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(153.0f, clamped.r()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(  1.0f, clamped.g()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(  0.0f, clamped.b()));

    // Test SkPMFloat <-> Sk4f conversion.
    Sk4f fs = clamped;
    SkPMFloat scaled = fs.multiply(0.25f);
    REPORTER_ASSERT(r, SkScalarNearlyEqual(63.75f, scaled.a()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(38.25f, scaled.r()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual( 0.25f, scaled.g()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual( 0.00f, scaled.b()));
}
