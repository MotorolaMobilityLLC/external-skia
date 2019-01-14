/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_GpuBase.h"
#include "GrBackendSurface.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "GrTextureAdjuster.h"
#include "SkBitmapCache.h"
#include "SkImage_Gpu.h"
#include "SkPromiseImageTexture.h"
#include "SkReadPixelsRec.h"
#include "effects/GrYUVtoRGBEffect.h"

SkImage_GpuBase::SkImage_GpuBase(sk_sp<GrContext> context, int width, int height, uint32_t uniqueID,
                                 SkAlphaType at, sk_sp<SkColorSpace> cs)
        : INHERITED(width, height, uniqueID)
        , fContext(std::move(context))
        , fAlphaType(at)
        , fColorSpace(std::move(cs)) {}

SkImage_GpuBase::~SkImage_GpuBase() {}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_GpuBase::ValidateBackendTexture(GrContext* ctx, const GrBackendTexture& tex,
                                             GrPixelConfig* config, SkColorType ct, SkAlphaType at,
                                             sk_sp<SkColorSpace> cs) {
    if (!tex.isValid()) {
        return false;
    }
    // TODO: Create a SkImageColorInfo struct for color, alpha, and color space so we don't need to
    // create a fake image info here.
    SkImageInfo info = SkImageInfo::Make(1, 1, ct, at, cs);
    if (!SkImageInfoIsValid(info)) {
        return false;
    }
    GrBackendFormat backendFormat = tex.getBackendFormat();
    if (!backendFormat.isValid()) {
        return false;
    }
    *config = ctx->contextPriv().caps()->getConfigFromBackendFormat(backendFormat, ct);
    return *config != kUnknown_GrPixelConfig;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_GpuBase::getROPixels(SkBitmap* dst, CachingHint chint) const {
    if (!fContext->contextPriv().resourceProvider()) {
        // DDL TODO: buffer up the readback so it occurs when the DDL is drawn?
        return false;
    }

    const auto desc = SkBitmapCacheDesc::Make(this);
    if (SkBitmapCache::Find(desc, dst)) {
        SkASSERT(dst->isImmutable());
        SkASSERT(dst->getPixels());
        return true;
    }

    SkBitmapCache::RecPtr rec = nullptr;
    SkPixmap pmap;
    if (kAllow_CachingHint == chint) {
        rec = SkBitmapCache::Alloc(desc, this->onImageInfo(), &pmap);
        if (!rec) {
            return false;
        }
    } else {
        if (!dst->tryAllocPixels(this->onImageInfo()) || !dst->peekPixels(&pmap)) {
            return false;
        }
    }

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
        this->asTextureProxyRef(),
        fColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(pmap.info(), pmap.writable_addr(), pmap.rowBytes(), 0, 0)) {
        return false;
    }

    if (rec) {
        SkBitmapCache::Add(std::move(rec), dst);
        this->notifyAddedToRasterCache();
    }
    return true;
}

sk_sp<SkImage> SkImage_GpuBase::onMakeSubset(const SkIRect& subset) const {
    sk_sp<GrSurfaceProxy> proxy = this->asTextureProxyRef();

    GrSurfaceDesc desc;
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();
    desc.fConfig = proxy->config();

    GrBackendFormat format = proxy->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    // TODO: Should this inherit our proxy's budgeted status?
    sk_sp<GrSurfaceContext> sContext(fContext->contextPriv().makeDeferredSurfaceContext(
            format, desc, proxy->origin(), GrMipMapped::kNo, SkBackingFit::kExact,
            proxy->isBudgeted()));
    if (!sContext) {
        return nullptr;
    }

    if (!sContext->copy(proxy.get(), subset, SkIPoint::Make(0, 0))) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'sContext' was kExact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID, fAlphaType,
                                   sContext->asTextureProxyRef(), fColorSpace);
}

static void apply_premul(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    switch (info.colorType()) {
    case kRGBA_8888_SkColorType:
    case kBGRA_8888_SkColorType:
        break;
    default:
        return; // nothing to do
    }

    // SkColor is not necessarily RGBA or BGRA, but it is one of them on little-endian,
    // and in either case, the alpha-byte is always in the same place, so we can safely call
    // SkPreMultiplyColor()
    //
    SkColor* row = (SkColor*)pixels;
    for (int y = 0; y < info.height(); ++y) {
        for (int x = 0; x < info.width(); ++x) {
            row[x] = SkPreMultiplyColor(row[x]);
        }
        row = (SkColor*)((char*)(row)+rowBytes);
    }
}

