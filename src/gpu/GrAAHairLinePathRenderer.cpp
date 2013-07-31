
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAHairLinePathRenderer.h"

#include "GrContext.h"
#include "GrDrawState.h"
#include "GrDrawTargetCaps.h"
#include "GrEffect.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrPathUtils.h"
#include "GrTBackendEffectFactory.h"
#include "SkGeometry.h"
#include "SkStroke.h"
#include "SkTemplates.h"

#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"

namespace {
// quadratics are rendered as 5-sided polys in order to bound the
// AA stroke around the center-curve. See comments in push_quad_index_buffer and
// bloat_quad. Quadratics and conics share an index buffer
static const int kVertsPerQuad = 5;
static const int kIdxsPerQuad = 9;

static const int kVertsPerLineSeg = 4;
static const int kIdxsPerLineSeg = 6;

static const int kNumQuadsInIdxBuffer = 256;
static const size_t kQuadIdxSBufize = kIdxsPerQuad *
                                      sizeof(uint16_t) *
                                      kNumQuadsInIdxBuffer;

bool push_quad_index_data(GrIndexBuffer* qIdxBuffer) {
    uint16_t* data = (uint16_t*) qIdxBuffer->lock();
    bool tempData = NULL == data;
    if (tempData) {
        data = SkNEW_ARRAY(uint16_t, kNumQuadsInIdxBuffer * kIdxsPerQuad);
    }
    for (int i = 0; i < kNumQuadsInIdxBuffer; ++i) {

        // Each quadratic is rendered as a five sided polygon. This poly bounds
        // the quadratic's bounding triangle but has been expanded so that the
        // 1-pixel wide area around the curve is inside the poly.
        // If a,b,c are the original control points then the poly a0,b0,c0,c1,a1
        // that is rendered would look like this:
        //              b0
        //              b
        //
        //     a0              c0
        //      a            c
        //       a1       c1
        // Each is drawn as three triangles specified by these 9 indices:
        int baseIdx = i * kIdxsPerQuad;
        uint16_t baseVert = (uint16_t)(i * kVertsPerQuad);
        data[0 + baseIdx] = baseVert + 0; // a0
        data[1 + baseIdx] = baseVert + 1; // a1
        data[2 + baseIdx] = baseVert + 2; // b0
        data[3 + baseIdx] = baseVert + 2; // b0
        data[4 + baseIdx] = baseVert + 4; // c1
        data[5 + baseIdx] = baseVert + 3; // c0
        data[6 + baseIdx] = baseVert + 1; // a1
        data[7 + baseIdx] = baseVert + 4; // c1
        data[8 + baseIdx] = baseVert + 2; // b0
    }
    if (tempData) {
        bool ret = qIdxBuffer->updateData(data, kQuadIdxSBufize);
        delete[] data;
        return ret;
    } else {
        qIdxBuffer->unlock();
        return true;
    }
}
}

GrPathRenderer* GrAAHairLinePathRenderer::Create(GrContext* context) {
    const GrIndexBuffer* lIdxBuffer = context->getQuadIndexBuffer();
    if (NULL == lIdxBuffer) {
        return NULL;
    }
    GrGpu* gpu = context->getGpu();
    GrIndexBuffer* qIdxBuf = gpu->createIndexBuffer(kQuadIdxSBufize, false);
    SkAutoTUnref<GrIndexBuffer> qIdxBuffer(qIdxBuf);
    if (NULL == qIdxBuf ||
        !push_quad_index_data(qIdxBuf)) {
        return NULL;
    }
    return SkNEW_ARGS(GrAAHairLinePathRenderer,
                      (context, lIdxBuffer, qIdxBuf));
}

GrAAHairLinePathRenderer::GrAAHairLinePathRenderer(
                                        const GrContext* context,
                                        const GrIndexBuffer* linesIndexBuffer,
                                        const GrIndexBuffer* quadsIndexBuffer) {
    fLinesIndexBuffer = linesIndexBuffer;
    linesIndexBuffer->ref();
    fQuadsIndexBuffer = quadsIndexBuffer;
    quadsIndexBuffer->ref();
}

GrAAHairLinePathRenderer::~GrAAHairLinePathRenderer() {
    fLinesIndexBuffer->unref();
    fQuadsIndexBuffer->unref();
}

