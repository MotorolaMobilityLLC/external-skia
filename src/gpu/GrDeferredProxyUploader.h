/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDeferredProxyUploader_DEFINED
#define GrDeferredProxyUploader_DEFINED

#include "SkAutoPixmapStorage.h"
#include "SkMakeUnique.h"
#include "SkRefCnt.h"
#include "SkSemaphore.h"

#include "GrOpFlushState.h"
#include "GrTextureProxyPriv.h"

/**
 * GrDeferredProxyUploader assists with threaded generation of textures. Currently used by both
 * software clip masks, and the software path renderer. The calling code typically needs to store
 * some additional data (T) for use on the worker thread. GrTDeferredProxyUploader allows storing
 * such data. The common flow is:
 *
 * 1) A GrTDeferredProxyUploader is created, with some payload (eg an SkPath to draw).
 *    The uploader is owned by the proxy that it's going to populate.
 * 2) A task is created with a pointer to the uploader. A worker thread executes that task, using
 *    the payload data to allocate and fill in the fPixels pixmap.
 * 3) The worker thread calls signalAndFreeData(), which notifies the main thread that the pixmap
 *    is ready, and then deletes the payload data (which is no longer needed).
 * 4) In parallel to 2-3, on the main thread... Some op is created that refers to the proxy. When
 *    that op is added to an op list, the op list retains a pointer to the "deferred" proxies.
 * 5) At flush time, the op list ensures that the deferred proxies are instantiated, then calls
 *    scheduleUpload on those proxies, which calls scheduleUpload on the uploader (below).
 * 6) scheduleUpload defers the upload even further, by adding an ASAPUpload to the flush.
 * 7) When the ASAP upload happens, we wait to make sure that the pixels are marked ready
 *    (from step #3 on the worker thread). Then we perform the actual upload to the texture.
 *    Finally, we call resetDeferredUploader, which deletes the uploader object, causing fPixels
 *    to be freed.
 */
class GrDeferredProxyUploader : public SkNoncopyable {
public:
    GrDeferredProxyUploader() : fScheduledUpload(false), fWaited(false) {}

    virtual ~GrDeferredProxyUploader() {
        if (!fWaited) {
            // This can happen if our owning proxy fails to instantiate
            fPixelsReady.wait();
        }
    }

    void scheduleUpload(GrOpFlushState* flushState, GrTextureProxy* proxy) {
        if (fScheduledUpload) {
            // Multiple references to the owning proxy may have caused us to already execute
            return;
        }

        auto uploadMask = [this, proxy](GrDrawOp::WritePixelsFn& writePixelsFn) {
            this->fPixelsReady.wait();
            this->fWaited = true;
            // If the worker thread was unable to allocate pixels, this check will fail, and we'll
            // end up drawing with an uninitialized mask texture, but at least we won't crash.
            if (this->fPixels.addr()) {
                writePixelsFn(proxy, 0, 0, this->fPixels.width(), this->fPixels.height(),
                              proxy->config(), this->fPixels.addr(), this->fPixels.rowBytes());
            }
            // Upload has finished, so tell the proxy to release this GrDeferredProxyUploader
            proxy->texPriv().resetDeferredUploader();
        };
        flushState->addASAPUpload(std::move(uploadMask));
        fScheduledUpload = true;
    }

    void signalAndFreeData() {
        fPixelsReady.signal();
        this->freeData();
    }

    SkAutoPixmapStorage* getPixels() { return &fPixels; }

private:
    virtual void freeData() {}

    SkAutoPixmapStorage fPixels;
    SkSemaphore fPixelsReady;
    bool fScheduledUpload;
    bool fWaited;
};

template <typename T>
class GrTDeferredProxyUploader : public GrDeferredProxyUploader {
public:
    template <typename... Args>
    GrTDeferredProxyUploader(Args&&... args)
        : fData(skstd::make_unique<T>(std::forward<Args>(args)...)) {
    }

    T& data() { return *fData; }

private:
    void freeData() override {
        fData.reset();
    }

    std::unique_ptr<T> fData;
};

#endif
