/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlSampler.h"

#include "GrMtlGpu.h"

static inline MTLSamplerAddressMode wrap_mode_to_mtl_sampler_address(
        GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return MTLSamplerAddressModeClampToEdge;
        case GrSamplerState::WrapMode::kRepeat:
            return MTLSamplerAddressModeRepeat;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return MTLSamplerAddressModeMirrorRepeat;
    }
    SK_ABORT("Unknown wrap mode.");
    return MTLSamplerAddressModeClampToEdge;
}

GrMtlSampler* GrMtlSampler::Create(const GrMtlGpu* gpu, const GrSamplerState& samplerState,
                                   uint32_t maxMipLevel) {
    static MTLSamplerMinMagFilter mtlMinMagFilterModes[] = {
        MTLSamplerMinMagFilterNearest,
        MTLSamplerMinMagFilterLinear,
        MTLSamplerMinMagFilterLinear
    };

    GR_STATIC_ASSERT((int)GrSamplerState::Filter::kNearest == 0);
    GR_STATIC_ASSERT((int)GrSamplerState::Filter::kBilerp == 1);
    GR_STATIC_ASSERT((int)GrSamplerState::Filter::kMipMap == 2);

    auto samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDesc.sAddressMode = wrap_mode_to_mtl_sampler_address(samplerState.wrapModeX());
    samplerDesc.tAddressMode = wrap_mode_to_mtl_sampler_address(samplerState.wrapModeY());
    samplerDesc.magFilter = mtlMinMagFilterModes[static_cast<int>(samplerState.filter())];
    samplerDesc.minFilter = mtlMinMagFilterModes[static_cast<int>(samplerState.filter())];
    samplerDesc.mipFilter = MTLSamplerMipFilterLinear;
    samplerDesc.lodMinClamp = 0.0f;
    bool useMipMaps = GrSamplerState::Filter::kMipMap == samplerState.filter() && maxMipLevel > 0;
    samplerDesc.lodMaxClamp = !useMipMaps ? 0.0f : (float)(maxMipLevel);
    samplerDesc.maxAnisotropy = 1.0f;
    samplerDesc.normalizedCoordinates = true;
    samplerDesc.compareFunction = MTLCompareFunctionNever;
    samplerDesc.borderColor = MTLSamplerBorderColorTransparentBlack;

    return new GrMtlSampler([gpu->device() newSamplerStateWithDescriptor: samplerDesc]);
}
