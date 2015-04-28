/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawTarget_DEFINED
#define GrDrawTarget_DEFINED

#include "GrClip.h"
#include "GrClipMaskManager.h"
#include "GrContext.h"
#include "GrPathProcessor.h"
#include "GrPrimitiveProcessor.h"
#include "GrIndexBuffer.h"
#include "GrPathRendering.h"
#include "GrPipelineBuilder.h"
#include "GrTraceMarker.h"
#include "GrVertexBuffer.h"

#include "SkClipStack.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkStrokeRec.h"
#include "SkTArray.h"
#include "SkTLazy.h"
#include "SkTypes.h"
#include "SkXfermode.h"

class GrBatch;
class GrClip;
class GrDrawTargetCaps;
class GrPath;
class GrPathRange;
class GrPipeline;

class GrDrawTarget : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrDrawTarget)

    typedef GrPathRange::PathIndexType PathIndexType;
    typedef GrPathRendering::PathTransformType PathTransformType;

    ///////////////////////////////////////////////////////////////////////////

    // The context may not be fully constructed and should not be used during GrDrawTarget
    // construction.
    GrDrawTarget(GrContext* context);
    virtual ~GrDrawTarget() {}

    /**
     * Gets the capabilities of the draw target.
     */
    const GrDrawTargetCaps* caps() const { return fCaps.get(); }

    // TODO devbounds should live on the batch
    void drawBatch(GrPipelineBuilder*, GrBatch*, const SkRect* devBounds = NULL);

    /**
     * Draws path into the stencil buffer. The fill must be either even/odd or
     * winding (not inverse or hairline). It will respect the HW antialias flag
     * on the GrPipelineBuilder (if possible in the 3D API).  Note, we will never have an inverse
     * fill with stencil path
     */
    void stencilPath(GrPipelineBuilder*, const GrPathProcessor*, const GrPath*,
                     GrPathRendering::FillType);

    /**
     * Draws a path. Fill must not be a hairline. It will respect the HW
     * antialias flag on the GrPipelineBuilder (if possible in the 3D API).
     */
    void drawPath(GrPipelineBuilder*, const GrPathProcessor*, const GrPath*,
                  GrPathRendering::FillType);

    /**
     * Draws the aggregate path from combining multiple. Note that this will not
     * always be equivalent to back-to-back calls to drawPath(). It will respect
     * the HW antialias flag on the GrPipelineBuilder (if possible in the 3D API).
     *
     * @param pathRange       Source paths to draw from
     * @param indices         Array of path indices to draw
     * @param indexType       Data type of the array elements in indexBuffer
     * @param transformValues Array of transforms for the individual paths
     * @param transformType   Type of transforms in transformBuffer
     * @param count           Number of paths to draw
     * @param fill            Fill type for drawing all the paths
     */
    void drawPaths(GrPipelineBuilder*,
                   const GrPathProcessor*,
                   const GrPathRange* pathRange,
                   const void* indices,
                   PathIndexType indexType,
                   const float transformValues[],
                   PathTransformType transformType,
                   int count,
                   GrPathRendering::FillType fill);

    /**
     * Helper function for drawing rects.
     *
     * @param rect        the rect to draw
     * @param localRect   optional rect that specifies local coords to map onto
     *                    rect. If NULL then rect serves as the local coords.
     * @param localMatrix Optional local matrix. The local coordinates are specified by localRect,
     *                    or if it is NULL by rect. This matrix applies to the coordinate implied by
     *                    that rectangle before it is input to GrCoordTransforms that read local
     *                    coordinates
     */
    void drawRect(GrPipelineBuilder* pipelineBuilder,
                  GrColor color,
                  const SkMatrix& viewMatrix,
                  const SkRect& rect,
                  const SkRect* localRect,
                  const SkMatrix* localMatrix) {
        this->onDrawRect(pipelineBuilder, color, viewMatrix, rect, localRect, localMatrix);
    }

    /**
     * Helper for drawRect when the caller doesn't need separate local rects or matrices.
     */
    void drawSimpleRect(GrPipelineBuilder* ds, GrColor color, const SkMatrix& viewM,
                        const SkRect& rect) {
        this->drawRect(ds, color, viewM, rect, NULL, NULL);
    }
    void drawSimpleRect(GrPipelineBuilder* ds, GrColor color, const SkMatrix& viewM,
                        const SkIRect& irect) {
        SkRect rect = SkRect::Make(irect);
        this->drawRect(ds, color, viewM, rect, NULL, NULL);
    }


    /**
     * Clear the passed in render target. Ignores the GrPipelineBuilder and clip. Clears the whole
     * thing if rect is NULL, otherwise just the rect. If canIgnoreRect is set then the entire
     * render target can be optionally cleared.
     */
    void clear(const SkIRect* rect,
               GrColor color,
               bool canIgnoreRect,
               GrRenderTarget* renderTarget);

    /**
     * Discards the contents render target.
     **/
    virtual void discard(GrRenderTarget*) = 0;

    /**
     * Called at start and end of gpu trace marking
     * GR_CREATE_GPU_TRACE_MARKER(marker_str, target) will automatically call these at the start
     * and end of a code block respectively
     */
    void addGpuTraceMarker(const GrGpuTraceMarker* marker);
    void removeGpuTraceMarker(const GrGpuTraceMarker* marker);

    /**
     * Takes the current active set of markers and stores them for later use. Any current marker
     * in the active set is removed from the active set and the targets remove function is called.
     * These functions do not work as a stack so you cannot call save a second time before calling
     * restore. Also, it is assumed that when restore is called the current active set of markers
     * is empty. When the stored markers are added back into the active set, the targets add marker
     * is called.
     */
    void saveActiveTraceMarkers();
    void restoreActiveTraceMarkers();

    /**
     * Copies a pixel rectangle from one surface to another. This call may finalize
     * reserved vertex/index data (as though a draw call was made). The src pixels
     * copied are specified by srcRect. They are copied to a rect of the same
     * size in dst with top left at dstPoint. If the src rect is clipped by the
     * src bounds then  pixel values in the dst rect corresponding to area clipped
     * by the src rect are not overwritten. This method can fail and return false
     * depending on the type of surface, configs, etc, and the backend-specific
     * limitations. If rect is clipped out entirely by the src or dst bounds then
     * true is returned since there is no actual copy necessary to succeed.
     */
    bool copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint);
    /**
     * Function that determines whether a copySurface call would succeed without actually
     * performing the copy.
     */
    bool canCopySurface(const GrSurface* dst,
                        const GrSurface* src,
                        const SkIRect& srcRect,
                        const SkIPoint& dstPoint);

    /**
     * Release any resources that are cached but not currently in use. This
     * is intended to give an application some recourse when resources are low.
     */
    virtual void purgeResources() {};

    ///////////////////////////////////////////////////////////////////////////
    // Draw execution tracking (for font atlases and other resources)
    class DrawToken {
    public:
        DrawToken(GrDrawTarget* drawTarget, uint32_t drawID) :
                  fDrawTarget(drawTarget), fDrawID(drawID) {}

        bool isIssued() { return fDrawTarget && fDrawTarget->isIssued(fDrawID); }

    private:
        GrDrawTarget*  fDrawTarget;
        uint32_t       fDrawID;   // this may wrap, but we're doing direct comparison
                                  // so that should be okay
    };

    virtual DrawToken getCurrentDrawToken() { return DrawToken(this, 0); }

    /**
     * Used to communicate draws to GPUs / subclasses
     */
    class DrawInfo {
    public:
        DrawInfo() { fDevBounds = NULL; }
        DrawInfo(const DrawInfo& di) { (*this) = di; }
        DrawInfo& operator =(const DrawInfo& di);

        GrPrimitiveType primitiveType() const { return fPrimitiveType; }
        int startVertex() const { return fStartVertex; }
        int startIndex() const { return fStartIndex; }
        int vertexCount() const { return fVertexCount; }
        int indexCount() const { return fIndexCount; }
        int verticesPerInstance() const { return fVerticesPerInstance; }
        int indicesPerInstance() const { return fIndicesPerInstance; }
        int instanceCount() const { return fInstanceCount; }

        void setPrimitiveType(GrPrimitiveType type) { fPrimitiveType = type; }
        void setStartVertex(int startVertex) { fStartVertex = startVertex; }
        void setStartIndex(int startIndex) { fStartIndex = startIndex; }
        void setVertexCount(int vertexCount) { fVertexCount = vertexCount; }
        void setIndexCount(int indexCount) { fIndexCount = indexCount; }
        void setVerticesPerInstance(int verticesPerI) { fVerticesPerInstance = verticesPerI; }
        void setIndicesPerInstance(int indicesPerI) { fIndicesPerInstance = indicesPerI; }
        void setInstanceCount(int instanceCount) { fInstanceCount = instanceCount; }

        bool isIndexed() const { return fIndexCount > 0; }
#ifdef SK_DEBUG
        bool isInstanced() const; // this version is longer because of asserts
#else
        bool isInstanced() const { return fInstanceCount > 0; }
#endif

        // adds or remove instances
        void adjustInstanceCount(int instanceOffset);
        // shifts the start vertex
        void adjustStartVertex(int vertexOffset) {
            fStartVertex += vertexOffset;
            SkASSERT(fStartVertex >= 0);
        }
        // shifts the start index
        void adjustStartIndex(int indexOffset) {
            SkASSERT(this->isIndexed());
            fStartIndex += indexOffset;
            SkASSERT(fStartIndex >= 0);
        }
        void setDevBounds(const SkRect& bounds) {
            fDevBoundsStorage = bounds;
            fDevBounds = &fDevBoundsStorage;
        }
        const GrVertexBuffer* vertexBuffer() const { return fVertexBuffer.get(); }
        const GrIndexBuffer* indexBuffer() const { return fIndexBuffer.get(); }
        void setVertexBuffer(const GrVertexBuffer* vb) {
            fVertexBuffer.reset(vb);
        }
        void setIndexBuffer(const GrIndexBuffer* ib) {
            fIndexBuffer.reset(ib);
        }
        const SkRect* getDevBounds() const { return fDevBounds; }

    private:
        friend class GrDrawTarget;

        GrPrimitiveType         fPrimitiveType;

        int                     fStartVertex;
        int                     fStartIndex;
        int                     fVertexCount;
        int                     fIndexCount;

        int                     fInstanceCount;
        int                     fVerticesPerInstance;
        int                     fIndicesPerInstance;

        SkRect                  fDevBoundsStorage;
        SkRect*                 fDevBounds;

        GrPendingIOResource<const GrVertexBuffer, kRead_GrIOType> fVertexBuffer;
        GrPendingIOResource<const GrIndexBuffer, kRead_GrIOType>  fIndexBuffer;
    };

    bool programUnitTest(int maxStages);

