/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapDevice.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkComposeImageFilter.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkFlattenableSerialization.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkImageSource.h"
#include "SkLightingImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPaintImageFilter.h"
#include "SkPerlinNoiseShader.h"
#include "SkPicture.h"
#include "SkPictureImageFilter.h"
#include "SkPictureRecorder.h"
#include "SkPoint3.h"
#include "SkReadBuffer.h"
#include "SkRect.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkSurface.h"
#include "SkTableColorFilter.h"
#include "SkTileImageFilter.h"
#include "SkXfermodeImageFilter.h"
#include "Test.h"
#include "TestingSpecialImageAccess.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "SkGpuDevice.h"
#endif

static const int kBitmapSize = 4;

namespace {

class MatrixTestImageFilter : public SkImageFilter {
public:
    MatrixTestImageFilter(skiatest::Reporter* reporter, const SkMatrix& expectedMatrix)
      : SkImageFilter(0, nullptr), fReporter(reporter), fExpectedMatrix(expectedMatrix) {
    }

    bool onFilterImageDeprecated(Proxy*, const SkBitmap& src, const Context& ctx,
                                 SkBitmap* result, SkIPoint* offset) const override {
        REPORTER_ASSERT(fReporter, ctx.ctm() == fExpectedMatrix);
        return true;
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(MatrixTestImageFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        this->INHERITED::flatten(buffer);
        buffer.writeFunctionPtr(fReporter);
        buffer.writeMatrix(fExpectedMatrix);
    }

private:
    skiatest::Reporter* fReporter;
    SkMatrix fExpectedMatrix;

    typedef SkImageFilter INHERITED;
};

void draw_gradient_circle(SkCanvas* canvas, int width, int height) {
    SkScalar x = SkIntToScalar(width / 2);
    SkScalar y = SkIntToScalar(height / 2);
    SkScalar radius = SkMinScalar(x, y) * 0.8f;
    canvas->clear(0x00000000);
    SkColor colors[2];
    colors[0] = SK_ColorWHITE;
    colors[1] = SK_ColorBLACK;
    sk_sp<SkShader> shader(
        SkGradientShader::MakeRadial(SkPoint::Make(x, y), radius, colors, nullptr, 2,
                                       SkShader::kClamp_TileMode)
    );
    SkPaint paint;
    paint.setShader(shader);
    canvas->drawCircle(x, y, radius, paint);
}

SkBitmap make_gradient_circle(int width, int height) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(width, height);
    SkCanvas canvas(bitmap);
    draw_gradient_circle(&canvas, width, height);
    return bitmap;
}

class FilterList {
public:
    FilterList(SkImageFilter* input = nullptr, const SkImageFilter::CropRect* cropRect = nullptr) {
        auto cf(SkColorFilter::MakeModeFilter(SK_ColorRED, SkXfermode::kSrcIn_Mode));
        SkPoint3 location = SkPoint3::Make(0, 0, SK_Scalar1);
        SkScalar kernel[9] = {
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar(-7), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
        };
        const SkISize kernelSize = SkISize::Make(3, 3);
        const SkScalar gain = SK_Scalar1, bias = 0;
        const SkScalar five = SkIntToScalar(5);

        sk_sp<SkImage> gradientImage(SkImage::MakeFromBitmap(make_gradient_circle(64, 64)));
        SkAutoTUnref<SkImageFilter> gradientSource(SkImageSource::Create(gradientImage.get()));
        SkAutoTUnref<SkImageFilter> blur(SkBlurImageFilter::Create(five, five, input));
        SkMatrix matrix;

        matrix.setTranslate(SK_Scalar1, SK_Scalar1);
        matrix.postRotate(SkIntToScalar(45), SK_Scalar1, SK_Scalar1);

        SkRTreeFactory factory;
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(64, 64, &factory, 0);

        SkPaint greenPaint;
        greenPaint.setColor(SK_ColorGREEN);
        recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeXYWH(10, 10, 30, 20)), greenPaint);
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
        sk_sp<SkImageFilter> pictureFilter(SkPictureImageFilter::Make(picture));
        sk_sp<SkShader> shader(SkPerlinNoiseShader::MakeTurbulence(SK_Scalar1, SK_Scalar1, 1, 0));

        SkPaint paint;
        paint.setShader(shader);
        sk_sp<SkImageFilter> paintFilter(SkPaintImageFilter::Make(paint));

        sk_sp<SkShader> greenColorShader(SkShader::MakeColorShader(SK_ColorGREEN));
        SkPaint greenColorShaderPaint;
        greenColorShaderPaint.setShader(greenColorShader);
        SkImageFilter::CropRect leftSideCropRect(SkRect::MakeXYWH(0, 0, 32, 64));
        sk_sp<SkImageFilter> paintFilterLeft(SkPaintImageFilter::Make(greenColorShaderPaint,
                                                                      &leftSideCropRect));
        SkImageFilter::CropRect rightSideCropRect(SkRect::MakeXYWH(32, 0, 32, 64));
        sk_sp<SkImageFilter> paintFilterRight(SkPaintImageFilter::Make(greenColorShaderPaint,
                                                                       &rightSideCropRect));

        this->addFilter("color filter",
            SkColorFilterImageFilter::Create(cf.get(), input, cropRect));
        this->addFilter("displacement map", SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            20.0f, gradientSource.get(), input, cropRect));
        this->addFilter("blur", SkBlurImageFilter::Create(SK_Scalar1, SK_Scalar1, input, cropRect));
        this->addFilter("drop shadow", SkDropShadowImageFilter::Create(
                  SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_ColorGREEN,
                  SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, input, cropRect));
        this->addFilter("diffuse lighting", SkLightingImageFilter::CreatePointLitDiffuse(
                  location, SK_ColorGREEN, 0, 0, input, cropRect));
        this->addFilter("specular lighting",
                  SkLightingImageFilter::CreatePointLitSpecular(location, SK_ColorGREEN, 0, 0, 0,
                                                                input, cropRect));
        this->addFilter("matrix convolution",
                  SkMatrixConvolutionImageFilter::Create(
                      kernelSize, kernel, gain, bias, SkIPoint::Make(1, 1),
                      SkMatrixConvolutionImageFilter::kRepeat_TileMode, false, input, cropRect));
        this->addFilter("merge", SkMergeImageFilter::Create(input, input, SkXfermode::kSrcOver_Mode,
                  cropRect));
        this->addFilter("merge with disjoint inputs", SkMergeImageFilter::Create(
                  paintFilterLeft.get(), paintFilterRight.get(),
                  SkXfermode::kSrcOver_Mode, cropRect));
        this->addFilter("offset",
                        SkOffsetImageFilter::Create(SK_Scalar1, SK_Scalar1, input, cropRect));
        this->addFilter("dilate", SkDilateImageFilter::Create(3, 2, input, cropRect));
        this->addFilter("erode", SkErodeImageFilter::Create(2, 3, input, cropRect));
        this->addFilter("tile", SkTileImageFilter::Create(
            SkRect::MakeXYWH(0, 0, 50, 50),
            cropRect ? cropRect->rect() : SkRect::MakeXYWH(0, 0, 100, 100),
            input));
        if (!cropRect) {
            this->addFilter("matrix", SkImageFilter::CreateMatrixFilter(
                matrix, kLow_SkFilterQuality, input));
        }
        this->addFilter("blur and offset", SkOffsetImageFilter::Create(
            five, five, blur.get(), cropRect));
        this->addFilter("picture and blur", SkBlurImageFilter::Create(
            five, five, pictureFilter.get(), cropRect));
        this->addFilter("paint and blur", SkBlurImageFilter::Create(
            five, five, paintFilter.get(), cropRect));
        this->addFilter("xfermode", SkXfermodeImageFilter::Make(
            SkXfermode::Make(SkXfermode::kSrc_Mode), input, input, cropRect).release());
    }
    int count() const { return fFilters.count(); }
    SkImageFilter* getFilter(int index) const { return fFilters[index].fFilter.get(); }
    const char* getName(int index) const { return fFilters[index].fName; }
private:
    struct Filter {
        Filter() : fName(nullptr), fFilter(nullptr) {}
        Filter(const char* name, SkImageFilter* filter) : fName(name), fFilter(filter) {}
        const char*                 fName;
        sk_sp<SkImageFilter>        fFilter;
    };
    void addFilter(const char* name, SkImageFilter* filter) {
        fFilters.push_back(Filter(name, filter));
    }

    SkTArray<Filter> fFilters;
};

}

