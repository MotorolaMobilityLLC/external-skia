/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This file was autogenerated from GrCircleBlurFragmentProcessor.fp; do not modify.
 */
#include "GrCircleBlurFragmentProcessor.h"
#if SK_SUPPORT_GPU

#include "GrResourceProvider.h"

static float make_unnormalized_half_kernel(float* halfKernel, int halfKernelSize, float sigma) {
    const float invSigma = 1.f / sigma;
    const float b = -0.5f * invSigma * invSigma;
    float tot = 0.0f;

    float t = 0.5f;
    for (int i = 0; i < halfKernelSize; ++i) {
        float value = expf(t * t * b);
        tot += value;
        halfKernel[i] = value;
        t += 1.f;
    }
    return tot;
}

static void make_half_kernel_and_summed_table(float* halfKernel, float* summedHalfKernel,
                                              int halfKernelSize, float sigma) {
    const float tot = 2.f * make_unnormalized_half_kernel(halfKernel, halfKernelSize, sigma);
    float sum = 0.f;
    for (int i = 0; i < halfKernelSize; ++i) {
        halfKernel[i] /= tot;
        sum += halfKernel[i];
        summedHalfKernel[i] = sum;
    }
}

void apply_kernel_in_y(float* results, int numSteps, float firstX, float circleR,
                       int halfKernelSize, const float* summedHalfKernelTable) {
    float x = firstX;
    for (int i = 0; i < numSteps; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            results[i] = 0;
            continue;
        }
        float y = sqrtf(circleR * circleR - x * x);

        y -= 0.5f;
        int yInt = SkScalarFloorToInt(y);
        SkASSERT(yInt >= -1);
        if (y < 0) {
            results[i] = (y + 0.5f) * summedHalfKernelTable[0];
        } else if (yInt >= halfKernelSize - 1) {
            results[i] = 0.5f;
        } else {
            float yFrac = y - yInt;
            results[i] = (1.f - yFrac) * summedHalfKernelTable[yInt] +
                         yFrac * summedHalfKernelTable[yInt + 1];
        }
    }
}

static uint8_t eval_at(float evalX, float circleR, const float* halfKernel, int halfKernelSize,
                       const float* yKernelEvaluations) {
    float acc = 0;

    float x = evalX - halfKernelSize;
    for (int i = 0; i < halfKernelSize; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = yKernelEvaluations[i];
        acc += verticalEval * halfKernel[halfKernelSize - i - 1];
    }
    for (int i = 0; i < halfKernelSize; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = yKernelEvaluations[i + halfKernelSize];
        acc += verticalEval * halfKernel[i];
    }

    return SkUnitScalarClampToByte(2.f * acc);
}

static uint8_t* create_circle_profile(float sigma, float circleR, int profileTextureWidth) {
    const int numSteps = profileTextureWidth;
    uint8_t* weights = new uint8_t[numSteps];

    int halfKernelSize = SkScalarCeilToInt(6.0f * sigma);

    halfKernelSize = ((halfKernelSize + 1) & ~1) >> 1;

    int numYSteps = numSteps + 2 * halfKernelSize;

    SkAutoTArray<float> bulkAlloc(halfKernelSize + halfKernelSize + numYSteps);
    float* halfKernel = bulkAlloc.get();
    float* summedKernel = bulkAlloc.get() + halfKernelSize;
    float* yEvals = bulkAlloc.get() + 2 * halfKernelSize;
    make_half_kernel_and_summed_table(halfKernel, summedKernel, halfKernelSize, sigma);

    float firstX = -halfKernelSize + 0.5f;
    apply_kernel_in_y(yEvals, numYSteps, firstX, circleR, halfKernelSize, summedKernel);

    for (int i = 0; i < numSteps - 1; ++i) {
        float evalX = i + 0.5f;
        weights[i] = eval_at(evalX, circleR, halfKernel, halfKernelSize, yEvals + i);
    }

    weights[numSteps - 1] = 0;
    return weights;
}

