/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrRenderTarget.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrGpu.h"
#include "GrRenderTargetOpList.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilAttachment.h"

GrRenderTarget::GrRenderTarget(GrGpu* gpu, const GrSurfaceDesc& desc, Flags flags,
                               GrStencilAttachment* stencil)
    : INHERITED(gpu, desc)
    , fStencilAttachment(stencil)
    , fMultisampleSpecsID(0)
    , fFlags(flags) {
    SkASSERT(!(fFlags & Flags::kMixedSampled) || fDesc.fSampleCnt > 0);
    SkASSERT(!(fFlags & Flags::kWindowRectsSupport) || gpu->caps()->maxWindowRectangles() > 0);
    fResolveRect.setLargestInverted();
}

void GrRenderTarget::discard() {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (!context) {
        return;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(
        context->contextPriv().makeWrappedRenderTargetContext(sk_ref_sp(this), nullptr));
    if (!renderTargetContext) {
        return;
    }

    renderTargetContext->discard();
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
    SkSafeSetNull(fStencilAttachment);

    INHERITED::onRelease();
}

void GrRenderTarget::onAbandon() {
    SkSafeSetNull(fStencilAttachment);

    // The contents of this renderTarget are gone/invalid. It isn't useful to point back
    // the creating opList.
    this->setLastOpList(nullptr);

    INHERITED::onAbandon();
}

///////////////////////////////////////////////////////////////////////////////

bool GrRenderTargetPriv::attachStencilAttachment(GrStencilAttachment* stencil) {
    if (!stencil && !fRenderTarget->fStencilAttachment) {
        // No need to do any work since we currently don't have a stencil attachment and
        // we're not acctually adding one.
        return true;
    }
    fRenderTarget->fStencilAttachment = stencil;
    if (!fRenderTarget->completeStencilAttachment()) {
        SkSafeSetNull(fRenderTarget->fStencilAttachment);
        return false;
    }
    return true;
}

int GrRenderTargetPriv::numStencilBits() const {
    return fRenderTarget->fStencilAttachment ? fRenderTarget->fStencilAttachment->bits() : 0;
}

const GrGpu::MultisampleSpecs&
GrRenderTargetPriv::getMultisampleSpecs(const GrStencilSettings& stencil) const {
    return fRenderTarget->getGpu()->getMultisampleSpecs(fRenderTarget, stencil);
}

int GrRenderTargetPriv::maxWindowRectangles() const {
    return (this->flags() & Flags::kWindowRectsSupport) ?
           fRenderTarget->getGpu()->caps()->maxWindowRectangles() : 0;
}