SkFlattenable* MatrixTestImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    skiatest::Reporter* reporter = (skiatest::Reporter*)buffer.readFunctionPtr();
    SkMatrix matrix;
    buffer.readMatrix(&matrix);
    return new MatrixTestImageFilter(reporter, matrix);
}

#ifndef SK_IGNORE_TO_STRING
void MatrixTestImageFilter::toString(SkString* str) const {
    str->appendf("MatrixTestImageFilter: (");
    str->append(")");
}
#endif

static sk_sp<SkImage> make_small_image() {
    auto surface(SkSurface::MakeRasterN32Premul(kBitmapSize, kBitmapSize));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(0x00000000);
    SkPaint darkPaint;
    darkPaint.setColor(0xFF804020);
    SkPaint lightPaint;
    lightPaint.setColor(0xFF244484);
    const int i = kBitmapSize / 4;
    for (int y = 0; y < kBitmapSize; y += i) {
        for (int x = 0; x < kBitmapSize; x += i) {
            canvas->save();
            canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas->drawRect(SkRect::MakeXYWH(0, 0,
                                             SkIntToScalar(i),
                                             SkIntToScalar(i)), darkPaint);
            canvas->drawRect(SkRect::MakeXYWH(SkIntToScalar(i),
                                             0,
                                             SkIntToScalar(i),
                                             SkIntToScalar(i)), lightPaint);
            canvas->drawRect(SkRect::MakeXYWH(0,
                                             SkIntToScalar(i),
                                             SkIntToScalar(i),
                                             SkIntToScalar(i)), lightPaint);
            canvas->drawRect(SkRect::MakeXYWH(SkIntToScalar(i),
                                             SkIntToScalar(i),
                                             SkIntToScalar(i),
                                             SkIntToScalar(i)), darkPaint);
            canvas->restore();
        }
    }

    return surface->makeImageSnapshot();
}

static SkImageFilter* make_scale(float amount, SkImageFilter* input = nullptr) {
    SkScalar s = amount;
    SkScalar matrix[20] = { s, 0, 0, 0, 0,
                            0, s, 0, 0, 0,
                            0, 0, s, 0, 0,
                            0, 0, 0, s, 0 };
    auto filter(SkColorFilter::MakeMatrixFilterRowMajor255(matrix));
    return SkColorFilterImageFilter::Create(filter.get(), input);
}

static SkImageFilter* make_grayscale(SkImageFilter* input, const SkImageFilter::CropRect* cropRect) {
    SkScalar matrix[20];
    memset(matrix, 0, 20 * sizeof(SkScalar));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    auto filter(SkColorFilter::MakeMatrixFilterRowMajor255(matrix));
    return SkColorFilterImageFilter::Create(filter.get(), input, cropRect);
}

static SkImageFilter* make_blue(SkImageFilter* input, const SkImageFilter::CropRect* cropRect) {
    auto filter(SkColorFilter::MakeModeFilter(SK_ColorBLUE, SkXfermode::kSrcIn_Mode));
    return SkColorFilterImageFilter::Create(filter.get(), input, cropRect);
}

static sk_sp<SkSpecialSurface> create_empty_special_surface(GrContext* context,
                                                            SkImageFilter::Proxy* proxy,
                                                            int widthHeight) {
    if (context) {
        GrSurfaceDesc desc;
        desc.fConfig = kSkia8888_GrPixelConfig;
        desc.fFlags  = kRenderTarget_GrSurfaceFlag;
        desc.fWidth  = widthHeight;
        desc.fHeight = widthHeight;
        return SkSpecialSurface::MakeRenderTarget(proxy, context, desc);
    } else {
        const SkImageInfo info = SkImageInfo::MakeN32(widthHeight, widthHeight,
                                                      kOpaque_SkAlphaType);
        return SkSpecialSurface::MakeRaster(proxy, info);
    }
}

static sk_sp<SkSpecialImage> create_empty_special_image(GrContext* context,
                                                        SkImageFilter::Proxy* proxy,
                                                        int widthHeight) {
    sk_sp<SkSpecialSurface> surf(create_empty_special_surface(context, proxy, widthHeight));

    SkASSERT(surf);

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    return surf->makeImageSnapshot();
}


DEF_TEST(ImageFilter, reporter) {
    {
        // Check that two non-clipping color-matrice-filters concatenate into a single filter.
        SkAutoTUnref<SkImageFilter> halfBrightness(make_scale(0.5f));
        SkAutoTUnref<SkImageFilter> quarterBrightness(make_scale(0.5f, halfBrightness));
        REPORTER_ASSERT(reporter, nullptr == quarterBrightness->getInput(0));
        SkColorFilter* cf;
        REPORTER_ASSERT(reporter, quarterBrightness->asColorFilter(&cf));
        REPORTER_ASSERT(reporter, cf->asColorMatrix(nullptr));
        cf->unref();
    }

    {
        // Check that a clipping color-matrice-filter followed by a color-matrice-filters
        // concatenates into a single filter, but not a matrixfilter (due to clamping).
        SkAutoTUnref<SkImageFilter> doubleBrightness(make_scale(2.0f));
        SkAutoTUnref<SkImageFilter> halfBrightness(make_scale(0.5f, doubleBrightness));
        REPORTER_ASSERT(reporter, nullptr == halfBrightness->getInput(0));
        SkColorFilter* cf;
        REPORTER_ASSERT(reporter, halfBrightness->asColorFilter(&cf));
        REPORTER_ASSERT(reporter, !cf->asColorMatrix(nullptr));
        cf->unref();
    }

    {
        // Check that a color filter image filter without a crop rect can be
        // expressed as a color filter.
        SkAutoTUnref<SkImageFilter> gray(make_grayscale(nullptr, nullptr));
        REPORTER_ASSERT(reporter, true == gray->asColorFilter(nullptr));
    }

    {
        // Check that a colorfilterimage filter without a crop rect but with an input
        // that is another colorfilterimage can be expressed as a colorfilter (composed).
        SkAutoTUnref<SkImageFilter> mode(make_blue(nullptr, nullptr));
        SkAutoTUnref<SkImageFilter> gray(make_grayscale(mode, nullptr));
        REPORTER_ASSERT(reporter, true == gray->asColorFilter(nullptr));
    }

    {
        // Test that if we exceed the limit of what ComposeColorFilter can combine, we still
        // can build the DAG and won't assert if we call asColorFilter.
        SkAutoTUnref<SkImageFilter> filter(make_blue(nullptr, nullptr));
        const int kWayTooManyForComposeColorFilter = 100;
        for (int i = 0; i < kWayTooManyForComposeColorFilter; ++i) {
            filter.reset(make_blue(filter, nullptr));
            // the first few of these will succeed, but after we hit the internal limit,
            // it will then return false.
            (void)filter->asColorFilter(nullptr);
        }
    }

    {
        // Check that a color filter image filter with a crop rect cannot
        // be expressed as a color filter.
        SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(0, 0, 100, 100));
        SkAutoTUnref<SkImageFilter> grayWithCrop(make_grayscale(nullptr, &cropRect));
        REPORTER_ASSERT(reporter, false == grayWithCrop->asColorFilter(nullptr));
    }

    {
        // Check that two non-commutative matrices are concatenated in
        // the correct order.
        SkScalar blueToRedMatrix[20] = { 0 };
        blueToRedMatrix[2] = blueToRedMatrix[18] = SK_Scalar1;
        SkScalar redToGreenMatrix[20] = { 0 };
        redToGreenMatrix[5] = redToGreenMatrix[18] = SK_Scalar1;
        auto blueToRed(SkColorFilter::MakeMatrixFilterRowMajor255(blueToRedMatrix));
        SkAutoTUnref<SkImageFilter> filter1(SkColorFilterImageFilter::Create(blueToRed.get()));
        auto redToGreen(SkColorFilter::MakeMatrixFilterRowMajor255(redToGreenMatrix));
        SkAutoTUnref<SkImageFilter> filter2(SkColorFilterImageFilter::Create(redToGreen.get(), filter1.get()));

        SkBitmap result;
        result.allocN32Pixels(kBitmapSize, kBitmapSize);

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setImageFilter(filter2.get());
        SkCanvas canvas(result);
        canvas.clear(0x0);
        SkRect rect = SkRect::Make(SkIRect::MakeWH(kBitmapSize, kBitmapSize));
        canvas.drawRect(rect, paint);
        uint32_t pixel = *result.getAddr32(0, 0);
        // The result here should be green, since we have effectively shifted blue to green.
        REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
    }

    {
        // Tests pass by not asserting
        sk_sp<SkImage> image(make_small_image());
        SkBitmap result;
        result.allocN32Pixels(kBitmapSize, kBitmapSize);

        {
            // This tests for :
            // 1 ) location at (0,0,1)
            SkPoint3 location = SkPoint3::Make(0, 0, SK_Scalar1);
            // 2 ) location and target at same value
            SkPoint3 target = SkPoint3::Make(location.fX, location.fY, location.fZ);
            // 3 ) large negative specular exponent value
            SkScalar specularExponent = -1000;

            SkAutoTUnref<SkImageFilter> bmSrc(SkImageSource::Create(image.get()));
            SkPaint paint;
            paint.setImageFilter(SkLightingImageFilter::CreateSpotLitSpecular(
                    location, target, specularExponent, 180,
                    0xFFFFFFFF, SK_Scalar1, SK_Scalar1, SK_Scalar1,
                    bmSrc))->unref();
            SkCanvas canvas(result);
            SkRect r = SkRect::MakeWH(SkIntToScalar(kBitmapSize),
                                      SkIntToScalar(kBitmapSize));
            canvas.drawRect(r, paint);
        }
    }
}

