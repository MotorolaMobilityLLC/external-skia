/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkAndroidCodec.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkCodecImageGenerator.h"
#include "SkData.h"
#include "SkFrontBufferedStream.h"
#include "SkMD5.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkPngChunkReader.h"
#include "Test.h"

#include "png.h"

static SkStreamAsset* resource(const char path[]) {
    SkString fullPath = GetResourcePath(path);
    return SkStream::NewFromFile(fullPath.c_str());
}

static void md5(const SkBitmap& bm, SkMD5::Digest* digest) {
    SkAutoLockPixels autoLockPixels(bm);
    SkASSERT(bm.getPixels());
    SkMD5 md5;
    size_t rowLen = bm.info().bytesPerPixel() * bm.width();
    for (int y = 0; y < bm.height(); ++y) {
        md5.write(bm.getAddr(0, y), rowLen);
    }
    md5.finish(*digest);
}

/**
 *  Compute the digest for bm and compare it to a known good digest.
 *  @param r Reporter to assert that bm's digest matches goodDigest.
 *  @param goodDigest The known good digest to compare to.
 *  @param bm The bitmap to test.
 */
static void compare_to_good_digest(skiatest::Reporter* r, const SkMD5::Digest& goodDigest,
                           const SkBitmap& bm) {
    SkMD5::Digest digest;
    md5(bm, &digest);
    REPORTER_ASSERT(r, digest == goodDigest);
}

/**
 *  Test decoding an SkCodec to a particular SkImageInfo.
 *
 *  Calling getPixels(info) should return expectedResult, and if goodDigest is non nullptr,
 *  the resulting decode should match.
 */
template<typename Codec>
static void test_info(skiatest::Reporter* r, Codec* codec, const SkImageInfo& info,
                      SkCodec::Result expectedResult, const SkMD5::Digest* goodDigest) {
    SkBitmap bm;
    bm.allocPixels(info);
    SkAutoLockPixels autoLockPixels(bm);

    SkCodec::Result result = codec->getPixels(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == expectedResult);

    if (goodDigest) {
        compare_to_good_digest(r, *goodDigest, bm);
    }
}

SkIRect generate_random_subset(SkRandom* rand, int w, int h) {
    SkIRect rect;
    do {
        rect.fLeft = rand->nextRangeU(0, w);
        rect.fTop = rand->nextRangeU(0, h);
        rect.fRight = rand->nextRangeU(0, w);
        rect.fBottom = rand->nextRangeU(0, h);
        rect.sort();
    } while (rect.isEmpty());
    return rect;
}

static void test_incremental_decode(skiatest::Reporter* r, SkCodec* codec, const SkImageInfo& info,
        const SkMD5::Digest& goodDigest) {
    SkBitmap bm;
    bm.allocPixels(info);
    SkAutoLockPixels autoLockPixels(bm);

    REPORTER_ASSERT(r, SkCodec::kSuccess == codec->startIncrementalDecode(info, bm.getPixels(),
                                                                          bm.rowBytes()));

    REPORTER_ASSERT(r, SkCodec::kSuccess == codec->incrementalDecode());

    compare_to_good_digest(r, goodDigest, bm);
}

// Test in stripes, similar to DM's kStripe_Mode
static void test_in_stripes(skiatest::Reporter* r, SkCodec* codec, const SkImageInfo& info,
                            const SkMD5::Digest& goodDigest) {
    SkBitmap bm;
    bm.allocPixels(info);
    bm.eraseColor(SK_ColorYELLOW);

    const int height = info.height();
    // Note that if numStripes does not evenly divide height there will be an extra
    // stripe.
    const int numStripes = 4;

    if (numStripes > height) {
        // Image is too small.
        return;
    }

    const int stripeHeight = height / numStripes;

    // Iterate through the image twice. Once to decode odd stripes, and once for even.
    for (int oddEven = 1; oddEven >= 0; oddEven--) {
        for (int y = oddEven * stripeHeight; y < height; y += 2 * stripeHeight) {
            SkIRect subset = SkIRect::MakeLTRB(0, y, info.width(),
                                               SkTMin(y + stripeHeight, height));
            SkCodec::Options options;
            options.fSubset = &subset;
            if (SkCodec::kSuccess != codec->startIncrementalDecode(info, bm.getAddr(0, y),
                        bm.rowBytes(), &options)) {
                ERRORF(r, "failed to start incremental decode!\ttop: %i\tbottom%i\n",
                       subset.top(), subset.bottom());
                return;
            }
            if (SkCodec::kSuccess != codec->incrementalDecode()) {
                ERRORF(r, "failed incremental decode starting from line %i\n", y);
                return;
            }
        }
    }

    compare_to_good_digest(r, goodDigest, bm);
}

