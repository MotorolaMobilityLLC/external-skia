/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRenderTask.h"

#include "src/gpu/GrAttachment.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/GrTextureResolveRenderTask.h"

uint32_t GrRenderTask::CreateUniqueID() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);
    return id;
}

GrRenderTask::GrRenderTask()
        : fUniqueID(CreateUniqueID())
        , fFlags(0) {
}

void GrRenderTask::disown(GrDrawingManager* drawingMgr) {
    SkASSERT(!fDrawingMgr || drawingMgr == fDrawingMgr);
    SkASSERT(this->isClosed());
    if (this->isSetFlag(kDisowned_Flag)) {
        return;
    }
    SkDEBUGCODE(fDrawingMgr = nullptr);
    this->setFlag(kDisowned_Flag);

    for (const sk_sp<GrSurfaceProxy>& target : fTargets) {
        if (this == drawingMgr->getLastRenderTask(target.get())) {
            drawingMgr->setLastRenderTask(target.get(), nullptr);
        }
    }
}

void GrRenderTask::canSkip() {
    SkASSERT(this->isClosed());
    this->onCanSkip();
}

#ifdef SK_DEBUG
GrRenderTask::~GrRenderTask() {
    SkASSERT(this->isSetFlag(kDisowned_Flag));
}

bool GrRenderTask::deferredProxiesAreInstantiated() const {
    for (int i = 0; i < fDeferredProxies.count(); ++i) {
        if (!fDeferredProxies[i]->isInstantiated()) {
            return false;
        }
    }

    return true;
}
#endif

void GrRenderTask::makeClosed(const GrCaps& caps) {
    if (this->isClosed()) {
        return;
    }

    SkIRect targetUpdateBounds;
    if (ExpectedOutcome::kTargetDirty == this->onMakeClosed(caps, &targetUpdateBounds)) {
        GrSurfaceProxy* proxy = this->target(0);
        if (proxy->requiresManualMSAAResolve()) {
            SkASSERT(this->target(0)->asRenderTargetProxy());
            this->target(0)->asRenderTargetProxy()->markMSAADirty(targetUpdateBounds);
        }
        GrTextureProxy* textureProxy = this->target(0)->asTextureProxy();
        if (textureProxy && GrMipmapped::kYes == textureProxy->mipmapped()) {
            textureProxy->markMipmapsDirty();
        }
    }

    if (fTextureResolveTask) {
        this->addDependency(fTextureResolveTask);
        fTextureResolveTask->makeClosed(caps);
        fTextureResolveTask = nullptr;
    }

    this->setFlag(kClosed_Flag);
}

void GrRenderTask::prepare(GrOpFlushState* flushState) {
    for (int i = 0; i < fDeferredProxies.count(); ++i) {
        fDeferredProxies[i]->texPriv().scheduleUpload(flushState);
    }

    this->onPrepare(flushState);
}

// Add a GrRenderTask-based dependency
void GrRenderTask::addDependency(GrRenderTask* dependedOn) {
    SkASSERT(!dependedOn->dependsOn(this));  // loops are bad
    SkASSERT(!this->dependsOn(dependedOn));  // caller should weed out duplicates

    fDependencies.push_back(dependedOn);
    dependedOn->addDependent(this);

    SkDEBUGCODE(this->validate());
}

void GrRenderTask::addDependenciesFromOtherTask(GrRenderTask* otherTask) {
    SkASSERT(otherTask);
    for (GrRenderTask* task : otherTask->fDependencies) {
        // The task should not be adding a dependency to itself.
        SkASSERT(task != this);
        if (!this->dependsOn(task)) {
            this->addDependency(task);
        }
    }
}

