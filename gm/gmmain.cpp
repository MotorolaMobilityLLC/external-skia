/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Code for the "gm" (Golden Master) rendering comparison tool.
 *
 * If you make changes to this, re-run the self-tests at gm/tests/run.sh
 * to make sure they still pass... you may need to change the expected
 * results of the self-test.
 */

#include "gm.h"
#include "gm_error.h"
#include "gm_expectations.h"
#include "system_preferences.h"
#include "SkBitmap.h"
#include "SkBitmapChecksummer.h"
#include "SkColorPriv.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkDrawFilter.h"
#include "SkGPipe.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTileGridPicture.h"
#include "SamplePipeControllers.h"

#ifdef SK_BUILD_FOR_WIN
    // json includes xlocale which generates warning 4530 because we're compiling without
    // exceptions; see https://code.google.com/p/skia/issues/detail?id=1067
    #pragma warning(push)
    #pragma warning(disable : 4530)
#endif
#include "json/value.h"
#ifdef SK_BUILD_FOR_WIN
    #pragma warning(pop)
#endif

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrRenderTarget.h"
#include "SkGpuDevice.h"
typedef GrContextFactory::GLContextType GLContextType;
#define DEFAULT_CACHE_VALUE -1
static int gGpuCacheSizeBytes;
static int gGpuCacheSizeCount;
#else
class GrContextFactory;
class GrContext;
class GrRenderTarget;
typedef int GLContextType;
#endif

extern bool gSkSuppressFontCachePurgeSpew;

#ifdef SK_SUPPORT_PDF
    #include "SkPDFDevice.h"
    #include "SkPDFDocument.h"
#endif

// Until we resolve http://code.google.com/p/skia/issues/detail?id=455 ,
// stop writing out XPS-format image baselines in gm.
#undef SK_SUPPORT_XPS
#ifdef SK_SUPPORT_XPS
    #include "SkXPSDevice.h"
#endif

#ifdef SK_BUILD_FOR_MAC
    #include "SkCGUtils.h"
    #define CAN_IMAGE_PDF   1
#else
    #define CAN_IMAGE_PDF   0
#endif

using namespace skiagm;

struct FailRec {
    SkString    fName;
    bool        fIsPixelError;

    FailRec() : fIsPixelError(false) {}
    FailRec(const SkString& name) : fName(name), fIsPixelError(false) {}
};

class Iter {
public:
    Iter() {
        this->reset();
    }

    void reset() {
        fReg = GMRegistry::Head();
    }

    GM* next() {
        if (fReg) {
            GMRegistry::Factory fact = fReg->factory();
            fReg = fReg->next();
            return fact(0);
        }
        return NULL;
    }

    static int Count() {
        const GMRegistry* reg = GMRegistry::Head();
        int count = 0;
        while (reg) {
            count += 1;
            reg = reg->next();
        }
        return count;
    }

private:
    const GMRegistry* fReg;
};

enum Backend {
    kRaster_Backend,
    kGPU_Backend,
    kPDF_Backend,
    kXPS_Backend,
};

enum BbhType {
    kNone_BbhType,
    kRTree_BbhType,
    kTileGrid_BbhType,
};

enum ConfigFlags {
    kNone_ConfigFlag  = 0x0,
    /* Write GM images if a write path is provided. */
    kWrite_ConfigFlag = 0x1,
    /* Read reference GM images if a read path is provided. */
    kRead_ConfigFlag  = 0x2,
    kRW_ConfigFlag    = (kWrite_ConfigFlag | kRead_ConfigFlag),
};

struct ConfigData {
    SkBitmap::Config                fConfig;
    Backend                         fBackend;
    GLContextType                   fGLContextType; // GPU backend only
    int                             fSampleCnt;     // GPU backend only
    ConfigFlags                     fFlags;
    const char*                     fName;
    bool                            fRunByDefault;
};

class BWTextDrawFilter : public SkDrawFilter {
public:
    virtual bool filter(SkPaint*, Type) SK_OVERRIDE;
};
bool BWTextDrawFilter::filter(SkPaint* p, Type t) {
    if (kText_Type == t) {
        p->setAntiAlias(false);
    }
    return true;
}

struct PipeFlagComboData {
    const char* name;
    uint32_t flags;
};

static PipeFlagComboData gPipeWritingFlagCombos[] = {
    { "", 0 },
    { " cross-process", SkGPipeWriter::kCrossProcess_Flag },
    { " cross-process, shared address", SkGPipeWriter::kCrossProcess_Flag
        | SkGPipeWriter::kSharedAddressSpace_Flag }
};

class GMMain {
public:
    GMMain() {
        // Set default values of member variables, which tool_main()
        // may override.
        fUseFileHierarchy = false;
        fIgnorableErrorCombination.add(kMissingExpectations_ErrorType);
        fMismatchPath = NULL;
    }

    SkString make_name(const char shortName[], const char configName[]) {
        SkString name;
        if (0 == strlen(configName)) {
            name.append(shortName);
        } else if (fUseFileHierarchy) {
            name.appendf("%s%c%s", configName, SkPATH_SEPARATOR, shortName);
        } else {
            name.appendf("%s_%s", shortName, configName);
        }
        return name;
    }

    /* since PNG insists on unpremultiplying our alpha, we take no
       precision chances and force all pixels to be 100% opaque,
       otherwise on compare we may not get a perfect match.
    */
    static void force_all_opaque(const SkBitmap& bitmap) {
        SkBitmap::Config config = bitmap.config();
        switch (config) {
        case SkBitmap::kARGB_8888_Config:
            force_all_opaque_8888(bitmap);
            break;
        case SkBitmap::kRGB_565_Config:
            // nothing to do here; 565 bitmaps are inherently opaque
            break;
        default:
            gm_fprintf(stderr, "unsupported bitmap config %d\n", config);
            DEBUGFAIL_SEE_STDERR;
        }
    }

    static void force_all_opaque_8888(const SkBitmap& bitmap) {
        SkAutoLockPixels lock(bitmap);
        for (int y = 0; y < bitmap.height(); y++) {
            for (int x = 0; x < bitmap.width(); x++) {
                *bitmap.getAddr32(x, y) |= (SK_A32_MASK << SK_A32_SHIFT);
            }
        }
    }

    static bool write_bitmap(const SkString& path, const SkBitmap& bitmap) {
        // TODO(epoger): Now that we have removed force_all_opaque()
        // from this method, we should be able to get rid of the
        // transformation to 8888 format also.
        SkBitmap copy;
        bitmap.copyTo(&copy, SkBitmap::kARGB_8888_Config);
        return SkImageEncoder::EncodeFile(path.c_str(), copy,
                                          SkImageEncoder::kPNG_Type, 100);
    }

    /**
     * Records the errors encountered in fFailedTests, except for any error
     * types we want to ignore.
     */
    void RecordError(const ErrorCombination& errorCombination, const SkString& name,
                     const char renderModeDescriptor []) {
        // The common case: no error means nothing to record.
        if (errorCombination.isEmpty()) {
            return;
        }

        // If only certain error type(s) were reported, we know we can ignore them.
        if (errorCombination.minus(fIgnorableErrorCombination).isEmpty()) {
            return;
        }

        FailRec& rec = fFailedTests.push_back(make_name(name.c_str(), renderModeDescriptor));
        rec.fIsPixelError = errorCombination.includes(kImageMismatch_ErrorType);
    }

    // List contents of fFailedTests via SkDebug.
    void ListErrors() {
        for (int i = 0; i < fFailedTests.count(); ++i) {
            if (fFailedTests[i].fIsPixelError) {
                gm_fprintf(stderr, "\t\t%s pixel_error\n", fFailedTests[i].fName.c_str());
            } else {
                gm_fprintf(stderr, "\t\t%s\n", fFailedTests[i].fName.c_str());
            }
        }
    }

