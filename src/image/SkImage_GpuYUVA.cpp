/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "GrTextureAdjuster.h"
#include "SkImage_Gpu.h"
#include "SkImage_GpuShared.h"
#include "SkImage_GpuYUVA.h"
#include "SkReadPixelsRec.h"
#include "effects/GrYUVtoRGBEffect.h"

using namespace SkImage_GpuShared;

SkImage_GpuYUVA::SkImage_GpuYUVA(sk_sp<GrContext> context, uint32_t uniqueID,
                                 SkYUVColorSpace colorSpace, sk_sp<GrTextureProxy> proxies[],
                                 SkYUVAIndex yuvaIndices[4], SkISize size, GrSurfaceOrigin origin,
                                 sk_sp<SkColorSpace> imageColorSpace, SkBudgeted budgeted)
        : INHERITED(size.width(), size.height(), uniqueID)
        , fContext(std::move(context))
        , fBudgeted(budgeted)
        , fColorSpace(colorSpace)
        , fOrigin(origin)
        , fImageAlphaType(kOpaque_SkAlphaType)
        , fImageColorSpace(std::move(imageColorSpace)) {
    for (int i = 0; i < 4; ++i) {
        fProxies[i] = std::move(proxies[i]);
    }
    memcpy(fYUVAIndices, yuvaIndices, 4*sizeof(SkYUVAIndex));
    // If an alpha channel is present we always switch to kPremul. This is because, although the
    // planar data is always un-premul, the final interleaved RGB image is/would-be premul.
    if (-1 != yuvaIndices[SkYUVAIndex::kA_Index].fIndex) {
        fImageAlphaType = kPremul_SkAlphaType;
    }
}

SkImage_GpuYUVA::~SkImage_GpuYUVA() {}

