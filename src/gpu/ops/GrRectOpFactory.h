/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRectOpFactory_DEFINED
#define GrRectOpFactory_DEFINED

#include "GrAAFillRectOp.h"
#include "GrAAStrokeRectOp.h"
#include "GrAnalyticRectOp.h"
#include "GrColor.h"
#include "GrNonAAFillRectOp.h"
#include "GrNonAAStrokeRectOp.h"
#include "GrPaint.h"
#include "SkMatrix.h"
#include "SkRefCnt.h"
#include "ops/GrDrawOp.h"

struct SkRect;
class SkStrokeRec;

/**
 * A factory for returning GrDrawOps which can draw rectangles.
 */
namespace GrRectOpFactory {

inline sk_sp<GrDrawOp> MakeNonAAFill(GrColor color,
                                     const SkMatrix& viewMatrix,
                                     const SkRect& rect,
                                     const SkRect* localRect,
                                     const SkMatrix* localMatrix) {
    if (viewMatrix.hasPerspective() || (localMatrix && localMatrix->hasPerspective())) {
        return GrNonAAFillRectOp::MakeWithPerspective(color, viewMatrix, rect, localRect,
                                                      localMatrix);
    } else {
        return GrNonAAFillRectOp::Make(color, viewMatrix, rect, localRect, localMatrix);
    }
}

inline sk_sp<GrDrawOp> MakeAAFill(const GrPaint& paint,
                                  const SkMatrix& viewMatrix,
                                  const SkRect& rect,
                                  const SkRect& croppedRect,
                                  const SkRect& devRect) {
    if (!paint.usesDistanceVectorField()) {
        return GrAAFillRectOp::Make(paint.getColor(), viewMatrix, croppedRect, devRect);
    } else {
        return GrAnalyticRectOp::Make(paint.getColor(), viewMatrix, rect, croppedRect, devRect);
    }
}

inline sk_sp<GrDrawOp> MakeAAFill(GrColor color,
                                  const SkMatrix& viewMatrix,
                                  const SkMatrix& localMatrix,
                                  const SkRect& rect,
                                  const SkRect& devRect) {
    return GrAAFillRectOp::Make(color, viewMatrix, localMatrix, rect, devRect);
}

inline sk_sp<GrDrawOp> MakeNonAAStroke(GrColor color,
                                       const SkMatrix& viewMatrix,
                                       const SkRect& rect,
                                       const SkStrokeRec& strokeRec,
                                       bool snapToPixelCenters) {
    return GrNonAAStrokeRectOp::Make(color, viewMatrix, rect, strokeRec, snapToPixelCenters);
}

inline sk_sp<GrDrawOp> MakeAAStroke(GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const SkStrokeRec& stroke) {
    return GrAAStrokeRectOp::Make(color, viewMatrix, rect, stroke);
}

// First rect is outer; second rect is inner
sk_sp<GrDrawOp> MakeAAFillNestedRects(GrColor, const SkMatrix& viewMatrix, const SkRect rects[2]);
};

#endif
