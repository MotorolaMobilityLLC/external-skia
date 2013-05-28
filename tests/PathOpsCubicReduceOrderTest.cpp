/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsCubicIntersectionTestData.h"
#include "PathOpsQuadIntersectionTestData.h"
#include "SkIntersections.h"
#include "SkPathOpsRect.h"
#include "SkReduceOrder.h"
#include "Test.h"

static bool controls_inside(const SkDCubic& cubic) {
    return between(cubic[0].fX, cubic[1].fX, cubic[3].fX)
            && between(cubic[0].fX, cubic[2].fX, cubic[3].fX)
            && between(cubic[0].fY, cubic[1].fY, cubic[3].fY)
            && between(cubic[0].fY, cubic[2].fY, cubic[3].fY);
}

static bool tiny(const SkDCubic& cubic) {
    int index, minX, maxX, minY, maxY;
    minX = maxX = minY = maxY = 0;
    for (index = 1; index < 4; ++index) {
        if (cubic[minX].fX > cubic[index].fX) {
            minX = index;
        }
        if (cubic[minY].fY > cubic[index].fY) {
            minY = index;
        }
        if (cubic[maxX].fX < cubic[index].fX) {
            maxX = index;
        }
        if (cubic[maxY].fY < cubic[index].fY) {
            maxY = index;
        }
    }
    return     approximately_equal(cubic[maxX].fX, cubic[minX].fX)
            && approximately_equal(cubic[maxY].fY, cubic[minY].fY);
}

static void find_tight_bounds(const SkDCubic& cubic, SkDRect& bounds) {
    SkDCubicPair cubicPair = cubic.chopAt(0.5);
    if (!tiny(cubicPair.first()) && !controls_inside(cubicPair.first())) {
        find_tight_bounds(cubicPair.first(), bounds);
    } else {
        bounds.add(cubicPair.first()[0]);
        bounds.add(cubicPair.first()[3]);
    }
    if (!tiny(cubicPair.second()) && !controls_inside(cubicPair.second())) {
        find_tight_bounds(cubicPair.second(), bounds);
    } else {
        bounds.add(cubicPair.second()[0]);
        bounds.add(cubicPair.second()[3]);
    }
}

