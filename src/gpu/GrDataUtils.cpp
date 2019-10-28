/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDataUtils.h"

#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkUtils.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrImageInfo.h"

struct ETC1Block {
    uint32_t fHigh;
    uint32_t fLow;
};

static const int kNumModifierTables = 8;
static const int kNumPixelIndices = 4;

// The index of each row in this table is the ETC1 table codeword
// The index of each column in this table is the ETC1 pixel index value
static const int kModifierTables[kNumModifierTables][kNumPixelIndices] = {
    /* 0 */ { 2,    8,  -2,   -8 },
    /* 1 */ { 5,   17,  -5,  -17 },
    /* 2 */ { 9,   29,  -9,  -29 },
    /* 3 */ { 13,  42, -13,  -42 },
    /* 4 */ { 18,  60, -18,  -60 },
    /* 5 */ { 24,  80, -24,  -80 },
    /* 6 */ { 33, 106, -33, -106 },
    /* 7 */ { 47, 183, -47, -183 }
};

static inline int convert_5To8(int b) {
    int c = b & 0x1f;
    return (c << 3) | (c >> 2);
}

// Evaluate one of the entries in 'kModifierTables' to see how close it can get (r8,g8,b8) to
// the original color (rOrig, gOrib, bOrig).
static int test_table_entry(int rOrig, int gOrig, int bOrig,
                            int r8, int g8, int b8,
                            int table, int offset) {
    SkASSERT(0 <= table && table < 8);
    SkASSERT(0 <= offset && offset < 4);

    r8 = SkTPin<uint8_t>(r8 + kModifierTables[table][offset], 0, 255);
    g8 = SkTPin<uint8_t>(g8 + kModifierTables[table][offset], 0, 255);
    b8 = SkTPin<uint8_t>(b8 + kModifierTables[table][offset], 0, 255);

    return SkTAbs(rOrig - r8) + SkTAbs(gOrig - g8) + SkTAbs(bOrig - b8);
}

// Create an ETC1 compressed block that is filled with 'col'
static void create_etc1_block(SkColor col, ETC1Block* block) {
    block->fHigh = 0;
    block->fLow = 0;

    int rOrig = SkColorGetR(col);
    int gOrig = SkColorGetG(col);
    int bOrig = SkColorGetB(col);

    int r5 = SkMulDiv255Round(31, rOrig);
    int g5 = SkMulDiv255Round(31, gOrig);
    int b5 = SkMulDiv255Round(31, bOrig);

    int r8 = convert_5To8(r5);
    int g8 = convert_5To8(g5);
    int b8 = convert_5To8(b5);

    // We always encode solid color textures as 555 + zero diffs
    block->fHigh |= (r5 << 27) | (g5 << 19) | (b5 << 11) | 0x2;

    int bestTableIndex = 0, bestPixelIndex = 0;
    int bestSoFar = 1024;
    for (int tableIndex = 0; tableIndex < kNumModifierTables; ++tableIndex) {
        for (int pixelIndex = 0; pixelIndex < kNumPixelIndices; ++pixelIndex) {
            int score = test_table_entry(rOrig, gOrig, bOrig, r8, g8, b8,
                                         tableIndex, pixelIndex);

            if (bestSoFar > score) {
                bestSoFar = score;
                bestTableIndex = tableIndex;
                bestPixelIndex = pixelIndex;
            }
        }
    }

    block->fHigh |= (bestTableIndex << 5) | (bestTableIndex << 2);

    for (int i = 0; i < 16; ++i) {
        block->fLow |= bestPixelIndex << 2*i;
    }
}

static int num_ETC1_blocks_w(int w) {
    if (w < 4) {
        w = 1;
    } else {
        SkASSERT((w & 3) == 0);
        w >>= 2;
    }
    return w;
}

static int num_ETC1_blocks(int w, int h) {
    w = num_ETC1_blocks_w(w);

    if (h < 4) {
        h = 1;
    } else {
       SkASSERT((h & 3) == 0);
       h >>= 2;
    }

    return w * h;
}

size_t GrCompressedDataSize(SkImage::CompressionType type, int width, int height) {
    switch (type) {
        case SkImage::kETC1_CompressionType:
            int numBlocks = num_ETC1_blocks(width, height);
            return numBlocks * sizeof(ETC1Block);
    }
    SK_ABORT("Unexpected compression type");
}

