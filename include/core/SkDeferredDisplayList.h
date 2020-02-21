/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayList_DEFINED
#define SkDeferredDisplayList_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/core/SkTypes.h"

class SkDeferredDisplayListPriv;

#if SK_SUPPORT_GPU
#include "include/private/GrRecordingContext.h"
#include "include/private/SkTArray.h"
#include <map>
class GrRenderTask;
struct GrCCPerOpsTaskPaths;
#endif

/*
 * This class contains pre-processed gpu operations that can be replayed into
 * an SkSurface via SkSurface::draw(SkDeferredDisplayList*).
 */
class SkDeferredDisplayList {
public:
    SK_API ~SkDeferredDisplayList();

    SK_API const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }

    // Provides access to functions that aren't part of the public API.
    SkDeferredDisplayListPriv priv();
    const SkDeferredDisplayListPriv priv() const;

private:
    friend class GrDrawingManager; // for access to 'fRenderTasks', 'fLazyProxyData', 'fArenas'
    friend class SkDeferredDisplayListRecorder; // for access to 'fLazyProxyData'
    friend class SkDeferredDisplayListPriv;

    class LazyProxyData;

    SK_API SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                 sk_sp<LazyProxyData>);

#if SK_SUPPORT_GPU
    const SkTArray<GrRecordingContext::ProgramData>& programData() const {
        return fProgramData;
    }
#endif

    const SkSurfaceCharacterization fCharacterization;

#if SK_SUPPORT_GPU
    // This needs to match the same type in GrCoverageCountingPathRenderer.h
    using PendingPathsMap = std::map<uint32_t, sk_sp<GrCCPerOpsTaskPaths>>;

    // These are ordered such that the destructor cleans op tasks up first (which may refer back
    // to the arena and memory pool in their destructors).
    GrRecordingContext::OwnedArenas fArenas;
    PendingPathsMap                 fPendingPaths;  // This is the path data from CCPR.
    SkTArray<sk_sp<GrRenderTask>>   fRenderTasks;

    SkTArray<GrRecordingContext::ProgramData> fProgramData;
    sk_sp<LazyProxyData>            fLazyProxyData;
#endif
};

#endif
