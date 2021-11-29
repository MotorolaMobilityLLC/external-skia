/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlBuffer_DEFINED
#define skgpu_MtlBuffer_DEFINED

#include "experimental/graphite/src/Buffer.h"

#include "experimental/graphite/include/mtl/MtlTypes.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class Gpu;

class Buffer : public skgpu::Buffer {
public:
    sk_sp<Buffer> Make(const Gpu*, size_t size, BufferType type, PrioritizeGpuReads);

private:
    Buffer(size_t size, BufferType type, PrioritizeGpuReads, sk_cfp<id<MTLBuffer>>);

    void onMap() override;
    void onUnmap() override;

    sk_cfp<id<MTLBuffer>> fBuffer;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlBuffer_DEFINED

