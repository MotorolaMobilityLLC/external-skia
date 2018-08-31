/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "Test.h"

static void test(skiatest::Reporter* r, const char* src, const GrShaderCaps& caps,
                 std::vector<const char*> expectedH, std::vector<const char*> expectedCPP) {
    SkSL::Program::Settings settings;
    settings.fCaps = &caps;
    SkSL::Compiler compiler;
    SkSL::StringStream output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::Program::kFragmentProcessor_Kind,
                                                             SkSL::String(src),
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
         "/* HEADER */"
         "void main() {"
         "sk_OutColor = half4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "/* HEADER */\n"
             "\n"
             "/**************************************************************************************************\n"
             " *** This file was autogenerated from GrTest.fp; do not modify.\n"
             " **************************************************************************************************/\n"
             "#ifndef GrTest_DEFINED\n"
             "#define GrTest_DEFINED\n"
             "#include \"SkTypes.h\"\n"
             "#include \"GrFragmentProcessor.h\"\n"
             "#include \"GrCoordTransform.h\"\n"
             "class GrTest : public GrFragmentProcessor {\n"
             "public:\n"
             "    static std::unique_ptr<GrFragmentProcessor> Make() {\n"
             "        return std::unique_ptr<GrFragmentProcessor>(new GrTest());\n"
             "    }\n"
             "    GrTest(const GrTest& src);\n"
             "    std::unique_ptr<GrFragmentProcessor> clone() const override;\n"
             "    const char* name() const override { return \"Test\"; }\n"
             "private:\n"
             "    GrTest()\n"
             "    : INHERITED(kGrTest_ClassID, kNone_OptimizationFlags) {\n"
             "    }\n"
             "    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;\n"
             "    void onGetGLSLProcessorKey(const GrShaderCaps&,GrProcessorKeyBuilder*) "
                    "const override;\n"
             "    bool onIsEqual(const GrFragmentProcessor&) const override;\n"
             "    GR_DECLARE_FRAGMENT_PROCESSOR_TEST\n"
             "    typedef GrFragmentProcessor INHERITED;\n"
             "};\n"
             "#endif\n"
         },
         {
             "/* HEADER */\n"
             "\n"
             "/**************************************************************************************************\n"
             " *** This file was autogenerated from GrTest.fp; do not modify.\n"
             " **************************************************************************************************/\n"
             "#include \"GrTest.h\"\n"
             "#include \"glsl/GrGLSLFragmentProcessor.h\"\n"
             "#include \"glsl/GrGLSLFragmentShaderBuilder.h\"\n"
             "#include \"glsl/GrGLSLProgramBuilder.h\"\n"
             "#include \"GrTexture.h\"\n"
             "#include \"SkSLCPP.h\"\n"
             "#include \"SkSLUtil.h\"\n"
             "class GrGLSLTest : public GrGLSLFragmentProcessor {\n"
             "public:\n"
             "    GrGLSLTest() {}\n"
             "    void emitCode(EmitArgs& args) override {\n"
             "        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;\n"
             "        const GrTest& _outer = args.fFp.cast<GrTest>();\n"
             "        (void) _outer;\n"
             "        fragBuilder->codeAppendf(\"%s = half4(1.0);\\n\", args.fOutputColor);\n"
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
             ": INHERITED(kGrTest_ClassID, src.optimizationFlags()) {\n"
             "}\n"
             "std::unique_ptr<GrFragmentProcessor> GrTest::clone() const {\n"
             "    return std::unique_ptr<GrFragmentProcessor>(new GrTest(*this));\n"
             "}\n"
         });
}

DEF_TEST(SkSLFPInput, r) {
    test(r,
         "in half2 point;"
         "void main() {"
         "sk_OutColor = half4(point, point);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "SkPoint point() const { return fPoint; }",
             "static std::unique_ptr<GrFragmentProcessor> Make(SkPoint point) {",
             "return std::unique_ptr<GrFragmentProcessor>(new GrTest(point));",
             "GrTest(SkPoint point)",
             ", fPoint(point)"
         },
         {
             "fragBuilder->codeAppendf(\"%s = half4(half2(%f, %f), half2(%f, %f));\\n\", "
                                      "args.fOutputColor, _outer.point().fX, _outer.point().fY, "
                                      "_outer.point().fX, _outer.point().fY);",
             "if (fPoint != that.fPoint) return false;"
         });
}

DEF_TEST(SkSLFPUniform, r) {
    test(r,
         "uniform half4 color;"
         "void main() {"
         "sk_OutColor = color;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "static std::unique_ptr<GrFragmentProcessor> Make()"
         },
         {
            "fColorVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType, "
                                                         "kDefault_GrSLPrecision, \"color\");",
         });
}

DEF_TEST(SkSLFPInUniform, r) {
    test(r,
         "in uniform half4 color;"
         "void main() {"
         "sk_OutColor = color;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "static std::unique_ptr<GrFragmentProcessor> Make(SkRect color) {",
         },
         {
            "fColorVar = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4_GrSLType, "
                                                         "kDefault_GrSLPrecision, \"color\");",
            "const SkRect colorValue = _outer.color();",
            "pdman.set4fv(fColorVar, 1, (float*) &colorValue);"
         });
}

