/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPolyUtils.h"

#include <set>
#include "SkPointPriv.h"
#include "SkTArray.h"
#include "SkTemplates.h"
#include "SkTDPQueue.h"
#include "SkTInternalLList.h"

//////////////////////////////////////////////////////////////////////////////////
// Helper data structures and functions

struct OffsetSegment {
    SkPoint fP0;
    SkVector fV;
};

// Computes perpDot for point p compared to segment defined by origin s0 and vector v0.
// A positive value means the point is to the left of the segment,
// negative is to the right, 0 is collinear.
static int compute_side(const SkPoint& s0, const SkVector& v0, const SkPoint& p) {
    SkVector v1 = p - s0;
    SkScalar perpDot = v0.cross(v1);
    if (!SkScalarNearlyZero(perpDot)) {
        return ((perpDot > 0) ? 1 : -1);
    }

    return 0;
}

// Returns 1 for cw, -1 for ccw and 0 if zero signed area (either degenerate or self-intersecting)
int SkGetPolygonWinding(const SkPoint* polygonVerts, int polygonSize) {
    if (polygonSize < 3) {
        return 0;
    }

    // compute area and use sign to determine winding
    SkScalar quadArea = 0;
    SkVector v0 = polygonVerts[1] - polygonVerts[0];
    for (int curr = 1; curr < polygonSize - 1; ++curr) {
        int next = (curr + 1) % polygonSize;
        SkVector v1 = polygonVerts[next] - polygonVerts[0];
        quadArea += v0.cross(v1);
        v0 = v1;
    }
    if (SkScalarNearlyZero(quadArea)) {
        return 0;
    }
    // 1 == ccw, -1 == cw
    return (quadArea > 0) ? 1 : -1;
}

// Helper function to compute the individual vector for non-equal offsets
inline void compute_offset(SkScalar d, const SkPoint& polyPoint, int side,
                           const SkPoint& outerTangentIntersect, SkVector* v) {
    SkScalar dsq = d * d;
    SkVector dP = outerTangentIntersect - polyPoint;
    SkScalar dPlenSq = SkPointPriv::LengthSqd(dP);
    if (SkScalarNearlyZero(dPlenSq)) {
        v->set(0, 0);
    } else {
        SkScalar discrim = SkScalarSqrt(dPlenSq - dsq);
        v->fX = (dsq*dP.fX - side * d*dP.fY*discrim) / dPlenSq;
        v->fY = (dsq*dP.fY + side * d*dP.fX*discrim) / dPlenSq;
    }
}

// Compute difference vector to offset p0-p1 'd0' and 'd1' units in direction specified by 'side'
bool compute_offset_vectors(const SkPoint& p0, const SkPoint& p1, SkScalar d0, SkScalar d1,
                            int side, SkPoint* vector0, SkPoint* vector1) {
    SkASSERT(side == -1 || side == 1);
    if (SkScalarNearlyEqual(d0, d1)) {
        // if distances are equal, can just outset by the perpendicular
        SkVector perp = SkVector::Make(p0.fY - p1.fY, p1.fX - p0.fX);
        perp.setLength(d0*side);
        *vector0 = perp;
        *vector1 = perp;
    } else {
        SkScalar d0abs = SkTAbs(d0);
        SkScalar d1abs = SkTAbs(d1);
        // Otherwise we need to compute the outer tangent.
        // See: http://www.ambrsoft.com/TrigoCalc/Circles2/Circles2Tangent_.htm
        if (d0abs < d1abs) {
            side = -side;
        }
        SkScalar dD = d0abs - d1abs;
        // if one circle is inside another, we can't compute an offset
        if (dD*dD >= SkPointPriv::DistanceToSqd(p0, p1)) {
            return false;
        }
        SkPoint outerTangentIntersect = SkPoint::Make((p1.fX*d0abs - p0.fX*d1abs) / dD,
                                                      (p1.fY*d0abs - p0.fY*d1abs) / dD);

        compute_offset(d0, p0, side, outerTangentIntersect, vector0);
        compute_offset(d1, p1, side, outerTangentIntersect, vector1);
    }

    return true;
}

// Offset line segment p0-p1 'd0' and 'd1' units in the direction specified by 'side'
bool SkOffsetSegment(const SkPoint& p0, const SkPoint& p1, SkScalar d0, SkScalar d1,
                     int side, SkPoint* offset0, SkPoint* offset1) {
    SkVector v0, v1;
    if (!compute_offset_vectors(p0, p1, d0, d1, side, &v0, &v1)) {
        return false;
    }
    *offset0 = p0 + v0;
    *offset1 = p1 + v1;

    return true;
}

// compute fraction of d along v
static inline SkScalar compute_param(const SkVector& v, const SkVector& d) {
    if (SkScalarNearlyZero(v.fX)) {
        return d.fY / v.fY;
    } else {
        return d.fX / v.fX;
    }
}

