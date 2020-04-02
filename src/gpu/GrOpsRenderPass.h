/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrOpsRenderPass_DEFINED
#define GrOpsRenderPass_DEFINED

#include "include/core/SkDrawable.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/ops/GrDrawOp.h"

class GrOpFlushState;
class GrFixedClip;
class GrGpu;
class GrPipeline;
class GrPrimitiveProcessor;
class GrProgramInfo;
class GrRenderTarget;
class GrSemaphore;
struct SkIRect;
struct SkRect;

/**
 * The GrOpsRenderPass is a series of commands (draws, clears, and discards), which all target the
 * same render target. It is possible that these commands execute immediately (GL), or get buffered
 * up for later execution (Vulkan). GrOps execute into a GrOpsRenderPass.
 */
class GrOpsRenderPass {
public:
    virtual ~GrOpsRenderPass() {}

    struct LoadAndStoreInfo {
        GrLoadOp    fLoadOp;
        GrStoreOp   fStoreOp;
        SkPMColor4f fClearColor;
    };

    // Load-time clears of the stencil buffer are always to 0 so we don't store
    // an 'fStencilClearValue'
    struct StencilLoadAndStoreInfo {
        GrLoadOp  fLoadOp;
        GrStoreOp fStoreOp;
    };

    void begin();
    // Signals the end of recording to the GrOpsRenderPass and that it can now be submitted.
    void end();

    // Updates the internal pipeline state for drawing with the provided GrProgramInfo. Enters an
    // internal "bad" state if the pipeline could not be set.
    void bindPipeline(const GrProgramInfo&, const SkRect& drawBounds);

    // The scissor rect is always dynamic state and therefore not stored on GrPipeline. If scissor
    // test is enabled on the current pipeline, then the client must call setScissorRect() before
    // drawing. The scissor rect may also be updated between draws without having to bind a new
    // pipeline.
    void setScissorRect(const SkIRect&);

    // Binds textures for the primitive processor and any FP on the GrPipeline. Texture bindings are
    // dynamic state and therefore not set during bindPipeline(). If the current program uses
    // textures, then the client must call bindTextures() before drawing. The primitive processor
    // textures may also be updated between draws by calling bindTextures() again with a different
    // array for primProcTextures. (On subsequent calls, if the backend is capable of updating the
    // primitive processor textures independently, then it will automatically skip re-binding
    // FP textures from GrPipeline.)
    //
    // If the current program does not use textures, this is a no-op.
    void bindTextures(const GrPrimitiveProcessor&, const GrSurfaceProxy* const primProcTextures[],
                      const GrPipeline&);

    void bindBuffers(const GrBuffer* indexBuffer, const GrBuffer* instanceBuffer,
                     const GrBuffer* vertexBuffer, GrPrimitiveRestart = GrPrimitiveRestart::kNo);

    // These methods issue draws using the current pipeline state. Before drawing, the caller must
    // configure the pipeline and dynamic state:
    //
    //   - Call bindPipeline()
    //   - If the scissor test is enabled, call setScissorRect()
    //   - If the current program uses textures, call bindTextures()
    //   - Call bindBuffers() (even if all buffers are null)
    void draw(int vertexCount, int baseVertex);
    void drawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue, uint16_t maxIndexValue,
                     int baseVertex);
    void drawInstanced(int instanceCount, int baseInstance, int vertexCount, int baseVertex);
    void drawIndexedInstanced(int indexCount, int baseIndex, int instanceCount, int baseInstance,
                              int baseVertex);

    // This is a helper method for drawing a repeating pattern of vertices. The bound index buffer
    // is understood to contain 'maxPatternRepetitionsInIndexBuffer' repetitions of the pattern.
    // If more repetitions are required, then we loop.
    void drawIndexPattern(int patternIndexCount, int patternRepeatCount,
                          int maxPatternRepetitionsInIndexBuffer, int patternVertexCount,
                          int baseVertex);

    // Performs an upload of vertex data in the middle of a set of a set of draws
    virtual void inlineUpload(GrOpFlushState*, GrDeferredTextureUploadFn&) = 0;

    /**
     * Clear the owned render target. Ignores the draw state and clip.
     */
    void clear(const GrFixedClip&, const SkPMColor4f&);

    void clearStencilClip(const GrFixedClip&, bool insideStencilMask);

    /**
     * Executes the SkDrawable object for the underlying backend.
     */
    void executeDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>);

protected:
    GrOpsRenderPass() : fOrigin(kTopLeft_GrSurfaceOrigin), fRenderTarget(nullptr) {}

    GrOpsRenderPass(GrRenderTarget* rt, GrSurfaceOrigin origin)
            : fOrigin(origin)
            , fRenderTarget(rt) {
    }

    void set(GrRenderTarget* rt, GrSurfaceOrigin origin) {
        SkASSERT(!fRenderTarget);

        fRenderTarget = rt;
        fOrigin = origin;
    }

    GrSurfaceOrigin fOrigin;
    GrRenderTarget* fRenderTarget;

    // Backends may defer binding of certain buffers if their draw API requires a buffer, or if
    // their bind methods don't support base values.
    sk_sp<const GrBuffer> fActiveIndexBuffer;
    sk_sp<const GrBuffer> fActiveVertexBuffer;
    sk_sp<const GrBuffer> fActiveInstanceBuffer;

private:
    virtual GrGpu* gpu() = 0;

    void resetActiveBuffers() {
        fActiveIndexBuffer.reset();
        fActiveInstanceBuffer.reset();
        fActiveVertexBuffer.reset();
    }

    bool prepareToDraw();

    // overridden by backend-specific derived class to perform the rendering command.
    virtual void onBegin() {}
    virtual void onEnd() {}
    virtual bool onBindPipeline(const GrProgramInfo&, const SkRect& drawBounds) = 0;
    virtual void onSetScissorRect(const SkIRect&) = 0;
    virtual bool onBindTextures(const GrPrimitiveProcessor&,
                                const GrSurfaceProxy* const primProcTextures[],
                                const GrPipeline&) = 0;
    virtual void onBindBuffers(const GrBuffer* indexBuffer, const GrBuffer* instanceBuffer,
                               const GrBuffer* vertexBuffer, GrPrimitiveRestart) = 0;
    virtual void onDraw(int vertexCount, int baseVertex) = 0;
    virtual void onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                               uint16_t maxIndexValue, int baseVertex) = 0;
    virtual void onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                                 int baseVertex) = 0;
    virtual void onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount,
                                        int baseInstance, int baseVertex) = 0;
    virtual void onClear(const GrFixedClip&, const SkPMColor4f&) = 0;
    virtual void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) = 0;
    virtual void onExecuteDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>) {}

    enum class DrawPipelineStatus {
        kOk = 0,
        kNotConfigured,
        kFailedToBind
    };

    DrawPipelineStatus fDrawPipelineStatus = DrawPipelineStatus::kNotConfigured;
    GrXferBarrierType fXferBarrierType;

#ifdef SK_DEBUG
    enum class DynamicStateStatus {
        kDisabled,
        kUninitialized,
        kConfigured
    };

    DynamicStateStatus fScissorStatus;
    DynamicStateStatus fTextureBindingStatus;
    bool fHasIndexBuffer;
    DynamicStateStatus fInstanceBufferStatus;
    DynamicStateStatus fVertexBufferStatus;
#endif

    typedef GrOpsRenderPass INHERITED;
};

#endif