static void test_crop_rects(SkImageFilter::Proxy* proxy,
                            skiatest::Reporter* reporter,
                            GrContext* context) {
    // Check that all filters offset to their absolute crop rect,
    // unaffected by the input crop rect.
    // Tests pass by not asserting.
    sk_sp<SkSpecialImage> srcImg(create_empty_special_image(context, proxy, 100));
    SkASSERT(srcImg);

    SkImageFilter::CropRect inputCropRect(SkRect::MakeXYWH(8, 13, 80, 80));
    SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(20, 30, 60, 60));
    SkAutoTUnref<SkImageFilter> input(make_grayscale(nullptr, &inputCropRect));

    FilterList filters(input.get(), &cropRect);

    for (int i = 0; i < filters.count(); ++i) {
        SkImageFilter* filter = filters.getFilter(i);
        SkIPoint offset;
        SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeWH(100, 100), nullptr);
        sk_sp<SkSpecialImage> resultImg(filter->filterImage(srcImg.get(), ctx, &offset));
        REPORTER_ASSERT_MESSAGE(reporter, resultImg, filters.getName(i));
        REPORTER_ASSERT_MESSAGE(reporter, offset.fX == 20 && offset.fY == 30, filters.getName(i));
    }
}

static void test_negative_blur_sigma(SkImageFilter::Proxy* proxy,
                                     skiatest::Reporter* reporter,
                                     GrContext* context) {
    // Check that SkBlurImageFilter will accept a negative sigma, either in
    // the given arguments or after CTM application.
    const int width = 32, height = 32;
    const SkScalar five = SkIntToScalar(5);

    SkAutoTUnref<SkImageFilter> positiveFilter(SkBlurImageFilter::Create(five, five));
    SkAutoTUnref<SkImageFilter> negativeFilter(SkBlurImageFilter::Create(-five, five));

    SkBitmap gradient = make_gradient_circle(width, height);
    sk_sp<SkSpecialImage> imgSrc(SkSpecialImage::MakeFromRaster(proxy,
                                                                SkIRect::MakeWH(width, height),
                                                                gradient));

    SkIPoint offset;
    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeWH(32, 32), nullptr);

    sk_sp<SkSpecialImage> positiveResult1(positiveFilter->filterImage(imgSrc.get(), ctx, &offset));
    REPORTER_ASSERT(reporter, positiveResult1);

    sk_sp<SkSpecialImage> negativeResult1(negativeFilter->filterImage(imgSrc.get(), ctx, &offset));
    REPORTER_ASSERT(reporter, negativeResult1);

    SkMatrix negativeScale;
    negativeScale.setScale(-SK_Scalar1, SK_Scalar1);
    SkImageFilter::Context negativeCTX(negativeScale, SkIRect::MakeWH(32, 32), nullptr);

    sk_sp<SkSpecialImage> negativeResult2(positiveFilter->filterImage(imgSrc.get(),
                                                                      negativeCTX,
                                                                      &offset));
    REPORTER_ASSERT(reporter, negativeResult2);

    sk_sp<SkSpecialImage> positiveResult2(negativeFilter->filterImage(imgSrc.get(),
                                                                      negativeCTX,
                                                                      &offset));
    REPORTER_ASSERT(reporter, positiveResult2);


    SkBitmap positiveResultBM1, positiveResultBM2;
    SkBitmap negativeResultBM1, negativeResultBM2;

    TestingSpecialImageAccess::GetROPixels(positiveResult1.get(), &positiveResultBM1);
    TestingSpecialImageAccess::GetROPixels(positiveResult2.get(), &positiveResultBM2);
    TestingSpecialImageAccess::GetROPixels(negativeResult1.get(), &negativeResultBM1);
    TestingSpecialImageAccess::GetROPixels(negativeResult2.get(), &negativeResultBM2);

    SkAutoLockPixels lockP1(positiveResultBM1);
    SkAutoLockPixels lockP2(positiveResultBM2);
    SkAutoLockPixels lockN1(negativeResultBM1);
    SkAutoLockPixels lockN2(negativeResultBM2);
    for (int y = 0; y < height; y++) {
        int diffs = memcmp(positiveResultBM1.getAddr32(0, y),
                           negativeResultBM1.getAddr32(0, y),
                           positiveResultBM1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
        diffs = memcmp(positiveResultBM1.getAddr32(0, y),
                       negativeResultBM2.getAddr32(0, y),
                       positiveResultBM1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
        diffs = memcmp(positiveResultBM1.getAddr32(0, y),
                       positiveResultBM2.getAddr32(0, y),
                       positiveResultBM1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
    }
}

typedef void (*PFTest)(SkImageFilter::Proxy* proxy,
                       skiatest::Reporter* reporter,
                       GrContext* context);

static void run_raster_test(skiatest::Reporter* reporter,
                            int widthHeight,
                            PFTest test) {
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    const SkImageInfo info = SkImageInfo::MakeN32Premul(widthHeight, widthHeight);

    SkAutoTUnref<SkBaseDevice> device(SkBitmapDevice::Create(info, props));
    SkImageFilter::DeviceProxy proxy(device);

    (*test)(&proxy, reporter, nullptr);
}

#if SK_SUPPORT_GPU
static void run_gpu_test(skiatest::Reporter* reporter,
                         GrContext* context,
                         int widthHeight,
                         PFTest test) {
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkBudgeted::kNo,
                                                         SkImageInfo::MakeN32Premul(widthHeight,
                                                                                    widthHeight),
                                                         0,
                                                         &props,
                                                         SkGpuDevice::kUninit_InitContents));
    SkImageFilter::DeviceProxy proxy(device);

    (*test)(&proxy, reporter, context);
}
#endif

DEF_TEST(TestNegativeBlurSigma, reporter) {
    run_raster_test(reporter, 100, test_negative_blur_sigma);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_NATIVE_CONTEXT(TestNegativeBlurSigma_Gpu, reporter, context) {
    run_gpu_test(reporter, context, 100, test_negative_blur_sigma);
}
#endif

static void test_zero_blur_sigma(SkImageFilter::Proxy* proxy,
                                 skiatest::Reporter* reporter,
                                 GrContext* context) {
    // Check that SkBlurImageFilter with a zero sigma and a non-zero srcOffset works correctly.
    SkImageFilter::CropRect cropRect(SkRect::Make(SkIRect::MakeXYWH(5, 0, 5, 10)));
    SkAutoTUnref<SkImageFilter> input(SkOffsetImageFilter::Create(0, 0, nullptr, &cropRect));
    SkAutoTUnref<SkImageFilter> filter(SkBlurImageFilter::Create(0, 0, input, &cropRect));

    sk_sp<SkSpecialSurface> surf(create_empty_special_surface(context, proxy, 10));
    surf->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkSpecialImage> image(surf->makeImageSnapshot());

    SkIPoint offset;
    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeWH(32, 32), nullptr);

    sk_sp<SkSpecialImage> result(filter->filterImage(image.get(), ctx, &offset));
    REPORTER_ASSERT(reporter, offset.fX == 5 && offset.fY == 0);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, result->width() == 5 && result->height() == 10);

    SkBitmap resultBM;

    TestingSpecialImageAccess::GetROPixels(result.get(), &resultBM);

    SkAutoLockPixels lock(resultBM);
    for (int y = 0; y < resultBM.height(); y++) {
        for (int x = 0; x < resultBM.width(); x++) {
            bool diff = *resultBM.getAddr32(x, y) != SK_ColorGREEN;
            REPORTER_ASSERT(reporter, !diff);
            if (diff) {
                break;
            }
        }
    }
}

