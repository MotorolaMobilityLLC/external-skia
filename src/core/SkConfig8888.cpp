/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorSpaceXform.h"
#include "SkColorSpaceXformPriv.h"
#include "SkConfig8888.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkImageInfoPriv.h"
#include "SkMathPriv.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkUnPreMultiply.h"

// Fast Path 1: The memcpy() case.
static inline bool can_memcpy(const SkImageInfo& dstInfo, const SkImageInfo& srcInfo) {
    if (dstInfo.colorType() != srcInfo.colorType()) {
        return false;
    }

    if (dstInfo.alphaType() != srcInfo.alphaType() &&
        kOpaque_SkAlphaType != dstInfo.alphaType() &&
        kOpaque_SkAlphaType != srcInfo.alphaType())
    {
        // We need to premultiply or unpremultiply.
        return false;
    }

    return !dstInfo.colorSpace() ||
           SkColorSpace::Equals(dstInfo.colorSpace(), srcInfo.colorSpace());
}

// Default: Use the pipeline.
static bool copy_pipeline_pixels(const SkImageInfo& dstInfo, void* dstRow, size_t dstRB,
                                 const SkImageInfo& srcInfo, const void* srcRow, size_t srcRB,
                                 bool isColorAware) {
    SkRasterPipeline pipeline;
    switch (srcInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::load_8888, &srcRow);
            break;
        case kBGRA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::load_8888, &srcRow);
            pipeline.append(SkRasterPipeline::swap_rb);
            break;
        case kRGB_565_SkColorType:
            pipeline.append(SkRasterPipeline::load_565, &srcRow);
            break;
        case kRGBA_F16_SkColorType:
            pipeline.append(SkRasterPipeline::load_f16, &srcRow);
            break;
        case kGray_8_SkColorType:
            pipeline.append(SkRasterPipeline::load_g8, &srcRow);
            break;
        default:
            return false;
    }

    if (isColorAware && srcInfo.gammaCloseToSRGB()) {
        pipeline.append_from_srgb(srcInfo.alphaType());
    }

    float matrix[12];
    if (isColorAware) {
        SkAssertResult(append_gamut_transform(&pipeline, matrix, srcInfo.colorSpace(),
                                              dstInfo.colorSpace()));
    }

    SkAlphaType sat = srcInfo.alphaType();
    SkAlphaType dat = dstInfo.alphaType();
    if (sat == kPremul_SkAlphaType && dat == kUnpremul_SkAlphaType) {
        pipeline.append(SkRasterPipeline::unpremul);
    } else if (sat == kUnpremul_SkAlphaType && dat == kPremul_SkAlphaType) {
        pipeline.append(SkRasterPipeline::premul);
    }

    if (isColorAware && dstInfo.gammaCloseToSRGB()) {
        pipeline.append(SkRasterPipeline::to_srgb);
    }

    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::store_8888, &dstRow);
            break;
        case kBGRA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::swap_rb);
            pipeline.append(SkRasterPipeline::store_8888, &dstRow);
            break;
        case kRGB_565_SkColorType:
            pipeline.append(SkRasterPipeline::store_565, &dstRow);
            break;
        case kRGBA_F16_SkColorType:
            pipeline.append(SkRasterPipeline::store_f16, &dstRow);
            break;
        default:
            return false;
    }

    auto p = pipeline.compile();

    for (int y = 0; y < srcInfo.height(); ++y) {
        p(0,srcInfo.width());
        // The pipeline has pointers to srcRow and dstRow, so we just need to update them in the
        // loop to move between rows of src/dst.
        dstRow = SkTAddOffset<void>(dstRow, dstRB);
        srcRow = SkTAddOffset<const void>(srcRow, srcRB);
    }
    return true;
}

enum AlphaVerb {
    kNothing_AlphaVerb,
    kPremul_AlphaVerb,
    kUnpremul_AlphaVerb,
};

