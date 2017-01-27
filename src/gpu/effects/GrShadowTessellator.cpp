/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShadowTessellator.h"
#include "GrPathUtils.h"

#include "SkGeometry.h"

static bool compute_normal(const SkPoint& p0, const SkPoint& p1, SkScalar radius, SkScalar dir,
                           SkVector* newNormal) {
    SkVector normal;
    // compute perpendicular
    normal.fX = p0.fY - p1.fY;
    normal.fY = p1.fX - p0.fX;
    if (!normal.normalize()) {
        return false;
    }
    normal *= radius*dir;
    *newNormal = normal;
    return true;
}

static void compute_radial_steps(const SkVector& v1, const SkVector& v2, SkScalar r,
                                 SkScalar* rotSin, SkScalar* rotCos, int* n) {
    const SkScalar kRecipPixelsPerArcSegment = 0.25f;

    SkScalar rCos = v1.dot(v2);
    SkScalar rSin = v1.cross(v2);
    SkScalar theta = SkScalarATan2(rSin, rCos);

    SkScalar steps = r*theta*kRecipPixelsPerArcSegment;

    SkScalar dTheta = theta / steps;
    *rotSin = SkScalarSinCos(dTheta, rotCos);
    *n = SkScalarFloorToInt(steps);
}

GrAmbientShadowTessellator::GrAmbientShadowTessellator(const SkPath& path,
                                                       SkScalar radius,
                                                       GrColor umbraColor,
                                                       GrColor penumbraColor,
                                                       bool transparent)
    : fRadius(radius)
    , fUmbraColor(umbraColor)
    , fPenumbraColor(penumbraColor)
    , fTransparent(transparent)
    , fPrevInnerIndex(-1) {

    // Outer ring: 3*numPts
    // Middle ring: numPts
    fPositions.setReserve(4 * path.countPoints());
    fColors.setReserve(4 * path.countPoints());
    // Outer ring: 12*numPts
    // Middle ring: 0
    fIndices.setReserve(12 * path.countPoints());

    fInitPoints.setReserve(3);

    // walk around the path, tessellate and generate outer ring
    // if original path is transparent, will accumulate sum of points for centroid
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;
    if (fTransparent) {
        *fPositions.push() = SkPoint::Make(0, 0);
        *fColors.push() = umbraColor;
        fCentroidCount = 0;
    }
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                this->handleLine(pts[1]);
                break;
            case SkPath::kQuad_Verb:
                this->handleQuad(pts);
                break;
            case SkPath::kCubic_Verb:
                this->handleCubic(pts);
                break;
            case SkPath::kConic_Verb:
                this->handleConic(pts, iter.conicWeight());
                break;
            case SkPath::kMove_Verb:
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                break;
        }
    }

    SkVector normal;
    if (compute_normal(fPositions[fPrevInnerIndex], fPositions[fFirstVertex], fRadius, fDirection,
                       &normal)) {
        this->addArc(normal);

        // close out previous arc
        *fPositions.push() = fPositions[fPrevInnerIndex] + normal;
        *fColors.push() = fPenumbraColor;
        *fIndices.push() = fPrevInnerIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;

        // add final edge
        *fPositions.push() = fPositions[fFirstVertex] + normal;
        *fColors.push() = fPenumbraColor;

        *fIndices.push() = fPrevInnerIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fFirstVertex;

        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fFirstVertex;
    }

    // finalize centroid
    if (fTransparent) {
        fPositions[0] *= SkScalarFastInvert(fCentroidCount);

        *fIndices.push() = 0;
        *fIndices.push() = fPrevInnerIndex;
        *fIndices.push() = fFirstVertex;
    }

    // final fan
    if (fPositions.count() >= 3) {
        fPrevInnerIndex = fFirstVertex;
        fPrevNormal = normal;
        this->addArc(fFirstNormal);

        *fIndices.push() = fFirstVertex;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fFirstVertex + 1;
    }
}

// tesselation tolerance values, in device space pixels
static const SkScalar kQuadTolerance = 0.2f;
static const SkScalar kCubicTolerance = 0.2f;
static const SkScalar kConicTolerance = 0.5f;

