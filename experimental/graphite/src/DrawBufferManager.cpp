/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawBufferManager.h"

#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/ResourceProvider.h"

namespace skgpu {

// TODO: Tune these values on real world data
static constexpr size_t kVertexBufferSize = 2 << 10;
static constexpr size_t kIndexBufferSize = 2 << 10;
static constexpr size_t kUniformBufferSize = 2 << 10;

DrawBufferManager::DrawBufferManager(ResourceProvider* resourceProvider,
                                     size_t uniformStartAlignment)
        : fResourceProvider(resourceProvider)
        , fUniformStartAlignment(uniformStartAlignment) {}

DrawBufferManager::~DrawBufferManager() {}

static bool can_fit(size_t requestedSize,
                    Buffer* buffer,
                    size_t currentOffset,
                    size_t alignment) {
    size_t startOffset = SkAlignTo(currentOffset, alignment);
    return requestedSize <= (buffer->size() - startOffset);
}

std::tuple<VertexWriter, BindBufferInfo> DrawBufferManager::getVertexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        BindBufferInfo bindInfo;
        bindInfo.fBuffer = nullptr;
        bindInfo.fOffset = 0;
        return {VertexWriter(nullptr), bindInfo};
    }
    if (fCurrentVertexBuffer &&
        !can_fit(requiredBytes, fCurrentVertexBuffer.get(), fVertexOffset, /*alignment=*/1)) {
        fUsedBuffers.push_back(std::move(fCurrentVertexBuffer));
    }

    if (!fCurrentVertexBuffer) {
        SkASSERT(requiredBytes <= kVertexBufferSize);
        fCurrentVertexBuffer = fResourceProvider->findOrCreateBuffer(kVertexBufferSize,
                                                                     BufferType::kVertex,
                                                                     PrioritizeGpuReads::kNo);
        fVertexOffset = 0;
        if (!fCurrentVertexBuffer) {
            return {VertexWriter(), BindBufferInfo()};
        }
    }
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = fCurrentVertexBuffer.get();
    bindInfo.fOffset = fVertexOffset;
    fVertexOffset += requiredBytes;
    return {VertexWriter(fCurrentVertexBuffer->map()), bindInfo};
}

std::tuple<IndexWriter, BindBufferInfo> DrawBufferManager::getIndexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        BindBufferInfo bindInfo;
        bindInfo.fBuffer = nullptr;
        bindInfo.fOffset = 0;
        return {IndexWriter(nullptr), bindInfo};
    }
    if (fCurrentIndexBuffer &&
        !can_fit(requiredBytes, fCurrentIndexBuffer.get(), fIndexOffset, /*alignment=*/1)) {
        fUsedBuffers.push_back(std::move(fCurrentIndexBuffer));
    }

    if (!fCurrentIndexBuffer) {
        SkASSERT(requiredBytes <= kIndexBufferSize);
        fCurrentIndexBuffer = fResourceProvider->findOrCreateBuffer(kIndexBufferSize,
                                                                    BufferType::kIndex,
                                                                    PrioritizeGpuReads::kNo);
        fIndexOffset = 0;
        if (!fCurrentIndexBuffer) {
            return {IndexWriter(), BindBufferInfo()};
        }
    }
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = fCurrentIndexBuffer.get();
    bindInfo.fOffset = fIndexOffset;
    fIndexOffset += requiredBytes;
    return {IndexWriter(fCurrentIndexBuffer->map()), bindInfo};
}

std::tuple<UniformWriter, BindBufferInfo> DrawBufferManager::getUniformWriter(
        size_t requiredBytes) {
    if (!requiredBytes) {
        BindBufferInfo bindInfo;
        bindInfo.fBuffer = nullptr;
        bindInfo.fOffset = 0;
        return {UniformWriter(nullptr), bindInfo};
    }
    if (fCurrentUniformBuffer &&
        !can_fit(requiredBytes,
                 fCurrentUniformBuffer.get(),
                 fUniformOffset,
                 fUniformStartAlignment)) {
        fUsedBuffers.push_back(std::move(fCurrentUniformBuffer));
    }

    if (!fCurrentUniformBuffer) {
        SkASSERT(requiredBytes <= kUniformBufferSize);
        fCurrentUniformBuffer = fResourceProvider->findOrCreateBuffer(kUniformBufferSize,
                                                                      BufferType::kUniform,
                                                                      PrioritizeGpuReads::kNo);
        fUniformOffset = 0;
        if (!fCurrentUniformBuffer) {
            return {UniformWriter(), BindBufferInfo()};
        }
    }
    fUniformOffset = SkAlignTo(fUniformOffset, fUniformStartAlignment);
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = fCurrentUniformBuffer.get();
    bindInfo.fOffset = fUniformOffset;
    fUniformOffset += requiredBytes;
    return {UniformWriter(fCurrentUniformBuffer->map()), bindInfo};
}

void DrawBufferManager::transferToCommandBuffer(CommandBuffer* commandBuffer) {
    for (auto& buffer : fUsedBuffers) {
        buffer->unmap();
        commandBuffer->trackResource(std::move(buffer));
    }
    fUsedBuffers.clear();
}

} // namespace skgpu

