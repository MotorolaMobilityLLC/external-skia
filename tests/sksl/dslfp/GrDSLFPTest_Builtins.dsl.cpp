

/**************************************************************************************************
 *** This file was autogenerated from GrDSLFPTest_Builtins.fp; do not modify.
 **************************************************************************************************/
/* TODO(skia:11854): DSLCPPCodeGenerator is currently a work in progress. */
#include "GrDSLFPTest_Builtins.h"

#include "src/core/SkUtils.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/dsl/priv/DSLFPs.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

class GrGLSLDSLFPTest_Builtins : public GrGLSLFragmentProcessor {
public:
    GrGLSLDSLFPTest_Builtins() {}
    void emitCode(EmitArgs& args) override {
        [[maybe_unused]] const GrDSLFPTest_Builtins& _outer = args.fFp.cast<GrDSLFPTest_Builtins>();

        using namespace SkSL::dsl;
        StartFragmentProcessor(this, &args);
zero = 0.0f;
one = 1.0f;
Var _zero(kConst_Modifier, DSLType(kFloat_Type), "zero", Float(zero));
Declare(_zero);
Var _one(kConst_Modifier, DSLType(kFloat_Type), "one", Float(one));
Declare(_one);
Var _m(kNo_Modifier, DSLType(kHalf4x4_Type), "m", Half4x4(Half(_one)));
Var _n(kNo_Modifier, DSLType(kHalf4_Type), "n", Half4(Half(_zero)));
Var _b(kNo_Modifier, DSLType(kBool4_Type), "b", Bool4(true));
Declare(_m);
Declare(_n);
Declare(_b);
_n.x() = Abs(_n.x());
_b.z() = All(Swizzle(_b, X, Y));
_b.w() = Any(Swizzle(_b, X, Y, Z));
Swizzle(_n, X, Y) = Atan(Swizzle(_n, X, Y));
Swizzle(_n, Z, W, X) = Atan(Swizzle(_n, Y, Y, Y), Swizzle(_n, Z, Z, Z));
_n = Ceil(_n);
_n.x() = Clamp(_n.y(), _n.z(), _n.w());
_n.y() = Cos(_n.y());
_n.w() = _n.x() * _n.w() - _n.y() * _n.z();
Swizzle(_n, X, Y, Z) = Degrees(Swizzle(_n, X, Y, Z));
_n.w() = Distance(Swizzle(_n, X, Z), Swizzle(_n, Y, W));
_n.x() = Dot(Swizzle(_n, Y, Z, W), Swizzle(_n, Y, Z, W));
Swizzle(_b, X, Y, Z) = Equal(Swizzle(_b, X, X, X), Swizzle(_b, W, W, W));
Swizzle(_n, Y, Z) = Exp(Swizzle(_n, W, X));
Swizzle(_n, Z, W) = Exp2(Swizzle(_n, X, Y));
_n.x() = Faceforward(_n.y(), _n.z(), _n.w());
_n = Floor(_n);
Swizzle(_n, Y, Z, W) = SkSL::dsl::Fract(Swizzle(_n, Y, Z, W));
Swizzle(_b, X, Y) = GreaterThan(Swizzle(_n, X, Y), Swizzle(_n, Z, W));
Swizzle(_b, X, Y) = GreaterThanEqual(Swizzle(_n, X, Y), Swizzle(_n, Z, W));
_n = Inversesqrt(_n);
_m = Inverse(_m);
_n.w() = Length(Swizzle(_n, Z, Y, Y, X));
Swizzle(_b, X, Y) = LessThan(Swizzle(_n, X, Y), Swizzle(_n, Z, W));
Swizzle(_b, X, Y) = LessThanEqual(Swizzle(_n, X, Y), Swizzle(_n, Z, W));
_n.x() = Log(_n.x());
_n.y() = Max(_n.z(), _n.w());
_n.z() = Min(_n.x(), _n.y());
_n.w() = Mod(_n.y(), _n.z());
_n = Normalize(_n);
_b = Not(_b);
_n.x() = Pow(_n.y(), _n.z());
Swizzle(_n, X, Y, Z) = Radians(Swizzle(_n, Y, Z, W));
Swizzle(_n, X, Y) = Reflect(Swizzle(_n, X, Y), Swizzle(_n, Z, W));
Swizzle(_n, W, Z) = Refract(Swizzle(_n, X, Y), Swizzle(_n, Z, W), 2.0f);
_n = Saturate(_n);
_n.x() = Sign(_n.x());
_n.y() = Sin(_n.y());
Swizzle(_n, Z, W) = Smoothstep(Swizzle(_n, X, X), Swizzle(_n, Y, Y), Swizzle(_n, Z, Z));
_n = Sqrt(_n);
Swizzle(_n, X, Y) = Step(Swizzle(_n, X, Y), Swizzle(_n, Z, W));
_n.x() = Tan(_n.x());
_n = Half4(Swizzle(_n, W, W, W) / Max(_n.w(), 9.9999997473787516e-05f), _n.w());
Return(Half4(0.0f, 1.0f, 0.0f, 1.0f));
        EndFragmentProcessor();
    }
private:
    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& _proc) override {
    }
float zero = 0;
float one = 0;
};
std::unique_ptr<GrGLSLFragmentProcessor> GrDSLFPTest_Builtins::onMakeProgramImpl() const {
    return std::make_unique<GrGLSLDSLFPTest_Builtins>();
}
void GrDSLFPTest_Builtins::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
float zero = 0.0f;
    b->add32(sk_bit_cast<uint32_t>(zero), "zero");
float one = 1.0f;
    b->add32(sk_bit_cast<uint32_t>(one), "one");
}
bool GrDSLFPTest_Builtins::onIsEqual(const GrFragmentProcessor& other) const {
    const GrDSLFPTest_Builtins& that = other.cast<GrDSLFPTest_Builtins>();
    (void) that;
    return true;
}
GrDSLFPTest_Builtins::GrDSLFPTest_Builtins(const GrDSLFPTest_Builtins& src)
: INHERITED(kGrDSLFPTest_Builtins_ClassID, src.optimizationFlags()) {
        this->cloneAndRegisterAllChildProcessors(src);
}
std::unique_ptr<GrFragmentProcessor> GrDSLFPTest_Builtins::clone() const {
    return std::make_unique<GrDSLFPTest_Builtins>(*this);
}
#if GR_TEST_UTILS
SkString GrDSLFPTest_Builtins::onDumpInfo() const {
    return SkString();
}
#endif