protected:
    friend class GrTargetCommands; // for PipelineInfo

    GrContext* getContext() { return fContext; }
    const GrContext* getContext() const { return fContext; }

    // Subclass must initialize this in its constructor.
    SkAutoTUnref<const GrDrawTargetCaps> fCaps;

    const GrTraceMarkerSet& getActiveTraceMarkers() { return fActiveTraceMarkers; }

    // Makes a copy of the dst if it is necessary for the draw. Returns false if a copy is required
    // but couldn't be made. Otherwise, returns true.  This method needs to be protected because it
    // needs to be accessed by GLPrograms to setup a correct drawstate
    bool setupDstReadIfNecessary(const GrPipelineBuilder&,
                                 const GrProcOptInfo& colorPOI,
                                 const GrProcOptInfo& coveragePOI,
                                 GrDeviceCoordTexture* dstCopy,
                                 const SkRect* drawBounds);

    struct PipelineInfo {
        PipelineInfo(GrPipelineBuilder* pipelineBuilder, GrScissorState* scissor,
                     const GrPrimitiveProcessor* primProc,
                     const SkRect* devBounds, GrDrawTarget* target);

        PipelineInfo(GrPipelineBuilder* pipelineBuilder, GrScissorState* scissor,
                     const GrBatch* batch, const SkRect* devBounds,
                     GrDrawTarget* target);

        bool willBlendWithDst(const GrPrimitiveProcessor* primProc) const {
            return fPipelineBuilder->willBlendWithDst(primProc);
        }
    private:
        friend class GrDrawTarget;

        bool mustSkipDraw() const { return (NULL == fPipelineBuilder); }

        GrPipelineBuilder*      fPipelineBuilder;
        GrScissorState*         fScissor;
        GrProcOptInfo           fColorPOI; 
        GrProcOptInfo           fCoveragePOI; 
        GrDeviceCoordTexture    fDstCopy;
    };

    void setupPipeline(const PipelineInfo& pipelineInfo, GrPipeline* pipeline);