namespace {

typedef SkTArray<SkPoint, true> PtArray;
#define PREALLOC_PTARRAY(N) SkSTArray<(N),SkPoint, true>
typedef SkTArray<int, true> IntArray;
typedef SkTArray<float, true> FloatArray;

// Takes 178th time of logf on Z600 / VC2010
int get_float_exp(float x) {
    GR_STATIC_ASSERT(sizeof(int) == sizeof(float));
#if GR_DEBUG
    static bool tested;
    if (!tested) {
        tested = true;
        GrAssert(get_float_exp(0.25f) == -2);
        GrAssert(get_float_exp(0.3f) == -2);
        GrAssert(get_float_exp(0.5f) == -1);
        GrAssert(get_float_exp(1.f) == 0);
        GrAssert(get_float_exp(2.f) == 1);
        GrAssert(get_float_exp(2.5f) == 1);
        GrAssert(get_float_exp(8.f) == 3);
        GrAssert(get_float_exp(100.f) == 6);
        GrAssert(get_float_exp(1000.f) == 9);
        GrAssert(get_float_exp(1024.f) == 10);
        GrAssert(get_float_exp(3000000.f) == 21);
    }
#endif
    const int* iptr = (const int*)&x;
    return (((*iptr) & 0x7f800000) >> 23) - 127;
}

// Uses the max curvature function for quads to estimate
// where to chop the conic. If the max curvature is not
// found along the curve segment it will return 1 and
// dst[0] is the orginal conic. If it returns 2 the dst[0]
// and dst[1] are the two new conics.
int chop_conic(const SkPoint src[3], SkConic dst[2], const SkScalar weight) {
    SkScalar t = SkFindQuadMaxCurvature(src);
    if (t == 0) {
        if (dst) {
            dst[0].set(src, weight);
        }
        return 1;
    } else {
        if (dst) {
            SkConic conic;
            conic.set(src, weight);
            conic.chopAt(t, dst);
        }
        return 2;
    }
}

// returns 0 if quad/conic is degen or close to it
// in this case approx the path with lines
// otherwise returns 1
int is_degen_quad_or_conic(const SkPoint p[3]) {
    static const SkScalar gDegenerateToLineTol = SK_Scalar1;
    static const SkScalar gDegenerateToLineTolSqd =
        SkScalarMul(gDegenerateToLineTol, gDegenerateToLineTol);

    if (p[0].distanceToSqd(p[1]) < gDegenerateToLineTolSqd ||
        p[1].distanceToSqd(p[2]) < gDegenerateToLineTolSqd) {
        return 1;
    }

    SkScalar dsqd = p[1].distanceToLineBetweenSqd(p[0], p[2]);
    if (dsqd < gDegenerateToLineTolSqd) {
        return 1;
    }

    if (p[2].distanceToLineBetweenSqd(p[1], p[0]) < gDegenerateToLineTolSqd) {
        return 1;
    }
    return 0;
}

// we subdivide the quads to avoid huge overfill
// if it returns -1 then should be drawn as lines
int num_quad_subdivs(const SkPoint p[3]) {
    static const SkScalar gDegenerateToLineTol = SK_Scalar1;
    static const SkScalar gDegenerateToLineTolSqd =
        SkScalarMul(gDegenerateToLineTol, gDegenerateToLineTol);

    if (p[0].distanceToSqd(p[1]) < gDegenerateToLineTolSqd ||
        p[1].distanceToSqd(p[2]) < gDegenerateToLineTolSqd) {
        return -1;
    }

    SkScalar dsqd = p[1].distanceToLineBetweenSqd(p[0], p[2]);
    if (dsqd < gDegenerateToLineTolSqd) {
        return -1;
    }

    if (p[2].distanceToLineBetweenSqd(p[1], p[0]) < gDegenerateToLineTolSqd) {
        return -1;
    }

    // tolerance of triangle height in pixels
    // tuned on windows  Quadro FX 380 / Z600
    // trade off of fill vs cpu time on verts
    // maybe different when do this using gpu (geo or tess shaders)
    static const SkScalar gSubdivTol = 175 * SK_Scalar1;

    if (dsqd <= SkScalarMul(gSubdivTol, gSubdivTol)) {
        return 0;
    } else {
        static const int kMaxSub = 4;
        // subdividing the quad reduces d by 4. so we want x = log4(d/tol)
        // = log4(d*d/tol*tol)/2
        // = log2(d*d/tol*tol)

#ifdef SK_SCALAR_IS_FLOAT
        // +1 since we're ignoring the mantissa contribution.
        int log = get_float_exp(dsqd/(gSubdivTol*gSubdivTol)) + 1;
        log = GrMin(GrMax(0, log), kMaxSub);
        return log;
#else
        SkScalar log = SkScalarLog(
                          SkScalarDiv(dsqd,
                                      SkScalarMul(gSubdivTol, gSubdivTol)));
        static const SkScalar conv = SkScalarInvert(SkScalarLog(2));
        log = SkScalarMul(log, conv);
        return  GrMin(GrMax(0, SkScalarCeilToInt(log)),kMaxSub);
#endif
    }
}

/**
 * Generates the lines and quads to be rendered. Lines are always recorded in
 * device space. We will do a device space bloat to account for the 1pixel
 * thickness.
 * Quads are recorded in device space unless m contains
 * perspective, then in they are in src space. We do this because we will
 * subdivide large quads to reduce over-fill. This subdivision has to be
 * performed before applying the perspective matrix.
 */
int generate_lines_and_quads(const SkPath& path,
                             const SkMatrix& m,
                             const SkIRect& devClipBounds,
                             PtArray* lines,
                             PtArray* quads,
                             PtArray* conics,
                             IntArray* quadSubdivCnts,
                             FloatArray* conicWeights) {
    SkPath::Iter iter(path, false);

    int totalQuadCount = 0;
    SkRect bounds;
    SkIRect ibounds;

    bool persp = m.hasPerspective();

    for (;;) {
        GrPoint pathPts[4];
        GrPoint devPts[4];
        SkPath::Verb verb = iter.next(pathPts);
        switch (verb) {
            case SkPath::kConic_Verb: {
                SkConic dst[2];
                int conicCnt = chop_conic(pathPts, dst, iter.conicWeight());
                for (int i = 0; i < conicCnt; ++i) {
                    SkPoint* chopPnts = dst[i].fPts;
                    m.mapPoints(devPts, chopPnts, 3);
                    bounds.setBounds(devPts, 3);
                    bounds.outset(SK_Scalar1, SK_Scalar1);
                    bounds.roundOut(&ibounds);
                    if (SkIRect::Intersects(devClipBounds, ibounds)) {
                        if (is_degen_quad_or_conic(devPts)) {
                            SkPoint* pts = lines->push_back_n(4);
                            pts[0] = devPts[0];
                            pts[1] = devPts[1];
                            pts[2] = devPts[1];
                            pts[3] = devPts[2];
                        } else {
                            // when in perspective keep conics in src space
                            SkPoint* cPts = persp ? chopPnts : devPts;
                            SkPoint* pts = conics->push_back_n(3);
                            pts[0] = cPts[0];
                            pts[1] = cPts[1];
                            pts[2] = cPts[2];
                            conicWeights->push_back() = dst[i].fW;
                        }
                    }
                }
                break;
            }
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb:
                m.mapPoints(devPts, pathPts, 2);
                bounds.setBounds(devPts, 2);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(devClipBounds, ibounds)) {
                    SkPoint* pts = lines->push_back_n(2);
                    pts[0] = devPts[0];
                    pts[1] = devPts[1];
                }
                break;
            case SkPath::kQuad_Verb: {
                SkPoint choppedPts[5];
                // Chopping the quad helps when the quad is either degenerate or nearly degenerate.
                // When it is degenerate it allows the approximation with lines to work since the
                // chop point (if there is one) will be at the parabola's vertex. In the nearly
                // degenerate the QuadUVMatrix computed for the points is almost singular which
                // can cause rendering artifacts.
                int n = SkChopQuadAtMaxCurvature(pathPts, choppedPts);
                for (int i = 0; i < n; ++i) {
                    SkPoint* quadPts = choppedPts + i * 2;
                    m.mapPoints(devPts, quadPts, 3);
                    bounds.setBounds(devPts, 3);
                    bounds.outset(SK_Scalar1, SK_Scalar1);
                    bounds.roundOut(&ibounds);

                    if (SkIRect::Intersects(devClipBounds, ibounds)) {
                        int subdiv = num_quad_subdivs(devPts);
                        GrAssert(subdiv >= -1);
                        if (-1 == subdiv) {
                            SkPoint* pts = lines->push_back_n(4);
                            pts[0] = devPts[0];
                            pts[1] = devPts[1];
                            pts[2] = devPts[1];
                            pts[3] = devPts[2];
                        } else {
                            // when in perspective keep quads in src space
                            SkPoint* qPts = persp ? quadPts : devPts;
                            SkPoint* pts = quads->push_back_n(3);
                            pts[0] = qPts[0];
                            pts[1] = qPts[1];
                            pts[2] = qPts[2];
                            quadSubdivCnts->push_back() = subdiv;
                            totalQuadCount += 1 << subdiv;
                        }
                    }
                }
                break;
            }
            case SkPath::kCubic_Verb:
                m.mapPoints(devPts, pathPts, 4);
                bounds.setBounds(devPts, 4);
                bounds.outset(SK_Scalar1, SK_Scalar1);
                bounds.roundOut(&ibounds);
                if (SkIRect::Intersects(devClipBounds, ibounds)) {
                    PREALLOC_PTARRAY(32) q;
                    // we don't need a direction if we aren't constraining the subdivision
                    static const SkPath::Direction kDummyDir = SkPath::kCCW_Direction;
                    // We convert cubics to quadratics (for now).
                    // In perspective have to do conversion in src space.
                    if (persp) {
                        SkScalar tolScale =
                            GrPathUtils::scaleToleranceToSrc(SK_Scalar1, m,
                                                             path.getBounds());
                        GrPathUtils::convertCubicToQuads(pathPts, tolScale, false, kDummyDir, &q);
                    } else {
                        GrPathUtils::convertCubicToQuads(devPts, SK_Scalar1, false, kDummyDir, &q);
                    }
                    for (int i = 0; i < q.count(); i += 3) {
                        SkPoint* qInDevSpace;
                        // bounds has to be calculated in device space, but q is
                        // in src space when there is perspective.
                        if (persp) {
                            m.mapPoints(devPts, &q[i], 3);
                            bounds.setBounds(devPts, 3);
                            qInDevSpace = devPts;
                        } else {
                            bounds.setBounds(&q[i], 3);
                            qInDevSpace = &q[i];
                        }
                        bounds.outset(SK_Scalar1, SK_Scalar1);
                        bounds.roundOut(&ibounds);
                        if (SkIRect::Intersects(devClipBounds, ibounds)) {
                            int subdiv = num_quad_subdivs(qInDevSpace);
                            GrAssert(subdiv >= -1);
                            if (-1 == subdiv) {
                                SkPoint* pts = lines->push_back_n(4);
                                // lines should always be in device coords
                                pts[0] = qInDevSpace[0];
                                pts[1] = qInDevSpace[1];
                                pts[2] = qInDevSpace[1];
                                pts[3] = qInDevSpace[2];
                            } else {
                                SkPoint* pts = quads->push_back_n(3);
                                // q is already in src space when there is no
                                // perspective and dev coords otherwise.
                                pts[0] = q[0 + i];
                                pts[1] = q[1 + i];
                                pts[2] = q[2 + i];
                                quadSubdivCnts->push_back() = subdiv;
                                totalQuadCount += 1 << subdiv;
                            }
                        }
                    }
                }
                break;
            case SkPath::kClose_Verb:
                break;
            case SkPath::kDone_Verb:
                return totalQuadCount;
        }
    }
}

