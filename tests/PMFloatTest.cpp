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

    // Test rounding.
    pmf = SkPMFloat(254.5f, 203.5f, 153.1f, 50.8f);
    REPORTER_ASSERT(r, c == pmf.get());

    // Test clamping.
    SkPMFloat clamped(SkPMFloat(510.0f, 153.0f, 1.0f, -0.2f).clamped());
    REPORTER_ASSERT(r, SkScalarNearlyEqual(255.0f, clamped.a()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(153.0f, clamped.r()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(  1.0f, clamped.g()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(  0.0f, clamped.b()));

    // Test SkPMFloat <-> Sk4f conversion.
    Sk4f fs = clamped;
#if defined(SK_CPU_ARM64) && defined(SK_RELEASE)   // skia:3570
    SkPMFloat scaled;
    scaled = fs * Sk4f(0.25f);
#else
    SkPMFloat scaled = fs * Sk4f(0.25f);
#endif
    REPORTER_ASSERT(r, SkScalarNearlyEqual(63.75f, scaled.a()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(38.25f, scaled.r()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual( 0.25f, scaled.g()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual( 0.00f, scaled.b()));

    // Test 4-at-a-time conversions.
    SkPMColor colors[4] = { 0xFF000000, 0xFFFF0000, 0xFF00FF00, 0xFF0000FF };
    SkPMFloat floats[4];
    SkPMFloat::From4PMColors(floats, colors);

    SkPMColor back[4];
    SkPMFloat::To4PMColors(back, floats);
    for (int i = 0; i < 4; i++) {
        REPORTER_ASSERT(r, back[i] == colors[i]);
    }

    SkPMFloat::ClampTo4PMColors(back, floats);
    for (int i = 0; i < 4; i++) {
        REPORTER_ASSERT(r, back[i] == colors[i]);
    }
}
