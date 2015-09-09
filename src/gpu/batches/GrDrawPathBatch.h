/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawPathBatch_DEFINED
#define GrDrawPathBatch_DEFINED

#include "GrBatchFlushState.h"
#include "GrDrawBatch.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrPathRendering.h"
#include "GrPathProcessor.h"

#include "SkTLList.h"

class GrDrawPathBatchBase : public GrDrawBatch {
public:
    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        this->pathProcessor()->getInvariantOutputColor(out);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        this->pathProcessor()->getInvariantOutputCoverage(out);
    }

    void setStencilSettings(const GrStencilSettings& stencil) { fStencilSettings = stencil; }

protected:
    GrDrawPathBatchBase(const GrPathProcessor* pathProc) : fPrimitiveProcessor(pathProc) {}

    GrBatchTracker* tracker() { return reinterpret_cast<GrBatchTracker*>(&fWhatchamacallit); }
    const GrPathProcessor* pathProcessor() const { return fPrimitiveProcessor.get(); }
    const GrStencilSettings& stencilSettings() const { return fStencilSettings; }
    const GrPipelineOptimizations& opts() const { return fOpts; }

private:
    void initBatchTracker(const GrPipelineOptimizations& opts) override {
        this->pathProcessor()->initBatchTracker(this->tracker(), opts);
        fOpts = opts;
    }

    GrPendingProgramElement<const GrPathProcessor>          fPrimitiveProcessor;
    PathBatchTracker                                        fWhatchamacallit; // TODO: delete this
    GrStencilSettings                                       fStencilSettings;
    GrPipelineOptimizations                                 fOpts;

    typedef GrDrawBatch INHERITED;
};

class GrDrawPathBatch final : public GrDrawPathBatchBase {
public:
    // This can't return a more abstract type because we install the stencil settings late :(
    static GrDrawPathBatchBase* Create(const GrPathProcessor* primProc, const GrPath* path) {
        return new GrDrawPathBatch(primProc, path);
    }

    const char* name() const override { return "DrawPath"; }

    SkString dumpInfo() const override;

private:
    GrDrawPathBatch(const GrPathProcessor* pathProc, const GrPath* path)
        : INHERITED(pathProc)
        , fPath(path) {
        fBounds = path->getBounds();
        this->pathProcessor()->viewMatrix().mapRect(&fBounds);
        this->initClassID<GrDrawPathBatch>();
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override;

    GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;

    typedef GrDrawPathBatchBase INHERITED;
};

/**
 * This could be nested inside the batch class, but for now it must be declarable in a public
 * header (GrDrawContext)
 */
class GrPathRangeDraw : public GrNonAtomicRef {
public:
    typedef GrPathRendering::PathTransformType TransformType;

    static GrPathRangeDraw* Create(GrPathRange* range, TransformType transformType,
        int reserveCnt) {
        return SkNEW_ARGS(GrPathRangeDraw, (range, transformType, reserveCnt));
    }

    void append(uint16_t index, float transform[]) {
        fTransforms.push_back_n(GrPathRendering::PathTransformSize(fTransformType), transform);
        fIndices.push_back(index);
    }

    int count() const { return fIndices.count(); }

    TransformType transformType() const { return fTransformType; }

    const float* transforms() const { return fTransforms.begin(); }

    const uint16_t* indices() const { return fIndices.begin(); }

    const GrPathRange* range() const { return fPathRange.get(); }

    static bool CanMerge(const GrPathRangeDraw& a, const GrPathRangeDraw& b) {
        return a.transformType() == b.transformType() && a.range() == b.range();
    }

private:
    GrPathRangeDraw(GrPathRange* range, TransformType transformType, int reserveCnt)
        : fPathRange(SkRef(range))
        , fTransformType(transformType)
        , fIndices(reserveCnt)
        , fTransforms(reserveCnt * GrPathRendering::PathTransformSize(transformType)) {
        SkDEBUGCODE(fUsedInBatch = false;)
    }

    // Reserve space for 64 paths where indices are 16 bit and transforms are translations.
    static const int kIndexReserveCnt = 64;
    static const int kTransformBufferReserveCnt = 2 * 64;

    GrPendingIOResource<const GrPathRange, kRead_GrIOType> fPathRange;
    GrPathRendering::PathTransformType                     fTransformType;
    SkSTArray<kIndexReserveCnt, uint16_t, true>            fIndices;
    SkSTArray<kTransformBufferReserveCnt, float, true>     fTransforms;

    // To ensure we don't reuse these across batches.
#ifdef SK_DEBUG
    bool fUsedInBatch;
    friend class GrDrawPathRangeBatch;
#endif

    typedef GrNonAtomicRef INHERITED;
};

// Template this if we decide to support index types other than 16bit
class GrDrawPathRangeBatch final : public GrDrawPathBatchBase {
public:
    // This can't return a more abstracet type because we install the stencil settings late :(
    static GrDrawPathBatchBase* Create(const GrPathProcessor* pathProc,
                                       GrPathRangeDraw* pathRangeDraw) {
        return SkNEW_ARGS(GrDrawPathRangeBatch, (pathProc, pathRangeDraw));
    }

    ~GrDrawPathRangeBatch() override;

    const char* name() const override { return "DrawPathRange"; }

    SkString dumpInfo() const override;

private:
    inline bool isWinding() const;

    GrDrawPathRangeBatch(const GrPathProcessor* pathProc, GrPathRangeDraw* pathRangeDraw);

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override;

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override;

    typedef SkTLList<GrPathRangeDraw*> DrawList;
    DrawList    fDraws;
    int         fTotalPathCount;

    typedef GrDrawPathBatchBase INHERITED;
};

#endif
