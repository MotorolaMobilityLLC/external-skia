/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProxyProvider_DEFINED
#define GrProxyProvider_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/private/GrResourceKey.h"
#include "src/core/SkTDynamicHash.h"
#include "src/gpu/GrTextureProxy.h"

class GrImageContext;
class GrBackendRenderTarget;
class SkBitmap;
class SkImage;

/*
 * A factory for creating GrSurfaceProxy-derived objects.
 */
class GrProxyProvider {
public:
    GrProxyProvider(GrImageContext*);

    ~GrProxyProvider();

    /*
     * Assigns a unique key to a proxy. The proxy will be findable via this key using
     * findProxyByUniqueKey(). It is an error if an existing proxy already has a key.
     */
    bool assignUniqueKeyToProxy(const GrUniqueKey&, GrTextureProxy*);

    /*
     * Sets the unique key of the provided proxy to the unique key of the surface. The surface must
     * have a valid unique key.
     */
    void adoptUniqueKeyFromSurface(GrTextureProxy* proxy, const GrSurface*);

    /*
     * Removes a unique key from a proxy. If the proxy has already been instantiated, it will
     * also remove the unique key from the target GrSurface.
     */
    void removeUniqueKeyFromProxy(GrTextureProxy*);

    /*
     * Finds a proxy by unique key.
     */
    sk_sp<GrTextureProxy> findProxyByUniqueKey(const GrUniqueKey&, GrSurfaceOrigin);

    /*
     * Finds a proxy by unique key or creates a new one that wraps a resource matching the unique
     * key.
     */
    sk_sp<GrTextureProxy> findOrCreateProxyByUniqueKey(const GrUniqueKey&, GrSurfaceOrigin);

    /*
     * Create an un-mipmapped texture proxy with data. The SkImage must be a raster backend image.
     * Since the SkImage is ref counted, we simply take a ref on it to keep the data alive until we
     * actually upload the data to the gpu.
     */
    sk_sp<GrTextureProxy> createTextureProxy(
            sk_sp<SkImage> srcImage, GrRenderable, int sampleCnt, SkBudgeted, SkBackingFit,
            GrInternalSurfaceFlags = GrInternalSurfaceFlags::kNone);

    /*
     * Create a mipmapped texture proxy without any data.
     *
     * Like the call above but there are no texels to upload. A texture proxy is returned that
     * simply has space allocated for the mips. We will allocated the full amount of mip levels
     * based on the width and height in the GrSurfaceDesc.
     */
    sk_sp<GrTextureProxy> createMipMapProxy(const GrBackendFormat&, const GrSurfaceDesc&,
                                            GrRenderable, GrSurfaceOrigin, SkBudgeted);

    /*
     * Creates a new texture proxy for the bitmap, optionally with mip levels generated by the cpu.
     */
    sk_sp<GrTextureProxy> createProxyFromBitmap(const SkBitmap& bitmap, GrMipMapped);

    /*
     * Create a GrSurfaceProxy without any data.
     */
    sk_sp<GrTextureProxy> createProxy(const GrBackendFormat&, const GrSurfaceDesc&, GrRenderable,
                                      GrSurfaceOrigin, GrMipMapped, SkBackingFit, SkBudgeted,
                                      GrInternalSurfaceFlags);

    sk_sp<GrTextureProxy> createProxy(
            const GrBackendFormat& format, const GrSurfaceDesc& desc, GrRenderable renderable,
            GrSurfaceOrigin origin, SkBackingFit fit, SkBudgeted budgeted,
            GrInternalSurfaceFlags surfaceFlags = GrInternalSurfaceFlags::kNone) {
        return this->createProxy(format, desc, renderable, origin, GrMipMapped::kNo, fit, budgeted,
                                 surfaceFlags);
    }

    /*
     * Create a texture proxy from compressed texture data.
     */
    sk_sp<GrTextureProxy> createCompressedTextureProxy(int width, int height, SkBudgeted budgeted,
                                                       SkImage::CompressionType compressionType,
                                                       sk_sp<SkData> data);

