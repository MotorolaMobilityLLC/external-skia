/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "Test.h"

#if SK_SUPPORT_GPU

static void test(skiatest::Reporter* r, const char* src, const GrShaderCaps& caps,
                 std::vector<const char*> expectedH, std::vector<const char*> expectedCPP) {
    SkSL::Program::Settings settings;
    settings.fCaps = &caps;
    SkSL::Compiler compiler;
    SkSL::StringStream output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::Program::kFragmentProcessor_Kind,
                                                             SkString(src),
                                                             settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        return;
    }
    REPORTER_ASSERT(r, program);
    bool success = compiler.toH(*program, "Test", output);
    if (!success) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, success);
    if (success) {
        for (const char* expected : expectedH) {
            bool found = strstr(output.str().c_str(), expected);
            if (!found) {
                SkDebugf("HEADER MISMATCH:\nsource:\n%s\n\nexpected:\n'%s'\n\nreceived:\n'%s'", src,
                         expected, output.str().c_str());
            }
            REPORTER_ASSERT(r, found);
        }
    }
    output.reset();
    success = compiler.toCPP(*program, "Test", output);
    if (!success) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, success);
    if (success) {
        for (const char* expected : expectedCPP) {
            bool found = strstr(output.str().c_str(), expected);
            if (!found) {
                SkDebugf("CPP MISMATCH:\nsource:\n%s\n\nexpected:\n'%s'\n\nreceived:\n'%s'", src,
                         expected, output.str().c_str());
            }
            REPORTER_ASSERT(r, found);
        }
    }
}

DEF_TEST(SkSLFPHelloWorld, r) {
    test(r,
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "/*\n"
             " * Copyright 2017 Google Inc.\n"
             " *\n"
             " * Use of this source code is governed by a BSD-style license that can be\n"
             " * found in the LICENSE file.\n"
             " */\n"
             "\n"
             "/*\n"
             " * This file was autogenerated from GrTest.fp; do not modify.\n"
             " */\n"
             "#ifndef GrTest_DEFINED\n"
             "#define GrTest_DEFINED\n"
             "#include \"SkTypes.h\"\n"
             "#if SK_SUPPORT_GPU\n"
             "#include \"GrFragmentProcessor.h\"\n"
             "#include \"GrCoordTransform.h\"\n"
             "#include \"GrColorSpaceXform.h\"\n"
             "class GrTest : public GrFragmentProcessor {\n"
             "public:\n"
             "    static sk_sp<GrFragmentProcessor> Make() {\n"
             "        return sk_sp<GrFragmentProcessor>(new GrTest());\n"
             "    }\n"
             "    GrTest(const GrTest& src);\n"
             "    sk_sp<GrFragmentProcessor> clone() const override;\n"
             "    const char* name() const override { return \"Test\"; }\n"
             "private:\n"
             "    GrTest()\n"
             "    : INHERITED(kNone_OptimizationFlags) {\n"
             "        this->initClassID<GrTest>();\n"
             "    }\n"
             "    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;\n"
             "    void onGetGLSLProcessorKey(const GrShaderCaps&,GrProcessorKeyBuilder*) "
                    "const override;\n"
             "    bool onIsEqual(const GrFragmentProcessor&) const override;\n"
             "    GR_DECLARE_FRAGMENT_PROCESSOR_TEST\n"
             "    typedef GrFragmentProcessor INHERITED;\n"
             "};\n"
             "#endif\n"
             "#endif\n"
         },
         {
             "/*\n"
             " * Copyright 2017 Google Inc.\n"
             " *\n"
             " * Use of this source code is governed by a BSD-style license that can be\n"
             " * found in the LICENSE file.\n"
             " */\n"
             "\n"
             "/*\n"
             " * This file was autogenerated from GrTest.fp; do not modify.\n"
             " */\n"
             "#include \"GrTest.h\"\n"
             "#if SK_SUPPORT_GPU\n"
             "#include \"glsl/GrGLSLColorSpaceXformHelper.h\"\n"
             "#include \"glsl/GrGLSLFragmentProcessor.h\"\n"
             "#include \"glsl/GrGLSLFragmentShaderBuilder.h\"\n"
             "#include \"glsl/GrGLSLProgramBuilder.h\"\n"
             "#include \"SkSLCPP.h\"\n"
             "#include \"SkSLUtil.h\"\n"
             "class GrGLSLTest : public GrGLSLFragmentProcessor {\n"
             "public:\n"
             "    GrGLSLTest() {}\n"
             "    void emitCode(EmitArgs& args) override {\n"
             "        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;\n"
             "        const GrTest& _outer = args.fFp.cast<GrTest>();\n"
             "        (void) _outer;\n"
             "        fragBuilder->codeAppendf(\"%s = float4(1.0);\\n\", args.fOutputColor);\n"
             "    }\n"
             "private:\n"
             "    void onSetData(const GrGLSLProgramDataManager& pdman, "
                                "const GrFragmentProcessor& _proc) override {\n"
             "    }\n"
             "};\n"
             "GrGLSLFragmentProcessor* GrTest::onCreateGLSLInstance() const {\n"
             "    return new GrGLSLTest();\n"
             "}\n"
             "void GrTest::onGetGLSLProcessorKey(const GrShaderCaps& caps, "
                                                "GrProcessorKeyBuilder* b) const {\n"
             "}\n"
             "bool GrTest::onIsEqual(const GrFragmentProcessor& other) const {\n"
             "    const GrTest& that = other.cast<GrTest>();\n"
             "    (void) that;\n"
             "    return true;\n"
             "}\n"
             "GrTest::GrTest(const GrTest& src)\n"
             ": INHERITED(src.optimizationFlags()) {\n"
             "    this->initClassID<GrTest>();\n"
             "}\n"
             "sk_sp<GrFragmentProcessor> GrTest::clone() const {\n"
             "    return sk_sp<GrFragmentProcessor>(new GrTest(*this));\n"
             "}\n"
             "#endif\n"
         });
}

