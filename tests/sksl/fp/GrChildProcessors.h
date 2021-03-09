

/**************************************************************************************************
 *** This file was autogenerated from GrChildProcessors.fp; do not modify.
 **************************************************************************************************/
#ifndef GrChildProcessors_DEFINED
#define GrChildProcessors_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkTypes.h"


#include "src/gpu/GrFragmentProcessor.h"

class GrChildProcessors : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child1, std::unique_ptr<GrFragmentProcessor> child2) {
        return std::unique_ptr<GrFragmentProcessor>(new GrChildProcessors(std::move(child1), std::move(child2)));
    }
    GrChildProcessors(const GrChildProcessors& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "ChildProcessors"; }
private:
    GrChildProcessors(std::unique_ptr<GrFragmentProcessor> child1, std::unique_ptr<GrFragmentProcessor> child2)
    : INHERITED(kGrChildProcessors_ClassID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child1), SkSL::SampleUsage::PassThrough());        this->registerChild(std::move(child2), SkSL::SampleUsage::PassThrough());    }
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
