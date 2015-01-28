/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAARectRenderer.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrGeometryProcessor.h"
#include "GrGpu.h"
#include "GrInvariantOutput.h"
#include "SkColorPriv.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

///////////////////////////////////////////////////////////////////////////////

namespace {
// Should the coverage be multiplied into the color attrib or use a separate attrib.
enum CoverageAttribType {
    kUseColor_CoverageAttribType,
    kUseCoverage_CoverageAttribType,
};
}

static const GrGeometryProcessor* create_rect_gp(const GrPipelineBuilder& pipelineBuilder,
                                                 GrColor color,
                                                 CoverageAttribType* type,
                                                 const SkMatrix& localMatrix) {
    uint32_t flags = GrDefaultGeoProcFactory::kColor_GPType;
    const GrGeometryProcessor* gp;
    if (pipelineBuilder.canTweakAlphaForCoverage()) {
        gp = GrDefaultGeoProcFactory::Create(flags, color, SkMatrix::I(), localMatrix);
        SkASSERT(gp->getVertexStride() == sizeof(GrDefaultGeoProcFactory::PositionColorAttr));
        *type = kUseColor_CoverageAttribType;
    } else {
        flags |= GrDefaultGeoProcFactory::kCoverage_GPType;
        gp = GrDefaultGeoProcFactory::Create(flags, color, SkMatrix::I(), localMatrix,
                                             GrColorIsOpaque(color));
        SkASSERT(gp->getVertexStride()==sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));
        *type = kUseCoverage_CoverageAttribType;
    }
    return gp;
}

static void set_inset_fan(SkPoint* pts, size_t stride,
                          const SkRect& r, SkScalar dx, SkScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy,
                    r.fRight - dx, r.fBottom - dy, stride);
}

void GrAARectRenderer::reset() {
    SkSafeSetNull(fAAFillRectIndexBuffer);
    SkSafeSetNull(fAAMiterStrokeRectIndexBuffer);
    SkSafeSetNull(fAABevelStrokeRectIndexBuffer);
}

static const uint16_t gFillAARectIdx[] = {
    0, 1, 5, 5, 4, 0,
    1, 2, 6, 6, 5, 1,
    2, 3, 7, 7, 6, 2,
    3, 0, 4, 4, 7, 3,
    4, 5, 6, 6, 7, 4,
};

static const int kIndicesPerAAFillRect = SK_ARRAY_COUNT(gFillAARectIdx);
static const int kVertsPerAAFillRect = 8;
static const int kNumAAFillRectsInIndexBuffer = 256;

static const uint16_t gMiterStrokeAARectIdx[] = {
    0 + 0, 1 + 0, 5 + 0, 5 + 0, 4 + 0, 0 + 0,
    1 + 0, 2 + 0, 6 + 0, 6 + 0, 5 + 0, 1 + 0,
    2 + 0, 3 + 0, 7 + 0, 7 + 0, 6 + 0, 2 + 0,
    3 + 0, 0 + 0, 4 + 0, 4 + 0, 7 + 0, 3 + 0,

    0 + 4, 1 + 4, 5 + 4, 5 + 4, 4 + 4, 0 + 4,
    1 + 4, 2 + 4, 6 + 4, 6 + 4, 5 + 4, 1 + 4,
    2 + 4, 3 + 4, 7 + 4, 7 + 4, 6 + 4, 2 + 4,
    3 + 4, 0 + 4, 4 + 4, 4 + 4, 7 + 4, 3 + 4,

    0 + 8, 1 + 8, 5 + 8, 5 + 8, 4 + 8, 0 + 8,
    1 + 8, 2 + 8, 6 + 8, 6 + 8, 5 + 8, 1 + 8,
    2 + 8, 3 + 8, 7 + 8, 7 + 8, 6 + 8, 2 + 8,
    3 + 8, 0 + 8, 4 + 8, 4 + 8, 7 + 8, 3 + 8,
};

static const int kIndicesPerMiterStrokeRect = SK_ARRAY_COUNT(gMiterStrokeAARectIdx);
static const int kVertsPerMiterStrokeRect = 16;
static const int kNumMiterStrokeRectsInIndexBuffer = 256;

