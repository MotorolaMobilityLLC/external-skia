/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetProxy_DEFINED
#define GrRenderTargetProxy_DEFINED

#include "GrSurfaceProxy.h"
#include "GrTypesPriv.h"

class GrResourceProvider;

// This class delays the acquisition of RenderTargets until they are actually
// required
// Beware: the uniqueID of the RenderTargetProxy will usually be different than
// the uniqueID of the RenderTarget it represents!
class GrRenderTargetProxy : virtual public GrSurfaceProxy {
public:
    GrRenderTargetProxy* asRenderTargetProxy() override { return this; }
    const GrRenderTargetProxy* asRenderTargetProxy() const override { return this; }

    // Actually instantiate the backing rendertarget, if necessary.
    bool instantiate(GrResourceProvider*) override;

    GrFSAAType fsaaType() const {
        if (fSampleCnt <= 1) {
            SkASSERT(!this->hasMixedSamples());
            return GrFSAAType::kNone;
        }
        return this->hasMixedSamples() ? GrFSAAType::kMixedSamples : GrFSAAType::kUnifiedMSAA;
    }

    /*
     * When instantiated does this proxy require a stencil buffer?
     */
    void setNeedsStencil() { fNeedsStencil = true; }
    bool needsStencil() const { return fNeedsStencil; }

    /**
     * Returns the number of samples/pixel in the stencil buffer (One if non-MSAA).
     */
    int numStencilSamples() const { return fSampleCnt; }

    /**
     * Returns the number of samples/pixel in the color buffer (One if non-MSAA or mixed sampled).
     */
    int numColorSamples() const {
        return GrFSAAType::kMixedSamples == this->fsaaType() ? 1 : fSampleCnt;
    }

    int maxWindowRectangles(const GrCaps& caps) const;

    // TODO: move this to a priv class!
    bool refsWrappedObjects() const;

protected:
    friend class GrProxyProvider;  // for ctors

    // Deferred version
    GrRenderTargetProxy(const GrCaps&, const GrSurfaceDesc&, GrSurfaceOrigin, SkBackingFit,
                        SkBudgeted, GrInternalSurfaceFlags);

    // Lazy-callback version
    // There are two main use cases for lazily-instantiated proxies:
    //   basic knowledge - width, height, config, samples, origin are known
    //   minimal knowledge - only config is known.
    //
    // The basic knowledge version is used for DDL where we know the type of proxy we are going to
    // use, but we don't have access to the GPU yet to instantiate it.
    //
    // The minimal knowledge version is used for CCPR where we are generating an atlas but we do not
    // know the final size until flush time.
    GrRenderTargetProxy(LazyInstantiateCallback&&, LazyInstantiationType lazyType,
                        const GrSurfaceDesc&, GrSurfaceOrigin, SkBackingFit, SkBudgeted,
                        GrInternalSurfaceFlags);

    // Wrapped version
    GrRenderTargetProxy(sk_sp<GrSurface>, GrSurfaceOrigin);

    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

private:
    size_t onUninstantiatedGpuMemorySize() const override;
    SkDEBUGCODE(void validateLazySurface(const GrSurface*) override;)

    int                 fSampleCnt;
    bool                fNeedsStencil;

    // For wrapped render targets the actual GrRenderTarget is stored in the GrIORefProxy class.
    // For deferred proxies that pointer is filled in when we need to instantiate the
    // deferred resource.

    typedef GrSurfaceProxy INHERITED;
};

#endif