size_t GrCompressedRowBytes(SkImage::CompressionType type, int width) {
    switch (type) {
        case SkImage::kETC1_CompressionType:
            int numBlocksWidth = num_ETC1_blocks_w(width);
            return numBlocksWidth * sizeof(ETC1Block);
    }
    SK_ABORT("Unexpected compression type");
}

// Fill in 'dest' with ETC1 blocks derived from 'colorf'
static void fillin_ETC1_with_color(int width, int height, const SkColor4f& colorf, void* dest) {
    SkColor color = colorf.toSkColor();

    ETC1Block block;
    create_etc1_block(color, &block);

    int numBlocks = num_ETC1_blocks(width, height);

    for (int i = 0; i < numBlocks; ++i) {
        ((ETC1Block*)dest)[i] = block;
    }
}

size_t GrComputeTightCombinedBufferSize(size_t bytesPerPixel, int baseWidth, int baseHeight,
                                        SkTArray<size_t>* individualMipOffsets, int mipLevelCount) {
    SkASSERT(individualMipOffsets && !individualMipOffsets->count());
    SkASSERT(mipLevelCount >= 1);

    individualMipOffsets->push_back(0);

    size_t combinedBufferSize = baseWidth * bytesPerPixel * baseHeight;
    int currentWidth = baseWidth;
    int currentHeight = baseHeight;

    // The Vulkan spec for copying a buffer to an image requires that the alignment must be at
    // least 4 bytes and a multiple of the bytes per pixel of the image config.
    SkASSERT(bytesPerPixel == 1 || bytesPerPixel == 2 || bytesPerPixel == 3 ||
             bytesPerPixel == 4 || bytesPerPixel == 8 || bytesPerPixel == 16);
    int desiredAlignment = (bytesPerPixel == 3) ? 12 : (bytesPerPixel > 4 ? bytesPerPixel : 4);

    for (int currentMipLevel = 1; currentMipLevel < mipLevelCount; ++currentMipLevel) {
        currentWidth = SkTMax(1, currentWidth / 2);
        currentHeight = SkTMax(1, currentHeight / 2);

        size_t trimmedSize = currentWidth * bytesPerPixel * currentHeight;
        const size_t alignmentDiff = combinedBufferSize % desiredAlignment;
        if (alignmentDiff != 0) {
            combinedBufferSize += desiredAlignment - alignmentDiff;
        }
        SkASSERT((0 == combinedBufferSize % 4) && (0 == combinedBufferSize % bytesPerPixel));

        individualMipOffsets->push_back(combinedBufferSize);
        combinedBufferSize += trimmedSize;
    }

    SkASSERT(individualMipOffsets->count() == mipLevelCount);
    return combinedBufferSize;
}

void GrFillInCompressedData(SkImage::CompressionType type, int baseWidth, int baseHeight,
                            char* dstPixels, const SkColor4f& colorf) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    int currentWidth = baseWidth;
    int currentHeight = baseHeight;
    if (SkImage::kETC1_CompressionType == type) {
        fillin_ETC1_with_color(currentWidth, currentHeight, colorf, dstPixels);
    }
}

