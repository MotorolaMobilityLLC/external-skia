/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/PathCurveTessellator.h"

#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/WangsFormula.h"

#if SK_GPU_V1
#include "src/gpu/GrMeshDrawTarget.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceProvider.h"
#endif

namespace skgpu {

using CubicPatch = PatchWriter::CubicPatch;
using ConicPatch = PatchWriter::ConicPatch;
using TrianglePatch = PatchWriter::TrianglePatch;

void PathCurveTessellator::prepare(GrMeshDrawTarget* target,
                                   int maxTessellationSegments,
                                   const SkMatrix& shaderMatrix,
                                   const PathDrawList& pathDrawList,
                                   int totalCombinedPathVerbCnt,
                                   const BreadcrumbTriangleList* breadcrumbTriangleList) {
    SkASSERT(fVertexChunkArray.empty());
    SkASSERT(!fFixedResolveLevel);

    // Determine how many triangles to allocate.
    int maxTriangles = 0;
    if (fDrawInnerFan) {
        int maxCombinedFanEdges = MaxCombinedFanEdgesInPathDrawList(totalCombinedPathVerbCnt);
        // A single n-sided polygon is fanned by n-2 triangles. Multiple polygons with a combined
        // edge count of n are fanned by strictly fewer triangles.
        int maxTrianglesInFans = std::max(maxCombinedFanEdges - 2, 0);
        maxTriangles += maxTrianglesInFans;
    }
    if (breadcrumbTriangleList) {
        maxTriangles += breadcrumbTriangleList->count();
    }
    // Over-allocate enough curves for 1 in 4 to chop.
    int curveAllocCount = (totalCombinedPathVerbCnt * 5 + 3) / 4;  // i.e., ceil(numVerbs * 5/4)
    int patchAllocCount = maxTriangles + curveAllocCount;
    if (!patchAllocCount) {
        return;
    }
    PatchWriter patchWriter(target, &fVertexChunkArray, PatchStride(fAttribs), patchAllocCount,
                            fAttribs);

    // Write out inner fan triangles.
    if (fDrawInnerFan) {
        for (auto [pathMatrix, path, color] : pathDrawList) {
            AffineMatrix m(pathMatrix);
            if (fAttribs & PatchAttribs::kColor) {
                patchWriter.updateColorAttrib(color);
            }
            for (PathMiddleOutFanIter it(path); !it.done();) {
                for (auto [p0, p1, p2] : it.nextStack()) {
                    TrianglePatch(patchWriter) << m.map2Points(p0, p1) << m.mapPoint(p2);
                }
            }
        }
    }

    // Write out breadcrumb triangles.
    if (breadcrumbTriangleList) {
        SkDEBUGCODE(int count = 0;)
#ifdef SK_DEBUG
        for (auto [pathMatrix, path, color] : pathDrawList) {
            // This assert isn't actually necessary, but we currently only use breadcrumb triangles
            // with an identity pathMatrix and transparent color. If that ever changes, this assert
            // will serve as a gentle reminder to make sure the breadcrumb triangles are also
            // transformed on the CPU.
            SkASSERT(pathMatrix.isIdentity());
            SkASSERT(color == SK_PMColor4fTRANSPARENT);
        }
#endif
        if (fAttribs & PatchAttribs::kColor) {
            patchWriter.updateColorAttrib(SK_PMColor4fTRANSPARENT);
        }
        for (const auto* tri = breadcrumbTriangleList->head(); tri; tri = tri->fNext) {
            SkDEBUGCODE(++count;)
            auto p0 = float2::Load(tri->fPts);
            auto p1 = float2::Load(tri->fPts + 1);
            auto p2 = float2::Load(tri->fPts + 2);
            if (skvx::any((p0 == p1) & (p1 == p2))) {
                // Cull completely horizontal or vertical triangles. GrTriangulator can't always
                // get these breadcrumb edges right when they run parallel to the sweep
                // direction because their winding is undefined by its current definition.
                // FIXME(skia:12060): This seemed safe, but if there is a view matrix it will
                // introduce T-junctions.
                continue;
            }
            TrianglePatch(patchWriter) << p0 << p1 << p2;
        }
        SkASSERT(count == breadcrumbTriangleList->count());
    }

    float maxSegments_pow2 = pow2(maxTessellationSegments);
    float maxSegments_pow4 = pow2(maxSegments_pow2);

    // If using fixed count, this is the number of segments we need to emit per instance. Always
    // emit at least 2 segments so we can support triangles.
    float numFixedSegments_pow4 = 2*2*2*2;

    for (auto [pathMatrix, path, color] : pathDrawList) {
        AffineMatrix m(pathMatrix);
        wangs_formula::VectorXform totalXform(SkMatrix::Concat(shaderMatrix, pathMatrix));
        if (fAttribs & PatchAttribs::kColor) {
            patchWriter.updateColorAttrib(color);
        }
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kQuad: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);
                    float n4 = wangs_formula::quadratic_pow4(kTessellationPrecision,
                                                             pts,
                                                             totalXform);
                    if (n4 <= 1) {
                        break;  // This quad only needs 1 segment, which is empty.
                    }
                    if (n4 <= maxSegments_pow4) {
                        // This quad already fits in "maxTessellationSegments".
                        CubicPatch(patchWriter) << QuadToCubic{p0, p1, p2};
                    } else {
                        // Chop until each quad tessellation requires "maxSegments" or fewer.
                        int numPatches =
                                SkScalarCeilToInt(wangs_formula::root4(n4/maxSegments_pow4));
                        patchWriter.chopAndWriteQuads(p0, p1, p2, numPatches);
                    }
                    numFixedSegments_pow4 = std::max(n4, numFixedSegments_pow4);
                    break;
                }