/**
 * As in miter-stroke, index = a + b, and a is the current index, b is the shift
 * from the first index. The index layout:
 * outer AA line: 0~3, 4~7
 * outer edge:    8~11, 12~15
 * inner edge:    16~19
 * inner AA line: 20~23
 * Following comes a bevel-stroke rect and its indices:
 *
 *           4                                 7
 *            *********************************
 *          *   ______________________________  *
 *         *  / 12                          15 \  *
 *        *  /                                  \  *
 *     0 *  |8     16_____________________19  11 |  * 3
 *       *  |       |                    |       |  *
 *       *  |       |  ****************  |       |  *
 *       *  |       |  * 20        23 *  |       |  *
 *       *  |       |  *              *  |       |  *
 *       *  |       |  * 21        22 *  |       |  *
 *       *  |       |  ****************  |       |  *
 *       *  |       |____________________|       |  *
 *     1 *  |9    17                      18   10|  * 2
 *        *  \                                  /  *
 *         *  \13 __________________________14/  *
 *          *                                   *
 *           **********************************
 *          5                                  6
 */
static const uint16_t gBevelStrokeAARectIdx[] = {
    // Draw outer AA, from outer AA line to outer edge, shift is 0.
    0 + 0, 1 + 0, 9 + 0, 9 + 0, 8 + 0, 0 + 0,
    1 + 0, 5 + 0, 13 + 0, 13 + 0, 9 + 0, 1 + 0,
    5 + 0, 6 + 0, 14 + 0, 14 + 0, 13 + 0, 5 + 0,
    6 + 0, 2 + 0, 10 + 0, 10 + 0, 14 + 0, 6 + 0,
    2 + 0, 3 + 0, 11 + 0, 11 + 0, 10 + 0, 2 + 0,
    3 + 0, 7 + 0, 15 + 0, 15 + 0, 11 + 0, 3 + 0,
    7 + 0, 4 + 0, 12 + 0, 12 + 0, 15 + 0, 7 + 0,
    4 + 0, 0 + 0, 8 + 0, 8 + 0, 12 + 0, 4 + 0,

    // Draw the stroke, from outer edge to inner edge, shift is 8.
    0 + 8, 1 + 8, 9 + 8, 9 + 8, 8 + 8, 0 + 8,
    1 + 8, 5 + 8, 9 + 8,
    5 + 8, 6 + 8, 10 + 8, 10 + 8, 9 + 8, 5 + 8,
    6 + 8, 2 + 8, 10 + 8,
    2 + 8, 3 + 8, 11 + 8, 11 + 8, 10 + 8, 2 + 8,
    3 + 8, 7 + 8, 11 + 8,
    7 + 8, 4 + 8, 8 + 8, 8 + 8, 11 + 8, 7 + 8,
    4 + 8, 0 + 8, 8 + 8,

    // Draw the inner AA, from inner edge to inner AA line, shift is 16.
    0 + 16, 1 + 16, 5 + 16, 5 + 16, 4 + 16, 0 + 16,
    1 + 16, 2 + 16, 6 + 16, 6 + 16, 5 + 16, 1 + 16,
    2 + 16, 3 + 16, 7 + 16, 7 + 16, 6 + 16, 2 + 16,
    3 + 16, 0 + 16, 4 + 16, 4 + 16, 7 + 16, 3 + 16,
};

static const int kIndicesPerBevelStrokeRect = SK_ARRAY_COUNT(gBevelStrokeAARectIdx);
static const int kVertsPerBevelStrokeRect = 24;
static const int kNumBevelStrokeRectsInIndexBuffer = 256;

static int aa_stroke_rect_index_count(bool miterStroke) {
    return miterStroke ? SK_ARRAY_COUNT(gMiterStrokeAARectIdx) :
                         SK_ARRAY_COUNT(gBevelStrokeAARectIdx);
}

