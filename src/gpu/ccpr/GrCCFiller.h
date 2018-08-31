/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPathParser_DEFINED
#define GrCCPathParser_DEFINED

#include "GrMesh.h"
#include "SkPath.h"
#include "SkPathPriv.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "GrTessellator.h"
#include "ccpr/GrCCCoverageProcessor.h"
#include "ccpr/GrCCFillGeometry.h"
#include "ops/GrDrawOp.h"

class GrOnFlushResourceProvider;
class SkMatrix;
class SkPath;

/**
 * This class parses SkPaths into CCPR primitives in GPU buffers, then issues calls to draw their
 * coverage counts.
 */
class GrCCFiller {
public:
    struct PathStats {
        int fMaxPointsPerPath = 0;
        int fNumTotalSkPoints = 0;
        int fNumTotalSkVerbs = 0;
        int fNumTotalConicWeights = 0;

        void statPath(const SkPath&);
    };

    GrCCFiller(int numPaths, const PathStats&);

    // Parses a device-space SkPath into the current batch, using the SkPath's original verbs and
    // 'deviceSpacePts'. Accepts an optional post-device-space translate for placement in an atlas.
    void parseDeviceSpaceFill(const SkPath&, const SkPoint* deviceSpacePts, GrScissorTest,
                              const SkIRect& clippedDevIBounds, const SkIVector& devToAtlasOffset);

    using BatchID = int;

    // Compiles the outstanding parsed paths into a batch, and returns an ID that can be used to
    // draw their fills in the future.
    BatchID closeCurrentBatch();

    // Builds internal GPU buffers and prepares for calls to drawFills(). Caller must close the
    // current batch before calling this method, and cannot parse new paths afer.
    bool prepareToDraw(GrOnFlushResourceProvider*);

    // Called after prepareToDraw(). Draws the given batch of path fills.
    void drawFills(GrOpFlushState*, BatchID, const SkIRect& drawBounds) const;

private:
    static constexpr int kNumScissorModes = 2;
    using PrimitiveTallies = GrCCFillGeometry::PrimitiveTallies;

    // Every kBeginPath verb has a corresponding PathInfo entry.
    class PathInfo {
    public:
        PathInfo(GrScissorTest scissorTest, const SkIVector& devToAtlasOffset)
                : fScissorTest(scissorTest), fDevToAtlasOffset(devToAtlasOffset) {}

        GrScissorTest scissorTest() const { return fScissorTest; }
        const SkIVector& devToAtlasOffset() const { return fDevToAtlasOffset; }

        // An empty tessellation fan is also valid; we use negative count to denote not tessellated.
        bool hasFanTessellation() const { return fFanTessellationCount >= 0; }
        int fanTessellationCount() const {
            SkASSERT(this->hasFanTessellation());
            return fFanTessellationCount;
        }
        const GrTessellator::WindingVertex* fanTessellation() const {
            SkASSERT(this->hasFanTessellation());
            return fFanTessellation.get();
        }
        void tessellateFan(const GrCCFillGeometry&, int verbsIdx, int ptsIdx,
                           const SkIRect& clippedDevIBounds, PrimitiveTallies* newTriangleCounts);

    private:
        GrScissorTest fScissorTest;
        SkIVector fDevToAtlasOffset;  // Translation from device space to location in atlas.
        int fFanTessellationCount = -1;
        std::unique_ptr<const GrTessellator::WindingVertex[]> fFanTessellation;
    };

    // Defines a batch of CCPR primitives. Start indices are deduced by looking at the previous
    // Batch in the list.
    struct Batch {
        PrimitiveTallies fEndNonScissorIndices;
        int fEndScissorSubBatchIdx;
        PrimitiveTallies fTotalPrimitiveCounts;
    };

    // Defines a sub-batch that will be drawn with the given scissor rect. Start indices are deduced
    // by looking at the previous ScissorSubBatch in the list.
    struct ScissorSubBatch {
        PrimitiveTallies fEndPrimitiveIndices;
        SkIRect fScissor;
    };

    void drawPrimitives(GrOpFlushState*, const GrPipeline&, BatchID,
                        GrCCCoverageProcessor::PrimitiveType, int PrimitiveTallies::*instanceType,
                        const SkIRect& drawBounds) const;

    GrCCFillGeometry fGeometry;
    SkSTArray<32, PathInfo, true> fPathInfos;
    SkSTArray<32, Batch, true> fBatches;
    SkSTArray<32, ScissorSubBatch, true> fScissorSubBatches;
    PrimitiveTallies fTotalPrimitiveCounts[kNumScissorModes];
    int fMaxMeshesPerDraw = 0;

    sk_sp<GrBuffer> fInstanceBuffer;
    PrimitiveTallies fBaseInstances[kNumScissorModes];
    mutable SkSTArray<32, GrMesh> fMeshesScratchBuffer;
    mutable SkSTArray<32, SkIRect> fScissorRectScratchBuffer;
};

inline void GrCCFiller::PathStats::statPath(const SkPath& path) {
    fMaxPointsPerPath = SkTMax(fMaxPointsPerPath, path.countPoints());
    fNumTotalSkPoints += path.countPoints();
    fNumTotalSkVerbs += path.countVerbs();
    fNumTotalConicWeights += SkPathPriv::ConicWeightCnt(path);
}

#endif
