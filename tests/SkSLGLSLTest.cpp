/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "Test.h"

static void test(skiatest::Reporter* r, const char* src, SkSL::GLCaps caps, const char* expected) {
    SkSL::Compiler compiler;
    std::string output;
    bool result = compiler.toGLSL(SkSL::Program::kFragment_Kind, src, caps, &output);
    if (!result) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, result);
    if (result) {
        if (output != expected) {
            SkDebugf("GLSL MISMATCH:\nsource:\n%s\n\nexpected:\n'%s'\n\nreceived:\n'%s'", src, 
                     expected, output.c_str());
        }
        REPORTER_ASSERT(r, output == expected);
    }
}

static SkSL::GLCaps default_caps() {
    return { 
             400, 
             SkSL::GLCaps::kGL_Standard,
             false, // isCoreProfile
             false, // usesPrecisionModifiers;
             false, // mustDeclareFragmentShaderOutput
             true   // canUseMinAndAbsTogether
           };
}

DEF_TEST(SkSLHelloWorld, r) {
    test(r,
         "void main() { sk_FragColor = vec4(0.75); }",
         default_caps(),
         "#version 400\n"
         "void main() {\n"
         "    gl_FragColor = vec4(0.75);\n"
         "}\n");
}

DEF_TEST(SkSLControl, r) {
    test(r,
         "void main() {"
         "if (1 + 2 + 3 > 5) { sk_FragColor = vec4(0.75); } else { discard; }"
         "int i = 0;"
         "while (i < 10) sk_FragColor *= 0.5;"
         "do { sk_FragColor += 0.01; } while (sk_FragColor.x < 0.7);"
         "for (int i = 0; i < 10; i++) {"
         "if (i % 0 == 1) break; else continue;"
         "}"
         "return;"
         "}",
         default_caps(),
         "#version 400\n"
         "void main() {\n"
         "    if ((1 + 2) + 3 > 5) {\n"
         "        gl_FragColor = vec4(0.75);\n"
         "    } else {\n"
         "        discard;\n"
         "    }\n"
         "    int i = 0;\n"
         "    while (i < 10) gl_FragColor *= 0.5;\n"
         "    do {\n"
         "        gl_FragColor += 0.01;\n"
         "    } while (gl_FragColor.x < 0.7);\n"
         "    for (int i = 0;i < 10; i++) {\n"
         "        if (i % 0 == 1) break; else continue;\n"
         "    }\n"
         "    return;\n"
         "}\n");
}

DEF_TEST(SkSLFunctions, r) {
    test(r,
         "float foo(float v[2]) { return v[0] * v[1]; }"
         "void bar(inout float x) { float y[2], z; y[0] = x; y[1] = x * 2; z = foo(y); x = z; }"
         "void main() { float x = 10; bar(x); sk_FragColor = vec4(x); }",
         default_caps(),
         "#version 400\n"
         "float foo(in float v[2]) {\n"
         "    return v[0] * v[1];\n"
         "}\n"
         "void bar(inout float x) {\n"
         "    float y[2], z;\n"
         "    y[0] = x;\n"
         "    y[1] = x * 2.0;\n"
         "    z = foo(y);\n"
         "    x = z;\n"
         "}\n"
         "void main() {\n"
         "    float x = 10.0;\n"
         "    bar(x);\n"
         "    gl_FragColor = vec4(x);\n"
         "}\n");
}