    static bool write_document(const SkString& path,
                               const SkDynamicMemoryWStream& document) {
        SkFILEWStream stream(path.c_str());
        SkAutoDataUnref data(document.copyToData());
        return stream.writeData(data.get());
    }

    /**
     * Prepare an SkBitmap to render a GM into.
     *
     * After you've rendered the GM into the SkBitmap, you must call
     * complete_bitmap()!
     *
     * @todo thudson 22 April 2011 - could refactor this to take in
     * a factory to generate the context, always call readPixels()
     * (logically a noop for rasters, if wasted time), and thus collapse the
     * GPU special case and also let this be used for SkPicture testing.
     */
    static void setup_bitmap(const ConfigData& gRec, SkISize& size,
                             SkBitmap* bitmap) {
        bitmap->setConfig(gRec.fConfig, size.width(), size.height());
        bitmap->allocPixels();
        bitmap->eraseColor(SK_ColorTRANSPARENT);
    }

    /**
     * Any finalization steps we need to perform on the SkBitmap after
     * we have rendered the GM into it.
     *
     * It's too bad that we are throwing away alpha channel data
     * we could otherwise be examining, but this had always been happening
     * before... it was buried within the compare() method at
     * https://code.google.com/p/skia/source/browse/trunk/gm/gmmain.cpp?r=7289#305 .
     *
     * Apparently we need this, at least for bitmaps that are either:
     * (a) destined to be written out as PNG files, or
     * (b) compared against bitmaps read in from PNG files
     * for the reasons described just above the force_all_opaque() method.
     *
     * Neglecting to do this led to the difficult-to-diagnose
     * http://code.google.com/p/skia/issues/detail?id=1079 ('gm generating
     * spurious pixel_error messages as of r7258')
     *
     * TODO(epoger): Come up with a better solution that allows us to
     * compare full pixel data, including alpha channel, while still being
     * robust in the face of transformations to/from PNG files.
     * Options include:
     *
     * 1. Continue to call force_all_opaque(), but ONLY for bitmaps that
     *    will be written to, or compared against, PNG files.
     *    PRO: Preserve/compare alpha channel info for the non-PNG cases
     *         (comparing different renderModes in-memory)
     *    CON: The bitmaps (and checksums) for these non-PNG cases would be
     *         different than those for the PNG-compared cases, and in the
     *         case of a failed renderMode comparison, how would we write the
     *         image to disk for examination?
     *
     * 2. Always compute image checksums from PNG format (either
     *    directly from the the bytes of a PNG file, or capturing the
     *    bytes we would have written to disk if we were writing the
     *    bitmap out as a PNG).
     *    PRO: I think this would allow us to never force opaque, and to
     *         the extent that alpha channel data can be preserved in a PNG
     *         file, we could observe it.
     *    CON: If we read a bitmap from disk, we need to take its checksum
     *         from the source PNG (we can't compute it from the bitmap we
     *         read out of the PNG, because we will have already premultiplied
     *         the alpha).
     *    CON: Seems wasteful to convert a bitmap to PNG format just to take
     *         its checksum. (Although we're wasting lots of effort already
     *         calling force_all_opaque().)
     *
     * 3. Make the alpha premultiply/unpremultiply routines 100% consistent,
     *    so we can transform images back and forth without fear of off-by-one
     *    errors.
     *    CON: Math is hard.
     *
     * 4. Perform a "close enough" comparison of bitmaps (+/- 1 bit in each
     *    channel), rather than demanding absolute equality.
     *    CON: Can't do this with checksums.
     */
    static void complete_bitmap(SkBitmap* bitmap) {
        force_all_opaque(*bitmap);
    }

    static void installFilter(SkCanvas* canvas);

    static void invokeGM(GM* gm, SkCanvas* canvas, bool isPDF, bool isDeferred) {
        SkAutoCanvasRestore acr(canvas, true);

        if (!isPDF) {
            canvas->concat(gm->getInitialTransform());
        }
        installFilter(canvas);
        gm->setCanvasIsDeferred(isDeferred);
        gm->draw(canvas);
        canvas->setDrawFilter(NULL);
    }

    static ErrorCombination generate_image(GM* gm, const ConfigData& gRec,
                                           GrContext* context,
                                           GrRenderTarget* rt,
                                           SkBitmap* bitmap,
                                           bool deferred) {
        SkISize size (gm->getISize());
        setup_bitmap(gRec, size, bitmap);

        SkAutoTUnref<SkCanvas> canvas;

        if (gRec.fBackend == kRaster_Backend) {
            SkAutoTUnref<SkDevice> device(new SkDevice(*bitmap));
            if (deferred) {
                canvas.reset(new SkDeferredCanvas(device));
            } else {
                canvas.reset(new SkCanvas(device));
            }
            invokeGM(gm, canvas, false, deferred);
            canvas->flush();
        }
#if SK_SUPPORT_GPU
        else {  // GPU
            if (NULL == context) {
                return ErrorCombination(kNoGpuContext_ErrorType);
            }
            SkAutoTUnref<SkDevice> device(new SkGpuDevice(context, rt));
            if (deferred) {
                canvas.reset(new SkDeferredCanvas(device));
            } else {
                canvas.reset(new SkCanvas(device));
            }
            invokeGM(gm, canvas, false, deferred);
            // the device is as large as the current rendertarget, so
            // we explicitly only readback the amount we expect (in
            // size) overwrite our previous allocation
            bitmap->setConfig(SkBitmap::kARGB_8888_Config, size.fWidth,
                              size.fHeight);
            canvas->readPixels(bitmap, 0, 0);
        }
#endif
        complete_bitmap(bitmap);
        return kEmpty_ErrorCombination;
    }

    static void generate_image_from_picture(GM* gm, const ConfigData& gRec,
                                            SkPicture* pict, SkBitmap* bitmap,
                                            SkScalar scale = SK_Scalar1) {
        SkISize size = gm->getISize();
        setup_bitmap(gRec, size, bitmap);
        SkCanvas canvas(*bitmap);
        installFilter(&canvas);
        canvas.scale(scale, scale);
        canvas.drawPicture(*pict);
        complete_bitmap(bitmap);
    }

    static void generate_pdf(GM* gm, SkDynamicMemoryWStream& pdf) {
#ifdef SK_SUPPORT_PDF
        SkMatrix initialTransform = gm->getInitialTransform();
        SkISize pageSize = gm->getISize();
        SkPDFDevice* dev = NULL;
        if (initialTransform.isIdentity()) {
            dev = new SkPDFDevice(pageSize, pageSize, initialTransform);
        } else {
            SkRect content = SkRect::MakeWH(SkIntToScalar(pageSize.width()),
                                            SkIntToScalar(pageSize.height()));
            initialTransform.mapRect(&content);
            content.intersect(0, 0, SkIntToScalar(pageSize.width()),
                              SkIntToScalar(pageSize.height()));
            SkISize contentSize =
                SkISize::Make(SkScalarRoundToInt(content.width()),
                              SkScalarRoundToInt(content.height()));
            dev = new SkPDFDevice(pageSize, contentSize, initialTransform);
        }
        SkAutoUnref aur(dev);

        SkCanvas c(dev);
        invokeGM(gm, &c, true, false);

        SkPDFDocument doc;
        doc.appendPage(dev);
        doc.emitPDF(&pdf);
#endif
    }

    static void generate_xps(GM* gm, SkDynamicMemoryWStream& xps) {
#ifdef SK_SUPPORT_XPS
        SkISize size = gm->getISize();

        SkSize trimSize = SkSize::Make(SkIntToScalar(size.width()),
                                       SkIntToScalar(size.height()));
        static const SkScalar inchesPerMeter = SkScalarDiv(10000, 254);
        static const SkScalar upm = 72 * inchesPerMeter;
        SkVector unitsPerMeter = SkPoint::Make(upm, upm);
        static const SkScalar ppm = 200 * inchesPerMeter;
        SkVector pixelsPerMeter = SkPoint::Make(ppm, ppm);

        SkXPSDevice* dev = new SkXPSDevice();
        SkAutoUnref aur(dev);

        SkCanvas c(dev);
        dev->beginPortfolio(&xps);
        dev->beginSheet(unitsPerMeter, pixelsPerMeter, trimSize);
        invokeGM(gm, &c, false, false);
        dev->endSheet();
        dev->endPortfolio();

#endif
    }

