/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrRectBlurEffect.fp; do not modify.
 **************************************************************************************************/
#ifndef GrRectBlurEffect_DEFINED
#define GrRectBlurEffect_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkTypes.h"

#include <cmath>
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkGpuBlurUtils.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrThreadSafeCache.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrTextureEffect.h"

#include "src/gpu/GrFragmentProcessor.h"

class GrRectBlurEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> MakeIntegralFP(GrRecordingContext* rContext,
                                                               float sixSigma) {
        SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(sixSigma / 6.f));
        auto threadSafeCache = rContext->priv().threadSafeCache();

        int width = SkGpuBlurUtils::CreateIntegralTable(sixSigma, nullptr);

        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        GrUniqueKey::Builder builder(&key, kDomain, 1, "Rect Blur Mask");
        builder[0] = width;
        builder.finish();

        SkMatrix m = SkMatrix::Scale(width / sixSigma, 1.f);

        GrSurfaceProxyView view = threadSafeCache->find(key);

        if (view) {
            SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
            return GrTextureEffect::Make(
                    std::move(view), kPremul_SkAlphaType, m, GrSamplerState::Filter::kLinear);
        }

        SkBitmap bitmap;
        if (!SkGpuBlurUtils::CreateIntegralTable(sixSigma, &bitmap)) {
            return {};
        }

        view = std::get<0>(GrMakeUncachedBitmapProxyView(rContext, bitmap));
        if (!view) {
            return {};
        }

        view = threadSafeCache->add(key, view);

        SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
        return GrTextureEffect::Make(
                std::move(view), kPremul_SkAlphaType, m, GrSamplerState::Filter::kLinear);
    }

    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                     GrRecordingContext* context,
                                                     const GrShaderCaps& caps,
                                                     const SkRect& srcRect,
                                                     const SkMatrix& viewMatrix,
                                                     float transformedSigma) {
        SkASSERT(viewMatrix.preservesRightAngles());
        SkASSERT(srcRect.isSorted());

        if (SkGpuBlurUtils::IsEffectivelyZeroSigma(transformedSigma)) {
            // No need to blur the rect
            return inputFP;
        }

        SkMatrix invM;
        SkRect rect;
        if (viewMatrix.rectStaysRect()) {
            invM = SkMatrix::I();
            // We can do everything in device space when the src rect projects to a rect in device
            // space.
            SkAssertResult(viewMatrix.mapRect(&rect, srcRect));
        } else {
            // The view matrix may scale, perhaps anisotropically. But we want to apply our device
            // space "transformedSigma" to the delta of frag coord from the rect edges. Factor out
            // the scaling to define a space that is purely rotation/translation from device space
            // (and scale from src space) We'll meet in the middle: pre-scale the src rect to be in
            // this space and then apply the inverse of the rotation/translation portion to the
            // frag coord.
            SkMatrix m;
            SkSize scale;
            if (!viewMatrix.decomposeScale(&scale, &m)) {
                return nullptr;
            }
            if (!m.invert(&invM)) {
                return nullptr;
            }
            rect = {srcRect.left() * scale.width(),
                    srcRect.top() * scale.height(),
                    srcRect.right() * scale.width(),
                    srcRect.bottom() * scale.height()};
        }

        if (!caps.floatIs32Bits()) {
            // We promote the math that gets us into the Gaussian space to full float when the rect
            // coords are large. If we don't have full float then fail. We could probably clip the
            // rect to an outset device bounds instead.
            if (SkScalarAbs(rect.fLeft) > 16000.f || SkScalarAbs(rect.fTop) > 16000.f ||
                SkScalarAbs(rect.fRight) > 16000.f || SkScalarAbs(rect.fBottom) > 16000.f) {
                return nullptr;
            }
        }

        const float sixSigma = 6 * transformedSigma;
        std::unique_ptr<GrFragmentProcessor> integral = MakeIntegralFP(context, sixSigma);
        if (!integral) {
            return nullptr;
        }

        // In the fast variant we think of the midpoint of the integral texture as aligning
        // with the closest rect edge both in x and y. To simplify texture coord calculation we
        // inset the rect so that the edge of the inset rect corresponds to t = 0 in the texture.
        // It actually simplifies things a bit in the !isFast case, too.
        float threeSigma = sixSigma / 2;
        SkRect insetRect = {rect.left() + threeSigma,
                            rect.top() + threeSigma,
                            rect.right() - threeSigma,
                            rect.bottom() - threeSigma};

        // In our fast variant we find the nearest horizontal and vertical edges and for each
        // do a lookup in the integral texture for each and multiply them. When the rect is
        // less than 6 sigma wide then things aren't so simple and we have to consider both the
        // left and right edge of the rectangle (and similar in y).
        bool isFast = insetRect.isSorted();
        return std::unique_ptr<GrFragmentProcessor>(new GrRectBlurEffect(std::move(inputFP),
                                                                         insetRect,
                                                                         !invM.isIdentity(),
                                                                         invM,
                                                                         std::move(integral),
                                                                         isFast));
    }
    GrRectBlurEffect(const GrRectBlurEffect& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "RectBlurEffect"; }
    SkRect rect;
    bool applyInvVM;
    SkMatrix invVM;
    bool isFast;

private:
    GrRectBlurEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                     SkRect rect,
                     bool applyInvVM,
                     SkMatrix invVM,
                     std::unique_ptr<GrFragmentProcessor> integral,
                     bool isFast)
            : INHERITED(kGrRectBlurEffect_ClassID,
                        (OptimizationFlags)(inputFP ? ProcessorOptimizationFlags(inputFP.get())
                                                    : kAll_OptimizationFlags) &
                                kCompatibleWithCoverageAsAlpha_OptimizationFlag)
            , rect(rect)
            , applyInvVM(applyInvVM)
            , invVM(invVM)
            , isFast(isFast) {
        this->registerChild(std::move(inputFP), SkSL::SampleUsage::PassThrough());
        this->registerChild(std::move(integral), SkSL::SampleUsage::Explicit());
    }
    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
#if GR_TEST_UTILS
    SkString onDumpInfo() const override;
#endif
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    using INHERITED = GrFragmentProcessor;
};
#endif