DEF_TEST(SkSLOperators, r) {
    test(r,
         "void main() {"
         "float x = 1, y = 2;"
         "int z = 3;"
         "x = x + y * z * x * (y - z);"
         "y = x / y / z;"
         "z = (z / 2 % 3 << 4) >> 2 << 1;"
         "bool b = (x > 4) == x < 2 || 2 >= 5 && y <= z && 12 != 11;"
         "x += 12;"
         "x -= 12;"
         "x *= y /= z = 10;"
         "b ||= false;"
         "b &&= true;"
         "b ^^= false;"
         "z |= 0;"
         "z &= -1;"
         "z ^= 0;"
         "z >>= 2;"
         "z <<= 4;"
         "z %= 5;"
         "}",
         default_caps(),
         "#version 400\n"
         "void main() {\n"
         "    float x = 1.0, y = 2.0;\n"
         "    int z = 3;\n"
         "    x = x + ((y * float(z)) * x) * (y - float(z));\n"
         "    y = (x / y) / float(z);\n"
         "    z = (((z / 2) % 3 << 4) >> 2) << 1;\n"
         "    bool b = x > 4.0 == x < 2.0 || (2 >= 5 && y <= float(z)) && 12 != 11;\n"
         "    x += 12.0;\n"
         "    x -= 12.0;\n"
         "    x *= (y /= float(z = 10));\n"
         "    b ||= false;\n"
         "    b &&= true;\n"
         "    b ^^= false;\n"
         "    z |= 0;\n"
         "    z &= -1;\n"
         "    z ^= 0;\n"
         "    z >>= 2;\n"
         "    z <<= 4;\n"
         "    z %= 5;\n"
         "}\n");
}

DEF_TEST(SkSLMatrices, r) {
    test(r,
         "void main() {"
         "mat2x4 x = mat2x4(1);"
         "mat3x2 y = mat3x2(1, 0, 0, 1, vec2(2, 2));"
         "mat3x4 z = x * y;"
         "vec3 v1 = mat3(1) * vec3(1);"
         "vec3 v2 = vec3(1) * mat3(1);"
         "}",
         default_caps(),
         "#version 400\n"
         "void main() {\n"
         "    mat2x4 x = mat2x4(1.0);\n"
         "    mat3x2 y = mat3x2(1.0, 0.0, 0.0, 1.0, vec2(2.0, 2.0));\n"
         "    mat3x4 z = x * y;\n"
         "    vec3 v1 = mat3(1.0) * vec3(1.0);\n"
         "    vec3 v2 = vec3(1.0) * mat3(1.0);\n"
         "}\n");
}

DEF_TEST(SkSLInterfaceBlock, r) {
    test(r,
         "uniform testBlock {"
         "float x;"
         "float y[2];"
         "layout(binding=12) mat3x2 z;"
         "bool w;"
         "};"
         "void main() {"
         "}",
         default_caps(),
         "#version 400\n"
         "uniform testBlock {\n"
         "    float x;\n"
         "    float[2] y;\n"
         "    layout (binding = 12) mat3x2 z;\n"
         "    bool w;\n"
         "};\n"
         "void main() {\n"
         "}\n");
}

DEF_TEST(SkSLStructs, r) {
    test(r,
         "struct A {"
         "int x;"
         "int y;"
         "} a1, a2;"
         "A a3;"
         "struct B {"
         "float x;"
         "float y[2];"
         "layout(binding=1) A z;"
         "};"
         "B b1, b2, b3;"
         "void main() {"
         "}",
         default_caps(),
         "#version 400\n"
         "struct A {\n"
         "    int x;\n"
         "    int y;\n"
         "}\n"
         " a1, a2;\n"
         "A a3;\n"
         "struct B {\n"
         "    float x;\n"
         "    float[2] y;\n"
         "    layout (binding = 1) A z;\n"
         "}\n"
         " b1, b2, b3;\n"
         "void main() {\n"
         "}\n");
}

DEF_TEST(SkSLVersion, r) {
    SkSL::GLCaps caps = default_caps();
    caps.fVersion = 450;
    caps.fIsCoreProfile = true;
    test(r,
         "in float test; void main() { sk_FragColor = vec4(0.75); }",
         caps,
         "#version 450 core\n"
         "in float test;\n"
         "void main() {\n"
         "    gl_FragColor = vec4(0.75);\n"
         "}\n");
    caps.fVersion = 110;
    caps.fIsCoreProfile = false;
    test(r,
         "in float test; void main() { sk_FragColor = vec4(0.75); }",
         caps,
         "#version 110\n"
         "varying float test;\n"
         "void main() {\n"
         "    gl_FragColor = vec4(0.75);\n"
         "}\n");
}

