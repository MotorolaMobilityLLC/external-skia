/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCCoverageProcessor.h"

#include "GrMesh.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

using Shader = GrCCCoverageProcessor::Shader;

static constexpr int kAttribIdx_X = 0;
static constexpr int kAttribIdx_Y = 1;
static constexpr int kAttribIdx_VertexData = 2;

/**
 * This class and its subclasses implement the coverage processor with vertex shaders.
 */
class GrCCCoverageProcessor::VSImpl : public GrGLSLGeometryProcessor {
protected:
    VSImpl(std::unique_ptr<Shader> shader) : fShader(std::move(shader)) {}

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) final {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
        const GrCCCoverageProcessor& proc = args.fGP.cast<GrCCCoverageProcessor>();

        // Vertex shader.
        GrGLSLVertexBuilder* v = args.fVertBuilder;
        int numInputPoints = proc.numInputPoints();

        const char* swizzle = (4 == numInputPoints) ? "xyzw" : "xyz";
        v->codeAppendf("float%ix2 pts = transpose(float2x%i(%s.%s, %s.%s));",
                       numInputPoints, numInputPoints, proc.getAttrib(kAttribIdx_X).fName, swizzle,
                       proc.getAttrib(kAttribIdx_Y).fName, swizzle);

        if (WindMethod::kCrossProduct == proc.fWindMethod) {
            v->codeAppend ("float area_x2 = determinant(float2x2(pts[0] - pts[1], "
                                                                "pts[0] - pts[2]));");
            if (4 == numInputPoints) {
                v->codeAppend ("area_x2 += determinant(float2x2(pts[0] - pts[2], "
                                                               "pts[0] - pts[3]));");
            }
            v->codeAppend ("half wind = sign(area_x2);");
        } else {
            SkASSERT(WindMethod::kInstanceData == proc.fWindMethod);
            SkASSERT(3 == numInputPoints);
            SkASSERT(kFloat4_GrVertexAttribType == proc.getAttrib(kAttribIdx_X).fType);
            v->codeAppendf("half wind = %s.w;", proc.getAttrib(kAttribIdx_X).fName);
        }

        float bloat = kAABloatRadius;
#ifdef SK_DEBUG
        if (proc.debugVisualizationsEnabled()) {
            bloat *= proc.debugBloat();
        }
#endif
        v->defineConstant("bloat", bloat);

        const char* coverage = this->emitVertexPosition(proc, v, gpArgs);
        SkASSERT(kFloat2_GrSLType == gpArgs->fPositionVar.getType());

        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        SkString varyingCode;
        fShader->emitVaryings(varyingHandler, GrGLSLVarying::Scope::kVertToFrag, &varyingCode,
                              gpArgs->fPositionVar.c_str(), coverage, "wind");
        v->codeAppend(varyingCode.c_str());

        varyingHandler->emitAttributes(proc);
        SkASSERT(!args.fFPCoordTransformHandler->nextCoordTransform());

        // Fragment shader.
        fShader->emitFragmentCode(proc, args.fFragBuilder, args.fOutputColor, args.fOutputCoverage);
    }

    virtual const char* emitVertexPosition(const GrCCCoverageProcessor&, GrGLSLVertexBuilder*,
                                           GrGPArgs*) const = 0;

    virtual ~VSImpl() {}

    const std::unique_ptr<Shader> fShader;

    typedef GrGLSLGeometryProcessor INHERITED;
};

static constexpr int kVertexData_LeftNeighborIdShift = 9;
static constexpr int kVertexData_RightNeighborIdShift = 7;
static constexpr int kVertexData_BloatIdxShift = 5;
static constexpr int kVertexData_InvertCoverageBit = 1 << 4;
static constexpr int kVertexData_IsEdgeBit = 1 << 3;
static constexpr int kVertexData_IsHullBit = 1 << 2;

/**
 * Vertex data tells the shader how to offset vertices for conservative raster, and how/whether to
 * calculate initial coverage values for edges. See VSHullAndEdgeImpl.
 */