void GrAmbientShadowTessellator::handleLine(const SkPoint& p)  {
    if (fInitPoints.count() < 2) {
        *fInitPoints.push() = p;
        return;
    }

    if (fInitPoints.count() == 2) {
        // determine if cw or ccw
        SkVector v0 = fInitPoints[1] - fInitPoints[0];
        SkVector v1 = p - fInitPoints[0];
        SkScalar perpDot = v0.fX*v1.fY - v0.fY*v1.fX;
        if (SkScalarNearlyZero(perpDot)) {
            // nearly parallel, just treat as straight line and continue
            fInitPoints[1] = p;
            return;
        }

        // if perpDot > 0, winding is ccw
        fDirection = (perpDot > 0) ? -1 : 1;

        // add first quad
        if (!compute_normal(fInitPoints[0], fInitPoints[1], fRadius, fDirection,
                            &fFirstNormal)) {
            // first two points are incident, make the third point the second and continue
            fInitPoints[1] = p;
            return;
        }

        fFirstVertex = fPositions.count();
        fPrevNormal = fFirstNormal;
        fPrevInnerIndex = fFirstVertex;

        *fPositions.push() = fInitPoints[0];
        *fColors.push() = fUmbraColor;
        *fPositions.push() = fInitPoints[0] + fFirstNormal;
        *fColors.push() = fPenumbraColor;
        if (fTransparent) {
            fPositions[0] += fInitPoints[0];
            fCentroidCount = 1;
        }
        this->addEdge(fInitPoints[1], fFirstNormal);

        // to ensure we skip this block next time
        *fInitPoints.push() = p;
    }

    SkVector normal;
    if (compute_normal(fPositions[fPrevInnerIndex], p, fRadius, fDirection, &normal)) {
        this->addArc(normal);
        this->finishArcAndAddEdge(p, normal);
    }
}

void GrAmbientShadowTessellator::handleQuad(const SkPoint pts[3]) {
    int maxCount = GrPathUtils::quadraticPointCount(pts, kQuadTolerance);
    fPointBuffer.setReserve(maxCount);
    SkPoint* target = fPointBuffer.begin();
    int count = GrPathUtils::generateQuadraticPoints(pts[0], pts[1], pts[2],
                                                     kQuadTolerance, &target, maxCount);
    fPointBuffer.setCount(count);
    for (int i = 0; i < count; i++) {
        this->handleLine(fPointBuffer[i]);
    }
}

void GrAmbientShadowTessellator::handleCubic(SkPoint pts[4]) {
    int maxCount = GrPathUtils::cubicPointCount(pts, kCubicTolerance);
    fPointBuffer.setReserve(maxCount);
    SkPoint* target = fPointBuffer.begin();
    int count = GrPathUtils::generateCubicPoints(pts[0], pts[1], pts[2], pts[3],
                                                 kCubicTolerance, &target, maxCount);
    fPointBuffer.setCount(count);
    for (int i = 0; i < count; i++) {
        this->handleLine(fPointBuffer[i]);
    }
}

void GrAmbientShadowTessellator::handleConic(SkPoint pts[3], SkScalar w) {
    SkAutoConicToQuads quadder;
    const SkPoint* quads = quadder.computeQuads(pts, w, kConicTolerance);
    SkPoint lastPoint = *(quads++);
    int count = quadder.countQuads();
    for (int i = 0; i < count; ++i) {
        SkPoint quadPts[3];
        quadPts[0] = lastPoint;
        quadPts[1] = quads[0];
        quadPts[2] = i == count - 1 ? pts[2] : quads[1];
        this->handleQuad(quadPts);
        lastPoint = quadPts[2];
        quads += 2;
    }
}

void GrAmbientShadowTessellator::addArc(const SkVector& nextNormal) {
    // fill in fan from previous quad
    SkScalar rotSin, rotCos;
    int numSteps;
    compute_radial_steps(fPrevNormal, nextNormal, fRadius, &rotSin, &rotCos, &numSteps);
    SkVector prevNormal = fPrevNormal;
    for (int i = 0; i < numSteps; ++i) {
        SkVector nextNormal;
        nextNormal.fX = prevNormal.fX*rotCos - prevNormal.fY*rotSin;
        nextNormal.fY = prevNormal.fY*rotCos + prevNormal.fX*rotSin;
        *fPositions.push() = fPositions[fPrevInnerIndex] + nextNormal;
        *fColors.push() = fPenumbraColor;
        *fIndices.push() = fPrevInnerIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;

        prevNormal = nextNormal;
    }
}

