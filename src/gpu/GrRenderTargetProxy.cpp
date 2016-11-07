/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRenderTargetProxy.h"

#include "GrCaps.h"
#include "GrRenderTargetOpList.h"
#include "GrRenderTargetPriv.h"
#include "GrTextureProvider.h"
#include "GrTextureRenderTargetProxy.h"

// Deferred version
// TODO: we can probably munge the 'desc' in both the wrapped and deferred
// cases to make the sampleConfig/numSamples stuff more rational.
GrRenderTargetProxy::GrRenderTargetProxy(const GrCaps& caps, const GrSurfaceDesc& desc,
                                         SkBackingFit fit, SkBudgeted budgeted)
    : INHERITED(desc, fit, budgeted)
    , fFlags(GrRenderTarget::Flags::kNone) {
    // Since we know the newly created render target will be internal, we are able to precompute
    // what the flags will ultimately end up being.
    if (caps.usesMixedSamples() && fDesc.fSampleCnt > 0) {
        fFlags |= GrRenderTarget::Flags::kMixedSampled;
    }
    if (caps.maxWindowRectangles() > 0) {
        fFlags |= GrRenderTarget::Flags::kWindowRectsSupport;
    }
}

// Wrapped version
GrRenderTargetProxy::GrRenderTargetProxy(sk_sp<GrRenderTarget> rt)
    : INHERITED(std::move(rt), SkBackingFit::kExact)
    , fFlags(fTarget->asRenderTarget()->renderTargetPriv().flags()) {
}

GrRenderTarget* GrRenderTargetProxy::instantiate(GrTextureProvider* texProvider) {
    if (fTarget) {
        return fTarget->asRenderTarget();
    }

    // TODO: it would be nice to not have to copy the desc here
    GrSurfaceDesc desc = fDesc;
    desc.fFlags |= GrSurfaceFlags::kRenderTarget_GrSurfaceFlag;

    if (SkBackingFit::kApprox == fFit) {
        fTarget = texProvider->createApproxTexture(desc);
    } else {
        fTarget = texProvider->createTexture(desc, fBudgeted);
    }
    if (!fTarget) {
        return nullptr;
    }

#ifdef SK_DEBUG
    if (kInvalidGpuMemorySize != this->getRawGpuMemorySize_debugOnly()) {
        SkASSERT(fTarget->gpuMemorySize() <= this->getRawGpuMemorySize_debugOnly());    
    }
#endif

    // Check that our a priori computation matched the ultimate reality
    SkASSERT(fFlags == fTarget->asRenderTarget()->renderTargetPriv().flags());

    return fTarget->asRenderTarget();
}


#ifdef SK_DEBUG
void GrRenderTargetProxy::validate(GrContext* context) const {
    if (fTarget) {
        SkASSERT(fTarget->getContext() == context);
    }

    INHERITED::validate();
}
#endif

size_t GrRenderTargetProxy::onGpuMemorySize() const {
    if (fTarget) {
        return fTarget->gpuMemorySize();
    }

    // TODO: do we have enough information to improve this worst case estimate?
    return GrSurface::ComputeSize(fDesc, fDesc.fSampleCnt+1, false);
}

sk_sp<GrRenderTargetProxy> GrRenderTargetProxy::Make(const GrCaps& caps,
                                                     const GrSurfaceDesc& desc,
                                                     SkBackingFit fit,
                                                     SkBudgeted budgeted) {
    // We know anything we instantiate later from this deferred path will be
    // both texturable and renderable
    return GrTextureRenderTargetProxy::Make(caps, desc, fit, budgeted);
}

sk_sp<GrRenderTargetProxy> GrRenderTargetProxy::Make(sk_sp<GrRenderTarget> rt) {
    if (rt->asTexture()) {
        return GrTextureRenderTargetProxy::Make(std::move(rt));
    }

    // Not texturable
    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(rt));
}

