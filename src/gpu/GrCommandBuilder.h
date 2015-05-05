/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCommandBuilder_DEFINED
#define GrCommandBuilder_DEFINED

#include "GrTargetCommands.h"

class GrInOrderDrawBuffer;
class GrVertexBufferAllocPool;
class GrIndexBufferAllocPool;

class GrCommandBuilder : ::SkNoncopyable {
public:
    typedef GrTargetCommands::Cmd Cmd;
    typedef GrTargetCommands::State State;

    GrCommandBuilder(GrGpu* gpu,
                     GrVertexBufferAllocPool* vertexPool,
                     GrIndexBufferAllocPool* indexPool)
        : fCommands(gpu, vertexPool, indexPool) {
    }

    virtual ~GrCommandBuilder() {}

    void reset() { fCommands.reset(); }
    void flush(GrInOrderDrawBuffer* iodb) { fCommands.flush(iodb); }

    virtual Cmd* recordClearStencilClip(const SkIRect& rect,
                                        bool insideClip,
                                        GrRenderTarget* renderTarget);
    virtual Cmd* recordDiscard(GrRenderTarget*);
    virtual Cmd* recordDrawBatch(State*, GrBatch*);
    virtual Cmd* recordStencilPath(const GrPipelineBuilder&,
                                   const GrPathProcessor*,
                                   const GrPath*,
                                   const GrScissorState&,
                                   const GrStencilSettings&);
    virtual Cmd* recordDrawPath(State*,
                                const GrPathProcessor*,
                                const GrPath*,
                                const GrStencilSettings&);
    virtual Cmd* recordDrawPaths(State*,
                                 GrInOrderDrawBuffer*,
                                 const GrPathProcessor*,
                                 const GrPathRange*,
                                 const void*,
                                 GrDrawTarget::PathIndexType,
                                 const float transformValues[],
                                 GrDrawTarget::PathTransformType ,
                                 int,
                                 const GrStencilSettings&,
                                 const GrDrawTarget::PipelineInfo&);
    virtual Cmd* recordClear(const SkIRect* rect,
                             GrColor,
                             bool canIgnoreRect,
                             GrRenderTarget*);
    virtual Cmd* recordCopySurface(GrSurface* dst,
                                   GrSurface* src,
                                   const SkIRect& srcRect,
                                   const SkIPoint& dstPoint);
    virtual Cmd* recordXferBarrierIfNecessary(const GrPipeline&, const GrDrawTargetCaps&);

private:
    typedef GrTargetCommands::DrawBatch DrawBatch;
    typedef GrTargetCommands::StencilPath StencilPath;
    typedef GrTargetCommands::DrawPath DrawPath;
    typedef GrTargetCommands::DrawPaths DrawPaths;
    typedef GrTargetCommands::Clear Clear;
    typedef GrTargetCommands::ClearStencilClip ClearStencilClip;
    typedef GrTargetCommands::CopySurface CopySurface;
    typedef GrTargetCommands::XferBarrier XferBarrier;

    GrTargetCommands::CmdBuffer* cmdBuffer() { return fCommands.cmdBuffer(); }
    GrBatchTarget* batchTarget() { return fCommands.batchTarget(); }

    GrTargetCommands fCommands;

};

#endif
