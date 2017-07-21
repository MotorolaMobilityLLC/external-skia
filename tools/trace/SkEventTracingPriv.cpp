/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkEventTracingPriv.h"

#include "SkATrace.h"
#include "SkChromeTracingTracer.h"
#include "SkCommandLineFlags.h"
#include "SkDebugfTracer.h"
#include "SkEventTracer.h"
#include "SkTraceEventCommon.h"

DEFINE_string(trace, "",
              "Log trace events in one of several modes:\n"
              "  debugf     : Show events using SkDebugf\n"
              "  atrace     : Send events to Android ATrace\n"
              "  <filename> : Any other string is interpreted as a filename. Writes\n"
              "               trace events to specified file as JSON, for viewing\n"
              "               with chrome://tracing");

void initializeEventTracingForTools(int32_t* threadsFlag) {
    if (FLAGS_trace.isEmpty()) {
        return;
    }

    const char* traceFlag = FLAGS_trace[0];
    SkEventTracer* eventTracer = nullptr;
    if (0 == strcmp(traceFlag, "atrace")) {
        eventTracer = new SkATrace();
    } else if (0 == strcmp(traceFlag, "debugf")) {
        eventTracer = new SkDebugfTracer();
    } else {
        if (threadsFlag && *threadsFlag != 0) {
            SkDebugf("JSON tracing is not yet thread-safe, disabling threading.\n");
            *threadsFlag = 0;
        }
        eventTracer = new SkChromeTracingTracer(traceFlag);
    }

    SkAssertResult(SkEventTracer::SetInstance(eventTracer));
}

uint8_t* SkEventTracingCategories::getCategoryGroupEnabled(const char* name) {
    static_assert(0 == offsetof(CategoryState, fEnabled), "CategoryState");

    // We ignore the "disabled-by-default-" prefix in our internal tools (though we could honor it)
    if (SkStrStartsWith(name, TRACE_DISABLED_BY_DEFAULT_PREFIX)) {
        name += strlen(TRACE_DISABLED_BY_DEFAULT_PREFIX);
    }

    // Chrome's implementation of this API does a two-phase lookup (once without a lock, then again
    // with a lock. But the tracing macros avoid calling these functions more than once per site,
    // so just do something simple (and easier to reason about):
    SkAutoMutexAcquire lock(&fMutex);
    for (int i = 0; i < fNumCategories; ++i) {
        if (0 == strcmp(name, fCategories[i].fName)) {
            return reinterpret_cast<uint8_t*>(&fCategories[i]);
        }
    }

    if (fNumCategories >= kMaxCategories) {
        SkDEBUGFAIL("Exhausted event tracing categories. Increase kMaxCategories.");
        return reinterpret_cast<uint8_t*>(&fCategories[0]);
    }

    fCategories[fNumCategories].fEnabled =
            SkEventTracer::kEnabledForRecording_CategoryGroupEnabledFlags;
    fCategories[fNumCategories].fName = name;
    return reinterpret_cast<uint8_t*>(&fCategories[fNumCategories++]);
}

const char* SkEventTracingCategories::getCategoryGroupName(const uint8_t* categoryEnabledFlag) {
    if (categoryEnabledFlag) {
        return reinterpret_cast<const CategoryState*>(categoryEnabledFlag)->fName;
    }
    return nullptr;
}
