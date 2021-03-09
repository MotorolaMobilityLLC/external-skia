

/**************************************************************************************************
 *** This file was autogenerated from GrSectionHeader.fp; do not modify.
 **************************************************************************************************/
#ifndef GrSectionHeader_DEFINED
#define GrSectionHeader_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkTypes.h"

header section
#include "src/gpu/GrFragmentProcessor.h"

class GrSectionHeader : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make() {
        return std::unique_ptr<GrFragmentProcessor>(new GrSectionHeader());
    }
    GrSectionHeader(const GrSectionHeader& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "SectionHeader"; }
private:
    GrSectionHeader()
    : INHERITED(kGrSectionHeader_ClassID, kNone_OptimizationFlags) {
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
