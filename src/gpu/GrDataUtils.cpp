/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDataUtils.h"

#include "include/private/GrColor.h"
#include "src/core/SkUtils.h"

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

static int num_ETC1_blocks(int w, int h) {
    if (w < 4) {
        w = 1;
    } else {
       SkASSERT((w & 3) == 0);
       w >>= 2;
    }

    if (h < 4) {
        h = 1;
    } else {
       SkASSERT((h & 3) == 0);
       h >>= 2;
    }

    return w * h;
}

size_t GrETC1CompressedDataSize(int width, int height) {
    int numBlocks = num_ETC1_blocks(width, height);

    return numBlocks * sizeof(ETC1Block);
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

// Fill in the width x height 'dest' with the munged version of 'colorf' that matches 'config'
static bool fill_buffer_with_color(GrPixelConfig config, int width, int height,
                                   const SkColor4f& colorf, void* dest) {
    SkASSERT(kRGB_ETC1_GrPixelConfig != config);

    GrColor color = colorf.toBytes_RGBA();

    uint8_t r = GrColorUnpackR(color);
    uint8_t g = GrColorUnpackG(color);
    uint8_t b = GrColorUnpackB(color);
    uint8_t a = GrColorUnpackA(color);

    switch (config) {
        case kAlpha_8_GrPixelConfig:                            // fall through
        case kAlpha_8_as_Alpha_GrPixelConfig:                   // fall through
        case kAlpha_8_as_Red_GrPixelConfig: {
            memset(dest, a, width * height);
            break;
        }
        case kGray_8_GrPixelConfig:                             // fall through
        case kGray_8_as_Lum_GrPixelConfig:                      // fall through
        case kGray_8_as_Red_GrPixelConfig: {
            uint8_t gray8 = SkComputeLuminance(r, g, b);

            memset(dest, gray8, width * height);
            break;
        }
        case kRGB_565_GrPixelConfig: {
            uint16_t rgb565 = SkPack888ToRGB16(r, g, b);

            sk_memset16((uint16_t*) dest, rgb565, width * height);
            break;
        }
        case kRGBA_4444_GrPixelConfig: {
            uint8_t r4 = (r >> 4) & 0xF;
            uint8_t g4 = (g >> 4) & 0xF;
            uint8_t b4 = (b >> 4) & 0xF;
            uint8_t a4 = (a >> 4) & 0xF;

            uint16_t rgba4444 = r4 << SK_R4444_SHIFT | g4 << SK_G4444_SHIFT |
                                b4 << SK_B4444_SHIFT | a4 << SK_A4444_SHIFT;

            sk_memset16((uint16_t*) dest, rgba4444, width * height);
            break;
        }
        case kRGBA_8888_GrPixelConfig: {
            sk_memset32((uint32_t *) dest, color, width * height);
            break;
        }
        case kRGB_888_GrPixelConfig: {
            uint8_t* dest8 = (uint8_t*) dest;
            for (int i = 0; i < width * height; ++i, dest8 += 3) {
                dest8[0] = r;
                dest8[1] = g;
                dest8[2] = b;
            }
            break;
        }
        case kRGB_888X_GrPixelConfig: {
            GrColor opaque = GrColorPackRGBA(r, g, b, 0xFF);

            sk_memset32((uint32_t *) dest, opaque, width * height);
            break;
        }
        case kRG_88_GrPixelConfig: {
            uint16_t rg88 = (r << 8) | g;

            sk_memset16((uint16_t*) dest, rg88, width * height);
            break;
        }
        case kBGRA_8888_GrPixelConfig: {
            GrColor swizzled = GrColorPackRGBA(b, g, r, a);

            sk_memset32((uint32_t *) dest, swizzled, width * height);
            break;
        }
        case kSRGBA_8888_GrPixelConfig: {
            sk_memset32((uint32_t *) dest, color, width * height);
            break;
        }
        case kSBGRA_8888_GrPixelConfig: {
            GrColor swizzled = GrColorPackRGBA(b, g, r, a);

            sk_memset32((uint32_t *) dest, swizzled, width * height);
            break;
        }
        case kRGBA_1010102_GrPixelConfig: {
            uint32_t r10 = SkScalarRoundToInt(colorf.fR * 1023.0f);
            uint32_t g10 = SkScalarRoundToInt(colorf.fG * 1023.0f);
            uint32_t b10 = SkScalarRoundToInt(colorf.fB * 1023.0f);
            uint8_t  a2  = SkScalarRoundToInt(colorf.fA * 3.0f);

            uint32_t rgba1010102 = a2 << 30 | b10 << 20 | g10 << 10 | r10;

            sk_memset32((uint32_t *) dest, rgba1010102, width * height);
            break;
        }
        case kRGBA_float_GrPixelConfig: {
            SkColor4f* destColor = (SkColor4f*) dest;
            for (int i = 0; i < width * height; ++i) {
                destColor[i] = colorf;
            }
            break;
        }
        case kRG_float_GrPixelConfig: {
            float* destFloat = (float*) dest;
            for (int i = 0; i < width * height; ++i, destFloat += 2) {
                destFloat[0] = colorf.fR;
                destFloat[1] = colorf.fG;
            }
            break;
        }
        case kAlpha_half_as_Red_GrPixelConfig:                  // fall through
        case kAlpha_half_GrPixelConfig: {
            SkHalf alphaHalf = SkFloatToHalf(colorf.fA);

            sk_memset16((uint16_t *) dest, alphaHalf, width * height);
            break;
        }
        case kRGBA_half_GrPixelConfig:                          // fall through
        case kRGBA_half_Clamped_GrPixelConfig: {
            uint64_t rHalf = SkFloatToHalf(colorf.fR);
            uint64_t gHalf = SkFloatToHalf(colorf.fG);
            uint64_t bHalf = SkFloatToHalf(colorf.fB);
            uint64_t aHalf = SkFloatToHalf(colorf.fA);

            uint64_t rgbaHalf = (aHalf << 48) | (bHalf << 32) | (gHalf << 16) | rHalf;

            sk_memset64((uint64_t *) dest, rgbaHalf, width * height);
            break;
        }
        default:
            return false;
            break;
    }

    return true;
}

size_t GrComputeTightCombinedBufferSize(GrCompression compression, size_t bytesPerPixel,
                                        int baseWidth, int baseHeight,
                                        SkTArray<size_t>* individualMipOffsets,
                                        int mipLevelCount) {
    SkASSERT(individualMipOffsets && !individualMipOffsets->count());
    SkASSERT(mipLevelCount >= 1);

    individualMipOffsets->push_back(0);

    size_t combinedBufferSize = baseWidth * bytesPerPixel * baseHeight;
    if (GrCompression::kETC1 == compression) {
        SkASSERT(0 == bytesPerPixel);
        bytesPerPixel = 4; // munge Bpp to make the following code work (and not assert)
        combinedBufferSize = GrETC1CompressedDataSize(baseWidth, baseHeight);
    }

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

        size_t trimmedSize;
        if (GrCompression::kETC1 == compression) {
            trimmedSize = GrETC1CompressedDataSize(currentWidth, currentHeight);
        } else {
            trimmedSize = currentWidth * bytesPerPixel * currentHeight;
        }
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

void GrFillInData(GrCompression compression, GrPixelConfig config,
                  int baseWidth, int baseHeight,
                  const SkTArray<size_t>& individualMipOffsets, char* dstPixels,
                  const SkColor4f& colorf) {

    int mipLevels = individualMipOffsets.count();

    int currentWidth = baseWidth;
    int currentHeight = baseHeight;
    for (int currentMipLevel = 0; currentMipLevel < mipLevels; ++currentMipLevel) {
        size_t offset = individualMipOffsets[currentMipLevel];

        if (GrCompression::kETC1 == compression) {
            // TODO: compute the ETC1 block for 'colorf' just once
            fillin_ETC1_with_color(currentWidth, currentHeight, colorf, &(dstPixels[offset]));
        } else {
            fill_buffer_with_color(config, currentWidth, currentHeight, colorf,
                                   &(dstPixels[offset]));
        }

        currentWidth = SkTMax(1, currentWidth / 2);
        currentHeight = SkTMax(1, currentHeight / 2);
    }
}