    ErrorCombination write_reference_image(const ConfigData& gRec, const char writePath [],
                                           const char renderModeDescriptor [], const SkString& name,
                                           SkBitmap& bitmap, SkDynamicMemoryWStream* document) {
        SkString path;
        bool success = false;
        if (gRec.fBackend == kRaster_Backend ||
            gRec.fBackend == kGPU_Backend ||
            (gRec.fBackend == kPDF_Backend && CAN_IMAGE_PDF)) {

            path = make_filename(writePath, renderModeDescriptor, name.c_str(),
                                 "png");
            success = write_bitmap(path, bitmap);
        }
        if (kPDF_Backend == gRec.fBackend) {
            path = make_filename(writePath, renderModeDescriptor, name.c_str(),
                                 "pdf");
            success = write_document(path, *document);
        }
        if (kXPS_Backend == gRec.fBackend) {
            path = make_filename(writePath, renderModeDescriptor, name.c_str(),
                                 "xps");
            success = write_document(path, *document);
        }
        if (success) {
            return kEmpty_ErrorCombination;
        } else {
            gm_fprintf(stderr, "FAILED to write %s\n", path.c_str());
            ErrorCombination errors(kWritingReferenceImage_ErrorType);
            RecordError(errors, name, renderModeDescriptor);
            return errors;
        }
    }

    /**
     * Log more detail about the mistmatch between expectedBitmap and
     * actualBitmap.
     */
    void report_bitmap_diffs(const SkBitmap& expectedBitmap, const SkBitmap& actualBitmap,
                             const char *testName) {
        const int expectedWidth = expectedBitmap.width();
        const int expectedHeight = expectedBitmap.height();
        const int width = actualBitmap.width();
        const int height = actualBitmap.height();
        if ((expectedWidth != width) || (expectedHeight != height)) {
            gm_fprintf(stderr, "---- %s: dimension mismatch --"
                       " expected [%d %d], actual [%d %d]\n",
                       testName, expectedWidth, expectedHeight, width, height);
            return;
        }

        if ((SkBitmap::kARGB_8888_Config != expectedBitmap.config()) ||
            (SkBitmap::kARGB_8888_Config != actualBitmap.config())) {
            gm_fprintf(stderr, "---- %s: not computing max per-channel"
                       " pixel mismatch because non-8888\n", testName);
            return;
        }

        SkAutoLockPixels alp0(expectedBitmap);
        SkAutoLockPixels alp1(actualBitmap);
        int errR = 0;
        int errG = 0;
        int errB = 0;
        int errA = 0;
        int differingPixels = 0;

        for (int y = 0; y < height; ++y) {
            const SkPMColor* expectedPixelPtr = expectedBitmap.getAddr32(0, y);
            const SkPMColor* actualPixelPtr = actualBitmap.getAddr32(0, y);
            for (int x = 0; x < width; ++x) {
                SkPMColor expectedPixel = *expectedPixelPtr++;
                SkPMColor actualPixel = *actualPixelPtr++;
                if (expectedPixel != actualPixel) {
                    differingPixels++;
                    errR = SkMax32(errR, SkAbs32((int)SkGetPackedR32(expectedPixel) -
                                                 (int)SkGetPackedR32(actualPixel)));
                    errG = SkMax32(errG, SkAbs32((int)SkGetPackedG32(expectedPixel) -
                                                 (int)SkGetPackedG32(actualPixel)));
                    errB = SkMax32(errB, SkAbs32((int)SkGetPackedB32(expectedPixel) -
                                                 (int)SkGetPackedB32(actualPixel)));
                    errA = SkMax32(errA, SkAbs32((int)SkGetPackedA32(expectedPixel) -
                                                 (int)SkGetPackedA32(actualPixel)));
                }
            }
        }
        gm_fprintf(stderr, "---- %s: %d (of %d) differing pixels,"
                   " max per-channel mismatch R=%d G=%d B=%d A=%d\n",
                   testName, differingPixels, width*height, errR, errG, errB, errA);
    }

    /**
     * Compares actual checksum to expectations, returning the set of errors
     * (if any) that we saw along the way.
     *
     * If fMismatchPath has been set, and there are pixel diffs, then the
     * actual bitmap will be written out to a file within fMismatchPath.
     *
     * @param expectations what expectations to compare actualBitmap against
     * @param actualBitmap the image we actually generated
     * @param baseNameString name of test without renderModeDescriptor added
     * @param renderModeDescriptor e.g., "-rtree", "-deferred"
     * @param addToJsonSummary whether to add these results (both actual and
     *        expected) to the JSON summary
     *
     * TODO: For now, addToJsonSummary is only set to true within
     * compare_test_results_to_stored_expectations(), so results of our
     * in-memory comparisons (Rtree vs regular, etc.) are not written to the
     * JSON summary.  We may wish to change that.
     */
    ErrorCombination compare_to_expectations(Expectations expectations,
                                             const SkBitmap& actualBitmap,
                                             const SkString& baseNameString,
                                             const char renderModeDescriptor[],
                                             bool addToJsonSummary=false) {
        ErrorCombination errors;
        Checksum actualChecksum = SkBitmapChecksummer::Compute64(actualBitmap);
        SkString completeNameString = baseNameString;
        completeNameString.append(renderModeDescriptor);
        const char* completeName = completeNameString.c_str();

        if (expectations.empty()) {
            errors.add(kMissingExpectations_ErrorType);
        } else if (!expectations.match(actualChecksum)) {
            errors.add(kImageMismatch_ErrorType);

            // Write out the "actuals" for any mismatches, if we have
            // been directed to do so.
            if (fMismatchPath) {
                SkString path =
                    make_filename(fMismatchPath, renderModeDescriptor,
                                  baseNameString.c_str(), "png");
                write_bitmap(path, actualBitmap);
            }

            // If we have access to a single expected bitmap, log more
            // detail about the mismatch.
            const SkBitmap *expectedBitmapPtr = expectations.asBitmap();
            if (NULL != expectedBitmapPtr) {
                report_bitmap_diffs(*expectedBitmapPtr, actualBitmap, completeName);
            }
        }
        RecordError(errors, baseNameString, renderModeDescriptor);

        if (addToJsonSummary) {
            add_actual_results_to_json_summary(completeName, actualChecksum, errors,
                                               expectations.ignoreFailure());
            add_expected_results_to_json_summary(completeName, expectations);
        }

        return errors;
    }

    /**
     * Add this result to the appropriate JSON collection of actual results,
     * depending on status.
     */
    void add_actual_results_to_json_summary(const char testName[],
                                            Checksum actualChecksum,
                                            ErrorCombination result,
                                            bool ignoreFailure) {
        Json::Value actualResults;
        actualResults[kJsonKey_ActualResults_AnyStatus_Checksum] =
            asJsonValue(actualChecksum);
        if (result.isEmpty()) {
            this->fJsonActualResults_Succeeded[testName] = actualResults;
        } else {
            if (ignoreFailure) {
                // TODO: Once we have added the ability to compare
                // actual results against expectations in a JSON file
                // (where we can set ignore-failure to either true or
                // false), add test cases that exercise ignored
                // failures (both for kMissingExpectations_ErrorType
                // and kImageMismatch_ErrorType).
                this->fJsonActualResults_FailureIgnored[testName] =
                    actualResults;
            } else {
                if (result.includes(kMissingExpectations_ErrorType)) {
                    // TODO: What about the case where there IS an
                    // expected image checksum, but that gm test
                    // doesn't actually run?  For now, those cases
                    // will always be ignored, because gm only looks
                    // at expectations that correspond to gm tests
                    // that were actually run.
                    //
                    // Once we have the ability to express
                    // expectations as a JSON file, we should fix this
                    // (and add a test case for which an expectation
                    // is given but the test is never run).
                    this->fJsonActualResults_NoComparison[testName] =
                        actualResults;
                }
                if (result.includes(kImageMismatch_ErrorType)) {
                    this->fJsonActualResults_Failed[testName] = actualResults;
                }
            }
        }
    }