// Convert from a GrSurface-based dependency to a GrRenderTask one
void GrRenderTask::addDependency(GrDrawingManager* drawingMgr, GrSurfaceProxy* dependedOn,
                                 GrMipmapped mipMapped,
                                 GrTextureResolveManager textureResolveManager,
                                 const GrCaps& caps) {
    // If it is still receiving dependencies, this GrRenderTask shouldn't be closed
    SkASSERT(!this->isClosed());

    GrRenderTask* dependedOnTask = drawingMgr->getLastRenderTask(dependedOn);

    if (dependedOnTask == this) {
        // self-read - presumably for dst reads. We don't need to do anything in this case. The
        // XferProcessor will detect what is happening and insert a texture barrier.
        SkASSERT(GrMipmapped::kNo == mipMapped);
        // We should never attempt a self-read on a surface that has a separate MSAA renderbuffer.
        SkASSERT(!dependedOn->requiresManualMSAAResolve());
        SkASSERT(!dependedOn->asTextureProxy() ||
                 !dependedOn->asTextureProxy()->texPriv().isDeferred());
        return;
    }

    if (dependedOnTask) {
        if (this->dependsOn(dependedOnTask) || fTextureResolveTask == dependedOnTask) {
            return;  // don't add duplicate dependencies
        }

        // We are closing 'dependedOnTask' here bc the current contents of it are what 'this'
        // renderTask depends on. We need a break in 'dependedOnTask' so that the usage of
        // that state has a chance to execute.
        dependedOnTask->makeClosed(caps);
    }

    auto resolveFlags = GrSurfaceProxy::ResolveFlags::kNone;

    if (dependedOn->requiresManualMSAAResolve()) {
        auto* renderTargetProxy = dependedOn->asRenderTargetProxy();
        SkASSERT(renderTargetProxy);
        if (renderTargetProxy->isMSAADirty()) {
            resolveFlags |= GrSurfaceProxy::ResolveFlags::kMSAA;
        }
    }

    GrTextureProxy* textureProxy = dependedOn->asTextureProxy();
    if (GrMipmapped::kYes == mipMapped) {
        SkASSERT(textureProxy);
        if (GrMipmapped::kYes != textureProxy->mipmapped()) {
            // There are some cases where we might be given a non-mipmapped texture with a mipmap
            // filter. See skbug.com/7094.
            mipMapped = GrMipmapped::kNo;
        } else if (textureProxy->mipmapsAreDirty()) {
            resolveFlags |= GrSurfaceProxy::ResolveFlags::kMipMaps;
        }
    }

    // Does this proxy have msaa to resolve and/or mipmaps to regenerate?
    if (GrSurfaceProxy::ResolveFlags::kNone != resolveFlags) {
        if (!fTextureResolveTask) {
            fTextureResolveTask = textureResolveManager.newTextureResolveRenderTask(caps);
        }
        fTextureResolveTask->addProxy(drawingMgr, sk_ref_sp(dependedOn), resolveFlags, caps);

        // addProxy() should have closed the texture proxy's previous task.
        SkASSERT(!dependedOnTask || dependedOnTask->isClosed());
        SkASSERT(drawingMgr->getLastRenderTask(dependedOn) == fTextureResolveTask);

#ifdef SK_DEBUG
        // addProxy() should have called addDependency (in this instance, recursively) on
        // fTextureResolveTask.
        if (dependedOnTask) {
            SkASSERT(fTextureResolveTask->dependsOn(dependedOnTask));
        }
        if (textureProxy && textureProxy->texPriv().isDeferred()) {
            SkASSERT(fTextureResolveTask->fDeferredProxies.back() == textureProxy);
        }

        // The GrTextureResolveRenderTask factory should have also marked the proxy clean, set the
        // last renderTask on the textureProxy to textureResolveTask, and closed textureResolveTask.
        if (GrRenderTargetProxy* renderTargetProxy = dependedOn->asRenderTargetProxy()) {
            SkASSERT(!renderTargetProxy->isMSAADirty());
        }
        if (textureProxy) {
            SkASSERT(!textureProxy->mipmapsAreDirty());
        }
        SkASSERT(drawingMgr->getLastRenderTask(dependedOn) == fTextureResolveTask);
#endif
        return;
    }

    if (textureProxy && textureProxy->texPriv().isDeferred()) {
        fDeferredProxies.push_back(textureProxy);
    }

    if (dependedOnTask) {
        this->addDependency(dependedOnTask);
    }
}