GrIndexBuffer* GrAARectRenderer::aaStrokeRectIndexBuffer(bool miterStroke) {
    if (miterStroke) {
        if (NULL == fAAMiterStrokeRectIndexBuffer) {
            fAAMiterStrokeRectIndexBuffer =
                    fGpu->createInstancedIndexBuffer(gMiterStrokeAARectIdx,
                                                     kIndicesPerMiterStrokeRect,
                                                     kNumMiterStrokeRectsInIndexBuffer,
                                                     kVertsPerMiterStrokeRect);
        }
        return fAAMiterStrokeRectIndexBuffer;
    } else {
        if (NULL == fAABevelStrokeRectIndexBuffer) {
            fAABevelStrokeRectIndexBuffer =
                    fGpu->createInstancedIndexBuffer(gBevelStrokeAARectIdx,
                                                     kIndicesPerBevelStrokeRect,
                                                     kNumBevelStrokeRectsInIndexBuffer,
                                                     kVertsPerBevelStrokeRect);
        }
        return fAABevelStrokeRectIndexBuffer;
    }
}

void GrAARectRenderer::geometryFillAARect(GrDrawTarget* target,
                                          GrPipelineBuilder* pipelineBuilder,
                                          GrColor color,
                                          const SkMatrix& viewMatrix,
                                          const SkRect& rect,
                                          const SkRect& devRect) {
    GrPipelineBuilder::AutoRestoreEffects are(pipelineBuilder);

    SkMatrix localMatrix;
    if (!viewMatrix.invert(&localMatrix)) {
        SkDebugf("Cannot invert\n");
        return;
    }

    CoverageAttribType type;
    SkAutoTUnref<const GrGeometryProcessor> gp(create_rect_gp(*pipelineBuilder, color, &type,
                                                              localMatrix));

    size_t vertexStride = gp->getVertexStride();
    GrDrawTarget::AutoReleaseGeometry geo(target, 8, vertexStride, 0);
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return;
    }

    if (NULL == fAAFillRectIndexBuffer) {
        fAAFillRectIndexBuffer = fGpu->createInstancedIndexBuffer(gFillAARectIdx,
                                                                  kIndicesPerAAFillRect,
                                                                  kNumAAFillRectsInIndexBuffer,
                                                                  kVertsPerAAFillRect);
    }
    GrIndexBuffer* indexBuffer = fAAFillRectIndexBuffer;
    if (NULL == indexBuffer) {
        SkDebugf("Failed to create index buffer!\n");
        return;
    }

    intptr_t verts = reinterpret_cast<intptr_t>(geo.vertices());

    SkPoint* fan0Pos = reinterpret_cast<SkPoint*>(verts);
    SkPoint* fan1Pos = reinterpret_cast<SkPoint*>(verts + 4 * vertexStride);

    SkScalar inset = SkMinScalar(devRect.width(), SK_Scalar1);
    inset = SK_ScalarHalf * SkMinScalar(inset, devRect.height());

    if (viewMatrix.rectStaysRect()) {
        // Temporarily #if'ed out. We don't want to pass in the devRect but
        // right now it is computed in GrContext::apply_aa_to_rect and we don't
        // want to throw away the work
#if 0
        SkRect devRect;
        combinedMatrix.mapRect(&devRect, rect);
#endif

        set_inset_fan(fan0Pos, vertexStride, devRect, -SK_ScalarHalf, -SK_ScalarHalf);
        set_inset_fan(fan1Pos, vertexStride, devRect, inset,  inset);
    } else {
        // compute transformed (1, 0) and (0, 1) vectors
        SkVector vec[2] = {
          { viewMatrix[SkMatrix::kMScaleX], viewMatrix[SkMatrix::kMSkewY] },
          { viewMatrix[SkMatrix::kMSkewX],  viewMatrix[SkMatrix::kMScaleY] }
        };

        vec[0].normalize();
        vec[0].scale(SK_ScalarHalf);
        vec[1].normalize();
        vec[1].scale(SK_ScalarHalf);

        // create the rotated rect
        fan0Pos->setRectFan(rect.fLeft, rect.fTop,
                            rect.fRight, rect.fBottom, vertexStride);
        viewMatrix.mapPointsWithStride(fan0Pos, vertexStride, 4);

        // Now create the inset points and then outset the original
        // rotated points

        // TL
        *((SkPoint*)((intptr_t)fan1Pos + 0 * vertexStride)) =
            *((SkPoint*)((intptr_t)fan0Pos + 0 * vertexStride)) + vec[0] + vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 0 * vertexStride)) -= vec[0] + vec[1];
        // BL
        *((SkPoint*)((intptr_t)fan1Pos + 1 * vertexStride)) =
            *((SkPoint*)((intptr_t)fan0Pos + 1 * vertexStride)) + vec[0] - vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 1 * vertexStride)) -= vec[0] - vec[1];
        // BR
        *((SkPoint*)((intptr_t)fan1Pos + 2 * vertexStride)) =
            *((SkPoint*)((intptr_t)fan0Pos + 2 * vertexStride)) - vec[0] - vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 2 * vertexStride)) += vec[0] + vec[1];
        // TR
        *((SkPoint*)((intptr_t)fan1Pos + 3 * vertexStride)) =
            *((SkPoint*)((intptr_t)fan0Pos + 3 * vertexStride)) - vec[0] + vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 3 * vertexStride)) += vec[0] - vec[1];
    }

    // Make verts point to vertex color and then set all the color and coverage vertex attrs values.
    verts += sizeof(SkPoint);
    for (int i = 0; i < 4; ++i) {
        if (kUseCoverage_CoverageAttribType == type) {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
            *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) = 0;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = 0;
        }
    }

    int scale;
    if (inset < SK_ScalarHalf) {
        scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
        SkASSERT(scale >= 0 && scale <= 255);
    } else {
        scale = 0xff;
    }

    verts += 4 * vertexStride;

    float innerCoverage = GrNormalizeByteToFloat(scale);
    GrColor scaledColor = (0xff == scale) ? color : SkAlphaMulQ(color, scale);

    for (int i = 0; i < 4; ++i) {
        if (kUseCoverage_CoverageAttribType == type) {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
            *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) = innerCoverage;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
        }
    }

    target->setIndexSourceToBuffer(indexBuffer);
    target->drawIndexedInstances(pipelineBuilder,
                                 gp,
                                 kTriangles_GrPrimitiveType,
                                 1,
                                 kVertsPerAAFillRect,
                                 kIndicesPerAAFillRect);
    target->resetIndexSource();
}