static constexpr int32_t pack_vertex_data(int32_t leftNeighborID, int32_t rightNeighborID,
                                          int32_t bloatIdx, int32_t cornerID,
                                          int32_t extraData = 0) {
    return (leftNeighborID << kVertexData_LeftNeighborIdShift) |
           (rightNeighborID << kVertexData_RightNeighborIdShift) |
           (bloatIdx << kVertexData_BloatIdxShift) |
           cornerID | extraData;
}

static constexpr int32_t hull_vertex_data(int32_t cornerID, int32_t bloatIdx, int n) {
    return pack_vertex_data((cornerID + n - 1) % n, (cornerID + 1) % n, bloatIdx, cornerID,
                            kVertexData_IsHullBit);
}

static constexpr int32_t edge_vertex_data(int32_t edgeID, int32_t endptIdx, int32_t bloatIdx,
                                          int n) {
    return pack_vertex_data(0 == endptIdx ? (edgeID + 1) % n : edgeID,
                            0 == endptIdx ? (edgeID + 1) % n : edgeID,
                            bloatIdx, 0 == endptIdx ? edgeID : (edgeID + 1) % n,
                            kVertexData_IsEdgeBit |
                            (!endptIdx ? kVertexData_InvertCoverageBit : 0));
}

static constexpr int32_t kHull3AndEdgeVertices[] = {
    hull_vertex_data(0, 0, 3),
    hull_vertex_data(0, 1, 3),
    hull_vertex_data(0, 2, 3),
    hull_vertex_data(1, 0, 3),
    hull_vertex_data(1, 1, 3),
    hull_vertex_data(1, 2, 3),
    hull_vertex_data(2, 0, 3),
    hull_vertex_data(2, 1, 3),
    hull_vertex_data(2, 2, 3),

    edge_vertex_data(0, 0, 0, 3),
    edge_vertex_data(0, 0, 1, 3),
    edge_vertex_data(0, 0, 2, 3),
    edge_vertex_data(0, 1, 0, 3),
    edge_vertex_data(0, 1, 1, 3),
    edge_vertex_data(0, 1, 2, 3),

    edge_vertex_data(1, 0, 0, 3),
    edge_vertex_data(1, 0, 1, 3),
    edge_vertex_data(1, 0, 2, 3),
    edge_vertex_data(1, 1, 0, 3),
    edge_vertex_data(1, 1, 1, 3),
    edge_vertex_data(1, 1, 2, 3),

    edge_vertex_data(2, 0, 0, 3),
    edge_vertex_data(2, 0, 1, 3),
    edge_vertex_data(2, 0, 2, 3),
    edge_vertex_data(2, 1, 0, 3),
    edge_vertex_data(2, 1, 1, 3),
    edge_vertex_data(2, 1, 2, 3),
};

GR_DECLARE_STATIC_UNIQUE_KEY(gHull3AndEdgeVertexBufferKey);

static constexpr uint16_t kRestartStrip = 0xffff;

static constexpr uint16_t kHull3AndEdgeIndicesAsStrips[] =  {
    1, 2, 0, 3, 8, kRestartStrip, // First corner and main body of the hull.
    4, 5, 3, 6, 8, 7, kRestartStrip, // Opposite side and corners of the hull.
    10, 9, 11, 14, 12, 13, kRestartStrip, // First edge.
    16, 15, 17, 20, 18, 19, kRestartStrip, // Second edge.
    22, 21, 23, 26, 24, 25 // Third edge.
};

static constexpr uint16_t kHull3AndEdgeIndicesAsTris[] =  {
    // First corner and main body of the hull.
    1, 2, 0,
    2, 3, 0,
    0, 3, 8, // Main body.

    // Opposite side and corners of the hull.
    4, 5, 3,
    5, 6, 3,
    3, 6, 8,
    6, 7, 8,

    // First edge.
    10,  9, 11,
     9, 14, 11,
    11, 14, 12,
    14, 13, 12,

    // Second edge.
    16, 15, 17,
    15, 20, 17,
    17, 20, 18,
    20, 19, 18,

    // Third edge.
    22, 21, 23,
    21, 26, 23,
    23, 26, 24,
    26, 25, 24,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gHull3AndEdgeIndexBufferKey);

