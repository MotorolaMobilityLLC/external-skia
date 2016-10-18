# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Common gypi for pathops unit tests.
{
  'include_dirs': [
    '../include/private',
    '../src/core',
    '../src/effects',
    '../src/lazy',
    '../src/pathops',
    '../src/pipe/utils',
    '../src/utils',
  ],
  'dependencies': [
    'flags.gyp:flags',
    'skia_lib.gyp:skia_lib',
    'tools.gyp:resources',
  ],
  'sources': [
    '../tests/Test.cpp',
    '../tests/Test.h',

    '../tests/PathOpsAngleTest.cpp',
    '../tests/PathOpsBoundsTest.cpp',
    '../tests/PathOpsBuilderConicTest.cpp',
    '../tests/PathOpsBuilderTest.cpp',
    '../tests/PathOpsBuildUseTest.cpp',
    '../tests/PathOpsChalkboardTest.cpp',
    '../tests/PathOpsConicIntersectionTest.cpp',
    '../tests/PathOpsConicLineIntersectionTest.cpp',
    '../tests/PathOpsConicQuadIntersectionTest.cpp',
    '../tests/PathOpsCubicConicIntersectionTest.cpp',
    '../tests/PathOpsCubicIntersectionTest.cpp',
    '../tests/PathOpsCubicIntersectionTestData.cpp',
    '../tests/PathOpsCubicLineIntersectionTest.cpp',
    '../tests/PathOpsCubicQuadIntersectionTest.cpp',
    '../tests/PathOpsCubicReduceOrderTest.cpp',
    '../tests/PathOpsDCubicTest.cpp',
    '../tests/PathOpsDLineTest.cpp',
    '../tests/PathOpsDPointTest.cpp',
    '../tests/PathOpsDRectTest.cpp',
    '../tests/PathOpsDVectorTest.cpp',
    '../tests/PathOpsExtendedTest.cpp',
    '../tests/PathOpsFuzz763Test.cpp',
    '../tests/PathOpsInverseTest.cpp',
    '../tests/PathOpsIssue3651.cpp',
    '../tests/PathOpsLineIntersectionTest.cpp',
    '../tests/PathOpsLineParametetersTest.cpp',
    '../tests/PathOpsOpCircleThreadedTest.cpp',
    '../tests/PathOpsOpCubicThreadedTest.cpp',
    '../tests/PathOpsOpRectThreadedTest.cpp',
    '../tests/PathOpsOpTest.cpp',
    '../tests/PathOpsQuadIntersectionTest.cpp',
    '../tests/PathOpsQuadIntersectionTestData.cpp',
    '../tests/PathOpsQuadLineIntersectionTest.cpp',
    '../tests/PathOpsQuadLineIntersectionThreadedTest.cpp',
    '../tests/PathOpsQuadReduceOrderTest.cpp',
    '../tests/PathOpsSimplifyDegenerateThreadedTest.cpp',
    '../tests/PathOpsSimplifyFailTest.cpp',
    '../tests/PathOpsSimplifyQuadralateralsThreadedTest.cpp',
    '../tests/PathOpsSimplifyQuadThreadedTest.cpp',
    '../tests/PathOpsSimplifyRectThreadedTest.cpp',
    '../tests/PathOpsSimplifyTest.cpp',
    '../tests/PathOpsSimplifyTrianglesThreadedTest.cpp',
    '../tests/PathOpsSkpTest.cpp',
    '../tests/PathOpsTestCommon.cpp',
    '../tests/PathOpsThreadedCommon.cpp',
    '../tests/PathOpsThreeWayTest.cpp',
    '../tests/PathOpsTigerTest.cpp',
    '../tests/PathOpsTightBoundsTest.cpp',
    '../tests/PathOpsTypesTest.cpp',
    '../tests/SubsetPath.cpp',

    '../tests/PathOpsCubicIntersectionTestData.h',
    '../tests/PathOpsExtendedTest.h',
    '../tests/PathOpsQuadIntersectionTestData.h',
    '../tests/PathOpsTestCommon.h',
    '../tests/PathOpsThreadedCommon.h',
    '../tests/PathOpsTSectDebug.h',
    '../tests/SubsetPath.h',
  ],
}