private:
    /**
     * This will be called before allocating a texture as a dst for copySurface. This function
     * populates the dstDesc's config, flags, and origin so as to maximize efficiency and guarantee
     * success of the copySurface call.
     */
    void initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* dstDesc) {
        if (!this->onInitCopySurfaceDstDesc(src, dstDesc)) {
            dstDesc->fOrigin = kDefault_GrSurfaceOrigin;
            dstDesc->fFlags = kRenderTarget_GrSurfaceFlag;
            dstDesc->fConfig = src->config();
        }
    }

    /** Internal implementation of canCopySurface. */
    bool internalCanCopySurface(const GrSurface* dst,
                                const GrSurface* src,
                                const SkIRect& clippedSrcRect,
                                const SkIPoint& clippedDstRect);

    virtual void onDrawBatch(GrBatch*, const PipelineInfo&) = 0;
    // TODO copy in order drawbuffer onDrawRect to here
    virtual void onDrawRect(GrPipelineBuilder*,
                            GrColor color,
                            const SkMatrix& viewMatrix,
                            const SkRect& rect,
                            const SkRect* localRect,
                            const SkMatrix* localMatrix) = 0;

    virtual void onStencilPath(const GrPipelineBuilder&,
                               const GrPathProcessor*,
                               const GrPath*,
                               const GrScissorState&,
                               const GrStencilSettings&) = 0;
    virtual void onDrawPath(const GrPathProcessor*,
                            const GrPath*,
                            const GrStencilSettings&,
                            const PipelineInfo&) = 0;
    virtual void onDrawPaths(const GrPathProcessor*,
                             const GrPathRange*,
                             const void* indices,
                             PathIndexType,
                             const float transformValues[],
                             PathTransformType,
                             int count,
                             const GrStencilSettings&,
                             const PipelineInfo&) = 0;

    virtual void onClear(const SkIRect* rect, GrColor color, bool canIgnoreRect,
                         GrRenderTarget* renderTarget) = 0;

    /** The subclass will get a chance to copy the surface for falling back to the default
        implementation, which simply draws a rectangle (and fails if dst isn't a render target). It
        should assume that any clipping has already been performed on the rect and point. It won't
        be called if the copy can be skipped. */
    virtual bool onCopySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint) = 0;

    /** Indicates whether onCopySurface would succeed. It should assume that any clipping has
        already been performed on the rect and point. It won't be called if the copy can be
        skipped. */
    virtual bool onCanCopySurface(const GrSurface* dst,
                                  const GrSurface* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) = 0;
    /**
     * This will be called before allocating a texture to be a dst for onCopySurface. Only the
     * dstDesc's config, flags, and origin need be set by the function. If the subclass cannot
     * create a surface that would succeed its implementation of onCopySurface, it should return
     * false. The base class will fall back to creating a render target to draw into using the src.
     */
    virtual bool onInitCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* dstDesc) = 0;

    // Check to see if this set of draw commands has been sent out
    virtual bool       isIssued(uint32_t drawID) { return true; }
    void getPathStencilSettingsForFilltype(GrPathRendering::FillType,
                                           const GrStencilAttachment*,
                                           GrStencilSettings*);
    virtual GrClipMaskManager* clipMaskManager() = 0;
    virtual bool setupClip(GrPipelineBuilder*,
                           GrPipelineBuilder::AutoRestoreFragmentProcessors*,
                           GrPipelineBuilder::AutoRestoreStencil*,
                           GrScissorState*,
                           const SkRect* devBounds) = 0;

    // The context owns us, not vice-versa, so this ptr is not ref'ed by DrawTarget.
    GrContext*                                                      fContext;
    // To keep track that we always have at least as many debug marker adds as removes
    int                                                             fGpuTraceMarkerCount;
    GrTraceMarkerSet                                                fActiveTraceMarkers;
    GrTraceMarkerSet                                                fStoredTraceMarkers;

    typedef SkRefCnt INHERITED;
};