static constexpr int32_t kHull4Vertices[] = {
    hull_vertex_data(0, 0, 4),
    hull_vertex_data(0, 1, 4),
    hull_vertex_data(0, 2, 4),
    hull_vertex_data(1, 0, 4),
    hull_vertex_data(1, 1, 4),
    hull_vertex_data(1, 2, 4),
    hull_vertex_data(2, 0, 4),
    hull_vertex_data(2, 1, 4),
    hull_vertex_data(2, 2, 4),
    hull_vertex_data(3, 0, 4),
    hull_vertex_data(3, 1, 4),
    hull_vertex_data(3, 2, 4),

    // No edges for now (beziers don't use edges).
};

GR_DECLARE_STATIC_UNIQUE_KEY(gHull4VertexBufferKey);

static constexpr uint16_t kHull4IndicesAsStrips[] =  {
    1, 0, 2, 11, 3, 5, 4, kRestartStrip, // First half of the hull (split diagonally).
    7, 6, 8, 5, 9, 11, 10 // Second half of the hull.
};

static constexpr uint16_t kHull4IndicesAsTris[] =  {
    // First half of the hull (split diagonally).
     1,  0,  2,
     0, 11,  2,
     2, 11,  3,
    11,  5,  3,
     3,  5,  4,

    // Second half of the hull.
    7,  6,  8,
    6,  5,  8,
    8,  5,  9,
    5, 11,  9,
    9, 11, 10,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gHull4IndexBufferKey);

/**
 * Generates a conservative raster hull around a convex polygon. For triangles, we also generate
 * independent conservative rasters around each edge. (See comments for RenderPass)
 */
class VSHullAndEdgeImpl : public GrCCCoverageProcessor::VSImpl {
public:
    VSHullAndEdgeImpl(std::unique_ptr<Shader> shader, int numSides)
            : VSImpl(std::move(shader)), fNumSides(numSides) {}