struct Vertex {
    GrPoint fPos;
    union {
        struct {
            SkScalar fA;
            SkScalar fB;
            SkScalar fC;
        } fLine;
        struct {
            SkScalar fA;
            SkScalar fB;
            SkScalar fC;
            SkScalar fD;
            SkScalar fE;
            SkScalar fF;
        } fConic;
        GrVec   fQuadCoord;
        struct {
            SkScalar fBogus[6];
        };
    };
};

GR_STATIC_ASSERT(sizeof(Vertex) == 4 * sizeof(GrPoint));

void intersect_lines(const SkPoint& ptA, const SkVector& normA,
                     const SkPoint& ptB, const SkVector& normB,
                     SkPoint* result) {

    SkScalar lineAW = -normA.dot(ptA);
    SkScalar lineBW = -normB.dot(ptB);

    SkScalar wInv = SkScalarMul(normA.fX, normB.fY) -
        SkScalarMul(normA.fY, normB.fX);
    wInv = SkScalarInvert(wInv);

    result->fX = SkScalarMul(normA.fY, lineBW) - SkScalarMul(lineAW, normB.fY);
    result->fX = SkScalarMul(result->fX, wInv);

    result->fY = SkScalarMul(lineAW, normB.fX) - SkScalarMul(normA.fX, lineBW);
    result->fY = SkScalarMul(result->fY, wInv);
}