    /**
     * Add this test to the JSON collection of expected results.
     */
    void add_expected_results_to_json_summary(const char testName[],
                                              Expectations expectations) {
        // For now, we assume that this collection starts out empty and we
        // just fill it in as we go; once gm accepts a JSON file as input,
        // we'll have to change that.
        Json::Value expectedResults;
        expectedResults[kJsonKey_ExpectedResults_Checksums] =
            expectations.allowedChecksumsAsJson();
        expectedResults[kJsonKey_ExpectedResults_IgnoreFailure] =
            expectations.ignoreFailure();
        this->fJsonExpectedResults[testName] = expectedResults;
    }

    /**
     * Compare actualBitmap to expectations stored in this->fExpectationsSource.
     *
     * @param gm which test generated the actualBitmap
     * @param gRec
     * @param writePath unless this is NULL, write out actual images into this
     *        directory
     * @param actualBitmap bitmap generated by this run
     * @param pdf
     */
    ErrorCombination compare_test_results_to_stored_expectations(
        GM* gm, const ConfigData& gRec, const char writePath[],
        SkBitmap& actualBitmap, SkDynamicMemoryWStream* pdf) {

        SkString name = make_name(gm->shortName(), gRec.fName);
        ErrorCombination errors;

        ExpectationsSource *expectationsSource = this->fExpectationsSource.get();
        if (expectationsSource && (gRec.fFlags & kRead_ConfigFlag)) {
            /*
             * Get the expected results for this test, as one or more allowed
             * checksums. The current implementation of expectationsSource
             * get this by computing the checksum of a single PNG file on disk.
             *
             * TODO(epoger): This relies on the fact that
             * force_all_opaque() was called on the bitmap before it
             * was written to disk as a PNG in the first place. If
             * not, the checksum returned here may not match the
             * checksum of actualBitmap, which *has* been run through
             * force_all_opaque().
             * See comments above complete_bitmap() for more detail.
             */
            Expectations expectations = expectationsSource->get(name.c_str());
            errors.add(compare_to_expectations(expectations, actualBitmap,
                                               name, "", true));
        } else {
            // If we are running without expectations, we still want to
            // record the actual results.
            Checksum actualChecksum =
                SkBitmapChecksummer::Compute64(actualBitmap);
            add_actual_results_to_json_summary(name.c_str(), actualChecksum,
                                               ErrorCombination(kMissingExpectations_ErrorType),
                                               false);
        }

        // TODO: Consider moving this into compare_to_expectations(),
        // similar to fMismatchPath... for now, we don't do that, because
        // we don't want to write out the actual bitmaps for all
        // renderModes of all tests!  That would be a lot of files.
        if (writePath && (gRec.fFlags & kWrite_ConfigFlag)) {
            errors.add(write_reference_image(gRec, writePath, "",
                                             name, actualBitmap, pdf));
        }

        return errors;
    }

    /**
     * Compare actualBitmap to referenceBitmap.
     *
     * @param gm which test generated the bitmap
     * @param gRec
     * @param renderModeDescriptor
     * @param actualBitmap actual bitmap generated by this run
     * @param referenceBitmap bitmap we expected to be generated
     */
    ErrorCombination compare_test_results_to_reference_bitmap(
        GM* gm, const ConfigData& gRec, const char renderModeDescriptor [],
        SkBitmap& actualBitmap, const SkBitmap* referenceBitmap) {

        SkASSERT(referenceBitmap);
        SkString name = make_name(gm->shortName(), gRec.fName);
        Expectations expectations(*referenceBitmap);
        return compare_to_expectations(expectations, actualBitmap,
                                       name, renderModeDescriptor);
    }

    static SkPicture* generate_new_picture(GM* gm, BbhType bbhType, uint32_t recordFlags,
                                           SkScalar scale = SK_Scalar1) {
        // Pictures are refcounted so must be on heap
        SkPicture* pict;
        int width = SkScalarCeilToInt(SkScalarMul(SkIntToScalar(gm->getISize().width()), scale));
        int height = SkScalarCeilToInt(SkScalarMul(SkIntToScalar(gm->getISize().height()), scale));

        if (kTileGrid_BbhType == bbhType) {
            SkTileGridPicture::TileGridInfo info;
            info.fMargin.setEmpty();
            info.fOffset.setZero();
            info.fTileInterval.set(16, 16);
            pict = new SkTileGridPicture(width, height, info);
        } else {
            pict = new SkPicture;
        }
        if (kNone_BbhType != bbhType) {
            recordFlags |= SkPicture::kOptimizeForClippedPlayback_RecordingFlag;
        }
        SkCanvas* cv = pict->beginRecording(width, height, recordFlags);
        cv->scale(scale, scale);
        invokeGM(gm, cv, false, false);
        pict->endRecording();

        return pict;
    }

    static SkPicture* stream_to_new_picture(const SkPicture& src) {

        // To do in-memory commiunications with a stream, we need to:
        // * create a dynamic memory stream
        // * copy it into a buffer
        // * create a read stream from it
        // ?!?!

        SkDynamicMemoryWStream storage;
        src.serialize(&storage);

        size_t streamSize = storage.getOffset();
        SkAutoMalloc dstStorage(streamSize);
        void* dst = dstStorage.get();
        //char* dst = new char [streamSize];
        //@todo thudson 22 April 2011 when can we safely delete [] dst?
        storage.copyTo(dst);
        SkMemoryStream pictReadback(dst, streamSize);
        SkPicture* retval = new SkPicture (&pictReadback);
        return retval;
    }

    // Test: draw into a bitmap or pdf.
    // Depending on flags, possibly compare to an expected image.
    ErrorCombination test_drawing(GM* gm,
                                  const ConfigData& gRec,
                                  const char writePath [],
                                  GrContext* context,
                                  GrRenderTarget* rt,
                                  SkBitmap* bitmap) {
        SkDynamicMemoryWStream document;

        if (gRec.fBackend == kRaster_Backend ||
            gRec.fBackend == kGPU_Backend) {
            // Early exit if we can't generate the image.
            ErrorCombination errors = generate_image(gm, gRec, context, rt, bitmap, false);
            if (!errors.isEmpty()) {
                // TODO: Add a test to exercise what the stdout and
                // JSON look like if we get an "early error" while
                // trying to generate the image.
                return errors;
            }
        } else if (gRec.fBackend == kPDF_Backend) {
            generate_pdf(gm, document);
#if CAN_IMAGE_PDF
            SkAutoDataUnref data(document.copyToData());
            SkMemoryStream stream(data->data(), data->size());
            SkPDFDocumentToBitmap(&stream, bitmap);
#endif
        } else if (gRec.fBackend == kXPS_Backend) {
            generate_xps(gm, document);
        }
        return compare_test_results_to_stored_expectations(
            gm, gRec, writePath, *bitmap, &document);
    }

