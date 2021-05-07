

/**************************************************************************************************
 *** This file was autogenerated from GrDSLFPTest_DoStatement.fp; do not modify.
 **************************************************************************************************/
/* TODO(skia:11854): DSLCPPCodeGenerator is currently a work in progress. */
#include "GrDSLFPTest_DoStatement.h"

#include "src/core/SkUtils.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/dsl/priv/DSLFPs.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

class GrGLSLDSLFPTest_DoStatement : public GrGLSLFragmentProcessor {
public:
    GrGLSLDSLFPTest_DoStatement() {}
    void emitCode(EmitArgs& args) override {
        [[maybe_unused]] const GrDSLFPTest_DoStatement& _outer = args.fFp.cast<GrDSLFPTest_DoStatement>();

        using namespace SkSL::dsl;
        StartFragmentProcessor(this, &args);
[[maybe_unused]] const auto& shouldLoop = _outer.shouldLoop;
Var _shouldLoop(kConst_Modifier, DSLType(kBool_Type), "shouldLoop", Bool(!!(shouldLoop)));
Declare(_shouldLoop);
Var _color(kNo_Modifier, DSLType(kHalf4_Type), "color", Half4(1.0f, 1.0f, 1.0f, 1.0f));
Declare(_color);
Do(_color.x() -= 0.25f, /*While:*/ _shouldLoop);
Do(Block(_color.x() -= 0.25f, If(_color.x() <= 0.0f, /*Then:*/ Break())), /*While:*/ _color.w() == 1.0f);
Do(Block(_color.z() -= 0.25f, If(_color.w() == 1.0f || sk_Caps.builtinFMASupport, /*Then:*/ Continue()), _color.y() = 0.0f), /*While:*/ _color.z() > 0.0f);
Return(_color);
        EndFragmentProcessor();
    }
private:
    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& _proc) override {
    }
};
std::unique_ptr<GrGLSLFragmentProcessor> GrDSLFPTest_DoStatement::onMakeProgramImpl() const {
    return std::make_unique<GrGLSLDSLFPTest_DoStatement>();
}
void GrDSLFPTest_DoStatement::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
    b->addBool(shouldLoop, "shouldLoop");
}
bool GrDSLFPTest_DoStatement::onIsEqual(const GrFragmentProcessor& other) const {
    const GrDSLFPTest_DoStatement& that = other.cast<GrDSLFPTest_DoStatement>();
    (void) that;
    if (shouldLoop != that.shouldLoop) return false;
    return true;
}
GrDSLFPTest_DoStatement::GrDSLFPTest_DoStatement(const GrDSLFPTest_DoStatement& src)
: INHERITED(kGrDSLFPTest_DoStatement_ClassID, src.optimizationFlags())
, shouldLoop(src.shouldLoop) {
        this->cloneAndRegisterAllChildProcessors(src);
}
std::unique_ptr<GrFragmentProcessor> GrDSLFPTest_DoStatement::clone() const {
    return std::make_unique<GrDSLFPTest_DoStatement>(*this);
}
#if GR_TEST_UTILS
SkString GrDSLFPTest_DoStatement::onDumpInfo() const {
    return SkStringPrintf("(shouldLoop=%d)", !!(shouldLoop));
}
#endif
