/*#pragma settings NoInline*/

/**************************************************************************************************
 *** This file was autogenerated from GrFunctionArgTypes.fp; do not modify.
 **************************************************************************************************/
#ifndef GrFunctionArgTypes_DEFINED
#define GrFunctionArgTypes_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkTypes.h"


#include "src/gpu/GrFragmentProcessor.h"

class GrFunctionArgTypes : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make() {
        return std::unique_ptr<GrFragmentProcessor>(new GrFunctionArgTypes());
    }
    GrFunctionArgTypes(const GrFunctionArgTypes& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "FunctionArgTypes"; }
private:
    GrFunctionArgTypes()
    : INHERITED(kGrFunctionArgTypes_ClassID, kNone_OptimizationFlags) {
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
