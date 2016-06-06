/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_Base_DEFINED
#define SkColorSpace_Base_DEFINED

#include "SkColorSpace.h"
#include "SkData.h"
#include "SkTemplates.h"

struct SkGammaCurve {
    bool isValue() const {
        bool result = (0.0f != fValue);
        SkASSERT(!result || (0 == fTableSize));
        SkASSERT(!result || (0.0f == fG && 0.0f == fE));
        return result;
    }

    bool isTable() const {
        bool result = (0 != fTableSize);
        SkASSERT(!result || (0.0f == fValue));
        SkASSERT(!result || (0.0f == fG && 0.0f == fE));
        SkASSERT(!result || fTable);
        return result;
    }

    bool isParametric() const {
        bool result = (0.0f != fG || 0.0f != fE);
        SkASSERT(!result || (0.0f == fValue));
        SkASSERT(!result || (0 == fTableSize));
        return result;
    }

    // We have three different ways to represent gamma.
    // (1) A single value:
    float                    fValue;

    // (2) A lookup table:
    uint32_t                 fTableSize;
    std::unique_ptr<float[]> fTable;

    // (3) Parameters for a curve:
    //     Y = (aX + b)^g + c  for X >= d
    //     Y = eX + f          otherwise
    float                    fG;
    float                    fA;
    float                    fB;
    float                    fC;
    float                    fD;
    float                    fE;
    float                    fF;

    SkGammaCurve() {
        memset(this, 0, sizeof(struct SkGammaCurve));
    }

    SkGammaCurve(float value)
        : fValue(value)
        , fTableSize(0)
        , fTable(nullptr)
        , fG(0.0f)
        , fA(0.0f)
        , fB(0.0f)
        , fC(0.0f)
        , fD(0.0f)
        , fE(0.0f)
        , fF(0.0f)
    {}
};

struct SkGammas : public SkRefCnt {
public:
    bool isValues() const {
        return fRed.isValue() && fGreen.isValue() && fBlue.isValue();
    }

    const SkGammaCurve fRed;
    const SkGammaCurve fGreen;
    const SkGammaCurve fBlue;

    SkGammas(float red, float green, float blue)
        : fRed(red)
        , fGreen(green)
        , fBlue(blue)
    {}

    SkGammas(SkGammaCurve red, SkGammaCurve green, SkGammaCurve blue)
        : fRed(std::move(red))
        , fGreen(std::move(green))
        , fBlue(std::move(blue))
    {}

    SkGammas() {}

    friend class SkColorSpace;
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
};

class SkColorSpace_Base : public SkColorSpace {
public:

    const sk_sp<SkGammas>& gammas() const { return fGammas; }

    /**
     *  Writes this object as an ICC profile.
     */
    sk_sp<SkData> writeToICC() const;

private:

    static sk_sp<SkColorSpace> NewRGB(const float gammas[3], const SkMatrix44& toXYZD50,
                                      sk_sp<SkData> profileData);

    SkColorSpace_Base(sk_sp<SkGammas> gammas, const SkMatrix44& toXYZ, Named,
                      sk_sp<SkData> profileData);

    SkColorSpace_Base(sk_sp<SkGammas> gammas, GammaNamed gammaNamed, const SkMatrix44& toXYZ,
                      Named, sk_sp<SkData> profileData);

    SkColorSpace_Base(SkColorLookUpTable* colorLUT, sk_sp<SkGammas> gammas,
                      const SkMatrix44& toXYZ, sk_sp<SkData> profileData);

    SkAutoTDelete<SkColorLookUpTable> fColorLUT;
    sk_sp<SkGammas>                   fGammas;
    sk_sp<SkData>                     fProfileData;

    friend class SkColorSpace;
    typedef SkColorSpace INHERITED;
};

static inline SkColorSpace_Base* as_CSB(SkColorSpace* colorSpace) {
    return static_cast<SkColorSpace_Base*>(colorSpace);
}

static inline const SkColorSpace_Base* as_CSB(const SkColorSpace* colorSpace) {
    return static_cast<const SkColorSpace_Base*>(colorSpace);
}

static inline SkColorSpace_Base* as_CSB(const sk_sp<SkColorSpace>& colorSpace) {
    return static_cast<SkColorSpace_Base*>(colorSpace.get());
}

#endif