template<typename Codec>
static void test_codec(skiatest::Reporter* r, Codec* codec, SkBitmap& bm, const SkImageInfo& info,
        const SkISize& size, SkCodec::Result expectedResult, SkMD5::Digest* digest,
        const SkMD5::Digest* goodDigest) {

    REPORTER_ASSERT(r, info.dimensions() == size);
    bm.allocPixels(info);
    SkAutoLockPixels autoLockPixels(bm);

    SkCodec::Result result = codec->getPixels(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == expectedResult);

    md5(bm, digest);
    if (goodDigest) {
        REPORTER_ASSERT(r, *digest == *goodDigest);
    }

    {
        // Test decoding to 565
        SkImageInfo info565 = info.makeColorType(kRGB_565_SkColorType);
        if (info.alphaType() == kOpaque_SkAlphaType) {
            // Decoding to 565 should succeed.
            SkBitmap bm565;
            bm565.allocPixels(info565);
            SkAutoLockPixels alp(bm565);

            // This will allow comparison even if the image is incomplete.
            bm565.eraseColor(SK_ColorBLACK);

            REPORTER_ASSERT(r, expectedResult == codec->getPixels(info565,
                    bm565.getPixels(), bm565.rowBytes()));

            SkMD5::Digest digest565;
            md5(bm565, &digest565);

            // A dumb client's request for non-opaque should also succeed.
            for (auto alpha : { kPremul_SkAlphaType, kUnpremul_SkAlphaType }) {
                info565 = info565.makeAlphaType(alpha);
                test_info(r, codec, info565, expectedResult, &digest565);
            }
        } else {
            test_info(r, codec, info565, SkCodec::kInvalidConversion, nullptr);
        }
    }

    if (codec->getInfo().colorType() == kGray_8_SkColorType) {
        SkImageInfo grayInfo = codec->getInfo();
        SkBitmap grayBm;
        grayBm.allocPixels(grayInfo);
        SkAutoLockPixels alp(grayBm);

        grayBm.eraseColor(SK_ColorBLACK);

        REPORTER_ASSERT(r, expectedResult == codec->getPixels(grayInfo,
                grayBm.getPixels(), grayBm.rowBytes()));

        SkMD5::Digest grayDigest;
        md5(grayBm, &grayDigest);

        for (auto alpha : { kPremul_SkAlphaType, kUnpremul_SkAlphaType }) {
            grayInfo = grayInfo.makeAlphaType(alpha);
            test_info(r, codec, grayInfo, expectedResult, &grayDigest);
        }
    }

    // Verify that re-decoding gives the same result.  It is interesting to check this after
    // a decode to 565, since choosing to decode to 565 may result in some of the decode
    // options being modified.  These options should return to their defaults on another
    // decode to kN32, so the new digest should match the old digest.
    test_info(r, codec, info, expectedResult, digest);

    {
        // Check alpha type conversions
        if (info.alphaType() == kOpaque_SkAlphaType) {
            test_info(r, codec, info.makeAlphaType(kUnpremul_SkAlphaType),
                      expectedResult, digest);
            test_info(r, codec, info.makeAlphaType(kPremul_SkAlphaType),
                      expectedResult, digest);
        } else {
            // Decoding to opaque should fail
            test_info(r, codec, info.makeAlphaType(kOpaque_SkAlphaType),
                      SkCodec::kInvalidConversion, nullptr);
            SkAlphaType otherAt = info.alphaType();
            if (kPremul_SkAlphaType == otherAt) {
                otherAt = kUnpremul_SkAlphaType;
            } else {
                otherAt = kPremul_SkAlphaType;
            }
            // The other non-opaque alpha type should always succeed, but not match.
            test_info(r, codec, info.makeAlphaType(otherAt), expectedResult, nullptr);
        }
    }
}

static bool supports_partial_scanlines(const char path[]) {
    static const char* const exts[] = {
        "jpg", "jpeg", "png", "webp"
        "JPG", "JPEG", "PNG", "WEBP"
    };

    for (uint32_t i = 0; i < SK_ARRAY_COUNT(exts); i++) {
        if (SkStrEndsWith(path, exts[i])) {
            return true;
        }
    }
    return false;
}