void GrAmbientShadowTessellator::finishArcAndAddEdge(const SkPoint& nextPoint,
                                                     const SkVector& nextNormal) {
    // close out previous arc
    *fPositions.push() = fPositions[fPrevInnerIndex] + nextNormal;
    *fColors.push() = fPenumbraColor;
    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPositions.count() - 2;
    *fIndices.push() = fPositions.count() - 1;

    this->addEdge(nextPoint, nextNormal);
}

void GrAmbientShadowTessellator::addEdge(const SkPoint& nextPoint, const SkVector& nextNormal) {
    // add next quad
    *fPositions.push() = nextPoint;
    *fColors.push() = fUmbraColor;
    *fPositions.push() = nextPoint + nextNormal;
    *fColors.push() = fPenumbraColor;

    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPositions.count() - 3;
    *fIndices.push() = fPositions.count() - 2;

    *fIndices.push() = fPositions.count() - 3;
    *fIndices.push() = fPositions.count() - 1;
    *fIndices.push() = fPositions.count() - 2;

    // if transparent, add point to first one in array and add to center fan
    if (fTransparent) {
        fPositions[0] += nextPoint;
        ++fCentroidCount;

        *fIndices.push() = 0;
        *fIndices.push() = fPrevInnerIndex;
        *fIndices.push() = fPositions.count() - 2;
    }

    fPrevInnerIndex = fPositions.count() - 2;
    fPrevNormal = nextNormal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrSpotShadowTessellator::GrSpotShadowTessellator(const SkPath& path,
                                                 SkScalar scale, const SkVector& translate,
                                                 SkScalar radius,
                                                 GrColor umbraColor, GrColor penumbraColor,
                                                 bool /* transparent */)
    : fRadius(radius)
    , fUmbraColor(umbraColor)
    , fPenumbraColor(penumbraColor)
    , fPrevInnerIndex(-1) {

    // TODO: calculate these better
    // Outer ring: 3*numPts
    // Inner ring: numPts
    fPositions.setReserve(4 * path.countPoints());
    fColors.setReserve(4 * path.countPoints());
    // Outer ring: 12*numPts
    // Inner ring: 0
    fIndices.setReserve(12 * path.countPoints());

    fInitPoints.setReserve(3);

    fClipPolygon.setReserve(path.countPoints());
    this->computeClipBounds(path);
    fCentroid *= scale;
    fCentroid += translate;

    // walk around the path, tessellate and generate inner and outer rings
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;
    *fPositions.push() = fCentroid;
    *fColors.push() = fUmbraColor;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                this->handleLine(scale, translate, pts[1]);
                break;
            case SkPath::kQuad_Verb:
                this->handleQuad(scale, translate, pts);
                break;
            case SkPath::kCubic_Verb:
                this->handleCubic(scale, translate, pts);
                break;
            case SkPath::kConic_Verb:
                this->handleConic(scale, translate, pts, iter.conicWeight());
                break;
            case SkPath::kMove_Verb:
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                break;
        }
    }

    SkVector normal;
    if (compute_normal(fPrevPoint, fFirstPoint, fRadius, fDirection,
                        &normal)) {
        this->addArc(normal);

        // close out previous arc
        *fPositions.push() = fPrevPoint + normal;
        *fColors.push() = fPenumbraColor;
        *fIndices.push() = fPrevInnerIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;

        // add final edge
        *fPositions.push() = fFirstPoint + normal;
        *fColors.push() = fPenumbraColor;

        *fIndices.push() = fPrevInnerIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fFirstVertex;

        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fFirstVertex;

        // add to center fan
        *fIndices.push() = 0;
        *fIndices.push() = fPrevInnerIndex;
        *fIndices.push() = fFirstVertex;
    }

    // final fan
    if (fPositions.count() >= 3) {
        fPrevInnerIndex = fFirstVertex;
        fPrevPoint = fFirstPoint;
        fPrevNormal = normal;
        this->addArc(fFirstNormal);

        *fIndices.push() = fFirstVertex;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fFirstVertex + 1;
    }
}

