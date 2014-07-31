/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompressor_LATC.h"
#include "SkTextureCompressor_Blitter.h"

#include "SkEndian.h"

// Compression options. In general, the slow version is much more accurate, but
// much slower. The fast option is much faster, but much less accurate. YMMV.
#define COMPRESS_LATC_SLOW 0
#define COMPRESS_LATC_FAST 1

////////////////////////////////////////////////////////////////////////////////

#if COMPRESS_LATC_SLOW

////////////////////////////////////////////////////////////////////////////////
//
// Utility Functions
//
////////////////////////////////////////////////////////////////////////////////

// Absolute difference between two values. More correct than SkTAbs(a - b)
// because it works on unsigned values.
template <typename T> inline T abs_diff(const T &a, const T &b) {
    return (a > b) ? (a - b) : (b - a);
}

static bool is_extremal(uint8_t pixel) {
    return 0 == pixel || 255 == pixel;
}

typedef uint64_t (*A84x4To64BitProc)(const uint8_t block[]);

// This function is used by both R11 EAC and LATC to compress 4x4 blocks
// of 8-bit alpha into 64-bit values that comprise the compressed data.
// For both formats, we need to make sure that the dimensions of the 
// src pixels are divisible by 4, and copy 4x4 blocks one at a time
// for compression.
static bool compress_4x4_a8_to_64bit(uint8_t* dst, const uint8_t* src,
                                     int width, int height, int rowBytes,
                                     A84x4To64BitProc proc) {
    // Make sure that our data is well-formed enough to be considered for compression
    if (0 == width || 0 == height || (width % 4) != 0 || (height % 4) != 0) {
        return false;
    }

    int blocksX = width >> 2;
    int blocksY = height >> 2;

    uint8_t block[16];
    uint64_t* encPtr = reinterpret_cast<uint64_t*>(dst);
    for (int y = 0; y < blocksY; ++y) {
        for (int x = 0; x < blocksX; ++x) {
            // Load block
            for (int k = 0; k < 4; ++k) {
                memcpy(block + k*4, src + k*rowBytes + 4*x, 4);
            }

            // Compress it
            *encPtr = proc(block);
            ++encPtr;
        }
        src += 4 * rowBytes;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// LATC compressor
//
////////////////////////////////////////////////////////////////////////////////

// LATC compressed texels down into square 4x4 blocks
static const int kLATCPaletteSize = 8;
static const int kLATCBlockSize = 4;
static const int kLATCPixelsPerBlock = kLATCBlockSize * kLATCBlockSize;

// Generates an LATC palette. LATC constructs
// a palette of eight colors from LUM0 and LUM1 using the algorithm:
//
// LUM0,              if lum0 > lum1 and code(x,y) == 0
// LUM1,              if lum0 > lum1 and code(x,y) == 1
// (6*LUM0+  LUM1)/7, if lum0 > lum1 and code(x,y) == 2
// (5*LUM0+2*LUM1)/7, if lum0 > lum1 and code(x,y) == 3
// (4*LUM0+3*LUM1)/7, if lum0 > lum1 and code(x,y) == 4
// (3*LUM0+4*LUM1)/7, if lum0 > lum1 and code(x,y) == 5
// (2*LUM0+5*LUM1)/7, if lum0 > lum1 and code(x,y) == 6
// (  LUM0+6*LUM1)/7, if lum0 > lum1 and code(x,y) == 7
//
// LUM0,              if lum0 <= lum1 and code(x,y) == 0
// LUM1,              if lum0 <= lum1 and code(x,y) == 1
// (4*LUM0+  LUM1)/5, if lum0 <= lum1 and code(x,y) == 2
// (3*LUM0+2*LUM1)/5, if lum0 <= lum1 and code(x,y) == 3
// (2*LUM0+3*LUM1)/5, if lum0 <= lum1 and code(x,y) == 4
// (  LUM0+4*LUM1)/5, if lum0 <= lum1 and code(x,y) == 5
// 0,                 if lum0 <= lum1 and code(x,y) == 6
// 255,               if lum0 <= lum1 and code(x,y) == 7

static void generate_latc_palette(uint8_t palette[], uint8_t lum0, uint8_t lum1) {
    palette[0] = lum0;
    palette[1] = lum1;
    if (lum0 > lum1) {
        for (int i = 1; i < 7; i++) {
            palette[i+1] = ((7-i)*lum0 + i*lum1) / 7;
        }
    } else {
        for (int i = 1; i < 5; i++) {
            palette[i+1] = ((5-i)*lum0 + i*lum1) / 5;
        }
        palette[6] = 0;
        palette[7] = 255;
    }
}

// Compress a block by using the bounding box of the pixels. It is assumed that
// there are no extremal pixels in this block otherwise we would have used
// compressBlockBBIgnoreExtremal.
static uint64_t compress_latc_block_bb(const uint8_t pixels[]) {
    uint8_t minVal = 255;
    uint8_t maxVal = 0;
    for (int i = 0; i < kLATCPixelsPerBlock; ++i) {
        minVal = SkTMin(pixels[i], minVal);
        maxVal = SkTMax(pixels[i], maxVal);
    }

    SkASSERT(!is_extremal(minVal));
    SkASSERT(!is_extremal(maxVal));

    uint8_t palette[kLATCPaletteSize];
    generate_latc_palette(palette, maxVal, minVal);

    uint64_t indices = 0;
    for (int i = kLATCPixelsPerBlock - 1; i >= 0; --i) {

        // Find the best palette index
        uint8_t bestError = abs_diff(pixels[i], palette[0]);
        uint8_t idx = 0;
        for (int j = 1; j < kLATCPaletteSize; ++j) {
            uint8_t error = abs_diff(pixels[i], palette[j]);
            if (error < bestError) {
                bestError = error;
                idx = j;
            }
        }

        indices <<= 3;
        indices |= idx;
    }

    return
        SkEndian_SwapLE64(
            static_cast<uint64_t>(maxVal) |
            (static_cast<uint64_t>(minVal) << 8) |
            (indices << 16));
}

// Compress a block by using the bounding box of the pixels without taking into
// account the extremal values. The generated palette will contain extremal values
// and fewer points along the line segment to interpolate.
static uint64_t compress_latc_block_bb_ignore_extremal(const uint8_t pixels[]) {
    uint8_t minVal = 255;
    uint8_t maxVal = 0;
    for (int i = 0; i < kLATCPixelsPerBlock; ++i) {
        if (is_extremal(pixels[i])) {
            continue;
        }

        minVal = SkTMin(pixels[i], minVal);
        maxVal = SkTMax(pixels[i], maxVal);
    }

    SkASSERT(!is_extremal(minVal));
    SkASSERT(!is_extremal(maxVal));

    uint8_t palette[kLATCPaletteSize];
    generate_latc_palette(palette, minVal, maxVal);

    uint64_t indices = 0;
    for (int i = kLATCPixelsPerBlock - 1; i >= 0; --i) {

        // Find the best palette index
        uint8_t idx = 0;
        if (is_extremal(pixels[i])) {
            if (0xFF == pixels[i]) {
                idx = 7;
            } else if (0 == pixels[i]) {
                idx = 6;
            } else {
                SkFAIL("Pixel is extremal but not really?!");
            }
        } else {
            uint8_t bestError = abs_diff(pixels[i], palette[0]);
            for (int j = 1; j < kLATCPaletteSize - 2; ++j) {
                uint8_t error = abs_diff(pixels[i], palette[j]);
                if (error < bestError) {
                    bestError = error;
                    idx = j;
                }
            }
        }

        indices <<= 3;
        indices |= idx;
    }

    return
        SkEndian_SwapLE64(
            static_cast<uint64_t>(minVal) |
            (static_cast<uint64_t>(maxVal) << 8) |
            (indices << 16));
}


// Compress LATC block. Each 4x4 block of pixels is decompressed by LATC from two
// values LUM0 and LUM1, and an index into the generated palette. Details of how
// the palette is generated can be found in the comments of generatePalette above.
//
// We choose which palette type to use based on whether or not 'pixels' contains
// any extremal values (0 or 255). If there are extremal values, then we use the
// palette that has the extremal values built in. Otherwise, we use the full bounding
// box.

static uint64_t compress_latc_block(const uint8_t pixels[]) {
    // Collect unique pixels
    int nUniquePixels = 0;
    uint8_t uniquePixels[kLATCPixelsPerBlock];
    for (int i = 0; i < kLATCPixelsPerBlock; ++i) {
        bool foundPixel = false;
        for (int j = 0; j < nUniquePixels; ++j) {
            foundPixel = foundPixel || uniquePixels[j] == pixels[i];
        }

        if (!foundPixel) {
            uniquePixels[nUniquePixels] = pixels[i];
            ++nUniquePixels;
        }
    }

    // If there's only one unique pixel, then our compression is easy.
    if (1 == nUniquePixels) {
        return SkEndian_SwapLE64(pixels[0] | (pixels[0] << 8));

    // Similarly, if there are only two unique pixels, then our compression is
    // easy again: place the pixels in the block header, and assign the indices
    // with one or zero depending on which pixel they belong to.
    } else if (2 == nUniquePixels) {
        uint64_t outBlock = 0;
        for (int i = kLATCPixelsPerBlock - 1; i >= 0; --i) {
            int idx = 0;
            if (pixels[i] == uniquePixels[1]) {
                idx = 1;
            }

            outBlock <<= 3;
            outBlock |= idx;
        }
        outBlock <<= 16;
        outBlock |= (uniquePixels[0] | (uniquePixels[1] << 8));
        return SkEndian_SwapLE64(outBlock);
    }

    // Count non-maximal pixel values
    int nonExtremalPixels = 0;
    for (int i = 0; i < nUniquePixels; ++i) {
        if (!is_extremal(uniquePixels[i])) {
            ++nonExtremalPixels;
        }
    }

    // If all the pixels are nonmaximal then compute the palette using
    // the bounding box of all the pixels.
    if (nonExtremalPixels == nUniquePixels) {
        // This is really just for correctness, in all of my tests we
        // never take this step. We don't lose too much perf here because
        // most of the processing in this function is worth it for the 
        // 1 == nUniquePixels optimization.
        return compress_latc_block_bb(pixels);
    } else {
        return compress_latc_block_bb_ignore_extremal(pixels);
    }
}

#endif  // COMPRESS_LATC_SLOW

////////////////////////////////////////////////////////////////////////////////

#if COMPRESS_LATC_FAST

// Take the top three indices of each int and pack them into the low 12
// bits of the integer.
static inline uint32_t convert_index(uint32_t x) {
    // Since the palette is 
    // 255, 0, 219, 182, 146, 109, 73, 36
    // we need to map the high three bits of each byte in the integer
    // from
    // 0 1 2 3 4 5 6 7
    // to
    // 1 7 6 5 4 3 2 0
    //
    // This first operation takes the mapping from
    // 0 1 2 3 4 5 6 7  -->  7 6 5 4 3 2 1 0
    x = 0x07070707 - ((x >> 5) & 0x07070707);

    // mask is 1 if index is non-zero
    const uint32_t mask = (x | (x >> 1) | (x >> 2)) & 0x01010101;

    // add mask:
    // 7 6 5 4 3 2 1 0 --> 8 7 6 5 4 3 2 0
    x = (x + mask);

    // Handle overflow:
    // 8 7 6 5 4 3 2 0 --> 9 7 6 5 4 3 2 0
    x |= (x >> 3) & 0x01010101;

    // Mask out high bits:
    // 9 7 6 5 4 3 2 0 --> 1 7 6 5 4 3 2 0
    x &= 0x07070707;

    // Pack it in...
#if defined (SK_CPU_BENDIAN)
    return
        (x >> 24) |
        ((x >> 13) & 0x38) |
        ((x >> 2) & 0x1C0) |
        ((x << 9) & 0xE00);
#else
    return
        (x & 0x7) |
        ((x >> 5) & 0x38) |
        ((x >> 10) & 0x1C0) |
        ((x >> 15) & 0xE00);
#endif
}

typedef uint64_t (*PackIndicesProc)(const uint8_t* alpha, int rowBytes);
template<PackIndicesProc packIndicesProc>
static void compress_a8_latc_block(uint8_t** dstPtr, const uint8_t* src, int rowBytes) {
    *(reinterpret_cast<uint64_t*>(*dstPtr)) =
        SkEndian_SwapLE64(0xFF | (packIndicesProc(src, rowBytes) << 16));
    *dstPtr += 8;
}

inline uint64_t PackRowMajor(const uint8_t *indices, int rowBytes) {
    uint64_t result = 0;
    for (int i = 0; i < 4; ++i) {
        const uint32_t idx = *(reinterpret_cast<const uint32_t*>(indices + i*rowBytes));
        result |= static_cast<uint64_t>(convert_index(idx)) << 12*i;
    }
    return result;
}

inline uint64_t PackColumnMajor(const uint8_t *indices, int rowBytes) {
    // !SPEED! Blarg, this is kind of annoying. SSE4 can make this
    // a LOT faster.
    uint8_t transposed[16];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            transposed[j*4+i] = indices[i*rowBytes + j];
        }
    }

    return PackRowMajor(transposed, 4);
}