DEF_TEST(SkSLFPInput, r) {
    test(r,
         "in float2 point;"
         "void main() {"
         "sk_OutColor = float4(point, point);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "SkPoint point() const { return fPoint; }",
             "static sk_sp<GrFragmentProcessor> Make(SkPoint point) {",
             "return sk_sp<GrFragmentProcessor>(new GrTest(point));",
             "GrTest(SkPoint point)",
             ", fPoint(point)"
         },
         {
             "fragBuilder->codeAppendf(\"%s = float4(float2(%f, %f), float2(%f, %f));\\n\", "
                                      "args.fOutputColor, _outer.point().fX, _outer.point().fY, "
                                      "_outer.point().fX, _outer.point().fY);",
             "if (fPoint != that.fPoint) return false;"
         });
}

DEF_TEST(SkSLFPUniform, r) {
    test(r,
         "uniform float4 color;"
         "void main() {"
         "sk_OutColor = color;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "static sk_sp<GrFragmentProcessor> Make()"
         },
         {
            "fColorVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kVec4f_GrSLType, "
                                                         "kDefault_GrSLPrecision, \"color\");",
         });
}

DEF_TEST(SkSLFPInUniform, r) {
    test(r,
         "in uniform float4 color;"
         "void main() {"
         "sk_OutColor = color;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "static sk_sp<GrFragmentProcessor> Make(SkRect color) {",
         },
         {
            "fColorVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kVec4f_GrSLType, "
                                                         "kDefault_GrSLPrecision, \"color\");",
            "const SkRect colorValue = _outer.color();",
            "pdman.set4fv(fColorVar, 1, (float*) &colorValue);"
         });
}

