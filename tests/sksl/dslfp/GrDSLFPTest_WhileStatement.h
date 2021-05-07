

/**************************************************************************************************
 *** This file was autogenerated from GrDSLFPTest_WhileStatement.fp; do not modify.
 **************************************************************************************************/
#ifndef GrDSLFPTest_WhileStatement_DEFINED
#define GrDSLFPTest_WhileStatement_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkTypes.h"


#include "src/gpu/GrFragmentProcessor.h"

class GrDSLFPTest_WhileStatement : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make() {
        return std::unique_ptr<GrFragmentProcessor>(new GrDSLFPTest_WhileStatement());
    }
    GrDSLFPTest_WhileStatement(const GrDSLFPTest_WhileStatement& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "DSLFPTest_WhileStatement"; }
private:
    GrDSLFPTest_WhileStatement()
    : INHERITED(kGrDSLFPTest_WhileStatement_ClassID, kNone_OptimizationFlags) {
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
