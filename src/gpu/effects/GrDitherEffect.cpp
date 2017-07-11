/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This file was autogenerated from GrDitherEffect.fp; do not modify.
 */
#include "GrDitherEffect.h"
#if SK_SUPPORT_GPU
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLDitherEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLDitherEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrDitherEffect& _outer = args.fFp.cast<GrDitherEffect>();
        (void) _outer;
        fragBuilder->codeAppendf("float r = fract(sin(dot(sk_FragCoord.xy, vec2(12.989800000000001, 78.233000000000004))) * 43758.545299999998);\n%s = clamp(0.0039215686274509803 * vec4(r) + %s, 0.0, 1.0);\n", args.fOutputColor, args.fInputColor ? args.fInputColor : "vec4(1)");
    }
private:
    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& _proc) override {
    }
};
GrGLSLFragmentProcessor* GrDitherEffect::onCreateGLSLInstance() const {
    return new GrGLSLDitherEffect();
}
void GrDitherEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
}
bool GrDitherEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrDitherEffect& that = other.cast<GrDitherEffect>();
    (void) that;
    return true;
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDitherEffect);
#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrDitherEffect::TestCreate(GrProcessorTestData* testData) {

    return GrDitherEffect::Make();
}
#endif
#endif
