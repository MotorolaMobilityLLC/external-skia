/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnGpuCommandBuffer_DEFINED
#define GrDawnGpuCommandBuffer_DEFINED

#include "src/gpu/GrGpuCommandBuffer.h"

#include "include/gpu/GrTypes.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrMesh.h"
#include "dawn/dawncpp.h"

class GrDawnGpu;
class GrDawnRenderTarget;

class GrDawnGpuTextureCommandBuffer : public GrGpuTextureCommandBuffer {
public:
    GrDawnGpuTextureCommandBuffer(GrDawnGpu* gpu, GrTexture* texture, GrSurfaceOrigin origin);
    ~GrDawnGpuTextureCommandBuffer() override;

    void copy(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) override;

    void transferFrom(const SkIRect& srcRect, GrColorType surfaceColorType,
                      GrColorType bufferColorType, GrGpuBuffer* transferBuffer,
                      size_t offset) override;
    void insertEventMarker(const char*) override {}
    void submit();

private:
    GrDawnGpu*                        fGpu;
    dawn::CommandEncoder              fEncoder;

    typedef GrGpuTextureCommandBuffer INHERITED;
};

class GrDawnGpuRTCommandBuffer : public GrGpuRTCommandBuffer, private GrMesh::SendToGpuImpl {
public:
    GrDawnGpuRTCommandBuffer(GrDawnGpu*, GrRenderTarget*, GrSurfaceOrigin,
                             const LoadAndStoreInfo&,
                             const StencilLoadAndStoreInfo&);

    ~GrDawnGpuRTCommandBuffer() override;

    void begin() override { }
    void end() override;

    dawn::RenderPassEncoder beginRenderPass(dawn::LoadOp colorOp, dawn::LoadOp stencilOp);
    void transferFrom(const SkIRect& srcRect, GrColorType surfaceColorType,
                      GrColorType bufferColorType, GrGpuBuffer* transferBuffer,
                      size_t offset) override;
    void insertEventMarker(const char*) override;

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override;

    void copy(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) override;

    void submit();

private:
    GrGpu* gpu() override;

    void setScissorState(const GrPipeline&,
                         const GrPipeline::FixedDynamicState* fixedDynamicState,
                         const GrPipeline::DynamicStateArrays* dynamicStateArrays);
    void applyState(const GrPipeline& pipeline,
                    const GrPrimitiveProcessor& primProc,
                    const GrTextureProxy* const primProcProxies[],
                    const GrPipeline::FixedDynamicState* fixedDynamicState,
                    const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                    const GrPrimitiveType primitiveType,
                    bool hasPoints);

    void onDraw(const GrPrimitiveProcessor& primProc,
                const GrPipeline& pipeline,
                const GrPipeline::FixedDynamicState* fixedDynamicState,
                const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                const GrMesh mesh[],
                int meshCount,
                const SkRect& bounds) override;

    void sendMeshToGpu(GrPrimitiveType primType, const GrBuffer* vertexBuffer, int vertexCount,
                       int baseVertex) final {
        this->sendInstancedMeshToGpu(primType, vertexBuffer, vertexCount, baseVertex,
                                     nullptr, 1, 0);
    }

    void sendIndexedMeshToGpu(GrPrimitiveType primType,
                              const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                              uint16_t /*minIndexValue*/, uint16_t /*maxIndexValue*/,
                              const GrBuffer* vertexBuffer, int baseVertex,
                              GrPrimitiveRestart restart) final {
        this->sendIndexedInstancedMeshToGpu(primType, indexBuffer, indexCount, baseIndex,
                                            vertexBuffer, baseVertex, nullptr, 1, 0, restart);
    }

    void sendInstancedMeshToGpu(GrPrimitiveType,
                                const GrBuffer* vertexBuffer, int vertexCount, int baseVertex,
                                const GrBuffer* instanceBuffer, int instanceCount,
                                int baseInstance) final;

    void sendIndexedInstancedMeshToGpu(GrPrimitiveType,
                                       const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                                       const GrBuffer* vertexBuffer, int baseVertex,
                                       const GrBuffer* instanceBuffer, int instanceCount,
                                       int baseInstance, GrPrimitiveRestart) final;

    void onClear(const GrFixedClip&, const SkPMColor4f& color) override;

    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override;

    struct InlineUploadInfo {
        InlineUploadInfo(GrOpFlushState* state, const GrDeferredTextureUploadFn& upload)
                : fFlushState(state), fUpload(upload) {}

        GrOpFlushState* fFlushState;
        GrDeferredTextureUploadFn fUpload;
    };

    GrDawnGpu*                  fGpu;
    dawn::CommandEncoder        fEncoder;
    dawn::RenderPassEncoder     fPassEncoder;
    LoadAndStoreInfo            fColorInfo;

    typedef GrGpuRTCommandBuffer INHERITED;
};

#endif