/*
 * This class is JUST for clip mask manager.  Everyone else should just use draw target above.
 */
class GrClipTarget : public GrDrawTarget {
public:
    GrClipTarget(GrContext* context) : INHERITED(context) {
        fClipMaskManager.setClipTarget(this);
    }

    /* Clip mask manager needs access to the context.
     * TODO we only need a very small subset of context in the CMM.
     */
    GrContext* getContext() { return INHERITED::getContext(); }
    const GrContext* getContext() const { return INHERITED::getContext(); }

    /**
     * Clip Mask Manager(and no one else) needs to clear private stencil bits.
     * ClipTarget subclass sets clip bit in the stencil buffer. The subclass
     * is free to clear the remaining bits to zero if masked clears are more
     * expensive than clearing all bits.
     */
    virtual void clearStencilClip(const SkIRect& rect, bool insideClip, GrRenderTarget* = NULL) = 0;

    /**
     * Release any resources that are cached but not currently in use. This
     * is intended to give an application some recourse when resources are low.
     */
    void purgeResources() override {
        // The clip mask manager can rebuild all its clip masks so just
        // get rid of them all.
        fClipMaskManager.purgeResources();
    };

protected:
    GrClipMaskManager           fClipMaskManager;

private:
    GrClipMaskManager* clipMaskManager() override { return &fClipMaskManager; }

    virtual bool setupClip(GrPipelineBuilder*,
                           GrPipelineBuilder::AutoRestoreFragmentProcessors*,
                           GrPipelineBuilder::AutoRestoreStencil*,
                           GrScissorState* scissorState,
                           const SkRect* devBounds) override;

    typedef GrDrawTarget INHERITED;
};

#endif