static uint8_t* create_half_plane_profile(int profileWidth) {
    SkASSERT(!(profileWidth & 0x1));

    float sigma = profileWidth / 6.f;
    int halfKernelSize = profileWidth / 2;

    SkAutoTArray<float> halfKernel(halfKernelSize);
    uint8_t* profile = new uint8_t[profileWidth];

    const float tot = 2.f * make_unnormalized_half_kernel(halfKernel.get(), halfKernelSize, sigma);
    float sum = 0.f;

    for (int i = 0; i < halfKernelSize; ++i) {
        halfKernel[halfKernelSize - i - 1] /= tot;
        sum += halfKernel[halfKernelSize - i - 1];
        profile[profileWidth - i - 1] = SkUnitScalarClampToByte(sum);
    }

    for (int i = 0; i < halfKernelSize; ++i) {
        sum += halfKernel[i];
        profile[halfKernelSize - i - 1] = SkUnitScalarClampToByte(sum);
    }

    profile[profileWidth - 1] = 0;
    return profile;
}

static sk_sp<GrTextureProxy> create_profile_texture(GrResourceProvider* resourceProvider,
                                                    const SkRect& circle, float sigma,
                                                    float* solidRadius, float* textureRadius) {
    float circleR = circle.width() / 2.0f;

    SkScalar sigmaToCircleRRatio = sigma / circleR;

    sigmaToCircleRRatio = SkTMin(sigmaToCircleRRatio, 8.f);
    SkFixed sigmaToCircleRRatioFixed;
    static const SkScalar kHalfPlaneThreshold = 0.1f;
    bool useHalfPlaneApprox = false;
    if (sigmaToCircleRRatio <= kHalfPlaneThreshold) {
        useHalfPlaneApprox = true;
        sigmaToCircleRRatioFixed = 0;
        *solidRadius = circleR - 3 * sigma;
        *textureRadius = 6 * sigma;
    } else {
        sigmaToCircleRRatioFixed = SkScalarToFixed(sigmaToCircleRRatio);

        sigmaToCircleRRatioFixed &= ~0xff;
        sigmaToCircleRRatio = SkFixedToScalar(sigmaToCircleRRatioFixed);
        sigma = circleR * sigmaToCircleRRatio;
        *solidRadius = 0;
        *textureRadius = circleR + 3 * sigma;
    }

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 1);
    builder[0] = sigmaToCircleRRatioFixed;
    builder.finish();

    sk_sp<GrTextureProxy> blurProfile = resourceProvider->findProxyByUniqueKey(
                                                                key, kTopLeft_GrSurfaceOrigin);
    if (!blurProfile) {
        static constexpr int kProfileTextureWidth = 512;
        GrSurfaceDesc texDesc;
        texDesc.fOrigin = kTopLeft_GrSurfaceOrigin;
        texDesc.fWidth = kProfileTextureWidth;
        texDesc.fHeight = 1;
        texDesc.fConfig = kAlpha_8_GrPixelConfig;

        std::unique_ptr<uint8_t[]> profile(nullptr);
        if (useHalfPlaneApprox) {
            profile.reset(create_half_plane_profile(kProfileTextureWidth));
        } else {
            SkScalar scale = kProfileTextureWidth / *textureRadius;
            profile.reset(
                    create_circle_profile(sigma * scale, circleR * scale, kProfileTextureWidth));
        }

        blurProfile = GrSurfaceProxy::MakeDeferred(resourceProvider, texDesc, SkBudgeted::kYes,
                                                   profile.get(), 0);
        if (!blurProfile) {
            return nullptr;
        }

        SkASSERT(blurProfile->origin() == kTopLeft_GrSurfaceOrigin);
        resourceProvider->assignUniqueKeyToProxy(key, blurProfile.get());
    }

    return blurProfile;
}