void GrAARectRenderer::strokeAARect(GrDrawTarget* target,
                                    GrPipelineBuilder* pipelineBuilder,
                                    GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const SkRect& devRect,
                                    const SkStrokeRec& stroke) {
    SkVector devStrokeSize;
    SkScalar width = stroke.getWidth();
    if (width > 0) {
        devStrokeSize.set(width, width);
        viewMatrix.mapVectors(&devStrokeSize, 1);
        devStrokeSize.setAbs(devStrokeSize);
    } else {
        devStrokeSize.set(SK_Scalar1, SK_Scalar1);
    }

    const SkScalar dx = devStrokeSize.fX;
    const SkScalar dy = devStrokeSize.fY;
    const SkScalar rx = SkScalarMul(dx, SK_ScalarHalf);
    const SkScalar ry = SkScalarMul(dy, SK_ScalarHalf);

    // Temporarily #if'ed out. We don't want to pass in the devRect but
    // right now it is computed in GrContext::apply_aa_to_rect and we don't
    // want to throw away the work
#if 0
    SkRect devRect;
    combinedMatrix.mapRect(&devRect, rect);
#endif

    SkScalar spare;
    {
        SkScalar w = devRect.width() - dx;
        SkScalar h = devRect.height() - dy;
        spare = SkTMin(w, h);
    }

    SkRect devOutside(devRect);
    devOutside.outset(rx, ry);

    bool miterStroke = true;
    // For hairlines, make bevel and round joins appear the same as mitered ones.
    // small miter limit means right angles show bevel...
    if ((width > 0) && (stroke.getJoin() != SkPaint::kMiter_Join ||
                        stroke.getMiter() < SK_ScalarSqrt2)) {
        miterStroke = false;
    }

    if (spare <= 0 && miterStroke) {
        this->fillAARect(target, pipelineBuilder, color, viewMatrix, devOutside,
                         devOutside);
        return;
    }

    SkRect devInside(devRect);
    devInside.inset(rx, ry);

    SkRect devOutsideAssist(devRect);

    // For bevel-stroke, use 2 SkRect instances(devOutside and devOutsideAssist)
    // to draw the outer of the rect. Because there are 8 vertices on the outer
    // edge, while vertex number of inner edge is 4, the same as miter-stroke.
    if (!miterStroke) {
        devOutside.inset(0, ry);
        devOutsideAssist.outset(0, ry);
    }

    this->geometryStrokeAARect(target, pipelineBuilder, color, viewMatrix, devOutside, devOutsideAssist,
                               devInside, miterStroke);
}

