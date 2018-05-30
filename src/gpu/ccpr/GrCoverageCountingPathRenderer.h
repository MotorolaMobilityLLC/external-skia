/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoverageCountingPathRenderer_DEFINED
#define GrCoverageCountingPathRenderer_DEFINED

#include "GrPathRenderer.h"
#include "GrRenderTargetOpList.h"
#include "SkArenaAlloc.h"
#include "SkTInternalLList.h"
#include "ccpr/GrCCClipPath.h"
#include "ccpr/GrCCPerFlushResources.h"
#include <map>

class GrCCDrawPathsOp;

/**
 * Tracks all the paths in a given opList that will be drawn when it flushes.
 */
// DDL TODO: given the usage pattern in DDL mode, this could probably be non-atomic refcounting.
struct GrCCPerOpListPaths : SkRefCnt {
    ~GrCCPerOpListPaths() {
        // Ensure there are no surviving DrawPathsOps with a dangling pointer into this class.
        if (!fDrawOps.isEmpty()) {
            SK_ABORT("GrCCDrawPathsOp(s) not deleted during flush");
        }
        // Clip lazy proxies also reference this class from their callbacks, but those callbacks
        // are only invoked at flush time while we are still alive. (Unlike DrawPathsOps, that
        // unregister themselves upon destruction.) So it shouldn't matter if any clip proxies
        // are still around.
    }

    SkTInternalLList<GrCCDrawPathsOp> fDrawOps;
    std::map<uint32_t, GrCCClipPath> fClipPaths;
    SkSTArenaAlloc<10 * 1024> fAllocator{10 * 1024 * 2};
    sk_sp<const GrCCPerFlushResources> fFlushResources;
};

/**
 * This is a path renderer that draws antialiased paths by counting coverage in an offscreen
 * buffer. (See GrCCCoverageProcessor, GrCCPathProcessor.)
 *
 * It also serves as the per-render-target tracker for pending path draws, and at the start of
 * flush, it compiles GPU buffers and renders a "coverage count atlas" for the upcoming paths.
 */
class GrCoverageCountingPathRenderer : public GrPathRenderer, public GrOnFlushCallbackObject {
public:
    static bool IsSupported(const GrCaps&);
    static sk_sp<GrCoverageCountingPathRenderer> CreateIfSupported(const GrCaps&,
                                                                   bool drawCachablePaths);
    ~GrCoverageCountingPathRenderer() override {
        // Ensure callers are actually flushing paths they record, not causing us to leak memory.
        SkASSERT(fPendingPaths.empty());
        SkASSERT(!fFlushing);
    }

    using PendingPathsMap = std::map<uint32_t, sk_sp<GrCCPerOpListPaths>>;

    // In DDL mode, Ganesh needs to be able to move the pending GrCCPerOpListPaths to the DDL object
    // (detachPendingPaths) and then return them upon replay (mergePendingPaths).
    PendingPathsMap detachPendingPaths() { return std::move(fPendingPaths); }

    void mergePendingPaths(const PendingPathsMap& paths) {
#ifdef SK_DEBUG
        // Ensure there are no duplicate opList IDs between the incoming path map and ours.
        // This should always be true since opList IDs are globally unique and these are coming
        // from different DDL recordings.
        for (const auto& it : paths) {
            SkASSERT(!fPendingPaths.count(it.first));
        }
#endif

        fPendingPaths.insert(paths.begin(), paths.end());
    }

    // GrPathRenderer overrides.
    StencilSupport onGetStencilSupport(const GrShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }
    CanDrawPath onCanDrawPath(const CanDrawPathArgs& args) const override;
    bool onDrawPath(const DrawPathArgs&) override;

    std::unique_ptr<GrFragmentProcessor> makeClipProcessor(GrProxyProvider*, uint32_t oplistID,
                                                           const SkPath& deviceSpacePath,
                                                           const SkIRect& accessRect,
                                                           int rtWidth, int rtHeight);

    // GrOnFlushCallbackObject overrides.
    void preFlush(GrOnFlushResourceProvider*, const uint32_t* opListIDs, int numOpListIDs,
                  SkTArray<sk_sp<GrRenderTargetContext>>* atlasDraws) override;
    void postFlush(GrDeferredUploadToken, const uint32_t* opListIDs, int numOpListIDs) override;

private:
    GrCoverageCountingPathRenderer(bool drawCachablePaths)
            : fDrawCachablePaths(drawCachablePaths) {}

    GrCCPerOpListPaths* lookupPendingPaths(uint32_t opListID);
    void adoptAndRecordOp(GrCCDrawPathsOp*, const DrawPathArgs&);

    // fPendingPaths holds the GrCCPerOpListPaths objects that have already been created, but not
    // flushed, and those that are still being created. All GrCCPerOpListPaths objects will first
    // reside in fPendingPaths, then be moved to fFlushingPaths during preFlush().
    PendingPathsMap fPendingPaths;

    // fFlushingPaths holds the GrCCPerOpListPaths objects that are currently being flushed.
    // (It will only contain elements when fFlushing is true.)
    SkSTArray<4, sk_sp<GrCCPerOpListPaths>> fFlushingPaths;
    SkDEBUGCODE(bool fFlushing = false);

    const bool fDrawCachablePaths;
};

#endif
