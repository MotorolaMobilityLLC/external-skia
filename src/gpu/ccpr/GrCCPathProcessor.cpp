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
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLVarying.h"

// Slightly undershoot an AA bloat radius of 0.5 so vertices that fall on integer boundaries don't
// accidentally reach into neighboring path masks within the atlas.
constexpr float kAABloatRadius = 0.491111f;

// Paths are drawn as octagons. Each point on the octagon is the intersection of two lines: one edge
// from the path's bounding box and one edge from its 45-degree bounding box. The below inputs
// define a vertex by the two edges that need to be intersected. Normals point out of the octagon,
// and the bounding boxes are sent in as instance attribs.
static constexpr float kOctoEdgeNorms[8 * 4] = {
    // bbox   // bbox45
    -1, 0,    -1,+1,
    -1, 0,    -1,-1,
     0,-1,    -1,-1,
     0,-1,    +1,-1,
    +1, 0,    +1,-1,
    +1, 0,    +1,+1,
     0,+1,    +1,+1,
     0,+1,    -1,+1,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gVertexBufferKey);

sk_sp<const GrBuffer> GrCCPathProcessor::FindVertexBuffer(GrOnFlushResourceProvider* onFlushRP) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gVertexBufferKey);
    return onFlushRP->findOrMakeStaticBuffer(kVertex_GrBufferType, sizeof(kOctoEdgeNorms),
                                             kOctoEdgeNorms, gVertexBufferKey);
}

static constexpr uint16_t kRestartStrip = 0xffff;

static constexpr uint16_t kOctoIndicesAsStrips[] = {
    1, 0, 2, 4, 3, kRestartStrip, // First half.
    5, 4, 6, 0, 7 // Second half.
};

static constexpr uint16_t kOctoIndicesAsTris[] = {
    // First half.
    1, 0, 2,
    0, 4, 2,
    2, 4, 3,

    // Second half.
    5, 4, 6,
    4, 0, 6,
    6, 0, 7,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gIndexBufferKey);

sk_sp<const GrBuffer> GrCCPathProcessor::FindIndexBuffer(GrOnFlushResourceProvider* onFlushRP) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);
    if (onFlushRP->caps()->usePrimitiveRestart()) {
        return onFlushRP->findOrMakeStaticBuffer(kIndex_GrBufferType, sizeof(kOctoIndicesAsStrips),
                                                 kOctoIndicesAsStrips, gIndexBufferKey);
    } else {
        return onFlushRP->findOrMakeStaticBuffer(kIndex_GrBufferType, sizeof(kOctoIndicesAsTris),
                                                 kOctoIndicesAsTris, gIndexBufferKey);
    }
}

GrCCPathProcessor::GrCCPathProcessor(GrResourceProvider* resourceProvider,
                                     sk_sp<GrTextureProxy> atlas,
                                     const SkMatrix& viewMatrixIfUsingLocalCoords)
        : INHERITED(kGrCCPathProcessor_ClassID)
        , fAtlasAccess(std::move(atlas), GrSamplerState::Filter::kNearest,
                       GrSamplerState::WrapMode::kClamp, kFragment_GrShaderFlag) {
    this->addInstanceAttrib("devbounds", kFloat4_GrVertexAttribType);
    this->addInstanceAttrib("devbounds45", kFloat4_GrVertexAttribType);
    this->addInstanceAttrib("atlas_offset", kShort2_GrVertexAttribType);
    this->addInstanceAttrib("color", kUByte4_norm_GrVertexAttribType);

    SkASSERT(offsetof(Instance, fDevBounds) ==
             this->getInstanceAttrib(InstanceAttribs::kDevBounds).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fDevBounds45) ==
             this->getInstanceAttrib(InstanceAttribs::kDevBounds45).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fAtlasOffset) ==
             this->getInstanceAttrib(InstanceAttribs::kAtlasOffset).fOffsetInRecord);
    SkASSERT(offsetof(Instance, fColor) ==
             this->getInstanceAttrib(InstanceAttribs::kColor).fOffsetInRecord);
    SkASSERT(sizeof(Instance) == this->getInstanceStride());

    GR_STATIC_ASSERT(4 == kNumInstanceAttribs);

    this->addVertexAttrib("edge_norms", kFloat4_GrVertexAttribType);

    if (resourceProvider->caps()->usePrimitiveRestart()) {
        this->setWillUsePrimitiveRestart();
    }

    fAtlasAccess.instantiate(resourceProvider);
    this->addTextureSampler(&fAtlasAccess);

    if (!viewMatrixIfUsingLocalCoords.invert(&fLocalMatrix)) {
        fLocalMatrix.setIdentity();
    }
}