static void PathOpsReduceOrderCubicTest(skiatest::Reporter* reporter) {
    size_t index;
    SkReduceOrder reducer;
    int order;
    enum {
        RunAll,
        RunPointDegenerates,
        RunNotPointDegenerates,
        RunLines,
        RunNotLines,
        RunModEpsilonLines,
        RunLessEpsilonLines,
        RunNegEpsilonLines,
        RunQuadraticLines,
        RunQuadraticPoints,
        RunQuadraticModLines,
        RunComputedLines,
        RunNone
    } run = RunAll;
    int firstTestIndex = 0;
#if 0
    run = RunComputedLines;
    firstTestIndex = 18;
#endif
    int firstPointDegeneratesTest = run == RunAll ? 0 : run == RunPointDegenerates
            ? firstTestIndex : SK_MaxS32;
    int firstNotPointDegeneratesTest = run == RunAll ? 0 : run == RunNotPointDegenerates
            ? firstTestIndex : SK_MaxS32;
    int firstLinesTest = run == RunAll ? 0 : run == RunLines ? firstTestIndex : SK_MaxS32;
    int firstNotLinesTest = run == RunAll ? 0 : run == RunNotLines ? firstTestIndex : SK_MaxS32;
    int firstModEpsilonTest = run == RunAll ? 0 : run == RunModEpsilonLines
            ? firstTestIndex : SK_MaxS32;
    int firstLessEpsilonTest = run == RunAll ? 0 : run == RunLessEpsilonLines
            ? firstTestIndex : SK_MaxS32;
    int firstNegEpsilonTest = run == RunAll ? 0 : run == RunNegEpsilonLines
            ? firstTestIndex : SK_MaxS32;
    int firstQuadraticPointTest = run == RunAll ? 0 : run == RunQuadraticPoints
            ? firstTestIndex : SK_MaxS32;
    int firstQuadraticLineTest = run == RunAll ? 0 : run == RunQuadraticLines
            ? firstTestIndex : SK_MaxS32;
    int firstQuadraticModLineTest = run == RunAll ? 0 : run == RunQuadraticModLines
            ? firstTestIndex : SK_MaxS32;
    int firstComputedLinesTest = run == RunAll ? 0 : run == RunComputedLines
            ? firstTestIndex : SK_MaxS32;

    for (index = firstPointDegeneratesTest; index < pointDegenerates_count; ++index) {
        const SkDCubic& cubic = pointDegenerates[index];
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics, SkReduceOrder::kFill_Style);
        if (order != 1) {
            SkDebugf("[%d] pointDegenerates order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
        }
    }
    for (index = firstNotPointDegeneratesTest; index < notPointDegenerates_count; ++index) {
        const SkDCubic& cubic = notPointDegenerates[index];
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics, SkReduceOrder::kFill_Style);
        if (order == 1) {
            SkDebugf("[%d] notPointDegenerates order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
        }
    }
    for (index = firstLinesTest; index < lines_count; ++index) {
        const SkDCubic& cubic = lines[index];
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics, SkReduceOrder::kFill_Style);
        if (order != 2) {
            SkDebugf("[%d] lines order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
        }
    }
    for (index = firstNotLinesTest; index < notLines_count; ++index) {
        const SkDCubic& cubic = notLines[index];
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics, SkReduceOrder::kFill_Style);
        if (order == 2) {
            SkDebugf("[%d] notLines order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
       }
    }
    for (index = firstModEpsilonTest; index < modEpsilonLines_count; ++index) {
        const SkDCubic& cubic = modEpsilonLines[index];
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics, SkReduceOrder::kFill_Style);
        if (order == 2) {
            SkDebugf("[%d] line mod by epsilon order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
        }
    }
    for (index = firstLessEpsilonTest; index < lessEpsilonLines_count; ++index) {
        const SkDCubic& cubic = lessEpsilonLines[index];
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics, SkReduceOrder::kFill_Style);
        if (order != 2) {
            SkDebugf("[%d] line less by epsilon/2 order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
        }
    }
    for (index = firstNegEpsilonTest; index < negEpsilonLines_count; ++index) {
        const SkDCubic& cubic = negEpsilonLines[index];
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics, SkReduceOrder::kFill_Style);
        if (order != 2) {
            SkDebugf("[%d] line neg by epsilon/2 order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
        }
    }
    for (index = firstQuadraticPointTest; index < quadraticPoints_count; ++index) {
        const SkDQuad& quad = quadraticPoints[index];
        SkDCubic cubic = quad.toCubic();
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics, SkReduceOrder::kFill_Style);
        if (order != 1) {
            SkDebugf("[%d] point quad order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
        }
    }
    for (index = firstQuadraticLineTest; index < quadraticLines_count; ++index) {
        const SkDQuad& quad = quadraticLines[index];
        SkDCubic cubic = quad.toCubic();
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics, SkReduceOrder::kFill_Style);
        if (order != 2) {
            SkDebugf("[%d] line quad order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
        }
    }
    for (index = firstQuadraticModLineTest; index < quadraticModEpsilonLines_count; ++index) {
        const SkDQuad& quad = quadraticModEpsilonLines[index];
        SkDCubic cubic = quad.toCubic();
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics, SkReduceOrder::kFill_Style);
        if (order != 3) {
            SkDebugf("[%d] line mod quad order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
        }
    }

    // test if computed line end points are valid
    for (index = firstComputedLinesTest; index < lines_count; ++index) {
        const SkDCubic& cubic = lines[index];
        bool controlsInside = controls_inside(cubic);
        order = reducer.reduce(cubic, SkReduceOrder::kAllow_Quadratics,
                SkReduceOrder::kStroke_Style);
        if (order == 2 && reducer.fLine[0] == reducer.fLine[1]) {
            SkDebugf("[%d] line computed ends match order=%d\n", static_cast<int>(index), order);
            REPORTER_ASSERT(reporter, 0);
        }
        if (controlsInside) {
            if (       (reducer.fLine[0].fX != cubic[0].fX && reducer.fLine[0].fX != cubic[3].fX)
                    || (reducer.fLine[0].fY != cubic[0].fY && reducer.fLine[0].fY != cubic[3].fY)
                    || (reducer.fLine[1].fX != cubic[0].fX && reducer.fLine[1].fX != cubic[3].fX)
                    || (reducer.fLine[1].fY != cubic[0].fY && reducer.fLine[1].fY != cubic[3].fY)) {
                SkDebugf("[%d] line computed ends order=%d\n", static_cast<int>(index), order);
                REPORTER_ASSERT(reporter, 0);
            }
        } else {
            // binary search for extrema, compare against actual results
                // while a control point is outside of bounding box formed by end points, split
            SkDRect bounds = {DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX};
            find_tight_bounds(cubic, bounds);
            if (      (!AlmostEqualUlps(reducer.fLine[0].fX, bounds.fLeft)
                    && !AlmostEqualUlps(reducer.fLine[0].fX, bounds.fRight))
                   || (!AlmostEqualUlps(reducer.fLine[0].fY, bounds.fTop)
                    && !AlmostEqualUlps(reducer.fLine[0].fY, bounds.fBottom))
                   || (!AlmostEqualUlps(reducer.fLine[1].fX, bounds.fLeft)
                    && !AlmostEqualUlps(reducer.fLine[1].fX, bounds.fRight))
                   || (!AlmostEqualUlps(reducer.fLine[1].fY, bounds.fTop)
                    && !AlmostEqualUlps(reducer.fLine[1].fY, bounds.fBottom))) {
                SkDebugf("[%d] line computed tight bounds order=%d\n", static_cast<int>(index), order);
                REPORTER_ASSERT(reporter, 0);
            }
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsReduceOrderCubicTest)