template <bool doSwapRB, AlphaVerb doAlpha> uint32_t convert32(uint32_t c) {
    if (doSwapRB) {
        c = SkSwizzle_RB(c);
    }

    // Lucky for us, in both RGBA and BGRA, the alpha component is always in the same place, so
    // we can perform premul or unpremul the same way without knowing the swizzles for RGB.
    switch (doAlpha) {
        case kNothing_AlphaVerb:
            // no change
            break;
        case kPremul_AlphaVerb:
            c = SkPreMultiplyARGB(SkGetPackedA32(c), SkGetPackedR32(c),
                                  SkGetPackedG32(c), SkGetPackedB32(c));
            break;
        case kUnpremul_AlphaVerb:
            c = SkUnPreMultiply::UnPreMultiplyPreservingByteOrder(c);
            break;
    }
    return c;
}

template <bool doSwapRB, AlphaVerb doAlpha>
void convert32_row(uint32_t* dst, const uint32_t* src, int count) {
    // This has to be correct if src == dst (but not partial overlap)
    for (int i = 0; i < count; ++i) {
        dst[i] = convert32<doSwapRB, doAlpha>(src[i]);
    }
}

static bool is_32bit_colortype(SkColorType ct) {
    return kRGBA_8888_SkColorType == ct || kBGRA_8888_SkColorType == ct;
}

static AlphaVerb compute_AlphaVerb(SkAlphaType src, SkAlphaType dst) {
    SkASSERT(kUnknown_SkAlphaType != src);
    SkASSERT(kUnknown_SkAlphaType != dst);

    if (kOpaque_SkAlphaType == src || kOpaque_SkAlphaType == dst || src == dst) {
        return kNothing_AlphaVerb;
    }
    if (kPremul_SkAlphaType == dst) {
        SkASSERT(kUnpremul_SkAlphaType == src);
        return kPremul_AlphaVerb;
    } else {
        SkASSERT(kPremul_SkAlphaType == src);
        SkASSERT(kUnpremul_SkAlphaType == dst);
        return kUnpremul_AlphaVerb;
    }
}

bool SkSrcPixelInfo::convertPixelsTo(SkDstPixelInfo* dst, int width, int height) const {
    SkASSERT(width > 0 && height > 0);

    if (!is_32bit_colortype(fColorType) || !is_32bit_colortype(dst->fColorType)) {
        return false;
    }

    void (*proc)(uint32_t* dst, const uint32_t* src, int count);
    AlphaVerb doAlpha = compute_AlphaVerb(fAlphaType, dst->fAlphaType);
    bool doSwapRB = fColorType != dst->fColorType;

    switch (doAlpha) {
        case kNothing_AlphaVerb:
            SkASSERT(doSwapRB);
            proc = convert32_row<true, kNothing_AlphaVerb>;
            break;
        case kPremul_AlphaVerb:
            if (doSwapRB) {
                proc = convert32_row<true, kPremul_AlphaVerb>;
            } else {
                proc = convert32_row<false, kPremul_AlphaVerb>;
            }
            break;
        case kUnpremul_AlphaVerb:
            if (doSwapRB) {
                proc = convert32_row<true, kUnpremul_AlphaVerb>;
            } else {
                proc = convert32_row<false, kUnpremul_AlphaVerb>;
            }
            break;
    }

    uint32_t* dstP = static_cast<uint32_t*>(dst->fPixels);
    const uint32_t* srcP = static_cast<const uint32_t*>(fPixels);
    size_t srcInc = fRowBytes >> 2;
    size_t dstInc = dst->fRowBytes >> 2;
    for (int y = 0; y < height; ++y) {
        proc(dstP, srcP, width);
        dstP += dstInc;
        srcP += srcInc;
    }
    return true;
}

