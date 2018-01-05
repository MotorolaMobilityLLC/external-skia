/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRect.h"

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"

namespace sksg {

Rect::Rect(const SkRect& rect) : fRect(rect) {}

void Rect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawRect(fRect, paint);
}

Node::RevalidationResult Rect::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasSelfInval());

    // Geometry does not contribute damage directly.
    return { fRect, Damage::kBlockSelf };
}

SkPath Rect::onAsPath() const {
    SkPath path;
    path.addRect(fRect);
    return path;
}

RRect::RRect(const SkRRect& rr) : fRRect(rr) {}

void RRect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawRRect(fRRect, paint);
}

Node::RevalidationResult RRect::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasSelfInval());

    // Geometry does not contribute damage directly.
    return { fRRect.getBounds(), Damage::kBlockSelf };
}

SkPath RRect::onAsPath() const {
    SkPath path;
    path.addRRect(fRRect);
    return path;
}

} // namespace sksg
