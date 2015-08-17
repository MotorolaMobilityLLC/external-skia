/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTargetCommands_DEFINED
#define GrTargetCommands_DEFINED

#include "GrDrawTarget.h"
#include "GrPath.h"
#include "GrPendingProgramElement.h"
#include "GrPrimitiveProcessor.h"
#include "GrRenderTarget.h"
#include "GrTRecorder.h"

#include "batches/GrDrawBatch.h"
#include "SkRect.h"

class GrResourceProvider;
class GrBatchFlushState;

// TODO: Convert all commands into GrBatch and remove this class.
class GrTargetCommands : ::SkNoncopyable {
public:
    GrTargetCommands() : fCmdBuffer(kCmdBufferInitialSizeInBytes), fLastFlushToken(0) {}

    class Cmd : ::SkNoncopyable {
    public:
        enum CmdType {
            kStencilPath_CmdType       = 1,
            kClear_CmdType             = 2,
            kClearStencil_CmdType      = 3,
            kCopySurface_CmdType       = 4,
            kDrawPath_CmdType          = 5,
            kDrawPaths_CmdType         = 6,
            kDrawBatch_CmdType         = 7,
        };

        Cmd(CmdType type)
            : fType(type)
#if GR_BATCH_SPEW
            , fUniqueID(GenID(&gUniqueID))
#endif
        {}
        virtual ~Cmd() {}

        virtual void execute(GrBatchFlushState*) = 0;

        CmdType type() const { return fType; }

        GrBATCH_SPEW(uint32_t uniqueID() const { return fUniqueID;} )

    private:
        // TODO move this to a common header so it can be shared with GrBatch
        static uint32_t GenID(int32_t* idCounter) {
            uint32_t id = static_cast<uint32_t>(sk_atomic_inc(idCounter)) + 1;
            if (!id) {
                SkFAIL("This should never wrap\n");
            }
            return id;
        }
        CmdType          fType;
        GrBATCH_SPEW(uint32_t fUniqueID);
        GrBATCH_SPEW(static int32_t gUniqueID;)
    };

    void reset();
    void flush(GrGpu*, GrResourceProvider*);

private:
    friend class GrCommandBuilder;
    friend class GrBufferedDrawTarget; // This goes away when State becomes just a pipeline
    friend class GrReorderCommandBuilder;

    typedef GrGpu::DrawArgs DrawArgs;

    // TODO: This can be just a pipeline once paths are in batch, and it should live elsewhere
    struct StateForPathDraw : public SkNVRefCnt<StateForPathDraw> {
        // TODO get rid of the prim proc parameter when we use batch everywhere
        StateForPathDraw(const GrPrimitiveProcessor* primProc = NULL)
            : fPrimitiveProcessor(primProc)
            , fCompiled(false) {}

        ~StateForPathDraw() { reinterpret_cast<GrPipeline*>(fPipeline.get())->~GrPipeline(); }

        // This function is only for getting the location in memory where we will create our
        // pipeline object.
        void* pipelineLocation() { return fPipeline.get(); }

        const GrPipeline* getPipeline() const {
            return reinterpret_cast<const GrPipeline*>(fPipeline.get());
        }
        GrRenderTarget* getRenderTarget() const {
            return this->getPipeline()->getRenderTarget();
        }
        const GrXferProcessor* getXferProcessor() const {
            return this->getPipeline()->getXferProcessor();
        }

        void operator delete(void* p) {}
        void* operator new(size_t) {
            SkFAIL("All States are created by placement new.");
            return sk_malloc_throw(0);
        }

        void* operator new(size_t, void* p) { return p; }
        void operator delete(void* target, void* placement) {
            ::operator delete(target, placement);
        }

        typedef GrPendingProgramElement<const GrPrimitiveProcessor> ProgramPrimitiveProcessor;
        ProgramPrimitiveProcessor               fPrimitiveProcessor;
        SkAlignedSStorage<sizeof(GrPipeline)>   fPipeline;
        GrProgramDesc                           fDesc;
        GrBatchTracker                          fBatchTracker;
        bool                                    fCompiled;
    };
    // TODO remove this when State is just a pipeline
    friend SkNVRefCnt<StateForPathDraw>;