    const char* emitVertexPosition(const GrCCCoverageProcessor& proc, GrGLSLVertexBuilder* v,
                                   GrGPArgs* gpArgs) const override {
        Shader::GeometryVars vars;
        fShader->emitSetupCode(v, "pts", nullptr, "wind", &vars);

        const char* hullPts = vars.fHullVars.fAlternatePoints;
        if (!hullPts) {
            hullPts = "pts";
        }

        // Reverse all indices if the wind is counter-clockwise: [0, 1, 2] -> [2, 1, 0].
        v->codeAppendf("int clockwise_indices = wind > 0 ? %s : 0x%x - %s;",
                       proc.getAttrib(kAttribIdx_VertexData).fName,
                       ((fNumSides - 1) << kVertexData_LeftNeighborIdShift) |
                       ((fNumSides - 1) << kVertexData_RightNeighborIdShift) |
                       (((1 << kVertexData_RightNeighborIdShift) - 1) ^ 3) |
                       (fNumSides - 1),
                       proc.getAttrib(kAttribIdx_VertexData).fName);

        // Here we generate conservative raster geometry for the input polygon. It is the convex
        // hull of N pixel-size boxes, one centered on each the input points. Each corner has three
        // vertices, where one or two may cause degenerate triangles. The vertex data tells us how
        // to offset each vertex. For more details on conservative raster, see:
        // https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter42.html
        //
        // Triangle edges are also handled here using the same concept (see kHull3AndEdgeVertices).
        v->codeAppendf("float2 corner = %s[clockwise_indices & 3];", hullPts);
        v->codeAppendf("float2 left = %s[clockwise_indices >> %i];",
                       hullPts, kVertexData_LeftNeighborIdShift);
        v->codeAppendf("float2 right = %s[(clockwise_indices >> %i) & 3];",
                       hullPts, kVertexData_RightNeighborIdShift);

        v->codeAppend ("float2 leftbloat = sign(corner - left);");
        v->codeAppend ("leftbloat = float2(0 != leftbloat.y ? leftbloat.y : leftbloat.x, "
                                          "0 != leftbloat.x ? -leftbloat.x : -leftbloat.y);");

        v->codeAppend ("float2 rightbloat = sign(right - corner);");
        v->codeAppend ("rightbloat = float2(0 != rightbloat.y ? rightbloat.y : rightbloat.x, "
                                           "0 != rightbloat.x ? -rightbloat.x : -rightbloat.y);");

        v->codeAppend ("bool2 left_right_notequal = notEqual(leftbloat, rightbloat);");

        // At each corner of the polygon, our hull will have either 1, 2, or 3 vertices. We begin
        // with the first hull vertex (leftbloat), then continue rotating 90 degrees clockwise until
        // we reach the desired vertex for this invocation. Corners with less than 3 corresponding
        // hull vertices will result in redundant vertices and degenerate triangles.
        v->codeAppend ("float2 bloatdir = leftbloat;");
        v->codeAppendf("int bloatidx = (%s >> %i) & 3;",
                       proc.getAttrib(kAttribIdx_VertexData).fName, kVertexData_BloatIdxShift);
        v->codeAppend ("switch (bloatidx) {");
        v->codeAppend (    "case 2:");
        v->codeAppendf(        "if (all(left_right_notequal)) {");
        v->codeAppend (            "bloatdir = float2(-bloatdir.y, +bloatdir.x);");
        v->codeAppend (        "}");
                               // fallthru.
        v->codeAppend (    "case 1:");
        v->codeAppendf(        "if (any(left_right_notequal)) {");
        v->codeAppend (            "bloatdir = float2(-bloatdir.y, +bloatdir.x);");
        v->codeAppend (        "}");
                               // fallthru.
        v->codeAppend ("}");

        // For triangles, we also emit coverage in order to handle edges and corners.
        const char* coverage = nullptr;
        if (3 == fNumSides) {
            v->codeAppend ("half coverage;");
            Shader::CalcEdgeCoverageAtBloatVertex(v, "left", "corner", "bloatdir", "coverage");
            v->codeAppendf("if (0 != (%s & %i)) {", // Are we the opposite endpoint of an edge?
                           proc.getAttrib(kAttribIdx_VertexData).fName,
                           kVertexData_InvertCoverageBit);
            v->codeAppend (    "coverage = -1 - coverage;");
            v->codeAppend ("}");

            v->codeAppendf("if (0 != (%s & %i)) {", // Are we a hull vertex?
                           proc.getAttrib(kAttribIdx_VertexData).fName, kVertexData_IsHullBit);
            v->codeAppend (    "coverage = +1;"); // Hull coverage is +1 all around.
            v->codeAppend ("}");

            coverage = "coverage";
        }

        v->codeAppend ("float2 vertex = corner + bloatdir * bloat;");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertex");

        return coverage;
    }

private:
    const int fNumSides;
};

static constexpr uint16_t kCornerIndicesAsStrips[] =  {
    0, 1, 2, 3, kRestartStrip, // First corner.
    4, 5, 6, 7, kRestartStrip, // Second corner.
    8, 9, 10, 11 // Third corner.
};

static constexpr uint16_t kCornerIndicesAsTris[] =  {
    // First corner.
    0,  1,  2,
    1,  3,  2,

    // Second corner.
    4,  5,  6,
    5,  7,  6,

    // Third corner.
    8,  9, 10,
    9, 11, 10,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gCornerIndexBufferKey);

/**
 * Generates conservative rasters around corners. (See comments for RenderPass)
 */
class VSCornerImpl : public GrCCCoverageProcessor::VSImpl {
public:
    VSCornerImpl(std::unique_ptr<Shader> shader) : VSImpl(std::move(shader)) {}

    const char* emitVertexPosition(const GrCCCoverageProcessor&, GrGLSLVertexBuilder* v,
                                   GrGPArgs* gpArgs) const override {
        Shader::GeometryVars vars;
        v->codeAppend ("int corner_id = sk_VertexID / 4;");
        fShader->emitSetupCode(v, "pts", "corner_id", "wind", &vars);

        v->codeAppendf("float2 vertex = %s;", vars.fCornerVars.fPoint);
        v->codeAppend ("vertex.x += (0 == (sk_VertexID & 2)) ? -bloat : +bloat;");
        v->codeAppend ("vertex.y += (0 == (sk_VertexID & 1)) ? -bloat : +bloat;");

        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertex");
        return nullptr; // Corner vertices don't have an initial coverage value.
    }
};

