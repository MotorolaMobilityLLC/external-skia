/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This file was autogenerated from GrConstColorProcessor.fp; do not modify.
 */
#include "GrConstColorProcessor.h"
#if SK_SUPPORT_GPU
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "GrTexture.h"
#include "SkSLCPP.h"
#include "SkSLUtil.h"
class GrGLSLConstColorProcessor : public GrGLSLFragmentProcessor {
public:
    GrGLSLConstColorProcessor() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrConstColorProcessor& _outer = args.fFp.cast<GrConstColorProcessor>();
        (void)_outer;
        auto color = _outer.color();
        (void)color;
        auto mode = _outer.mode();
        (void)mode;
        fColorUniformVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType,
                                                            kDefault_GrSLPrecision, "colorUniform");
        fragBuilder->codeAppendf(
                "half4 prevColor;\n@switch (%d) {\n    case 0:\n        %s = %s;\n        break;\n "
                "   case 1:\n        %s = %s * %s;\n        break;\n    case 2:\n        %s = %s.w "
                "* %s;\n        break;\n}\n",
                (int)_outer.mode(), args.fOutputColor,
                args.fUniformHandler->getUniformCStr(fColorUniformVar), args.fOutputColor,
                args.fInputColor ? args.fInputColor : "half4(1)",
                args.fUniformHandler->getUniformCStr(fColorUniformVar), args.fOutputColor,
                args.fInputColor ? args.fInputColor : "half4(1)",
                args.fUniformHandler->getUniformCStr(fColorUniformVar));
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrConstColorProcessor& _outer = _proc.cast<GrConstColorProcessor>();
        auto color = _outer.color();
        (void)color;
        UniformHandle& colorUniform = fColorUniformVar;
        (void)colorUniform;
        auto mode = _outer.mode();
        (void)mode;

        // We use the "illegal" color value as an uninit sentinel. With GrColor4f, the "illegal"
        // color is *really* illegal (not just unpremultiplied), so this check is simple.
        if (prevColor != color) {
            pdman.set4fv(colorUniform, 1, color.fRGBA);
            prevColor = color;
        }
    }
    GrColor4f prevColor = GrColor4f::kIllegalConstructor;
    UniformHandle fColorUniformVar;
};
GrGLSLFragmentProcessor* GrConstColorProcessor::onCreateGLSLInstance() const {
    return new GrGLSLConstColorProcessor();
}
void GrConstColorProcessor::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {
    b->add32((int32_t)fMode);
}
bool GrConstColorProcessor::onIsEqual(const GrFragmentProcessor& other) const {
    const GrConstColorProcessor& that = other.cast<GrConstColorProcessor>();
    (void)that;
    if (fColor != that.fColor) return false;
    if (fMode != that.fMode) return false;
    return true;
}
GrConstColorProcessor::GrConstColorProcessor(const GrConstColorProcessor& src)
        : INHERITED(kGrConstColorProcessor_ClassID, src.optimizationFlags())
        , fColor(src.fColor)
        , fMode(src.fMode) {}
std::unique_ptr<GrFragmentProcessor> GrConstColorProcessor::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrConstColorProcessor(*this));
}
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConstColorProcessor);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrConstColorProcessor::TestCreate(GrProcessorTestData* d) {
    GrColor4f color;
    int colorPicker = d->fRandom->nextULessThan(3);
    switch (colorPicker) {
        case 0: {
            uint32_t a = d->fRandom->nextULessThan(0x100);
            uint32_t r = d->fRandom->nextULessThan(a + 1);
            uint32_t g = d->fRandom->nextULessThan(a + 1);
            uint32_t b = d->fRandom->nextULessThan(a + 1);
            color = GrColor4f::FromGrColor(GrColorPackRGBA(r, g, b, a));
            break;
        }
        case 1:
            color = GrColor4f::TransparentBlack();
            break;
        case 2:
            uint32_t c = d->fRandom->nextULessThan(0x100);
            color = GrColor4f::FromGrColor(c | (c << 8) | (c << 16) | (c << 24));
            break;
    }
    InputMode mode = static_cast<InputMode>(d->fRandom->nextULessThan(kInputModeCnt));
    return GrConstColorProcessor::Make(color, mode);
}
#endif
#endif