static bool extract_alpha(void* dst, size_t dstRB, const void* src, size_t srcRB,
                          const SkImageInfo& srcInfo, SkColorTable* ctable) {
    uint8_t* SK_RESTRICT dst8 = (uint8_t*)dst;

    const int w = srcInfo.width();
    const int h = srcInfo.height();
    if (srcInfo.isOpaque()) {
        // src is opaque, so just fill alpha with 0xFF
        for (int y = 0; y < h; ++y) {
           memset(dst8, 0xFF, w);
           dst8 += dstRB;
        }
        return true;
    }
    switch (srcInfo.colorType()) {
        case kN32_SkColorType: {
            const SkPMColor* SK_RESTRICT src32 = (const SkPMColor*)src;
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    dst8[x] = SkGetPackedA32(src32[x]);
                }
                dst8 += dstRB;
                src32 = (const SkPMColor*)((const char*)src32 + srcRB);
            }
            break;
        }
        case kARGB_4444_SkColorType: {
            const SkPMColor16* SK_RESTRICT src16 = (const SkPMColor16*)src;
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    dst8[x] = SkPacked4444ToA32(src16[x]);
                }
                dst8 += dstRB;
                src16 = (const SkPMColor16*)((const char*)src16 + srcRB);
            }
            break;
        }
        case kIndex_8_SkColorType: {
            if (nullptr == ctable) {
                return false;
            }
            const SkPMColor* SK_RESTRICT table = ctable->readColors();
            const uint8_t* SK_RESTRICT src8 = (const uint8_t*)src;
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    dst8[x] = SkGetPackedA32(table[src8[x]]);
                }
                dst8 += dstRB;
                src8 += srcRB;
            }
            break;
        }
        default:
            return false;
    }
    return true;
}

static inline bool optimized_color_xform(const SkImageInfo& dstInfo, const SkImageInfo& srcInfo) {
    if (kUnpremul_SkAlphaType == dstInfo.alphaType() && kPremul_SkAlphaType == srcInfo.alphaType())
    {
        return false;
    }

    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_F16_SkColorType:
            break;
        default:
            return false;
    }

    switch (srcInfo.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            break;
        default:
            return false;
    }

    return true;
}

static inline void apply_color_xform(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                     const SkImageInfo& srcInfo, const void* srcPixels,
                                     size_t srcRB) {
    SkColorSpaceXform::ColorFormat dstFormat = select_xform_format(dstInfo.colorType());
    SkColorSpaceXform::ColorFormat srcFormat = select_xform_format(srcInfo.colorType());
    SkAlphaType xformAlpha;
    switch (srcInfo.alphaType()) {
        case kOpaque_SkAlphaType:
            xformAlpha = kOpaque_SkAlphaType;
            break;
        case kPremul_SkAlphaType:
            SkASSERT(kPremul_SkAlphaType == dstInfo.alphaType());

            // This signal means: copy the src alpha to the dst, do not premultiply (in this
            // case because the pixels are already premultiplied).
            xformAlpha = kUnpremul_SkAlphaType;
            break;
        case kUnpremul_SkAlphaType:
            SkASSERT(kPremul_SkAlphaType == dstInfo.alphaType() ||
                     kUnpremul_SkAlphaType == dstInfo.alphaType());

            xformAlpha = dstInfo.alphaType();
            break;
        default:
            SkASSERT(false);
            xformAlpha = kUnpremul_SkAlphaType;
            break;
    }

    std::unique_ptr<SkColorSpaceXform> xform = SkColorSpaceXform::New(srcInfo.colorSpace(),
                                                                      dstInfo.colorSpace());
    SkASSERT(xform);

    for (int y = 0; y < dstInfo.height(); y++) {
        SkAssertResult(xform->apply(dstFormat, dstPixels, srcFormat, srcPixels, dstInfo.width(),
                       xformAlpha));
        dstPixels = SkTAddOffset<void>(dstPixels, dstRB);
        srcPixels = SkTAddOffset<const void>(srcPixels, srcRB);
    }
}

