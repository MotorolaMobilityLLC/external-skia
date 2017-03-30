/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContextPriv.h"
#include "GrRenderTargetPriv.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxy.h"
#include "GrTextureProxy.h"

int32_t GrIORefProxy::getProxyRefCnt_TestOnly() const {
    return fRefCnt;
}

int32_t GrIORefProxy::getBackingRefCnt_TestOnly() const {
    if (fTarget) {
        return fTarget->fRefCnt;
    }

    return fRefCnt;
}

int32_t GrIORefProxy::getPendingReadCnt_TestOnly() const {
    if (fTarget) {
        SkASSERT(!fPendingReads);
        return fTarget->fPendingReads;
    }

    return fPendingReads;
}

int32_t GrIORefProxy::getPendingWriteCnt_TestOnly() const {
    if (fTarget) {
        SkASSERT(!fPendingWrites);
        return fTarget->fPendingWrites;
    }

    return fPendingWrites;
}

static const int kWidthHeight = 128;

static void check_refs(skiatest::Reporter* reporter,
                       GrTextureProxy* proxy,
                       int32_t expectedProxyRefs,
                       int32_t expectedBackingRefs,
                       int32_t expectedNumReads,
                       int32_t expectedNumWrites) {
    REPORTER_ASSERT(reporter, proxy->getProxyRefCnt_TestOnly() == expectedProxyRefs);
    REPORTER_ASSERT(reporter, proxy->getBackingRefCnt_TestOnly() == expectedBackingRefs);
    REPORTER_ASSERT(reporter, proxy->getPendingReadCnt_TestOnly() == expectedNumReads);
    REPORTER_ASSERT(reporter, proxy->getPendingWriteCnt_TestOnly() == expectedNumWrites);

    SkASSERT(proxy->getProxyRefCnt_TestOnly() == expectedProxyRefs);
    SkASSERT(proxy->getBackingRefCnt_TestOnly() == expectedBackingRefs);
    SkASSERT(proxy->getPendingReadCnt_TestOnly() == expectedNumReads);
    SkASSERT(proxy->getPendingWriteCnt_TestOnly() == expectedNumWrites);
}

static sk_sp<GrTextureProxy> make_deferred(GrContext* context) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = kWidthHeight;
    desc.fHeight = kWidthHeight;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    return GrSurfaceProxy::MakeDeferred(context->resourceProvider(), desc,
                                        SkBackingFit::kApprox, SkBudgeted::kYes);
}

static sk_sp<GrTextureProxy> make_wrapped(GrContext* context) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = kWidthHeight;
    desc.fHeight = kWidthHeight;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    sk_sp<GrTexture> tex(context->resourceProvider()->createTexture(desc, SkBudgeted::kNo));

    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeWrapped(std::move(tex));

    // Flush the IOWrite from the initial discard or it will confuse the later ref count checks
    context->contextPriv().flushSurfaceWrites(proxy.get());

    return proxy;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ProxyRefTest, reporter, ctxInfo) {
    GrResourceProvider* provider = ctxInfo.grContext()->resourceProvider();
    const GrCaps& caps = *ctxInfo.grContext()->caps();

    // Currently the op itself takes a pending write and the render target op list does as well.
    static const int kWritesForDiscard = 2;
    for (auto make : { make_deferred, make_wrapped }) {
        // A single write
        {
            sk_sp<GrTextureProxy> proxy((*make)(ctxInfo.grContext()));

            GrPendingIOResource<GrSurfaceProxy, kWrite_GrIOType> fWrite(proxy.get());

            check_refs(reporter, proxy.get(), 1, 1, 0, 1);

            // In the deferred case, the discard op created on instantiation adds an
            // extra ref and write
            bool proxyGetsDiscardRef = !proxy->isWrapped_ForTesting() &&
                                       caps.discardRenderTargetSupport();
            int expectedWrites = 1 + (proxyGetsDiscardRef ? kWritesForDiscard : 0);

            proxy->instantiate(provider);

            // In the deferred case, this checks that the refs transfered to the GrSurface
            check_refs(reporter, proxy.get(), 1, 1, 0, expectedWrites);
        }

        // A single read
        {
            sk_sp<GrTextureProxy> proxy((*make)(ctxInfo.grContext()));

            GrPendingIOResource<GrSurfaceProxy, kRead_GrIOType> fRead(proxy.get());

            check_refs(reporter, proxy.get(), 1, 1, 1, 0);

            // In the deferred case, the discard op created on instantiation adds an
            // extra ref and write
            bool proxyGetsDiscardRef = !proxy->isWrapped_ForTesting() &&
                                       caps.discardRenderTargetSupport();
            int expectedWrites = proxyGetsDiscardRef ? kWritesForDiscard : 0;

            proxy->instantiate(provider);

            // In the deferred case, this checks that the refs transfered to the GrSurface
            check_refs(reporter, proxy.get(), 1, 1, 1, expectedWrites);
        }

        // A single read/write pair
        {
            sk_sp<GrTextureProxy> proxy((*make)(ctxInfo.grContext()));

            GrPendingIOResource<GrSurfaceProxy, kRW_GrIOType> fRW(proxy.get());

            check_refs(reporter, proxy.get(), 1, 1, 1, 1);

            // In the deferred case, the discard op created on instantiation adds an
            // extra ref and write
            bool proxyGetsDiscardRef = !proxy->isWrapped_ForTesting() &&
                                       caps.discardRenderTargetSupport();
            int expectedWrites = 1 + (proxyGetsDiscardRef ? kWritesForDiscard : 0);

            proxy->instantiate(provider);

            // In the deferred case, this checks that the refs transferred to the GrSurface
            check_refs(reporter, proxy.get(), 1, 1, 1, expectedWrites);
        }

        // Multiple normal refs
        {
            sk_sp<GrTextureProxy> proxy((*make)(ctxInfo.grContext()));
            proxy->ref();
            proxy->ref();

            check_refs(reporter, proxy.get(), 3, 3, 0, 0);

            bool proxyGetsDiscardRef = !proxy->isWrapped_ForTesting() &&
                                       caps.discardRenderTargetSupport();
            int expectedWrites = proxyGetsDiscardRef ? kWritesForDiscard : 0;

            proxy->instantiate(provider);

            // In the deferred case, this checks that the refs transferred to the GrSurface
            check_refs(reporter, proxy.get(), 3, 3, 0, expectedWrites);

            proxy->unref();
            proxy->unref();
        }

        // Continue using (reffing) proxy after instantiation
        {
            sk_sp<GrTextureProxy> proxy((*make)(ctxInfo.grContext()));
            proxy->ref();

            GrPendingIOResource<GrSurfaceProxy, kWrite_GrIOType> fWrite(proxy.get());

            check_refs(reporter, proxy.get(), 2, 2, 0, 1);

            bool proxyGetsDiscardRef = !proxy->isWrapped_ForTesting() &&
                                       caps.discardRenderTargetSupport();
            int expectedWrites = 1 + (proxyGetsDiscardRef ? kWritesForDiscard : 0);

            proxy->instantiate(provider);

            // In the deferred case, this checks that the refs transfered to the GrSurface
            check_refs(reporter, proxy.get(), 2, 2, 0, expectedWrites);

            proxy->unref();
            check_refs(reporter, proxy.get(), 1, 1, 0, expectedWrites);

            GrPendingIOResource<GrSurfaceProxy, kRead_GrIOType> fRead(proxy.get());
            check_refs(reporter, proxy.get(), 1, 1, 1, expectedWrites);
        }
    }
}
#endif