void bloat_quad(const SkPoint qpts[3], const SkMatrix* toDevice,
                const SkMatrix* toSrc, Vertex verts[kVertsPerQuad],
                SkRect* devBounds) {
    GrAssert(!toDevice == !toSrc);
    // original quad is specified by tri a,b,c
    SkPoint a = qpts[0];
    SkPoint b = qpts[1];
    SkPoint c = qpts[2];

    // this should be in the src space, not dev coords, when we have perspective
    GrPathUtils::QuadUVMatrix DevToUV(qpts);

    if (toDevice) {
        toDevice->mapPoints(&a, 1);
        toDevice->mapPoints(&b, 1);
        toDevice->mapPoints(&c, 1);
    }
    // make a new poly where we replace a and c by a 1-pixel wide edges orthog
    // to edges ab and bc:
    //
    //   before       |        after
    //                |              b0
    //         b      |
    //                |
    //                |     a0            c0
    // a         c    |        a1       c1
    //
    // edges a0->b0 and b0->c0 are parallel to original edges a->b and b->c,
    // respectively.
    Vertex& a0 = verts[0];
    Vertex& a1 = verts[1];
    Vertex& b0 = verts[2];
    Vertex& c0 = verts[3];
    Vertex& c1 = verts[4];

    SkVector ab = b;
    ab -= a;
    SkVector ac = c;
    ac -= a;
    SkVector cb = b;
    cb -= c;

    // We should have already handled degenerates
    GrAssert(ab.length() > 0 && cb.length() > 0);

    ab.normalize();
    SkVector abN;
    abN.setOrthog(ab, SkVector::kLeft_Side);
    if (abN.dot(ac) > 0) {
        abN.negate();
    }

    cb.normalize();
    SkVector cbN;
    cbN.setOrthog(cb, SkVector::kLeft_Side);
    if (cbN.dot(ac) < 0) {
        cbN.negate();
    }

    a0.fPos = a;
    a0.fPos += abN;
    a1.fPos = a;
    a1.fPos -= abN;

    c0.fPos = c;
    c0.fPos += cbN;
    c1.fPos = c;
    c1.fPos -= cbN;

    // This point may not be within 1 pixel of a control point. We update the bounding box to
    // include it.
    intersect_lines(a0.fPos, abN, c0.fPos, cbN, &b0.fPos);
    devBounds->growToInclude(b0.fPos.fX, b0.fPos.fY);

    if (toSrc) {
        toSrc->mapPointsWithStride(&verts[0].fPos, sizeof(Vertex), kVertsPerQuad);
    }
    DevToUV.apply<kVertsPerQuad, sizeof(Vertex), sizeof(GrPoint)>(verts);
}


// Input Parametric:
// P(t) = (P0*(1-t)^2 + 2*w*P1*t*(1-t) + P2*t^2) / (1-t)^2 + 2*w*t*(1-t) + t^2)
// Output Implicit:
// Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0
// A = 4w^2*(y0-y1)(y1-y2)-(y0-y2)^2
// B = 4w^2*((x0-x1)(y2-y1)+(x1-x2)(y1-y0)) + 2(x0-x2)(y0-y2)
// C = 4w^2(x0-x1)(x1-x2) - (x0-x2)^2
// D = 4w^2((x0y1-x1y0)(y1-y2)+(x1y2-x2y1)(y0-y1)) + 2(y2-y0)(x0y2-x2y0)
// E = 4w^2((y0x1-y1x0)(x1-x2)+(y1x2-y2x1)(x0-x1)) + 2(x2-x0)(y0x2-y2x0)
// F = 4w^2(x1y2-x2y1)(x0y1-x1y0) - (x2y0-x0y2)^2