DEF_TEST(SkSLFPSections, r) {
    test(r,
         "@header { header section }"
         "void main() {"
         "sk_OutColor = half4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "header section"
         },
         {});
    test(r,
         "@class { class section }"
         "void main() {"
         "sk_OutColor = half4(1);"
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
         "sk_OutColor = half4(1);"
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
             "return std::unique_ptr<GrFragmentProcessor>(new GrTest(w, x, y, z));",
             "GrTest(float w,  int x, float y, std::vector<float> z )",
             ", fW(w) {"
         },
         {});
    test(r,
         "@constructor { constructor section }"
         "void main() {"
         "sk_OutColor = half4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             "private:\n constructor section"
         },
         {});
    test(r,
         "@initializers { initializers section }"
         "void main() {"
         "sk_OutColor = half4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
             ": INHERITED(kGrTest_ClassID, kNone_OptimizationFlags)\n    ,  initializers section"
         },
         {});
    test(r,
         "half x = 10;"
         "@emitCode { fragBuilder->codeAppendf(\"half y = %d\\n\", x * 2); }"
         "void main() {"
         "sk_OutColor = half4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {},
         {
            "x = 10.0;\n"
            " fragBuilder->codeAppendf(\"half y = %d\\n\", x * 2);"
         });
    test(r,
         "@fields { fields section }"
         "@clone { }"
         "void main() {"
         "sk_OutColor = half4(1);"
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
         "sk_OutColor = half4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
            "public:\n"
            " make section"
         },
         {});
    test(r,
         "uniform half calculated;"
         "in half provided;"
         "@setData(varName) { varName.set1f(calculated, provided * 2); }"
         "void main() {"
         "sk_OutColor = half4(1);"
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
         "sk_OutColor = half4(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {},
         {
             "#if GR_TEST_UTILS\n"
             "std::unique_ptr<GrFragmentProcessor> GrTest::TestCreate(GrProcessorTestData* testDataName) {\n"
             " testDataName section }\n"
             "#endif"
         });
}

DEF_TEST(SkSLFPTransformedCoords, r) {
    test(r,
         "void main() {"
         "sk_OutColor = half4(sk_TransformedCoords2D[0], sk_TransformedCoords2D[0]);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {},
         {
            "SkString sk_TransformedCoords2D_0 = "
                                         "fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);",
            "fragBuilder->codeAppendf(\"%s = half4(%s, %s);\\n\", args.fOutputColor, "
                              "sk_TransformedCoords2D_0.c_str(), sk_TransformedCoords2D_0.c_str());"
         });

}

DEF_TEST(SkSLFPLayoutWhen, r) {
    test(r,
         "layout(when=someExpression(someOtherExpression())) uniform half sometimes;"
         "void main() {"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {},
         {
            "if (someExpression(someOtherExpression())) {\n"
            "            fSometimesVar = args.fUniformHandler->addUniform"
         });

}

DEF_TEST(SkSLFPChildProcessors, r) {
    test(r,
         "in fragmentProcessor child1;"
         "in fragmentProcessor child2;"
         "void main() {"
         "    sk_OutColor = process(child1) * process(child2);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
            "this->registerChildProcessor(std::move(child1));",
            "this->registerChildProcessor(std::move(child2));"
         },
         {
            "SkString _child0(\"_child0\");",
            "this->emitChild(0, &_child0, args);",
            "SkString _child1(\"_child1\");",
            "this->emitChild(1, &_child1, args);",
            "this->registerChildProcessor(src.childProcessor(0).clone());",
            "this->registerChildProcessor(src.childProcessor(1).clone());"
         });
}

DEF_TEST(SkSLFPChildProcessorsWithInput, r) {
    test(r,
         "in fragmentProcessor child1;"
         "in fragmentProcessor child2;"
         "void main() {"
         "    half4 childIn = sk_InColor;"
         "    half4 childOut1 = process(child1, childIn);"
         "    half4 childOut2 = process(child2, childOut1);"
         "    sk_OutColor = childOut2;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
            "this->registerChildProcessor(std::move(child1));",
            "this->registerChildProcessor(std::move(child2));"
         },
         {
            "SkString _input0(\"childIn\");",
            "SkString _child0(\"_child0\");",
            "this->emitChild(0, _input0.c_str(), &_child0, args);",
            "SkString _input1(\"childOut1\");",
            "SkString _child1(\"_child1\");",
            "this->emitChild(1, _input1.c_str(), &_child1, args);",
            "this->registerChildProcessor(src.childProcessor(0).clone());",
            "this->registerChildProcessor(src.childProcessor(1).clone());"
         });
}

DEF_TEST(SkSLFPChildProcessorWithInputExpression, r) {
    test(r,
         "in fragmentProcessor child;"
         "void main() {"
         "    sk_OutColor = process(child, sk_InColor * half4(0.5));"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
            "this->registerChildProcessor(std::move(child));",
         },
         {
            "SkString _input0 = SkStringPrintf(\"%s * half4(0.5)\", args.fInputColor);",
            "SkString _child0(\"_child0\");",
            "this->emitChild(0, _input0.c_str(), &_child0, args);",
            "this->registerChildProcessor(src.childProcessor(0).clone());",
         });
}

DEF_TEST(SkSLFPNestedChildProcessors, r) {
    test(r,
         "in fragmentProcessor child1;"
         "in fragmentProcessor child2;"
         "void main() {"
         "    sk_OutColor = process(child2, sk_InColor * process(child1, sk_InColor * half4(0.5)));"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         {
            "this->registerChildProcessor(std::move(child1));",
            "this->registerChildProcessor(std::move(child2));"
         },
         {
            "SkString _input0 = SkStringPrintf(\"%s * half4(0.5)\", args.fInputColor);",
            "SkString _child0(\"_child0\");",
            "this->emitChild(0, _input0.c_str(), &_child0, args);",
            "SkString _input1 = SkStringPrintf(\"%s * %s\", args.fInputColor, _child0.c_str());",
            "SkString _child1(\"_child1\");",
            "this->emitChild(1, _input1.c_str(), &_child1, args);",
            "this->registerChildProcessor(src.childProcessor(0).clone());",
            "this->registerChildProcessor(src.childProcessor(1).clone());"
         });
}