void GrAARectRenderer::geometryStrokeAARect(GrDrawTarget* target,
                                            GrPipelineBuilder* pipelineBuilder,
                                            GrColor color,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& devOutside,
                                            const SkRect& devOutsideAssist,
                                            const SkRect& devInside,
                                            bool miterStroke) {
    GrPipelineBuilder::AutoRestoreEffects are(pipelineBuilder);

    SkMatrix localMatrix;
    if (!viewMatrix.invert(&localMatrix)) {
        SkDebugf("Cannot invert\n");
        return;
    }

    CoverageAttribType type;
    SkAutoTUnref<const GrGeometryProcessor> gp(create_rect_gp(*pipelineBuilder, color, &type,
                                                              localMatrix));

    int innerVertexNum = 4;
    int outerVertexNum = miterStroke ? 4 : 8;
    int totalVertexNum = (outerVertexNum + innerVertexNum) * 2;

    size_t vstride = gp->getVertexStride();
    GrDrawTarget::AutoReleaseGeometry geo(target, totalVertexNum, vstride, 0);
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return;
    }
    GrIndexBuffer* indexBuffer = this->aaStrokeRectIndexBuffer(miterStroke);
    if (NULL == indexBuffer) {
        SkDebugf("Failed to create index buffer!\n");
        return;
    }

    intptr_t verts = reinterpret_cast<intptr_t>(geo.vertices());

    // We create vertices for four nested rectangles. There are two ramps from 0 to full
    // coverage, one on the exterior of the stroke and the other on the interior.
    // The following pointers refer to the four rects, from outermost to innermost.
    SkPoint* fan0Pos = reinterpret_cast<SkPoint*>(verts);
    SkPoint* fan1Pos = reinterpret_cast<SkPoint*>(verts + outerVertexNum * vstride);
    SkPoint* fan2Pos = reinterpret_cast<SkPoint*>(verts + 2 * outerVertexNum * vstride);
    SkPoint* fan3Pos = reinterpret_cast<SkPoint*>(verts + (2 * outerVertexNum + innerVertexNum) * vstride);

#ifndef SK_IGNORE_THIN_STROKED_RECT_FIX
    // TODO: this only really works if the X & Y margins are the same all around
    // the rect (or if they are all >= 1.0).
    SkScalar inset = SkMinScalar(SK_Scalar1, devOutside.fRight - devInside.fRight);
    inset = SkMinScalar(inset, devInside.fLeft - devOutside.fLeft);
    inset = SkMinScalar(inset, devInside.fTop - devOutside.fTop);
    if (miterStroke) {
        inset = SK_ScalarHalf * SkMinScalar(inset, devOutside.fBottom - devInside.fBottom);
    } else {
        inset = SK_ScalarHalf * SkMinScalar(inset, devOutsideAssist.fBottom - devInside.fBottom);
    }
    SkASSERT(inset >= 0);
#else
    SkScalar inset = SK_ScalarHalf;
