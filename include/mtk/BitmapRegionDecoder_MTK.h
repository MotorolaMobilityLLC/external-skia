/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BitmapRegionDecoder_MTK_DEFINED
#define BitmapRegionDecoder_MTK_DEFINED

#include "client_utils/android/BitmapRegionDecoder.h"
#include "SkImageDecoder.h"

namespace android {
namespace skia {

class BitmapRegionDecoder_MTK: public BitmapRegionDecoder {
public:
   BitmapRegionDecoder_MTK(sk_sp<SkData> data, SkImageDecoder *decoder)
    : BitmapRegionDecoder(std::move(SkAndroidCodec::MakeFromData(std::move(data)))),
      fDecoder(decoder)
    {
        setVendorExt(true);
    }

    ~BitmapRegionDecoder_MTK()
    {
        if (fDecoder) delete fDecoder;
    }

    bool decodeRegion(SkBitmap* bitmap, BRDAllocator* allocator,
        const SkIRect& desiredSubset, int sampleSize, SkColorType dstColorType,
        bool requireUnpremul, sk_sp<SkColorSpace> dstColorSpace) {
            fDecoder->setSampleSize(sampleSize);
        return fDecoder->decodeSubset(bitmap, allocator, desiredSubset, dstColorType, sampleSize, dstColorSpace);
    }

    SkColorType computeOutputColorType(SkColorType requestedColorType) {
        return fDecoder->computeOutputColorType(requestedColorType);
    }

    sk_sp<SkColorSpace> computeOutputColorSpace(SkColorType outputColorType,
        sk_sp<SkColorSpace> prefColorSpace = nullptr) {
        return fDecoder->computeOutputColorSpace(outputColorType, prefColorSpace);
    }

    SkImageDecoder* getDecoder() const { return fDecoder; }

private:
    SkImageDecoder* fDecoder;
};

} // namespace skia
} // namespace android
#endif  // BitmapRegionDecoder_MTK_DEFINED