                case SkPathVerb::kConic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);
                    float n2 = wangs_formula::conic_pow2(kTessellationPrecision,
                                                         pts,
                                                         *w,
                                                         totalXform);
                    if (n2 <= 1) {
                        break;  // This conic only needs 1 segment, which is empty.
                    }
                    if (n2 <= maxSegments_pow2) {
                        // This conic already fits in "maxTessellationSegments".
                        ConicPatch(patchWriter) << p0 << p1 << p2 << *w;
                    } else {
                        // Chop until each conic tessellation requires "maxSegments" or fewer.
                        int numPatches = SkScalarCeilToInt(sqrtf(n2/maxSegments_pow2));
                        patchWriter.chopAndWriteConics(p0, p1, p2, *w, numPatches);
                    }
                    numFixedSegments_pow4 = std::max(n2*n2, numFixedSegments_pow4);
                    break;
                }

                case SkPathVerb::kCubic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto [p2, p3] = m.map2Points(pts+2);
                    float n4 = wangs_formula::cubic_pow4(kTessellationPrecision,
                                                         pts,
                                                         totalXform);
                    if (n4 <= 1) {
                        break;  // This cubic only needs 1 segment, which is empty.
                    }
                    if (n4 <= maxSegments_pow4) {
                        // This cubic already fits in "maxTessellationSegments".
                        CubicPatch(patchWriter) << p0 << p1 << p2 << p3;
                    } else {
                        // Chop until each cubic tessellation requires "maxSegments" or fewer.
                        int numPatches =
                                SkScalarCeilToInt(wangs_formula::root4(n4/maxSegments_pow4));
                        patchWriter.chopAndWriteCubics(p0, p1, p2, p3, numPatches);
                    }
                    numFixedSegments_pow4 = std::max(n4, numFixedSegments_pow4);
                    break;
                }

                default: break;
            }
        }
    }

    // log16(n^4) == log2(n).
    // We already chopped curves to make sure none needed a higher resolveLevel than
    // kMaxFixedResolveLevel.
    fFixedResolveLevel = std::min(wangs_formula::nextlog16(numFixedSegments_pow4),
                                  int(kMaxFixedResolveLevel));
}

void PathCurveTessellator::WriteFixedVertexBuffer(VertexWriter vertexWriter, size_t bufferSize) {
    SkASSERT(bufferSize >= sizeof(SkPoint) * 2);
    SkASSERT(bufferSize % sizeof(SkPoint) == 0);
    int vertexCount = bufferSize / sizeof(SkPoint);
    SkASSERT(vertexCount > 3);
    SkDEBUGCODE(VertexWriter end = vertexWriter.makeOffset(vertexCount * sizeof(SkPoint));)

    // Lay out the vertices in "middle-out" order:
    //
    // T= 0/1, 1/1,              ; resolveLevel=0
    //    1/2,                   ; resolveLevel=1  (0/2 and 2/2 are already in resolveLevel 0)
    //    1/4, 3/4,              ; resolveLevel=2  (2/4 is already in resolveLevel 1)
    //    1/8, 3/8, 5/8, 7/8,    ; resolveLevel=3  (2/8 and 6/8 are already in resolveLevel 2)
    //    ...                    ; resolveLevel=...
    //
    // Resolve level 0 is just the beginning and ending vertices.
    vertexWriter << (float)0/*resolveLevel*/ << (float)0/*idx*/;
    vertexWriter << (float)0/*resolveLevel*/ << (float)1/*idx*/;

    // Resolve levels 1..kMaxResolveLevel.
    int maxResolveLevel = SkPrevLog2(vertexCount - 1);
    SkASSERT((1 << maxResolveLevel) + 1 == vertexCount);
    for (int resolveLevel = 1; resolveLevel <= maxResolveLevel; ++resolveLevel) {
        int numSegmentsInResolveLevel = 1 << resolveLevel;
        // Write out the odd vertices in this resolveLevel. The even vertices were already written
        // out in previous resolveLevels and will be indexed from there.
        for (int i = 1; i < numSegmentsInResolveLevel; i += 2) {
            vertexWriter << (float)resolveLevel << (float)i;
        }
    }

    SkASSERT(vertexWriter == end);
}

