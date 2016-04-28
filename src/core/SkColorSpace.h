/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_DEFINED
#define SkColorSpace_DEFINED

// Some terms
//
//  PCS : Profile Connection Space : where color number values have an absolute meaning.
//        Part of the work float is to convert colors to and from this space...
//        src_linear_unit_floats --> PCS --> PCS' --> dst_linear_unit_floats
//
// Some nice documents
//
// http://www.cambridgeincolour.com/tutorials/color-space-conversion.htm
// https://www.w3.org/Graphics/Color/srgb
// http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html
//

#include "SkRefCnt.h"

struct SkFloat3 {
    float fVec[3];

    void dump() const;
};

struct SkFloat3x3 {
    float fMat[9];

    void dump() const;
};

struct SkColorLookUpTable {
    static const uint8_t kMaxChannels = 16;

    uint8_t                  fInputChannels;
    uint8_t                  fOutputChannels;
    uint8_t                  fGridPoints[kMaxChannels];
    std::unique_ptr<float[]> fTable;

    SkColorLookUpTable() {
        memset(this, 0, sizeof(struct SkColorLookUpTable));
    }

    SkColorLookUpTable(SkColorLookUpTable&& that)
        : fInputChannels(that.fInputChannels)
        , fOutputChannels(that.fOutputChannels)
        , fTable(std::move(that.fTable))
    {
        memcpy(fGridPoints, that.fGridPoints, kMaxChannels);
    }
};

class SkColorSpace : public SkRefCnt {
public:
    enum Named {
        kUnknown_Named,
        kSRGB_Named,
    };

    /**
     *  Return a colorspace instance, given a 3x3 transform from linear_RGB to D50_XYZ
     *  and the src-gamma, return a ColorSpace
     */
    static sk_sp<SkColorSpace> NewRGB(const SkFloat3x3& toXYZD50, const SkFloat3& gamma);

    static sk_sp<SkColorSpace> NewNamed(Named);
    static sk_sp<SkColorSpace> NewICC(const void*, size_t);

    SkFloat3 gamma() const { return fGamma; }
    SkFloat3x3 xyz() const { return fToXYZD50; }
    SkFloat3 xyzOffset() const { return fToXYZOffset; }
    Named named() const { return fNamed; }
    uint32_t uniqueID() const { return fUniqueID; }

private:
    SkColorSpace(const SkFloat3& gamma, const SkFloat3x3& toXYZ, Named);

    SkColorSpace(SkColorLookUpTable colorLUT, const SkFloat3& gamma, const SkFloat3x3& toXYZ,
                 const SkFloat3& toXYZOffset);

    const SkColorLookUpTable fColorLUT;
    const SkFloat3           fGamma;
    const SkFloat3x3         fToXYZD50;
    const SkFloat3           fToXYZOffset;

    const uint32_t           fUniqueID;
    const Named              fNamed;
};

#endif