void GrSpotShadowTessellator::computeClipBounds(const SkPath& path) {
    // walk around the path and compute clip polygon
    // if original path is transparent, will accumulate sum of points for centroid
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;

    fCentroid = SkPoint::Make(0, 0);
    int centroidCount = 0;
    fClipPolygon.reset();

    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb:
                fCentroid += pts[1];
                centroidCount++;
                *fClipPolygon.push() = pts[1];
                break;
            case SkPath::kQuad_Verb:
                fCentroid += pts[1];
                fCentroid += pts[2];
                centroidCount += 2;
                *fClipPolygon.push() = pts[2];
                break;
            case SkPath::kConic_Verb:
                fCentroid += pts[1];
                fCentroid += pts[2];
                centroidCount += 2;
                *fClipPolygon.push() = pts[2];
                break;
            case SkPath::kCubic_Verb:
                fCentroid += pts[1];
                fCentroid += pts[2];
                fCentroid += pts[3];
                centroidCount += 3;
                *fClipPolygon.push() = pts[3];
                break;
            case SkPath::kClose_Verb:
                break;
            default:
                SkDEBUGFAIL("unknown verb");
        }
    }

    fCentroid *= SkScalarInvert(centroidCount);
}

void GrSpotShadowTessellator::mapPoints(SkScalar scale, const SkVector& xlate,
                                        SkPoint* pts, int count) {
    // TODO: vectorize
    for (int i = 0; i < count; ++i) {
        pts[i] *= scale;
        pts[i] += xlate;
    }
}

void GrSpotShadowTessellator::handleLine(const SkPoint& p) {
    if (fInitPoints.count() < 2) {
        *fInitPoints.push() = p;
        return;
    }

    if (fInitPoints.count() == 2) {
        // determine if cw or ccw
        SkVector v0 = fInitPoints[1] - fInitPoints[0];
        SkVector v1 = p - fInitPoints[0];
        SkScalar perpDot = v0.fX*v1.fY - v0.fY*v1.fX;
        if (SkScalarNearlyZero(perpDot)) {
            // nearly parallel, just treat as straight line and continue
            fInitPoints[1] = p;
            return;
        }

        // if perpDot > 0, winding is ccw
        fDirection = (perpDot > 0) ? -1 : 1;

        // add first quad
        if (!compute_normal(fInitPoints[0], fInitPoints[1], fRadius, fDirection,
                            &fFirstNormal)) {
            // first two points are incident, make the third point the second and continue
            fInitPoints[1] = p;
            return;
        }

        fFirstPoint = fInitPoints[0];
        fFirstVertex = fPositions.count();
        fPrevNormal = fFirstNormal;
        fPrevPoint = fFirstPoint;
        fPrevInnerIndex = fFirstVertex;

        this->addInnerPoint(fFirstPoint, fUmbraColor, fRadius);
        SkPoint newPoint = fFirstPoint + fFirstNormal;
        *fPositions.push() = newPoint;
        *fColors.push() = fPenumbraColor;
        this->addEdge(fInitPoints[1], fFirstNormal);

        // to ensure we skip this block next time
        *fInitPoints.push() = p;
    }

    SkVector normal;
    if (compute_normal(fPrevPoint, p, fRadius, fDirection, &normal)) {
        this->addArc(normal);
        this->finishArcAndAddEdge(p, normal);
    }
}

void GrSpotShadowTessellator::handleLine(SkScalar scale, const SkVector& xlate, SkPoint p) {
    this->mapPoints(scale, xlate, &p, 1);
    this->handleLine(p);
}

void GrSpotShadowTessellator::handleQuad(const SkPoint pts[3]) {
    int maxCount = GrPathUtils::quadraticPointCount(pts, kQuadTolerance);
    fPointBuffer.setReserve(maxCount);
    SkPoint* target = fPointBuffer.begin();
    int count = GrPathUtils::generateQuadraticPoints(pts[0], pts[1], pts[2],
                                                     kQuadTolerance, &target, maxCount);
    fPointBuffer.setCount(count);
    for (int i = 0; i < count; i++) {
        this->handleLine(fPointBuffer[i]);
    }
}

void GrSpotShadowTessellator::handleQuad(SkScalar scale, const SkVector& xlate, SkPoint pts[3]) {
    this->mapPoints(scale, xlate, pts, 3);
    this->handleQuad(pts);
}