    ErrorCombination test_deferred_drawing(GM* gm,
                                           const ConfigData& gRec,
                                           const SkBitmap& referenceBitmap,
                                           GrContext* context,
                                           GrRenderTarget* rt) {
        SkDynamicMemoryWStream document;

        if (gRec.fBackend == kRaster_Backend ||
            gRec.fBackend == kGPU_Backend) {
            SkBitmap bitmap;
            // Early exit if we can't generate the image, but this is
            // expected in some cases, so don't report a test failure.
            ErrorCombination errors = generate_image(gm, gRec, context, rt, &bitmap, true);
            // TODO(epoger): This logic is the opposite of what is
            // described above... if we succeeded in generating the
            // -deferred image, we exit early!  We should fix this
            // ASAP, because it is hiding -deferred errors... but for
            // now, I'm leaving the logic as it is so that the
            // refactoring change
            // https://codereview.chromium.org/12992003/ is unblocked.
            //
            // Filed as https://code.google.com/p/skia/issues/detail?id=1180
            // ('image-surface gm test is failing in "deferred" mode,
            // and gm is not reporting the failure')
            if (errors.isEmpty()) {
                return kEmpty_ErrorCombination;
            }
            return compare_test_results_to_reference_bitmap(
                gm, gRec, "-deferred", bitmap, &referenceBitmap);
        }
        return kEmpty_ErrorCombination;
    }

    ErrorCombination test_pipe_playback(GM* gm,
                                        const ConfigData& gRec,
                                        const SkBitmap& referenceBitmap) {
        ErrorCombination errors;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gPipeWritingFlagCombos); ++i) {
            SkBitmap bitmap;
            SkISize size = gm->getISize();
            setup_bitmap(gRec, size, &bitmap);
            SkCanvas canvas(bitmap);
            installFilter(&canvas);
            PipeController pipeController(&canvas);
            SkGPipeWriter writer;
            SkCanvas* pipeCanvas = writer.startRecording(
              &pipeController, gPipeWritingFlagCombos[i].flags);
            invokeGM(gm, pipeCanvas, false, false);
            complete_bitmap(&bitmap);
            writer.endRecording();
            SkString string("-pipe");
            string.append(gPipeWritingFlagCombos[i].name);
            errors.add(compare_test_results_to_reference_bitmap(
                gm, gRec, string.c_str(), bitmap, &referenceBitmap));
            if (!errors.isEmpty()) {
                break;
            }
        }
        return errors;
    }

    ErrorCombination test_tiled_pipe_playback(GM* gm, const ConfigData& gRec,
                                              const SkBitmap& referenceBitmap) {
        ErrorCombination errors;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gPipeWritingFlagCombos); ++i) {
            SkBitmap bitmap;
            SkISize size = gm->getISize();
            setup_bitmap(gRec, size, &bitmap);
            SkCanvas canvas(bitmap);
            installFilter(&canvas);
            TiledPipeController pipeController(bitmap);
            SkGPipeWriter writer;
            SkCanvas* pipeCanvas = writer.startRecording(
              &pipeController, gPipeWritingFlagCombos[i].flags);
            invokeGM(gm, pipeCanvas, false, false);
            complete_bitmap(&bitmap);
            writer.endRecording();
            SkString string("-tiled pipe");
            string.append(gPipeWritingFlagCombos[i].name);
            errors.add(compare_test_results_to_reference_bitmap(
                gm, gRec, string.c_str(), bitmap, &referenceBitmap));
            if (!errors.isEmpty()) {
                break;
            }
        }
        return errors;
    }

    //
    // member variables.
    // They are public for now, to allow easier setting by tool_main().
    //

    bool fUseFileHierarchy;
    ErrorCombination fIgnorableErrorCombination;

    const char* fMismatchPath;

    // information about all failed tests we have encountered so far
    SkTArray<FailRec> fFailedTests;

    // Where to read expectations (expected image checksums, etc.) from.
    // If unset, we don't do comparisons.
    SkAutoTUnref<ExpectationsSource> fExpectationsSource;

    // JSON summaries that we generate as we go (just for output).
    Json::Value fJsonExpectedResults;
    Json::Value fJsonActualResults_Failed;
    Json::Value fJsonActualResults_FailureIgnored;
    Json::Value fJsonActualResults_NoComparison;
    Json::Value fJsonActualResults_Succeeded;

}; // end of GMMain class definition

#if SK_SUPPORT_GPU
static const GLContextType kDontCare_GLContextType = GrContextFactory::kNative_GLContextType;
#else
static const GLContextType kDontCare_GLContextType = 0;
#endif

// If the platform does not support writing PNGs of PDFs then there will be no
// reference images to read. However, we can always write the .pdf files
static const ConfigFlags kPDFConfigFlags = CAN_IMAGE_PDF ? kRW_ConfigFlag :
                                                           kWrite_ConfigFlag;

static const ConfigData gRec[] = {
    { SkBitmap::kARGB_8888_Config, kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "8888",         true },
#if 0   // stop testing this (for now at least) since we want to remove support for it (soon please!!!)
    { SkBitmap::kARGB_4444_Config, kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "4444",         true },
#endif
    { SkBitmap::kRGB_565_Config,   kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "565",          true },
#if SK_SUPPORT_GPU
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kNative_GLContextType,  0, kRW_ConfigFlag,    "gpu",          true },
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kNative_GLContextType, 16, kRW_ConfigFlag,    "msaa16",       true },
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kNative_GLContextType,  4, kRW_ConfigFlag,    "msaa4",        false},
    /* The debug context does not generate images */
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kDebug_GLContextType,   0, kNone_ConfigFlag,  "gpudebug",     GR_DEBUG},
#if SK_ANGLE
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kANGLE_GLContextType,   0, kRW_ConfigFlag,    "angle",        true },
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kANGLE_GLContextType,  16, kRW_ConfigFlag,    "anglemsaa16",  true },
#endif // SK_ANGLE
#ifdef SK_MESA
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kMESA_GLContextType,    0, kRW_ConfigFlag,    "mesa",         true },
#endif // SK_MESA
#endif // SK_SUPPORT_GPU
#ifdef SK_SUPPORT_XPS
    /* At present we have no way of comparing XPS files (either natively or by converting to PNG). */
    { SkBitmap::kARGB_8888_Config, kXPS_Backend,    kDontCare_GLContextType,                  0, kWrite_ConfigFlag, "xps",          true },
#endif // SK_SUPPORT_XPS
#ifdef SK_SUPPORT_PDF
    { SkBitmap::kARGB_8888_Config, kPDF_Backend,    kDontCare_GLContextType,                  0, kPDFConfigFlags,   "pdf",          true },
#endif // SK_SUPPORT_PDF
};

static SkString configUsage() {
    SkString result;
    result.appendf("Space delimited list of which configs to run. Possible options: [");
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        if (i > 0) {
            result.append("|");
        }
        result.appendf("%s", gRec[i].fName);
    }
    result.append("]\n");
    result.appendf("The default value is: \"");
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        if (gRec[i].fRunByDefault) {
            if (i > 0) {
                result.append(" ");
            }
            result.appendf("%s", gRec[i].fName);
        }
    }
    result.appendf("\"");

    return result;
}

// Macro magic to convert a numeric preprocessor token into a string.
// Adapted from http://stackoverflow.com/questions/240353/convert-a-preprocessor-token-to-a-string
// This should probably be moved into one of our common headers...
#define TOSTRING_INTERNAL(x) #x
#define TOSTRING(x) TOSTRING_INTERNAL(x)

// Alphabetized ignoring "no" prefix ("readPath", "noreplay", "resourcePath").
DEFINE_string(config, "", configUsage().c_str());
DEFINE_bool(deferred, true, "Exercise the deferred rendering test pass.");
DEFINE_bool(enableMissingWarning, true, "Print message to stderr (but don't fail) if "
            "unable to read a reference image for any tests.");
DEFINE_string(excludeConfig, "", "Space delimited list of configs to skip.");
DEFINE_bool(forceBWtext, false, "Disable text anti-aliasing.");
#if SK_SUPPORT_GPU
DEFINE_string(gpuCacheSize, "", "<bytes> <count>: Limit the gpu cache to byte size or "
              "object count. " TOSTRING(DEFAULT_CACHE_VALUE) " for either value means "
              "use the default. 0 for either disables the cache.");
#endif
DEFINE_bool(hierarchy, false, "Whether to use multilevel directory structure "
            "when reading/writing files.");
DEFINE_string(match, "",  "Only run tests whose name includes this substring/these substrings "
              "(more than one can be supplied, separated by spaces).");
