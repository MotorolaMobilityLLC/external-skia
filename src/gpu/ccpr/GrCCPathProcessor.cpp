/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPathProcessor.h"

#include "GrGpuCommandBuffer.h"
#include "GrOnFlushResourceProvider.h"
#include "GrTexture.h"
#include "ccpr/GrCCPerFlushResources.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLVarying.h"

// Paths are drawn as octagons. Each point on the octagon is the intersection of two lines: one edge
// from the path's bounding box and one edge from its 45-degree bounding box. The selectors
// below indicate one corner from the bounding box, paired with a corner from the 45-degree bounding
// box. The octagon vertex is the point that lies between these two corners, found by intersecting
// their edges.
static constexpr float kOctoEdgeNorms[8*4] = {
    // bbox   // bbox45
    0,0,      0,0,
    0,0,      1,0,
    1,0,      1,0,
    1,0,      1,1,
    1,1,      1,1,
    1,1,      0,1,
    0,1,      0,1,
    0,1,      0,0,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gVertexBufferKey);

sk_sp<const GrGpuBuffer> GrCCPathProcessor::FindVertexBuffer(GrOnFlushResourceProvider* onFlushRP) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gVertexBufferKey);
    return onFlushRP->findOrMakeStaticBuffer(GrGpuBufferType::kVertex, sizeof(kOctoEdgeNorms),
                                             kOctoEdgeNorms, gVertexBufferKey);
}

static constexpr uint16_t kRestartStrip = 0xffff;

static constexpr uint16_t kOctoIndicesAsStrips[] = {
    3, 4, 2, 0, 1, kRestartStrip,  // First half.
    7, 0, 6, 4, 5  // Second half.
};

static constexpr uint16_t kOctoIndicesAsTris[] = {
    // First half.
    3, 4, 2,
    4, 0, 2,
    2, 0, 1,

    // Second half.
    7, 0, 6,
    0, 4, 6,
    6, 4, 5,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gIndexBufferKey);

constexpr GrPrimitiveProcessor::Attribute GrCCPathProcessor::kInstanceAttribs[];
constexpr GrPrimitiveProcessor::Attribute GrCCPathProcessor::kCornersAttrib;

sk_sp<const GrGpuBuffer> GrCCPathProcessor::FindIndexBuffer(GrOnFlushResourceProvider* onFlushRP) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);
    if (onFlushRP->caps()->usePrimitiveRestart()) {
        return onFlushRP->findOrMakeStaticBuffer(GrGpuBufferType::kIndex,
                                                 sizeof(kOctoIndicesAsStrips), kOctoIndicesAsStrips,
                                                 gIndexBufferKey);
    } else {
        return onFlushRP->findOrMakeStaticBuffer(GrGpuBufferType::kIndex,
                                                 sizeof(kOctoIndicesAsTris), kOctoIndicesAsTris,
                                                 gIndexBufferKey);
    }
}

GrCCPathProcessor::GrCCPathProcessor(const GrTextureProxy* atlas,
                                     const SkMatrix& viewMatrixIfUsingLocalCoords)
        : INHERITED(kGrCCPathProcessor_ClassID)
        , fAtlasAccess(atlas->textureType(), atlas->config(), GrSamplerState::Filter::kNearest,
                       GrSamplerState::WrapMode::kClamp)
        , fAtlasSize(atlas->isize())
        , fAtlasOrigin(atlas->origin()) {
    // TODO: Can we just assert that atlas has GrCCAtlas::kTextureOrigin and remove fAtlasOrigin?
    this->setInstanceAttributes(kInstanceAttribs, SK_ARRAY_COUNT(kInstanceAttribs));
    SkASSERT(this->instanceStride() == sizeof(Instance));

    this->setVertexAttributes(&kCornersAttrib, 1);
    this->setTextureSamplerCnt(1);

    if (!viewMatrixIfUsingLocalCoords.invert(&fLocalMatrix)) {
        fLocalMatrix.setIdentity();
    }
}

class GrCCPathProcessor::Impl : public GrGLSLGeometryProcessor {
public:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override;

private:
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 FPCoordTransformIter&& transformIter) override {
        const GrCCPathProcessor& proc = primProc.cast<GrCCPathProcessor>();
        pdman.set2f(fAtlasAdjustUniform, 1.0f / proc.atlasSize().fWidth,
                    1.0f / proc.atlasSize().fHeight);
        this->setTransformDataHelper(proc.localMatrix(), pdman, &transformIter);
    }

    GrGLSLUniformHandler::UniformHandle fAtlasAdjustUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLSLPrimitiveProcessor* GrCCPathProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl();
}

void GrCCPathProcessor::drawPaths(GrOpFlushState* flushState, const GrPipeline& pipeline,
                                  const GrPipeline::FixedDynamicState* fixedDynamicState,
                                  const GrCCPerFlushResources& resources, int baseInstance,
                                  int endInstance, const SkRect& bounds) const {
    const GrCaps& caps = flushState->caps();
    GrPrimitiveType primitiveType = caps.usePrimitiveRestart()
                                            ? GrPrimitiveType::kTriangleStrip
                                            : GrPrimitiveType::kTriangles;
    int numIndicesPerInstance = caps.usePrimitiveRestart()
                                        ? SK_ARRAY_COUNT(kOctoIndicesAsStrips)
                                        : SK_ARRAY_COUNT(kOctoIndicesAsTris);
    GrMesh mesh(primitiveType);
    auto enablePrimitiveRestart = GrPrimitiveRestart(flushState->caps().usePrimitiveRestart());

    mesh.setIndexedInstanced(resources.refIndexBuffer(), numIndicesPerInstance,
                             resources.refInstanceBuffer(), endInstance - baseInstance,
                             baseInstance, enablePrimitiveRestart);
    mesh.setVertexData(resources.refVertexBuffer());

    flushState->rtCommandBuffer()->draw(*this, pipeline, fixedDynamicState, nullptr, &mesh, 1,
                                        bounds);
}

