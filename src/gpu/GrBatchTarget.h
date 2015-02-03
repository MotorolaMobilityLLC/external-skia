/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBatchBuffer_DEFINED
#define GrBatchBuffer_DEFINED

#include "GrPendingProgramElement.h"
#include "GrGpu.h"
#include "GrTRecorder.h"

/*
 * GrBatch instances use this object to allocate space for their geometry and to issue the draws
 * that render their batch.
 */

class GrBatchTarget : public SkNoncopyable {
public:
    GrBatchTarget(GrGpu* gpu,
                  GrVertexBufferAllocPool* vpool,
                  GrIndexBufferAllocPool* ipool)
        : fGpu(gpu)
        , fVertexPool(vpool)
        , fIndexPool(ipool)
        , fFlushBuffer(kFlushBufferInitialSizeInBytes)
        , fIter(fFlushBuffer) {}

    typedef GrDrawTarget::DrawInfo DrawInfo;
    void initDraw(const GrPrimitiveProcessor* primProc, const GrPipeline* pipeline) {
        GrNEW_APPEND_TO_RECORDER(fFlushBuffer, BufferedFlush, (primProc, pipeline));
    }

    void draw(const GrDrawTarget::DrawInfo& draw) {
        fFlushBuffer.back().fDraws.push_back(draw);
    }

    // TODO this is temporary until batch is everywhere
    //void flush();
    void preFlush() { fIter = FlushBuffer::Iter(fFlushBuffer); }
    void flushNext();
    void postFlush() { SkASSERT(!fIter.next()); fFlushBuffer.reset(); }

    // TODO This goes away when everything uses batch
    GrBatchTracker* currentBatchTracker() {
        SkASSERT(!fFlushBuffer.empty());
        return &fFlushBuffer.back().fBatchTracker;
    }

    GrVertexBufferAllocPool* vertexPool() { return fVertexPool; }
    GrIndexBufferAllocPool* indexPool() { return fIndexPool; }

private:
    GrGpu* fGpu;
    GrVertexBufferAllocPool* fVertexPool;
    GrIndexBufferAllocPool* fIndexPool;

    typedef void* TBufferAlign; // This wouldn't be enough align if a command used long double.

    struct BufferedFlush {
        BufferedFlush(const GrPrimitiveProcessor* primProc, const GrPipeline* pipeline)
            : fPrimitiveProcessor(primProc)
            , fPipeline(pipeline)
            , fDraws(kDrawRecorderInitialSizeInBytes) {}
        typedef GrPendingProgramElement<const GrPrimitiveProcessor> ProgramPrimitiveProcessor;
        ProgramPrimitiveProcessor fPrimitiveProcessor;
        const GrPipeline* fPipeline;
        GrBatchTracker fBatchTracker;
        SkSTArray<4, DrawInfo, true> fDraws;
    };

    enum {
        kFlushBufferInitialSizeInBytes = 8 * sizeof(BufferedFlush),
        kDrawRecorderInitialSizeInBytes = 8 * sizeof(DrawInfo),
    };

    typedef GrTRecorder<BufferedFlush, TBufferAlign> FlushBuffer;

    FlushBuffer fFlushBuffer;
    // TODO this is temporary
    FlushBuffer::Iter fIter;
};

#endif