DEF_TEST(TestZeroBlurSigma, reporter) {
    run_raster_test(reporter, 100, test_zero_blur_sigma);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_NATIVE_CONTEXT(TestZeroBlurSigma_Gpu, reporter, context) {
    run_gpu_test(reporter, context, 100, test_zero_blur_sigma);
}
#endif

DEF_TEST(ImageFilterDrawTiled, reporter) {
    // Check that all filters when drawn tiled (with subsequent clip rects) exactly
    // match the same filters drawn with a single full-canvas bitmap draw.
    // Tests pass by not asserting.

    FilterList filters;

    SkBitmap untiledResult, tiledResult;
    const int width = 64, height = 64;
    untiledResult.allocN32Pixels(width, height);
    tiledResult.allocN32Pixels(width, height);
    SkCanvas tiledCanvas(tiledResult);
    SkCanvas untiledCanvas(untiledResult);
    int tileSize = 8;

    for (int scale = 1; scale <= 2; ++scale) {
        for (int i = 0; i < filters.count(); ++i) {
            tiledCanvas.clear(0);
            untiledCanvas.clear(0);
            SkPaint paint;
            paint.setImageFilter(filters.getFilter(i));
            paint.setTextSize(SkIntToScalar(height));
            paint.setColor(SK_ColorWHITE);
            SkString str;
            const char* text = "ABC";
            SkScalar ypos = SkIntToScalar(height);
            untiledCanvas.save();
            untiledCanvas.scale(SkIntToScalar(scale), SkIntToScalar(scale));
            untiledCanvas.drawText(text, strlen(text), 0, ypos, paint);
            untiledCanvas.restore();
            for (int y = 0; y < height; y += tileSize) {
                for (int x = 0; x < width; x += tileSize) {
                    tiledCanvas.save();
                    tiledCanvas.clipRect(SkRect::Make(SkIRect::MakeXYWH(x, y, tileSize, tileSize)));
                    tiledCanvas.scale(SkIntToScalar(scale), SkIntToScalar(scale));
                    tiledCanvas.drawText(text, strlen(text), 0, ypos, paint);
                    tiledCanvas.restore();
                }
            }
            untiledCanvas.flush();
            tiledCanvas.flush();
            for (int y = 0; y < height; y++) {
                int diffs = memcmp(untiledResult.getAddr32(0, y), tiledResult.getAddr32(0, y), untiledResult.rowBytes());
                REPORTER_ASSERT_MESSAGE(reporter, !diffs, filters.getName(i));
                if (diffs) {
                    break;
                }
            }
        }
    }
}

static void draw_saveLayer_picture(int width, int height, int tileSize,
                                   SkBBHFactory* factory, SkBitmap* result) {

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(50), 0);

    auto cf(SkColorFilter::MakeModeFilter(SK_ColorWHITE, SkXfermode::kSrc_Mode));
    SkAutoTUnref<SkImageFilter> cfif(SkColorFilterImageFilter::Create(cf.get()));
    SkAutoTUnref<SkImageFilter> imageFilter(SkImageFilter::CreateMatrixFilter(matrix, kNone_SkFilterQuality, cfif.get()));

    SkPaint paint;
    paint.setImageFilter(imageFilter.get());
    SkPictureRecorder recorder;
    SkRect bounds = SkRect::Make(SkIRect::MakeXYWH(0, 0, 50, 50));
    SkCanvas* recordingCanvas = recorder.beginRecording(SkIntToScalar(width),
                                                        SkIntToScalar(height),
                                                        factory, 0);
    recordingCanvas->translate(-55, 0);
    recordingCanvas->saveLayer(&bounds, &paint);
    recordingCanvas->restore();
    sk_sp<SkPicture> picture1(recorder.finishRecordingAsPicture());

    result->allocN32Pixels(width, height);
    SkCanvas canvas(*result);
    canvas.clear(0);
    canvas.clipRect(SkRect::Make(SkIRect::MakeWH(tileSize, tileSize)));
    canvas.drawPicture(picture1.get());
}

DEF_TEST(ImageFilterDrawMatrixBBH, reporter) {
    // Check that matrix filter when drawn tiled with BBH exactly
    // matches the same thing drawn without BBH.
    // Tests pass by not asserting.

    const int width = 200, height = 200;
    const int tileSize = 100;
    SkBitmap result1, result2;
    SkRTreeFactory factory;

    draw_saveLayer_picture(width, height, tileSize, &factory, &result1);
    draw_saveLayer_picture(width, height, tileSize, nullptr, &result2);

    for (int y = 0; y < height; y++) {
        int diffs = memcmp(result1.getAddr32(0, y), result2.getAddr32(0, y), result1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
    }
}

static SkImageFilter* makeBlur(SkImageFilter* input = nullptr) {
    return SkBlurImageFilter::Create(SK_Scalar1, SK_Scalar1, input);
}

static SkImageFilter* makeDropShadow(SkImageFilter* input = nullptr) {
    return SkDropShadowImageFilter::Create(
        SkIntToScalar(100), SkIntToScalar(100),
        SkIntToScalar(10), SkIntToScalar(10),
        SK_ColorBLUE, SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
        input, nullptr);
}

DEF_TEST(ImageFilterBlurThenShadowBounds, reporter) {
    SkAutoTUnref<SkImageFilter> filter1(makeBlur());
    SkAutoTUnref<SkImageFilter> filter2(makeDropShadow(filter1.get()));

    SkIRect bounds = SkIRect::MakeXYWH(0, 0, 100, 100);
    SkIRect expectedBounds = SkIRect::MakeXYWH(-133, -133, 236, 236);
    bounds = filter2->filterBounds(bounds, SkMatrix::I());

    REPORTER_ASSERT(reporter, bounds == expectedBounds);
}

DEF_TEST(ImageFilterShadowThenBlurBounds, reporter) {
    SkAutoTUnref<SkImageFilter> filter1(makeDropShadow());
    SkAutoTUnref<SkImageFilter> filter2(makeBlur(filter1.get()));

    SkIRect bounds = SkIRect::MakeXYWH(0, 0, 100, 100);
    SkIRect expectedBounds = SkIRect::MakeXYWH(-133, -133, 236, 236);
    bounds = filter2->filterBounds(bounds, SkMatrix::I());

    REPORTER_ASSERT(reporter, bounds == expectedBounds);
}

DEF_TEST(ImageFilterDilateThenBlurBounds, reporter) {
    SkAutoTUnref<SkImageFilter> filter1(SkDilateImageFilter::Create(2, 2));
    SkAutoTUnref<SkImageFilter> filter2(makeDropShadow(filter1.get()));

    SkIRect bounds = SkIRect::MakeXYWH(0, 0, 100, 100);
    SkIRect expectedBounds = SkIRect::MakeXYWH(-132, -132, 234, 234);
    bounds = filter2->filterBounds(bounds, SkMatrix::I());

    REPORTER_ASSERT(reporter, bounds == expectedBounds);
}

DEF_TEST(ImageFilterComposedBlurFastBounds, reporter) {
    sk_sp<SkImageFilter> filter1(makeBlur());
    sk_sp<SkImageFilter> filter2(makeBlur());
    sk_sp<SkImageFilter> composedFilter(SkComposeImageFilter::Make(std::move(filter1),
                                                                   std::move(filter2)));

    SkRect boundsSrc = SkRect::MakeWH(SkIntToScalar(100), SkIntToScalar(100));
    SkRect expectedBounds = SkRect::MakeXYWH(
        SkIntToScalar(-6), SkIntToScalar(-6), SkIntToScalar(112), SkIntToScalar(112));
    SkRect boundsDst = composedFilter->computeFastBounds(boundsSrc);

    REPORTER_ASSERT(reporter, boundsDst == expectedBounds);
}

DEF_TEST(ImageFilterUnionBounds, reporter) {
    SkAutoTUnref<SkImageFilter> offset(SkOffsetImageFilter::Create(50, 0));
    // Regardless of which order they appear in, the image filter bounds should
    // be combined correctly.
    {
        sk_sp<SkImageFilter> composite(SkXfermodeImageFilter::Make(nullptr, offset.get()));
        SkRect bounds = SkRect::MakeWH(100, 100);
        // Intentionally aliasing here, as that's what the real callers do.
        bounds = composite->computeFastBounds(bounds);
        REPORTER_ASSERT(reporter, bounds == SkRect::MakeWH(150, 100));
    }
    {
        sk_sp<SkImageFilter> composite(SkXfermodeImageFilter::Make(nullptr, nullptr,
                                                                   offset.get(), nullptr));
        SkRect bounds = SkRect::MakeWH(100, 100);
        // Intentionally aliasing here, as that's what the real callers do.
        bounds = composite->computeFastBounds(bounds);
        REPORTER_ASSERT(reporter, bounds == SkRect::MakeWH(150, 100));
    }
}

static void test_imagefilter_merge_result_size(SkImageFilter::Proxy* proxy,
                                               skiatest::Reporter* reporter,
                                               GrContext* context) {
    SkBitmap greenBM;
    greenBM.allocN32Pixels(20, 20);
    greenBM.eraseColor(SK_ColorGREEN);
    sk_sp<SkImage> greenImage(SkImage::MakeFromBitmap(greenBM));
    SkAutoTUnref<SkImageFilter> source(SkImageSource::Create(greenImage.get()));
    SkAutoTUnref<SkImageFilter> merge(SkMergeImageFilter::Create(source.get(), source.get()));

    sk_sp<SkSpecialImage> srcImg(create_empty_special_image(context, proxy, 1));

    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeXYWH(0, 0, 100, 100), nullptr);
    SkIPoint offset;

    sk_sp<SkSpecialImage> resultImg(merge->filterImage(srcImg.get(), ctx, &offset));
    REPORTER_ASSERT(reporter, resultImg);

    REPORTER_ASSERT(reporter, resultImg->width() == 20 && resultImg->height() == 20);
}