    // These match the definitions in SkImage & GrTexture.h, for whence they came
    typedef void* ReleaseContext;
    typedef void (*ReleaseProc)(ReleaseContext);

    /*
     * Create a texture proxy that wraps a (non-renderable) backend texture. GrIOType must be
     * kRead or kRW.
     */
    sk_sp<GrTextureProxy> wrapBackendTexture(const GrBackendTexture&, GrColorType, GrSurfaceOrigin,
                                             GrWrapOwnership, GrWrapCacheable, GrIOType,
                                             ReleaseProc = nullptr, ReleaseContext = nullptr);

    /*
     * Create a texture proxy that wraps a backend texture and is both texture-able and renderable
     */
    sk_sp<GrTextureProxy> wrapRenderableBackendTexture(const GrBackendTexture&, GrSurfaceOrigin,
                                                       int sampleCnt, GrColorType,
                                                       GrWrapOwnership, GrWrapCacheable,
                                                       ReleaseProc = nullptr,
                                                       ReleaseContext = nullptr);

    /*
     * Create a render target proxy that wraps a backend render target
     */
    sk_sp<GrSurfaceProxy> wrapBackendRenderTarget(const GrBackendRenderTarget&, GrColorType,
                                                  GrSurfaceOrigin,
                                                  ReleaseProc = nullptr, ReleaseContext = nullptr);

    /*
     * Create a render target proxy that wraps a backend texture
     */
    sk_sp<GrSurfaceProxy> wrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                           GrColorType,
                                                           GrSurfaceOrigin,
                                                           int sampleCnt);

    sk_sp<GrRenderTargetProxy> wrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo&,
                                                                   const GrVkDrawableInfo&);

    using LazyInstantiationKeyMode = GrSurfaceProxy::LazyInstantiationKeyMode;
    using LazyInstantiationResult = GrSurfaceProxy::LazyInstantiationResult;
    using LazyInstantiateCallback = GrSurfaceProxy::LazyInstantiateCallback;

    enum class Renderable : bool {
        kNo = false,
        kYes = true
    };

    struct TextureInfo {
        GrMipMapped fMipMapped;
        GrTextureType fTextureType;
    };

    using LazyInstantiationType = GrSurfaceProxy::LazyInstantiationType;
    /**
     * Creates a texture proxy that will be instantiated by a user-supplied callback during flush.
     * (Stencil is not supported by this method.) The width and height must either both be greater
     * than 0 or both less than or equal to zero. A non-positive value is a signal that the width
     * and height are currently unknown.
     *
     * When called, the callback must be able to cleanup any resources that it captured at creation.
     * It also must support being passed in a null GrResourceProvider. When this happens, the
     * callback should cleanup any resources it captured and return an empty sk_sp<GrTextureProxy>.
     */
    sk_sp<GrTextureProxy> createLazyProxy(LazyInstantiateCallback&&, const GrBackendFormat&,
                                          const GrSurfaceDesc&, GrRenderable, GrSurfaceOrigin,
                                          GrMipMapped, GrInternalSurfaceFlags, SkBackingFit,
                                          SkBudgeted, LazyInstantiationType);

    sk_sp<GrTextureProxy> createLazyProxy(LazyInstantiateCallback&&, const GrBackendFormat&,
                                          const GrSurfaceDesc&, GrRenderable, GrSurfaceOrigin,
                                          GrMipMapped, GrInternalSurfaceFlags, SkBackingFit,
                                          SkBudgeted);

    sk_sp<GrTextureProxy> createLazyProxy(LazyInstantiateCallback&&, const GrBackendFormat&,
                                          const GrSurfaceDesc&, GrRenderable, GrSurfaceOrigin,
                                          GrMipMapped, SkBackingFit, SkBudgeted);

    /** A null TextureInfo indicates a non-textureable render target. */
    sk_sp<GrRenderTargetProxy> createLazyRenderTargetProxy(LazyInstantiateCallback&&,
                                                           const GrBackendFormat&,
                                                           const GrSurfaceDesc&,
                                                           GrSurfaceOrigin origin,
                                                           GrInternalSurfaceFlags,
                                                           const TextureInfo*,
                                                           SkBackingFit,
                                                           SkBudgeted,
                                                           bool wrapsVkSecondaryCB);

    /**
     * Fully lazy proxies have unspecified width and height. Methods that rely on those values
     * (e.g., width, height, getBoundsRect) should be avoided.
     */
    static sk_sp<GrTextureProxy> MakeFullyLazyProxy(
            LazyInstantiateCallback&&, const GrBackendFormat&, Renderable, GrSurfaceOrigin,
            GrPixelConfig, const GrCaps&, int sampleCnt = 1);

    // 'proxy' is about to be used as a texture src or drawn to. This query can be used to
    // determine if it is going to need a texture domain or a full clear.
    static bool IsFunctionallyExact(GrSurfaceProxy* proxy);

    enum class InvalidateGPUResource : bool { kNo = false, kYes = true };

    /*
     * This method ensures that, if a proxy w/ the supplied unique key exists, it is removed from
     * the proxy provider's map and its unique key is removed. If 'invalidateSurface' is true, it
     * will independently ensure that the unique key is removed from any GrGpuResources that may
     * have it.
     *
     * If 'proxy' is provided (as an optimization to stop re-looking it up), its unique key must be
     * valid and match the provided unique key.
     *
     * This method is called if either the proxy attached to the unique key is being deleted
     * (in which case we don't want it cluttering up the hash table) or the client has indicated
     * that it will never refer to the unique key again.
     */
    void processInvalidUniqueKey(const GrUniqueKey&, GrTextureProxy*, InvalidateGPUResource);

    // TODO: remove these entry points - it is a bit sloppy to be getting context info from here
    uint32_t contextID() const;
    const GrCaps* caps() const;
    sk_sp<const GrCaps> refCaps() const;

    int numUniqueKeyProxies_TestOnly() const;

    // This is called on a DDL's proxyprovider when the DDL is finished. The uniquely keyed
    // proxies need to keep their unique key but cannot hold on to the proxy provider unique
    // pointer.
    void orphanAllUniqueKeys();
    // This is only used by GrContext::releaseResourcesAndAbandonContext()
    void removeAllUniqueKeys();

    /**
     * Does the proxy provider have access to a GrDirectContext? If so, proxies will be
     * instantiated immediately.
     */
    bool renderingDirectly() const;

