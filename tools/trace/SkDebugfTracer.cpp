/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebugfTracer.h"
#include "SkTraceEvent.h"

SkEventTracer::Handle SkDebugfTracer::addTraceEvent(char phase,
                                                    const uint8_t* categoryEnabledFlag,
                                                    const char* name,
                                                    uint64_t id,
                                                    int numArgs,
                                                    const char** argNames,
                                                    const uint8_t* argTypes,
                                                    const uint64_t* argValues,
                                                    uint8_t flags) {
    SkString args;
    for (int i = 0; i < numArgs; ++i) {
        if (i > 0) {
            args.append(", ");
        } else {
            args.append(" ");
        }
        switch (argTypes[i]) {
            case TRACE_VALUE_TYPE_BOOL:
                args.appendf("%s=%s",
                             argNames[i],
                             (*reinterpret_cast<bool*>(argValues[i]) ? "true" : "false"));
                break;
            case TRACE_VALUE_TYPE_UINT:
                args.appendf("%s=%u", argNames[i], static_cast<uint32_t>(argValues[i]));
                break;
            case TRACE_VALUE_TYPE_INT:
                args.appendf("%s=%d", argNames[i], static_cast<int32_t>(argValues[i]));
                break;
            case TRACE_VALUE_TYPE_DOUBLE:
                args.appendf("%s=%g", argNames[i], *SkTCast<const double*>(&argValues[i]));
                break;
            case TRACE_VALUE_TYPE_POINTER:
                args.appendf("%s=0x%p", argNames[i], reinterpret_cast<void*>(argValues[i]));
                break;
            case TRACE_VALUE_TYPE_STRING:
            case TRACE_VALUE_TYPE_COPY_STRING: {
                static constexpr size_t kMaxLen = 20;
                const char* str = reinterpret_cast<const char*>(argValues[i]);
                SkString string(str);
                size_t truncAt = string.size();
                size_t newLineAt = SkStrFind(string.c_str(), "\n");
                if (newLineAt > 0) {
                    truncAt = newLineAt;
                }
                truncAt = SkTMin(truncAt, kMaxLen);
                if (truncAt < string.size()) {
                    string.resize(truncAt);
                    string.append("...");
                }
                args.appendf("%s=\"%s\"", argNames[i], string.c_str());
                break;
            }
            default:
                args.appendf("%s=<unknown type>", argNames[i]);
                break;
        }
    }
    bool open = (phase == TRACE_EVENT_PHASE_COMPLETE);
    if (open) {
        SkDebugf(
                "[% 2d]%s %s%s #%d {\n", fIndent.size(), fIndent.c_str(), name, args.c_str(), fCnt);
        fIndent.append(" ");
    } else {
        SkDebugf("%s%s #%d\n", name, args.c_str(), fCnt);
    }
    ++fCnt;
    return 0;
}

void SkDebugfTracer::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                              const char* name,
                                              SkEventTracer::Handle handle) {
    fIndent.resize(fIndent.size() - 1);
    SkDebugf("[% 2d]%s } %s\n", fIndent.size(), fIndent.c_str(), name);
}

const uint8_t* SkDebugfTracer::getCategoryGroupEnabled(const char* name) {
    static uint8_t yes = SkEventTracer::kEnabledForRecording_CategoryGroupEnabledFlags;
    return &yes;
}