DEF_TEST(ImageFilterMergeResultSize, reporter) {
    run_raster_test(reporter, 100, test_imagefilter_merge_result_size);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_NATIVE_CONTEXT(ImageFilterMergeResultSize_Gpu, reporter, context) {
    run_gpu_test(reporter, context, 100, test_imagefilter_merge_result_size);
}
#endif

static void draw_blurred_rect(SkCanvas* canvas) {
    SkAutoTUnref<SkImageFilter> filter(SkBlurImageFilter::Create(SkIntToScalar(8), 0));
    SkPaint filterPaint;
    filterPaint.setColor(SK_ColorWHITE);
    filterPaint.setImageFilter(filter);
    canvas->saveLayer(nullptr, &filterPaint);
    SkPaint whitePaint;
    whitePaint.setColor(SK_ColorWHITE);
    canvas->drawRect(SkRect::Make(SkIRect::MakeWH(4, 4)), whitePaint);
    canvas->restore();
}

static void draw_picture_clipped(SkCanvas* canvas, const SkRect& clipRect, const SkPicture* picture) {
    canvas->save();
    canvas->clipRect(clipRect);
    canvas->drawPicture(picture);
    canvas->restore();
}

DEF_TEST(ImageFilterDrawTiledBlurRTree, reporter) {
    // Check that the blur filter when recorded with RTree acceleration,
    // and drawn tiled (with subsequent clip rects) exactly
    // matches the same filter drawn with without RTree acceleration.
    // This tests that the "bleed" from the blur into the otherwise-blank
    // tiles is correctly rendered.
    // Tests pass by not asserting.

    int width = 16, height = 8;
    SkBitmap result1, result2;
    result1.allocN32Pixels(width, height);
    result2.allocN32Pixels(width, height);
    SkCanvas canvas1(result1);
    SkCanvas canvas2(result2);
    int tileSize = 8;

    canvas1.clear(0);
    canvas2.clear(0);

    SkRTreeFactory factory;

    SkPictureRecorder recorder1, recorder2;
    // The only difference between these two pictures is that one has RTree aceleration.
    SkCanvas* recordingCanvas1 = recorder1.beginRecording(SkIntToScalar(width),
                                                          SkIntToScalar(height),
                                                          nullptr, 0);
    SkCanvas* recordingCanvas2 = recorder2.beginRecording(SkIntToScalar(width),
                                                          SkIntToScalar(height),
                                                          &factory, 0);
    draw_blurred_rect(recordingCanvas1);
    draw_blurred_rect(recordingCanvas2);
    sk_sp<SkPicture> picture1(recorder1.finishRecordingAsPicture());
    sk_sp<SkPicture> picture2(recorder2.finishRecordingAsPicture());
    for (int y = 0; y < height; y += tileSize) {
        for (int x = 0; x < width; x += tileSize) {
            SkRect tileRect = SkRect::Make(SkIRect::MakeXYWH(x, y, tileSize, tileSize));
            draw_picture_clipped(&canvas1, tileRect, picture1.get());
            draw_picture_clipped(&canvas2, tileRect, picture2.get());
        }
    }
    for (int y = 0; y < height; y++) {
        int diffs = memcmp(result1.getAddr32(0, y), result2.getAddr32(0, y), result1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
    }
}

DEF_TEST(ImageFilterMatrixConvolution, reporter) {
    // Check that a 1x3 filter does not cause a spurious assert.
    SkScalar kernel[3] = {
        SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
    };
    SkISize kernelSize = SkISize::Make(1, 3);
    SkScalar gain = SK_Scalar1, bias = 0;
    SkIPoint kernelOffset = SkIPoint::Make(0, 0);

    SkAutoTUnref<SkImageFilter> filter(
        SkMatrixConvolutionImageFilter::Create(
            kernelSize, kernel, gain, bias, kernelOffset,
            SkMatrixConvolutionImageFilter::kRepeat_TileMode, false));

    SkBitmap result;
    int width = 16, height = 16;
    result.allocN32Pixels(width, height);
    SkCanvas canvas(result);
    canvas.clear(0);

    SkPaint paint;
    paint.setImageFilter(filter);
    SkRect rect = SkRect::Make(SkIRect::MakeWH(width, height));
    canvas.drawRect(rect, paint);
}

DEF_TEST(ImageFilterMatrixConvolutionBorder, reporter) {
    // Check that a filter with borders outside the target bounds
    // does not crash.
    SkScalar kernel[3] = {
        0, 0, 0,
    };
    SkISize kernelSize = SkISize::Make(3, 1);
    SkScalar gain = SK_Scalar1, bias = 0;
    SkIPoint kernelOffset = SkIPoint::Make(2, 0);

    SkAutoTUnref<SkImageFilter> filter(
        SkMatrixConvolutionImageFilter::Create(
            kernelSize, kernel, gain, bias, kernelOffset,
            SkMatrixConvolutionImageFilter::kClamp_TileMode, true));

    SkBitmap result;

    int width = 10, height = 10;
    result.allocN32Pixels(width, height);
    SkCanvas canvas(result);
    canvas.clear(0);

    SkPaint filterPaint;
    filterPaint.setImageFilter(filter);
    SkRect bounds = SkRect::MakeWH(1, 10);
    SkRect rect = SkRect::Make(SkIRect::MakeWH(width, height));
    SkPaint rectPaint;
    canvas.saveLayer(&bounds, &filterPaint);
    canvas.drawRect(rect, rectPaint);
    canvas.restore();
}

DEF_TEST(ImageFilterCropRect, reporter) {
    run_raster_test(reporter, 100, test_crop_rects);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_NATIVE_CONTEXT(ImageFilterCropRect_Gpu, reporter, context) {
    run_gpu_test(reporter, context, 100, test_crop_rects);
}
#endif

DEF_TEST(ImageFilterMatrix, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkCanvas canvas(temp);
    canvas.scale(SkIntToScalar(2), SkIntToScalar(2));

    SkMatrix expectedMatrix = canvas.getTotalMatrix();

    SkRTreeFactory factory;
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(100, 100, &factory, 0);

    SkPaint paint;
    SkAutoTUnref<MatrixTestImageFilter> imageFilter(
        new MatrixTestImageFilter(reporter, expectedMatrix));
    paint.setImageFilter(imageFilter.get());
    recordingCanvas->saveLayer(nullptr, &paint);
    SkPaint solidPaint;
    solidPaint.setColor(0xFFFFFFFF);
    recordingCanvas->save();
    recordingCanvas->scale(SkIntToScalar(10), SkIntToScalar(10));
    recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(100, 100)), solidPaint);
    recordingCanvas->restore(); // scale
    recordingCanvas->restore(); // saveLayer

    canvas.drawPicture(recorder.finishRecordingAsPicture());
}

DEF_TEST(ImageFilterCrossProcessPictureImageFilter, reporter) {
    SkRTreeFactory factory;
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(1, 1, &factory, 0);

    // Create an SkPicture which simply draws a green 1x1 rectangle.
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(1, 1)), greenPaint);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    // Wrap that SkPicture in an SkPictureImageFilter.
    sk_sp<SkImageFilter> imageFilter(SkPictureImageFilter::Make(picture));

    // Check that SkPictureImageFilter successfully serializes its contained
    // SkPicture when not in cross-process mode.
    SkPaint paint;
    paint.setImageFilter(imageFilter);
    SkPictureRecorder outerRecorder;
    SkCanvas* outerCanvas = outerRecorder.beginRecording(1, 1, &factory, 0);
    SkPaint redPaintWithFilter;
    redPaintWithFilter.setColor(SK_ColorRED);
    redPaintWithFilter.setImageFilter(imageFilter);
    outerCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(1, 1)), redPaintWithFilter);
    sk_sp<SkPicture> outerPicture(outerRecorder.finishRecordingAsPicture());

    SkBitmap bitmap;
    bitmap.allocN32Pixels(1, 1);
    SkCanvas canvas(bitmap);

    // The result here should be green, since the filter replaces the primitive's red interior.
    canvas.clear(0x0);
    canvas.drawPicture(outerPicture);
    uint32_t pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    // Check that, for now, SkPictureImageFilter does not serialize or
    // deserialize its contained picture when the filter is serialized
    // cross-process. Do this by "laundering" it through SkValidatingReadBuffer.
    SkAutoTUnref<SkData> data(SkValidatingSerializeFlattenable(imageFilter.get()));
    SkAutoTUnref<SkFlattenable> flattenable(SkValidatingDeserializeFlattenable(
        data->data(), data->size(), SkImageFilter::GetFlattenableType()));
    SkImageFilter* unflattenedFilter = static_cast<SkImageFilter*>(flattenable.get());

    redPaintWithFilter.setImageFilter(unflattenedFilter);
    SkPictureRecorder crossProcessRecorder;
    SkCanvas* crossProcessCanvas = crossProcessRecorder.beginRecording(1, 1, &factory, 0);
    crossProcessCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(1, 1)), redPaintWithFilter);
    sk_sp<SkPicture> crossProcessPicture(crossProcessRecorder.finishRecordingAsPicture());

    canvas.clear(0x0);
    canvas.drawPicture(crossProcessPicture);
    pixel = *bitmap.getAddr32(0, 0);
    // If the security precautions are enabled, the result here should not be green, since the
    // filter draws nothing.
    REPORTER_ASSERT(reporter, SkPicture::PictureIOSecurityPrecautionsEnabled()
        ? pixel != SK_ColorGREEN : pixel == SK_ColorGREEN);
}

