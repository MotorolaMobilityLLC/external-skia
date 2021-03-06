/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrConfigConversionEffect_DEFINED
#define GrConfigConversionEffect_DEFINED

#include "GrSingleTextureEffect.h"

class GrInvariantOutput;

/**
 * This class is used to perform config conversions. Clients may want to read/write data that is
 * unpremultiplied.
 */
class GrConfigConversionEffect : public GrSingleTextureEffect {
public:
    /**
     * The PM->UPM or UPM->PM conversions to apply.
     */
    enum PMConversion {
        kMulByAlpha_RoundUp_PMConversion = 0,
        kMulByAlpha_RoundDown_PMConversion,
        kDivByAlpha_RoundUp_PMConversion,
        kDivByAlpha_RoundDown_PMConversion,

        kPMConversionCnt
    };

    static sk_sp<GrFragmentProcessor> Make(GrTexture*, PMConversion, const SkMatrix&);

    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider*, sk_sp<GrTextureProxy>,
                                           PMConversion, const SkMatrix&);

    const char* name() const override { return "Config Conversion"; }

    PMConversion  pmConversion() const { return fPMConversion; }

    // This function determines whether it is possible to choose PM->UPM and UPM->PM conversions
    // for which in any PM->UPM->PM->UPM sequence the two UPM values are the same. This means that
    // if pixels are read back to a UPM buffer, written back to PM to the GPU, and read back again
    // both reads will produce the same result. This test is quite expensive and should not be run
    // multiple times for a given context.
    static void TestForPreservingPMConversions(GrContext* context,
                                               PMConversion* PMToUPMRule,
                                               PMConversion* UPMToPMRule);
private:
    GrConfigConversionEffect(GrTexture*, PMConversion, const SkMatrix& matrix);

    GrConfigConversionEffect(GrResourceProvider*, sk_sp<GrTextureProxy>,
                             PMConversion, const SkMatrix& matrix);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    PMConversion    fPMConversion;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