void set_conic_coeffs(const SkPoint p[3], Vertex verts[kVertsPerQuad], const float weight) {
    const float ww4 = 4 * weight * weight;
    const float x0Mx1 = p[0].fX - p[1].fX;
    const float x1Mx2 = p[1].fX - p[2].fX;
    const float x0Mx2 = p[0].fX - p[2].fX;
    const float y0My1 = p[0].fY - p[1].fY;
    const float y1My2 = p[1].fY - p[2].fY;
    const float y0My2 = p[0].fY - p[2].fY;
    const float x0y1Mx1y0 = p[0].fX*p[1].fY - p[1].fX*p[0].fY;
    const float x1y2Mx2y1 = p[1].fX*p[2].fY - p[2].fX*p[1].fY;
    const float x0y2Mx2y0 = p[0].fX*p[2].fY - p[2].fX*p[0].fY;
    const float a = ww4 * y0My1 * y1My2 - y0My2 * y0My2;
    const float b = -ww4 * (x0Mx1 * y1My2 + x1Mx2 * y0My1) + 2 * x0Mx2 * y0My2;
    const float c = ww4 * x0Mx1 * x1Mx2 - x0Mx2 * x0Mx2;
    const float d = ww4 * (x0y1Mx1y0 * y1My2 + x1y2Mx2y1 * y0My1) - 2 * y0My2 * x0y2Mx2y0;
    const float e = -ww4 * (x0y1Mx1y0 * x1Mx2 + x1y2Mx2y1 * x0Mx1) + 2 * x0Mx2 * x0y2Mx2y0;
    const float f = ww4 * x1y2Mx2y1 * x0y1Mx1y0 - x0y2Mx2y0 * x0y2Mx2y0;

    for (int i = 0; i < kVertsPerQuad; ++i) {
        verts[i].fConic.fA = a/f;
        verts[i].fConic.fB = b/f;
        verts[i].fConic.fC = c/f;
        verts[i].fConic.fD = d/f;
        verts[i].fConic.fE = e/f;
        verts[i].fConic.fF = f/f;
    }
}

void add_conics(const SkPoint p[3],
                float weight,
                const SkMatrix* toDevice,
                const SkMatrix* toSrc,
                Vertex** vert,
                SkRect* devBounds) {
    bloat_quad(p, toDevice, toSrc, *vert, devBounds);
    set_conic_coeffs(p, *vert, weight);
    *vert += kVertsPerQuad;
}

void add_quads(const SkPoint p[3],
               int subdiv,
               const SkMatrix* toDevice,
               const SkMatrix* toSrc,
               Vertex** vert,
               SkRect* devBounds) {
    GrAssert(subdiv >= 0);
    if (subdiv) {
        SkPoint newP[5];
        SkChopQuadAtHalf(p, newP);
        add_quads(newP + 0, subdiv-1, toDevice, toSrc, vert, devBounds);
        add_quads(newP + 2, subdiv-1, toDevice, toSrc, vert, devBounds);
    } else {
        bloat_quad(p, toDevice, toSrc, *vert, devBounds);
        *vert += kVertsPerQuad;
    }
}

void add_line(const SkPoint p[2],
              int rtHeight,
              const SkMatrix* toSrc,
              Vertex** vert) {
    const SkPoint& a = p[0];
    const SkPoint& b = p[1];

    SkVector orthVec = b;
    orthVec -= a;

    if (orthVec.setLength(SK_Scalar1)) {
        orthVec.setOrthog(orthVec);

        SkScalar lineC = -(a.dot(orthVec));
        for (int i = 0; i < kVertsPerLineSeg; ++i) {
            (*vert)[i].fPos = (i < 2) ? a : b;
            if (0 == i || 3 == i) {
                (*vert)[i].fPos -= orthVec;
            } else {
                (*vert)[i].fPos += orthVec;
            }
            (*vert)[i].fLine.fA = orthVec.fX;
            (*vert)[i].fLine.fB = orthVec.fY;
            (*vert)[i].fLine.fC = lineC;
        }
        if (NULL != toSrc) {
            toSrc->mapPointsWithStride(&(*vert)->fPos,
                                       sizeof(Vertex),
                                       kVertsPerLineSeg);
        }
    } else {
        // just make it degenerate and likely offscreen
        (*vert)[0].fPos.set(SK_ScalarMax, SK_ScalarMax);
        (*vert)[1].fPos.set(SK_ScalarMax, SK_ScalarMax);
        (*vert)[2].fPos.set(SK_ScalarMax, SK_ScalarMax);
        (*vert)[3].fPos.set(SK_ScalarMax, SK_ScalarMax);
    }

    *vert += kVertsPerLineSeg;
}

}

/**
 * The output of this effect is a hairline edge for conics.
 * Conics specified by implicit equation Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0.
 * A, B, C, D are the first vec4 of vertex attributes and
 * E and F are the vec2 attached to 2nd vertex attrribute.
 * Coverage is max(0, 1-distance).
 */
class HairConicEdgeEffect : public GrEffect {
public:
    static GrEffectRef* Create() {
        GR_CREATE_STATIC_EFFECT(gHairConicEdgeEffect, HairConicEdgeEffect, ());
        gHairConicEdgeEffect->ref();
        return gHairConicEdgeEffect;
    }

    virtual ~HairConicEdgeEffect() {}

