/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrMixerEffect.fp; do not modify.
 **************************************************************************************************/
#include "GrMixerEffect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrTexture.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLMixerEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLMixerEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrMixerEffect& _outer = args.fFp.cast<GrMixerEffect>();
        (void)_outer;
        auto weight = _outer.weight();
        (void)weight;
        fWeightVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                      kDefault_GrSLPrecision, "weight");
        SkString _input0 = SkStringPrintf("%s", args.fInputColor);
        SkString _child0("_child0");
        this->emitChild(_outer.fp0_index(), _input0.c_str(), &_child0, args);
        fragBuilder->codeAppendf("half4 in0 = %s;", _child0.c_str());
        SkString _input1 = SkStringPrintf("%s", args.fInputColor);
        SkString _child1("_child1");
        if (_outer.fp1_index() >= 0) {
            this->emitChild(_outer.fp1_index(), _input1.c_str(), &_child1, args);
        } else {
            fragBuilder->codeAppendf("half4 %s;", _child1.c_str());
        }
        fragBuilder->codeAppendf("\nhalf4 in1 = %s ? %s : %s;\n%s = mix(in0, in1, %s);\n",
                                 _outer.fp1_index() >= 0 ? "true" : "false", _child1.c_str(),
                                 args.fInputColor, args.fOutputColor,
                                 args.fUniformHandler->getUniformCStr(fWeightVar));
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrMixerEffect& _outer = _proc.cast<GrMixerEffect>();
        { pdman.set1f(fWeightVar, (_outer.weight())); }
    }
    UniformHandle fWeightVar;
};
GrGLSLFragmentProcessor* GrMixerEffect::onCreateGLSLInstance() const {
    return new GrGLSLMixerEffect();
}
void GrMixerEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                          GrProcessorKeyBuilder* b) const {}
bool GrMixerEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrMixerEffect& that = other.cast<GrMixerEffect>();
    (void)that;
    if (fWeight != that.fWeight) return false;
    return true;
}
GrMixerEffect::GrMixerEffect(const GrMixerEffect& src)
        : INHERITED(kGrMixerEffect_ClassID, src.optimizationFlags())
        , fFp0_index(src.fFp0_index)
        , fFp1_index(src.fFp1_index)
        , fWeight(src.fWeight) {
    this->registerChildProcessor(src.childProcessor(fFp0_index).clone());
    if (fFp1_index >= 0) {
        this->registerChildProcessor(src.childProcessor(fFp1_index).clone());
    }
}
std::unique_ptr<GrFragmentProcessor> GrMixerEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrMixerEffect(*this));
}