bool GrRenderTask::dependsOn(const GrRenderTask* dependedOn) const {
    for (int i = 0; i < fDependencies.count(); ++i) {
        if (fDependencies[i] == dependedOn) {
            return true;
        }
    }

    return false;
}


void GrRenderTask::addDependent(GrRenderTask* dependent) {
    fDependents.push_back(dependent);
}

#ifdef SK_DEBUG
bool GrRenderTask::isDependent(const GrRenderTask* dependent) const {
    for (int i = 0; i < fDependents.count(); ++i) {
        if (fDependents[i] == dependent) {
            return true;
        }
    }

    return false;
}

void GrRenderTask::validate() const {
    // TODO: check for loops and duplicates

    for (int i = 0; i < fDependencies.count(); ++i) {
        SkASSERT(fDependencies[i]->isDependent(this));
    }
}
#endif

void GrRenderTask::closeThoseWhoDependOnMe(const GrCaps& caps) {
    for (int i = 0; i < fDependents.count(); ++i) {
        if (!fDependents[i]->isClosed()) {
            fDependents[i]->makeClosed(caps);
        }
    }
}

bool GrRenderTask::isInstantiated() const {
    for (const sk_sp<GrSurfaceProxy>& target : fTargets) {
        GrSurfaceProxy* proxy = target.get();
        if (!proxy->isInstantiated()) {
            return false;
        }

        GrSurface* surface = proxy->peekSurface();
        if (surface->wasDestroyed()) {
            return false;
        }
    }

    return true;
}

void GrRenderTask::addTarget(GrDrawingManager* drawingMgr, sk_sp<GrSurfaceProxy> proxy) {
    SkASSERT(proxy);
    SkASSERT(!this->isClosed());
    SkASSERT(!fDrawingMgr || drawingMgr == fDrawingMgr);
    SkDEBUGCODE(fDrawingMgr = drawingMgr);
    drawingMgr->setLastRenderTask(proxy.get(), this);
    proxy->isUsedAsTaskTarget();
    fTargets.emplace_back(std::move(proxy));
}

#if GR_TEST_UTILS
void GrRenderTask::dump(const SkString& label,
                        SkString indent,
                        bool printDependencies,
                        bool close) const {
    SkDebugf("%s%s --------------------------------------------------------------\n",
             indent.c_str(),
             label.c_str());
    SkDebugf("%s%s task - renderTaskID: %d\n", indent.c_str(), this->name(), fUniqueID);

    if (!fTargets.empty()) {
        SkDebugf("%sTargets: \n", indent.c_str());
        for (const sk_sp<GrSurfaceProxy>& target : fTargets) {
            SkASSERT(target);
            SkString proxyStr = target->dump();
            SkDebugf("%s%s\n", indent.c_str(), proxyStr.c_str());
        }
    }

    if (printDependencies) {
        SkDebugf("%sI rely On (%d): ", indent.c_str(), fDependencies.count());
        for (int i = 0; i < fDependencies.count(); ++i) {
            SkDebugf("%d, ", fDependencies[i]->fUniqueID);
        }
        SkDebugf("\n");

        SkDebugf("%s(%d) Rely On Me: ", indent.c_str(), fDependents.count());
        for (int i = 0; i < fDependents.count(); ++i) {
            SkDebugf("%d, ", fDependents[i]->fUniqueID);
        }
        SkDebugf("\n");
    }

    if (close) {
        SkDebugf("%s--------------------------------------------------------------\n\n",
                 indent.c_str());
    }
}
#endif