void GrCCCoverageProcessor::initVS(GrResourceProvider* rp) {
    SkASSERT(Impl::kVertexShader == fImpl);
    const GrCaps& caps = *rp->caps();

    switch (fRenderPass) {
        case RenderPass::kTriangleHulls: {
            GR_DEFINE_STATIC_UNIQUE_KEY(gHull3AndEdgeVertexBufferKey);
            fVertexBuffer = rp->findOrMakeStaticBuffer(kVertex_GrBufferType,
                                                       sizeof(kHull3AndEdgeVertices),
                                                       kHull3AndEdgeVertices,
                                                       gHull3AndEdgeVertexBufferKey);
            GR_DEFINE_STATIC_UNIQUE_KEY(gHull3AndEdgeIndexBufferKey);
            if (caps.usePrimitiveRestart()) {
                fIndexBuffer = rp->findOrMakeStaticBuffer(kIndex_GrBufferType,
                                                          sizeof(kHull3AndEdgeIndicesAsStrips),
                                                          kHull3AndEdgeIndicesAsStrips,
                                                          gHull3AndEdgeIndexBufferKey);
                fNumIndicesPerInstance = SK_ARRAY_COUNT(kHull3AndEdgeIndicesAsStrips);
            } else {
                fIndexBuffer = rp->findOrMakeStaticBuffer(kIndex_GrBufferType,
                                                          sizeof(kHull3AndEdgeIndicesAsTris),
                                                          kHull3AndEdgeIndicesAsTris,
                                                          gHull3AndEdgeIndexBufferKey);
                fNumIndicesPerInstance = SK_ARRAY_COUNT(kHull3AndEdgeIndicesAsTris);
            }
            break;
        }
        case RenderPass::kQuadraticHulls:
        case RenderPass::kCubicHulls: {
            GR_DEFINE_STATIC_UNIQUE_KEY(gHull4VertexBufferKey);
            fVertexBuffer = rp->findOrMakeStaticBuffer(kVertex_GrBufferType, sizeof(kHull4Vertices),
                                                       kHull4Vertices, gHull4VertexBufferKey);
            GR_DEFINE_STATIC_UNIQUE_KEY(gHull4IndexBufferKey);
            if (caps.usePrimitiveRestart()) {
                fIndexBuffer = rp->findOrMakeStaticBuffer(kIndex_GrBufferType,
                                                          sizeof(kHull4IndicesAsStrips),
                                                          kHull4IndicesAsStrips,
                                                          gHull4IndexBufferKey);
                fNumIndicesPerInstance = SK_ARRAY_COUNT(kHull4IndicesAsStrips);
            } else {
                fIndexBuffer = rp->findOrMakeStaticBuffer(kIndex_GrBufferType,
                                                          sizeof(kHull4IndicesAsTris),
                                                          kHull4IndicesAsTris,
                                                          gHull4IndexBufferKey);
                fNumIndicesPerInstance = SK_ARRAY_COUNT(kHull4IndicesAsTris);
            }
            break;
        }
        case RenderPass::kTriangleEdges:
            SK_ABORT("kTriangleEdges RenderPass is not used by VSImpl.");
            break;
        case RenderPass::kTriangleCorners:
        case RenderPass::kQuadraticCorners:
        case RenderPass::kCubicCorners: {
            GR_DEFINE_STATIC_UNIQUE_KEY(gCornerIndexBufferKey);
            if (caps.usePrimitiveRestart()) {
                fIndexBuffer = rp->findOrMakeStaticBuffer(kIndex_GrBufferType,
                                                          sizeof(kCornerIndicesAsStrips),
                                                          kCornerIndicesAsStrips,
                                                          gCornerIndexBufferKey);
                fNumIndicesPerInstance = SK_ARRAY_COUNT(kCornerIndicesAsStrips);
            } else {
                fIndexBuffer = rp->findOrMakeStaticBuffer(kIndex_GrBufferType,
                                                          sizeof(kCornerIndicesAsTris),
                                                          kCornerIndicesAsTris,
                                                          gCornerIndexBufferKey);
                fNumIndicesPerInstance = SK_ARRAY_COUNT(kCornerIndicesAsTris);
            }
            if (RenderPass::kTriangleCorners != fRenderPass) {
                fNumIndicesPerInstance = fNumIndicesPerInstance * 2/3;
            }
            break;
        }
    }

    if (RenderPassIsCubic(fRenderPass) || WindMethod::kInstanceData == fWindMethod) {
        SkASSERT(WindMethod::kCrossProduct == fWindMethod || 3 == this->numInputPoints());

        SkASSERT(kAttribIdx_X == this->numAttribs());
        this->addInstanceAttrib("X", kFloat4_GrVertexAttribType);

        SkASSERT(kAttribIdx_Y == this->numAttribs());
        this->addInstanceAttrib("Y", kFloat4_GrVertexAttribType);

        SkASSERT(offsetof(QuadPointInstance, fX) == this->getAttrib(kAttribIdx_X).fOffsetInRecord);
        SkASSERT(offsetof(QuadPointInstance, fY) == this->getAttrib(kAttribIdx_Y).fOffsetInRecord);
        SkASSERT(sizeof(QuadPointInstance) == this->getInstanceStride());
    } else {
        SkASSERT(kAttribIdx_X == this->numAttribs());
        this->addInstanceAttrib("X", kFloat3_GrVertexAttribType);

        SkASSERT(kAttribIdx_Y == this->numAttribs());
        this->addInstanceAttrib("Y", kFloat3_GrVertexAttribType);

        SkASSERT(offsetof(TriPointInstance, fX) == this->getAttrib(kAttribIdx_X).fOffsetInRecord);
        SkASSERT(offsetof(TriPointInstance, fY) == this->getAttrib(kAttribIdx_Y).fOffsetInRecord);
        SkASSERT(sizeof(TriPointInstance) == this->getInstanceStride());
    }

    if (fVertexBuffer) {
        SkASSERT(kAttribIdx_VertexData == this->numAttribs());
        this->addVertexAttrib("vertexdata", kInt_GrVertexAttribType);

        SkASSERT(sizeof(int32_t) == this->getVertexStride());
    }

    if (caps.usePrimitiveRestart()) {
        this->setWillUsePrimitiveRestart();
        fPrimitiveType = GrPrimitiveType::kTriangleStrip;
    } else {
        fPrimitiveType = GrPrimitiveType::kTriangles;
    }
}

