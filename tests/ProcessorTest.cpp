/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuResource.h"
#include "GrMemoryPool.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "ops/GrMeshDrawOp.h"
#include "ops/GrRectOpFactory.h"
#include "TestUtils.h"

#include <random>

namespace {
class TestOp : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID
    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          std::unique_ptr<GrFragmentProcessor> fp) {
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

        return pool->allocate<TestOp>(std::move(fp));
    }

    const char* name() const override { return "TestOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessors.visitProxies(func);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        static constexpr GrProcessorAnalysisColor kUnknownColor;
        GrColor overrideColor;
        fProcessors.finalize(kUnknownColor, GrProcessorAnalysisCoverage::kNone, clip, false, caps,
                             &overrideColor);
        return RequiresDstTexture::kNo;
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    TestOp(std::unique_ptr<GrFragmentProcessor> fp)
            : INHERITED(ClassID()), fProcessors(std::move(fp)) {
        this->setBounds(SkRect::MakeWH(100, 100), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) override { return; }

    GrProcessorSet fProcessors;

    typedef GrMeshDrawOp INHERITED;
};

/**
 * FP used to test ref/IO counts on owned GrGpuResources. Can also be a parent FP to test counts
 * of resources owned by child FPs.
 */
class TestFP : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child) {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(std::move(child)));
    }
    static std::unique_ptr<GrFragmentProcessor> Make(const SkTArray<sk_sp<GrTextureProxy>>& proxies,
                                                     const SkTArray<sk_sp<GrBuffer>>& buffers) {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(proxies, buffers));
    }

    const char* name() const override { return "test"; }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // We don't really care about reusing these.
        static int32_t gKey = 0;
        b->add32(sk_atomic_inc(&gKey));
    }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new TestFP(*this));
    }

private:
    TestFP(const SkTArray<sk_sp<GrTextureProxy>>& proxies, const SkTArray<sk_sp<GrBuffer>>& buffers)
            : INHERITED(kTestFP_ClassID, kNone_OptimizationFlags), fSamplers(4) {
        for (const auto& proxy : proxies) {
            fSamplers.emplace_back(proxy);
        }
        this->setTextureSamplerCnt(fSamplers.count());
    }

    TestFP(std::unique_ptr<GrFragmentProcessor> child)
            : INHERITED(kTestFP_ClassID, kNone_OptimizationFlags), fSamplers(4) {
        this->registerChildProcessor(std::move(child));
    }

    explicit TestFP(const TestFP& that)
            : INHERITED(kTestFP_ClassID, that.optimizationFlags()), fSamplers(4) {
        for (int i = 0; i < that.fSamplers.count(); ++i) {
            fSamplers.emplace_back(that.fSamplers[i]);
        }
        for (int i = 0; i < that.numChildProcessors(); ++i) {
            this->registerChildProcessor(that.childProcessor(i).clone());
        }
        this->setTextureSamplerCnt(fSamplers.count());
    }

    virtual GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class TestGLSLFP : public GrGLSLFragmentProcessor {
        public:
            TestGLSLFP() {}
            void emitCode(EmitArgs& args) override {
                GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, args.fInputColor);
            }

        private:
        };
        return new TestGLSLFP();
    }

    bool onIsEqual(const GrFragmentProcessor&) const override { return false; }
    const TextureSampler& onTextureSampler(int i) const override { return fSamplers[i]; }

    GrTAllocator<TextureSampler> fSamplers;
    typedef GrFragmentProcessor INHERITED;
};
}

template <typename T>
inline void testingOnly_getIORefCnts(const T* resource, int* refCnt, int* readCnt, int* writeCnt) {
    *refCnt = resource->fRefCnt;
    *readCnt = resource->fPendingReads;
    *writeCnt = resource->fPendingWrites;
}

