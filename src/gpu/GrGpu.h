/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpu_DEFINED
#define GrGpu_DEFINED

#include "GrDrawTarget.h"
#include "GrPathRendering.h"
#include "GrProgramDesc.h"
#include "SkPath.h"

class GrContext;
class GrIndexBufferAllocPool;
class GrOptDrawState;
class GrPath;
class GrPathRange;
class GrPathRenderer;
class GrPathRendererChain;
class GrStencilBuffer;
class GrVertexBufferAllocPool;

class GrGpu : public SkRefCnt {
public:

    /**
     * Additional blend coefficients for dual source blending, not exposed
     * through GrPaint/GrContext.
     */
    enum ExtendedBlendCoeffs {
        // source 2 refers to second output color when
        // using dual source blending.
        kS2C_GrBlendCoeff = kPublicGrBlendCoeffCount,
        kIS2C_GrBlendCoeff,
        kS2A_GrBlendCoeff,
        kIS2A_GrBlendCoeff,

        kTotalGrBlendCoeffCount
    };

    /**
     * Create an instance of GrGpu that matches the specified backend. If the requested backend is
     * not supported (at compile-time or run-time) this returns NULL. The context will not be
     * fully constructed and should not be used by GrGpu until after this function returns.
     */
    static GrGpu* Create(GrBackend, GrBackendContext, GrContext* context);

    ////////////////////////////////////////////////////////////////////////////

    GrGpu(GrContext* context);
    virtual ~GrGpu();

    GrContext* getContext() { return fContext; }
    const GrContext* getContext() const { return fContext; }

    /**
     * Gets the capabilities of the draw target.
     */
    const GrDrawTargetCaps* caps() const { return fCaps.get(); }

    GrPathRendering* pathRendering() {
        return fPathRendering.get();
    }

    // Called by GrContext when the underlying backend context has been destroyed.
    // GrGpu should use this to ensure that no backend API calls will be made from
    // here onward, including in its destructor. Subclasses should call
    // INHERITED::contextAbandoned() if they override this.
    virtual void contextAbandoned();

    /**
     * The GrGpu object normally assumes that no outsider is setting state
     * within the underlying 3D API's context/device/whatever. This call informs
     * the GrGpu that the state was modified and it shouldn't make assumptions
     * about the state.
     */
    void markContextDirty(uint32_t state = kAll_GrBackendState) {
        fResetBits |= state;
    }

    void unimpl(const char[]);

    /**
     * Creates a texture object. If kRenderTarget_GrSurfaceFlag the texture can
     * be used as a render target by calling GrTexture::asRenderTarget(). Not all
     * pixel configs can be used as render targets. Support for configs as textures
     * or render targets can be checked using GrDrawTargetCaps.
     *
     * @param desc        describes the texture to be created.
     * @param srcData     texel data to load texture. Begins with full-size
     *                    palette data for paletted textures. For compressed
     *                    formats it contains the compressed pixel data. Otherwise,
     *                    it contains width*height texels. If NULL texture data
     *                    is uninitialized.
     * @param rowBytes    the number of bytes between consecutive rows. Zero
     *                    means rows are tightly packed. This field is ignored
     *                    for compressed formats.
     *
     * @return    The texture object if successful, otherwise NULL.
     */
    GrTexture* createTexture(const GrSurfaceDesc& desc, const void* srcData, size_t rowBytes);

    /**
     * Implements GrContext::wrapBackendTexture
     */
    GrTexture* wrapBackendTexture(const GrBackendTextureDesc&);

    /**
     * Implements GrContext::wrapBackendTexture
     */
    GrRenderTarget* wrapBackendRenderTarget(const GrBackendRenderTargetDesc&);

    /**
     * Creates a vertex buffer.
     *
     * @param size    size in bytes of the vertex buffer
     * @param dynamic hints whether the data will be frequently changed
     *                by either GrVertexBuffer::map() or
     *                GrVertexBuffer::updateData().
     *
     * @return    The vertex buffer if successful, otherwise NULL.
     */
    GrVertexBuffer* createVertexBuffer(size_t size, bool dynamic);

    /**
     * Creates an index buffer.
     *
     * @param size    size in bytes of the index buffer
     * @param dynamic hints whether the data will be frequently changed
     *                by either GrIndexBuffer::map() or
     *                GrIndexBuffer::updateData().
     *
     * @return The index buffer if successful, otherwise NULL.
     */
    GrIndexBuffer* createIndexBuffer(size_t size, bool dynamic);

