/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrSingleIntervalGradientColorizer.fp; do not modify.
 **************************************************************************************************/
#include "GrSingleIntervalGradientColorizer.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrTexture.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLSingleIntervalGradientColorizer : public GrGLSLFragmentProcessor {
public:
    GrGLSLSingleIntervalGradientColorizer() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrSingleIntervalGradientColorizer& _outer =
                args.fFp.cast<GrSingleIntervalGradientColorizer>();
        (void)_outer;
        auto start = _outer.start;
        (void)start;
        auto end = _outer.end;
        (void)end;
        startVar =
                args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType, "start");
        endVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType, "end");
        fragBuilder->codeAppendf("half t = %s.x;\n%s = (1.0 - t) * %s + t * %s;\n",
                                 args.fInputColor, args.fOutputColor,
                                 args.fUniformHandler->getUniformCStr(startVar),
                                 args.fUniformHandler->getUniformCStr(endVar));
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrSingleIntervalGradientColorizer& _outer =
                _proc.cast<GrSingleIntervalGradientColorizer>();
        {
            const SkPMColor4f& startValue = _outer.start;
            if (startPrev != startValue) {
                startPrev = startValue;
                pdman.set4fv(startVar, 1, startValue.vec());
            }
            const SkPMColor4f& endValue = _outer.end;
            if (endPrev != endValue) {
                endPrev = endValue;
                pdman.set4fv(endVar, 1, endValue.vec());
            }
        }
    }
    SkPMColor4f startPrev = {SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN};
    SkPMColor4f endPrev = {SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN};
    UniformHandle startVar;
    UniformHandle endVar;
};
GrGLSLFragmentProcessor* GrSingleIntervalGradientColorizer::onCreateGLSLInstance() const {
    return new GrGLSLSingleIntervalGradientColorizer();
}
void GrSingleIntervalGradientColorizer::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                              GrProcessorKeyBuilder* b) const {}
bool GrSingleIntervalGradientColorizer::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSingleIntervalGradientColorizer& that = other.cast<GrSingleIntervalGradientColorizer>();
    (void)that;
    if (start != that.start) return false;
    if (end != that.end) return false;
    return true;
}
GrSingleIntervalGradientColorizer::GrSingleIntervalGradientColorizer(
        const GrSingleIntervalGradientColorizer& src)
        : INHERITED(kGrSingleIntervalGradientColorizer_ClassID, src.optimizationFlags())
        , start(src.start)
        , end(src.end) {}
std::unique_ptr<GrFragmentProcessor> GrSingleIntervalGradientColorizer::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrSingleIntervalGradientColorizer(*this));
}
