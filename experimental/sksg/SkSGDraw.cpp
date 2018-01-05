/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGDraw.h"

#include "SkSGGeometryNode.h"
#include "SkSGInvalidationController.h"
#include "SkSGPaintNode.h"

namespace sksg {

Draw::Draw(sk_sp<GeometryNode> geometry, sk_sp<PaintNode> paint)
    : fGeometry(std::move(geometry))
    , fPaint(std::move(paint)) {
    fGeometry->addInvalReceiver(this);
    fPaint->addInvalReceiver(this);
}

Draw::~Draw() {
    fGeometry->removeInvalReceiver(this);
    fPaint->removeInvalReceiver(this);
}

void Draw::onRender(SkCanvas* canvas) const {
    fGeometry->draw(canvas, fPaint->makePaint());
}

Node::RevalidationResult Draw::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // TODO: adjust bounds for paint
    const auto bounds = fGeometry->revalidate(ic, ctm);
    fPaint->revalidate(ic, ctm);

    // Neither paint nor geometry contribute to damage directly; instead we generate
    // damage here, at the binding point.
    return { bounds, Damage::kForceSelf };
}

} // namespace sksg
