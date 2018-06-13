/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTexture_DEFINED
#define GrVkTexture_DEFINED

#include "GrTexture.h"
#include "GrVkImage.h"

class GrVkGpu;
class GrVkImageView;
struct GrVkImageInfo;

class GrVkTexture : public GrTexture, public virtual GrVkImage {
public:
    static sk_sp<GrVkTexture> CreateNewTexture(GrVkGpu*,
                                               SkBudgeted budgeted,
                                               const GrSurfaceDesc&,
                                               const GrVkImage::ImageDesc&,
                                               GrMipMapsStatus);

    static sk_sp<GrVkTexture> MakeWrappedTexture(GrVkGpu*, const GrSurfaceDesc&,
                                                 GrWrapOwnership, const GrVkImageInfo&,
                                                 sk_sp<GrVkImageLayout>);

    ~GrVkTexture() override;

    GrBackendTexture getBackendTexture() const override;

    void textureParamsModified() override {}

    const GrVkImageView* textureView();

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrVkImage::Resource object (which occurs after the GPU has finsihed all work on it).
    void setRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {
        // Forward the release proc on to GrVkImage
        this->setResourceRelease(std::move(releaseHelper));
    }

protected:
    GrVkTexture(GrVkGpu*, const GrSurfaceDesc&, const GrVkImageInfo&, sk_sp<GrVkImageLayout>,
                const GrVkImageView*, GrMipMapsStatus, GrBackendObjectOwnership);

    GrVkGpu* getVkGpu() const;

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
        return false;
    }

private:
    enum Wrapped { kWrapped };
    GrVkTexture(GrVkGpu*, SkBudgeted, const GrSurfaceDesc&, const GrVkImageInfo&,
                sk_sp<GrVkImageLayout> layout, const GrVkImageView* imageView,
                GrMipMapsStatus);
    GrVkTexture(GrVkGpu*, Wrapped, const GrSurfaceDesc&, const GrVkImageInfo&,
                sk_sp<GrVkImageLayout> layout, const GrVkImageView* imageView, GrMipMapsStatus,
                GrBackendObjectOwnership);

    const GrVkImageView*     fTextureView;

    typedef GrTexture INHERITED;
};

#endif