bool SkImage_GpuBase::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                   int srcX, int srcY, CachingHint) const {
    if (!fContext->contextPriv().resourceProvider()) {
        // DDL TODO: buffer up the readback so it occurs when the DDL is drawn?
        return false;
    }

    if (!SkImageInfoValidConversion(dstInfo, this->onImageInfo())) {
        return false;
    }

    SkReadPixelsRec rec(dstInfo, dstPixels, dstRB, srcX, srcY);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }

    // TODO: this seems to duplicate code in GrTextureContext::onReadPixels and
    // GrRenderTargetContext::onReadPixels
    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == rec.fInfo.alphaType() && kPremul_SkAlphaType == fAlphaType) {
        // let the GPU perform this transformation for us
        flags = GrContextPriv::kUnpremul_PixelOpsFlag;
    }

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
        this->asTextureProxyRef(), fColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY, flags)) {
        return false;
    }

    // do we have to manually fix-up the alpha channel?
    //      src         dst
    //      unpremul    premul      fix manually
    //      premul      unpremul    done by kUnpremul_PixelOpsFlag
    // all other combos need to change.
    //
    // Should this be handled by Ganesh? todo:?
    //
    if (kPremul_SkAlphaType == rec.fInfo.alphaType() && kUnpremul_SkAlphaType == fAlphaType) {
        apply_premul(rec.fInfo, rec.fPixels, rec.fRowBytes);
    }
    return true;
}

sk_sp<GrTextureProxy> SkImage_GpuBase::asTextureProxyRef(GrContext* context,
                                                         const GrSamplerState& params,
                                                         SkScalar scaleAdjust[2]) const {
    if (context->uniqueID() != fContext->uniqueID()) {
        SkASSERT(0);
        return nullptr;
    }

    GrTextureAdjuster adjuster(fContext.get(), this->asTextureProxyRef(), fAlphaType,
                               this->uniqueID(), fColorSpace.get());
    return adjuster.refTextureProxyForParams(params, scaleAdjust);
}

GrBackendTexture SkImage_GpuBase::onGetBackendTexture(bool flushPendingGrContextIO,
                                                      GrSurfaceOrigin* origin) const {
    sk_sp<GrTextureProxy> proxy = this->asTextureProxyRef();
    SkASSERT(proxy);

    if (!fContext->contextPriv().resourceProvider() && !proxy->isInstantiated()) {
        // This image was created with a DDL context and cannot be instantiated.
        return GrBackendTexture();
}

    if (!proxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return GrBackendTexture(); // invalid
    }

    GrTexture* texture = proxy->peekTexture();

    if (texture) {
        if (flushPendingGrContextIO) {
            fContext->contextPriv().prepareSurfaceForExternalIO(proxy.get());
        }
        if (origin) {
            *origin = proxy->origin();
        }
        return texture->getBackendTexture();
    }
    return GrBackendTexture(); // invalid
}

GrTexture* SkImage_GpuBase::onGetTexture() const {
    GrTextureProxy* proxy = this->peekProxy();
    if (!proxy) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxyRef = this->asTextureProxyRef();
    if (!fContext->contextPriv().resourceProvider() && !proxyRef->isInstantiated()) {
        // This image was created with a DDL context and cannot be instantiated.
        return nullptr;
    }

    if (!proxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return nullptr;
    }

    return proxy->peekTexture();
}

bool SkImage_GpuBase::onIsValid(GrContext* context) const {
    // The base class has already checked that context isn't abandoned (if it's not nullptr)
    if (fContext->abandoned()) {
        return false;
    }

    if (context && context != fContext.get()) {
        return false;
    }

    return true;
}