static bool compress_4x4_a8_latc(uint8_t* dst, const uint8_t* src,
                                 int width, int height, int rowBytes) {

    if (width < 0 || ((width % 4) != 0) || height < 0 || ((height % 4) != 0)) {
        return false;
    }

    uint8_t** dstPtr = &dst;
    for (int y = 0; y < height; y += 4) {
        for (int x = 0; x < width; x += 4) {
            compress_a8_latc_block<PackRowMajor>(dstPtr, src + y*rowBytes + x, rowBytes);
        }
    }

    return true;
}

void CompressA8LATCBlockVertical(uint8_t* dst, const uint8_t block[]) {
    compress_a8_latc_block<PackColumnMajor>(&dst, block, 4);
}

#endif  // COMPRESS_LATC_FAST

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

bool CompressA8ToLATC(uint8_t* dst, const uint8_t* src, int width, int height, int rowBytes) {
#if COMPRESS_LATC_FAST
    return compress_4x4_a8_latc(dst, src, width, height, rowBytes);
#elif COMPRESS_LATC_SLOW
    return compress_4x4_a8_to_64bit(dst, src, width, height, rowBytes, compress_latc_block);
#else
#error "Must choose either fast or slow LATC compression"
#endif
}

SkBlitter* CreateLATCBlitter(int width, int height, void* outputBuffer) {
#if COMPRESS_LATC_FAST
    return new
        SkTCompressedAlphaBlitter<4, 8, CompressA8LATCBlockVertical>
        (width, height, outputBuffer);
#elif COMPRESS_LATC_SLOW
    // TODO (krajcevski)
    return NULL;
#endif
}

}  // SkTextureCompressor