void GrSpotShadowTessellator::handleCubic(SkScalar scale, const SkVector& xlate, SkPoint pts[4]) {
    this->mapPoints(scale, xlate, pts, 4);
    int maxCount = GrPathUtils::cubicPointCount(pts, kCubicTolerance);
    fPointBuffer.setReserve(maxCount);
    SkPoint* target = fPointBuffer.begin();
    int count = GrPathUtils::generateCubicPoints(pts[0], pts[1], pts[2], pts[3],
                                                 kCubicTolerance, &target, maxCount);
    fPointBuffer.setCount(count);
    for (int i = 0; i < count; i++) {
        this->handleLine(fPointBuffer[i]);
    }
}

void GrSpotShadowTessellator::handleConic(SkScalar scale, const SkVector& xlate,
                                          SkPoint pts[3], SkScalar w) {
    this->mapPoints(scale, xlate, pts, 3);
    SkAutoConicToQuads quadder;
    const SkPoint* quads = quadder.computeQuads(pts, w, kConicTolerance);
    SkPoint lastPoint = *(quads++);
    int count = quadder.countQuads();
    for (int i = 0; i < count; ++i) {
        SkPoint quadPts[3];
        quadPts[0] = lastPoint;
        quadPts[1] = quads[0];
        quadPts[2] = i == count - 1 ? pts[2] : quads[1];
        this->handleQuad(quadPts);
        lastPoint = quadPts[2];
        quads += 2;
    }
}

void GrSpotShadowTessellator::addInnerPoint(const SkPoint& pathPoint, GrColor umbraColor,
                                            SkScalar radius) {
    SkVector v = fCentroid - pathPoint;
    SkScalar distance = v.length();
    if (distance < radius) {
        *fPositions.push() = fCentroid;
        *fColors.push() = umbraColor; // fix this
        // TODO: deal with fanning from centroid
    } else {
        SkScalar t = radius / distance;
        v *= t;
        SkPoint innerPoint = pathPoint + v;
        *fPositions.push() = innerPoint;
        *fColors.push() = umbraColor;
    }
    fPrevPoint = pathPoint;
}

void GrSpotShadowTessellator::addArc(const SkVector& nextNormal) {
    // fill in fan from previous quad
    SkScalar rotSin, rotCos;
    int numSteps;
    compute_radial_steps(fPrevNormal, nextNormal, fRadius, &rotSin, &rotCos, &numSteps);
    SkVector prevNormal = fPrevNormal;
    for (int i = 0; i < numSteps; ++i) {
        SkVector nextNormal;
        nextNormal.fX = prevNormal.fX*rotCos - prevNormal.fY*rotSin;
        nextNormal.fY = prevNormal.fY*rotCos + prevNormal.fX*rotSin;
        *fPositions.push() = fPrevPoint + nextNormal;
        *fColors.push() = fPenumbraColor;
        *fIndices.push() = fPrevInnerIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;

        prevNormal = nextNormal;
    }
}

void GrSpotShadowTessellator::finishArcAndAddEdge(const SkPoint& nextPoint,
                                                  const SkVector& nextNormal) {
    // close out previous arc
    SkPoint newPoint = fPrevPoint + nextNormal;
    *fPositions.push() = newPoint;
    *fColors.push() = fPenumbraColor;
    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPositions.count() - 2;
    *fIndices.push() = fPositions.count() - 1;

    this->addEdge(nextPoint, nextNormal);
}

void GrSpotShadowTessellator::addEdge(const SkPoint& nextPoint, const SkVector& nextNormal) {
    // add next quad
    this->addInnerPoint(nextPoint, fUmbraColor, fRadius);
    SkPoint newPoint = nextPoint + nextNormal;
    *fPositions.push() = newPoint;
    *fColors.push() = fPenumbraColor;

    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPositions.count() - 3;
    *fIndices.push() = fPositions.count() - 2;

    *fIndices.push() = fPositions.count() - 3;
    *fIndices.push() = fPositions.count() - 1;
    *fIndices.push() = fPositions.count() - 2;

    // add to center fan
    *fIndices.push() = 0;
    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPositions.count() - 2;

    fPrevInnerIndex = fPositions.count() - 2;
    fPrevNormal = nextNormal;
}
