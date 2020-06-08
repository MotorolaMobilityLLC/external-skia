/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImage.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkStencilAttachment.h"
#include "src/gpu/vk/GrVkUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkStencilAttachment::GrVkStencilAttachment(GrVkGpu* gpu,
                                             const Format& format,
                                             const GrVkImage::ImageDesc& desc,
                                             const GrVkImageInfo& info,
                                             sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                                             const GrVkImageView* stencilView)
    : GrStencilAttachment(gpu, desc.fWidth, desc.fHeight, format.fStencilBits, desc.fSamples)
    , GrVkImage(gpu, info, std::move(mutableState), GrBackendObjectOwnership::kOwned)
    , fStencilView(stencilView) {
    this->registerWithCache(SkBudgeted::kYes);
    stencilView->ref();
}

GrVkStencilAttachment* GrVkStencilAttachment::Create(GrVkGpu* gpu,
                                                     int width,
                                                     int height,
                                                     int sampleCnt,
                                                     const Format& format) {
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = format.fInternalFormat;
    imageDesc.fWidth = width;
    imageDesc.fHeight = height;
    imageDesc.fLevels = 1;
    imageDesc.fSamples = sampleCnt;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }

    const GrVkImageView* imageView = GrVkImageView::Create(gpu, info.fImage,
                                                           format.fInternalFormat,
                                                           GrVkImageView::kStencil_Type, 1,
                                                           GrVkYcbcrConversionInfo());
    if (!imageView) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }

    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState(new GrBackendSurfaceMutableStateImpl(
        info.fImageLayout, info.fCurrentQueueFamily));
    GrVkStencilAttachment* stencil = new GrVkStencilAttachment(gpu, format, imageDesc,
                                                               info, std::move(mutableState),
                                                               imageView);
    imageView->unref();

    return stencil;
}

GrVkStencilAttachment::~GrVkStencilAttachment() {
    // should have been released or abandoned first
    SkASSERT(!fStencilView);
}

size_t GrVkStencilAttachment::onGpuMemorySize() const {
    uint64_t size = this->width();
    size *= this->height();
    size *= GrVkCaps::GetStencilFormatTotalBitCount(this->imageFormat());
    size *= this->numSamples();
    return static_cast<size_t>(size / 8);
}

void GrVkStencilAttachment::onRelease() {
    this->releaseImage();
    fStencilView->unref();
    fStencilView = nullptr;

    GrStencilAttachment::onRelease();
}

void GrVkStencilAttachment::onAbandon() {
    this->releaseImage();
    fStencilView->unref();
    fStencilView = nullptr;

    GrStencilAttachment::onAbandon();
}

GrVkGpu* GrVkStencilAttachment::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
