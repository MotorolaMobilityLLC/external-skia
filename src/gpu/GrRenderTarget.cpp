
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrRenderTarget.h"

#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrGpu.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilAttachment.h"

void GrRenderTarget::discard() {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (!context) {
        return;
    }

    SkAutoTUnref<GrDrawContext> drawContext(context->drawContext());
    if (!drawContext) {
        return;
    }

    drawContext->discard(this);
}

void GrRenderTarget::flagAsNeedingResolve(const SkIRect* rect) {
    if (kCanResolve_ResolveType == getResolveType()) {
        if (rect) {
            fResolveRect.join(*rect);
            if (!fResolveRect.intersect(0, 0, this->width(), this->height())) {
                fResolveRect.setEmpty();
            }
        } else {
            fResolveRect.setLTRB(0, 0, this->width(), this->height());
        }
    }
}

void GrRenderTarget::overrideResolveRect(const SkIRect rect) {
    fResolveRect = rect;
    if (fResolveRect.isEmpty()) {
        fResolveRect.setLargestInverted();
    } else {
        if (!fResolveRect.intersect(0, 0, this->width(), this->height())) {
            fResolveRect.setLargestInverted();
        }
    }
}

void GrRenderTarget::onRelease() {
    this->renderTargetPriv().didAttachStencilAttachment(nullptr);

    INHERITED::onRelease();
}

void GrRenderTarget::onAbandon() {
    this->renderTargetPriv().didAttachStencilAttachment(nullptr);

    INHERITED::onAbandon();
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetPriv::didAttachStencilAttachment(GrStencilAttachment* stencilAttachment) {
    SkRefCnt_SafeAssign(fRenderTarget->fStencilAttachment, stencilAttachment);
}

GrStencilAttachment* GrRenderTargetPriv::attachStencilAttachment() const {
    if (fRenderTarget->fStencilAttachment) {
        return fRenderTarget->fStencilAttachment;
    }
    if (!fRenderTarget->wasDestroyed() && fRenderTarget->canAttemptStencilAttachment()) {
        fRenderTarget->getGpu()->attachStencilAttachmentToRenderTarget(fRenderTarget);
    }
    return fRenderTarget->fStencilAttachment;
}
