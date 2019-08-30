/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrPrimitiveProcessor.h"

#include "src/gpu/GrCoordTransform.h"

/**
 * We specialize the vertex code for each of these matrix types.
 */
enum MatrixType {
    kNoPersp_MatrixType  = 0,
    kGeneral_MatrixType  = 1,
};

GrPrimitiveProcessor::GrPrimitiveProcessor(ClassID classID) : GrProcessor(classID) {}

const GrPrimitiveProcessor::TextureSampler& GrPrimitiveProcessor::textureSampler(int i) const {
    SkASSERT(i >= 0 && i < this->numTextureSamplers());
    return this->onTextureSampler(i);
}

uint32_t
GrPrimitiveProcessor::getTransformKey(const SkTArray<GrCoordTransform*, true>& coords,
                                      int numCoords) const {
    uint32_t totalKey = 0;
    for (int t = 0; t < numCoords; ++t) {
        uint32_t key = 0;
        const GrCoordTransform* coordTransform = coords[t];
        if (coordTransform->getMatrix().hasPerspective()) {
            key |= kGeneral_MatrixType;
        } else {
            key |= kNoPersp_MatrixType;
        }
        key <<= t;
        SkASSERT(0 == (totalKey & key)); // keys for each transform ought not to overlap
        totalKey |= key;
    }
    return totalKey;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline GrSamplerState::Filter clamp_filter(GrTextureType type,
                                                  GrSamplerState::Filter requestedFilter) {
    if (GrTextureTypeHasRestrictedSampling(type)) {
        return SkTMin(requestedFilter, GrSamplerState::Filter::kBilerp);
    }
    return requestedFilter;
}

GrPrimitiveProcessor::TextureSampler::TextureSampler(GrTextureType textureType,
                                                     const GrSamplerState& samplerState,
                                                     const GrSwizzle& swizzle,
                                                     uint32_t extraSamplerKey) {
    this->reset(textureType, samplerState, swizzle, extraSamplerKey);
}

GrPrimitiveProcessor::TextureSampler::TextureSampler(GrTextureType textureType,
                                                     GrSamplerState::Filter filterMode,
                                                     GrSamplerState::WrapMode wrapXAndY,
                                                     const GrSwizzle& swizzle) {
    this->reset(textureType, filterMode, wrapXAndY, swizzle);
}

void GrPrimitiveProcessor::TextureSampler::reset(GrTextureType textureType,
                                                 const GrSamplerState& samplerState,
                                                 const GrSwizzle& swizzle,
                                                 uint32_t extraSamplerKey) {
    fSamplerState = samplerState;
    fSamplerState.setFilterMode(clamp_filter(textureType, samplerState.filter()));
    fSwizzle = swizzle;
    fTextureType = textureType;
    fExtraSamplerKey = extraSamplerKey;
    fIsInitialized = true;
}

void GrPrimitiveProcessor::TextureSampler::reset(GrTextureType textureType,
                                                 GrSamplerState::Filter filterMode,
                                                 GrSamplerState::WrapMode wrapXAndY,
                                                 const GrSwizzle& swizzle) {
    filterMode = clamp_filter(textureType, filterMode);
    fSamplerState = GrSamplerState(wrapXAndY, filterMode);
    fSwizzle = swizzle;
    fTextureType = textureType;
    fIsInitialized = true;
}