sk_sp<GrFragmentProcessor> GrCircleBlurFragmentProcessor::Make(GrResourceProvider* resourceProvider,
                                                               const SkRect& circle,
                                                               float sigma) {
    float solidRadius;
    float textureRadius;
    sk_sp<GrTextureProxy> profile(
            create_profile_texture(resourceProvider, circle, sigma, &solidRadius, &textureRadius));
    if (!profile) {
        return nullptr;
    }
    return sk_sp<GrFragmentProcessor>(new GrCircleBlurFragmentProcessor(
            circle, textureRadius, solidRadius, std::move(profile), resourceProvider));
}
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLCircleBlurFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    GrGLSLCircleBlurFragmentProcessor() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrCircleBlurFragmentProcessor& _outer =
                args.fFp.cast<GrCircleBlurFragmentProcessor>();
        (void)_outer;
        fCircleDataVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kVec4f_GrSLType,
                                                          kDefault_GrSLPrecision, "circleData");
        fragBuilder->codeAppendf(
                "vec2 vec = vec2((sk_FragCoord.x - %s.x) * %s.w, (sk_FragCoord.y - %s.y) * "
                "%s.w);\nfloat dist = length(vec) + (0.5 - %s.z) * %s.w;\n%s = %s * texture(%s, "
                "vec2(dist, 0.5)).%s.w;\n",
                args.fUniformHandler->getUniformCStr(fCircleDataVar),
                args.fUniformHandler->getUniformCStr(fCircleDataVar),
                args.fUniformHandler->getUniformCStr(fCircleDataVar),
                args.fUniformHandler->getUniformCStr(fCircleDataVar),
                args.fUniformHandler->getUniformCStr(fCircleDataVar),
                args.fUniformHandler->getUniformCStr(fCircleDataVar), args.fOutputColor,
                args.fInputColor ? args.fInputColor : "vec4(1)",
                fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]).c_str(),
                fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str());
    }

private:
    void onSetData(const GrGLSLProgramDataManager& data,
                   const GrFragmentProcessor& _proc) override {
        const GrCircleBlurFragmentProcessor& _outer = _proc.cast<GrCircleBlurFragmentProcessor>();
        auto circleRect = _outer.circleRect();
        (void)circleRect;
        auto textureRadius = _outer.textureRadius();
        (void)textureRadius;
        auto solidRadius = _outer.solidRadius();
        (void)solidRadius;
        UniformHandle& blurProfileSampler = fBlurProfileSamplerVar;
        (void)blurProfileSampler;
        UniformHandle& circleData = fCircleDataVar;
        (void)circleData;

        data.set4f(circleData, circleRect.centerX(), circleRect.centerY(), solidRadius,
                   1.f / textureRadius);
    }
    UniformHandle fCircleDataVar;
    UniformHandle fBlurProfileSamplerVar;
};
GrGLSLFragmentProcessor* GrCircleBlurFragmentProcessor::onCreateGLSLInstance() const {
    return new GrGLSLCircleBlurFragmentProcessor();
}
void GrCircleBlurFragmentProcessor::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                          GrProcessorKeyBuilder* b) const {}
bool GrCircleBlurFragmentProcessor::onIsEqual(const GrFragmentProcessor& other) const {
    const GrCircleBlurFragmentProcessor& that = other.cast<GrCircleBlurFragmentProcessor>();
    (void)that;
    if (fCircleRect != that.fCircleRect) return false;
    if (fTextureRadius != that.fTextureRadius) return false;
    if (fSolidRadius != that.fSolidRadius) return false;
    if (fBlurProfileSampler != that.fBlurProfileSampler) return false;
    return true;
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrCircleBlurFragmentProcessor);
#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrCircleBlurFragmentProcessor::TestCreate(
        GrProcessorTestData* testData) {
    SkScalar wh = testData->fRandom->nextRangeScalar(100.f, 1000.f);
    SkScalar sigma = testData->fRandom->nextRangeF(1.f, 10.f);
    SkRect circle = SkRect::MakeWH(wh, wh);
    return GrCircleBlurFragmentProcessor::Make(testData->resourceProvider(), circle, sigma);
}
#endif
#endif