    static const char* Name() { return "HairConicEdge"; }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        *validFlags = 0;
    }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<HairConicEdgeEffect>::getInstance();
    }

    class GLEffect : public GrGLEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
            : INHERITED (factory) {}

        virtual void emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            const char *vsCoeffABCDName, *fsCoeffABCDName;
            const char *vsCoeffEFName, *fsCoeffEFName;

            SkAssertResult(builder->enableFeature(
                    GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
            builder->addVarying(kVec4f_GrSLType, "ConicCoeffsABCD",
                                &vsCoeffABCDName, &fsCoeffABCDName);
            const SkString* attr0Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsCoeffABCDName, attr0Name->c_str());

            builder->addVarying(kVec2f_GrSLType, "ConicCoeffsEF",
                                &vsCoeffEFName, &fsCoeffEFName);
            const SkString* attr1Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[1]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsCoeffEFName, attr1Name->c_str());

            // Based on Gustavson 2006: "Beyond the Pixel: towards infinite resolution textures"
            builder->fsCodeAppendf("\t\tfloat edgeAlpha;\n");

            builder->fsCodeAppendf("\t\tvec3 uv1 = vec3(%s.xy, 1);\n", builder->fragmentPosition());
            builder->fsCodeAppend("\t\tvec3 u2uvv2 = uv1.xxy * uv1.xyy;\n");
            builder->fsCodeAppendf("\t\tvec3 ABC = %s.xyz;\n", fsCoeffABCDName);
            builder->fsCodeAppendf("\t\tvec3 DEF = vec3(%s.w, %s.xy);\n",
                                   fsCoeffABCDName, fsCoeffEFName);

            builder->fsCodeAppend("\t\tfloat dfdx = dot(uv1,vec3(2.0*ABC.x,ABC.y,DEF.x));\n");
            builder->fsCodeAppend("\t\tfloat dfdy = dot(uv1,vec3(ABC.y, 2.0*ABC.z,DEF.y));\n");
            builder->fsCodeAppend("\t\tfloat gF = dfdx*dfdx + dfdy*dfdy;\n");
            builder->fsCodeAppend("\t\tedgeAlpha = dot(ABC,u2uvv2) + dot(DEF,uv1);\n");
            builder->fsCodeAppend("\t\tedgeAlpha = sqrt(edgeAlpha*edgeAlpha / gF);\n");
            builder->fsCodeAppend("\t\tedgeAlpha = max((1.0 - edgeAlpha), 0.0);\n");
            // Add line below for smooth cubic ramp
            // builder->fsCodeAppend("\t\tedgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);\n");

            SkString modulate;
            GrGLSLModulatef<4>(&modulate, inputColor, "edgeAlpha");
            builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            return 0x0;
        }

        virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

    private:
        typedef GrGLEffect INHERITED;
    };

private:
    HairConicEdgeEffect() {
        this->addVertexAttrib(kVec4f_GrSLType);
        this->addVertexAttrib(kVec2f_GrSLType);
        this->setWillReadFragmentPosition();
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        return true;
    }

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

GR_DEFINE_EFFECT_TEST(HairConicEdgeEffect);

GrEffectRef* HairConicEdgeEffect::TestCreate(SkMWCRandom* random,
                                             GrContext*,
                                             const GrDrawTargetCaps& caps,
                                             GrTexture*[]) {
    return HairConicEdgeEffect::Create();
}
///////////////////////////////////////////////////////////////////////////////

/**
 * The output of this effect is a hairline edge for quadratics.
 * Quadratic specified by 0=u^2-v canonical coords. u and v are the first
 * two components of the vertex attribute. Uses unsigned distance.
 * Coverage is min(0, 1-distance). 3rd & 4th component unused.
 * Requires shader derivative instruction support.
 */
class HairQuadEdgeEffect : public GrEffect {
public:

    static GrEffectRef* Create() {
        GR_CREATE_STATIC_EFFECT(gHairQuadEdgeEffect, HairQuadEdgeEffect, ());
        gHairQuadEdgeEffect->ref();
        return gHairQuadEdgeEffect;
    }

    virtual ~HairQuadEdgeEffect() {}

    static const char* Name() { return "HairQuadEdge"; }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        *validFlags = 0;
    }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<HairQuadEdgeEffect>::getInstance();
    }

    class GLEffect : public GrGLEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
            : INHERITED (factory) {}

        virtual void emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            const char *vsName, *fsName;
            const SkString* attrName =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            builder->fsCodeAppendf("\t\tfloat edgeAlpha;\n");

            SkAssertResult(builder->enableFeature(
                    GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
            builder->addVarying(kVec4f_GrSLType, "HairQuadEdge", &vsName, &fsName);

            builder->fsCodeAppendf("\t\tvec2 duvdx = dFdx(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\t\tvec2 duvdy = dFdy(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\t\tvec2 gF = vec2(2.0*%s.x*duvdx.x - duvdx.y,\n"
                                   "\t\t               2.0*%s.x*duvdy.x - duvdy.y);\n",
                                   fsName, fsName);
            builder->fsCodeAppendf("\t\tedgeAlpha = (%s.x*%s.x - %s.y);\n", fsName, fsName,
                                   fsName);
            builder->fsCodeAppend("\t\tedgeAlpha = sqrt(edgeAlpha*edgeAlpha / dot(gF, gF));\n");
            builder->fsCodeAppend("\t\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");

            SkString modulate;
            GrGLSLModulatef<4>(&modulate, inputColor, "edgeAlpha");
            builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());

            builder->vsCodeAppendf("\t%s = %s;\n", vsName, attrName->c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            return 0x0;
        }

        virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

    private:
        typedef GrGLEffect INHERITED;
    };

private:
    HairQuadEdgeEffect() {
        this->addVertexAttrib(kVec4f_GrSLType);
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        return true;
    }

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

GR_DEFINE_EFFECT_TEST(HairQuadEdgeEffect);