// Compute the intersection 'p' between segments s0 and s1, if any.
// 's' is the parametric value for the intersection along 's0' & 't' is the same for 's1'.
// Returns false if there is no intersection.
static bool compute_intersection(const OffsetSegment& s0, const OffsetSegment& s1,
                                 SkPoint* p, SkScalar* s, SkScalar* t) {
    const SkVector& v0 = s0.fV;
    const SkVector& v1 = s1.fV;
    SkVector d = s1.fP0 - s0.fP0;
    SkScalar perpDot = v0.cross(v1);
    SkScalar localS, localT;
    if (SkScalarNearlyZero(perpDot)) {
        // segments are parallel, but not collinear
        if (!SkScalarNearlyZero(d.cross(v0)) || !SkScalarNearlyZero(d.cross(v1))) {
            return false;
        }

        // Check for degenerate segments
        if (!SkPointPriv::CanNormalize(v0.fX, v0.fY)) {
            // Both are degenerate
            if (!SkPointPriv::CanNormalize(v1.fX, v1.fY)) {
                // Check if they're the same point
                if (!SkPointPriv::CanNormalize(d.fX, d.fY)) {
                    *p = s0.fP0;
                    *s = 0;
                    *t = 0;
                    return true;
                } else {
                    return false;
                }
            }
            // Otherwise project onto segment1
            localT = compute_param(v1, -d);
            if (localT < 0 || localT > SK_Scalar1) {
                return false;
            }
            localS = 0;
        } else {
            // Project segment1's endpoints onto segment0
            localS = compute_param(v0, d);
            localT = 0;
            if (localS < 0 || localS > SK_Scalar1) {
                // The first endpoint doesn't lie on segment0
                // If segment1 is degenerate, then there's no collision
                if (!SkPointPriv::CanNormalize(v1.fX, v1.fY)) {
                    return false;
                }

                // Otherwise try the other one
                SkScalar oldLocalS = localS;
                localS = compute_param(v0, d + v1);
                localT = SK_Scalar1;
                if (localS < 0 || localS > SK_Scalar1) {
                    // it's possible that segment1's interval surrounds segment0
                    // this is false if params have the same signs, and in that case no collision
                    if (localS*oldLocalS > 0) {
                        return false;
                    }
                    // otherwise project segment0's endpoint onto segment1 instead
                    localS = 0;
                    localT = compute_param(v1, -d);
                }
            }
        }
    } else {
        localS = d.cross(v1) / perpDot;
        if (localS < 0 || localS > SK_Scalar1) {
            return false;
        }
        localT = d.cross(v0) / perpDot;
        if (localT < 0 || localT > SK_Scalar1) {
            return false;
        }
    }

    *p = s0.fP0 + v0*localS;
    *s = localS;
    *t = localT;

    return true;
}

// computes the line intersection and then the distance to s0's endpoint
static SkScalar compute_crossing_distance(const OffsetSegment& s0, const OffsetSegment& s1) {
    const SkVector& v0 = s0.fV;
    const SkVector& v1 = s1.fV;

    SkScalar perpDot = v0.cross(v1);
    if (SkScalarNearlyZero(perpDot)) {
        // segments are parallel
        return SK_ScalarMax;
    }

    SkVector d = s1.fP0 - s0.fP0;
    SkScalar localS = d.cross(v1) / perpDot;
    if (localS < 0) {
        localS = -localS;
    } else {
        localS -= SK_Scalar1;
    }

    localS *= v0.length();

    return localS;
}

bool SkIsConvexPolygon(const SkPoint* polygonVerts, int polygonSize) {
    if (polygonSize < 3) {
        return false;
    }

    SkScalar lastArea = 0;
    SkScalar lastPerpDot = 0;

    int prevIndex = polygonSize - 1;
    int currIndex = 0;
    int nextIndex = 1;
    SkPoint origin = polygonVerts[0];
    SkVector v0 = polygonVerts[currIndex] - polygonVerts[prevIndex];
    SkVector v1 = polygonVerts[nextIndex] - polygonVerts[currIndex];
    SkVector w0 = polygonVerts[currIndex] - origin;
    SkVector w1 = polygonVerts[nextIndex] - origin;
    for (int i = 0; i < polygonSize; ++i) {
        if (!polygonVerts[i].isFinite()) {
            return false;
        }

        // Check that winding direction is always the same (otherwise we have a reflex vertex)
        SkScalar perpDot = v0.cross(v1);
        if (lastPerpDot*perpDot < 0) {
            return false;
        }
        if (0 != perpDot) {
            lastPerpDot = perpDot;
        }

        // If the signed area ever flips it's concave
        // TODO: see if we can verify convexity only with signed area
        SkScalar quadArea = w0.cross(w1);
        if (quadArea*lastArea < 0) {
            return false;
        }
        if (0 != quadArea) {
            lastArea = quadArea;
        }

        prevIndex = currIndex;
        currIndex = nextIndex;
        nextIndex = (currIndex + 1) % polygonSize;
        v0 = v1;
        v1 = polygonVerts[nextIndex] - polygonVerts[currIndex];
        w0 = w1;
        w1 = polygonVerts[nextIndex] - origin;
    }

    return true;
}

struct OffsetEdge {
    OffsetEdge*   fPrev;
    OffsetEdge*   fNext;
    OffsetSegment fInset;
    SkPoint       fIntersection;
    SkScalar      fTValue;
    uint16_t      fIndex;
    uint16_t      fEnd;

    void init(uint16_t start = 0, uint16_t end = 0) {
        fIntersection = fInset.fP0;
        fTValue = SK_ScalarMin;
        fIndex = start;
        fEnd = end;
    }

    // special intersection check that looks for endpoint intersection
    bool checkIntersection(const OffsetEdge* that,
                           SkPoint* p, SkScalar* s, SkScalar* t) {
        if (this->fEnd == that->fIndex) {
            SkPoint p1 = this->fInset.fP0 + this->fInset.fV;
            if (SkPointPriv::EqualsWithinTolerance(p1, that->fInset.fP0)) {
                *p = p1;
                *s = SK_Scalar1;
                *t = 0;
                return true;
            }
        }

        return compute_intersection(this->fInset, that->fInset, p, s, t);
    }
};

