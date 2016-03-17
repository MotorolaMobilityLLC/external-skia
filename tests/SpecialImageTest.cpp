/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "SkAutoPixmapStorage.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkPixmap.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "Test.h"
#include "TestingSpecialImageAccess.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif


// This test creates backing resources exactly sized to [kFullSize x kFullSize].
// It then wraps them in an SkSpecialImage with only the center (red) region being active.
// It then draws the SkSpecialImage to a full sized (all blue) canvas and checks that none
// of the inactive (green) region leaked out.

static const int kSmallerSize = 10;
static const int kPad = 3;
static const int kFullSize = kSmallerSize + 2 * kPad;

// Create a bitmap with red in the center and green around it
static SkBitmap create_bm() {
    SkBitmap bm;
    bm.allocN32Pixels(kFullSize, kFullSize, true);

    SkCanvas temp(bm);

    temp.clear(SK_ColorGREEN);
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(false);

    temp.drawRect(SkRect::MakeXYWH(SkIntToScalar(kPad), SkIntToScalar(kPad),
                                   SkIntToScalar(kSmallerSize), SkIntToScalar(kSmallerSize)), 
                  p);

    return bm;
}

// Basic test of the SkSpecialImage public API (e.g., peekTexture, peekPixels & draw)
static void test_image(const sk_sp<SkSpecialImage>& img, skiatest::Reporter* reporter,
                       bool peekPixelsSucceeds, bool peekTextureSucceeds,
                       int offset, int size) {
    const SkIRect subset = TestingSpecialImageAccess::Subset(img.get());
    REPORTER_ASSERT(reporter, offset == subset.left());
    REPORTER_ASSERT(reporter, offset == subset.top());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.width());
    REPORTER_ASSERT(reporter, kSmallerSize == subset.height());

    //--------------
    REPORTER_ASSERT(reporter, peekTextureSucceeds ==
                                        !!TestingSpecialImageAccess::PeekTexture(img.get()));

    //--------------
    SkPixmap pixmap;
    REPORTER_ASSERT(reporter, peekPixelsSucceeds ==
                              !!TestingSpecialImageAccess::PeekPixels(img.get(), &pixmap));
    if (peekPixelsSucceeds) {
        REPORTER_ASSERT(reporter, size == pixmap.width());
        REPORTER_ASSERT(reporter, size == pixmap.height());
    }

    //--------------
    SkImageInfo info = SkImageInfo::MakeN32(kFullSize, kFullSize, kOpaque_SkAlphaType);

    sk_sp<SkSpecialSurface> surf(img->makeSurface(info));

    SkCanvas* canvas = surf->getCanvas();

    canvas->clear(SK_ColorBLUE);
    img->draw(canvas, SkIntToScalar(kPad), SkIntToScalar(kPad), nullptr);

    SkBitmap bm;
    bm.allocN32Pixels(kFullSize, kFullSize, true);

    bool result = canvas->readPixels(bm.info(), bm.getPixels(), bm.rowBytes(), 0, 0);
    SkASSERT_RELEASE(result);

    // Only the center (red) portion should've been drawn into the canvas
    REPORTER_ASSERT(reporter, SK_ColorBLUE == bm.getColor(kPad-1, kPad-1));
    REPORTER_ASSERT(reporter, SK_ColorRED  == bm.getColor(kPad, kPad));
    REPORTER_ASSERT(reporter, SK_ColorRED  == bm.getColor(kSmallerSize+kPad-1,
                                                          kSmallerSize+kPad-1));
    REPORTER_ASSERT(reporter, SK_ColorBLUE == bm.getColor(kSmallerSize+kPad,
                                                          kSmallerSize+kPad));
}

DEF_TEST(SpecialImage_Raster, reporter) {
    SkBitmap bm = create_bm();

    sk_sp<SkSpecialImage> fullSImage(SkSpecialImage::MakeFromRaster(
                                                            nullptr,
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            bm));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(SkSpecialImage::MakeFromRaster(nullptr, subset, bm));
        test_image(subSImg1, reporter, true, false, kPad, kFullSize);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImage->makeSubset(subset));
        test_image(subSImg2, reporter, true, false, 0, kSmallerSize);
    }
}

DEF_TEST(SpecialImage_Image, reporter) {
    SkBitmap bm = create_bm();

    sk_sp<SkImage> fullImage(SkImage::MakeFromBitmap(bm));

    sk_sp<SkSpecialImage> fullSImage(SkSpecialImage::MakeFromImage(
                                                            nullptr,
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            fullImage));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(SkSpecialImage::MakeFromImage(nullptr, subset,
                                                                     fullImage));
        test_image(subSImg1, reporter, true, false, kPad, kFullSize);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImage->makeSubset(subset));
        test_image(subSImg2, reporter, true, false, 0, kSmallerSize);
    }
}

