/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockGpuCommandBuffer_DEFINED
#define GrMockGpuCommandBuffer_DEFINED

#include "GrGpuCommandBuffer.h"
#include "GrMockGpu.h"

class GrMockGpuCommandBuffer : public GrGpuCommandBuffer {
public:
    GrMockGpuCommandBuffer(GrMockGpu* gpu) : fGpu(gpu) {}

    GrGpu* gpu() override { return fGpu; }
    void inlineUpload(GrOpFlushState*, GrDrawOp::DeferredUploadFn&,
                      GrRenderTargetProxy*) override {}
    void discard(GrRenderTargetProxy*) override {}
    void insertEventMarker(GrRenderTargetProxy*, const char*) override {}
    void end() override {}

    int numDraws() const { return fNumDraws; }

private:
    void onSubmit() override { fGpu->submitCommandBuffer(this); }
    void onDraw(const GrPipeline&, const GrPrimitiveProcessor&, const GrMesh[],
                const GrPipeline::DynamicState[], int meshCount, const SkRect& bounds) override {
        ++fNumDraws;
    }
    void onClear(GrRenderTargetProxy*, const GrFixedClip&, GrColor) override {}
    void onClearStencilClip(GrRenderTargetProxy*, const GrFixedClip&,
                            bool insideStencilMask) override {}
    GrRenderTarget* renderTarget() override { return nullptr; }

    GrMockGpu* fGpu;
    int fNumDraws = 0;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif
