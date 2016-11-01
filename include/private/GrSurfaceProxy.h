/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxy_DEFINED
#define GrSurfaceProxy_DEFINED

#include "GrGpuResource.h"
#include "GrSurface.h"
#include "SkRect.h"

class GrOpList;
class GrTextureProxy;
class GrRenderTargetProxy;

// This class replicates the functionality GrIORef<GrSurface> but tracks the
// utilitization for later resource allocation (for the deferred case) and
// forwards on the utilization in the wrapped case
class GrIORefProxy : public SkNoncopyable {
public:
    void ref() const {
        this->validate();

        ++fRefCnt;
        if (fTarget) {
            fTarget->ref();
        }
    }

    void unref() const {
        this->validate();

        if (fTarget) {
            fTarget->unref();
        }

        if (!(--fRefCnt)) {
            delete this;
            return;
        }

        this->validate();
    }

    void validate() const {
#ifdef SK_DEBUG    
        SkASSERT(fRefCnt >= 0);
#endif
    }

protected:
    GrIORefProxy() : fRefCnt(1), fTarget(nullptr) {}
    GrIORefProxy(sk_sp<GrSurface> surface) : fRefCnt(1) {
        // Since we're manually forwarding on refs & unrefs we don't want sk_sp doing
        // anything extra.
        fTarget = surface.release();
    }
    virtual ~GrIORefProxy() {
        // We don't unref 'fTarget' here since the 'unref' method will already
        // have forwarded on the unref call that got use here.
    }

    // TODO: add the IO ref counts. Although if we can delay shader creation to flush time
    // we may not even need to do that.
    mutable int32_t fRefCnt;

    // For deferred proxies this will be null. For wrapped proxies it will point to the
    // wrapped resource.
    GrSurface* fTarget;
};

class GrSurfaceProxy : public GrIORefProxy {
public:
    const GrSurfaceDesc& desc() const { return fDesc; }

    GrSurfaceOrigin origin() const {
        SkASSERT(kTopLeft_GrSurfaceOrigin == fDesc.fOrigin ||
                 kBottomLeft_GrSurfaceOrigin == fDesc.fOrigin);
        return fDesc.fOrigin;
    }
    int width() const { return fDesc.fWidth; }
    int height() const { return fDesc.fHeight; }
    GrPixelConfig config() const { return fDesc.fConfig; }

    uint32_t uniqueID() const { return fUniqueID; }

    /**
     * Helper that gets the width and height of the surface as a bounding rectangle.
     */
    SkRect getBoundsRect() const { return SkRect::MakeIWH(this->width(), this->height()); }
  
    /**
     * @return the texture proxy associated with the surface proxy, may be NULL.
     */
    virtual GrTextureProxy* asTextureProxy() { return nullptr; }
    virtual const GrTextureProxy* asTextureProxy() const { return nullptr; }

    /**
     * @return the render target proxy associated with the surface proxy, may be NULL.
     */
    virtual GrRenderTargetProxy* asRenderTargetProxy() { return nullptr; }
    virtual const GrRenderTargetProxy* asRenderTargetProxy() const { return nullptr; }

    /**
     * Does the resource count against the resource budget?
     */
    SkBudgeted isBudgeted() const { return fBudgeted; }

    void setLastOpList(GrOpList* opList);
    GrOpList* getLastOpList() { return fLastOpList; }

protected:
    // Deferred version
    GrSurfaceProxy(const GrSurfaceDesc& desc, SkBackingFit fit, SkBudgeted budgeted)
        : fDesc(desc)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fUniqueID(GrGpuResource::CreateUniqueID())
        , fLastOpList(nullptr) {
    }

    // Wrapped version
    GrSurfaceProxy(sk_sp<GrSurface> surface, SkBackingFit fit);

    virtual ~GrSurfaceProxy();

    // For wrapped resources, 'fDesc' will always be filled in from the wrapped resource.
    const GrSurfaceDesc fDesc;
    const SkBackingFit  fFit;      // always exact for wrapped resources
    const SkBudgeted    fBudgeted; // set from the backing resource for wrapped resources
    const uint32_t      fUniqueID; // set from the backing resource for wrapped resources

private:
    // The last opList that wrote to or is currently going to write to this surface
    // The opList can be closed (e.g., no render target context is currently bound
    // to this renderTarget).
    // This back-pointer is required so that we can add a dependancy between
    // the opList used to create the current contents of this surface
    // and the opList of a destination surface to which this one is being drawn or copied.
    GrOpList* fLastOpList;

    typedef GrIORefProxy INHERITED;
};

#endif