bool SkPixelInfo::CopyPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                             const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB,
                             SkColorTable* ctable) {
    SkASSERT(dstInfo.dimensions() == srcInfo.dimensions());
    SkASSERT(SkImageInfoValidConversion(dstInfo, srcInfo));

    const int width = srcInfo.width();
    const int height = srcInfo.height();

    // Fast Path 1: The memcpy() case.
    if (can_memcpy(dstInfo, srcInfo)) {
        SkRectMemcpy(dstPixels, dstRB, srcPixels, srcRB, dstInfo.minRowBytes(), dstInfo.height());
        return true;
    }

    const bool isColorAware = dstInfo.colorSpace();
    SkASSERT(srcInfo.colorSpace() || !isColorAware);

    // Handle fancy alpha swizzling if both are ARGB32
    if (4 == srcInfo.bytesPerPixel() && 4 == dstInfo.bytesPerPixel() && !isColorAware) {
        SkDstPixelInfo dstPI;
        dstPI.fColorType = dstInfo.colorType();
        dstPI.fAlphaType = dstInfo.alphaType();
        dstPI.fPixels = dstPixels;
        dstPI.fRowBytes = dstRB;

        SkSrcPixelInfo srcPI;
        srcPI.fColorType = srcInfo.colorType();
        srcPI.fAlphaType = srcInfo.alphaType();
        srcPI.fPixels = srcPixels;
        srcPI.fRowBytes = srcRB;

        return srcPI.convertPixelsTo(&dstPI, width, height);
    }

    if (isColorAware && optimized_color_xform(dstInfo, srcInfo)) {
        apply_color_xform(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB);
        return true;
    }

    /*
     *  Begin section where we try to change colorTypes along the way. Not all combinations
     *  are supported.
     */

    if (kAlpha_8_SkColorType == dstInfo.colorType() &&
        extract_alpha(dstPixels, dstRB, srcPixels, srcRB, srcInfo, ctable)) {
        return true;
    }

    //  Try the pipeline
    //
    if (copy_pipeline_pixels(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, ctable)) {
        return true;
    }

    // Can no longer draw directly into 4444, but we can manually whack it for a few combinations
    if (kARGB_4444_SkColorType == dstInfo.colorType() &&
        (kN32_SkColorType == srcInfo.colorType() || kIndex_8_SkColorType == srcInfo.colorType())) {
        if (srcInfo.alphaType() == kUnpremul_SkAlphaType) {
            // Our method for converting to 4444 assumes premultiplied.
            return false;
        }

        const SkPMColor* table = nullptr;
        if (kIndex_8_SkColorType == srcInfo.colorType()) {
            SkASSERT(ctable);
            table = ctable->readColors();
        }

        for (int y = 0; y < height; ++y) {
            DITHER_4444_SCAN(y);
            SkPMColor16* SK_RESTRICT dstRow = (SkPMColor16*)dstPixels;
            if (table) {
                const uint8_t* SK_RESTRICT srcRow = (const uint8_t*)srcPixels;
                for (int x = 0; x < width; ++x) {
                    dstRow[x] = SkDitherARGB32To4444(table[srcRow[x]], DITHER_VALUE(x));
                }
            } else {
                const SkPMColor* SK_RESTRICT srcRow = (const SkPMColor*)srcPixels;
                for (int x = 0; x < width; ++x) {
                    dstRow[x] = SkDitherARGB32To4444(srcRow[x], DITHER_VALUE(x));
                }
            }
            dstPixels = (char*)dstPixels + dstRB;
            srcPixels = (const char*)srcPixels + srcRB;
        }
        return true;
    }

    if (dstInfo.alphaType() == kUnpremul_SkAlphaType) {
        // We do not support drawing to unpremultiplied bitmaps.
        return false;
    }

    // Final fall-back, draw with a canvas
    //
    // Always clear the dest in case one of the blitters accesses it
    // TODO: switch the allocation of tmpDst to call sk_calloc_throw
    {
        SkBitmap bm;
        if (!bm.installPixels(srcInfo, const_cast<void*>(srcPixels), srcRB, ctable, nullptr, nullptr)) {
            return false;
        }
        std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirect(dstInfo, dstPixels, dstRB);
        if (!canvas) {
            return false;
        }

        SkPaint  paint;
        paint.setDither(true);

        canvas->clear(0);
        canvas->drawBitmap(bm, 0, 0, &paint);
        return true;
    }
}