DEF_TEST(SpecialImage_Pixmap, reporter) {
    SkAutoPixmapStorage pixmap;

    const SkImageInfo info = SkImageInfo::MakeN32(kFullSize, kFullSize, kOpaque_SkAlphaType);
    pixmap.alloc(info);
    pixmap.erase(SK_ColorGREEN);

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    pixmap.erase(SK_ColorRED, subset);

    {
        // The SkAutoPixmapStorage keeps hold of the memory
        sk_sp<SkSpecialImage> img(SkSpecialImage::MakeFromPixmap(nullptr, subset, pixmap,
                                                                 nullptr, nullptr));
        test_image(img, reporter, true, false, kPad, kFullSize);
    }

    {
        // The image takes ownership of the memory
        sk_sp<SkSpecialImage> img(SkSpecialImage::MakeFromPixmap(
                                               nullptr, subset, pixmap,
                                               [] (void* addr, void*) -> void {
                                                   sk_free(addr);
                                               },
                                               nullptr));
        pixmap.release();
        test_image(img, reporter, true, false, kPad, kFullSize);
    }
}


#if SK_SUPPORT_GPU

static void test_texture_backed(skiatest::Reporter* reporter,
                                const sk_sp<SkSpecialImage>& orig,
                                const sk_sp<SkSpecialImage>& gpuBacked) {
    REPORTER_ASSERT(reporter, gpuBacked);
    REPORTER_ASSERT(reporter, gpuBacked->peekTexture());
    REPORTER_ASSERT(reporter, gpuBacked->uniqueID() == orig->uniqueID());
    REPORTER_ASSERT(reporter, gpuBacked->subset().width() == orig->subset().width() &&
                              gpuBacked->subset().height() == orig->subset().height());
}

// Test out the SkSpecialImage::makeTextureImage entry point
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_MakeTexture, reporter, context) {
    SkBitmap bm = create_bm();

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        // raster
        sk_sp<SkSpecialImage> rasterImage(SkSpecialImage::MakeFromRaster(
                                                                        nullptr,
                                                                        SkIRect::MakeWH(kFullSize,
                                                                                        kFullSize),
                                                                        bm));

        {
            sk_sp<SkSpecialImage> fromRaster(rasterImage->makeTextureImage(nullptr, context));
            test_texture_backed(reporter, rasterImage, fromRaster);
        }

        {
            sk_sp<SkSpecialImage> subRasterImage(rasterImage->makeSubset(subset));

            sk_sp<SkSpecialImage> fromSubRaster(subRasterImage->makeTextureImage(nullptr, context));
            test_texture_backed(reporter, subRasterImage, fromSubRaster);
        }
    }

    {
        // gpu
        GrSurfaceDesc desc;
        desc.fConfig = kSkia8888_GrPixelConfig;
        desc.fFlags = kNone_GrSurfaceFlags;
        desc.fWidth = kFullSize;
        desc.fHeight = kFullSize;

        SkAutoTUnref<GrTexture> texture(context->textureProvider()->createTexture(desc,
                                                                                  SkBudgeted::kNo,
                                                                                  bm.getPixels(),
                                                                                  0));
        if (!texture) {
            return;
        }

        sk_sp<SkSpecialImage> gpuImage(SkSpecialImage::MakeFromGpu(
                                                                nullptr,
                                                                SkIRect::MakeWH(kFullSize,
                                                                                kFullSize),
                                                                kNeedNewImageUniqueID_SpecialImage,
                                                                texture));

        {
            sk_sp<SkSpecialImage> fromGPU(gpuImage->makeTextureImage(nullptr, context));
            test_texture_backed(reporter, gpuImage, fromGPU);
        }

        {
            sk_sp<SkSpecialImage> subGPUImage(gpuImage->makeSubset(subset));

            sk_sp<SkSpecialImage> fromSubGPU(subGPUImage->makeTextureImage(nullptr, context));
            test_texture_backed(reporter, subGPUImage, fromSubGPU);
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialImage_Gpu, reporter, context) {
    SkBitmap bm = create_bm();

    GrSurfaceDesc desc;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fFlags  = kNone_GrSurfaceFlags;
    desc.fWidth  = kFullSize;
    desc.fHeight = kFullSize;

    SkAutoTUnref<GrTexture> texture(context->textureProvider()->createTexture(desc,
                                                                              SkBudgeted::kNo,
                                                                              bm.getPixels(), 0));
    if (!texture) {
        return;
    }

    sk_sp<SkSpecialImage> fullSImg(SkSpecialImage::MakeFromGpu(
                                                            nullptr,
                                                            SkIRect::MakeWH(kFullSize, kFullSize),
                                                            kNeedNewImageUniqueID_SpecialImage,
                                                            texture));

    const SkIRect& subset = SkIRect::MakeXYWH(kPad, kPad, kSmallerSize, kSmallerSize);

    {
        sk_sp<SkSpecialImage> subSImg1(SkSpecialImage::MakeFromGpu(
                                                               nullptr, subset, 
                                                               kNeedNewImageUniqueID_SpecialImage,
                                                               texture));
        test_image(subSImg1, reporter, false, true, kPad, kFullSize);
    }

    {
        sk_sp<SkSpecialImage> subSImg2(fullSImg->makeSubset(subset));
        test_image(subSImg2, reporter, false, true, kPad, kFullSize);
    }
}

#endif
