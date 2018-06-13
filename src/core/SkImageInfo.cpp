/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageInfoPriv.h"
#include "SkSafeMath.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

int SkColorTypeBytesPerPixel(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return 0;
        case kAlpha_8_SkColorType:      return 1;
        case kRGB_565_SkColorType:      return 2;
        case kARGB_4444_SkColorType:    return 2;
        case kRGBA_8888_SkColorType:    return 4;
        case kBGRA_8888_SkColorType:    return 4;
        case kRGB_888x_SkColorType:     return 4;
        case kRGBA_1010102_SkColorType: return 4;
        case kRGB_101010x_SkColorType:  return 4;
        case kGray_8_SkColorType:       return 1;
        case kRGBA_F16_SkColorType:     return 8;
    }
    return 0;
}

// These values must be constant over revisions, though they can be renamed to reflect if/when
// they are deprecated.
enum Stored_SkColorType {
    kUnknown_Stored_SkColorType             = 0,
    kAlpha_8_Stored_SkColorType             = 1,
    kRGB_565_Stored_SkColorType             = 2,
    kARGB_4444_Stored_SkColorType           = 3,
    kRGBA_8888_Stored_SkColorType           = 4,
    kBGRA_8888_Stored_SkColorType           = 5,
    kIndex_8_Stored_SkColorType_DEPRECATED  = 6,
    kGray_8_Stored_SkColorType              = 7,
    kRGBA_F16_Stored_SkColorType            = 8,
    kRGB_888x_Stored_SkColorType            = 9,
    kRGBA_1010102_Stored_SkColorType        = 10,
    kRGB_101010x_Stored_SkColorType         = 11,
};

bool SkColorTypeIsAlwaysOpaque(SkColorType ct) {
    return !(kAlpha_SkColorTypeComponentFlag & SkColorTypeComponentFlags(ct));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int SkImageInfo::bytesPerPixel() const { return SkColorTypeBytesPerPixel(fColorType); }

int SkImageInfo::shiftPerPixel() const { return SkColorTypeShiftPerPixel(fColorType); }

size_t SkImageInfo::computeOffset(int x, int y, size_t rowBytes) const {
    SkASSERT((unsigned)x < (unsigned)fWidth);
    SkASSERT((unsigned)y < (unsigned)fHeight);
    return SkColorTypeComputeOffset(fColorType, x, y, rowBytes);
}

size_t SkImageInfo::computeByteSize(size_t rowBytes) const {
    if (0 == fHeight) {
        return 0;
    }
    SkSafeMath safe;
    size_t bytes = safe.add(safe.mul(safe.addInt(fHeight, -1), rowBytes),
                            safe.mul(fWidth, this->bytesPerPixel()));
    return safe ? bytes : SIZE_MAX;
}

SkImageInfo SkImageInfo::MakeS32(int width, int height, SkAlphaType at) {
    return SkImageInfo(width, height, kN32_SkColorType, at,
                       SkColorSpace::MakeSRGB());
}

bool SkColorTypeValidateAlphaType(SkColorType colorType, SkAlphaType alphaType,
                                  SkAlphaType* canonical) {
    switch (colorType) {
        case kUnknown_SkColorType:
            alphaType = kUnknown_SkAlphaType;
            break;
        case kAlpha_8_SkColorType:
            if (kUnpremul_SkAlphaType == alphaType) {
                alphaType = kPremul_SkAlphaType;
            }
            // fall-through
        case kARGB_4444_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_1010102_SkColorType:
        case kRGBA_F16_SkColorType:
            if (kUnknown_SkAlphaType == alphaType) {
                return false;
            }
            break;
        case kGray_8_SkColorType:
        case kRGB_565_SkColorType:
        case kRGB_888x_SkColorType:
        case kRGB_101010x_SkColorType:
            alphaType = kOpaque_SkAlphaType;
            break;
        default:
            return false;
    }
    if (canonical) {
        *canonical = alphaType;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkReadPixelsRec.h"

bool SkReadPixelsRec::trim(int srcWidth, int srcHeight) {
    if (nullptr == fPixels || fRowBytes < fInfo.minRowBytes()) {
        return false;
    }
    if (0 >= fInfo.width() || 0 >= fInfo.height()) {
        return false;
    }

    int x = fX;
    int y = fY;
    SkIRect srcR = SkIRect::MakeXYWH(x, y, fInfo.width(), fInfo.height());
    if (!srcR.intersect(0, 0, srcWidth, srcHeight)) {
        return false;
    }

    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    // we negate and add them so UBSAN (pointer-overflow) doesn't get confused.
    fPixels = ((char*)fPixels + -y*fRowBytes + -x*fInfo.bytesPerPixel());
    // the intersect may have shrunk info's logical size
    fInfo = fInfo.makeWH(srcR.width(), srcR.height());
    fX = srcR.x();
    fY = srcR.y();

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkWritePixelsRec.h"

bool SkWritePixelsRec::trim(int dstWidth, int dstHeight) {
    if (nullptr == fPixels || fRowBytes < fInfo.minRowBytes()) {
        return false;
    }
    if (0 >= fInfo.width() || 0 >= fInfo.height()) {
        return false;
    }

    int x = fX;
    int y = fY;
    SkIRect dstR = SkIRect::MakeXYWH(x, y, fInfo.width(), fInfo.height());
    if (!dstR.intersect(0, 0, dstWidth, dstHeight)) {
        return false;
    }

    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    // we negate and add them so UBSAN (pointer-overflow) doesn't get confused.
    fPixels = ((const char*)fPixels + -y*fRowBytes + -x*fInfo.bytesPerPixel());
    // the intersect may have shrunk info's logical size
    fInfo = fInfo.makeWH(dstR.width(), dstR.height());
    fX = dstR.x();
    fY = dstR.y();

    return true;
}