// FIXME: Break up this giant function
static void check(skiatest::Reporter* r,
                  const char path[],
                  SkISize size,
                  bool supportsScanlineDecoding,
                  bool supportsSubsetDecoding,
                  bool supportsIncomplete,
                  bool supportsNewScanlineDecoding = false) {

    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }

    SkAutoTDelete<SkCodec> codec(nullptr);
    bool isIncomplete = supportsIncomplete;
    if (isIncomplete) {
        size_t size = stream->getLength();
        SkAutoTUnref<SkData> data((SkData::NewFromStream(stream, 2 * size / 3)));
        codec.reset(SkCodec::NewFromData(data));
    } else {
        codec.reset(SkCodec::NewFromStream(stream.release()));
    }
    if (!codec) {
        ERRORF(r, "Unable to decode '%s'", path);
        return;
    }

    // Test full image decodes with SkCodec
    SkMD5::Digest codecDigest;
    const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bm;
    SkCodec::Result expectedResult = isIncomplete ? SkCodec::kIncompleteInput : SkCodec::kSuccess;
    test_codec(r, codec.get(), bm, info, size, expectedResult, &codecDigest, nullptr);

    // Scanline decoding follows.

    if (supportsNewScanlineDecoding && !isIncomplete) {
        test_incremental_decode(r, codec, info, codecDigest);
        test_in_stripes(r, codec, info, codecDigest);
    }

    // Need to call startScanlineDecode() first.
    REPORTER_ASSERT(r, codec->getScanlines(bm.getAddr(0, 0), 1, 0) == 0);
    REPORTER_ASSERT(r, !codec->skipScanlines(1));
    const SkCodec::Result startResult = codec->startScanlineDecode(info);
    if (supportsScanlineDecoding) {
        bm.eraseColor(SK_ColorYELLOW);

        REPORTER_ASSERT(r, startResult == SkCodec::kSuccess);

        for (int y = 0; y < info.height(); y++) {
            const int lines = codec->getScanlines(bm.getAddr(0, y), 1, 0);
            if (!isIncomplete) {
                REPORTER_ASSERT(r, 1 == lines);
            }
        }
        // verify that scanline decoding gives the same result.
        if (SkCodec::kTopDown_SkScanlineOrder == codec->getScanlineOrder()) {
            compare_to_good_digest(r, codecDigest, bm);
        }

        // Cannot continue to decode scanlines beyond the end
        REPORTER_ASSERT(r, codec->getScanlines(bm.getAddr(0, 0), 1, 0)
                == 0);

        // Interrupting a scanline decode with a full decode starts from
        // scratch
        REPORTER_ASSERT(r, codec->startScanlineDecode(info) == SkCodec::kSuccess);
        const int lines = codec->getScanlines(bm.getAddr(0, 0), 1, 0);
        if (!isIncomplete) {
            REPORTER_ASSERT(r, lines == 1);
        }
        REPORTER_ASSERT(r, codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes())
                == expectedResult);
        REPORTER_ASSERT(r, codec->getScanlines(bm.getAddr(0, 0), 1, 0)
                == 0);
        REPORTER_ASSERT(r, codec->skipScanlines(1)
                == 0);

        // Test partial scanline decodes
        if (supports_partial_scanlines(path) && info.width() >= 3) {
            SkCodec::Options options;
            int width = info.width();
            int height = info.height();
            SkIRect subset = SkIRect::MakeXYWH(2 * (width / 3), 0, width / 3, height);
            options.fSubset = &subset;

            const SkCodec::Result partialStartResult = codec->startScanlineDecode(info, &options,
                    nullptr, nullptr);
            REPORTER_ASSERT(r, partialStartResult == SkCodec::kSuccess);

            for (int y = 0; y < height; y++) {
                const int lines = codec->getScanlines(bm.getAddr(0, y), 1, 0);
                if (!isIncomplete) {
                    REPORTER_ASSERT(r, 1 == lines);
                }
            }
        }
    } else {
        REPORTER_ASSERT(r, startResult == SkCodec::kUnimplemented);
    }

    // The rest of this function tests decoding subsets, and will decode an arbitrary number of
    // random subsets.
    // Do not attempt to decode subsets of an image of only once pixel, since there is no
    // meaningful subset.
    if (size.width() * size.height() == 1) {
        return;
    }

    SkRandom rand;
    SkIRect subset;
    SkCodec::Options opts;
    opts.fSubset = &subset;
    for (int i = 0; i < 5; i++) {
        subset = generate_random_subset(&rand, size.width(), size.height());
        SkASSERT(!subset.isEmpty());
        const bool supported = codec->getValidSubset(&subset);
        REPORTER_ASSERT(r, supported == supportsSubsetDecoding);

        SkImageInfo subsetInfo = info.makeWH(subset.width(), subset.height());
        SkBitmap bm;
        bm.allocPixels(subsetInfo);
        const SkCodec::Result result = codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes(),
                                                        &opts, nullptr, nullptr);

        if (supportsSubsetDecoding) {
            REPORTER_ASSERT(r, result == expectedResult);
            // Webp is the only codec that supports subsets, and it will have modified the subset
            // to have even left/top.
            REPORTER_ASSERT(r, SkIsAlign2(subset.fLeft) && SkIsAlign2(subset.fTop));
        } else {
            // No subsets will work.
            REPORTER_ASSERT(r, result == SkCodec::kUnimplemented);
        }
    }

    // SkAndroidCodec tests
    if (supportsScanlineDecoding || supportsSubsetDecoding || supportsNewScanlineDecoding) {

        SkAutoTDelete<SkStream> stream(resource(path));
        if (!stream) {
            SkDebugf("Missing resource '%s'\n", path);
            return;
        }

        SkAutoTDelete<SkAndroidCodec> androidCodec(nullptr);
        if (isIncomplete) {
            size_t size = stream->getLength();
            SkAutoTUnref<SkData> data((SkData::NewFromStream(stream, 2 * size / 3)));
            androidCodec.reset(SkAndroidCodec::NewFromData(data));
        } else {
            androidCodec.reset(SkAndroidCodec::NewFromStream(stream.release()));
        }
        if (!androidCodec) {
            ERRORF(r, "Unable to decode '%s'", path);
            return;
        }

        SkBitmap bm;
        SkMD5::Digest androidCodecDigest;
        test_codec(r, androidCodec.get(), bm, info, size, expectedResult, &androidCodecDigest,
                   &codecDigest);
    }

    if (!isIncomplete) {
        // Test SkCodecImageGenerator
        SkAutoTDelete<SkStream> stream(resource(path));
        SkAutoTUnref<SkData> fullData(SkData::NewFromStream(stream, stream->getLength()));
        SkAutoTDelete<SkImageGenerator> gen(SkCodecImageGenerator::NewFromEncodedCodec(fullData));
        SkBitmap bm;
        bm.allocPixels(info);
        SkAutoLockPixels autoLockPixels(bm);
        REPORTER_ASSERT(r, gen->getPixels(info, bm.getPixels(), bm.rowBytes()));
        compare_to_good_digest(r, codecDigest, bm);

#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR < 5
        // FIXME: With older versions of libpng, SkPngCodec requires being able to call
        // SkStream::move(), which is not supported by SkFrontBufferedStream. (Android
        // has a more recent version of libpng which uses png_process_data_pause to
        // avoid calling move().)
        if (!SkStrEndsWith(path, ".png"))
#endif
        {
            // Test using SkFrontBufferedStream, as Android does
            SkStream* bufferedStream = SkFrontBufferedStream::Create(new SkMemoryStream(fullData),
                    SkCodec::MinBufferedBytesNeeded());
            REPORTER_ASSERT(r, bufferedStream);
            codec.reset(SkCodec::NewFromStream(bufferedStream));
            REPORTER_ASSERT(r, codec);
            if (codec) {
                test_info(r, codec.get(), info, SkCodec::kSuccess, &codecDigest);
            }
        }
    }

    // If we've just tested incomplete decodes, let's run the same test again on full decodes.
    if (isIncomplete) {
        check(r, path, size, supportsScanlineDecoding, supportsSubsetDecoding, false);
    }
}