static GrSwizzle get_load_and_get_swizzle(GrColorType ct, SkRasterPipeline::StockStage* load,
                                          bool* isNormalized, bool* isSRGB) {
    GrSwizzle swizzle("rgba");
    *isNormalized = true;
    *isSRGB = false;
    switch (ct) {
        case GrColorType::kAlpha_8:          *load = SkRasterPipeline::load_a8;       break;
        case GrColorType::kAlpha_16:         *load = SkRasterPipeline::load_a16;      break;
        case GrColorType::kBGR_565:          *load = SkRasterPipeline::load_565;      break;
        case GrColorType::kABGR_4444:        *load = SkRasterPipeline::load_4444;     break;
        case GrColorType::kRGBA_8888:        *load = SkRasterPipeline::load_8888;     break;
        case GrColorType::kRG_88:            *load = SkRasterPipeline::load_rg88;     break;
        case GrColorType::kRGBA_1010102:     *load = SkRasterPipeline::load_1010102;  break;
        case GrColorType::kAlpha_F16:        *load = SkRasterPipeline::load_af16;     break;
        case GrColorType::kRGBA_F16_Clamped: *load = SkRasterPipeline::load_f16;      break;
        case GrColorType::kRG_1616:          *load = SkRasterPipeline::load_rg1616;   break;
        case GrColorType::kRGBA_16161616:    *load = SkRasterPipeline::load_16161616; break;

        case GrColorType::kRGBA_8888_SRGB:   *load = SkRasterPipeline::load_8888;
                                             *isSRGB = true;
                                             break;
        case GrColorType::kRG_F16:           *load = SkRasterPipeline::load_rgf16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F16:         *load = SkRasterPipeline::load_f16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F32:         *load = SkRasterPipeline::load_f32;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kAlpha_8xxx:       *load = SkRasterPipeline::load_8888;
                                             swizzle = GrSwizzle("000r");
                                             break;
        case GrColorType::kAlpha_F32xxx:     *load = SkRasterPipeline::load_f32;
                                             swizzle = GrSwizzle("000r");
                                             break;
        case GrColorType::kGray_8xxx:       *load = SkRasterPipeline::load_8888;
                                             swizzle = GrSwizzle("rrr1");
                                             break;
        case GrColorType::kGray_8:           *load = SkRasterPipeline::load_a8;
                                             swizzle = GrSwizzle("aaa1");
                                             break;
        case GrColorType::kBGRA_8888:        *load = SkRasterPipeline::load_8888;
                                             swizzle = GrSwizzle("bgra");
                                             break;
        case GrColorType::kRGB_888x:         *load = SkRasterPipeline::load_8888;
                                             swizzle = GrSwizzle("rgb1");
                                             break;

        case GrColorType::kUnknown:
            SK_ABORT("unexpected CT");
    }
    return swizzle;
}

static GrSwizzle get_dst_swizzle_and_store(GrColorType ct, SkRasterPipeline::StockStage* store,
                                           bool* isNormalized, bool* isSRGB) {
    GrSwizzle swizzle("rgba");
    *isNormalized = true;
    *isSRGB = false;
    switch (ct) {
        case GrColorType::kAlpha_8:          *store = SkRasterPipeline::store_a8;       break;
        case GrColorType::kAlpha_16:         *store = SkRasterPipeline::store_a16;      break;
        case GrColorType::kBGR_565:          *store = SkRasterPipeline::store_565;      break;
        case GrColorType::kABGR_4444:        *store = SkRasterPipeline::store_4444;     break;
        case GrColorType::kRGBA_8888:        *store = SkRasterPipeline::store_8888;     break;
        case GrColorType::kRG_88:            *store = SkRasterPipeline::store_rg88;     break;
        case GrColorType::kRGBA_1010102:     *store = SkRasterPipeline::store_1010102;  break;
        case GrColorType::kRGBA_F16_Clamped: *store = SkRasterPipeline::store_f16;      break;
        case GrColorType::kRG_1616:          *store = SkRasterPipeline::store_rg1616;   break;
        case GrColorType::kRGBA_16161616:    *store = SkRasterPipeline::store_16161616; break;

        case GrColorType::kRGBA_8888_SRGB:   *store = SkRasterPipeline::store_8888;
                                             *isSRGB = true;
                                             break;
        case GrColorType::kRG_F16:           *store = SkRasterPipeline::store_rgf16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kAlpha_F16:        *store = SkRasterPipeline::store_af16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F16:         *store = SkRasterPipeline::store_f16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F32:         *store = SkRasterPipeline::store_f32;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kAlpha_8xxx:       *store = SkRasterPipeline::store_8888;
                                             swizzle = GrSwizzle("a000");
                                             break;
        case GrColorType::kAlpha_F32xxx:     *store = SkRasterPipeline::store_f32;
                                             swizzle = GrSwizzle("a000");
                                             break;
        case GrColorType::kBGRA_8888:        swizzle = GrSwizzle("bgra");
                                             *store = SkRasterPipeline::store_8888;
                                             break;
        case GrColorType::kRGB_888x:         swizzle = GrSwizzle("rgb1");
                                             *store = SkRasterPipeline::store_8888;
                                             break;

        case GrColorType::kGray_8:  // not currently supported as output
        case GrColorType::kGray_8xxx:  // not currently supported as output
        case GrColorType::kUnknown:
            SK_ABORT("unexpected CT");
    }
    return swizzle;
}