DEF_TEST(SkSLDeclareOutput, r) {
    SkSL::GLCaps caps = default_caps();    
    caps.fMustDeclareFragmentShaderOutput = true;
    test(r,
         "void main() { sk_FragColor = vec4(0.75); }",
         caps,
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(0.75);\n"
         "}\n");    
}

DEF_TEST(SkSLUsesPrecisionModifiers, r) {
    SkSL::GLCaps caps = default_caps();
    test(r,
         "void main() { float x = 0.75; highp float y = 1; }",
         caps,
         "#version 400\n"
         "void main() {\n"
         "    float x = 0.75;\n"
         "    float y = 1.0;\n"
         "}\n");    
    caps.fStandard = SkSL::GLCaps::kGLES_Standard;
    caps.fUsesPrecisionModifiers = true;
    test(r,
         "void main() { float x = 0.75; highp float y = 1; }",
         caps,
         "#version 400 es\n"
         "precision highp float;\n"
         "void main() {\n"
         "    float x = 0.75;\n"
         "    highp float y = 1.0;\n"
         "}\n");    
}

DEF_TEST(SkSLMinAbs, r) {
    test(r,
         "void main() {"
         "float x = -5;"
         "x = min(abs(x), 6);"
         "}",
         default_caps(),
         "#version 400\n"
         "void main() {\n"
         "    float x = -5.0;\n"
         "    x = min(abs(x), 6.0);\n"
         "}\n");

    SkSL::GLCaps caps = default_caps();
    caps.fCanUseMinAndAbsTogether = false;
    test(r,
         "void main() {"
         "float x = -5.0;"
         "x = min(abs(x), 6.0);"
         "}",
         caps,
         "#version 400\n"
         "void main() {\n"
         "    float minAbsHackVar0;\n"
         "    float x = -5.0;\n"
         "    x = (abs(x) > (minAbsHackVar0 = 6.0) ? minAbsHackVar0 : abs(x));\n"
         "}\n");
}

DEF_TEST(SkSLModifiersDeclaration, r) {
    test(r,
         "layout(blend_support_all_equations) out;"
         "void main() { }",
         default_caps(),
         "#version 400\n"
         "layout (blend_support_all_equations) out ;\n"
         "void main() {\n"
         "}\n");
}

DEF_TEST(SkSLHex, r) {
    test(r,
         "void main() {"
         "int i1 = 0x0;"
         "int i2 = 0x1234abcd;"
         "int i3 = 0x7fffffff;"
         "int i4 = 0xffffffff;"
         "int i5 = -0xbeef;"
         "uint u1 = 0x0;"
         "uint u2 = 0x1234abcd;"
         "uint u3 = 0x7fffffff;"
         "uint u4 = 0xffffffff;"
         "}",
         default_caps(),
         "#version 400\n"
         "void main() {\n"
         "    int i1 = 0;\n"
         "    int i2 = 305441741;\n"
         "    int i3 = 2147483647;\n"
         "    int i4 = -1;\n"
         "    int i5 = -48879;\n"
         "    uint u1 = 0u;\n"
         "    uint u2 = 305441741u;\n"
         "    uint u3 = 2147483647u;\n"
         "    uint u4 = 4294967295u;\n"
         "}\n");
}

DEF_TEST(SkSLArrayConstructors, r) {
    test(r,
         "float test1[] = float[](1, 2, 3, 4);"
         "vec2 test2[] = vec2[](vec2(1, 2), vec2(3, 4));"
         "mat4 test3[] = mat4[]();",
         default_caps(),
         "#version 400\n"
         "float test1[] = float[](1.0, 2.0, 3.0, 4.0);\n"
         "vec2 test2[] = vec2[](vec2(1.0, 2.0), vec2(3.0, 4.0));\n"
         "mat4 test3[] = mat4[]();\n");
}