DEF_TEST(Codec, r) {
    // WBMP
    check(r, "mandrill.wbmp", SkISize::Make(512, 512), true, false, true);

    // WEBP
    check(r, "baby_tux.webp", SkISize::Make(386, 395), false, true, true);
    check(r, "color_wheel.webp", SkISize::Make(128, 128), false, true, true);
    check(r, "yellow_rose.webp", SkISize::Make(400, 301), false, true, true);

    // BMP
    check(r, "randPixels.bmp", SkISize::Make(8, 8), true, false, true);
    check(r, "rle.bmp", SkISize::Make(320, 240), true, false, true);

    // ICO
    // FIXME: We are not ready to test incomplete ICOs
    // These two tests examine interestingly different behavior:
    // Decodes an embedded BMP image
    check(r, "color_wheel.ico", SkISize::Make(128, 128), true, false, false);
    // Decodes an embedded PNG image
    check(r, "google_chrome.ico", SkISize::Make(256, 256), false, false, false, true);

    // GIF
    // FIXME: We are not ready to test incomplete GIFs
    check(r, "box.gif", SkISize::Make(200, 55), true, false, false);
    check(r, "color_wheel.gif", SkISize::Make(128, 128), true, false, false);
    // randPixels.gif is too small to test incomplete
    check(r, "randPixels.gif", SkISize::Make(8, 8), true, false, false);

    // JPG
    check(r, "CMYK.jpg", SkISize::Make(642, 516), true, false, true);
    check(r, "color_wheel.jpg", SkISize::Make(128, 128), true, false, true);
    // grayscale.jpg is too small to test incomplete
    check(r, "grayscale.jpg", SkISize::Make(128, 128), true, false, false);
    check(r, "mandrill_512_q075.jpg", SkISize::Make(512, 512), true, false, true);
    // randPixels.jpg is too small to test incomplete
    check(r, "randPixels.jpg", SkISize::Make(8, 8), true, false, false);

    // PNG
    check(r, "arrow.png", SkISize::Make(187, 312), false, false, true, true);
    check(r, "baby_tux.png", SkISize::Make(240, 246), false, false, true, true);
    check(r, "color_wheel.png", SkISize::Make(128, 128), false, false, true, true);
    // half-transparent-white-pixel.png is too small to test incomplete
    check(r, "half-transparent-white-pixel.png", SkISize::Make(1, 1), false, false, false, true);
    check(r, "mandrill_128.png", SkISize::Make(128, 128), false, false, true, true);
    check(r, "mandrill_16.png", SkISize::Make(16, 16), false, false, true, true);
    check(r, "mandrill_256.png", SkISize::Make(256, 256), false, false, true, true);
    check(r, "mandrill_32.png", SkISize::Make(32, 32), false, false, true, true);
    check(r, "mandrill_512.png", SkISize::Make(512, 512), false, false, true, true);
    check(r, "mandrill_64.png", SkISize::Make(64, 64), false, false, true, true);
    check(r, "plane.png", SkISize::Make(250, 126), false, false, true, true);
    check(r, "plane_interlaced.png", SkISize::Make(250, 126), false, false, true, true);
    check(r, "randPixels.png", SkISize::Make(8, 8), false, false, true, true);
    check(r, "yellow_rose.png", SkISize::Make(400, 301), false, false, true, true);

    // RAW
// Disable RAW tests for Win32.
#if defined(SK_CODEC_DECODES_RAW) && (!defined(_WIN32))
    check(r, "sample_1mp.dng", SkISize::Make(600, 338), false, false, false);
    check(r, "sample_1mp_rotated.dng", SkISize::Make(600, 338), false, false, false);
    check(r, "dng_with_preview.dng", SkISize::Make(600, 338), true, false, false);
#endif
}