void testingOnly_getIORefCnts(GrTextureProxy* proxy, int* refCnt, int* readCnt, int* writeCnt) {
    *refCnt = proxy->getBackingRefCnt_TestOnly();
    *readCnt = proxy->getPendingReadCnt_TestOnly();
    *writeCnt = proxy->getPendingWriteCnt_TestOnly();
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(ProcessorRefTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fWidth = 10;
    desc.fHeight = 10;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    for (bool makeClone : {false, true}) {
        for (int parentCnt = 0; parentCnt < 2; parentCnt++) {
            sk_sp<GrRenderTargetContext> renderTargetContext(
                    context->contextPriv().makeDeferredRenderTargetContext(
                                                             SkBackingFit::kApprox, 1, 1,
                                                             kRGBA_8888_GrPixelConfig, nullptr));
            {
                sk_sp<GrTextureProxy> proxy1 = proxyProvider->createProxy(
                        desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact, SkBudgeted::kYes);
                sk_sp<GrTextureProxy> proxy2 = proxyProvider->createProxy(
                        desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact, SkBudgeted::kYes);
                sk_sp<GrTextureProxy> proxy3 = proxyProvider->createProxy(
                        desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact, SkBudgeted::kYes);
                sk_sp<GrTextureProxy> proxy4 = proxyProvider->createProxy(
                        desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact, SkBudgeted::kYes);
                {
                    SkTArray<sk_sp<GrTextureProxy>> proxies;
                    SkTArray<sk_sp<GrBuffer>> buffers;
                    proxies.push_back(proxy1);
                    auto fp = TestFP::Make(std::move(proxies), std::move(buffers));
                    for (int i = 0; i < parentCnt; ++i) {
                        fp = TestFP::Make(std::move(fp));
                    }
                    std::unique_ptr<GrFragmentProcessor> clone;
                    if (makeClone) {
                        clone = fp->clone();
                    }
                    std::unique_ptr<GrDrawOp> op(TestOp::Make(context, std::move(fp)));
                    renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    if (clone) {
                        op = TestOp::Make(context, std::move(clone));
                        renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                    }
                }
                int refCnt, readCnt, writeCnt;

                testingOnly_getIORefCnts(proxy1.get(), &refCnt, &readCnt, &writeCnt);
                // IO counts should be double if there is a clone of the FP.
                int ioRefMul = makeClone ? 2 : 1;
                REPORTER_ASSERT(reporter, -1 == refCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 1 == readCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 0 == writeCnt);

                context->flush();

                testingOnly_getIORefCnts(proxy1.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 0 == readCnt);
                REPORTER_ASSERT(reporter, ioRefMul * 0 == writeCnt);

            }
        }
    }
}

// This test uses the random GrFragmentProcessor test factory, which relies on static initializers.
#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#include "SkCommandLineFlags.h"
DEFINE_bool(randomProcessorTest, false, "Use non-deterministic seed for random processor tests?");
DEFINE_uint32(processorSeed, 0, "Use specific seed for processor tests. Overridden by " \
                                "--randomProcessorTest.");

#if GR_TEST_UTILS

static GrColor input_texel_color(int i, int j) {
    GrColor color = GrColorPackRGBA((uint8_t)j, (uint8_t)(i + j), (uint8_t)(2 * j - i), (uint8_t)i);
    return GrPremulColor(color);
}

static SkPMColor4f input_texel_color4f(int i, int j) {
    return GrColor4f::FromGrColor(input_texel_color(i, j)).asRGBA4f<kPremul_SkAlphaType>();
}

void test_draw_op(GrContext* context,
                  GrRenderTargetContext* rtc,
                  std::unique_ptr<GrFragmentProcessor> fp,
                  sk_sp<GrTextureProxy> inputDataProxy) {
    GrPaint paint;
    paint.addColorTextureProcessor(std::move(inputDataProxy), SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    auto op = GrRectOpFactory::MakeNonAAFill(context, std::move(paint), SkMatrix::I(),
                                             SkRect::MakeWH(rtc->width(), rtc->height()),
                                             GrAAType::kNone);
    rtc->addDrawOp(GrNoClip(), std::move(op));
}

/** Initializes the two test texture proxies that are available to the FP test factories. */
bool init_test_textures(GrProxyProvider* proxyProvider, SkRandom* random,
                        sk_sp<GrTextureProxy> proxies[2]) {
    static const int kTestTextureSize = 256;

    {
        // Put premul data into the RGBA texture that the test FPs can optionally use.
        std::unique_ptr<GrColor[]> rgbaData(new GrColor[kTestTextureSize * kTestTextureSize]);
        for (int y = 0; y < kTestTextureSize; ++y) {
            for (int x = 0; x < kTestTextureSize; ++x) {
                rgbaData[kTestTextureSize * y + x] =
                        input_texel_color(random->nextULessThan(256), random->nextULessThan(256));
            }
        }

        SkImageInfo ii = SkImageInfo::Make(kTestTextureSize, kTestTextureSize,
                                           kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        SkPixmap pixmap(ii, rgbaData.get(), ii.minRowBytes());
        sk_sp<SkImage> img = SkImage::MakeRasterCopy(pixmap);
        proxies[0] = proxyProvider->createTextureProxy(img, kNone_GrSurfaceFlags, 1,
                                                       SkBudgeted::kYes, SkBackingFit::kExact);
    }

    {
        // Put random values into the alpha texture that the test FPs can optionally use.
        std::unique_ptr<uint8_t[]> alphaData(new uint8_t[kTestTextureSize * kTestTextureSize]);
        for (int y = 0; y < kTestTextureSize; ++y) {
            for (int x = 0; x < kTestTextureSize; ++x) {
                alphaData[kTestTextureSize * y + x] = random->nextULessThan(256);
            }
        }

        SkImageInfo ii = SkImageInfo::Make(kTestTextureSize, kTestTextureSize,
                                           kAlpha_8_SkColorType, kPremul_SkAlphaType);
        SkPixmap pixmap(ii, alphaData.get(), ii.minRowBytes());
        sk_sp<SkImage> img = SkImage::MakeRasterCopy(pixmap);
        proxies[1] = proxyProvider->createTextureProxy(img, kNone_GrSurfaceFlags, 1,
                                                       SkBudgeted::kYes, SkBackingFit::kExact);
    }

    return proxies[0] && proxies[1];
}

// Creates a texture of premul colors used as the output of the fragment processor that precedes
// the fragment processor under test. Color values are those provided by input_texel_color().
sk_sp<GrTextureProxy> make_input_texture(GrProxyProvider* proxyProvider, int width, int height) {
    std::unique_ptr<GrColor[]> data(new GrColor[width * height]);
    for (int y = 0; y < width; ++y) {
        for (int x = 0; x < height; ++x) {
            data.get()[width * y + x] = input_texel_color(x, y);
        }
    }

    SkImageInfo ii = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkPixmap pixmap(ii, data.get(), ii.minRowBytes());
    sk_sp<SkImage> img = SkImage::MakeRasterCopy(pixmap);
    return proxyProvider->createTextureProxy(img, kNone_GrSurfaceFlags, 1,
                                             SkBudgeted::kYes, SkBackingFit::kExact);
}

bool log_surface_context(sk_sp<GrSurfaceContext> src, SkString* dst) {
    SkImageInfo ii = SkImageInfo::Make(src->width(), src->height(), kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    SkBitmap bm;
    SkAssertResult(bm.tryAllocPixels(ii));
    SkAssertResult(src->readPixels(ii, bm.getPixels(), bm.rowBytes(), 0, 0));

    return bitmap_to_base64_data_uri(bm, dst);
}

bool log_surface_proxy(GrContext* context, sk_sp<GrSurfaceProxy> src, SkString* dst) {
    sk_sp<GrSurfaceContext> sContext(context->contextPriv().makeWrappedSurfaceContext(src));
    return log_surface_context(sContext, dst);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(ProcessorOptimizationValidationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    auto resourceProvider = context->contextPriv().resourceProvider();
    using FPFactory = GrFragmentProcessorTestFactory;

    uint32_t seed = FLAGS_processorSeed;
    if (FLAGS_randomProcessorTest) {
        std::random_device rd;
        seed = rd();
    }
    // If a non-deterministic bot fails this test, check the output to see what seed it used, then
    // use --processorSeed <seed> (without --randomProcessorTest) to reproduce.
    SkRandom random(seed);

    // Make the destination context for the test.
    static constexpr int kRenderSize = 256;
    sk_sp<GrRenderTargetContext> rtc = context->contextPriv().makeDeferredRenderTargetContext(
            SkBackingFit::kExact, kRenderSize, kRenderSize, kRGBA_8888_GrPixelConfig, nullptr);

    sk_sp<GrTextureProxy> proxies[2];
    if (!init_test_textures(proxyProvider, &random, proxies)) {
        ERRORF(reporter, "Could not create test textures");
        return;
    }
    GrProcessorTestData testData(&random, context, rtc.get(), proxies);

    auto inputTexture = make_input_texture(proxyProvider, kRenderSize, kRenderSize);

    std::unique_ptr<GrColor[]> readData(new GrColor[kRenderSize * kRenderSize]);
    // Encoded images are very verbose and this tests many potential images, so only export the
    // first failure (subsequent failures have a reasonable chance of being related).
    bool loggedFirstFailure = false;
    bool loggedFirstWarning = false;
    // Because processor factories configure themselves in random ways, this is not exhaustive.
    for (int i = 0; i < FPFactory::Count(); ++i) {
        int timesToInvokeFactory = 5;
        // Increase the number of attempts if the FP has child FPs since optimizations likely depend
        // on child optimizations being present.
        std::unique_ptr<GrFragmentProcessor> fp = FPFactory::MakeIdx(i, &testData);
        for (int j = 0; j < fp->numChildProcessors(); ++j) {
            // This value made a reasonable trade off between time and coverage when this test was
            // written.
            timesToInvokeFactory *= FPFactory::Count() / 2;
        }
        for (int j = 0; j < timesToInvokeFactory; ++j) {
            fp = FPFactory::MakeIdx(i, &testData);
            if (!fp->instantiate(resourceProvider)) {
                continue;
            }

            if (!fp->hasConstantOutputForConstantInput() && !fp->preservesOpaqueInput() &&
                !fp->compatibleWithCoverageAsAlpha()) {
                continue;
            }

            // Since we transfer away ownership of the original FP, we make a clone.
            auto clone = fp->clone();

            test_draw_op(context, rtc.get(), std::move(fp), inputTexture);
            memset(readData.get(), 0x0, sizeof(GrColor) * kRenderSize * kRenderSize);
            rtc->readPixels(SkImageInfo::Make(kRenderSize, kRenderSize, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType),
                            readData.get(), 0, 0, 0);
            if (0) {  // Useful to see what FPs are being tested.
                SkString children;
                for (int c = 0; c < clone->numChildProcessors(); ++c) {
                    if (!c) {
                        children.append("(");
                    }
                    children.append(clone->name());
                    children.append(c == clone->numChildProcessors() - 1 ? ")" : ", ");
                }
                SkDebugf("%s %s\n", clone->name(), children.c_str());
            }

            // This test has a history of being flaky on a number of devices. If an FP is logically
            // violating the optimizations, it's reasonable to expect it to violate requirements on
            // a large number of pixels in the image. Sporadic pixel violations are more indicative
            // of device errors and represents a separate problem.
#if defined(SK_SKQP_GLOBAL_ERROR_TOLERANCE)
            static constexpr int kMaxAcceptableFailedPixels = 0; // Strict when running as SKQP
#else
            static constexpr int kMaxAcceptableFailedPixels = 2 * kRenderSize; // ~0.7% of the image
#endif

            int failedPixelCount = 0;
            // Collect first optimization failure message, to be output later as a warning or an
            // error depending on whether the rendering "passed" or failed.
            SkString coverageMessage;
            SkString opaqueMessage;
            SkString constMessage;
            for (int y = 0; y < kRenderSize; ++y) {
                for (int x = 0; x < kRenderSize; ++x) {
                    bool passing = true;
                    GrColor input = input_texel_color(x, y);
                    GrColor output = readData.get()[y * kRenderSize + x];
                    if (clone->compatibleWithCoverageAsAlpha()) {
                        // A modulating processor is allowed to modulate either the input color or
                        // just the input alpha.
                        bool legalColorModulation =
                                GrColorUnpackA(output) <= GrColorUnpackA(input) &&
                                GrColorUnpackR(output) <= GrColorUnpackR(input) &&
                                GrColorUnpackG(output) <= GrColorUnpackG(input) &&
                                GrColorUnpackB(output) <= GrColorUnpackB(input);
                        bool legalAlphaModulation =
                                GrColorUnpackA(output) <= GrColorUnpackA(input) &&
                                GrColorUnpackR(output) <= GrColorUnpackA(input) &&
                                GrColorUnpackG(output) <= GrColorUnpackA(input) &&
                                GrColorUnpackB(output) <= GrColorUnpackA(input);
                        if (!legalColorModulation && !legalAlphaModulation) {
                            passing = false;

                            if (coverageMessage.isEmpty()) {
                                coverageMessage.printf("\"Modulating\" processor %s made color/"
                                        "alpha value larger. Input: 0x%08x, Output: 0x%08x, pixel "
                                        "(%d, %d).", clone->name(), input, output, x, y);
                            }
                        }
                    }
                    SkPMColor4f input4f = input_texel_color4f(x, y);
                    GrColor4f output4f = GrColor4f::FromGrColor(output);
                    SkPMColor4f expected4f;
                    if (clone->hasConstantOutputForConstantInput(input4f, &expected4f)) {
                        float rDiff = fabsf(output4f.fRGBA[0] - expected4f.fR);
                        float gDiff = fabsf(output4f.fRGBA[1] - expected4f.fG);
                        float bDiff = fabsf(output4f.fRGBA[2] - expected4f.fB);
                        float aDiff = fabsf(output4f.fRGBA[3] - expected4f.fA);
                        static constexpr float kTol = 4 / 255.f;
                        if (rDiff > kTol || gDiff > kTol || bDiff > kTol || aDiff > kTol) {
                            if (constMessage.isEmpty()) {
                                passing = false;

                                constMessage.printf("Processor %s claimed output for const input "
                                        "doesn't match actual output. Error: %f, Tolerance: %f, "
                                        "input: (%f, %f, %f, %f), actual: (%f, %f, %f, %f), "
                                        "expected(%f, %f, %f, %f)", clone->name(),
                                        SkTMax(rDiff, SkTMax(gDiff, SkTMax(bDiff, aDiff))), kTol,
                                        input4f.fR, input4f.fG, input4f.fB, input4f.fA,
                                        output4f.fRGBA[0], output4f.fRGBA[1], output4f.fRGBA[2],
                                        output4f.fRGBA[3], expected4f.fR, expected4f.fG,
                                        expected4f.fB, expected4f.fA);
                            }
                        }
                    }
                    if (GrColorIsOpaque(input) && clone->preservesOpaqueInput() &&
                        !GrColorIsOpaque(output)) {
                        passing = false;

                        if (opaqueMessage.isEmpty()) {
                            opaqueMessage.printf("Processor %s claimed opaqueness is preserved but "
                                    "it is not. Input: 0x%08x, Output: 0x%08x.",
                                    clone->name(), input, output);
                        }
                    }

                    if (!passing) {
                        // Regardless of how many optimizations the pixel violates, count it as a
                        // single bad pixel.
                        failedPixelCount++;
                    }
                }
            }

            // Finished analyzing the entire image, see if the number of pixel failures meets the
            // threshold for an FP violating the optimization requirements.
            if (failedPixelCount > kMaxAcceptableFailedPixels) {
                ERRORF(reporter, "Processor violated %d of %d pixels, seed: 0x%08x, processor: %s"
                       ", first failing pixel details are below:",
                       failedPixelCount, kRenderSize * kRenderSize, seed,
                       clone->dumpInfo().c_str());

                // Print first failing pixel's details.
                if (!coverageMessage.isEmpty()) {
                    ERRORF(reporter, coverageMessage.c_str());
                }
                if (!constMessage.isEmpty()) {
                    ERRORF(reporter, constMessage.c_str());
                }
                if (!opaqueMessage.isEmpty()) {
                    ERRORF(reporter, opaqueMessage.c_str());
                }

                if (!loggedFirstFailure) {
                    // Print with ERRORF to make sure the encoded image is output
                    SkString input;
                    log_surface_proxy(context, inputTexture, &input);
                    SkString output;
                    log_surface_context(rtc, &output);
                    ERRORF(reporter, "Input image: %s\n\n"
                           "===========================================================\n\n"
                           "Output image: %s\n", input.c_str(), output.c_str());
                    loggedFirstFailure = true;
                }
            } else {
                // Don't trigger an error, but don't just hide the failures either.
                INFOF(reporter, "Processor violated %d of %d pixels (below error threshold), seed: "
                      "0x%08x, processor: %s", failedPixelCount, kRenderSize * kRenderSize,
                      seed, clone->dumpInfo().c_str());
                if (!coverageMessage.isEmpty()) {
                    INFOF(reporter, coverageMessage.c_str());
                }
                if (!constMessage.isEmpty()) {
                    INFOF(reporter, constMessage.c_str());
                }
                if (!opaqueMessage.isEmpty()) {
                    INFOF(reporter, opaqueMessage.c_str());
                }
                if (!loggedFirstWarning) {
                    SkString input;
                    log_surface_proxy(context, inputTexture, &input);
                    SkString output;
                    log_surface_context(rtc, &output);
                    INFOF(reporter, "Input image: %s\n\n"
                          "===========================================================\n\n"
                          "Output image: %s\n", input.c_str(), output.c_str());
                    loggedFirstWarning = true;
                }
            }
        }
    }
}

// Tests that fragment processors returned by GrFragmentProcessor::clone() are equivalent to their
// progenitors.
DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(ProcessorCloneTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    auto resourceProvider = context->contextPriv().resourceProvider();

    SkRandom random;

    // Make the destination context for the test.
    static constexpr int kRenderSize = 1024;
    sk_sp<GrRenderTargetContext> rtc = context->contextPriv().makeDeferredRenderTargetContext(
            SkBackingFit::kExact, kRenderSize, kRenderSize, kRGBA_8888_GrPixelConfig, nullptr);

    sk_sp<GrTextureProxy> proxies[2];
    if (!init_test_textures(proxyProvider, &random, proxies)) {
        ERRORF(reporter, "Could not create test textures");
        return;
    }
    GrProcessorTestData testData(&random, context, rtc.get(), proxies);

    auto inputTexture = make_input_texture(proxyProvider, kRenderSize, kRenderSize);
    std::unique_ptr<GrColor[]> readData1(new GrColor[kRenderSize * kRenderSize]);
    std::unique_ptr<GrColor[]> readData2(new GrColor[kRenderSize * kRenderSize]);
    auto readInfo = SkImageInfo::Make(kRenderSize, kRenderSize, kRGBA_8888_SkColorType,
                                      kPremul_SkAlphaType);

    // Because processor factories configure themselves in random ways, this is not exhaustive.
    for (int i = 0; i < GrFragmentProcessorTestFactory::Count(); ++i) {
        static constexpr int kTimesToInvokeFactory = 10;
        for (int j = 0; j < kTimesToInvokeFactory; ++j) {
            auto fp = GrFragmentProcessorTestFactory::MakeIdx(i, &testData);
            auto clone = fp->clone();
            if (!clone) {
                ERRORF(reporter, "Clone of processor %s failed.", fp->name());
                continue;
            }
            const char* name = fp->name();
            if (!fp->instantiate(resourceProvider) || !clone->instantiate(resourceProvider)) {
                continue;
            }
            REPORTER_ASSERT(reporter, !strcmp(fp->name(), clone->name()));
            REPORTER_ASSERT(reporter, fp->compatibleWithCoverageAsAlpha() ==
                                      clone->compatibleWithCoverageAsAlpha());
            REPORTER_ASSERT(reporter, fp->isEqual(*clone));
            REPORTER_ASSERT(reporter, fp->preservesOpaqueInput() == clone->preservesOpaqueInput());
            REPORTER_ASSERT(reporter, fp->hasConstantOutputForConstantInput() ==
                                      clone->hasConstantOutputForConstantInput());
            REPORTER_ASSERT(reporter, fp->numChildProcessors() == clone->numChildProcessors());
            REPORTER_ASSERT(reporter, fp->usesLocalCoords() == clone->usesLocalCoords());
            // Draw with original and read back the results.
            test_draw_op(context, rtc.get(), std::move(fp), inputTexture);
            memset(readData1.get(), 0x0, sizeof(GrColor) * kRenderSize * kRenderSize);
            rtc->readPixels(readInfo, readData1.get(), 0, 0, 0);

            // Draw with clone and read back the results.
            test_draw_op(context, rtc.get(), std::move(clone), inputTexture);
            memset(readData2.get(), 0x0, sizeof(GrColor) * kRenderSize * kRenderSize);
            rtc->readPixels(readInfo, readData2.get(), 0, 0, 0);

            // Check that the results are the same.
            bool passing = true;
            for (int y = 0; y < kRenderSize && passing; ++y) {
                for (int x = 0; x < kRenderSize && passing; ++x) {
                    int idx = y * kRenderSize + x;
                    if (readData1[idx] != readData2[idx]) {
                        ERRORF(reporter,
                               "Processor %s made clone produced different output. "
                               "Input color: 0x%08x, Original Output Color: 0x%08x, "
                               "Clone Output Color: 0x%08x..",
                               name, input_texel_color(x, y), readData1[idx], readData2[idx]);
                        passing = false;
                    }
                }
            }
        }
    }
}

#endif  // GR_TEST_UTILS
#endif  // SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