class GLSLPathProcessor : public GrGLSLGeometryProcessor {
public:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override;

private:
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 FPCoordTransformIter&& transformIter) override {
        const GrCCPathProcessor& proc = primProc.cast<GrCCPathProcessor>();
        pdman.set2f(fAtlasAdjustUniform, 1.0f / proc.atlas()->width(),
                    1.0f / proc.atlas()->height());
        this->setTransformDataHelper(proc.localMatrix(), pdman, &transformIter);
    }

    GrGLSLUniformHandler::UniformHandle fAtlasAdjustUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLSLPrimitiveProcessor* GrCCPathProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new GLSLPathProcessor();
}

void GrCCPathProcessor::drawPaths(GrOpFlushState* flushState, const GrPipeline& pipeline,
                                  const GrBuffer* indexBuffer, const GrBuffer* vertexBuffer,
                                  GrBuffer* instanceBuffer, int baseInstance, int endInstance,
                                  const SkRect& bounds) const {
    const GrCaps& caps = flushState->caps();
    GrPrimitiveType primitiveType = caps.usePrimitiveRestart()
                                            ? GrPrimitiveType::kTriangleStrip
                                            : GrPrimitiveType::kTriangles;
    int numIndicesPerInstance = caps.usePrimitiveRestart()
                                        ? SK_ARRAY_COUNT(kOctoIndicesAsStrips)
                                        : SK_ARRAY_COUNT(kOctoIndicesAsTris);
    GrMesh mesh(primitiveType);
    mesh.setIndexedInstanced(indexBuffer, numIndicesPerInstance, instanceBuffer,
                             endInstance - baseInstance, baseInstance);
    mesh.setVertexData(vertexBuffer);

    flushState->rtCommandBuffer()->draw(pipeline, *this, &mesh, nullptr, 1, bounds);
}

