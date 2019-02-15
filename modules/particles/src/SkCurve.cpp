/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkCurve.h"

#include "SkRandom.h"
#include "SkReflected.h"

static SkScalar eval_cubic(const SkScalar* pts, SkScalar x) {
    SkScalar ix = (1 - x);
    return pts[0]*ix*ix*ix + pts[1]*3*ix*ix*x + pts[2]*3*ix*x*x + pts[3]*x*x*x;
}

SkScalar SkCurveSegment::eval(SkScalar x, SkRandom& random) const {
    SkScalar result = fConstant ? fMin[0] : eval_cubic(fMin, x);
    if (fRanged) {
        result += ((fConstant ? fMax[0] : eval_cubic(fMax, x)) - result) * random.nextF();
    }
    if (fBidirectional && random.nextBool()) {
        result = -result;
    }
    return result;
}

void SkCurveSegment::visitFields(SkFieldVisitor* v) {
    v->visit("Constant", fConstant);
    v->visit("Ranged", fRanged);
    v->visit("Bidirectional", fBidirectional);
    v->visit("A0", fMin[0]);
    v->visit("B0", fMin[1]);
    v->visit("C0", fMin[2]);
    v->visit("D0", fMin[3]);
    v->visit("A1", fMax[0]);
    v->visit("B1", fMax[1]);
    v->visit("C1", fMax[2]);
    v->visit("D1", fMax[3]);
}

SkScalar SkCurve::eval(SkScalar x, SkRandom& random) const {
    SkASSERT(fSegments.count() == fXValues.count() + 1);

    int i = 0;
    for (; i < fXValues.count(); ++i) {
        if (x <= fXValues[i]) {
            break;
        }
    }

    SkScalar rangeMin = (i == 0) ? 0.0f : fXValues[i - 1];
    SkScalar rangeMax = (i == fXValues.count()) ? 1.0f : fXValues[i];
    SkScalar segmentX = (x - rangeMin) / (rangeMax - rangeMin);
    SkASSERT(0.0f <= segmentX && segmentX <= 1.0f);
    return fSegments[i].eval(segmentX, random);
}

void SkCurve::visitFields(SkFieldVisitor* v) {
    v->visit("XValues", fXValues);
    v->visit("Segments", fSegments);

    // Validate and fixup
    if (fSegments.empty()) {
        fSegments.push_back().setConstant(0.0f);
    }
    fXValues.resize_back(fSegments.count() - 1);
    for (int i = 0; i < fXValues.count(); ++i) {
        fXValues[i] = SkTPin(fXValues[i], i > 0 ? fXValues[i - 1] : 0.0f, 1.0f);
    }
}