static void remove_node(const OffsetEdge* node, OffsetEdge** head) {
    // remove from linked list
    node->fPrev->fNext = node->fNext;
    node->fNext->fPrev = node->fPrev;
    if (node == *head) {
        *head = (node->fNext == node) ? nullptr : node->fNext;
    }
}

//////////////////////////////////////////////////////////////////////////////////

// The objective here is to inset all of the edges by the given distance, and then
// remove any invalid inset edges by detecting right-hand turns. In a ccw polygon,
// we should only be making left-hand turns (for cw polygons, we use the winding
// parameter to reverse this). We detect this by checking whether the second intersection
// on an edge is closer to its tail than the first one.
//
// We might also have the case that there is no intersection between two neighboring inset edges.
// In this case, one edge will lie to the right of the other and should be discarded along with
// its previous intersection (if any).
//
// Note: the assumption is that inputPolygon is convex and has no coincident points.
//
bool SkInsetConvexPolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                          std::function<SkScalar(const SkPoint&)> insetDistanceFunc,
                          SkTDArray<SkPoint>* insetPolygon) {
    if (inputPolygonSize < 3) {
        return false;
    }

    // restrict this to match other routines
    // practically we don't want anything bigger than this anyway
    if (inputPolygonSize >= (1 << 16)) {
        return false;
    }

    // get winding direction
    int winding = SkGetPolygonWinding(inputPolygonVerts, inputPolygonSize);
    if (0 == winding) {
        return false;
    }

    // set up
    SkAutoSTMalloc<64, OffsetEdge> edgeData(inputPolygonSize);
    int prev = inputPolygonSize - 1;
    for (int curr = 0; curr < inputPolygonSize; prev = curr, ++curr) {
        int next = (curr + 1) % inputPolygonSize;
        if (!inputPolygonVerts[curr].isFinite()) {
            return false;
        }
        // check for convexity just to be sure
        if (compute_side(inputPolygonVerts[prev], inputPolygonVerts[curr] - inputPolygonVerts[prev],
                         inputPolygonVerts[next])*winding < 0) {
            return false;
        }
        SkPoint p0, p1;
        if (!SkOffsetSegment(inputPolygonVerts[curr], inputPolygonVerts[next],
                             insetDistanceFunc(inputPolygonVerts[curr]),
                             insetDistanceFunc(inputPolygonVerts[next]),
                             winding,
                             &p0, &p1)) {
            return false;
        }
        edgeData[curr].fPrev = &edgeData[prev];
        edgeData[curr].fNext = &edgeData[next];
        edgeData[curr].fInset.fP0 = p0;
        edgeData[curr].fInset.fV = p1 - p0;
        edgeData[curr].init();
    }

    OffsetEdge* head = &edgeData[0];
    OffsetEdge* currEdge = head;
    OffsetEdge* prevEdge = currEdge->fPrev;
    int insetVertexCount = inputPolygonSize;
    int iterations = 0;
    while (head && prevEdge != currEdge) {
        ++iterations;
        // we should check each edge against each other edge at most once
        if (iterations > inputPolygonSize*inputPolygonSize) {
            return false;
        }

        SkScalar s, t;
        SkPoint intersection;
        if (compute_intersection(prevEdge->fInset, currEdge->fInset,
                                 &intersection, &s, &t)) {
            // if new intersection is further back on previous inset from the prior intersection
            if (s < prevEdge->fTValue) {
                // no point in considering this one again
                remove_node(prevEdge, &head);
                --insetVertexCount;
                // go back one segment
                prevEdge = prevEdge->fPrev;
            // we've already considered this intersection, we're done
            } else if (currEdge->fTValue > SK_ScalarMin &&
                       SkPointPriv::EqualsWithinTolerance(intersection,
                                                          currEdge->fIntersection,
                                                          1.0e-6f)) {
                break;
            } else {
                // add intersection
                currEdge->fIntersection = intersection;
                currEdge->fTValue = t;

                // go to next segment
                prevEdge = currEdge;
                currEdge = currEdge->fNext;
            }
        } else {
            // if prev to right side of curr
            int side = winding*compute_side(currEdge->fInset.fP0,
                                            currEdge->fInset.fV,
                                            prevEdge->fInset.fP0);
            if (side < 0 &&
                side == winding*compute_side(currEdge->fInset.fP0,
                                             currEdge->fInset.fV,
                                             prevEdge->fInset.fP0 + prevEdge->fInset.fV)) {
                // no point in considering this one again
                remove_node(prevEdge, &head);
                --insetVertexCount;
                // go back one segment
                prevEdge = prevEdge->fPrev;
            } else {
                // move to next segment
                remove_node(currEdge, &head);
                --insetVertexCount;
                currEdge = currEdge->fNext;
            }
        }
    }

    // store all the valid intersections that aren't nearly coincident
    // TODO: look at the main algorithm and see if we can detect these better
    insetPolygon->reset();
    if (head) {
        static constexpr SkScalar kCleanupTolerance = 0.01f;
        if (insetVertexCount >= 0) {
            insetPolygon->setReserve(insetVertexCount);
        }
        int currIndex = 0;
        OffsetEdge* currEdge = head;
        *insetPolygon->push() = currEdge->fIntersection;
        currEdge = currEdge->fNext;
        while (currEdge != head) {
            if (!SkPointPriv::EqualsWithinTolerance(currEdge->fIntersection,
                                                    (*insetPolygon)[currIndex],
                                                    kCleanupTolerance)) {
                *insetPolygon->push() = currEdge->fIntersection;
                currIndex++;
            }
            currEdge = currEdge->fNext;
        }
        // make sure the first and last points aren't coincident
        if (currIndex >= 1 &&
           SkPointPriv::EqualsWithinTolerance((*insetPolygon)[0], (*insetPolygon)[currIndex],
                                              kCleanupTolerance)) {
            insetPolygon->pop();
        }
    }

    return SkIsConvexPolygon(insetPolygon->begin(), insetPolygon->count());
}

