/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessor_DEFINED
#define GrProcessor_DEFINED

#include "include/core/SkMath.h"
#include "include/core/SkString.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrProcessorUnitTest.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxy.h"

class GrResourceProvider;

/**
 * Used by processors to build their keys. It incorporates each per-processor key into a larger
 * shader key.
 */
class GrProcessorKeyBuilder {
public:
    GrProcessorKeyBuilder(GrKeyBuilder* key) : fKey(key) {
        SkASSERT(0 == fKey->size() % sizeof(uint32_t));
    }

    // Introduces a word-boundary in the underlying key. Must be called before using the key with
    // any cache, but can be called elsewhere to create a clean break between generic data and
    // backend-specific data.
    void flush() { fKey->flush(); }

    void addBits(uint32_t numBits, uint32_t val, const char* label) {
        fKey->addBits(numBits, val, label);
    }

    void addBytes(uint32_t numBytes, const void* data, const char* label) {
        fKey->addBytes(numBytes, data, label);
    }

    void add32(uint32_t v, const char* label = "unknown") {
        this->addBits(32, v, label);
    }

    template <typename StringFunc>
    void addString(StringFunc&& sf) {
        fKey->addString(std::move(sf));
    }

    size_t sizeInBits() const { return fKey->sizeInBits(); }

private:
    GrKeyBuilder* fKey;    // unowned ptr to the larger key.
};

/** Provides custom shader code to the Ganesh shading pipeline. GrProcessor objects *must* be
    immutable: after being constructed, their fields may not change.

    Dynamically allocated GrProcessors are managed by a per-thread memory pool. The ref count of an
    processor must reach 0 before the thread terminates and the pool is destroyed.
 */
class GrProcessor {
public:
    enum ClassID {
        kNull_ClassID,  // Reserved ID for missing (null) processors