static void test_invalid_stream(skiatest::Reporter* r, const void* stream, size_t len) {
    // Neither of these calls should return a codec. Bots should catch us if we leaked anything.
    SkCodec* codec = SkCodec::NewFromStream(new SkMemoryStream(stream, len, false));
    REPORTER_ASSERT(r, !codec);

    SkAndroidCodec* androidCodec =
            SkAndroidCodec::NewFromStream(new SkMemoryStream(stream, len, false));
    REPORTER_ASSERT(r, !androidCodec);
}

// Ensure that SkCodec::NewFromStream handles freeing the passed in SkStream,
// even on failure. Test some bad streams.
DEF_TEST(Codec_leaks, r) {
    // No codec should claim this as their format, so this tests SkCodec::NewFromStream.
    const char nonSupportedStream[] = "hello world";
    // The other strings should look like the beginning of a file type, so we'll call some
    // internal version of NewFromStream, which must also delete the stream on failure.
    const unsigned char emptyPng[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
    const unsigned char emptyJpeg[] = { 0xFF, 0xD8, 0xFF };
    const char emptyWebp[] = "RIFF1234WEBPVP";
    const char emptyBmp[] = { 'B', 'M' };
    const char emptyIco[] = { '\x00', '\x00', '\x01', '\x00' };
    const char emptyGif[] = "GIFVER";

    test_invalid_stream(r, nonSupportedStream, sizeof(nonSupportedStream));
    test_invalid_stream(r, emptyPng, sizeof(emptyPng));
    test_invalid_stream(r, emptyJpeg, sizeof(emptyJpeg));
    test_invalid_stream(r, emptyWebp, sizeof(emptyWebp));
    test_invalid_stream(r, emptyBmp, sizeof(emptyBmp));
    test_invalid_stream(r, emptyIco, sizeof(emptyIco));
    test_invalid_stream(r, emptyGif, sizeof(emptyGif));
}

DEF_TEST(Codec_null, r) {
    // Attempting to create an SkCodec or an SkAndroidCodec with null should not
    // crash.
    SkCodec* codec = SkCodec::NewFromStream(nullptr);
    REPORTER_ASSERT(r, !codec);

    SkAndroidCodec* androidCodec = SkAndroidCodec::NewFromStream(nullptr);
    REPORTER_ASSERT(r, !androidCodec);
}

static void test_dimensions(skiatest::Reporter* r, const char path[]) {
    // Create the codec from the resource file
    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }
    SkAutoTDelete<SkAndroidCodec> codec(SkAndroidCodec::NewFromStream(stream.release()));
    if (!codec) {
        ERRORF(r, "Unable to create codec '%s'", path);
        return;
    }

    // Check that the decode is successful for a variety of scales
    for (int sampleSize = 1; sampleSize < 32; sampleSize++) {
        // Scale the output dimensions
        SkISize scaledDims = codec->getSampledDimensions(sampleSize);
        SkImageInfo scaledInfo = codec->getInfo()
                .makeWH(scaledDims.width(), scaledDims.height())
                .makeColorType(kN32_SkColorType);

        // Set up for the decode
        size_t rowBytes = scaledDims.width() * sizeof(SkPMColor);
        size_t totalBytes = scaledInfo.getSafeSize(rowBytes);
        SkAutoTMalloc<SkPMColor> pixels(totalBytes);

        SkAndroidCodec::AndroidOptions options;
        options.fSampleSize = sampleSize;
        SkCodec::Result result =
                codec->getAndroidPixels(scaledInfo, pixels.get(), rowBytes, &options);
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    }
}

// Ensure that onGetScaledDimensions returns valid image dimensions to use for decodes
DEF_TEST(Codec_Dimensions, r) {
    // JPG
    test_dimensions(r, "CMYK.jpg");
    test_dimensions(r, "color_wheel.jpg");
    test_dimensions(r, "grayscale.jpg");
    test_dimensions(r, "mandrill_512_q075.jpg");
    test_dimensions(r, "randPixels.jpg");

    // Decoding small images with very large scaling factors is a potential
    // source of bugs and crashes.  We disable these tests in Gold because
    // tiny images are not very useful to look at.
    // Here we make sure that we do not crash or access illegal memory when
    // performing scaled decodes on small images.
    test_dimensions(r, "1x1.png");
    test_dimensions(r, "2x2.png");
    test_dimensions(r, "3x3.png");
    test_dimensions(r, "3x1.png");
    test_dimensions(r, "1x1.png");
    test_dimensions(r, "16x1.png");
    test_dimensions(r, "1x16.png");
    test_dimensions(r, "mandrill_16.png");

    // RAW
// Disable RAW tests for Win32.
#if defined(SK_CODEC_DECODES_RAW) && (!defined(_WIN32))
    test_dimensions(r, "sample_1mp.dng");
    test_dimensions(r, "sample_1mp_rotated.dng");
    test_dimensions(r, "dng_with_preview.dng");
#endif
}

static void test_invalid(skiatest::Reporter* r, const char path[]) {
    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream.release()));
    REPORTER_ASSERT(r, nullptr == codec);
}