    /**
     * Creates an index buffer for instance drawing with a specific pattern.
     *
     * @param pattern     the pattern to repeat
     * @param patternSize size in bytes of the pattern
     * @param reps        number of times to repeat the pattern
     * @param vertCount   number of vertices the pattern references
     * @param dynamic     hints whether the data will be frequently changed
     *                    by either GrIndexBuffer::map() or
     *                    GrIndexBuffer::updateData().
     *
     * @return The index buffer if successful, otherwise NULL.
     */
    GrIndexBuffer* createInstancedIndexBuffer(const uint16_t* pattern,
                                              int patternSize,
                                              int reps,
                                              int vertCount,
                                              bool isDynamic = false);

    /**
     * Returns an index buffer that can be used to render quads.
     * Six indices per quad: 0, 1, 2, 0, 2, 3, etc.
     * The max number of quads can be queried using GrIndexBuffer::maxQuads().
     * Draw with kTriangles_GrPrimitiveType
     * @ return the quad index buffer
     */
    const GrIndexBuffer* getQuadIndexBuffer() const;

    /**
     * Resolves MSAA.
     */
    void resolveRenderTarget(GrRenderTarget* target);

    /**
     * Gets a preferred 8888 config to use for writing/reading pixel data to/from a surface with
     * config surfaceConfig. The returned config must have at least as many bits per channel as the
     * readConfig or writeConfig param.
     */
    virtual GrPixelConfig preferredReadPixelsConfig(GrPixelConfig readConfig,
                                                    GrPixelConfig surfaceConfig) const {
        return readConfig;
    }
    virtual GrPixelConfig preferredWritePixelsConfig(GrPixelConfig writeConfig,
                                                     GrPixelConfig surfaceConfig) const {
        return writeConfig;
    }

    /**
     * Called before uploading writing pixels to a GrTexture when the src pixel config doesn't
     * match the texture's config.
     */
    virtual bool canWriteTexturePixels(const GrTexture*, GrPixelConfig srcConfig) const = 0;

    /**
     * OpenGL's readPixels returns the result bottom-to-top while the skia
     * API is top-to-bottom. Thus we have to do a y-axis flip. The obvious
     * solution is to have the subclass do the flip using either the CPU or GPU.
     * However, the caller (GrContext) may have transformations to apply and can
     * simply fold in the y-flip for free. On the other hand, the subclass may
     * be able to do it for free itself. For example, the subclass may have to
     * do memcpys to handle rowBytes that aren't tight. It could do the y-flip
     * concurrently.
     *
     * This function returns true if a y-flip is required to put the pixels in
     * top-to-bottom order and the subclass cannot do it for free.
     *
     * See read pixels for the params
     * @return true if calling readPixels with the same set of params will
     *              produce bottom-to-top data
     */
     virtual bool readPixelsWillPayForYFlip(GrRenderTarget* renderTarget,
                                            int left, int top,
                                            int width, int height,
                                            GrPixelConfig config,
                                            size_t rowBytes) const = 0;
     /**
      * This should return true if reading a NxM rectangle of pixels from a
      * render target is faster if the target has dimensons N and M and the read
      * rectangle has its top-left at 0,0.
      */
     virtual bool fullReadPixelsIsFasterThanPartial() const { return false; };

    /**
     * Reads a rectangle of pixels from a render target.
     *
     * @param renderTarget  the render target to read from. NULL means the
     *                      current render target.
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param config        the pixel config of the destination buffer
     * @param buffer        memory to read the rectangle into.
     * @param rowBytes      the number of bytes between consecutive rows. Zero
     *                      means rows are tightly packed.
     * @param invertY       buffer should be populated bottom-to-top as opposed
     *                      to top-to-bottom (skia's usual order)
     *
     * @return true if the read succeeded, false if not. The read can fail
     *              because of a unsupported pixel config or because no render
     *              target is currently set.
     */
    bool readPixels(GrRenderTarget* renderTarget,
                    int left, int top, int width, int height,
                    GrPixelConfig config, void* buffer, size_t rowBytes);

    /**
     * Updates the pixels in a rectangle of a texture.
     *
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read pixels from
     * @param rowBytes      number of bytes between consecutive rows. Zero
     *                      means rows are tightly packed.
     */
    bool writeTexturePixels(GrTexture* texture,
                            int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t rowBytes);