void GrCCPathProcessor::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    using Interpolation = GrGLSLVaryingHandler::Interpolation;

    const GrCCPathProcessor& proc = args.fGP.cast<GrCCPathProcessor>();
    GrGLSLUniformHandler* uniHandler = args.fUniformHandler;
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;

    const char* atlasAdjust;
    fAtlasAdjustUniform = uniHandler->addUniform(
            kVertex_GrShaderFlag, kFloat2_GrSLType, "atlas_adjust", &atlasAdjust);

    varyingHandler->emitAttributes(proc);

    GrGLSLVarying texcoord(kFloat3_GrSLType);
    GrGLSLVarying color(kHalf4_GrSLType);
    varyingHandler->addVarying("texcoord", &texcoord);
    varyingHandler->addPassThroughAttribute(
            kInstanceAttribs[kColorAttribIdx], args.fOutputColor, Interpolation::kCanBeFlat);

    // The vertex shader bloats and intersects the devBounds and devBounds45 rectangles, in order to
    // find an octagon that circumscribes the (bloated) path.
    GrGLSLVertexBuilder* v = args.fVertBuilder;

    // Are we clockwise (positive wind, nonzero fill), or counter-clockwise (negative wind,
    // even/odd fill)?
    v->codeAppendf("float wind = sign(devbounds.z - devbounds.x);");

    // Find our reference corner from the device-space bounding box.
    v->codeAppendf("float2 refpt = mix(devbounds.xy, devbounds.zw, corners.xy);");

    // Find our reference corner from the 45-degree bounding box.
    v->codeAppendf("float2 refpt45 = mix(devbounds45.xy, devbounds45.zw, corners.zw);");
    // Transform back to device space.
    v->codeAppendf("refpt45 *= float2x2(+1, +1, -wind, +wind) * .5;");

    // Find the normals to each edge, then intersect them to find our octagon vertex.
    v->codeAppendf("float2x2 N = float2x2("
                           "corners.z + corners.w - 1, corners.w - corners.z, "
                           "corners.xy*2 - 1);");
    v->codeAppendf("N = float2x2(wind, 0, 0, 1) * N;");
    v->codeAppendf("float2 K = float2(dot(N[0], refpt), dot(N[1], refpt45));");
    v->codeAppendf("float2 octocoord = K * inverse(N);");

    // Round the octagon out to ensure we rasterize every pixel the path might touch. (Positive
    // bloatdir means we should take the "ceil" and negative means to take the "floor".)
    //
    // NOTE: If we were just drawing a rect, ceil/floor would be enough. But since there are also
    // diagonals in the octagon that cross through pixel centers, we need to outset by another
    // quarter px to ensure those pixels get rasterized.
    v->codeAppendf("float2 bloatdir = (0 != N[0].x) "
                           "? float2(N[0].x, N[1].y)"
                           ": float2(N[1].x, N[0].y);");
    v->codeAppendf("octocoord = (ceil(octocoord * bloatdir - 1e-4) + 0.25) * bloatdir;");

    // Convert to atlas coordinates in order to do our texture lookup.
    v->codeAppendf("float2 atlascoord = octocoord + float2(dev_to_atlas_offset);");
    if (kTopLeft_GrSurfaceOrigin == proc.atlasOrigin()) {
        v->codeAppendf("%s.xy = atlascoord * %s;", texcoord.vsOut(), atlasAdjust);
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == proc.atlasOrigin());
        v->codeAppendf("%s.xy = float2(atlascoord.x * %s.x, 1 - atlascoord.y * %s.y);",
                       texcoord.vsOut(), atlasAdjust, atlasAdjust);
    }
    v->codeAppendf("%s.z = wind * .5;", texcoord.vsOut());

    gpArgs->fPositionVar.set(kFloat2_GrSLType, "octocoord");
    this->emitTransforms(v, varyingHandler, uniHandler, gpArgs->fPositionVar, proc.localMatrix(),
                         args.fFPCoordTransformHandler);

    // Fragment shader.
    GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

    // Look up coverage count in the atlas.
    f->codeAppend ("half coverage = ");
    f->appendTextureLookup(args.fTexSamplers[0], SkStringPrintf("%s.xy", texcoord.fsIn()).c_str(),
                           kFloat2_GrSLType);
    f->codeAppend (".a;");

    // Scale coverage count by .5. Make it negative for even-odd paths and positive for winding
    // ones. Clamp winding coverage counts at 1.0 (i.e. min(coverage/2, .5)).
    f->codeAppendf("coverage = min(abs(coverage) * half(%s.z), .5);", texcoord.fsIn());

    // For negative values, this finishes the even-odd sawtooth function. Since positive (winding)
    // values were clamped at "coverage/2 = .5", this only undoes the previous multiply by .5.
    f->codeAppend ("coverage = 1 - abs(fract(coverage) * 2 - 1);");

    f->codeAppendf("%s = half4(coverage);", args.fOutputCoverage);
}