GrEffectRef* HairQuadEdgeEffect::TestCreate(SkMWCRandom* random,
                                            GrContext*,
                                            const GrDrawTargetCaps& caps,
                                            GrTexture*[]) {
    // Doesn't work without derivative instructions.
    return caps.shaderDerivativeSupport() ? HairQuadEdgeEffect::Create() : NULL;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * The output of this effect is a 1-pixel wide line.
 * Input is 2D implicit device coord line eq (a*x + b*y +c = 0). 4th component unused.
 */
class HairLineEdgeEffect : public GrEffect {
public:

    static GrEffectRef* Create() {
        GR_CREATE_STATIC_EFFECT(gHairLineEdge, HairLineEdgeEffect, ());
        gHairLineEdge->ref();
        return gHairLineEdge;
    }

    virtual ~HairLineEdgeEffect() {}

    static const char* Name() { return "HairLineEdge"; }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        *validFlags = 0;
    }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<HairLineEdgeEffect>::getInstance();
    }

    class GLEffect : public GrGLEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
            : INHERITED (factory) {}

        virtual void emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            const char *vsName, *fsName;
            const SkString* attrName =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            builder->fsCodeAppendf("\t\tfloat edgeAlpha;\n");

            builder->addVarying(kVec4f_GrSLType, "HairLineEdge", &vsName, &fsName);

            builder->fsCodeAppendf("\t\tedgeAlpha = abs(dot(vec3(%s.xy,1), %s.xyz));\n",
                                   builder->fragmentPosition(), fsName);
            builder->fsCodeAppendf("\t\tedgeAlpha = max(1.0 - edgeAlpha, 0.0);\n");

            SkString modulate;
            GrGLSLModulatef<4>(&modulate, inputColor, "edgeAlpha");
            builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());

            builder->vsCodeAppendf("\t%s = %s;\n", vsName, attrName->c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            return 0x0;
        }

        virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

    private:
        typedef GrGLEffect INHERITED;
    };

private:
    HairLineEdgeEffect() {
        this->addVertexAttrib(kVec4f_GrSLType);
        this->setWillReadFragmentPosition();
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        return true;
    }

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

GR_DEFINE_EFFECT_TEST(HairLineEdgeEffect);

GrEffectRef* HairLineEdgeEffect::TestCreate(SkMWCRandom* random,
                                            GrContext*,
                                            const GrDrawTargetCaps& caps,
                                            GrTexture*[]) {
    return HairLineEdgeEffect::Create();
}

///////////////////////////////////////////////////////////////////////////////

namespace {

// position + edge
extern const GrVertexAttrib gHairlineAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,                  kPosition_GrVertexAttribBinding},
    {kVec4f_GrVertexAttribType, sizeof(GrPoint),    kEffect_GrVertexAttribBinding}
};

// Conic
// position + ABCD + EF
extern const GrVertexAttrib gConicVertexAttribs[] = {
    { kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding },
    { kVec4f_GrVertexAttribType, sizeof(GrPoint),   kEffect_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, 3*sizeof(GrPoint), kEffect_GrVertexAttribBinding }
};
};

bool GrAAHairLinePathRenderer::createGeom(
            const SkPath& path,
            GrDrawTarget* target,
            int* lineCnt,
            int* quadCnt,
            int* conicCnt,
            GrDrawTarget::AutoReleaseGeometry* arg,
            SkRect* devBounds) {
    GrDrawState* drawState = target->drawState();
    int rtHeight = drawState->getRenderTarget()->height();

    SkIRect devClipBounds;
    target->getClip()->getConservativeBounds(drawState->getRenderTarget(), &devClipBounds);

    SkMatrix viewM = drawState->getViewMatrix();

    // All the vertices that we compute are within 1 of path control points with the exception of
    // one of the bounding vertices for each quad. The add_quads() function will update the bounds
    // for each quad added.
    *devBounds = path.getBounds();
    viewM.mapRect(devBounds);
    devBounds->outset(SK_Scalar1, SK_Scalar1);

    PREALLOC_PTARRAY(128) lines;
    PREALLOC_PTARRAY(128) quads;
    PREALLOC_PTARRAY(128) conics;
    IntArray qSubdivs;
    FloatArray cWeights;
    *quadCnt = generate_lines_and_quads(path, viewM, devClipBounds,
                                        &lines, &quads, &conics, &qSubdivs, &cWeights);

    *lineCnt = lines.count() / 2;
    *conicCnt = conics.count() / 3;
    int vertCnt = kVertsPerLineSeg * *lineCnt + kVertsPerQuad * *quadCnt +
        kVertsPerQuad * *conicCnt;

    target->drawState()->setVertexAttribs<gConicVertexAttribs>(SK_ARRAY_COUNT(gConicVertexAttribs));
    GrAssert(sizeof(Vertex) == target->getDrawState().getVertexSize());

    if (!arg->set(target, vertCnt, 0)) {
        return false;
    }

    Vertex* verts = reinterpret_cast<Vertex*>(arg->vertices());

    const SkMatrix* toDevice = NULL;
    const SkMatrix* toSrc = NULL;
    SkMatrix ivm;

    if (viewM.hasPerspective()) {
        if (viewM.invert(&ivm)) {
            toDevice = &viewM;
            toSrc = &ivm;
        }
    }

    for (int i = 0; i < *lineCnt; ++i) {
        add_line(&lines[2*i], rtHeight, toSrc, &verts);
    }

    int unsubdivQuadCnt = quads.count() / 3;
    for (int i = 0; i < unsubdivQuadCnt; ++i) {
        GrAssert(qSubdivs[i] >= 0);
        add_quads(&quads[3*i], qSubdivs[i], toDevice, toSrc, &verts, devBounds);
    }

    // Start Conics
    for (int i = 0; i < *conicCnt; ++i) {
        add_conics(&conics[3*i], cWeights[i], toDevice, toSrc, &verts, devBounds);
    }
    return true;
}

