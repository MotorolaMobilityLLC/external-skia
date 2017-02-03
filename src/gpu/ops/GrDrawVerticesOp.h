/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawVerticesOp_DEFINED
#define GrDrawVerticesOp_DEFINED

#include "GrColor.h"
#include "GrMeshDrawOp.h"
#include "GrRenderTargetContext.h"
#include "GrTypes.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "SkTDArray.h"

class GrOpFlushState;
struct GrInitInvariantOutput;

class GrDrawVerticesOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrColor color, GrPrimitiveType primitiveType,
                                          const SkMatrix& viewMatrix, const SkPoint* positions,
                                          int vertexCount, const uint16_t* indices, int indexCount,
                                          const uint32_t* colors, const SkPoint* localCoords,
                                          const SkRect& bounds,
                                          GrRenderTargetContext::ColorArrayType colorArrayType) {
        return std::unique_ptr<GrDrawOp>(new GrDrawVerticesOp(
                color, primitiveType, viewMatrix, positions, vertexCount, indices, indexCount,
                colors, localCoords, bounds, colorArrayType));
    }

    const char* name() const override { return "DrawVerticesOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("PrimType: %d, VarColor: %d, VCount: %d, ICount: %d\n", fPrimitiveType,
                       fVariableColor, fVertexCount, fIndexCount);
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrDrawVerticesOp(GrColor color, GrPrimitiveType primitiveType, const SkMatrix& viewMatrix,
                     const SkPoint* positions, int vertexCount, const uint16_t* indices,
                     int indexCount, const uint32_t* colors, const SkPoint* localCoords,
                     const SkRect& bounds, GrRenderTargetContext::ColorArrayType colorArrayType);

    void getPipelineAnalysisInput(GrPipelineAnalysisDrawOpInput* input) const override;
    void applyPipelineOptimizations(const GrPipelineOptimizations&) override;
    void onPrepareDraws(Target*) const override;

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    bool combinablePrimitive() const {
        return kTriangles_GrPrimitiveType == fPrimitiveType ||
               kLines_GrPrimitiveType == fPrimitiveType ||
               kPoints_GrPrimitiveType == fPrimitiveType;
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps&) override;

    struct Mesh {
        GrColor fColor;  // Only used if there are no per-vertex colors
        SkTDArray<SkPoint> fPositions;
        SkTDArray<uint16_t> fIndices;
        SkTDArray<uint32_t> fColors;
        SkTDArray<SkPoint> fLocalCoords;
        SkMatrix fViewMatrix;
    };

    GrPrimitiveType fPrimitiveType;
    bool fVariableColor;
    int fVertexCount;
    int fIndexCount;
    bool fMultipleViewMatrices = false;
    bool fPipelineNeedsLocalCoords;
    GrRenderTargetContext::ColorArrayType fColorArrayType;
    SkSTArray<1, Mesh, true> fMeshes;

    typedef GrMeshDrawOp INHERITED;
};

#endif