///////////////////////////////////////////////////////////////////////////////////////////

// compute the number of points needed for a circular join when offsetting a reflex vertex
bool SkComputeRadialSteps(const SkVector& v1, const SkVector& v2, SkScalar r,
                          SkScalar* rotSin, SkScalar* rotCos, int* n) {
    const SkScalar kRecipPixelsPerArcSegment = 0.25f;

    SkScalar rCos = v1.dot(v2);
    if (!SkScalarIsFinite(rCos)) {
        return false;
    }
    SkScalar rSin = v1.cross(v2);
    if (!SkScalarIsFinite(rSin)) {
        return false;
    }
    SkScalar theta = SkScalarATan2(rSin, rCos);

    SkScalar floatSteps = SkScalarAbs(r*theta*kRecipPixelsPerArcSegment);
    // limit the number of steps to at most max uint16_t (that's all we can index)
    // knock one value off the top to account for rounding
    if (floatSteps >= (1 << 16)-1) {
        return false;
    }
    int steps = SkScalarRoundToInt(floatSteps);

    SkScalar dTheta = steps > 0 ? theta / steps : 0;
    *rotSin = SkScalarSinCos(dTheta, rotCos);
    *n = steps;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

// a point is "left" to another if its x coordinate is less, or if equal, its y coordinate
static bool left(const SkPoint& p0, const SkPoint& p1) {
    return p0.fX < p1.fX || (!(p0.fX > p1.fX) && p0.fY < p1.fY);
}

struct Vertex {
    static bool Left(const Vertex& qv0, const Vertex& qv1) {
        return left(qv0.fPosition, qv1.fPosition);
    }

    // packed to fit into 16 bytes (one cache line)
    SkPoint  fPosition;
    uint16_t fIndex;       // index in unsorted polygon
    uint16_t fPrevIndex;   // indices for previous and next vertex in unsorted polygon
    uint16_t fNextIndex;
    uint16_t fFlags;
};

enum VertexFlags {
    kPrevLeft_VertexFlag = 0x1,
    kNextLeft_VertexFlag = 0x2,
};

struct ActiveEdge {
    ActiveEdge(const SkPoint& p0, const SkPoint& p1, uint16_t index0, uint16_t index1)
        : fSegment({p0, p1-p0})
        , fIndex0(index0)
        , fIndex1(index1) {}

    // returns true if "this" is above "that"
    bool above(const ActiveEdge& that) const {
        SkASSERT(this->fSegment.fP0.fX <= that.fSegment.fP0.fX);
        const SkScalar kTolerance = SK_ScalarNearlyZero * SK_ScalarNearlyZero;
        const SkVector& u = this->fSegment.fV;
        SkVector dv = that.fSegment.fP0 - this->fSegment.fP0;
        // The idea here is that if the vector between the origins of the two segments (dv)
        // rotates counterclockwise up to the vector representing the "this" segment (u),
        // then we know that "this" is above that. If the result is clockwise we say it's below.
        if (this->fIndex0 != that.fIndex0) {
            SkScalar cross = dv.cross(u);
            if (cross > kTolerance) {
                return true;
            } else if (cross < -kTolerance) {
                return false;
            }
        } else if (this->fIndex1 == that.fIndex1) {
            // they're the same edge
            return false;
        }
        // At this point either the two origins are nearly equal or the origin of "that"
        // lies on dv. So then we try the same for the vector from the tail of "this"
        // to the head of "that". Again, ccw means "this" is above "that".
        // dv = that.P1 - this.P0
        //    = that.fP0 + that.fV - this.fP0
        //    = that.fP0 - this.fP0 + that.fV
        //    = old_dv + that.fV
        dv += that.fSegment.fV;
        SkScalar cross = dv.cross(u);
        if (cross > kTolerance) {
            return true;
        } else if (cross < -kTolerance) {
            return false;
        }
        // If the previous check fails, the two segments are nearly collinear
        // First check y-coord of first endpoints
        if (this->fSegment.fP0.fX < that.fSegment.fP0.fX) {
            return (this->fSegment.fP0.fY >= that.fSegment.fP0.fY);
        } else if (this->fSegment.fP0.fY > that.fSegment.fP0.fY) {
            return true;
        } else if (this->fSegment.fP0.fY < that.fSegment.fP0.fY) {
            return false;
        }
        // The first endpoints are the same, so check the other endpoint
        SkPoint thisP1 = this->fSegment.fP0 + this->fSegment.fV;
        SkPoint thatP1 = that.fSegment.fP0 + that.fSegment.fV;
        if (thisP1.fX < thatP1.fX) {
            return (thisP1.fY >= thatP1.fY);
        } else {
            return (thisP1.fY > thatP1.fY);
        }
    }

    bool intersect(const ActiveEdge& that) const {
        SkPoint intersection;
        SkScalar s, t;
        // check first to see if these edges are neighbors in the polygon
        if (this->fIndex0 == that.fIndex0 || this->fIndex1 == that.fIndex0 ||
            this->fIndex0 == that.fIndex1 || this->fIndex1 == that.fIndex1) {
            return false;
        }
        return compute_intersection(this->fSegment, that.fSegment, &intersection, &s, &t);
    }

    bool lessThan(const ActiveEdge& that) const {
        if (this->fSegment.fP0.fX > that.fSegment.fP0.fX ||
            (this->fSegment.fP0.fX == that.fSegment.fP0.fX &&
             this->fSegment.fP0.fY < that.fSegment.fP0.fY)) {
            return !that.above(*this);
        }
        return this->above(that);
    }

    bool operator<(const ActiveEdge& that) const {
        SkASSERT(!this->lessThan(*this));
        SkASSERT(!that.lessThan(that));
        SkASSERT(!(this->lessThan(that) && that.lessThan(*this)));
        return this->lessThan(that);
    }

    OffsetSegment fSegment;
    uint16_t fIndex0;   // indices for previous and next vertex
    uint16_t fIndex1;
};

class ActiveEdgeList {
public:
    bool insert(const SkPoint& p0, const SkPoint& p1, uint16_t index0, uint16_t index1) {
        std::pair<Iterator, bool> result = fEdgeTree.emplace(p0, p1, index0, index1);
        if (!result.second) {
            return false;
        }

        Iterator& curr = result.first;
        if (curr != fEdgeTree.begin() && curr->intersect(*std::prev(curr))) {
            return false;
        }
        Iterator next = std::next(curr);
        if (next != fEdgeTree.end() && curr->intersect(*next)) {
            return false;
        }

        return true;
    }

    bool remove(const ActiveEdge& edge) {
        auto element = fEdgeTree.find(edge);
        // this better not happen
        if (element == fEdgeTree.end()) {
            return false;
        }
        if (element != fEdgeTree.begin() && element->intersect(*std::prev(element))) {
            return false;
        }
        Iterator next = std::next(element);
        if (next != fEdgeTree.end() && element->intersect(*next)) {
            return false;
        }

        fEdgeTree.erase(element);
        return true;
    }

private:
    std::set<ActiveEdge> fEdgeTree;
    typedef std::set<ActiveEdge>::iterator Iterator;
};

// Here we implement a sweep line algorithm to determine whether the provided points
// represent a simple polygon, i.e., the polygon is non-self-intersecting.
// We first insert the vertices into a priority queue sorting horizontally from left to right.
// Then as we pop the vertices from the queue we generate events which indicate that an edge
// should be added or removed from an edge list. If any intersections are detected in the edge
// list, then we know the polygon is self-intersecting and hence not simple.
bool SkIsSimplePolygon(const SkPoint* polygon, int polygonSize) {
    if (polygonSize < 3) {
        return false;
    }

    // need to be able to represent all the vertices in the 16-bit indices
    if (polygonSize >= (1 << 16)) {
        return false;
    }

    SkTDPQueue <Vertex, Vertex::Left> vertexQueue(polygonSize);
    for (int i = 0; i < polygonSize; ++i) {
        Vertex newVertex;
        if (!polygon[i].isFinite()) {
            return false;
        }
        newVertex.fPosition = polygon[i];
        newVertex.fIndex = i;
        newVertex.fPrevIndex = (i - 1 + polygonSize) % polygonSize;
        newVertex.fNextIndex = (i + 1) % polygonSize;
        newVertex.fFlags = 0;
        if (left(polygon[newVertex.fPrevIndex], polygon[i])) {
            newVertex.fFlags |= kPrevLeft_VertexFlag;
        }
        if (left(polygon[newVertex.fNextIndex], polygon[i])) {
            newVertex.fFlags |= kNextLeft_VertexFlag;
        }
        vertexQueue.insert(newVertex);
    }

    // pop each vertex from the queue and generate events depending on
    // where it lies relative to its neighboring edges
    ActiveEdgeList sweepLine;
    while (vertexQueue.count() > 0) {
        const Vertex& v = vertexQueue.peek();

        // check edge to previous vertex
        if (v.fFlags & kPrevLeft_VertexFlag) {
            ActiveEdge edge(polygon[v.fPrevIndex], v.fPosition, v.fPrevIndex, v.fIndex);
            if (!sweepLine.remove(edge)) {
                break;
            }
        } else {
            if (!sweepLine.insert(v.fPosition, polygon[v.fPrevIndex], v.fIndex, v.fPrevIndex)) {
                break;
            }
        }

        // check edge to next vertex
        if (v.fFlags & kNextLeft_VertexFlag) {
            ActiveEdge edge(polygon[v.fNextIndex], v.fPosition, v.fNextIndex, v.fIndex);
            if (!sweepLine.remove(edge)) {
                break;
            }
        } else {
            if (!sweepLine.insert(v.fPosition, polygon[v.fNextIndex], v.fIndex, v.fNextIndex)) {
                break;
            }
        }

        vertexQueue.pop();
    }

    return (vertexQueue.count() == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////

// helper function for SkOffsetSimplePolygon
static void setup_offset_edge(OffsetEdge* currEdge,
                              const SkPoint& endpoint0, const SkPoint& endpoint1,
                              int startIndex, int endIndex) {
    currEdge->fInset.fP0 = endpoint0;
    currEdge->fInset.fV = endpoint1 - endpoint0;
    currEdge->init(startIndex, endIndex);
}

bool SkOffsetSimplePolygon(const SkPoint* inputPolygonVerts, int inputPolygonSize,
                           std::function<SkScalar(const SkPoint&)> offsetDistanceFunc,
                           SkTDArray<SkPoint>* offsetPolygon, SkTDArray<int>* polygonIndices) {
    if (inputPolygonSize < 3) {
        return false;
    }

    // need to be able to represent all the vertices in the 16-bit indices
    if (inputPolygonSize >= (1 << 16)) {
        return false;
    }

    // get winding direction
    int winding = SkGetPolygonWinding(inputPolygonVerts, inputPolygonSize);
    if (0 == winding) {
        return false;
    }

    // build normals
    SkAutoSTMalloc<64, SkVector> normal0(inputPolygonSize);
    SkAutoSTMalloc<64, SkVector> normal1(inputPolygonSize);
    SkScalar currOffset = offsetDistanceFunc(inputPolygonVerts[0]);
    if (!SkScalarIsFinite(currOffset)) {
        return false;
    }
    for (int curr = 0; curr < inputPolygonSize; ++curr) {
        if (!inputPolygonVerts[curr].isFinite()) {
            return false;
        }
        int next = (curr + 1) % inputPolygonSize;
        SkScalar nextOffset = offsetDistanceFunc(inputPolygonVerts[next]);
        if (!SkScalarIsFinite(nextOffset)) {
            return false;
        }
        if (!compute_offset_vectors(inputPolygonVerts[curr], inputPolygonVerts[next],
                                    currOffset, nextOffset, winding,
                                    &normal0[curr], &normal1[next])) {
            return false;
        }
        currOffset = nextOffset;
    }

    // build initial offset edge list
    SkSTArray<64, OffsetEdge> edgeData(inputPolygonSize);
    int prevIndex = inputPolygonSize - 1;
    int currIndex = 0;
    int nextIndex = 1;
    while (currIndex < inputPolygonSize) {
        int side = compute_side(inputPolygonVerts[prevIndex],
                                inputPolygonVerts[currIndex] - inputPolygonVerts[prevIndex],
                                inputPolygonVerts[nextIndex]);
        SkScalar offset = offsetDistanceFunc(inputPolygonVerts[currIndex]);
        // if reflex point, fill in curve
        if (side*winding*offset < 0) {
            SkScalar rotSin, rotCos;
            int numSteps;
            SkVector prevNormal = normal1[currIndex];
            if (!SkComputeRadialSteps(prevNormal, normal0[currIndex], SkScalarAbs(offset),
                                      &rotSin, &rotCos, &numSteps)) {
                return false;
            }
            auto currEdge = edgeData.push_back_n(SkTMax(numSteps, 1));
            for (int i = 0; i < numSteps - 1; ++i) {
                SkVector currNormal = SkVector::Make(prevNormal.fX*rotCos - prevNormal.fY*rotSin,
                                                     prevNormal.fY*rotCos + prevNormal.fX*rotSin);
                setup_offset_edge(currEdge,
                                  inputPolygonVerts[currIndex] + prevNormal,
                                  inputPolygonVerts[currIndex] + currNormal,
                                  currIndex, currIndex);
                prevNormal = currNormal;
                ++currEdge;
            }
            setup_offset_edge(currEdge,
                              inputPolygonVerts[currIndex] + prevNormal,
                              inputPolygonVerts[currIndex] + normal0[currIndex],
                              currIndex, currIndex);
            ++currEdge;

        }

        // Add the edge
        auto edge = edgeData.push_back_n(1);
        setup_offset_edge(edge,
                          inputPolygonVerts[currIndex] + normal0[currIndex],
                          inputPolygonVerts[nextIndex] + normal1[nextIndex],
                          currIndex, nextIndex);

        prevIndex = currIndex;
        currIndex++;
        nextIndex = (nextIndex + 1) % inputPolygonSize;
    }

    // build linked list
    // we have to do this as a post-process step because we might have reallocated
    // the array when adding fans for reflex verts
    prevIndex = edgeData.count()-1;
    for (int currIndex = 0; currIndex < edgeData.count(); prevIndex = currIndex, ++currIndex) {
        int nextIndex = (currIndex + 1) % edgeData.count();
        edgeData[currIndex].fPrev = &edgeData[prevIndex];
        edgeData[currIndex].fNext = &edgeData[nextIndex];
    }

    // now clip edges
    int edgeDataSize = edgeData.count();
    auto head = &edgeData[0];
    auto currEdge = head;
    auto prevEdge = currEdge->fPrev;
    int offsetVertexCount = edgeDataSize;
    int iterations = 0;
    while (head && prevEdge != currEdge) {
        ++iterations;
        // we should check each edge against each other edge at most once
        if (iterations > edgeDataSize*edgeDataSize) {
            return false;
        }

        SkScalar s, t;
        SkPoint intersection;
        if (prevEdge->checkIntersection(currEdge, &intersection, &s, &t)) {
            // if new intersection is further back on previous inset from the prior intersection
            if (s < prevEdge->fTValue) {
                // no point in considering this one again
                remove_node(prevEdge, &head);
                --offsetVertexCount;
                // go back one segment
                prevEdge = prevEdge->fPrev;
                // we've already considered this intersection, we're done
            } else if (currEdge->fTValue > SK_ScalarMin &&
                       SkPointPriv::EqualsWithinTolerance(intersection,
                                                          currEdge->fIntersection,
                                                          1.0e-6f)) {
                break;
            } else {
                // add intersection
                currEdge->fIntersection = intersection;
                currEdge->fTValue = t;
                currEdge->fIndex = prevEdge->fEnd;

                // go to next segment
                prevEdge = currEdge;
                currEdge = currEdge->fNext;
            }
        } else {
            // If there is no intersection, we want to minimize the distance between
            // the point where the segment lines cross and the segments themselves.
            OffsetEdge* prevPrevEdge = prevEdge->fPrev;
            OffsetEdge* currNextEdge = currEdge->fNext;
            SkScalar dist0 = compute_crossing_distance(currEdge->fInset,
                                                       prevPrevEdge->fInset);
            SkScalar dist1 = compute_crossing_distance(prevEdge->fInset,
                                                       currNextEdge->fInset);
            if (dist0 < dist1) {
                remove_node(prevEdge, &head);
                prevEdge = prevPrevEdge;
            } else {
                remove_node(currEdge, &head);
                currEdge = currNextEdge;
            }
            --offsetVertexCount;
        }
    }

    // store all the valid intersections that aren't nearly coincident
    // TODO: look at the main algorithm and see if we can detect these better
    offsetPolygon->reset();
    if (head) {
        static constexpr SkScalar kCleanupTolerance = 0.01f;
        if (offsetVertexCount >= 0) {
            offsetPolygon->setReserve(offsetVertexCount);
        }
        int currIndex = 0;
        OffsetEdge* currEdge = head;
        *offsetPolygon->push() = currEdge->fIntersection;
        if (polygonIndices) {
            *polygonIndices->push() = currEdge->fIndex;
        }
        currEdge = currEdge->fNext;
        while (currEdge != head) {
            if (!SkPointPriv::EqualsWithinTolerance(currEdge->fIntersection,
                                                    (*offsetPolygon)[currIndex],
                                                    kCleanupTolerance)) {
                *offsetPolygon->push() = currEdge->fIntersection;
                if (polygonIndices) {
                    *polygonIndices->push() = currEdge->fIndex;
                }
                currIndex++;
            }
            currEdge = currEdge->fNext;
        }
        // make sure the first and last points aren't coincident
        if (currIndex >= 1 &&
            SkPointPriv::EqualsWithinTolerance((*offsetPolygon)[0], (*offsetPolygon)[currIndex],
                                               kCleanupTolerance)) {
            offsetPolygon->pop();
            if (polygonIndices) {
                polygonIndices->pop();
            }
        }
    }

    // check winding of offset polygon (it should be same as the original polygon)
    SkScalar offsetWinding = SkGetPolygonWinding(offsetPolygon->begin(), offsetPolygon->count());

    return (winding*offsetWinding > 0 &&
            SkIsSimplePolygon(offsetPolygon->begin(), offsetPolygon->count()));
}

//////////////////////////////////////////////////////////////////////////////////////////

struct TriangulationVertex {
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(TriangulationVertex);

    enum class VertexType { kConvex, kReflex };

    SkPoint    fPosition;
    VertexType fVertexType;
    uint16_t   fIndex;
    uint16_t   fPrevIndex;
    uint16_t   fNextIndex;
};

// test to see if point p is in triangle p0p1p2.
// for now assuming strictly inside -- if on the edge it's outside
static bool point_in_triangle(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                              const SkPoint& p) {
    SkVector v0 = p1 - p0;
    SkVector v1 = p2 - p1;
    SkScalar n = v0.cross(v1);

    SkVector w0 = p - p0;
    if (n*v0.cross(w0) < SK_ScalarNearlyZero) {
        return false;
    }

    SkVector w1 = p - p1;
    if (n*v1.cross(w1) < SK_ScalarNearlyZero) {
        return false;
    }

    SkVector v2 = p0 - p2;
    SkVector w2 = p - p2;
    if (n*v2.cross(w2) < SK_ScalarNearlyZero) {
        return false;
    }

    return true;
}

// Data structure to track reflex vertices and check whether any are inside a given triangle
class ReflexHash {
public:
    void add(TriangulationVertex* v) {
        fReflexList.addToTail(v);
    }

    void remove(TriangulationVertex* v) {
        fReflexList.remove(v);
    }

    bool checkTriangle(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                       uint16_t ignoreIndex0, uint16_t ignoreIndex1) {
        for (SkTInternalLList<TriangulationVertex>::Iter reflexIter = fReflexList.begin();
             reflexIter != fReflexList.end(); ++reflexIter) {
            TriangulationVertex* reflexVertex = *reflexIter;
            if (reflexVertex->fIndex != ignoreIndex0 && reflexVertex->fIndex != ignoreIndex1 &&
                point_in_triangle(p0, p1, p2, reflexVertex->fPosition)) {
                return true;
            }
        }

        return false;
    }

private:
    // TODO: switch to an actual spatial hash
    SkTInternalLList<TriangulationVertex> fReflexList;
};

// Check to see if a reflex vertex has become a convex vertex after clipping an ear
static void reclassify_vertex(TriangulationVertex* p, const SkPoint* polygonVerts,
                              int winding, ReflexHash* reflexHash,
                              SkTInternalLList<TriangulationVertex>* convexList) {
    if (TriangulationVertex::VertexType::kReflex == p->fVertexType) {
        SkVector v0 = p->fPosition - polygonVerts[p->fPrevIndex];
        SkVector v1 = polygonVerts[p->fNextIndex] - p->fPosition;
        if (winding*v0.cross(v1) > SK_ScalarNearlyZero*SK_ScalarNearlyZero) {
            p->fVertexType = TriangulationVertex::VertexType::kConvex;
            reflexHash->remove(p);
            p->fPrev = p->fNext = nullptr;
            convexList->addToTail(p);
        }
    }
}

bool SkTriangulateSimplePolygon(const SkPoint* polygonVerts, uint16_t* indexMap, int polygonSize,
                                SkTDArray<uint16_t>* triangleIndices) {
    if (polygonSize < 3) {
        return false;
    }
    // need to be able to represent all the vertices in the 16-bit indices
    if (polygonSize >= (1 << 16)) {
        return false;
    }

    // get winding direction
    // TODO: we do this for all the polygon routines -- might be better to have the client
    // compute it and pass it in
    int winding = SkGetPolygonWinding(polygonVerts, polygonSize);
    if (0 == winding) {
        return false;
    }

    // Classify initial vertices into a list of convex vertices and a hash of reflex vertices
    // TODO: possibly sort the convexList in some way to get better triangles
    SkTInternalLList<TriangulationVertex> convexList;
    ReflexHash reflexHash;
    SkAutoSTMalloc<64, TriangulationVertex> triangulationVertices(polygonSize);
    int prevIndex = polygonSize - 1;
    int currIndex = 0;
    int nextIndex = 1;
    SkVector v0 = polygonVerts[currIndex] - polygonVerts[prevIndex];
    SkVector v1 = polygonVerts[nextIndex] - polygonVerts[currIndex];
    for (int i = 0; i < polygonSize; ++i) {
        SkDEBUGCODE(memset(&triangulationVertices[currIndex], 0, sizeof(TriangulationVertex)));
        triangulationVertices[currIndex].fPosition = polygonVerts[currIndex];
        triangulationVertices[currIndex].fIndex = currIndex;
        triangulationVertices[currIndex].fPrevIndex = prevIndex;
        triangulationVertices[currIndex].fNextIndex = nextIndex;
        if (winding*v0.cross(v1) > SK_ScalarNearlyZero*SK_ScalarNearlyZero) {
            triangulationVertices[currIndex].fVertexType = TriangulationVertex::VertexType::kConvex;
            convexList.addToTail(&triangulationVertices[currIndex]);
        } else {
            // We treat near collinear vertices as reflex
            triangulationVertices[currIndex].fVertexType = TriangulationVertex::VertexType::kReflex;
            reflexHash.add(&triangulationVertices[currIndex]);
        }

        prevIndex = currIndex;
        currIndex = nextIndex;
        nextIndex = (currIndex + 1) % polygonSize;
        v0 = v1;
        v1 = polygonVerts[nextIndex] - polygonVerts[currIndex];
    }

    // The general concept: We are trying to find three neighboring vertices where
    // no other vertex lies inside the triangle (an "ear"). If we find one, we clip
    // that ear off, and then repeat on the new polygon. Once we get down to three vertices
    // we have triangulated the entire polygon.
    // In the worst case this is an n^2 algorithm. We can cut down the search space somewhat by
    // noting that only convex vertices can be potential ears, and we only need to check whether
    // any reflex vertices lie inside the ear.
    triangleIndices->setReserve(triangleIndices->count() + 3 * (polygonSize - 2));
    int vertexCount = polygonSize;
    while (vertexCount > 3) {
        bool success = false;
        TriangulationVertex* earVertex = nullptr;
        TriangulationVertex* p0 = nullptr;
        TriangulationVertex* p2 = nullptr;
        // find a convex vertex to clip
        for (SkTInternalLList<TriangulationVertex>::Iter convexIter = convexList.begin();
             convexIter != convexList.end(); ++convexIter) {
            earVertex = *convexIter;
            SkASSERT(TriangulationVertex::VertexType::kReflex != earVertex->fVertexType);

            p0 = &triangulationVertices[earVertex->fPrevIndex];
            p2 = &triangulationVertices[earVertex->fNextIndex];

            // see if any reflex vertices are inside the ear
            bool failed = reflexHash.checkTriangle(p0->fPosition, earVertex->fPosition,
                                                   p2->fPosition, p0->fIndex, p2->fIndex);
            if (failed) {
                continue;
            }

            // found one we can clip
            success = true;
            break;
        }
        // If we can't find any ears to clip, this probably isn't a simple polygon
        if (!success) {
            return false;
        }

        // add indices
        auto indices = triangleIndices->append(3);
        indices[0] = indexMap[p0->fIndex];
        indices[1] = indexMap[earVertex->fIndex];
        indices[2] = indexMap[p2->fIndex];

        // clip the ear
        convexList.remove(earVertex);
        --vertexCount;

        // reclassify reflex verts
        p0->fNextIndex = earVertex->fNextIndex;
        reclassify_vertex(p0, polygonVerts, winding, &reflexHash, &convexList);

        p2->fPrevIndex = earVertex->fPrevIndex;
        reclassify_vertex(p2, polygonVerts, winding, &reflexHash, &convexList);
    }

    // output indices
    for (SkTInternalLList<TriangulationVertex>::Iter vertexIter = convexList.begin();
         vertexIter != convexList.end(); ++vertexIter) {
        TriangulationVertex* vertex = *vertexIter;
        *triangleIndices->push() = indexMap[vertex->fIndex];
    }

    return true;
}
