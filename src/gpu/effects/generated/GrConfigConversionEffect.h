/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrConfigConversionEffect.fp; do not modify.
 **************************************************************************************************/
#ifndef GrConfigConversionEffect_DEFINED
#define GrConfigConversionEffect_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkTypes.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrSurfaceDrawContext.h"

#include "src/gpu/GrFragmentProcessor.h"

class GrConfigConversionEffect : public GrFragmentProcessor {
public:
    static bool TestForPreservingPMConversions(GrDirectContext* dContext);

    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                     PMConversion pmConversion) {
        if (!fp) {
            return nullptr;
        }
        return std::unique_ptr<GrFragmentProcessor>(
                new GrConfigConversionEffect(std::move(fp), pmConversion));
    }
    GrConfigConversionEffect(const GrConfigConversionEffect& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "ConfigConversionEffect"; }
    PMConversion pmConversion;

private:
    GrConfigConversionEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                             PMConversion pmConversion)
            : INHERITED(kGrConfigConversionEffect_ClassID, kNone_OptimizationFlags)
            , pmConversion(pmConversion) {
        this->registerChild(std::move(inputFP), SkSL::SampleUsage::PassThrough());
    }
    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
#if GR_TEST_UTILS
    SkString onDumpInfo() const override;
#endif
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    using INHERITED = GrFragmentProcessor;
};
#endif