bool SkImage_GpuBase::MakeTempTextureProxies(GrContext* ctx, const GrBackendTexture yuvaTextures[],
                                             int numTextures, const SkYUVAIndex yuvaIndices[4],
                                             GrSurfaceOrigin imageOrigin,
                                             sk_sp<GrTextureProxy> tempTextureProxies[4]) {
    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();

    // We need to make a copy of the input backend textures because we need to preserve the result
    // of validate_backend_texture.
    GrBackendTexture yuvaTexturesCopy[4];
    for (int textureIndex = 0; textureIndex < numTextures; ++textureIndex) {
        yuvaTexturesCopy[textureIndex] = yuvaTextures[textureIndex];
        GrBackendFormat backendFormat = yuvaTexturesCopy[textureIndex].getBackendFormat();
        if (!backendFormat.isValid()) {
            return false;
        }
        yuvaTexturesCopy[textureIndex].fConfig =
                ctx->contextPriv().caps()->getYUVAConfigFromBackendFormat(backendFormat);
        if (yuvaTexturesCopy[textureIndex].fConfig == kUnknown_GrPixelConfig) {
            return false;
        }
        SkASSERT(yuvaTexturesCopy[textureIndex].isValid());

        tempTextureProxies[textureIndex] =
                proxyProvider->wrapBackendTexture(yuvaTexturesCopy[textureIndex], imageOrigin,
                                                  kBorrow_GrWrapOwnership, kRead_GrIOType);
        if (!tempTextureProxies[textureIndex]) {
            return false;
        }

        // Check that each texture contains the channel data for the corresponding YUVA index
        GrPixelConfig config = yuvaTexturesCopy[textureIndex].fConfig;
        for (int yuvaIndex = 0; yuvaIndex < SkYUVAIndex::kIndexCount; ++yuvaIndex) {
            if (yuvaIndices[yuvaIndex].fIndex == textureIndex) {
                switch (yuvaIndices[yuvaIndex].fChannel) {
                    case SkColorChannel::kR:
                        if (kAlpha_8_as_Alpha_GrPixelConfig == config) {
                            return false;
                        }
                        break;
                    case SkColorChannel::kG:
                    case SkColorChannel::kB:
                        if (kAlpha_8_as_Alpha_GrPixelConfig == config ||
                            kAlpha_8_as_Red_GrPixelConfig == config) {
                            return false;
                        }
                        break;
                    case SkColorChannel::kA:
                    default:
                        if (kRGB_888_GrPixelConfig == config) {
                            return false;
                        }
                        break;
                }
            }
        }
    }

    return true;
}

