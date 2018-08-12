/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGMaskEffect.h"

#include "SkCanvas.h"

namespace sksg {

MaskEffect::MaskEffect(sk_sp<RenderNode> child, sk_sp<RenderNode> mask, Mode mode)
    : INHERITED(std::move(child))
    , fMaskNode(std::move(mask))
    , fMaskMode(mode) {
    this->observeInval(fMaskNode);
}

MaskEffect::~MaskEffect() {
    this->unobserveInval(fMaskNode);
}

void MaskEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    if (this->bounds().isEmpty())
        return;

    SkAutoCanvasRestore acr(canvas, false);

    canvas->saveLayer(this->bounds(), nullptr);
    // Note: the paint overrides in ctx don't apply to the mask.
    fMaskNode->render(canvas);


    SkPaint p;
    p.setBlendMode(fMaskMode == Mode::kNormal ? SkBlendMode::kSrcIn : SkBlendMode::kSrcOut);
    canvas->saveLayer(this->bounds(), &p);

    this->INHERITED::onRender(canvas, ctx);
}


SkRect MaskEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    const auto maskBounds = fMaskNode->revalidate(ic, ctm);
    auto childBounds = this->INHERITED::onRevalidate(ic, ctm);

    return (fMaskMode == Mode::kInvert || childBounds.intersect(maskBounds))
        ? childBounds
        : SkRect::MakeEmpty();
}

} // namespace sksg