void PathCurveTessellator::WriteFixedIndexBufferBaseIndex(VertexWriter vertexWriter,
                                                          size_t bufferSize,
                                                          uint16_t baseIndex) {
    SkASSERT(bufferSize % (sizeof(uint16_t) * 3) == 0);
    int triangleCount = bufferSize / (sizeof(uint16_t) * 3);
    SkASSERT(triangleCount >= 1);
    SkTArray<std::array<uint16_t, 3>> indexData(triangleCount);

    // Connect the vertices with a middle-out triangulation. Refer to InitFixedCountVertexBuffer()
    // for the exact vertex ordering.
    //
    // Resolve level 1 is just a single triangle at T=[0, 1/2, 1].
    const auto* neighborInLastResolveLevel = &indexData.push_back({baseIndex,
                                                                   (uint16_t)(baseIndex + 2),
                                                                   (uint16_t)(baseIndex + 1)});

    // Resolve levels 2..maxResolveLevel
    int maxResolveLevel = SkPrevLog2(triangleCount + 1);
    uint16_t nextIndex = baseIndex + 3;
    SkASSERT(NumCurveTrianglesAtResolveLevel(maxResolveLevel) == triangleCount);
    for (int resolveLevel = 2; resolveLevel <= maxResolveLevel; ++resolveLevel) {
        SkDEBUGCODE(auto* firstTriangleInCurrentResolveLevel = indexData.end());
        int numOuterTrianglelsInResolveLevel = 1 << (resolveLevel - 1);
        SkASSERT(numOuterTrianglelsInResolveLevel % 2 == 0);
        int numTrianglePairsInResolveLevel = numOuterTrianglelsInResolveLevel >> 1;
        for (int i = 0; i < numTrianglePairsInResolveLevel; ++i) {
            // First triangle shares the left edge of "neighborInLastResolveLevel".
            indexData.push_back({(*neighborInLastResolveLevel)[0],
                                 nextIndex++,
                                 (*neighborInLastResolveLevel)[1]});
            // Second triangle shares the right edge of "neighborInLastResolveLevel".
            indexData.push_back({(*neighborInLastResolveLevel)[1],
                                 nextIndex++,
                                 (*neighborInLastResolveLevel)[2]});
            ++neighborInLastResolveLevel;
        }
        SkASSERT(neighborInLastResolveLevel == firstTriangleInCurrentResolveLevel);
    }
    SkASSERT(indexData.count() == triangleCount);
    SkASSERT(nextIndex == baseIndex + triangleCount + 2);

    vertexWriter.writeArray(indexData.data(), indexData.count());
}

#if SK_GPU_V1

GR_DECLARE_STATIC_UNIQUE_KEY(gFixedVertexBufferKey);
GR_DECLARE_STATIC_UNIQUE_KEY(gFixedIndexBufferKey);

void PathCurveTessellator::prepareFixedCountBuffers(GrResourceProvider* rp) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gFixedVertexBufferKey);

    fFixedVertexBuffer = rp->findOrMakeStaticBuffer(GrGpuBufferType::kVertex,
                                                    FixedVertexBufferSize(kMaxFixedResolveLevel),
                                                    gFixedVertexBufferKey,
                                                    WriteFixedVertexBuffer);

    GR_DEFINE_STATIC_UNIQUE_KEY(gFixedIndexBufferKey);

    fFixedIndexBuffer = rp->findOrMakeStaticBuffer(GrGpuBufferType::kIndex,
                                                   FixedIndexBufferSize(kMaxFixedResolveLevel),
                                                   gFixedIndexBufferKey,
                                                   WriteFixedIndexBuffer);
}

void PathCurveTessellator::drawTessellated(GrOpFlushState* flushState) const {
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(nullptr, nullptr, chunk.fBuffer);
        flushState->draw(chunk.fCount * 4, chunk.fBase * 4);
    }
}

void PathCurveTessellator::drawFixedCount(GrOpFlushState* flushState) const {
    if (!fFixedVertexBuffer || !fFixedIndexBuffer) {
        return;
    }
    int fixedIndexCount = NumCurveTrianglesAtResolveLevel(fFixedResolveLevel) * 3;
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(fFixedIndexBuffer, chunk.fBuffer, fFixedVertexBuffer);
        flushState->drawIndexedInstanced(fixedIndexCount, 0, chunk.fCount, chunk.fBase, 0);
    }
}

void PathCurveTessellator::drawHullInstances(GrOpFlushState* flushState,
                                             sk_sp<const GrGpuBuffer> vertexBufferIfNeeded) const {
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(nullptr, chunk.fBuffer, vertexBufferIfNeeded);
        flushState->drawInstanced(chunk.fCount, chunk.fBase, 4, 0);
    }
}

#endif

}  // namespace skgpu