static inline void append_clamp_gamut(SkRasterPipeline* pipeline) {
    // SkRasterPipeline may not know our color type and also doesn't like caller to directly
    // append clamp_gamut. Fake it out.
    static SkImageInfo fakeII = SkImageInfo::MakeN32Premul(1, 1);
    pipeline->append_gamut_clamp_if_normalized(fakeII);
}

bool GrConvertPixels(const GrImageInfo& dstInfo,       void* dst, size_t dstRB,
                     const GrImageInfo& srcInfo, const void* src, size_t srcRB,
                     bool flipY) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (!srcInfo.isValid() || !dstInfo.isValid()) {
        return false;
    }
    if (!src || !dst) {
        return false;
    }
    if (dstInfo.width() != srcInfo.width() || srcInfo.height() != dstInfo.height()) {
        return false;
    }
    if (GrColorTypeComponentFlags(dstInfo.colorType()) & kGray_SkColorTypeComponentFlag) {
        // We don't currently support conversion to Gray.
        return false;
    }
    if (dstRB < dstInfo.minRowBytes() || srcRB < srcInfo.minRowBytes()) {
        return false;
    }

    size_t srcBpp = srcInfo.bpp();
    size_t dstBpp = dstInfo.bpp();

    // SkRasterPipeline operates on row-pixels not row-bytes.
    SkASSERT(dstRB % dstBpp == 0);
    SkASSERT(srcRB % srcBpp == 0);

    bool premul   = srcInfo.alphaType() == kUnpremul_SkAlphaType &&
                    dstInfo.alphaType() == kPremul_SkAlphaType;
    bool unpremul = srcInfo.alphaType() == kPremul_SkAlphaType &&
                    dstInfo.alphaType() == kUnpremul_SkAlphaType;
    bool alphaOrCSConversion =
            premul || unpremul || !SkColorSpace::Equals(srcInfo.colorSpace(), dstInfo.colorSpace());

    if (srcInfo.colorType() == dstInfo.colorType() && !alphaOrCSConversion) {
        size_t tightRB = dstBpp * dstInfo.width();
        if (flipY) {
            dst = static_cast<char*>(dst) + dstRB * (dstInfo.height() - 1);
            for (int y = 0; y < dstInfo.height(); ++y) {
                memcpy(dst, src, tightRB);
                src = static_cast<const char*>(src) + srcRB;
                dst = static_cast<      char*>(dst) - dstRB;
            }
        } else {
            SkRectMemcpy(dst, dstRB, src, srcRB, tightRB, srcInfo.height());
        }
        return true;
    }

    SkRasterPipeline::StockStage load;
    bool srcIsNormalized;
    bool srcIsSRGB;
    auto loadSwizzle = get_load_and_get_swizzle(srcInfo.colorType(), &load, &srcIsNormalized,
                                                &srcIsSRGB);

    SkRasterPipeline::StockStage store;
    bool dstIsNormalized;
    bool dstIsSRGB;
    auto storeSwizzle = get_dst_swizzle_and_store(dstInfo.colorType(), &store, &dstIsNormalized,
                                                  &dstIsSRGB);

    bool clampGamut;
    SkTLazy<SkColorSpaceXformSteps> steps;
    GrSwizzle loadStoreSwizzle;
    if (alphaOrCSConversion) {
        steps.init(srcInfo.colorSpace(), srcInfo.alphaType(),
                   dstInfo.colorSpace(), dstInfo.alphaType());
        clampGamut = dstIsNormalized && dstInfo.alphaType() == kPremul_SkAlphaType;
    } else {
        clampGamut =
                dstIsNormalized && !srcIsNormalized && dstInfo.alphaType() == kPremul_SkAlphaType;
        if (!clampGamut) {
            loadStoreSwizzle = GrSwizzle::Concat(loadSwizzle, storeSwizzle);
        }
    }
    int cnt = 1;
    int height = srcInfo.height();
    SkRasterPipeline_MemoryCtx srcCtx{const_cast<void*>(src), SkToInt(srcRB / srcBpp)},
                               dstCtx{                  dst , SkToInt(dstRB / dstBpp)};

    if (flipY) {
        // It *almost* works to point the src at the last row and negate the stride and run the
        // whole rectangle. However, SkRasterPipeline::run()'s control loop uses size_t loop
        // variables so it winds up relying on unsigned overflow math. It works out in practice
        // but UBSAN says "no!" as it's technically undefined and in theory a compiler could emit
        // code that didn't do what is intended. So we go one row at a time. :(
        srcCtx.pixels = static_cast<char*>(srcCtx.pixels) + srcRB * (height - 1);
        std::swap(cnt, height);
    }

    bool hasConversion = alphaOrCSConversion || clampGamut;

    if (srcIsSRGB && dstIsSRGB && !hasConversion) {
        // No need to convert from srgb if we are just going to immediately convert it back.
        srcIsSRGB = dstIsSRGB = false;
    }

    hasConversion = hasConversion || srcIsSRGB || dstIsSRGB;

    for (int i = 0; i < cnt; ++i) {
        SkRasterPipeline_<256> pipeline;
        pipeline.append(load, &srcCtx);
        if (hasConversion) {
            loadSwizzle.apply(&pipeline);
            if (srcIsSRGB) {
                pipeline.append(SkRasterPipeline::from_srgb);
            }
            if (alphaOrCSConversion) {
                steps->apply(&pipeline, srcIsNormalized);
            }
            if (clampGamut) {
                append_clamp_gamut(&pipeline);
            }
            // If we add support for storing to Gray we would add a luminance to alpha conversion
            // here. We also wouldn't then need a to_srgb stage after since it would have not effect
            // on the alpha channel. It would also mean we have an SRGB Gray color type which
            // doesn't exist currently.
            if (dstIsSRGB) {
                pipeline.append(SkRasterPipeline::to_srgb);
            }
            storeSwizzle.apply(&pipeline);
        } else {
            loadStoreSwizzle.apply(&pipeline);
        }
        pipeline.append(store, &dstCtx);
        pipeline.run(0, 0, srcInfo.width(), height);
        srcCtx.pixels = static_cast<char*>(srcCtx.pixels) - srcRB;
        dstCtx.pixels = static_cast<char*>(dstCtx.pixels) + dstRB;
    }
    return true;
}