    /**
     * Clear the passed in render target. Ignores the draw state and clip. Clears the whole thing if
     * rect is NULL, otherwise just the rect. If canIgnoreRect is set then the entire render target
     * can be optionally cleared.
     */
    void clear(const SkIRect* rect, GrColor color, bool canIgnoreRect,GrRenderTarget* renderTarget);


    void clearStencilClip(const SkIRect& rect, bool insideClip, GrRenderTarget* renderTarget);

    /**
     * Discards the contents render target. NULL indicates that the current render target should
     * be discarded.
     **/
    virtual void discard(GrRenderTarget* = NULL) = 0;

    /**
     * This is can be called before allocating a texture to be a dst for copySurface. It will
     * populate the origin, config, and flags fields of the desc such that copySurface is more
     * likely to succeed and be efficient.
     */
    virtual void initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc);

    // After the client interacts directly with the 3D context state the GrGpu
    // must resync its internal state and assumptions about 3D context state.
    // Each time this occurs the GrGpu bumps a timestamp.
    // state of the 3D context
    // At 10 resets / frame and 60fps a 64bit timestamp will overflow in about
    // a billion years.
    typedef uint64_t ResetTimestamp;

    // This timestamp is always older than the current timestamp
    static const ResetTimestamp kExpiredTimestamp = 0;
    // Returns a timestamp based on the number of times the context was reset.
    // This timestamp can be used to lazily detect when cached 3D context state
    // is dirty.
    ResetTimestamp getResetTimestamp() const { return fResetTimestamp; }

    enum DrawType {
        kDrawPoints_DrawType,
        kDrawLines_DrawType,
        kDrawTriangles_DrawType,
        kStencilPath_DrawType,
        kDrawPath_DrawType,
        kDrawPaths_DrawType,
    };

    static bool IsPathRenderingDrawType(DrawType type) {
        return kDrawPath_DrawType == type || kDrawPaths_DrawType == type;
    }

    GrContext::GPUStats* gpuStats() { return &fGPUStats; }

    virtual void buildProgramDesc(const GrOptDrawState&,
                                  const GrProgramDesc::DescInfo&,
                                  GrGpu::DrawType,
                                  GrProgramDesc*) = 0;

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

    // Called to determine whether an onCopySurface call would succeed or not. This is useful for
    // proxy subclasses to test whether the copy would succeed without executing it yet. Derived
    // classes must keep this consistent with their implementation of onCopySurface(). The inputs
    // are the same as onCopySurface(), i.e. srcRect and dstPoint are clipped to be inside the src
    // and dst bounds.
    virtual bool canCopySurface(const GrSurface* dst,
                                const GrSurface* src,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint) = 0;

    // This method is called by copySurface  The srcRect is guaranteed to be entirely within the
    // src bounds. Likewise, the dst rect implied by dstPoint and srcRect's width and height falls
    // entirely within the dst. The default implementation will draw a rect from the src to the
    // dst if the src is a texture and the dst is a render target and fail otherwise.
    virtual bool copySurface(GrSurface* dst,
                             GrSurface* src,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint) = 0;

    virtual void draw(const GrOptDrawState&, const GrDrawTarget::DrawInfo&);
    virtual void stencilPath(const GrOptDrawState&,
                             const GrPath*,
                             const GrStencilSettings&);
    virtual void drawPath(const GrOptDrawState&,
                          const GrPath*,
                          const GrStencilSettings&);
    virtual void drawPaths(const GrOptDrawState&,
                           const GrPathRange*,
                           const void* indices,
                           GrDrawTarget::PathIndexType,
                           const float transformValues[],
                           GrDrawTarget::PathTransformType,
                           int count,
                           const GrStencilSettings&);

    static DrawType PrimTypeToDrawType(GrPrimitiveType type) {
        switch (type) {
            case kTriangles_GrPrimitiveType:
            case kTriangleStrip_GrPrimitiveType:
            case kTriangleFan_GrPrimitiveType:
                return kDrawTriangles_DrawType;
            case kPoints_GrPrimitiveType:
                return kDrawPoints_DrawType;
            case kLines_GrPrimitiveType:
            case kLineStrip_GrPrimitiveType:
                return kDrawLines_DrawType;
            default:
                SkFAIL("Unexpected primitive type");
                return kDrawTriangles_DrawType;
        }
    }