        kBigKeyProcessor_ClassID,
        kBlendFragmentProcessor_ClassID,
        kBlockInputFragmentProcessor_ClassID,
        kButtCapStrokedCircleGeometryProcessor_ClassID,
        kCircleGeometryProcessor_ClassID,
        kCircularRRectEffect_ClassID,
        kClockwiseTestProcessor_ClassID,
        kColorTableEffect_ClassID,
        kCoverageSetOpXP_ClassID,
        kCubicStrokeProcessor_ClassID,
        kCustomXP_ClassID,
        kDashingCircleEffect_ClassID,
        kDashingLineEffect_ClassID,
        kDefaultGeoProc_ClassID,
        kDIEllipseGeometryProcessor_ClassID,
        kDisableColorXP_ClassID,
        kDrawAtlasPathShader_ClassID,
        kEllipseGeometryProcessor_ClassID,
        kEllipticalRRectEffect_ClassID,
        kGP_ClassID,
        kVertexColorSpaceBenchGP_ClassID,
        kGrArithmeticProcessor_ClassID,
        kGrAARectEffect_ClassID,
        kGrAlphaThresholdFragmentProcessor_ClassID,
        kGrBicubicEffect_ClassID,
        kGrBitmapTextGeoProc_ClassID,
        kGrBlurredEdgeFragmentProcessor_ClassID,
        kGrCCClipProcessor_ClassID,
        kGrCCPathProcessor_ClassID,
        kGrCircleBlurFragmentProcessor_ClassID,
        kGrCircleEffect_ClassID,
        kGrClampedGradientEffect_ClassID,
        kGrClampFragmentProcessor_ClassID,
        kGrColorMatrixFragmentProcessor_ClassID,
        kGrColorSpaceXformEffect_ClassID,
        kGrComposeLerpEffect_ClassID,
        kGrConfigConversionEffect_ClassID,
        kGrConicEffect_ClassID,
        kGrConstColorProcessor_ClassID,
        kGrConvexPolyEffect_ClassID,
        kGrDeviceSpaceEffect_ClassID,
        kGrDiffuseLightingEffect_ClassID,
        kGrDisplacementMapEffect_ClassID,
        kGrDistanceFieldA8TextGeoProc_ClassID,
        kGrDistanceFieldLCDTextGeoProc_ClassID,
        kGrDistanceFieldPathGeoProc_ClassID,
        kGrDitherEffect_ClassID,
        kGrDualIntervalGradientColorizer_ClassID,
        kGrEllipseEffect_ClassID,
        kGrFillRRectOp_Processor_ClassID,
        kGrGaussianConvolutionFragmentProcessor_ClassID,
        kGrGSCoverageProcessor_ClassID,
        kGrHighContrastFilterEffect_ClassID,
        kGrHSLToRGBFilterEffect_ClassID,
        kGrImprovedPerlinNoiseEffect_ClassID,
        kGrLinearGradientLayout_ClassID,
        kGrLumaColorFilterEffect_ClassID,
        kGrMagnifierEffect_ClassID,
        kGrMatrixConvolutionEffect_ClassID,
        kGrMatrixEffect_ClassID,
        kGrMeshTestProcessor_ClassID,
        kGrMorphologyEffect_ClassID,
        kGrOverrideInputFragmentProcessor_ClassID,
        kGrPathProcessor_ClassID,
        kGrPerlinNoise2Effect_ClassID,
        kGrPipelineDynamicStateTestProcessor_ClassID,
        kGrQuadEffect_ClassID,
        kGrRadialGradientLayout_ClassID,
        kGrRectBlurEffect_ClassID,
        kGrRGBToHSLFilterEffect_ClassID,
        kGrRRectBlurEffect_ClassID,
        kGrRRectShadowGeoProc_ClassID,
        kGrSingleIntervalGradientColorizer_ClassID,
        kGrSkSLFP_ClassID,
        kGrSpecularLightingEffect_ClassID,
        kGrSampleMaskProcessor_ClassID,
        kGrSweepGradientLayout_ClassID,
        kGrTextureEffect_ClassID,
        kGrTiledGradientEffect_ClassID,
        kGrTwoPointConicalGradientLayout_ClassID,
        kGrUnrolledBinaryGradientColorizer_ClassID,
        kGrVSCoverageProcessor_ClassID,
        kGrYUVtoRGBEffect_ClassID,
        kHighContrastFilterEffect_ClassID,
        kLatticeGP_ClassID,
        kPDLCDXferProcessor_ClassID,
        kPorterDuffXferProcessor_ClassID,
        kPremulFragmentProcessor_ClassID,
        kQuadEdgeEffect_ClassID,
        kQuadPerEdgeAAGeometryProcessor_ClassID,
        kSampleLocationsTestProcessor_ClassID,
        kSeriesFragmentProcessor_ClassID,
        kShaderPDXferProcessor_ClassID,
        kStencilResolveProcessor_ClassID,
        kFwidthSquircleTestProcessor_ClassID,
        kSwizzleFragmentProcessor_ClassID,
        kTessellate_GrCubicTessellateShader_ClassID,
        kTessellate_GrFillBoundingBoxShader_ClassID,
        kTessellate_GrFillCubicHullShader_ClassID,
        kTessellate_GrFillTriangleShader_ClassID,
        kTessellate_GrMiddleOutCubicShader_ClassID,
        kTessellate_GrStencilTriangleShader_ClassID,
        kTessellate_GrStrokeTessellateShader_ClassID,
        kTessellate_GrWedgeTessellateShader_ClassID,
        kTessellationTestTriShader_ClassID,
        kTessellationTestRectShader_ClassID,
        kTestFP_ClassID,
        kTestRectOp_ClassID,
        kFlatNormalsFP_ClassID,
        kMappedNormalsFP_ClassID,
        kLightingFP_ClassID,
        kLinearStrokeProcessor_ClassID,
        kVerticesGP_ClassID,
    };

    virtual ~GrProcessor() = default;

    /** Human-meaningful string to identify this processor; may be embedded in generated shader
        code and must be a legal SkSL identifier prefix. */
    virtual const char* name() const = 0;

    /** Human-readable dump of all information */
#if GR_TEST_UTILS
    virtual SkString onDumpInfo() const { return SkString(); }

    virtual SkString dumpInfo() const final {
        SkString info(name());
        info.append(this->onDumpInfo());
        return info;
    }
#endif

    /**
     * Custom shader features provided by the framework. These require special handling when
     * preparing shaders, so a processor must call setWillUseCustomFeature() from its constructor if
     * it intends to use one.
     */
    enum class CustomFeatures {
        kNone = 0,
        kSampleLocations = 1 << 0,
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(CustomFeatures);

    CustomFeatures requestedFeatures() const { return fRequestedFeatures; }

    void* operator new(size_t size);
    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }
    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }

    /** Helper for down-casting to a GrProcessor subclass */
    template <typename T> const T& cast() const { return *static_cast<const T*>(this); }

    ClassID classID() const { return fClassID; }

protected:
    GrProcessor(ClassID classID) : fClassID(classID) {}
    GrProcessor(const GrProcessor&) = delete;
    GrProcessor& operator=(const GrProcessor&) = delete;

    void setWillUseCustomFeature(CustomFeatures feature) { fRequestedFeatures |= feature; }
    void resetCustomFeatures() { fRequestedFeatures = CustomFeatures::kNone; }

    const ClassID fClassID;
    CustomFeatures fRequestedFeatures = CustomFeatures::kNone;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrProcessor::CustomFeatures)

#endif
