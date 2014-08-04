/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompressor.h"
#include "SkTextureCompressor_ASTC.h"
#include "SkTextureCompressor_LATC.h"
#include "SkTextureCompressor_R11EAC.h"

#include "SkBitmap.h"
#include "SkData.h"
#include "SkEndian.h"

#include "SkTextureCompression_opts.h"

////////////////////////////////////////////////////////////////////////////////

namespace SkTextureCompressor {

void GetBlockDimensions(Format format, int* dimX, int* dimY, bool matchSpec) {
    if (NULL == dimX || NULL == dimY) {
        return;
    }

    if (!matchSpec && SkTextureCompressorGetPlatformDims(format, dimX, dimY)) {
        return;
    }

    // No specialized arguments, return the dimensions as they are in the spec.
    switch(format) {
        // These formats are 64 bits per 4x4 block.
        default:
            SkDEBUGFAIL("Unknown compression format!");
            // fall through
        case kR11_EAC_Format:
        case kLATC_Format:
            *dimX = 4;
            *dimY = 4;
            break;

        // This format is 12x12 blocks to 128 bits.
        case kASTC_12x12_Format:
            *dimX = 12;
            *dimY = 12;
            break;        
    }
}

int GetCompressedDataSize(Format fmt, int width, int height) {
    int dimX, dimY;
    GetBlockDimensions(fmt, &dimX, &dimY, true);

    int encodedBlockSize = 0;
            
    switch (fmt) {
        // These formats are 64 bits per 4x4 block.
        case kR11_EAC_Format:
        case kLATC_Format:
            encodedBlockSize = 8;
            break;

        // This format is 12x12 blocks to 128 bits.
        case kASTC_12x12_Format:
            encodedBlockSize = 16;
            break;

        default:
            SkFAIL("Unknown compressed format!");
            return -1;
    }

    if(((width % dimX) == 0) && ((height % dimY) == 0)) {
        const int blocksX = width / dimX;
        const int blocksY = height / dimY;

        return blocksX * blocksY * encodedBlockSize;
    }

    return -1;
}

bool CompressBufferToFormat(uint8_t* dst, const uint8_t* src, SkColorType srcColorType,
                            int width, int height, int rowBytes, Format format, bool opt) {
    CompressionProc proc = NULL;
    if (opt) {
        proc = SkTextureCompressorGetPlatformProc(srcColorType, format);
    }

    if (NULL == proc) {
        switch (srcColorType) {
            case kAlpha_8_SkColorType:
            {
                switch (format) {
                    case kLATC_Format:
                        proc = CompressA8ToLATC;
                        break;
                    case kR11_EAC_Format:
                        proc = CompressA8ToR11EAC;
                        break;
                    case kASTC_12x12_Format:
                        proc = CompressA8To12x12ASTC;
                        break;
                    default:
                        // Do nothing...
                        break;
                }
            }
            break;

            default:
                // Do nothing...
                break;
        }
    }

    if (NULL != proc) {
        return proc(dst, src, width, height, rowBytes);
    }

    return false;
}

SkData *CompressBitmapToFormat(const SkBitmap &bitmap, Format format) {
    SkAutoLockPixels alp(bitmap);

    int compressedDataSize = GetCompressedDataSize(format, bitmap.width(), bitmap.height());
    if (compressedDataSize < 0) {
        return NULL;
    }

    const uint8_t* src = reinterpret_cast<const uint8_t*>(bitmap.getPixels());
    uint8_t* dst = reinterpret_cast<uint8_t*>(sk_malloc_throw(compressedDataSize));

    if (CompressBufferToFormat(dst, src, bitmap.colorType(), bitmap.width(), bitmap.height(),
                               bitmap.rowBytes(), format)) {
        return SkData::NewFromMalloc(dst, compressedDataSize);
    }

    sk_free(dst);
    return NULL;
}

SkBlitter* CreateBlitterForFormat(int width, int height, void* compressedBuffer, Format format) {
    switch(format) {
        case kLATC_Format:
            return CreateLATCBlitter(width, height, compressedBuffer);

        case kR11_EAC_Format:
            return CreateR11EACBlitter(width, height, compressedBuffer);

        case kASTC_12x12_Format:
            return CreateASTCBlitter(width, height, compressedBuffer);

        default:
            return NULL;
    }

    return NULL;
}

bool DecompressBufferFromFormat(uint8_t* dst, int dstRowBytes, const uint8_t* src,
                                int width, int height, Format format) {
    int dimX, dimY;
    GetBlockDimensions(format, &dimX, &dimY, true);

    if (width < 0 || ((width % dimX) != 0) || height < 0 || ((height % dimY) != 0)) {
        return false;
    }

    switch(format) {
        case kLATC_Format:
            DecompressLATC(dst, dstRowBytes, src, width, height);
            return true;

        case kR11_EAC_Format:
            DecompressR11EAC(dst, dstRowBytes, src, width, height);
            return true;

        case kASTC_12x12_Format:
            // TODO(krajcevski) .. right now just fall through and return false.
            return false;
    }

    return false;
}

}  // namespace SkTextureCompressor