    struct StencilPath : public Cmd {
        StencilPath(const GrPath* path, GrRenderTarget* rt)
            : Cmd(kStencilPath_CmdType)
            , fRenderTarget(rt)
            , fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        void execute(GrBatchFlushState*) override;

        SkMatrix                                                fViewMatrix;
        bool                                                    fUseHWAA;
        GrStencilSettings                                       fStencil;
        GrScissorState                                          fScissor;
    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;
        GrPendingIOResource<const GrPath, kRead_GrIOType>       fPath;
    };

    struct DrawPath : public Cmd {
        DrawPath(StateForPathDraw* state, const GrPath* path)
            : Cmd(kDrawPath_CmdType)
            , fState(SkRef(state))
            , fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        void execute(GrBatchFlushState*) override;

        SkAutoTUnref<StateForPathDraw>  fState;
        GrStencilSettings               fStencilSettings;
    private:
        GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;
    };

    struct DrawPaths : public Cmd {
        DrawPaths(StateForPathDraw* state, const GrPathRange* pathRange)
            : Cmd(kDrawPaths_CmdType)
            , fState(SkRef(state))
            , fPathRange(pathRange) {}

        const GrPathRange* pathRange() const { return fPathRange.get();  }

        void execute(GrBatchFlushState*) override;

        SkAutoTUnref<StateForPathDraw>  fState;
        char*                           fIndices;
        GrDrawTarget::PathIndexType     fIndexType;
        float*                          fTransforms;
        GrDrawTarget::PathTransformType fTransformType;
        int                             fCount;
        GrStencilSettings               fStencilSettings;

    private:
        GrPendingIOResource<const GrPathRange, kRead_GrIOType> fPathRange;
    };

    // This is also used to record a discard by setting the color to GrColor_ILLEGAL
    struct Clear : public Cmd {
        Clear(GrRenderTarget* rt) : Cmd(kClear_CmdType), fRenderTarget(rt) {}

        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        void execute(GrBatchFlushState*) override;

        SkIRect fRect;
        GrColor fColor;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    // This command is ONLY used by the clip mask manager to clear the stencil clip bits
    struct ClearStencilClip : public Cmd {
        ClearStencilClip(GrRenderTarget* rt) : Cmd(kClearStencil_CmdType), fRenderTarget(rt) {}

        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        void execute(GrBatchFlushState*) override;

        SkIRect fRect;
        bool    fInsideClip;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    struct CopySurface : public Cmd {
        CopySurface(GrSurface* dst, GrSurface* src)
            : Cmd(kCopySurface_CmdType)
            , fDst(dst)
            , fSrc(src) {
        }

        GrSurface* dst() const { return fDst.get(); }
        GrSurface* src() const { return fSrc.get(); }

        void execute(GrBatchFlushState*) override;

        SkIPoint    fDstPoint;
        SkIRect     fSrcRect;

    private:
        GrPendingIOResource<GrSurface, kWrite_GrIOType> fDst;
        GrPendingIOResource<GrSurface, kRead_GrIOType> fSrc;
    };

    struct DrawBatch : public Cmd {
        DrawBatch(GrDrawBatch* batch)
            : Cmd(kDrawBatch_CmdType)
            , fBatch(SkRef(batch)){
            SkASSERT(!batch->isUsed());
        }

        GrDrawBatch* batch() { return fBatch; }
        void execute(GrBatchFlushState*) override;

    private:
        SkAutoTUnref<GrDrawBatch>   fBatch;
    };

    static const int kCmdBufferInitialSizeInBytes = 8 * 1024;

    typedef void* TCmdAlign; // This wouldn't be enough align if a command used long double.
    typedef GrTRecorder<Cmd, TCmdAlign> CmdBuffer;

    CmdBuffer* cmdBuffer() { return &fCmdBuffer; }

    CmdBuffer                           fCmdBuffer;
    GrBatchToken                        fLastFlushToken;
};

#endif
