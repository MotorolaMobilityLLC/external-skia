

/**************************************************************************************************
 *** This file was autogenerated from GrChildProcessorWithInputExpression.fp; do not modify.
 **************************************************************************************************/
#ifndef GrChildProcessorWithInputExpression_DEFINED
#define GrChildProcessorWithInputExpression_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkTypes.h"


#include "src/gpu/GrFragmentProcessor.h"

class GrChildProcessorWithInputExpression : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child) {
        return std::unique_ptr<GrFragmentProcessor>(new GrChildProcessorWithInputExpression(std::move(child)));
    }
    GrChildProcessorWithInputExpression(const GrChildProcessorWithInputExpression& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "ChildProcessorWithInputExpression"; }
private:
    GrChildProcessorWithInputExpression(std::unique_ptr<GrFragmentProcessor> child)
    : INHERITED(kGrChildProcessorWithInputExpression_ClassID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child), SkSL::SampleUsage::PassThrough());    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
#if GR_TEST_UTILS
    SkString onDumpInfo() const override;
#endif
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    using INHERITED = GrFragmentProcessor;
};
#endif