SkImageInfo SkImage_GpuYUVA::onImageInfo() const {
    // Note: this is the imageInfo for the flattened image, not the YUV planes
    return SkImageInfo::Make(this->width(), this->height(), kRGBA_8888_SkColorType,
                             fImageAlphaType, fImageColorSpace);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<GrTextureProxy> SkImage_GpuYUVA::asTextureProxyRef() const {
    if (!fRGBProxy) {
        sk_sp<GrTextureProxy> yProxy = fProxies[fYUVAIndices[SkYUVAIndex::kY_Index].fIndex];
        sk_sp<GrTextureProxy> uProxy = fProxies[fYUVAIndices[SkYUVAIndex::kU_Index].fIndex];
        sk_sp<GrTextureProxy> vProxy = fProxies[fYUVAIndices[SkYUVAIndex::kV_Index].fIndex];

        if (!yProxy || !uProxy || !vProxy) {
            return nullptr;
        }

        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        // TODO: Modify the fragment processor to sample from different channel
        // instead of taking nv12 bool.
        bool nv12 = (fYUVAIndices[SkYUVAIndex::kU_Index].fIndex ==
                     fYUVAIndices[SkYUVAIndex::kV_Index].fIndex);
        // TODO: modify the YUVtoRGBEffect to do premul if fImageAlphaType is kPremul_AlphaType
        paint.addColorFragmentProcessor(GrYUVtoRGBEffect::Make(std::move(yProxy), std::move(uProxy),
                                                               std::move(vProxy), fColorSpace,
                                                               nv12));

        const SkRect rect = SkRect::MakeIWH(this->width(), this->height());

        // Needs to create a render target in order to draw to it for the yuv->rgb conversion.
        sk_sp<GrRenderTargetContext> renderTargetContext(
            fContext->contextPriv().makeDeferredRenderTargetContext(
                SkBackingFit::kExact, this->width(), this->height(), kRGBA_8888_GrPixelConfig,
                std::move(fImageColorSpace), 1, GrMipMapped::kNo, fOrigin));
        if (!renderTargetContext) {
            return nullptr;
        }
        renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

        if (!renderTargetContext->asSurfaceProxy()) {
            return nullptr;
        }

        // DDL TODO: in the promise image version we must not flush here
        fContext->contextPriv().flushSurfaceWrites(renderTargetContext->asSurfaceProxy());

        // cast to non-const
        (sk_sp<GrTextureProxy>)(fRGBProxy) = renderTargetContext->asTextureProxyRef();
    }

    return fRGBProxy;
}

sk_sp<GrTextureProxy> SkImage_GpuYUVA::asTextureProxyRef(GrContext* context,
                                                         const GrSamplerState& params,
                                                         SkColorSpace* dstColorSpace,
                                                         sk_sp<SkColorSpace>* texColorSpace,
                                                         SkScalar scaleAdjust[2]) const {
    return AsTextureProxyRef(context, params, dstColorSpace, texColorSpace, scaleAdjust,
                             fContext.get(), this, fImageAlphaType, fImageColorSpace.get());
}

bool SkImage_GpuYUVA::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
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

    // Flatten the YUVA planes to a single texture
    sk_sp<GrSurfaceProxy> proxy = this->asTextureProxyRef();

    // TODO: this seems to duplicate code in GrTextureContext::onReadPixels and
    // GrRenderTargetContext::onReadPixels
    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == rec.fInfo.alphaType() && kPremul_SkAlphaType == fImageAlphaType) {
        // let the GPU perform this transformation for us
        flags = GrContextPriv::kUnpremul_PixelOpsFlag;
    }

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
        proxy, fImageColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY, flags)) {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//*** bundle this into a helper function used by this and SkImage_Gpu?
sk_sp<SkImage> SkImage_GpuYUVA::MakeFromYUVATextures(GrContext* ctx,
                                                     SkYUVColorSpace colorSpace,
                                                     const GrBackendTexture yuvaTextures[],
                                                     SkYUVAIndex yuvaIndices[4],
                                                     SkISize size,
                                                     GrSurfaceOrigin origin,
                                                     sk_sp<SkColorSpace> imageColorSpace) {
    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();

    // Right now this still only deals with YUV and NV12 formats. Assuming that YUV has different
    // textures for U and V planes, while NV12 uses same texture for U and V planes.
    bool nv12 = (yuvaIndices[SkYUVAIndex::kU_Index].fIndex ==
                 yuvaIndices[SkYUVAIndex::kV_Index].fIndex);
    auto ct = nv12 ? kRGBA_8888_SkColorType : kAlpha_8_SkColorType;

    // We need to make a copy of the input backend textures because we need to preserve the result
    // of validate_backend_texture.
    GrBackendTexture yuvaTexturesCopy[4];
    for (int i = 0; i < 4; ++i) {
        // Validate that the yuvaIndices refer to valid backend textures.
        const SkYUVAIndex& yuvaIndex = yuvaIndices[i];
        if (SkYUVAIndex::kA_Index == i && yuvaIndex.fIndex == -1) {
            // Meaning the A plane isn't passed in.
            continue;
        }
        if (yuvaIndex.fIndex == -1 || yuvaIndex.fIndex > 3) {
            // Y plane, U plane, and V plane must refer to image sources being passed in. There are
            // at most 4 image sources being passed in, could not have a index more than 3.
            return nullptr;
        }
        if (!yuvaTexturesCopy[yuvaIndex.fIndex].isValid()) {
            yuvaTexturesCopy[yuvaIndex.fIndex] = yuvaTextures[yuvaIndex.fIndex];
            // TODO: Instead of using assumption about whether it is NV12 format to guess colorType,
            // actually use channel information here.
            if (!ValidateBackendTexture(ctx, yuvaTexturesCopy[i], &yuvaTexturesCopy[i].fConfig,
                                        ct, kUnpremul_SkAlphaType, nullptr)) {
                return nullptr;
            }
        }

        // TODO: Check that for each plane, the channel actually exist in the image source we are
        // reading from.
    }

    sk_sp<GrTextureProxy> tempTextureProxies[4]; // build from yuvaTextures
    for (int i = 0; i < 4; ++i) {
        // Fill in tempTextureProxies to avoid duplicate texture proxies.
        int textureIndex = yuvaIndices[i].fIndex;

        // Safely ignore since this means we are missing the A plane.
        if (textureIndex == -1) {
            SkASSERT(SkYUVAIndex::kA_Index == i);
            continue;
        }

        if (!tempTextureProxies[textureIndex]) {
            SkASSERT(yuvaTexturesCopy[textureIndex].isValid());
            tempTextureProxies[textureIndex] =
                proxyProvider->wrapBackendTexture(yuvaTexturesCopy[textureIndex], origin);
        }
    }
    if (!tempTextureProxies[yuvaIndices[SkYUVAIndex::kY_Index].fIndex] ||
        !tempTextureProxies[yuvaIndices[SkYUVAIndex::kU_Index].fIndex] ||
        !tempTextureProxies[yuvaIndices[SkYUVAIndex::kV_Index].fIndex]) {
        return nullptr;
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(ctx), kNeedNewImageUniqueID, colorSpace,
                                       tempTextureProxies, yuvaIndices, size, origin,
                                       imageColorSpace, SkBudgeted::kYes);
}