bool GrAAHairLinePathRenderer::canDrawPath(const SkPath& path,
                                           const SkStrokeRec& stroke,
                                           const GrDrawTarget* target,
                                           bool antiAlias) const {
    if (!stroke.isHairlineStyle() || !antiAlias) {
        return false;
    }

    static const uint32_t gReqDerivMask = SkPath::kCubic_SegmentMask |
                                          SkPath::kQuad_SegmentMask;
    if (!target->caps()->shaderDerivativeSupport() &&
        (gReqDerivMask & path.getSegmentMasks())) {
        return false;
    }
    return true;
}

bool GrAAHairLinePathRenderer::onDrawPath(const SkPath& path,
                                          const SkStrokeRec&,
                                          GrDrawTarget* target,
                                          bool antiAlias) {

    int lineCnt;
    int quadCnt;
    int conicCnt;
    GrDrawTarget::AutoReleaseGeometry arg;
    SkRect devBounds;

    if (!this->createGeom(path,
                          target,
                          &lineCnt,
                          &quadCnt,
                          &conicCnt,
                          &arg,
                          &devBounds)) {
        return false;
    }

    GrDrawTarget::AutoStateRestore asr;

    // createGeom transforms the geometry to device space when the matrix does not have
    // perspective.
    if (target->getDrawState().getViewMatrix().hasPerspective()) {
        asr.set(target, GrDrawTarget::kPreserve_ASRInit);
    } else if (!asr.setIdentity(target, GrDrawTarget::kPreserve_ASRInit)) {
        return false;
    }
    GrDrawState* drawState = target->drawState();

    // TODO: See whether rendering lines as degenerate quads improves perf
    // when we have a mix

    static const int kEdgeAttrIndex = 1;

    GrEffectRef* hairLineEffect = HairLineEdgeEffect::Create();
    GrEffectRef* hairQuadEffect = HairQuadEdgeEffect::Create();
    GrEffectRef* hairConicEffect = HairConicEdgeEffect::Create();

    // Check devBounds
#if GR_DEBUG
    SkRect tolDevBounds = devBounds;
    tolDevBounds.outset(SK_Scalar1 / 10000, SK_Scalar1 / 10000);
    SkRect actualBounds;
    Vertex* verts = reinterpret_cast<Vertex*>(arg.vertices());
    int vCount = kVertsPerLineSeg * lineCnt + kVertsPerQuad * quadCnt + kVertsPerQuad * conicCnt;
    bool first = true;
    for (int i = 0; i < vCount; ++i) {
        SkPoint pos = verts[i].fPos;
        // This is a hack to workaround the fact that we move some degenerate segments offscreen.
        if (SK_ScalarMax == pos.fX) {
            continue;
        }
        drawState->getViewMatrix().mapPoints(&pos, 1);
        if (first) {
            actualBounds.set(pos.fX, pos.fY, pos.fX, pos.fY);
            first = false;
        } else {
            actualBounds.growToInclude(pos.fX, pos.fY);
        }
    }
    if (!first) {
        GrAssert(tolDevBounds.contains(actualBounds));
    }
#endif

    {
        GrDrawState::AutoRestoreEffects are(drawState);
        target->setIndexSourceToBuffer(fLinesIndexBuffer);
        int lines = 0;
        int nBufLines = fLinesIndexBuffer->maxQuads();
        drawState->addCoverageEffect(hairLineEffect, kEdgeAttrIndex)->unref();
        while (lines < lineCnt) {
            int n = GrMin(lineCnt - lines, nBufLines);
            target->drawIndexed(kTriangles_GrPrimitiveType,
                                kVertsPerLineSeg*lines,     // startV
                                0,                          // startI
                                kVertsPerLineSeg*n,         // vCount
                                kIdxsPerLineSeg*n,
                                &devBounds);                // iCount
            lines += n;
        }
    }

    {
        GrDrawState::AutoRestoreEffects are(drawState);
        target->setIndexSourceToBuffer(fQuadsIndexBuffer);
        int quads = 0;
        drawState->addCoverageEffect(hairQuadEffect, kEdgeAttrIndex)->unref();
        while (quads < quadCnt) {
            int n = GrMin(quadCnt - quads, kNumQuadsInIdxBuffer);
            target->drawIndexed(kTriangles_GrPrimitiveType,
                                kVertsPerLineSeg * lineCnt + kVertsPerQuad*quads,  // startV
                                0,                                                 // startI
                                kVertsPerQuad*n,                                   // vCount
                                kIdxsPerQuad*n,                                    // iCount
                                &devBounds);
            quads += n;
        }
    }

    {
        GrDrawState::AutoRestoreEffects are(drawState);
        int conics = 0;
        drawState->addCoverageEffect(hairConicEffect, 1, 2)->unref();
        while (conics < conicCnt) {
            int n = GrMin(conicCnt - conics, kNumQuadsInIdxBuffer);
            target->drawIndexed(kTriangles_GrPrimitiveType,
                                kVertsPerLineSeg*lineCnt +
                                kVertsPerQuad*(quadCnt + conics),  // startV
                                0,                                 // startI
                                kVertsPerQuad*n,                   // vCount
                                kIdxsPerQuad*n,                    // iCount
                                &devBounds);
            conics += n;
        }
    }
    target->resetIndexSource();

    return true;
}