static void test_clipped_picture_imagefilter(SkImageFilter::Proxy* proxy,
                                             skiatest::Reporter* reporter,
                                             GrContext* context) {
    sk_sp<SkPicture> picture;

    {
        SkRTreeFactory factory;
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(1, 1, &factory, 0);

        // Create an SkPicture which simply draws a green 1x1 rectangle.
        SkPaint greenPaint;
        greenPaint.setColor(SK_ColorGREEN);
        recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(1, 1)), greenPaint);
        picture = recorder.finishRecordingAsPicture();
    }

    sk_sp<SkSpecialImage> srcImg(create_empty_special_image(context, proxy, 2));

    sk_sp<SkImageFilter> imageFilter(SkPictureImageFilter::Make(picture));

    SkIPoint offset;
    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeXYWH(1, 1, 1, 1), nullptr);

    sk_sp<SkSpecialImage> resultImage(imageFilter->filterImage(srcImg.get(), ctx, &offset));
    REPORTER_ASSERT(reporter, !resultImage);
}

DEF_TEST(ImageFilterClippedPictureImageFilter, reporter) {
    run_raster_test(reporter, 2, test_clipped_picture_imagefilter);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_NATIVE_CONTEXT(ImageFilterClippedPictureImageFilter_Gpu, reporter, context) {
    run_gpu_test(reporter, context, 2, test_clipped_picture_imagefilter);
}
#endif

DEF_TEST(ImageFilterEmptySaveLayer, reporter) {
    // Even when there's an empty saveLayer()/restore(), ensure that an image
    // filter or color filter which affects transparent black still draws.

    SkBitmap bitmap;
    bitmap.allocN32Pixels(10, 10);
    SkCanvas canvas(bitmap);

    SkRTreeFactory factory;
    SkPictureRecorder recorder;

    auto green(SkColorFilter::MakeModeFilter(SK_ColorGREEN, SkXfermode::kSrc_Mode));
    SkAutoTUnref<SkImageFilter> imageFilter(
        SkColorFilterImageFilter::Create(green.get()));
    SkPaint imageFilterPaint;
    imageFilterPaint.setImageFilter(imageFilter.get());
    SkPaint colorFilterPaint;
    colorFilterPaint.setColorFilter(green);

    SkRect bounds = SkRect::MakeWH(10, 10);

    SkCanvas* recordingCanvas = recorder.beginRecording(10, 10, &factory, 0);
    recordingCanvas->saveLayer(&bounds, &imageFilterPaint);
    recordingCanvas->restore();
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    canvas.clear(0);
    canvas.drawPicture(picture);
    uint32_t pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    recordingCanvas = recorder.beginRecording(10, 10, &factory, 0);
    recordingCanvas->saveLayer(nullptr, &imageFilterPaint);
    recordingCanvas->restore();
    sk_sp<SkPicture> picture2(recorder.finishRecordingAsPicture());

    canvas.clear(0);
    canvas.drawPicture(picture2);
    pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    recordingCanvas = recorder.beginRecording(10, 10, &factory, 0);
    recordingCanvas->saveLayer(&bounds, &colorFilterPaint);
    recordingCanvas->restore();
    sk_sp<SkPicture> picture3(recorder.finishRecordingAsPicture());

    canvas.clear(0);
    canvas.drawPicture(picture3);
    pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
}

static void test_huge_blur(SkCanvas* canvas, skiatest::Reporter* reporter) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    bitmap.eraseARGB(0, 0, 0, 0);

    // Check that a blur with an insane radius does not crash or assert.
    SkAutoTUnref<SkImageFilter> blur(SkBlurImageFilter::Create(SkIntToScalar(1<<30), SkIntToScalar(1<<30)));

    SkPaint paint;
    paint.setImageFilter(blur);
    canvas->drawBitmap(bitmap, 0, 0, &paint);
}

DEF_TEST(HugeBlurImageFilter, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkCanvas canvas(temp);
    test_huge_blur(&canvas, reporter);
}