bool SkImage_GpuBase::RenderYUVAToRGBA(GrContext* ctx, GrRenderTargetContext* renderTargetContext,
                                       const SkRect& rect, SkYUVColorSpace yuvColorSpace,
                                       const sk_sp<GrTextureProxy> proxies[4],
                                       const SkYUVAIndex yuvaIndices[4]) {
    SkASSERT(renderTargetContext);
    if (!renderTargetContext->asSurfaceProxy()) {
        return false;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    paint.addColorFragmentProcessor(GrYUVtoRGBEffect::Make(proxies, yuvaIndices,
                                                           yuvColorSpace,
                                                           GrSamplerState::Filter::kNearest));

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

    // DDL TODO: in the promise image version we must not flush here
    ctx->contextPriv().flushSurfaceWrites(renderTargetContext->asSurfaceProxy());

    return true;
}

sk_sp<GrTextureProxy> SkImage_GpuBase::MakePromiseImageLazyProxy(
        GrContext* context, int width, int height, GrSurfaceOrigin origin, GrPixelConfig config,
        GrBackendFormat backendFormat, GrMipMapped mipMapped,
        PromiseImageTextureFulfillProc fulfillProc,
        PromiseImageTextureReleaseProc releaseProc,
        PromiseImageTextureDoneProc doneProc,
        PromiseImageTextureContext textureContext) {
    SkASSERT(context);
    SkASSERT(width > 0 && height > 0);
    SkASSERT(doneProc);
    SkASSERT(config != kUnknown_GrPixelConfig);

    if (!fulfillProc || !releaseProc) {
        doneProc(textureContext);
        return nullptr;
    }

    if (mipMapped == GrMipMapped::kYes &&
        GrTextureTypeHasRestrictedSampling(backendFormat.textureType())) {
        // It is invalid to have a GL_TEXTURE_EXTERNAL or GL_TEXTURE_RECTANGLE and have mips as
        // well.
        doneProc(textureContext);
        return nullptr;
    }

    /**
     * This class is the lazy instantiation callback for promise images. It manages calling the
     * client's Fulfill, Release, and Done procs. It attempts to reuse a GrTexture instance in
     * cases where the client provides the same SkPromiseImageTexture for successive Fulfill calls.
     * The created GrTexture is given a key based on a unique ID associated with the
     * SkPromiseImageTexture. When the texture enters "idle" state (meaning it is not being used by
     * the GPU and is at rest in the resource cache) the client's Release proc is called
     * using GrTexture's idle proc mechanism. If the same SkPromiseImageTexture is provided for
     * another fulfill we find the cached GrTexture. If the proxy, and therefore this object,
     * is destroyed, we invalidate the GrTexture's key. Also if the client overwrites or
     * destroys their SkPromiseImageTexture we invalidate the key.
     *
     * Currently a GrTexture is only reused for a given SkPromiseImageTexture if the
     * SkPromiseImageTexture is reused in Fulfill for the same promise SkImage. However, we'd
     * like to relax that so that a SkPromiseImageTexture can be reused with different promise
     * SkImages that will reuse a single GrTexture.
     */
    class PromiseLazyInstantiateCallback {
    public:
        PromiseLazyInstantiateCallback(PromiseImageTextureFulfillProc fulfillProc,
                                       PromiseImageTextureReleaseProc releaseProc,
                                       PromiseImageTextureDoneProc doneProc,
                                       PromiseImageTextureContext context,
                                       GrPixelConfig config)
                : fFulfillProc(fulfillProc)
                , fReleaseProc(releaseProc)
                , fContext(context)
                , fConfig(config) {
            fDoneHelper = sk_make_sp<GrReleaseProcHelper>(doneProc, context);
            static std::atomic<uint32_t> gUniqueID;
            fUniqueID = gUniqueID.fetch_add(1) + 1;
        }
        ~PromiseLazyInstantiateCallback() {
            // If we've already released the texture then it is safe to call done now. Here we may
            // be on any thread.
            if (fIdleContext && fIdleContext->fWasReleased.load()) {
                // We still own a ref on fDoneHelper so no other thread can be calling the done
                // proc.
                fDoneHelper->callAndClear();
            }
            // Remove the key from the texture so that the texture will be removed from the cache.
            // If we didn't just call the done proc above then it will get called when the texture
            // is removed from the cache after this message is processed.
            if (fLastFulfilledKey.isValid()) {
                SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(
                        GrUniqueKeyInvalidatedMessage(fLastFulfilledKey, fContextID));
            }
        }

        sk_sp<GrSurface> operator()(GrResourceProvider* resourceProvider) {
            if (!resourceProvider) {
                return nullptr;
            }

            sk_sp<GrTexture> cachedTexture;
            SkASSERT(fLastFulfilledKey.isValid() == (fLastFulfillID > 0));
            if (fLastFulfilledKey.isValid()) {
                auto surf = resourceProvider->findByUniqueKey<GrSurface>(fLastFulfilledKey);
                if (surf) {
                    cachedTexture = sk_ref_sp(surf->asTexture());
                    SkASSERT(cachedTexture);
                }
            }
            // If the release callback hasn't been called already by releasing the GrTexture
            // then we can be sure that won't happen so long as we have a ref to the texture.
            // Moreoever, only this thread should be able to change the atomic to true, hence the
            // relaxed memory order.
            if (cachedTexture && !fIdleContext->fWasReleased.load(std::memory_order_relaxed)) {
                return std::move(cachedTexture);
            }
            GrBackendTexture backendTexture;
            sk_sp<SkPromiseImageTexture> promiseTexture = fFulfillProc(fContext);
            if (!promiseTexture) {
                fReleaseProc(fContext);
                return sk_sp<GrTexture>();
            }
            bool same = promiseTexture->uniqueID() == fLastFulfillID;
            SkASSERT(!same || fLastFulfilledKey.isValid());
            if (same && cachedTexture) {
                SkASSERT(fIdleContext->unique());
                // Reset the purgeable context so that we balance the new fulfill with a release.
                fIdleContext->ref();
                SkASSERT(fIdleContext->fReleaseProc == fReleaseProc);
                SkASSERT(fIdleContext->fTextureContext == fContext);
                // Memory order relaxed because only this thread can change fWasReleased to true.
                fIdleContext->fWasReleased.store(false, std::memory_order_relaxed);
                cachedTexture->setIdleProc(IdleProc, fIdleContext.get());
                return std::move(cachedTexture);
            } else if (cachedTexture) {
                cachedTexture->resourcePriv().removeUniqueKey();
                // We don't want calling the client's done proc to be tied to the old texture.
                cachedTexture->setRelease(nullptr);
            }
            fLastFulfillID = promiseTexture->uniqueID();

            backendTexture = promiseTexture->backendTexture();
            backendTexture.fConfig = fConfig;
            if (!backendTexture.isValid()) {
                // Even though the GrBackendTexture is not valid, we must call the release
                // proc to keep our contract of always calling Fulfill and Release in pairs.
                fReleaseProc(fContext);
                return sk_sp<GrTexture>();
            }

            auto tex = resourceProvider->wrapBackendTexture(backendTexture, kBorrow_GrWrapOwnership,
                                                            kRead_GrIOType);
            if (!tex) {
                // Even though we failed to wrap the backend texture, we must call the release
                // proc to keep our contract of always calling Fulfill and Release in pairs.
                fReleaseProc(fContext);
                return sk_sp<GrTexture>();
            }
            fIdleContext = sk_make_sp<IdleContext>(fReleaseProc, fContext);
            // The texture gets a ref, which is balanced when the idle callback is called.
            fIdleContext->ref();
            tex->setIdleProc(IdleProc, fIdleContext.get());
            tex->setRelease(fDoneHelper);
            static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
            GrUniqueKey::Builder builder(&fLastFulfilledKey, kDomain, 2, "promise");
            builder[0] = promiseTexture->uniqueID();
            builder[1] = fUniqueID;
            builder.finish();
            tex->resourcePriv().setUniqueKey(fLastFulfilledKey);
            SkASSERT(fContextID == SK_InvalidUniqueID ||
                     fContextID == tex->getContext()->uniqueID());
            fContextID = tex->getContext()->uniqueID();
            promiseTexture->addKeyToInvalidate(fContextID, fLastFulfilledKey);
            return std::move(tex);
        }

    private:
        struct IdleContext : public SkNVRefCnt<IdleContext> {
            IdleContext(PromiseImageTextureReleaseProc proc, PromiseImageTextureContext context)
                    : fReleaseProc(proc), fTextureContext(context) {}
            PromiseImageTextureReleaseProc fReleaseProc;
            PromiseImageTextureContext fTextureContext;
            std::atomic<bool> fWasReleased{false};
        };
        static void IdleProc(void* context) {
            IdleContext* rc = static_cast<IdleContext*>(context);
            SkASSERT(!rc->fWasReleased.load());
            rc->fReleaseProc(rc->fTextureContext);
            rc->fWasReleased.store(true);
            // Drop the texture's implicit ref on the IdleContext.
            rc->unref();
        }

        PromiseImageTextureFulfillProc fFulfillProc;
        PromiseImageTextureReleaseProc fReleaseProc;
        PromiseImageTextureContext fContext;

        GrPixelConfig fConfig;
        sk_sp<IdleContext> fIdleContext;
        sk_sp<GrReleaseProcHelper> fDoneHelper;

        // ID of the last SkPromiseImageTexture given to us by the client.
        uint32_t fLastFulfillID = 0;
        // ID of the GrContext that we are interacting with.
        uint32_t fContextID = SK_InvalidUniqueID;
        // Unique ID of this lazy instantiation callback.
        uint32_t fUniqueID;
        GrUniqueKey fLastFulfilledKey;
    } callback(fulfillProc, releaseProc, doneProc, textureContext, config);

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;

    // We pass kReadOnly here since we should treat content of the client's texture as immutable.
    return proxyProvider->createLazyProxy(std::move(callback), backendFormat, desc, origin,
                                          mipMapped, GrInternalSurfaceFlags::kReadOnly,
                                          SkBackingFit::kExact, SkBudgeted::kNo,
                                          GrSurfaceProxy::LazyInstantiationType::kDeinstantiate);
}