protected:
    // Functions used to map clip-respecting stencil tests into normal
    // stencil funcs supported by GPUs.
    static GrStencilFunc ConvertStencilFunc(bool stencilInClip,
                                            GrStencilFunc func);
    static void ConvertStencilFuncAndMask(GrStencilFunc func,
                                          bool clipInStencil,
                                          unsigned int clipBit,
                                          unsigned int userBits,
                                          unsigned int* ref,
                                          unsigned int* mask);

    const GrTraceMarkerSet& getActiveTraceMarkers() { return fActiveTraceMarkers; }

    GrContext::GPUStats         fGPUStats;

    SkAutoTDelete<GrPathRendering> fPathRendering;

    // Subclass must initialize this in its constructor.
    SkAutoTUnref<const GrDrawTargetCaps> fCaps;

private:
    // called when the 3D context state is unknown. Subclass should emit any
    // assumed 3D context state and dirty any state cache.
    virtual void onResetContext(uint32_t resetBits) = 0;

    // overridden by backend-specific derived class to create objects.
    virtual GrTexture* onCreateTexture(const GrSurfaceDesc& desc,
                                       const void* srcData,
                                       size_t rowBytes) = 0;
    virtual GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc,
                                                 const void* srcData) = 0;
    virtual GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&) = 0;
    virtual GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&) = 0;
    virtual GrVertexBuffer* onCreateVertexBuffer(size_t size, bool dynamic) = 0;
    virtual GrIndexBuffer* onCreateIndexBuffer(size_t size, bool dynamic) = 0;

    // overridden by backend-specific derived class to perform the clear.
    virtual void onClear(GrRenderTarget*, const SkIRect* rect, GrColor color,
                         bool canIgnoreRect) = 0;


    // Overridden by backend specific classes to perform a clear of the stencil clip bits.  This is
    // ONLY used by the the clip target
    virtual void onClearStencilClip(GrRenderTarget*,
                                    const SkIRect& rect,
                                    bool insideClip) = 0;

    // overridden by backend-specific derived class to perform the draw call.
    virtual void onDraw(const GrOptDrawState&, const GrDrawTarget::DrawInfo&) = 0;

    // overridden by backend-specific derived class to perform the read pixels.
    virtual bool onReadPixels(GrRenderTarget* target,
                              int left, int top, int width, int height,
                              GrPixelConfig,
                              void* buffer,
                              size_t rowBytes) = 0;

    // overridden by backend-specific derived class to perform the texture update
    virtual bool onWriteTexturePixels(GrTexture* texture,
                                      int left, int top, int width, int height,
                                      GrPixelConfig config, const void* buffer,
                                      size_t rowBytes) = 0;

    // overridden by backend-specific derived class to perform the resolve
    virtual void onResolveRenderTarget(GrRenderTarget* target) = 0;

    // width and height may be larger than rt (if underlying API allows it).
    // Should attach the SB to the RT. Returns false if compatible sb could
    // not be created.
    virtual bool createStencilBufferForRenderTarget(GrRenderTarget*, int width, int height) = 0;

    // attaches an existing SB to an existing RT.
    virtual bool attachStencilBufferToRenderTarget(GrStencilBuffer*, GrRenderTarget*) = 0;

    // The GrGpu typically records the clients requested state and then flushes
    // deltas from previous state at draw time. This function does the
    // backend-specific flush of the state.
    // returns false if current state is unsupported.
    virtual bool flushGraphicsState(const GrOptDrawState&, DrawType) = 0;

    // clears target's entire stencil buffer to 0
    virtual void clearStencil(GrRenderTarget* target) = 0;

    // Given a rt, find or create a stencil buffer and attach it
    bool attachStencilBufferToRenderTarget(GrRenderTarget* target);

    virtual void didAddGpuTraceMarker() = 0;
    virtual void didRemoveGpuTraceMarker() = 0;

    void resetContext() {
        this->onResetContext(fResetBits);
        fResetBits = 0;
        ++fResetTimestamp;
    }

    void handleDirtyContext() {
        if (fResetBits) {
            this->resetContext();
        }
    }

    ResetTimestamp                                                      fResetTimestamp;
    uint32_t                                                            fResetBits;
    // these are mutable so they can be created on-demand
    mutable GrIndexBuffer*                                              fQuadIndexBuffer;
    // To keep track that we always have at least as many debug marker adds as removes
    int                                                                 fGpuTraceMarkerCount;
    GrTraceMarkerSet                                                    fActiveTraceMarkers;
    GrTraceMarkerSet                                                    fStoredTraceMarkers;
    // The context owns us, not vice-versa, so this ptr is not ref'ed by Gpu.
    GrContext*                                                          fContext;

    typedef SkRefCnt INHERITED;
};

#endif