#if GR_TEST_UTILS
    /*
     * Create a texture proxy that is backed by an instantiated GrSurface.
     */
    sk_sp<GrTextureProxy> testingOnly_createInstantiatedProxy(const GrSurfaceDesc&, GrRenderable,
                                                              GrSurfaceOrigin, SkBackingFit,
                                                              SkBudgeted);
    sk_sp<GrTextureProxy> testingOnly_createWrapped(sk_sp<GrTexture>, GrSurfaceOrigin);
#endif

private:
    friend class GrAHardwareBufferImageGenerator; // for createWrapped
    friend class GrResourceProvider; // for createWrapped

    bool isAbandoned() const;

    sk_sp<GrTextureProxy> createWrapped(sk_sp<GrTexture> tex, GrSurfaceOrigin origin);

    struct UniquelyKeyedProxyHashTraits {
        static const GrUniqueKey& GetKey(const GrTextureProxy& p) { return p.getUniqueKey(); }

        static uint32_t Hash(const GrUniqueKey& key) { return key.hash(); }
    };
    typedef SkTDynamicHash<GrTextureProxy, GrUniqueKey, UniquelyKeyedProxyHashTraits> UniquelyKeyedProxyHash;

    // This holds the texture proxies that have unique keys. The resourceCache does not get a ref
    // on these proxies but they must send a message to the resourceCache when they are deleted.
    UniquelyKeyedProxyHash fUniquelyKeyedProxies;

    GrImageContext*        fImageContext;
};

#endif