DEFINE_string(mismatchPath, "", "Write images for tests that failed due to "
              "pixel mismatches into this directory.");
DEFINE_string(modulo, "", "[--modulo <remainder> <divisor>]: only run tests for which "
              "testIndex %% divisor == remainder.");
DEFINE_bool(pdf, true, "Exercise the pdf rendering test pass.");
DEFINE_bool(pipe, true, "Exercise the SkGPipe replay test pass.");
DEFINE_string2(readPath, r, "", "Read reference images from this dir, and report "
               "any differences between those and the newly generated ones.");
DEFINE_bool(replay, true, "Exercise the SkPicture replay test pass.");
DEFINE_string2(resourcePath, i, "", "Directory that stores image resources.");
DEFINE_bool(rtree, true, "Exercise the R-Tree variant of SkPicture test pass.");
DEFINE_bool(serialize, true, "Exercise the SkPicture serialization & deserialization test pass.");
DEFINE_bool(tiledPipe, false, "Exercise tiled SkGPipe replay.");
DEFINE_bool(tileGrid, true, "Exercise the tile grid variant of SkPicture.");
DEFINE_string(tileGridReplayScales, "", "Space separated list of floating-point scale "
              "factors to be used for tileGrid playback testing. Default value: 1.0");
DEFINE_string(writeJsonSummaryPath, "", "Write a JSON-formatted result summary to this file.");
DEFINE_bool2(verbose, v, false, "Print diagnostics (e.g. list each config to be tested).");
DEFINE_string2(writePath, w, "",  "Write rendered images into this directory.");
DEFINE_string2(writePicturePath, wp, "", "Write .skp files into this directory.");

static int findConfig(const char config[]) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        if (!strcmp(config, gRec[i].fName)) {
            return (int) i;
        }
    }
    return -1;
}

static bool skip_name(const SkTDArray<const char*> array, const char name[]) {
    if (0 == array.count()) {
        // no names, so don't skip anything
        return false;
    }
    for (int i = 0; i < array.count(); ++i) {
        if (strstr(name, array[i])) {
            // found the name, so don't skip
            return false;
        }
    }
    return true;
}

namespace skiagm {
#if SK_SUPPORT_GPU
SkAutoTUnref<GrContext> gGrContext;
/**
 * Sets the global GrContext, accessible by individual GMs
 */
static void SetGr(GrContext* grContext) {
    SkSafeRef(grContext);
    gGrContext.reset(grContext);
}

/**
 * Gets the global GrContext, can be called by GM tests.
 */
GrContext* GetGr();
GrContext* GetGr() {
    return gGrContext.get();
}

/**
 * Sets the global GrContext and then resets it to its previous value at
 * destruction.
 */
class AutoResetGr : SkNoncopyable {
public:
    AutoResetGr() : fOld(NULL) {}
    void set(GrContext* context) {
        SkASSERT(NULL == fOld);
        fOld = GetGr();
        SkSafeRef(fOld);
        SetGr(context);
    }
    ~AutoResetGr() { SetGr(fOld); SkSafeUnref(fOld); }
private:
    GrContext* fOld;
};
#else
GrContext* GetGr();
GrContext* GetGr() { return NULL; }
#endif
}

template <typename T> void appendUnique(SkTDArray<T>* array, const T& value) {
    int index = array->find(value);
    if (index < 0) {
        *array->append() = value;
    }
}

/**
 * Run this test in a number of different configs (8888, 565, PDF,
 * etc.), confirming that the resulting bitmaps match expectations
 * (which may be different for each config).
 *
 * Returns all errors encountered while doing so.
 */
ErrorCombination run_multiple_configs(GMMain &gmmain, GM *gm, const SkTDArray<size_t> &configs,
                                      GrContextFactory *grFactory);
ErrorCombination run_multiple_configs(GMMain &gmmain, GM *gm, const SkTDArray<size_t> &configs,
                                      GrContextFactory *grFactory) {
    ErrorCombination errorsForAllConfigs;
    uint32_t gmFlags = gm->getFlags();

    for (int i = 0; i < configs.count(); i++) {
        ConfigData config = gRec[configs[i]];

        // Skip any tests that we don't even need to try.
        if ((kPDF_Backend == config.fBackend) &&
            (!FLAGS_pdf|| (gmFlags & GM::kSkipPDF_Flag))) {
                continue;
            }
        if ((gmFlags & GM::kSkip565_Flag) &&
            (kRaster_Backend == config.fBackend) &&
            (SkBitmap::kRGB_565_Config == config.fConfig)) {
            continue;
        }
        if ((gmFlags & GM::kSkipGPU_Flag) &&
            kGPU_Backend == config.fBackend) {
            continue;
        }

        // Now we know that we want to run this test and record its
        // success or failure.
        ErrorCombination errorsForThisConfig;
        GrRenderTarget* renderTarget = NULL;
#if SK_SUPPORT_GPU
        SkAutoTUnref<GrRenderTarget> rt;
        AutoResetGr autogr;
        if ((errorsForThisConfig.isEmpty()) && (kGPU_Backend == config.fBackend)) {
            GrContext* gr = grFactory->get(config.fGLContextType);
            bool grSuccess = false;
            if (gr) {
                // create a render target to back the device
                GrTextureDesc desc;
                desc.fConfig = kSkia8888_GrPixelConfig;
                desc.fFlags = kRenderTarget_GrTextureFlagBit;
                desc.fWidth = gm->getISize().width();
                desc.fHeight = gm->getISize().height();
                desc.fSampleCnt = config.fSampleCnt;
                GrTexture* tex = gr->createUncachedTexture(desc, NULL, 0);
                if (tex) {
                    rt.reset(tex->asRenderTarget());
                    rt.get()->ref();
                    tex->unref();
                    autogr.set(gr);
                    renderTarget = rt.get();
                    grSuccess = NULL != renderTarget;
                }
                // Set the user specified cache limits if non-default.
                size_t bytes;
                int count;
                gr->getTextureCacheLimits(&count, &bytes);
                if (DEFAULT_CACHE_VALUE != gGpuCacheSizeBytes) {
                    bytes = static_cast<size_t>(gGpuCacheSizeBytes);
                }
                if (DEFAULT_CACHE_VALUE != gGpuCacheSizeCount) {
                    count = gGpuCacheSizeCount;
                }
                gr->setTextureCacheLimits(count, bytes);
            }
            if (!grSuccess) {
                errorsForThisConfig.add(kNoGpuContext_ErrorType);
            }
        }
#endif

        SkBitmap comparisonBitmap;

        const char* writePath;
        if (FLAGS_writePath.count() == 1) {
            writePath = FLAGS_writePath[0];
        } else {
            writePath = NULL;
        }
        if (errorsForThisConfig.isEmpty()) {
            errorsForThisConfig.add(gmmain.test_drawing(gm, config, writePath, GetGr(),
                                                        renderTarget, &comparisonBitmap));
        }

        if (FLAGS_deferred && errorsForThisConfig.isEmpty() &&
            (kGPU_Backend == config.fBackend || kRaster_Backend == config.fBackend)) {
            errorsForThisConfig.add(gmmain.test_deferred_drawing(gm, config, comparisonBitmap,
                                                                 GetGr(), renderTarget));
        }

        errorsForAllConfigs.add(errorsForThisConfig);
    }
    return errorsForAllConfigs;
}

/**
 * Run this test in a number of different drawing modes (pipe,
 * deferred, tiled, etc.), confirming that the resulting bitmaps all
 * *exactly* match comparisonBitmap.
 *
 * Returns all errors encountered while doing so.
 */
ErrorCombination run_multiple_modes(GMMain &gmmain, GM *gm, const ConfigData &compareConfig,
                                    const SkBitmap &comparisonBitmap,
                                    const SkTDArray<SkScalar> &tileGridReplayScales);
