/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_SUPPORT_GPU

#include "GrCaps.h"
#include "GrContext.h"
#include "GrGpu.h"

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GrDrawTargetPrint, reporter, ctxInfo) {
    // This used to assert.
    SkString result = ctxInfo.fGrContext->caps()->dump();
    SkASSERT(!result.isEmpty());
    SkString shaderResult = ctxInfo.fGrContext->caps()->shaderCaps()->dump();
    SkASSERT(!shaderResult.isEmpty());
}

#endif