void GLSLPathProcessor::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    using InstanceAttribs = GrCCPathProcessor::InstanceAttribs;
    using Interpolation = GrGLSLVaryingHandler::Interpolation;

    const GrCCPathProcessor& proc = args.fGP.cast<GrCCPathProcessor>();
    GrGLSLUniformHandler* uniHandler = args.fUniformHandler;
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;

    const char* atlasAdjust;
    fAtlasAdjustUniform = uniHandler->addUniform(
            kVertex_GrShaderFlag,
            kFloat2_GrSLType, "atlas_adjust", &atlasAdjust);

    varyingHandler->emitAttributes(proc);

    GrGLSLVarying texcoord(kFloat3_GrSLType);
    GrGLSLVarying color(kHalf4_GrSLType);
    varyingHandler->addVarying("texcoord", &texcoord);
    varyingHandler->addPassThroughAttribute(&proc.getInstanceAttrib(InstanceAttribs::kColor),
                                            args.fOutputColor, Interpolation::kCanBeFlat);

    // The vertex shader bloats and intersects the devBounds and devBounds45 rectangles, in order to
    // find an octagon that circumscribes the (bloated) path.
    GrGLSLVertexBuilder* v = args.fVertBuilder;

    // Each vertex is the intersection of one edge from devBounds and one from devBounds45.
    // 'N' holds the normals to these edges as column vectors.
    //
    // NOTE: "float2x2(float4)" is valid and equivalent to "float2x2(float4.xy, float4.zw)",
    // however Intel compilers crash when we use the former syntax in this shader.
    v->codeAppendf("float2x2 N = float2x2(%s.xy, %s.zw);",
                   proc.getEdgeNormsAttrib().fName, proc.getEdgeNormsAttrib().fName);

    // N[0] is the normal for the edge we are intersecting from the regular bounding box, pointing
    // out of the octagon.
    v->codeAppendf("float4 devbounds = %s;",
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds).fName);
    v->codeAppend ("float2 refpt = (0 == sk_VertexID >> 2)"
                           "? float2(min(devbounds.x, devbounds.z), devbounds.y)"
                           ": float2(max(devbounds.x, devbounds.z), devbounds.w);");
    v->codeAppendf("refpt += N[0] * %f;", kAABloatRadius); // bloat for AA.

    // N[1] is the normal for the edge we are intersecting from the 45-degree bounding box, pointing
    // out of the octagon.
    v->codeAppendf("float2 refpt45 = (0 == ((sk_VertexID + 1) & (1 << 2))) ? %s.xy : %s.zw;",
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds45).fName,
                   proc.getInstanceAttrib(InstanceAttribs::kDevBounds45).fName);
    v->codeAppendf("refpt45 *= float2x2(.5,.5,-.5,.5);"); // transform back to device space.
    v->codeAppendf("refpt45 += N[1] * %f;", kAABloatRadius); // bloat for AA.

    v->codeAppend ("float2 K = float2(dot(N[0], refpt), dot(N[1], refpt45));");
    v->codeAppendf("float2 octocoord = K * inverse(N);");

    gpArgs->fPositionVar.set(kFloat2_GrSLType, "octocoord");

    // Convert to atlas coordinates in order to do our texture lookup.
    v->codeAppendf("float2 atlascoord = octocoord + float2(%s);",
                   proc.getInstanceAttrib(InstanceAttribs::kAtlasOffset).fName);
    if (kTopLeft_GrSurfaceOrigin == proc.atlasProxy()->origin()) {
        v->codeAppendf("%s.xy = atlascoord * %s;", texcoord.vsOut(), atlasAdjust);
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == proc.atlasProxy()->origin());
        v->codeAppendf("%s.xy = float2(atlascoord.x * %s.x, 1 - atlascoord.y * %s.y);",
                       texcoord.vsOut(), atlasAdjust, atlasAdjust);
    }
    // The third texture coordinate is -.5 for even-odd paths and +.5 for winding ones.
    // ("right < left" indicates even-odd fill type.)
    v->codeAppendf("%s.z = sign(devbounds.z - devbounds.x) * .5;", texcoord.vsOut());

    this->emitTransforms(v, varyingHandler, uniHandler, GrShaderVar("octocoord", kFloat2_GrSLType),
                         proc.localMatrix(), args.fFPCoordTransformHandler);

    // Fragment shader.
    GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

    // Look up coverage count in the atlas.
    f->codeAppend ("half coverage = ");
    f->appendTextureLookup(args.fTexSamplers[0], SkStringPrintf("%s.xy", texcoord.fsIn()).c_str(),
                           kFloat2_GrSLType);
    f->codeAppend (".a;");

    // Scale coverage count by .5. Make it negative for even-odd paths and positive for winding
    // ones. Clamp winding coverage counts at 1.0 (i.e. min(coverage/2, .5)).
    f->codeAppendf("coverage = min(abs(coverage) * %s.z, .5);", texcoord.fsIn());

    // For negative values, this finishes the even-odd sawtooth function. Since positive (winding)
    // values were clamped at "coverage/2 = .5", this only undoes the previous multiply by .5.
    f->codeAppend ("coverage = 1 - abs(fract(coverage) * 2 - 1);");

    f->codeAppendf("%s = half4(coverage);", args.fOutputCoverage);
}