ErrorCombination run_multiple_modes(GMMain &gmmain, GM *gm, const ConfigData &compareConfig,
                                    const SkBitmap &comparisonBitmap,
                                    const SkTDArray<SkScalar> &tileGridReplayScales) {
    ErrorCombination errorsForAllModes;
    uint32_t gmFlags = gm->getFlags();

    // run the picture centric GM steps
    if (!(gmFlags & GM::kSkipPicture_Flag)) {

        ErrorCombination pictErrors;

        //SkAutoTUnref<SkPicture> pict(generate_new_picture(gm));
        SkPicture* pict = gmmain.generate_new_picture(gm, kNone_BbhType, 0);
        SkAutoUnref aur(pict);

        if (FLAGS_replay) {
            SkBitmap bitmap;
            gmmain.generate_image_from_picture(gm, compareConfig, pict, &bitmap);
            pictErrors.add(gmmain.compare_test_results_to_reference_bitmap(
                gm, compareConfig, "-replay", bitmap, &comparisonBitmap));
        }

        if ((pictErrors.isEmpty()) && FLAGS_serialize) {
            SkPicture* repict = gmmain.stream_to_new_picture(*pict);
            SkAutoUnref aurr(repict);

            SkBitmap bitmap;
            gmmain.generate_image_from_picture(gm, compareConfig, repict, &bitmap);
            pictErrors.add(gmmain.compare_test_results_to_reference_bitmap(
                gm, compareConfig, "-serialize", bitmap, &comparisonBitmap));
        }

        if (FLAGS_writePicturePath.count() == 1) {
            const char* pictureSuffix = "skp";
            SkString path = make_filename(FLAGS_writePicturePath[0], "",
                                          gm->shortName(), pictureSuffix);
            SkFILEWStream stream(path.c_str());
            pict->serialize(&stream);
        }

        errorsForAllModes.add(pictErrors);
    }

    // TODO: add a test in which the RTree rendering results in a
    // different bitmap than the standard rendering.  It should
    // show up as failed in the JSON summary, and should be listed
    // in the stdout also.
    if (!(gmFlags & GM::kSkipPicture_Flag) && FLAGS_rtree) {
        SkPicture* pict = gmmain.generate_new_picture(
            gm, kRTree_BbhType, SkPicture::kUsePathBoundsForClip_RecordingFlag);
        SkAutoUnref aur(pict);
        SkBitmap bitmap;
        gmmain.generate_image_from_picture(gm, compareConfig, pict, &bitmap);
        errorsForAllModes.add(gmmain.compare_test_results_to_reference_bitmap(
            gm, compareConfig, "-rtree", bitmap, &comparisonBitmap));
    }

    if (!(gmFlags & GM::kSkipPicture_Flag) && FLAGS_tileGrid) {
        for(int scaleIndex = 0; scaleIndex < tileGridReplayScales.count(); ++scaleIndex) {
            SkScalar replayScale = tileGridReplayScales[scaleIndex];
            if ((gmFlags & GM::kSkipScaledReplay_Flag) && replayScale != 1) {
                continue;
            }
            // We record with the reciprocal scale to obtain a replay
            // result that can be validated against comparisonBitmap.
            SkScalar recordScale = SkScalarInvert(replayScale);
            SkPicture* pict = gmmain.generate_new_picture(
                gm, kTileGrid_BbhType, SkPicture::kUsePathBoundsForClip_RecordingFlag, recordScale);
            SkAutoUnref aur(pict);
            SkBitmap bitmap;
            gmmain.generate_image_from_picture(gm, compareConfig, pict, &bitmap, replayScale);
            SkString suffix("-tilegrid");
            if (SK_Scalar1 != replayScale) {
                suffix += "-scale-";
                suffix.appendScalar(replayScale);
            }
            errorsForAllModes.add(gmmain.compare_test_results_to_reference_bitmap(
                gm, compareConfig, suffix.c_str(), bitmap, &comparisonBitmap));
        }
    }

    // run the pipe centric GM steps
    if (!(gmFlags & GM::kSkipPipe_Flag)) {

        ErrorCombination pipeErrors;

        if (FLAGS_pipe) {
            pipeErrors.add(gmmain.test_pipe_playback(gm, compareConfig, comparisonBitmap));
        }

        if ((pipeErrors.isEmpty()) &&
            FLAGS_tiledPipe && !(gmFlags & GM::kSkipTiled_Flag)) {
            pipeErrors.add(gmmain.test_tiled_pipe_playback(gm, compareConfig, comparisonBitmap));
        }

        errorsForAllModes.add(pipeErrors);
    }
    return errorsForAllModes;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {

#if SK_ENABLE_INST_COUNT
    gPrintInstCount = true;
#endif

    SkGraphics::Init();
    // we don't need to see this during a run
    gSkSuppressFontCachePurgeSpew = true;

    setSystemPreferences();
    GMMain gmmain;

    SkTDArray<size_t> configs;
    SkTDArray<size_t> excludeConfigs;
    bool userConfig = false;

    SkString usage;
    usage.printf("Run the golden master tests.\n");
    SkCommandLineFlags::SetUsage(usage.c_str());
    SkCommandLineFlags::Parse(argc, argv);

    gmmain.fUseFileHierarchy = FLAGS_hierarchy;
    if (FLAGS_mismatchPath.count() == 1) {
        gmmain.fMismatchPath = FLAGS_mismatchPath[0];
    }

    for (int i = 0; i < FLAGS_config.count(); i++) {
        int index = findConfig(FLAGS_config[i]);
        if (index >= 0) {
            appendUnique<size_t>(&configs, index);
            userConfig = true;
        } else {
            gm_fprintf(stderr, "unrecognized config %s\n", FLAGS_config[i]);
            return -1;
        }
    }

    for (int i = 0; i < FLAGS_excludeConfig.count(); i++) {
        int index = findConfig(FLAGS_excludeConfig[i]);
        if (index >= 0) {
            *excludeConfigs.append() = index;
        } else {
            gm_fprintf(stderr, "unrecognized excludeConfig %s\n", FLAGS_excludeConfig[i]);
            return -1;
        }
    }

    int moduloRemainder = -1;
    int moduloDivisor = -1;

    if (FLAGS_modulo.count() == 2) {
        moduloRemainder = atoi(FLAGS_modulo[0]);
        moduloDivisor = atoi(FLAGS_modulo[1]);
        if (moduloRemainder < 0 || moduloDivisor <= 0 || moduloRemainder >= moduloDivisor) {
            gm_fprintf(stderr, "invalid modulo values.");
            return -1;
        }
    }

#if SK_SUPPORT_GPU
    if (FLAGS_gpuCacheSize.count() > 0) {
        if (FLAGS_gpuCacheSize.count() != 2) {
            gm_fprintf(stderr, "--gpuCacheSize requires two arguments\n");
            return -1;
        }
        gGpuCacheSizeBytes = atoi(FLAGS_gpuCacheSize[0]);
        gGpuCacheSizeCount = atoi(FLAGS_gpuCacheSize[1]);
    } else {
        gGpuCacheSizeBytes = DEFAULT_CACHE_VALUE;
        gGpuCacheSizeCount = DEFAULT_CACHE_VALUE;
    }
#endif

    SkTDArray<SkScalar> tileGridReplayScales;
    *tileGridReplayScales.append() = SK_Scalar1; // By default only test at scale 1.0
    if (FLAGS_tileGridReplayScales.count() > 0) {
        tileGridReplayScales.reset();
        for (int i = 0; i < FLAGS_tileGridReplayScales.count(); i++) {
            double val = atof(FLAGS_tileGridReplayScales[i]);
            if (0 < val) {
                *tileGridReplayScales.append() = SkDoubleToScalar(val);
            }
        }
        if (0 == tileGridReplayScales.count()) {
            // Should have at least one scale
            gm_fprintf(stderr, "--tileGridReplayScales requires at least one scale.\n");
            return -1;
        }
    }

    if (!userConfig) {
        // if no config is specified by user, add the defaults
        for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
            if (gRec[i].fRunByDefault) {
                *configs.append() = i;
            }
        }
    }
    // now remove any explicitly excluded configs
    for (int i = 0; i < excludeConfigs.count(); ++i) {
        int index = configs.find(excludeConfigs[i]);
        if (index >= 0) {
            configs.remove(index);
            // now assert that there was only one copy in configs[]
            SkASSERT(configs.find(excludeConfigs[i]) < 0);
        }
    }

#if SK_SUPPORT_GPU
    GrContextFactory* grFactory = new GrContextFactory;
    for (int i = 0; i < configs.count(); ++i) {
        size_t index = configs[i];
        if (kGPU_Backend == gRec[index].fBackend) {
            GrContext* ctx = grFactory->get(gRec[index].fGLContextType);
            if (NULL == ctx) {
                SkDebugf("GrContext could not be created for config %s. Config will be skipped.",
                         gRec[index].fName);
                configs.remove(i);
                --i;
            }
            if (gRec[index].fSampleCnt > ctx->getMaxSampleCount()) {
                SkDebugf("Sample count (%d) of config %s is not supported. Config will be skipped.",
                         gRec[index].fSampleCnt, gRec[index].fName);
                configs.remove(i);
                --i;
            }
        }
    }
#else
    GrContextFactory* grFactory = NULL;
#endif

    if (FLAGS_verbose) {
        SkString str;
        str.printf("%d configs:", configs.count());
        for (int i = 0; i < configs.count(); ++i) {
            str.appendf(" %s", gRec[configs[i]].fName);
        }
        gm_fprintf(stderr, "%s\n", str.c_str());
    }

    if (FLAGS_resourcePath.count() == 1) {
        GM::SetResourcePath(FLAGS_resourcePath[0]);
    }

    if (FLAGS_readPath.count() == 1) {
        const char* readPath = FLAGS_readPath[0];
        if (!sk_exists(readPath)) {
            gm_fprintf(stderr, "readPath %s does not exist!\n", readPath);
            return -1;
        }
        if (sk_isdir(readPath)) {
            gm_fprintf(stdout, "reading from %s\n", readPath);
            gmmain.fExpectationsSource.reset(SkNEW_ARGS(
                IndividualImageExpectationsSource,
                (readPath, FLAGS_enableMissingWarning)));
        } else {
            gm_fprintf(stdout, "reading expectations from JSON summary file %s\n", readPath);
            gmmain.fExpectationsSource.reset(SkNEW_ARGS(
                JsonExpectationsSource, (readPath)));
        }
    }
    if (FLAGS_writePath.count() == 1) {
        gm_fprintf(stderr, "writing to %s\n", FLAGS_writePath[0]);
    }
    if (FLAGS_writePicturePath.count() == 1) {
        gm_fprintf(stderr, "writing pictures to %s\n", FLAGS_writePicturePath[0]);
    }
    if (FLAGS_resourcePath.count() == 1) {
        gm_fprintf(stderr, "reading resources from %s\n", FLAGS_resourcePath[0]);
    }

    if (moduloDivisor <= 0) {
        moduloRemainder = -1;
    }
    if (moduloRemainder < 0 || moduloRemainder >= moduloDivisor) {
        moduloRemainder = -1;
    }

    // Accumulate success of all tests.
    int testsRun = 0;
    int testsPassed = 0;
    int testsFailed = 0;
    int testsMissingReferenceImages = 0;

    int gmIndex = -1;
    SkString moduloStr;

    // If we will be writing out files, prepare subdirectories.
    if (FLAGS_writePath.count() == 1) {
        if (!sk_mkdir(FLAGS_writePath[0])) {
            return -1;
        }
        if (gmmain.fUseFileHierarchy) {
            for (int i = 0; i < configs.count(); i++) {
                ConfigData config = gRec[configs[i]];
                SkString subdir;
                subdir.appendf("%s%c%s", FLAGS_writePath[0], SkPATH_SEPARATOR,
                               config.fName);
                if (!sk_mkdir(subdir.c_str())) {
                    return -1;
                }
            }
        }
    }

    Iter iter;
    GM* gm;
    while ((gm = iter.next()) != NULL) {

        ++gmIndex;
        if (moduloRemainder >= 0) {
            if ((gmIndex % moduloDivisor) != moduloRemainder) {
                continue;
            }
            moduloStr.printf("[%d.%d] ", gmIndex, moduloDivisor);
        }

        const char* shortName = gm->shortName();
        if (skip_name(FLAGS_match, shortName)) {
            SkDELETE(gm);
            continue;
        }

        SkISize size = gm->getISize();
        gm_fprintf(stdout, "%sdrawing... %s [%d %d]\n", moduloStr.c_str(), shortName,
                   size.width(), size.height());

        ErrorCombination testErrors;
        testErrors.add(run_multiple_configs(gmmain, gm, configs, grFactory));

        SkBitmap comparisonBitmap;
        const ConfigData compareConfig =
            { SkBitmap::kARGB_8888_Config, kRaster_Backend, kDontCare_GLContextType, 0, kRW_ConfigFlag, "comparison", false };
        testErrors.add(gmmain.generate_image(
            gm, compareConfig, NULL, NULL, &comparisonBitmap, false));

        // TODO(epoger): only run this if gmmain.generate_image() succeeded?
        // Otherwise, what are we comparing against?
        testErrors.add(run_multiple_modes(gmmain, gm, compareConfig, comparisonBitmap,
                                          tileGridReplayScales));

        // Update overall results.
        // We only tabulate the particular error types that we currently
        // care about (e.g., missing reference images). Later on, if we
        // want to also tabulate other error types, we can do so.
        testsRun++;
        if (!gmmain.fExpectationsSource.get() ||
            (testErrors.includes(kMissingExpectations_ErrorType))) {
            testsMissingReferenceImages++;
        }
        if (testErrors.minus(gmmain.fIgnorableErrorCombination).isEmpty()) {
            testsPassed++;
        } else {
            testsFailed++;
        }

        SkDELETE(gm);
    }
    gm_fprintf(stdout, "Ran %d tests: %d passed, %d failed, %d missing reference images\n",
               testsRun, testsPassed, testsFailed, testsMissingReferenceImages);
    gmmain.ListErrors();

    if (FLAGS_writeJsonSummaryPath.count() == 1) {
        Json::Value actualResults;
        actualResults[kJsonKey_ActualResults_Failed] =
            gmmain.fJsonActualResults_Failed;
        actualResults[kJsonKey_ActualResults_FailureIgnored] =
            gmmain.fJsonActualResults_FailureIgnored;
        actualResults[kJsonKey_ActualResults_NoComparison] =
            gmmain.fJsonActualResults_NoComparison;
        actualResults[kJsonKey_ActualResults_Succeeded] =
            gmmain.fJsonActualResults_Succeeded;
        Json::Value root;
        root[kJsonKey_ActualResults] = actualResults;
        root[kJsonKey_ExpectedResults] = gmmain.fJsonExpectedResults;
        std::string jsonStdString = root.toStyledString();
        SkFILEWStream stream(FLAGS_writeJsonSummaryPath[0]);
        stream.write(jsonStdString.c_str(), jsonStdString.length());
    }

#if SK_SUPPORT_GPU

#if GR_CACHE_STATS
    for (int i = 0; i < configs.count(); i++) {
        ConfigData config = gRec[configs[i]];

        if (kGPU_Backend == config.fBackend) {
            GrContext* gr = grFactory->get(config.fGLContextType);

            gm_fprintf(stdout, "config: %s %x\n", config.fName, gr);
            gr->printCacheStats();
        }
    }
#endif

    delete grFactory;
#endif
    SkGraphics::Term();

    return (0 == testsFailed) ? 0 : -1;
}

void GMMain::installFilter(SkCanvas* canvas) {
    if (FLAGS_forceBWtext) {
        canvas->setDrawFilter(SkNEW(BWTextDrawFilter))->unref();
    }
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