DEF_TEST(MatrixConvolutionSanityTest, reporter) {
    SkScalar kernel[1] = { 0 };
    SkScalar gain = SK_Scalar1, bias = 0;
    SkIPoint kernelOffset = SkIPoint::Make(1, 1);

    // Check that an enormous (non-allocatable) kernel gives a nullptr filter.
    SkAutoTUnref<SkImageFilter> conv(SkMatrixConvolutionImageFilter::Create(
        SkISize::Make(1<<30, 1<<30),
        kernel,
        gain,
        bias,
        kernelOffset,
        SkMatrixConvolutionImageFilter::kRepeat_TileMode,
        false));

    REPORTER_ASSERT(reporter, nullptr == conv.get());

    // Check that a nullptr kernel gives a nullptr filter.
    conv.reset(SkMatrixConvolutionImageFilter::Create(
        SkISize::Make(1, 1),
        nullptr,
        gain,
        bias,
        kernelOffset,
        SkMatrixConvolutionImageFilter::kRepeat_TileMode,
        false));

    REPORTER_ASSERT(reporter, nullptr == conv.get());

    // Check that a kernel width < 1 gives a nullptr filter.
    conv.reset(SkMatrixConvolutionImageFilter::Create(
        SkISize::Make(0, 1),
        kernel,
        gain,
        bias,
        kernelOffset,
        SkMatrixConvolutionImageFilter::kRepeat_TileMode,
        false));

    REPORTER_ASSERT(reporter, nullptr == conv.get());

    // Check that kernel height < 1 gives a nullptr filter.
    conv.reset(SkMatrixConvolutionImageFilter::Create(
        SkISize::Make(1, -1),
        kernel,
        gain,
        bias,
        kernelOffset,
        SkMatrixConvolutionImageFilter::kRepeat_TileMode,
        false));

    REPORTER_ASSERT(reporter, nullptr == conv.get());
}

static void test_xfermode_cropped_input(SkCanvas* canvas, skiatest::Reporter* reporter) {
    canvas->clear(0);

    SkBitmap bitmap;
    bitmap.allocN32Pixels(1, 1);
    bitmap.eraseARGB(255, 255, 255, 255);

    auto green(SkColorFilter::MakeModeFilter(SK_ColorGREEN, SkXfermode::kSrcIn_Mode));
    SkAutoTUnref<SkImageFilter> greenFilter(SkColorFilterImageFilter::Create(green.get()));
    SkImageFilter::CropRect cropRect(SkRect::MakeEmpty());
    SkAutoTUnref<SkImageFilter> croppedOut(
        SkColorFilterImageFilter::Create(green.get(), nullptr, &cropRect));

    // Check that an xfermode image filter whose input has been cropped out still draws the other
    // input. Also check that drawing with both inputs cropped out doesn't cause a GPU warning.
    auto mode = SkXfermode::Make(SkXfermode::kSrcOver_Mode);
    auto xfermodeNoFg(SkXfermodeImageFilter::Make(mode, greenFilter, croppedOut, nullptr));
    auto xfermodeNoBg(SkXfermodeImageFilter::Make(mode, croppedOut, greenFilter, nullptr));
    auto xfermodeNoFgNoBg(SkXfermodeImageFilter::Make(mode, croppedOut, croppedOut, nullptr));

    SkPaint paint;
    paint.setImageFilter(xfermodeNoFg);
    canvas->drawBitmap(bitmap, 0, 0, &paint);   // drawSprite

    uint32_t pixel;
    SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType);
    canvas->readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    paint.setImageFilter(xfermodeNoBg);
    canvas->drawBitmap(bitmap, 0, 0, &paint);   // drawSprite
    canvas->readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    paint.setImageFilter(xfermodeNoFgNoBg);
    canvas->drawBitmap(bitmap, 0, 0, &paint);   // drawSprite
    canvas->readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
}

DEF_TEST(ImageFilterNestedSaveLayer, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(50, 50);
    SkCanvas canvas(temp);
    canvas.clear(0x0);

    SkBitmap bitmap;
    bitmap.allocN32Pixels(10, 10);
    bitmap.eraseColor(SK_ColorGREEN);

    SkMatrix matrix;
    matrix.setScale(SkIntToScalar(2), SkIntToScalar(2));
    matrix.postTranslate(SkIntToScalar(-20), SkIntToScalar(-20));
    SkAutoTUnref<SkImageFilter> matrixFilter(
        SkImageFilter::CreateMatrixFilter(matrix, kLow_SkFilterQuality));

    // Test that saveLayer() with a filter nested inside another saveLayer() applies the
    // correct offset to the filter matrix.
    SkRect bounds1 = SkRect::MakeXYWH(10, 10, 30, 30);
    canvas.saveLayer(&bounds1, nullptr);
    SkPaint filterPaint;
    filterPaint.setImageFilter(matrixFilter);
    SkRect bounds2 = SkRect::MakeXYWH(20, 20, 10, 10);
    canvas.saveLayer(&bounds2, &filterPaint);
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    canvas.drawRect(bounds2, greenPaint);
    canvas.restore();
    canvas.restore();
    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setColor(SK_ColorRED);

    SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType);
    uint32_t pixel;
    canvas.readPixels(info, &pixel, 4, 25, 25);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    // Test that drawSprite() with a filter nested inside a saveLayer() applies the
    // correct offset to the filter matrix.
    canvas.clear(0x0);
    canvas.readPixels(info, &pixel, 4, 25, 25);
    canvas.saveLayer(&bounds1, nullptr);
    canvas.drawBitmap(bitmap, 20, 20, &filterPaint);    // drawSprite
    canvas.restore();

    canvas.readPixels(info, &pixel, 4, 25, 25);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
}

DEF_TEST(XfermodeImageFilterCroppedInput, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkCanvas canvas(temp);
    test_xfermode_cropped_input(&canvas, reporter);
}

static void test_composed_imagefilter_offset(SkImageFilter::Proxy* proxy,
                                             skiatest::Reporter* reporter,
                                             GrContext* context) {
    sk_sp<SkSpecialImage> srcImg(create_empty_special_image(context, proxy, 100));

    SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(1, 0, 20, 20));
    sk_sp<SkImageFilter> offsetFilter(SkOffsetImageFilter::Create(0, 0, nullptr, &cropRect));
    sk_sp<SkImageFilter> blurFilter(SkBlurImageFilter::Create(SK_Scalar1, SK_Scalar1,
                                                              nullptr, &cropRect));
    sk_sp<SkImageFilter> composedFilter(SkComposeImageFilter::Make(std::move(blurFilter),
                                                                   std::move(offsetFilter)));
    SkIPoint offset;
    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeWH(100, 100), nullptr);

    sk_sp<SkSpecialImage> resultImg(composedFilter->filterImage(srcImg.get(), ctx, &offset));
    REPORTER_ASSERT(reporter, resultImg);
    REPORTER_ASSERT(reporter, offset.fX == 1 && offset.fY == 0);
}

DEF_TEST(ComposedImageFilterOffset, reporter) {
    run_raster_test(reporter, 100, test_composed_imagefilter_offset);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_NATIVE_CONTEXT(ComposedImageFilterOffset_Gpu, reporter, context) {
    run_gpu_test(reporter, context, 100, test_composed_imagefilter_offset);
}
#endif

static void test_composed_imagefilter_bounds(SkImageFilter::Proxy* proxy,
                                             skiatest::Reporter* reporter,
                                             GrContext* context) {
    // The bounds passed to the inner filter must be filtered by the outer
    // filter, so that the inner filter produces the pixels that the outer
    // filter requires as input. This matters if the outer filter moves pixels.
    // Here, accounting for the outer offset is necessary so that the green
    // pixels of the picture are not clipped.

    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(SkRect::MakeWH(200, 100));
    recordingCanvas->clipRect(SkRect::MakeXYWH(100, 0, 100, 100));
    recordingCanvas->clear(SK_ColorGREEN);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    sk_sp<SkImageFilter> pictureFilter(SkPictureImageFilter::Make(picture));
    SkImageFilter::CropRect cropRect(SkRect::MakeWH(100, 100));
    sk_sp<SkImageFilter> offsetFilter(SkOffsetImageFilter::Create(-100, 0, nullptr, &cropRect));
    sk_sp<SkImageFilter> composedFilter(SkComposeImageFilter::Make(std::move(offsetFilter),
                                                                   std::move(pictureFilter)));

    sk_sp<SkSpecialImage> sourceImage(create_empty_special_image(context, proxy, 100));
    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeWH(100, 100), nullptr);
    SkIPoint offset;
    sk_sp<SkSpecialImage> result(composedFilter->filterImage(sourceImage.get(), ctx, &offset));
    REPORTER_ASSERT(reporter, offset.isZero());
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, result->subset().size() == SkISize::Make(100, 100));

    SkBitmap resultBM;
    TestingSpecialImageAccess::GetROPixels(result.get(), &resultBM);
    SkAutoLockPixels lock(resultBM);
    REPORTER_ASSERT(reporter, resultBM.getColor(50, 50) == SK_ColorGREEN);
}