DEF_TEST(Codec_Empty, r) {
    // Test images that should not be able to create a codec
    test_invalid(r, "empty_images/zero-dims.gif");
    test_invalid(r, "empty_images/zero-embedded.ico");
    test_invalid(r, "empty_images/zero-width.bmp");
    test_invalid(r, "empty_images/zero-height.bmp");
    test_invalid(r, "empty_images/zero-width.jpg");
    test_invalid(r, "empty_images/zero-height.jpg");
    test_invalid(r, "empty_images/zero-width.png");
    test_invalid(r, "empty_images/zero-height.png");
    test_invalid(r, "empty_images/zero-width.wbmp");
    test_invalid(r, "empty_images/zero-height.wbmp");
    // This image is an ico with an embedded mask-bmp.  This is illegal.
    test_invalid(r, "invalid_images/mask-bmp-ico.ico");
}

static void test_invalid_parameters(skiatest::Reporter* r, const char path[]) {
    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }
    SkAutoTDelete<SkCodec> decoder(SkCodec::NewFromStream(stream.release()));
    if (!decoder) {
        SkDebugf("Missing codec for %s\n", path);
        return;
    }

    const SkImageInfo info = decoder->getInfo().makeColorType(kIndex_8_SkColorType);

    // This should return kSuccess because kIndex8 is supported.
    SkPMColor colorStorage[256];
    int colorCount;
    SkCodec::Result result = decoder->startScanlineDecode(info, nullptr, colorStorage,
                                                          &colorCount);
    if (SkCodec::kSuccess == result) {
        // This should return kInvalidParameters because, in kIndex_8 mode, we must pass in a valid
        // colorPtr and a valid colorCountPtr.
        result = decoder->startScanlineDecode(info, nullptr, nullptr, nullptr);
        REPORTER_ASSERT(r, SkCodec::kInvalidParameters == result);
        result = decoder->startScanlineDecode(info);
        REPORTER_ASSERT(r, SkCodec::kInvalidParameters == result);
    } else if (SkCodec::kUnimplemented == result) {
        // New method should be supported:
        SkBitmap bm;
        sk_sp<SkColorTable> colorTable(new SkColorTable(colorStorage, 256));
        bm.allocPixels(info, nullptr, colorTable.get());
        result = decoder->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes(), nullptr,
                                                 colorStorage, &colorCount);
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);
        result = decoder->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes());
        REPORTER_ASSERT(r, SkCodec::kInvalidParameters == result);
    } else {
        // The test is uninteresting if kIndex8 is not supported
        ERRORF(r, "Should not call test_invalid_parameters for non-Index8 file: %s\n", path);
        return;
    }

}

DEF_TEST(Codec_Params, r) {
    test_invalid_parameters(r, "index8.png");
    test_invalid_parameters(r, "mandrill.wbmp");
}

static void codex_test_write_fn(png_structp png_ptr, png_bytep data, png_size_t len) {
    SkWStream* sk_stream = (SkWStream*)png_get_io_ptr(png_ptr);
    if (!sk_stream->write(data, len)) {
        png_error(png_ptr, "sk_write_fn Error!");
    }
}

