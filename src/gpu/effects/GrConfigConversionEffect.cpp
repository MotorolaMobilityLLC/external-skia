/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrConfigConversionEffect.fp; do not modify.
 **************************************************************************************************/
#include "GrConfigConversionEffect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrTexture.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLConfigConversionEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLConfigConversionEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrConfigConversionEffect& _outer = args.fFp.cast<GrConfigConversionEffect>();
        (void)_outer;
        auto pmConversion = _outer.pmConversion();
        (void)pmConversion;

        fragBuilder->forceHighPrecision();
        fragBuilder->codeAppendf(
                "%s = floor(%s * 255.0 + 0.5) / 255.0;\n@switch (%d) {\n    case 0:\n        "
                "%s.xyz = floor((%s.xyz * %s.w) * 255.0 + 0.5) / 255.0;\n        break;\n    case "
                "1:\n        %s.xyz = %s.w <= 0.0 ? half3(0.0) : floor((%s.xyz / %s.w) * 255.0 + "
                "0.5) / 255.0;\n        break;\n}\n",
                args.fOutputColor, args.fInputColor, (int)_outer.pmConversion(), args.fOutputColor,
                args.fOutputColor, args.fOutputColor, args.fOutputColor, args.fOutputColor,
                args.fOutputColor, args.fOutputColor);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {}
};
GrGLSLFragmentProcessor* GrConfigConversionEffect::onCreateGLSLInstance() const {
    return new GrGLSLConfigConversionEffect();
}
void GrConfigConversionEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                     GrProcessorKeyBuilder* b) const {
    b->add32((int32_t)fPmConversion);
}
bool GrConfigConversionEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrConfigConversionEffect& that = other.cast<GrConfigConversionEffect>();
    (void)that;
    if (fPmConversion != that.fPmConversion) return false;
    return true;
}
GrConfigConversionEffect::GrConfigConversionEffect(const GrConfigConversionEffect& src)
        : INHERITED(kGrConfigConversionEffect_ClassID, src.optimizationFlags())
        , fPmConversion(src.fPmConversion) {}
std::unique_ptr<GrFragmentProcessor> GrConfigConversionEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrConfigConversionEffect(*this));
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConfigConversionEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrConfigConversionEffect::TestCreate(
        GrProcessorTestData* data) {
    PMConversion pmConv = static_cast<PMConversion>(
            data->fRandom->nextULessThan((int)PMConversion::kPMConversionCnt));
    return std::unique_ptr<GrFragmentProcessor>(new GrConfigConversionEffect(pmConv));
}
#endif