#endif

    if (miterStroke) {
        // outermost
        set_inset_fan(fan0Pos, vstride, devOutside, -SK_ScalarHalf, -SK_ScalarHalf);
        // inner two
        set_inset_fan(fan1Pos, vstride, devOutside,  inset,  inset);
        set_inset_fan(fan2Pos, vstride, devInside,  -inset, -inset);
        // innermost
        set_inset_fan(fan3Pos, vstride, devInside,   SK_ScalarHalf,  SK_ScalarHalf);
    } else {
        SkPoint* fan0AssistPos = reinterpret_cast<SkPoint*>(verts + 4 * vstride);
        SkPoint* fan1AssistPos = reinterpret_cast<SkPoint*>(verts + (outerVertexNum + 4) * vstride);
        // outermost
        set_inset_fan(fan0Pos, vstride, devOutside, -SK_ScalarHalf, -SK_ScalarHalf);
        set_inset_fan(fan0AssistPos, vstride, devOutsideAssist, -SK_ScalarHalf, -SK_ScalarHalf);
        // outer one of the inner two
        set_inset_fan(fan1Pos, vstride, devOutside,  inset,  inset);
        set_inset_fan(fan1AssistPos, vstride, devOutsideAssist,  inset,  inset);
        // inner one of the inner two
        set_inset_fan(fan2Pos, vstride, devInside,  -inset, -inset);
        // innermost
        set_inset_fan(fan3Pos, vstride, devInside,   SK_ScalarHalf,  SK_ScalarHalf);
    }

    // Make verts point to vertex color and then set all the color and coverage vertex attrs values.
    // The outermost rect has 0 coverage
    verts += sizeof(SkPoint);
    for (int i = 0; i < outerVertexNum; ++i) {
        if (kUseCoverage_CoverageAttribType == type) {
            *reinterpret_cast<GrColor*>(verts + i * vstride) = color;
            *reinterpret_cast<float*>(verts + i * vstride + sizeof(GrColor)) = 0;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vstride) = 0;
        }
    }

    // scale is the coverage for the the inner two rects.
    int scale;
    if (inset < SK_ScalarHalf) {
        scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
        SkASSERT(scale >= 0 && scale <= 255);
    } else {
        scale = 0xff;
    }

    float innerCoverage = GrNormalizeByteToFloat(scale);
    GrColor scaledColor = (0xff == scale) ? color : SkAlphaMulQ(color, scale);

    verts += outerVertexNum * vstride;
    for (int i = 0; i < outerVertexNum + innerVertexNum; ++i) {
        if (kUseCoverage_CoverageAttribType == type) {
            *reinterpret_cast<GrColor*>(verts + i * vstride) = color;
            *reinterpret_cast<float*>(verts + i * vstride + sizeof(GrColor)) = innerCoverage;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vstride) = scaledColor;
        }
    }

    // The innermost rect has 0 coverage
    verts += (outerVertexNum + innerVertexNum) * vstride;
    for (int i = 0; i < innerVertexNum; ++i) {
        if (kUseCoverage_CoverageAttribType == type) {
            *reinterpret_cast<GrColor*>(verts + i * vstride) = color;
            *reinterpret_cast<GrColor*>(verts + i * vstride + sizeof(GrColor)) = 0;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vstride) = 0;
        }
    }

    target->setIndexSourceToBuffer(indexBuffer);
    target->drawIndexedInstances(pipelineBuilder,
                                 gp,
                                 kTriangles_GrPrimitiveType,
                                 1,
                                 totalVertexNum,
                                 aa_stroke_rect_index_count(miterStroke));
    target->resetIndexSource();
}

void GrAARectRenderer::fillAANestedRects(GrDrawTarget* target,
                                         GrPipelineBuilder* pipelineBuilder,
                                         GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkRect rects[2]) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(!rects[1].isEmpty());

    SkRect devOutside, devOutsideAssist, devInside;
    viewMatrix.mapRect(&devOutside, rects[0]);
    // can't call mapRect for devInside since it calls sort
    viewMatrix.mapPoints((SkPoint*)&devInside, (const SkPoint*)&rects[1], 2);

    if (devInside.isEmpty()) {
        this->fillAARect(target, pipelineBuilder, color, viewMatrix, devOutside,
                         devOutside);
        return;
    }

    this->geometryStrokeAARect(target, pipelineBuilder, color, viewMatrix, devOutside,
                               devOutsideAssist, devInside, true);
}
