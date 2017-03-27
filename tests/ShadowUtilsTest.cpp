/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPath.h"
#include "SkShadowTessellator.h"
#include "SkShadowUtils.h"
#include "SkVertices.h"
#include "Test.h"

void tessellate_shadow(skiatest::Reporter* reporter, const SkPath& path, const SkMatrix& ctm,
                       bool expectSuccess) {
    static constexpr SkScalar kAmbientAlpha = 0.25f;
    static constexpr SkScalar kSpotAlpha = 0.25f;

    auto heightFunc = [] (SkScalar, SkScalar) { return 4; };
    
    auto verts = SkShadowTessellator::MakeAmbient(path, ctm, heightFunc, kAmbientAlpha, true);
    if (expectSuccess != SkToBool(verts)) {
        ERRORF(reporter, "Expected shadow tessellation to %s but it did not.",
               expectSuccess ? "succeed" : "fail");
    }
    verts = SkShadowTessellator::MakeAmbient(path, ctm, heightFunc, kAmbientAlpha, false);
    if (expectSuccess != SkToBool(verts)) {
        ERRORF(reporter, "Expected shadow tessellation to %s but it did not.",
               expectSuccess ? "succeed" : "fail");
    }
    verts = SkShadowTessellator::MakeSpot(path, ctm, heightFunc, {0, 0, 128}, 128.f,
                                          kSpotAlpha, false);
    if (expectSuccess != SkToBool(verts)) {
        ERRORF(reporter, "Expected shadow tessellation to %s but it did not.",
               expectSuccess ? "succeed" : "fail");
    }
    verts = SkShadowTessellator::MakeSpot(path, ctm, heightFunc, {0, 0, 128}, 128.f,
                                          kSpotAlpha, false);
    if (expectSuccess != SkToBool(verts)) {
        ERRORF(reporter, "Expected shadow tessellation to %s but it did not.",
               expectSuccess ? "succeed" : "fail");
    }
}

DEF_TEST(ShadowUtils, reporter) {
    SkCanvas canvas(100, 100);
    // Currently SkShadowUtils doesn't really support cubics when compiled without SK_SUPPORT_GPU.
    // However, this should now not crash.
    SkPath path;
    path.cubicTo(100, 50, 20, 100, 0, 0);
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), (bool)SK_SUPPORT_GPU);

    // This line segment has no area and no shadow.
    path.reset();
    path.lineTo(10.f, 10.f);
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), false);

    // A series of colinear line segments
    path.reset();
    for (int i = 0; i < 10; ++i) {
        path.lineTo((SkScalar)i, (SkScalar)i);
    }
    tessellate_shadow(reporter, path, canvas.getTotalMatrix(), false);
}