DEF_TEST(SkSLFPSections, r) {
    test(r,
         "@header { header section }"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "#if SK_SUPPORT_GPU\n header section"
         },
         {});
    test(r,
         "@class { class section }"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "class GrTest : public GrFragmentProcessor {\n"
             "public:\n"
             " class section"
         },
         {});
    test(r,
         "@cpp { cpp section }"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {},
         {"cpp section"});
    test(r,
         "@constructorParams { int x, float y, std::vector<float> z }"
         "in float w;"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "Make(float w,  int x, float y, std::vector<float> z )",
             "return sk_sp<GrFragmentProcessor>(new GrTest(w, x, y, z));",
             "GrTest(float w,  int x, float y, std::vector<float> z )",
             ", fW(w) {"
         },
         {});
    test(r,
         "@constructor { constructor section }"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "private:\n constructor section"
         },
         {});
    test(r,
         "@initializers { initializers section }"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             ": INHERITED(kNone_OptimizationFlags)\n    ,  initializers section"
         },
         {});
    test(r,
         "float x = 10;"
         "@emitCode { fragBuilder->codeAppendf(\"float y = %d\\n\", x * 2); }"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {},
         {
            "x = 10.0;\n"
            " fragBuilder->codeAppendf(\"float y = %d\\n\", x * 2);"
         });
    test(r,
         "@fields { fields section }"
         "@clone { }"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
            "GR_DECLARE_FRAGMENT_PROCESSOR_TEST\n"
            " fields section     typedef GrFragmentProcessor INHERITED;"
         },
         {});
    test(r,
         "@make { make section }"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
            "public:\n"
            " make section"
         },
         {});
    test(r,
         "uniform float calculated;"
         "in float provided;"
         "@setData(varName) { varName.set1f(calculated, provided * 2); }"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {},
         {
             "void onSetData(const GrGLSLProgramDataManager& varName, "
                            "const GrFragmentProcessor& _proc) override {\n",
             "UniformHandle& calculated = fCalculatedVar;",
             "auto provided = _outer.provided();",
             "varName.set1f(calculated, provided * 2);"
         });
    test(r,
         "@test(testDataName) { testDataName section }"
         "void main() {"
         "sk_OutColor = float4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {},
         {
             "#if GR_TEST_UTILS\n"
             "sk_sp<GrFragmentProcessor> GrTest::TestCreate(GrProcessorTestData* testDataName) {\n"
             " testDataName section }\n"
             "#endif"
         });
}

DEF_TEST(SkSLFPColorSpaceXform, r) {
    test(r,
         "in uniform sampler2D image;"
         "in uniform colorSpaceXform colorXform;"
         "void main() {"
         "sk_OutColor = sk_InColor * texture(image, float2(0, 0), colorXform);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "sk_sp<GrColorSpaceXform> colorXform() const { return fColorXform; }",
             "GrTest(sk_sp<GrTextureProxy> image, sk_sp<GrColorSpaceXform> colorXform)",
             "this->addTextureSampler(&fImage);",
             "sk_sp<GrColorSpaceXform> fColorXform;"
         },
         {
             "fragBuilder->codeAppendf(\"float4 _tmpVar1;%s = %s * %stexture(%s, "
             "float2(0.0, 0.0)).%s%s;\\n\", args.fOutputColor, args.fInputColor ? args.fInputColor : "
             "\"float4(1)\", fColorSpaceHelper.isValid() ? \"(_tmpVar1 = \" : \"\", "
             "fragBuilder->getProgramBuilder()->samplerVariable(args.fTexSamplers[0]).c_str(), "
             "fragBuilder->getProgramBuilder()->samplerSwizzle(args.fTexSamplers[0]).c_str(), "
             "fColorSpaceHelper.isValid() ? SkStringPrintf(\", float4(clamp((%s * float4(_tmpVar1.rgb, "
             "1.0)).rgb, 0.0, _tmpVar1.a), _tmpVar1.a))\", args.fUniformHandler->getUniformCStr("
             "fColorSpaceHelper.gamutXformUniform())).c_str() : \"\");"
         });
}

DEF_TEST(SkSLFPTransformedCoords, r) {
    test(r,
         "void main() {"
         "sk_OutColor = float4(sk_TransformedCoords2D[0], sk_TransformedCoords2D[0]);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {},
         {
            "SkSL::String sk_TransformedCoords2D_0 = "
                                         "fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);",
            "fragBuilder->codeAppendf(\"%s = float4(%s, %s);\\n\", args.fOutputColor, "
                              "sk_TransformedCoords2D_0.c_str(), sk_TransformedCoords2D_0.c_str());"
         });

}

DEF_TEST(SkSLFPLayoutWhen, r) {
    test(r,
         "layout(when=someExpression(someOtherExpression())) uniform float sometimes;"
         "void main() {"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {},
         {
            "if (someExpression(someOtherExpression())) {\n"
            "            fSometimesVar = args.fUniformHandler->addUniform"
         });

}

#endif
