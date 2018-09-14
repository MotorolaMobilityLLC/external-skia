/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrTextureGradientColorizer.fp; do not modify.
 **************************************************************************************************/
#include "GrTextureGradientColorizer.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrTexture.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLTextureGradientColorizer : public GrGLSLFragmentProcessor {
public:
    GrGLSLTextureGradientColorizer() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrTextureGradientColorizer& _outer = args.fFp.cast<GrTextureGradientColorizer>();
        (void)_outer;
        fragBuilder->codeAppendf(
                "half2 coord = half2(%s.x, 0.5);\n%s = texture(%s, float2(coord)).%s;\n",
                args.fInputColor, args.fOutputColor,
                fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]).c_str(),
                fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str());
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {}
};
GrGLSLFragmentProcessor* GrTextureGradientColorizer::onCreateGLSLInstance() const {
    return new GrGLSLTextureGradientColorizer();
}
void GrTextureGradientColorizer::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                       GrProcessorKeyBuilder* b) const {}
bool GrTextureGradientColorizer::onIsEqual(const GrFragmentProcessor& other) const {
    const GrTextureGradientColorizer& that = other.cast<GrTextureGradientColorizer>();
    (void)that;
    if (fGradient != that.fGradient) return false;
    return true;
}
GrTextureGradientColorizer::GrTextureGradientColorizer(const GrTextureGradientColorizer& src)
        : INHERITED(kGrTextureGradientColorizer_ClassID, src.optimizationFlags())
        , fGradient(src.fGradient) {
    this->setTextureSamplerCnt(1);
}
std::unique_ptr<GrFragmentProcessor> GrTextureGradientColorizer::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrTextureGradientColorizer(*this));
}
const GrFragmentProcessor::TextureSampler& GrTextureGradientColorizer::onTextureSampler(
        int index) const {
    return IthTextureSampler(index, fGradient);
}
