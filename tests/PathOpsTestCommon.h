/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PathOpsTestCommon_DEFINED
#define PathOpsTestCommon_DEFINED

#include "SkPathOpsQuad.h"
#include "SkTDArray.h"

void CubicToQuads(const SkDCubic& cubic, double precision, SkTDArray<SkDQuad>& quads);

#endif