void GrFillInData(GrColorType colorType, int baseWidth, int baseHeight,
                  const SkTArray<size_t>& individualMipOffsets, char* dstPixels,
                  const SkColor4f& color) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    int mipLevels = individualMipOffsets.count();

    int currentWidth = baseWidth;
    int currentHeight = baseHeight;
    for (int currentMipLevel = 0; currentMipLevel < mipLevels; ++currentMipLevel) {
        bool isNormalized;
        bool dstIsSRGB;
        SkRasterPipeline::StockStage store;
        GrSwizzle storeSwizzle = get_dst_swizzle_and_store(colorType, &store, &isNormalized,
                                                           &dstIsSRGB);
        char block[64];
        SkArenaAlloc alloc(block, sizeof(block), 1024);
        SkRasterPipeline_<256> pipeline;
        pipeline.append_constant_color(&alloc, color);
        if (dstIsSRGB) {
            pipeline.append(SkRasterPipeline::to_srgb);
        }
        storeSwizzle.apply(&pipeline);
        size_t offset = individualMipOffsets[currentMipLevel];
        SkRasterPipeline_MemoryCtx  dstCtx{dstPixels + offset, currentWidth};
        pipeline.append(store, &dstCtx);
        pipeline.run(0, 0, currentWidth, currentHeight);

        currentWidth = SkTMax(1, currentWidth / 2);
        currentHeight = SkTMax(1, currentHeight / 2);
    }
}

GrColorType SkColorTypeAndFormatToGrColorType(const GrCaps* caps,
                                              SkColorType skCT,
                                              const GrBackendFormat& format) {
    GrColorType grCT = SkColorTypeToGrColorType(skCT);
    // Until we support SRGB in the SkColorType we have to do this manual check here to make sure
    // we use the correct GrColorType.
    if (caps->isFormatSRGB(format)) {
        if (grCT != GrColorType::kRGBA_8888) {
            return GrColorType::kUnknown;
        }
        grCT = GrColorType::kRGBA_8888_SRGB;
    }
    return grCT;
}