#ifdef PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
DEF_TEST(Codec_pngChunkReader, r) {
    // Create a dummy bitmap. Use unpremul RGBA for libpng.
    SkBitmap bm;
    const int w = 1;
    const int h = 1;
    const SkImageInfo bmInfo = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType,
                                                 kUnpremul_SkAlphaType);
    bm.setInfo(bmInfo);
    bm.allocPixels();
    bm.eraseColor(SK_ColorBLUE);
    SkMD5::Digest goodDigest;
    md5(bm, &goodDigest);

    // Write to a png file.
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    REPORTER_ASSERT(r, png);
    if (!png) {
        return;
    }

    png_infop info = png_create_info_struct(png);
    REPORTER_ASSERT(r, info);
    if (!info) {
        png_destroy_write_struct(&png, nullptr);
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        ERRORF(r, "failed writing png");
        png_destroy_write_struct(&png, &info);
        return;
    }

    SkDynamicMemoryWStream wStream;
    png_set_write_fn(png, (void*) (&wStream), codex_test_write_fn, nullptr);

    png_set_IHDR(png, info, (png_uint_32)w, (png_uint_32)h, 8,
                 PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    // Create some chunks that match the Android framework's use.
    static png_unknown_chunk gUnknowns[] = {
        { "npOl", (png_byte*)"outline", sizeof("outline"), PNG_HAVE_IHDR },
        { "npLb", (png_byte*)"layoutBounds", sizeof("layoutBounds"), PNG_HAVE_IHDR },
        { "npTc", (png_byte*)"ninePatchData", sizeof("ninePatchData"), PNG_HAVE_IHDR },
    };

    png_set_keep_unknown_chunks(png, PNG_HANDLE_CHUNK_ALWAYS, (png_byte*)"npOl\0npLb\0npTc\0", 3);
    png_set_unknown_chunks(png, info, gUnknowns, SK_ARRAY_COUNT(gUnknowns));
#if PNG_LIBPNG_VER < 10600
    /* Deal with unknown chunk location bug in 1.5.x and earlier */
    png_set_unknown_chunk_location(png, info, 0, PNG_HAVE_IHDR);
    png_set_unknown_chunk_location(png, info, 1, PNG_HAVE_IHDR);
#endif

    png_write_info(png, info);

    for (int j = 0; j < h; j++) {
        png_bytep row = (png_bytep)(bm.getAddr(0, j));
        png_write_rows(png, &row, 1);
    }
    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);

    class ChunkReader : public SkPngChunkReader {
    public:
        ChunkReader(skiatest::Reporter* r)
            : fReporter(r)
        {
            this->reset();
        }

        bool readChunk(const char tag[], const void* data, size_t length) override {
            for (size_t i = 0; i < SK_ARRAY_COUNT(gUnknowns); ++i) {
                if (!strcmp(tag, (const char*) gUnknowns[i].name)) {
                    // Tag matches. This should have been the first time we see it.
                    REPORTER_ASSERT(fReporter, !fSeen[i]);
                    fSeen[i] = true;

                    // Data and length should match
                    REPORTER_ASSERT(fReporter, length == gUnknowns[i].size);
                    REPORTER_ASSERT(fReporter, !strcmp((const char*) data,
                                                       (const char*) gUnknowns[i].data));
                    return true;
                }
            }
            ERRORF(fReporter, "Saw an unexpected unknown chunk.");
            return true;
        }

        bool allHaveBeenSeen() {
            bool ret = true;
            for (auto seen : fSeen) {
                ret &= seen;
            }
            return ret;
        }

        void reset() {
            sk_bzero(fSeen, sizeof(fSeen));
        }

    private:
        skiatest::Reporter* fReporter;  // Unowned
        bool fSeen[3];
    };

    ChunkReader chunkReader(r);

    // Now read the file with SkCodec.
    SkAutoTUnref<SkData> data(wStream.copyToData());
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(data, &chunkReader));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    // Now compare to the original.
    SkBitmap decodedBm;
    decodedBm.setInfo(codec->getInfo());
    decodedBm.allocPixels();
    SkCodec::Result result = codec->getPixels(codec->getInfo(), decodedBm.getPixels(),
                                              decodedBm.rowBytes());
    REPORTER_ASSERT(r, SkCodec::kSuccess == result);

    if (decodedBm.colorType() != bm.colorType()) {
        SkBitmap tmp;
        bool success = decodedBm.copyTo(&tmp, bm.colorType());
        REPORTER_ASSERT(r, success);
        if (!success) {
            return;
        }

        tmp.swap(decodedBm);
    }

    compare_to_good_digest(r, goodDigest, decodedBm);
    REPORTER_ASSERT(r, chunkReader.allHaveBeenSeen());

    // Decoding again will read the chunks again.
    chunkReader.reset();
    REPORTER_ASSERT(r, !chunkReader.allHaveBeenSeen());
    result = codec->getPixels(codec->getInfo(), decodedBm.getPixels(), decodedBm.rowBytes());
    REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    REPORTER_ASSERT(r, chunkReader.allHaveBeenSeen());
}
#endif // PNG_READ_UNKNOWN_CHUNKS_SUPPORTED

// Stream that can only peek up to a limit
class LimitedPeekingMemStream : public SkStream {
public:
    LimitedPeekingMemStream(SkData* data, size_t limit)
        : fStream(data)
        , fLimit(limit) {}

    size_t peek(void* buf, size_t bytes) const override {
        return fStream.peek(buf, SkTMin(bytes, fLimit));
    }
    size_t read(void* buf, size_t bytes) override {
        return fStream.read(buf, bytes);
    }
    bool rewind() override {
        return fStream.rewind();
    }
    bool isAtEnd() const override {
        return false;
    }
private:
    SkMemoryStream fStream;
    const size_t   fLimit;
};

// Stream that is not an asset stream (!hasPosition() or !hasLength())
class NotAssetMemStream : public SkStream {
public:
    NotAssetMemStream(SkData* data) : fStream(data) {}

    bool hasPosition() const override {
        return false;
    }

    bool hasLength() const override {
        return false;
    }

    size_t peek(void* buf, size_t bytes) const override {
        return fStream.peek(buf, bytes);
    }
    size_t read(void* buf, size_t bytes) override {
        return fStream.read(buf, bytes);
    }
    bool rewind() override {
        return fStream.rewind();
    }
    bool isAtEnd() const override {
        return fStream.isAtEnd();
    }
private:
    SkMemoryStream fStream;
};

// Disable RAW tests for Win32.
#if defined(SK_CODEC_DECODES_RAW) && (!defined(_WIN32))
// Test that the RawCodec works also for not asset stream. This will test the code path using
// SkRawBufferedStream instead of SkRawAssetStream.
DEF_TEST(Codec_raw_notseekable, r) {
    const char* path = "dng_with_preview.dng";
    SkString fullPath(GetResourcePath(path));
    SkAutoTUnref<SkData> data(SkData::NewFromFileName(fullPath.c_str()));
    if (!data) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }

    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(new NotAssetMemStream(data)));
    REPORTER_ASSERT(r, codec);

    test_info(r, codec.get(), codec->getInfo(), SkCodec::kSuccess, nullptr);
}
#endif

