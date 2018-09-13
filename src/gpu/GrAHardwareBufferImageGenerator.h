/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrAHardwareBufferImageGenerator_DEFINED
#define GrAHardwareBufferImageGenerator_DEFINED

#include "SkImageGenerator.h"

#include "GrTypesPriv.h"

class GrGpuResource;

extern "C" {
    typedef struct AHardwareBuffer AHardwareBuffer;
}

/**
 *  GrAHardwareBufferImageGenerator allows to create an SkImage attached to
 *  an existing android native hardware buffer. A hardware buffer has to be
 *  created with AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE usage, because it is
 *  bound to an external texture using an EGLImage. The image generator will
 *  keep a reference to the hardware buffer for its lifetime. A hardware buffer
 *  can be shared between processes and same buffer can be used in multiple GPU
 *  contexts.
 *  To implement certain features like tiling, Skia may copy the texture to
 *  avoid OpenGL API limitations.
 */
class GrAHardwareBufferImageGenerator : public SkImageGenerator {
public:
    static std::unique_ptr<SkImageGenerator> Make(AHardwareBuffer*, SkAlphaType,
                                                  sk_sp<SkColorSpace>);

    ~GrAHardwareBufferImageGenerator() override;

    typedef void* DeleteImageCtx;
    typedef void (*DeleteImageProc)(DeleteImageCtx);

    static void DeleteGLTexture(void* ctx);

#ifdef SK_VULKAN
    static void DeleteVkImage(void* ctx);
#endif

protected:

    bool onIsValid(GrContext*) const override;

    TexGenType onCanGenerateTexture() const override { return TexGenType::kCheap; }
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&, const SkIPoint&,
                                            bool willNeedMipMaps) override;

private:
    GrAHardwareBufferImageGenerator(const SkImageInfo&, AHardwareBuffer*, SkAlphaType,
                                    bool isProtectedContent, uint32_t bufferFormat);
    sk_sp<GrTextureProxy> makeProxy(GrContext* context);

    void releaseTextureRef();

    static void ReleaseRefHelper_TextureReleaseProc(void* ctx);

    AHardwareBuffer* fHardwareBuffer;
    uint32_t         fBufferFormat;
    const bool       fIsProtectedContent;

    typedef SkImageGenerator INHERITED;
};
#endif  // GrAHardwareBufferImageGenerator_DEFINED
