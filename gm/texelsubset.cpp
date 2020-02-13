/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkTArray.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"
#include "tools/Resources.h"
#include "tools/gpu/TestOps.h"

#include <memory>
#include <utility>

namespace skiagm {
/**
 * This GM directly exercises GrTextureEffect::MakeTexelSubset.
 */
class TexelSubset : public GpuGM {
public:
    TexelSubset(GrSamplerState::Filter filter, bool upscale) : fFilter(filter), fUpscale(upscale) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        SkString name("texel_subset");
        switch (fFilter) {
            case GrSamplerState::Filter::kNearest:
                name.append("_nearest");
                break;
            case GrSamplerState::Filter::kBilerp:
                name.append("_bilerp");
                break;
            case GrSamplerState::Filter::kMipMap:
                name.append("_mip_map");
                break;
        }
        name.append(fUpscale ? "_up" : "_down");
        return name;
    }

    SkISize onISize() override {
        static constexpr int kN = GrSamplerState::kWrapModeCount;
        int w = kTestPad + 2*kN * (kImageSize.width()  + 2*kDrawPad + kTestPad);
        int h = kTestPad + 2*kN * (kImageSize.height() + 2*kDrawPad + kTestPad);
        return {w, h};
    }

    void onOnceBeforeDraw() override {
        GetResourceAsBitmap("images/mandrill_128.png", &fBitmap);
        // Make the bitmap non-square to detect any width/height confusion.
        fBitmap.extractSubset(&fBitmap, SkIRect::MakeSize(fBitmap.dimensions()).makeInset(0, 20));
        SkASSERT(fBitmap.dimensions() == kImageSize);
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                      SkCanvas* canvas, SkString* errorMsg) override {
        GrMipMapped mipMapped = fFilter == GrSamplerState::Filter::kMipMap &&
                                context->priv().caps()->mipMapSupport()
                ? GrMipMapped::kYes : GrMipMapped::kNo;
        GrBitmapTextureMaker maker(context, fBitmap);
        auto[view, grCT] = maker.view(mipMapped);
        if (!view.proxy()) {
            *errorMsg = "Failed to create proxy.";
            return DrawResult::kFail;
        }

        SkIRect texelSubset;
        // Use a smaller subset when upscaling so that wrap is hit on all sides of the rect we
        // will draw.
        if (fUpscale) {
            texelSubset = SkIRect::MakeXYWH(fBitmap.width()/3 - 1, 2*fBitmap.height()/5 - 1,
                                            fBitmap.width()/4 + 2,   fBitmap.height()/5 + 2);
        } else {
            texelSubset = SkIRect::MakeXYWH(  fBitmap.width()/8 - 1,   fBitmap.height()/7 - 1,
                                            3*fBitmap.width()/5 + 2, 4*fBitmap.height()/5 + 2);
        }

        SkTArray<SkMatrix> textureMatrices;

        SkRect a = SkRect::Make(texelSubset);
        SkRect b = fUpscale ? a.makeInset (.31f * a.width(), .31f * a.height())
                            : a.makeOutset(.25f * a.width(), .25f * a.height());
        textureMatrices.push_back().setRectToRect(a, b, SkMatrix::kFill_ScaleToFit);

        b = fUpscale ? a.makeInset (.25f * a.width(), .35f * a.height())
                     : a.makeOutset(.20f * a.width(), .35f * a.height());
        textureMatrices.push_back().setRectToRect(a, b, SkMatrix::kFill_ScaleToFit);
        textureMatrices.back().preRotate(45.f, a.centerX(), a.centerY());
        textureMatrices.back().postSkew(.05f, -.05f);

        SkBitmap subsetBmp;
        fBitmap.extractSubset(&subsetBmp, texelSubset);
        subsetBmp.setImmutable();
        GrBitmapTextureMaker subsetMaker(context, subsetBmp);
        auto[subsetView, subsetCT] = subsetMaker.view(mipMapped);

        SkRect localRect = SkRect::Make(fBitmap.bounds()).makeOutset(kDrawPad, kDrawPad);

        auto size = this->onISize();

        SkScalar y = kDrawPad + kTestPad;
        SkRect drawRect;
        for (int tm = 0; tm < textureMatrices.count(); ++tm) {
            for (int my = 0; my < GrSamplerState::kWrapModeCount; ++my) {
                SkScalar x = kDrawPad + kTestPad;
                auto wmy = static_cast<GrSamplerState::WrapMode>(my);
                for (int mx = 0; mx < GrSamplerState::kWrapModeCount; ++mx) {
                    auto wmx = static_cast<GrSamplerState::WrapMode>(mx);

                    const auto& caps = *context->priv().caps();

                    GrSamplerState sampler(wmx, wmy, fFilter);

                    // kRepeat and kMirrorRepeat don't currently work with filtering.
                    bool shouldWork =
                            fFilter == GrSamplerState::Filter::kNearest || !sampler.isRepeated();

                    drawRect = localRect.makeOffset(x, y);

                    std::unique_ptr<GrFragmentProcessor> fp1;
                    if (shouldWork) {
                        fp1 = GrTextureEffect::MakeTexelSubset(view,
                                                               fBitmap.alphaType(),
                                                               textureMatrices[tm],
                                                               sampler,
                                                               texelSubset,
                                                               caps);
                        if (!fp1) {
                            continue;
                        }
                    } else {
                        fp1 = GrConstColorProcessor::Make(
                                {0, 0, 0, 0.3f}, GrConstColorProcessor::InputMode::kModulateRGBA);
                    }
                    // Throw a translate in the local matrix just to test having something other
                    // than identity. Compensate with an offset local rect.
                    static constexpr SkVector kT = {-100, 300};
                    if (auto op = sk_gpu_test::test_ops::MakeRect(context,
                                                                  std::move(fp1),
                                                                  drawRect,
                                                                  localRect.makeOffset(kT),
                                                                  SkMatrix::MakeTrans(-kT))) {
                        renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    }

                    x += localRect.width() + kTestPad;

                    // Now draw with a subsetted proxy using fixed function texture sampling
                    // rather than a texture subset as a comparison.
                    drawRect = localRect.makeOffset(x, y);
                    SkMatrix subsetTextureMatrix = SkMatrix::Concat(
                            SkMatrix::MakeTrans(-texelSubset.topLeft()), textureMatrices[tm]);

                    auto fp2 = GrTextureEffect::Make(subsetView, fBitmap.alphaType(),
                                                     subsetTextureMatrix,
                                                     GrSamplerState(wmx, wmy, fFilter), caps);
                    if (auto op = sk_gpu_test::test_ops::MakeRect(context, std::move(fp2), drawRect,
                                                                  localRect)) {
                        renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    }

                    if (mx < GrSamplerState::kWrapModeCount - 1) {
                        SkScalar midX =
                                SkScalarFloorToScalar(drawRect.right() + kTestPad/2.f) + 0.5f;
                        canvas->drawLine({midX, -1}, {midX, (float)size.fHeight+1}, {});
                    }
                    x += localRect.width() + kTestPad;
                }
                if (my < GrSamplerState::kWrapModeCount - 1) {
                    SkScalar midY = SkScalarFloorToScalar(drawRect.bottom() + kTestPad/2.f) + 0.5f;
                    canvas->drawLine({-1, midY}, {(float)size.fWidth+1, midY}, {});
                }
                y += localRect.height() + kTestPad;
            }
            if (tm < textureMatrices.count() - 1) {
                SkPaint paint;
                paint.setColor(SK_ColorRED);
                SkScalar midY = SkScalarFloorToScalar(drawRect.bottom() + kTestPad/2.f) + 0.5f;
                canvas->drawLine({-1, midY}, {(float)size.fWidth + 1, midY}, paint);
            }
        }
        return DrawResult::kOk;
    }

private:
    static constexpr SkISize kImageSize = {128, 88};
    static constexpr SkScalar kDrawPad = 10.f;
    static constexpr SkScalar kTestPad = 10.f;
    SkBitmap fBitmap;
    GrSamplerState::Filter fFilter;
    bool fUpscale;

    typedef GM INHERITED;
};

DEF_GM(return new TexelSubset(GrSamplerState::Filter::kNearest, false);)
DEF_GM(return new TexelSubset(GrSamplerState::Filter::kNearest, true);)
DEF_GM(return new TexelSubset(GrSamplerState::Filter::kBilerp , false);)
DEF_GM(return new TexelSubset(GrSamplerState::Filter::kBilerp , true);)
// It doesn't make sense to have upscaling MIP map.
DEF_GM(return new TexelSubset(GrSamplerState::Filter::kMipMap,  false);)

}