// Test that even if webp_parse_header fails to peek enough, it will fall back to read()
// + rewind() and succeed.
DEF_TEST(Codec_webp_peek, r) {
    const char* path = "baby_tux.webp";
    SkString fullPath(GetResourcePath(path));
    auto data = SkData::MakeFromFileName(fullPath.c_str());
    if (!data) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }

    // The limit is less than webp needs to peek or read.
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(
                                 new LimitedPeekingMemStream(data.get(), 25)));
    REPORTER_ASSERT(r, codec);

    test_info(r, codec.get(), codec->getInfo(), SkCodec::kSuccess, nullptr);

    // Similarly, a stream which does not peek should still succeed.
    codec.reset(SkCodec::NewFromStream(new LimitedPeekingMemStream(data.get(), 0)));
    REPORTER_ASSERT(r, codec);

    test_info(r, codec.get(), codec->getInfo(), SkCodec::kSuccess, nullptr);
}

// SkCodec's wbmp decoder was initially unnecessarily restrictive.
// It required the second byte to be zero. The wbmp specification allows
// a couple of bits to be 1 (so long as they do not overlap with 0x9F).
// Test that SkCodec now supports an image with these bits set.
DEF_TEST(Codec_wbmp, r) {
    const char* path = "mandrill.wbmp";
    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }

    // Modify the stream to contain a second byte with some bits set.
    auto data = SkCopyStreamToData(stream);
    uint8_t* writeableData = static_cast<uint8_t*>(data->writable_data());
    writeableData[1] = static_cast<uint8_t>(~0x9F);

    // SkCodec should support this.
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(data.get()));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }
    test_info(r, codec.get(), codec->getInfo(), SkCodec::kSuccess, nullptr);
}

// wbmp images have a header that can be arbitrarily large, depending on the
// size of the image. We cap the size at 65535, meaning we only need to look at
// 8 bytes to determine whether we can read the image. This is important
// because SkCodec only passes 14 bytes to SkWbmpCodec to determine whether the
// image is a wbmp.
DEF_TEST(Codec_wbmp_max_size, r) {
    const unsigned char maxSizeWbmp[] = { 0x00, 0x00,           // Header
                                          0x83, 0xFF, 0x7F,     // W: 65535
                                          0x83, 0xFF, 0x7F };   // H: 65535
    SkAutoTDelete<SkStream> stream(new SkMemoryStream(maxSizeWbmp, sizeof(maxSizeWbmp), false));
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream.release()));

    REPORTER_ASSERT(r, codec);
    if (!codec) return;

    REPORTER_ASSERT(r, codec->getInfo().width() == 65535);
    REPORTER_ASSERT(r, codec->getInfo().height() == 65535);

    // Now test an image which is too big. Any image with a larger header (i.e.
    // has bigger width/height) is also too big.
    const unsigned char tooBigWbmp[] = { 0x00, 0x00,           // Header
                                         0x84, 0x80, 0x00,     // W: 65536
                                         0x84, 0x80, 0x00 };   // H: 65536
    stream.reset(new SkMemoryStream(tooBigWbmp, sizeof(tooBigWbmp), false));
    codec.reset(SkCodec::NewFromStream(stream.release()));

    REPORTER_ASSERT(r, !codec);
}

// Only rewinds up to a limit.
class LimitedRewindingStream : public SkStream {
public:
    static SkStream* Make(const char path[], size_t limit) {
        SkStream* stream = resource(path);
        if (!stream) {
            return nullptr;
        }
        return new LimitedRewindingStream(stream, limit);
    }

    size_t read(void* buffer, size_t size) override {
        const size_t bytes = fStream->read(buffer, size);
        fPosition += bytes;
        return bytes;
    }

    bool isAtEnd() const override {
        return fStream->isAtEnd();
    }

    bool rewind() override {
        if (fPosition <= fLimit && fStream->rewind()) {
            fPosition = 0;
            return true;
        }

        return false;
    }

private:
    SkAutoTDelete<SkStream> fStream;
    const size_t            fLimit;
    size_t                  fPosition;

    LimitedRewindingStream(SkStream* stream, size_t limit)
        : fStream(stream)
        , fLimit(limit)
        , fPosition(0)
    {
        SkASSERT(fStream);
    }
};

DEF_TEST(Codec_fallBack, r) {
    // SkAndroidCodec needs to be able to fall back to scanline decoding
    // if incremental decoding does not work. Make sure this does not
    // require a rewind.

    // Formats that currently do not support incremental decoding
    auto files = {
            "box.gif",
            "CMYK.jpg",
            "color_wheel.ico",
            "mandrill.wbmp",
            "randPixels.bmp",
            };
    for (auto file : files) {
        SkStream* stream = LimitedRewindingStream::Make(file, 14);
        if (!stream) {
            SkDebugf("Missing resources (%s). Set --resourcePath.\n", file);
            return;
        }

        SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream));
        if (!codec) {
            ERRORF(r, "Failed to create codec for %s,", file);
            continue;
        }

        SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
        SkBitmap bm;
        bm.allocPixels(info);

        if (SkCodec::kUnimplemented != codec->startIncrementalDecode(info, bm.getPixels(),
                bm.rowBytes())) {
            ERRORF(r, "Is scanline decoding now implemented for %s?", file);
            continue;
        }

        // Scanline decoding should not require a rewind.
        SkCodec::Result result = codec->startScanlineDecode(info);
        if (SkCodec::kSuccess != result) {
            ERRORF(r, "Scanline decoding failed for %s with %i", file, result);
        }
    }
}