DEF_TEST(ComposedImageFilterBounds, reporter) {
    run_raster_test(reporter, 100, test_composed_imagefilter_bounds);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_NATIVE_CONTEXT(ComposedImageFilterBounds_Gpu, reporter, context) {
    run_gpu_test(reporter, context, 100, test_composed_imagefilter_bounds);
}
#endif

static void test_partial_crop_rect(SkImageFilter::Proxy* proxy,
                                   skiatest::Reporter* reporter,
                                   GrContext* context) {
    sk_sp<SkSpecialImage> srcImg(create_empty_special_image(context, proxy, 100));

    SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(100, 0, 20, 30),
        SkImageFilter::CropRect::kHasWidth_CropEdge | SkImageFilter::CropRect::kHasHeight_CropEdge);
    SkAutoTUnref<SkImageFilter> filter(make_grayscale(nullptr, &cropRect));
    SkIPoint offset;
    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeWH(100, 100), nullptr);

    sk_sp<SkSpecialImage> resultImg(filter->filterImage(srcImg.get(), ctx, &offset));
    REPORTER_ASSERT(reporter, resultImg);

    REPORTER_ASSERT(reporter, offset.fX == 0);
    REPORTER_ASSERT(reporter, offset.fY == 0);
    REPORTER_ASSERT(reporter, resultImg->width() == 20);
    REPORTER_ASSERT(reporter, resultImg->height() == 30);
}

DEF_TEST(PartialCropRect, reporter) {
    run_raster_test(reporter, 100, test_partial_crop_rect);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_NATIVE_CONTEXT(PartialCropRect_Gpu, reporter, context) {
    run_gpu_test(reporter, context, 100, test_partial_crop_rect);
}
#endif

DEF_TEST(ImageFilterCanComputeFastBounds, reporter) {

    SkPoint3 location = SkPoint3::Make(0, 0, SK_Scalar1);
    SkAutoTUnref<SkImageFilter> lighting(SkLightingImageFilter::CreatePointLitDiffuse(
          location, SK_ColorGREEN, 0, 0));
    REPORTER_ASSERT(reporter, !lighting->canComputeFastBounds());

    SkAutoTUnref<SkImageFilter> gray(make_grayscale(nullptr, nullptr));
    REPORTER_ASSERT(reporter, gray->canComputeFastBounds());
    {
        SkColorFilter* grayCF;
        REPORTER_ASSERT(reporter, gray->asAColorFilter(&grayCF));
        REPORTER_ASSERT(reporter, !grayCF->affectsTransparentBlack());
        grayCF->unref();
    }
    REPORTER_ASSERT(reporter, gray->canComputeFastBounds());

    SkAutoTUnref<SkImageFilter> grayBlur(SkBlurImageFilter::Create(SK_Scalar1, SK_Scalar1, gray.get()));
    REPORTER_ASSERT(reporter, grayBlur->canComputeFastBounds());

    SkScalar greenMatrix[20] = { 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 1,
                                 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 1 };
    auto greenCF(SkColorFilter::MakeMatrixFilterRowMajor255(greenMatrix));
    SkAutoTUnref<SkImageFilter> green(SkColorFilterImageFilter::Create(greenCF.get()));

    REPORTER_ASSERT(reporter, greenCF->affectsTransparentBlack());
    REPORTER_ASSERT(reporter, !green->canComputeFastBounds());

    SkAutoTUnref<SkImageFilter> greenBlur(SkBlurImageFilter::Create(SK_Scalar1, SK_Scalar1, green.get()));
    REPORTER_ASSERT(reporter, !greenBlur->canComputeFastBounds());

    uint8_t allOne[256], identity[256];
    for (int i = 0; i < 256; ++i) {
        identity[i] = i;
        allOne[i] = 255;
    }

    auto identityCF(SkTableColorFilter::MakeARGB(identity, identity, identity, allOne));
    SkAutoTUnref<SkImageFilter> identityFilter(SkColorFilterImageFilter::Create(identityCF.get()));
    REPORTER_ASSERT(reporter, !identityCF->affectsTransparentBlack());
    REPORTER_ASSERT(reporter, identityFilter->canComputeFastBounds());

    auto forceOpaqueCF(SkTableColorFilter::MakeARGB(allOne, identity, identity, identity));
    SkAutoTUnref<SkImageFilter> forceOpaque(SkColorFilterImageFilter::Create(forceOpaqueCF.get()));
    REPORTER_ASSERT(reporter, forceOpaqueCF->affectsTransparentBlack());
    REPORTER_ASSERT(reporter, !forceOpaque->canComputeFastBounds());
}

// Verify that SkImageSource survives serialization
DEF_TEST(ImageFilterImageSourceSerialization, reporter) {
    auto surface(SkSurface::MakeRasterN32Premul(10, 10));
    surface->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    SkAutoTUnref<SkImageFilter> filter(SkImageSource::Create(image.get()));

    SkAutoTUnref<SkData> data(SkValidatingSerializeFlattenable(filter));
    SkAutoTUnref<SkFlattenable> flattenable(SkValidatingDeserializeFlattenable(
        data->data(), data->size(), SkImageFilter::GetFlattenableType()));
    SkImageFilter* unflattenedFilter = static_cast<SkImageFilter*>(flattenable.get());
    REPORTER_ASSERT(reporter, unflattenedFilter);

    SkBitmap bm;
    bm.allocN32Pixels(10, 10);
    bm.eraseColor(SK_ColorBLUE);
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setImageFilter(unflattenedFilter);

    SkCanvas canvas(bm);
    canvas.drawRect(SkRect::MakeWH(10, 10), paint);
    REPORTER_ASSERT(reporter, *bm.getAddr32(0, 0) == SkPreMultiplyColor(SK_ColorGREEN));
}

static void test_large_blur_input(skiatest::Reporter* reporter, SkCanvas* canvas) {
    SkBitmap largeBmp;
    int largeW = 5000;
    int largeH = 5000;
#if SK_SUPPORT_GPU
    // If we're GPU-backed make the bitmap too large to be converted into a texture.
    if (GrContext* ctx = canvas->getGrContext()) {
        largeW = ctx->caps()->maxTextureSize() + 1;
    }
#endif

    largeBmp.allocN32Pixels(largeW, largeH);
    largeBmp.eraseColor(0);
    if (!largeBmp.getPixels()) {
        ERRORF(reporter, "Failed to allocate large bmp.");
        return;
    }

    sk_sp<SkImage> largeImage(SkImage::MakeFromBitmap(largeBmp));
    if (!largeImage) {
        ERRORF(reporter, "Failed to create large image.");
        return;
    }

    SkAutoTUnref<SkImageFilter> largeSource(SkImageSource::Create(largeImage.get()));
    if (!largeSource) {
        ERRORF(reporter, "Failed to create large SkImageSource.");
        return;
    }

    SkAutoTUnref<SkImageFilter> blur(SkBlurImageFilter::Create(10.f, 10.f, largeSource));
    if (!blur) {
        ERRORF(reporter, "Failed to create SkBlurImageFilter.");
        return;
    }

    SkPaint paint;
    paint.setImageFilter(blur);

    // This should not crash (http://crbug.com/570479).
    canvas->drawRect(SkRect::MakeIWH(largeW, largeH), paint);
}

DEF_TEST(BlurLargeImage, reporter) {
    auto surface(SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(100, 100)));
    test_large_blur_input(reporter, surface->getCanvas());
}

#if SK_SUPPORT_GPU

DEF_GPUTEST_FOR_NATIVE_CONTEXT(HugeBlurImageFilter_Gpu, reporter, context) {
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkBudgeted::kNo,
                                                         SkImageInfo::MakeN32Premul(100, 100),
                                                         0,
                                                         &props,
                                                         SkGpuDevice::kUninit_InitContents));
    SkCanvas canvas(device);

    test_huge_blur(&canvas, reporter);
}

DEF_GPUTEST_FOR_NATIVE_CONTEXT(XfermodeImageFilterCroppedInput_Gpu, reporter, context) {
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkBudgeted::kNo,
                                                         SkImageInfo::MakeN32Premul(1, 1),
                                                         0,
                                                         &props,
                                                         SkGpuDevice::kUninit_InitContents));
    SkCanvas canvas(device);

    test_xfermode_cropped_input(&canvas, reporter);
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(BlurLargeImage_Gpu, reporter, context) {
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kYes,
                                             SkImageInfo::MakeN32Premul(100, 100)));
    test_large_blur_input(reporter, surface->getCanvas());
}
#endif