void GrCCCoverageProcessor::appendVSMesh(GrBuffer* instanceBuffer, int instanceCount,
                                         int baseInstance, SkTArray<GrMesh>* out) const {
    SkASSERT(Impl::kVertexShader == fImpl);
    GrMesh& mesh = out->emplace_back(fPrimitiveType);
    mesh.setIndexedInstanced(fIndexBuffer.get(), fNumIndicesPerInstance, instanceBuffer,
                             instanceCount, baseInstance);
    if (fVertexBuffer) {
        mesh.setVertexData(fVertexBuffer.get(), 0);
    }
}

GrGLSLPrimitiveProcessor* GrCCCoverageProcessor::createVSImpl(std::unique_ptr<Shader> shadr) const {
    switch (fRenderPass) {
        case RenderPass::kTriangleHulls:
            return new VSHullAndEdgeImpl(std::move(shadr), 3);
        case RenderPass::kQuadraticHulls:
        case RenderPass::kCubicHulls:
            return new VSHullAndEdgeImpl(std::move(shadr), 4);
        case RenderPass::kTriangleEdges:
            SK_ABORT("kTriangleEdges RenderPass is not used by VSImpl.");
            return nullptr;
        case RenderPass::kTriangleCorners:
        case RenderPass::kQuadraticCorners:
        case RenderPass::kCubicCorners:
            return new VSCornerImpl(std::move(shadr));
    }
    SK_ABORT("Invalid RenderPass");
    return nullptr;
}
