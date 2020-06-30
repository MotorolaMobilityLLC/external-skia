/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STANDALONE

#include "include/core/SkPoint3.h"
#include "include/private/SkVx.h"
#include "src/core/SkUtils.h"   // sk_unaligned_load
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLByteCodeGenerator.h"
#include "src/sksl/SkSLExternalValue.h"

#include <vector>

namespace SkSL {

#if defined(SK_ENABLE_SKSL_INTERPRETER)

constexpr int VecWidth = ByteCode::kVecWidth;

struct Interpreter {

using F32 = skvx::Vec<VecWidth, float>;
using I32 = skvx::Vec<VecWidth, int32_t>;
using U32 = skvx::Vec<VecWidth, uint32_t>;

#define READ8() (*(ip++))
#define READ16() (ip += 2, sk_unaligned_load<uint16_t>(ip - 2))
#define READ32() (ip += 4, sk_unaligned_load<uint32_t>(ip - 4))
#define READ_INST() (ip += sizeof(ByteCodeInstruction), \
                     sk_unaligned_load<ByteCodeInstruction>(ip - sizeof(ByteCodeInstruction)))

#define VECTOR_DISASSEMBLE(op, text)                          \
    case ByteCodeInstruction::op: printf(text); break;        \
    case ByteCodeInstruction::op##2: printf(text "2"); break; \
    case ByteCodeInstruction::op##3: printf(text "3"); break; \
    case ByteCodeInstruction::op##4: printf(text "4"); break;

#define VECTOR_MATRIX_DISASSEMBLE(op, text) \
    VECTOR_DISASSEMBLE(op, text)            \
    case ByteCodeInstruction::op##N: printf(text "N %d", READ8()); break;

static const uint8_t* DisassembleInstruction(const uint8_t* ip) {
    auto inst = READ_INST();
    printf("%04x ", (int)inst);
    switch (inst) {
        VECTOR_MATRIX_DISASSEMBLE(kAddF, "addf")
        VECTOR_DISASSEMBLE(kAddI, "addi")
        case ByteCodeInstruction::kAndB: printf("andb"); break;
        VECTOR_DISASSEMBLE(kATan, "atan")
        case ByteCodeInstruction::kBranch: printf("branch %d", READ16()); break;
        case ByteCodeInstruction::kCall: printf("call %d", READ8()); break;
        case ByteCodeInstruction::kCallExternal: {
            int argumentCount = READ8();
            int returnCount = READ8();
            int externalValue = READ8();
            printf("callexternal %d, %d, %d", argumentCount, returnCount, externalValue);
            break;
        }
        VECTOR_DISASSEMBLE(kCeil, "ceil")
        case ByteCodeInstruction::kClampIndex: printf("clampindex %d", READ8()); break;
        VECTOR_DISASSEMBLE(kCompareIEQ, "compareieq")
        VECTOR_DISASSEMBLE(kCompareINEQ, "compareineq")
        VECTOR_MATRIX_DISASSEMBLE(kCompareFEQ, "comparefeq")
        VECTOR_MATRIX_DISASSEMBLE(kCompareFNEQ, "comparefneq")
        VECTOR_DISASSEMBLE(kCompareFGT, "comparefgt")
        VECTOR_DISASSEMBLE(kCompareFGTEQ, "comparefgteq")
        VECTOR_DISASSEMBLE(kCompareFLT, "compareflt")
        VECTOR_DISASSEMBLE(kCompareFLTEQ, "compareflteq")
        VECTOR_DISASSEMBLE(kCompareSGT, "comparesgt")
        VECTOR_DISASSEMBLE(kCompareSGTEQ, "comparesgteq")
        VECTOR_DISASSEMBLE(kCompareSLT, "compareslt")
        VECTOR_DISASSEMBLE(kCompareSLTEQ, "compareslteq")
        VECTOR_DISASSEMBLE(kCompareUGT, "compareugt")
        VECTOR_DISASSEMBLE(kCompareUGTEQ, "compareugteq")
        VECTOR_DISASSEMBLE(kCompareULT, "compareult")
        VECTOR_DISASSEMBLE(kCompareULTEQ, "compareulteq")
        VECTOR_DISASSEMBLE(kConvertFtoI, "convertftoi")
        VECTOR_DISASSEMBLE(kConvertStoF, "convertstof")
        VECTOR_DISASSEMBLE(kConvertUtoF, "convertutof")
        VECTOR_DISASSEMBLE(kCos, "cos")
        VECTOR_MATRIX_DISASSEMBLE(kDivideF, "dividef")
        VECTOR_DISASSEMBLE(kDivideS, "divideS")
        VECTOR_DISASSEMBLE(kDivideU, "divideu")
        VECTOR_MATRIX_DISASSEMBLE(kDup, "dup")
        VECTOR_DISASSEMBLE(kFloor, "floor")
        VECTOR_DISASSEMBLE(kFract, "fract")
        case ByteCodeInstruction::kInverse2x2: printf("inverse2x2"); break;
        case ByteCodeInstruction::kInverse3x3: printf("inverse3x3"); break;
        case ByteCodeInstruction::kInverse4x4: printf("inverse4x4"); break;
        VECTOR_DISASSEMBLE(kLerp, "lerp")
        case ByteCodeInstruction::kLoad: printf("load %d", READ8()); break;
        case ByteCodeInstruction::kLoad2: printf("load2 %d", READ8()); break;
        case ByteCodeInstruction::kLoad3: printf("load3 %d", READ8()); break;
        case ByteCodeInstruction::kLoad4: printf("load4 %d", READ8()); break;
        case ByteCodeInstruction::kLoadGlobal: printf("loadglobal %d", READ8()); break;
        case ByteCodeInstruction::kLoadGlobal2: printf("loadglobal2 %d", READ8()); break;
        case ByteCodeInstruction::kLoadGlobal3: printf("loadglobal3 %d", READ8()); break;
        case ByteCodeInstruction::kLoadGlobal4: printf("loadglobal4 %d", READ8()); break;
        case ByteCodeInstruction::kLoadUniform: printf("loaduniform %d", READ8()); break;
        case ByteCodeInstruction::kLoadUniform2: printf("loaduniform2 %d", READ8()); break;
        case ByteCodeInstruction::kLoadUniform3: printf("loaduniform3 %d", READ8()); break;
        case ByteCodeInstruction::kLoadUniform4: printf("loaduniform4 %d", READ8()); break;
        case ByteCodeInstruction::kLoadExtended: printf("loadextended %d", READ8()); break;
        case ByteCodeInstruction::kLoadExtendedGlobal: printf("loadextendedglobal %d", READ8());
            break;
        case ByteCodeInstruction::kLoadExtendedUniform: printf("loadextendeduniform %d", READ8());
            break;
        case ByteCodeInstruction::kLoadFragCoord: printf("loadfragcoord"); break;
        case ByteCodeInstruction::kMatrixToMatrix: {
            int srcCols = READ8();
            int srcRows = READ8();
            int dstCols = READ8();
            int dstRows = READ8();
            printf("matrixtomatrix %dx%d %dx%d", srcCols, srcRows, dstCols, dstRows);
            break;
        }
        case ByteCodeInstruction::kMatrixMultiply: {
            int lCols = READ8();
            int lRows = READ8();
            int rCols = READ8();
            printf("matrixmultiply %dx%d %dx%d", lCols, lRows, rCols, lCols);
            break;
        }
        VECTOR_DISASSEMBLE(kMaxF, "maxf")
        VECTOR_DISASSEMBLE(kMaxS, "maxs")
        VECTOR_DISASSEMBLE(kMinF, "minf")
        VECTOR_DISASSEMBLE(kMinS, "mins")
        VECTOR_DISASSEMBLE(kMix, "mix")
        VECTOR_MATRIX_DISASSEMBLE(kMultiplyF, "multiplyf")
        VECTOR_DISASSEMBLE(kMultiplyI, "multiplyi")
        VECTOR_MATRIX_DISASSEMBLE(kNegateF, "negatef")
        VECTOR_DISASSEMBLE(kNegateI, "negatei")
        VECTOR_DISASSEMBLE(kNotB, "notb")
        case ByteCodeInstruction::kOrB: printf("orb"); break;
        VECTOR_MATRIX_DISASSEMBLE(kPop, "pop")
        VECTOR_DISASSEMBLE(kPow, "pow")
        case ByteCodeInstruction::kPushImmediate: {
            uint32_t v = READ32();
            union { uint32_t u; float f; } pun = { v };
            printf("pushimmediate %s", (to_string(v) + "(" + to_string(pun.f) + ")").c_str());
            break;
        }
        case ByteCodeInstruction::kReadExternal: printf("readexternal %d", READ8()); break;
        case ByteCodeInstruction::kReadExternal2: printf("readexternal2 %d", READ8()); break;
        case ByteCodeInstruction::kReadExternal3: printf("readexternal3 %d", READ8()); break;
        case ByteCodeInstruction::kReadExternal4: printf("readexternal4 %d", READ8()); break;
        VECTOR_DISASSEMBLE(kRemainderF, "remainderf")
        VECTOR_DISASSEMBLE(kRemainderS, "remainders")
        VECTOR_DISASSEMBLE(kRemainderU, "remainderu")
        case ByteCodeInstruction::kReserve: printf("reserve %d", READ8()); break;
        case ByteCodeInstruction::kReturn: printf("return %d", READ8()); break;
        case ByteCodeInstruction::kSampleExplicit: printf("sample %d", READ8()); break;
        case ByteCodeInstruction::kSampleMatrix: printf("sampleMtx %d", READ8()); break;
        case ByteCodeInstruction::kScalarToMatrix: {
            int cols = READ8();
            int rows = READ8();
            printf("scalartomatrix %dx%d", cols, rows);
            break;
        }
        case ByteCodeInstruction::kShiftLeft: printf("shl %d", READ8()); break;
        case ByteCodeInstruction::kShiftRightS: printf("shrs %d", READ8()); break;
        case ByteCodeInstruction::kShiftRightU: printf("shru %d", READ8()); break;
        VECTOR_DISASSEMBLE(kSin, "sin")
        VECTOR_DISASSEMBLE(kSqrt, "sqrt")
        case ByteCodeInstruction::kStore: printf("store %d", READ8()); break;
        case ByteCodeInstruction::kStore2: printf("store2 %d", READ8()); break;
        case ByteCodeInstruction::kStore3: printf("store3 %d", READ8()); break;
        case ByteCodeInstruction::kStore4: printf("store4 %d", READ8()); break;
        case ByteCodeInstruction::kStoreGlobal: printf("storeglobal %d", READ8()); break;
        case ByteCodeInstruction::kStoreGlobal2: printf("storeglobal2 %d", READ8()); break;
        case ByteCodeInstruction::kStoreGlobal3: printf("storeglobal3 %d", READ8()); break;
        case ByteCodeInstruction::kStoreGlobal4: printf("storeglobal4 %d", READ8()); break;
        case ByteCodeInstruction::kStoreExtended: printf("storeextended %d", READ8()); break;
        case ByteCodeInstruction::kStoreExtendedGlobal: printf("storeextendedglobal %d", READ8());
            break;
        VECTOR_MATRIX_DISASSEMBLE(kSubtractF, "subtractf")
        VECTOR_DISASSEMBLE(kSubtractI, "subtracti")
        case ByteCodeInstruction::kSwizzle: {
            printf("swizzle %d, ", READ8());
            int count = READ8();
            printf("%d", count);
            for (int i = 0; i < count; ++i) {
                printf(", %d", READ8());
            }
            break;
        }
        VECTOR_DISASSEMBLE(kTan, "tan")
        case ByteCodeInstruction::kWriteExternal: printf("writeexternal %d", READ8()); break;
        case ByteCodeInstruction::kWriteExternal2: printf("writeexternal2 %d", READ8()); break;
        case ByteCodeInstruction::kWriteExternal3: printf("writeexternal3 %d", READ8()); break;
        case ByteCodeInstruction::kWriteExternal4: printf("writeexternal4 %d", READ8()); break;
        case ByteCodeInstruction::kXorB: printf("xorb"); break;
        case ByteCodeInstruction::kMaskPush: printf("maskpush"); break;
        case ByteCodeInstruction::kMaskPop: printf("maskpop"); break;
        case ByteCodeInstruction::kMaskNegate: printf("masknegate"); break;
        case ByteCodeInstruction::kMaskBlend: printf("maskblend %d", READ8()); break;
        case ByteCodeInstruction::kBranchIfAllFalse:
            printf("branchifallfalse %d", READ16());
            break;
        case ByteCodeInstruction::kLoopBegin: printf("loopbegin"); break;
        case ByteCodeInstruction::kLoopNext: printf("loopnext"); break;
        case ByteCodeInstruction::kLoopMask: printf("loopmask"); break;
        case ByteCodeInstruction::kLoopEnd: printf("loopend"); break;
        case ByteCodeInstruction::kLoopContinue: printf("loopcontinue"); break;
        case ByteCodeInstruction::kLoopBreak: printf("loopbreak"); break;
        default:
            ip -= sizeof(ByteCodeInstruction);
            printf("unknown(%d)\n", (int) (intptr_t) READ_INST());
            SkASSERT(false);
    }
    return ip;
}

#define VECTOR_BINARY_OP(base, field, op)                             \
    case ByteCodeInstruction::base ## 4: {                            \
        sp[-4] = sp[-4].field op sp[0].field;                         \
        POP();                                                        \
        [[fallthrough]];                                              \
    }                                                                 \
    case ByteCodeInstruction::base ## 3: {                            \
        int count = (int)inst - (int)(ByteCodeInstruction::base) - 1; \
        sp[count] = sp[count].field op sp[0].field;                   \
        POP();                                                        \
        [[fallthrough]];                                              \
    }                                                                 \
    case ByteCodeInstruction::base ## 2: {                            \
        int count = (int)inst - (int)(ByteCodeInstruction::base) - 1; \
        sp[count] = sp[count].field op sp[0].field;                   \
        POP();                                                        \
        [[fallthrough]];                                              \
    }                                                                 \
    case ByteCodeInstruction::base: {                                 \
        int count = (int)inst - (int)(ByteCodeInstruction::base) - 1; \
        sp[count] = sp[count].field op sp[0].field;                   \
        POP();                                                        \
        continue;                                                     \
    }

// A naive implementation of / or % using skvx operations will likely crash with a divide by zero
// in inactive vector lanes, so we need to be sure to avoid masked-off lanes.
#define VECTOR_BINARY_MASKED_OP(base, field, op)                      \
    case ByteCodeInstruction::base ## 4: {                            \
        for (int i = 0; i < VecWidth; ++i) {                          \
            if (mask()[i]) {                                          \
                sp[-4].field[i] op ## = sp[0].field[i];               \
            }                                                         \
        }                                                             \
        POP();                                                        \
        [[fallthrough]];                                              \
    }                                                                 \
    case ByteCodeInstruction::base ## 3: {                            \
        int count = (int)inst - (int)(ByteCodeInstruction::base) - 1; \
        for (int i = 0; i < VecWidth; ++i) {                          \
            if (mask()[i]) {                                          \
                sp[count].field[i] op ## = sp[0].field[i];            \
            }                                                         \
        }                                                             \
        POP();                                                        \
        [[fallthrough]];                                              \
    }                                                                 \
    case ByteCodeInstruction::base ## 2: {                            \
        int count = (int)inst - (int)(ByteCodeInstruction::base) - 1; \
        for (int i = 0; i < VecWidth; ++i) {                          \
            if (mask()[i]) {                                          \
                sp[count].field[i] op ## = sp[0].field[i];            \
            }                                                         \
        }                                                             \
        POP();                                                        \
        [[fallthrough]];                                              \
    }                                                                 \
    case ByteCodeInstruction::base: {                                 \
        int count = (int)inst - (int)(ByteCodeInstruction::base) - 1; \
        for (int i = 0; i < VecWidth; ++i) {                          \
            if (mask()[i]) {                                          \
                sp[count].field[i] op ## = sp[0].field[i];            \
            }                                                         \
        }                                                             \
        POP();                                                        \
        continue;                                                     \
    }


#define VECTOR_MATRIX_BINARY_OP(base, field, op)          \
    VECTOR_BINARY_OP(base, field, op)                     \
    case ByteCodeInstruction::base ## N: {                \
        int count = READ8();                              \
        for (int i = count; i > 0; --i) {                 \
            sp[-count] = sp[-count].field op sp[0].field; \
            POP();                                        \
        }                                                 \
        continue;                                         \
    }

#define VECTOR_BINARY_FN(base, field, fn)                             \
    case ByteCodeInstruction::base ## 4: {                            \
        sp[-4] = fn(sp[-4].field, sp[0].field);                       \
        POP();                                                        \
        [[fallthrough]];                                              \
    }                                                                 \
    case ByteCodeInstruction::base ## 3: {                            \
        int count = (int)inst - (int)(ByteCodeInstruction::base) - 1; \
        sp[count] = fn(sp[count].field, sp[0].field);                 \
        POP();                                                        \
        [[fallthrough]];                                              \
    }                                                                 \
    case ByteCodeInstruction::base ## 2: {                            \
        int count = (int)inst - (int)(ByteCodeInstruction::base) - 1; \
        sp[count] = fn(sp[count].field, sp[0].field);                 \
        POP();                                                        \
        [[fallthrough]];                                              \
    }                                                                 \
    case ByteCodeInstruction::base: {                                 \
        int count = (int)inst - (int)(ByteCodeInstruction::base) - 1; \
        sp[count] = fn(sp[count].field, sp[0].field);                 \
        POP();                                                        \
        continue;                                                     \
    }

#define VECTOR_UNARY_FN(base, fn, field)                                              \
    case ByteCodeInstruction::base ## 4:  sp[-3] = fn(sp[-3].field); [[fallthrough]]; \
    case ByteCodeInstruction::base ## 3:  sp[-2] = fn(sp[-2].field); [[fallthrough]]; \
    case ByteCodeInstruction::base ## 2:  sp[-1] = fn(sp[-1].field); [[fallthrough]]; \
    case ByteCodeInstruction::base:       sp[ 0] = fn(sp[ 0].field);                  \
                      continue;

union VValue {
    VValue() {}
    VValue(F32 f) : fFloat(f) {}
    VValue(I32 s) : fSigned(s) {}
    VValue(U32 u) : fUnsigned(u) {}

    F32 fFloat;
    I32 fSigned;
    U32 fUnsigned;
};

struct StackFrame {
    const uint8_t* fCode;
    const uint8_t* fIP;
    VValue* fStack;
    int fParameterCount;
};

static F32 VecMod(F32 a, F32 b) {
    return a - skvx::trunc(a / b) * b;
}

#define spf(index)  sp[index].fFloat

static void CallExternal(const ByteCode* byteCode, const uint8_t*& ip, VValue*& sp,
                          int baseIndex, I32 mask) {
    int argumentCount = READ8();
    int returnCount = READ8();
    int target = READ8();
    ExternalValue* v = byteCode->fExternalValues[target];
    sp -= argumentCount - 1;

    float tmpArgs[4];
    float tmpReturn[4];
    SkASSERT(argumentCount <= (int)SK_ARRAY_COUNT(tmpArgs));
    SkASSERT(returnCount <= (int)SK_ARRAY_COUNT(tmpReturn));

    for (int i = 0; i < VecWidth; ++i) {
        if (mask[i]) {
            for (int j = 0; j < argumentCount; ++j) {
                tmpArgs[j] = sp[j].fFloat[i];
            }
            v->call(baseIndex + i, tmpArgs, tmpReturn);
            for (int j = 0; j < returnCount; ++j) {
                sp[j].fFloat[i] = tmpReturn[j];
            }
        }
    }
    sp += returnCount - 1;
}

static void Inverse2x2(VValue* sp) {
    F32 a = sp[-3].fFloat,
        b = sp[-2].fFloat,
        c = sp[-1].fFloat,
        d = sp[ 0].fFloat;
    F32 idet = F32(1) / (a*d - b*c);
    sp[-3].fFloat = d * idet;
    sp[-2].fFloat = -b * idet;
    sp[-1].fFloat = -c * idet;
    sp[ 0].fFloat = a * idet;
}

static void Inverse3x3(VValue* sp) {
    F32 a11 = sp[-8].fFloat, a12 = sp[-5].fFloat, a13 = sp[-2].fFloat,
        a21 = sp[-7].fFloat, a22 = sp[-4].fFloat, a23 = sp[-1].fFloat,
        a31 = sp[-6].fFloat, a32 = sp[-3].fFloat, a33 = sp[ 0].fFloat;
    F32 idet = F32(1) / (a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 -
                         a11 * a23 * a32 - a12 * a21 * a33 - a13 * a22 * a31);
    sp[-8].fFloat = (a22 * a33 - a23 * a32) * idet;
    sp[-7].fFloat = (a23 * a31 - a21 * a33) * idet;
    sp[-6].fFloat = (a21 * a32 - a22 * a31) * idet;
    sp[-5].fFloat = (a13 * a32 - a12 * a33) * idet;
    sp[-4].fFloat = (a11 * a33 - a13 * a31) * idet;
    sp[-3].fFloat = (a12 * a31 - a11 * a32) * idet;
    sp[-2].fFloat = (a12 * a23 - a13 * a22) * idet;
    sp[-1].fFloat = (a13 * a21 - a11 * a23) * idet;
    sp[ 0].fFloat = (a11 * a22 - a12 * a21) * idet;
}

static void Inverse4x4(VValue* sp) {
    F32 a00 = spf(-15), a10 = spf(-11), a20 = spf( -7), a30 = spf( -3),
        a01 = spf(-14), a11 = spf(-10), a21 = spf( -6), a31 = spf( -2),
        a02 = spf(-13), a12 = spf( -9), a22 = spf( -5), a32 = spf( -1),
        a03 = spf(-12), a13 = spf( -8), a23 = spf( -4), a33 = spf(  0);

    F32 b00 = a00 * a11 - a01 * a10,
        b01 = a00 * a12 - a02 * a10,
        b02 = a00 * a13 - a03 * a10,
        b03 = a01 * a12 - a02 * a11,
        b04 = a01 * a13 - a03 * a11,
        b05 = a02 * a13 - a03 * a12,
        b06 = a20 * a31 - a21 * a30,
        b07 = a20 * a32 - a22 * a30,
        b08 = a20 * a33 - a23 * a30,
        b09 = a21 * a32 - a22 * a31,
        b10 = a21 * a33 - a23 * a31,
        b11 = a22 * a33 - a23 * a32;

    F32 idet = F32(1) /
               (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

    b00 *= idet;
    b01 *= idet;
    b02 *= idet;
    b03 *= idet;
    b04 *= idet;
    b05 *= idet;
    b06 *= idet;
    b07 *= idet;
    b08 *= idet;
    b09 *= idet;
    b10 *= idet;
    b11 *= idet;

    spf(-15) = a11 * b11 - a12 * b10 + a13 * b09;
    spf(-14) = a02 * b10 - a01 * b11 - a03 * b09;
    spf(-13) = a31 * b05 - a32 * b04 + a33 * b03;
    spf(-12) = a22 * b04 - a21 * b05 - a23 * b03;
    spf(-11) = a12 * b08 - a10 * b11 - a13 * b07;
    spf(-10) = a00 * b11 - a02 * b08 + a03 * b07;
    spf( -9) = a32 * b02 - a30 * b05 - a33 * b01;
    spf( -8) = a20 * b05 - a22 * b02 + a23 * b01;
    spf( -7) = a10 * b10 - a11 * b08 + a13 * b06;
    spf( -6) = a01 * b08 - a00 * b10 - a03 * b06;
    spf( -5) = a30 * b04 - a31 * b02 + a33 * b00;
    spf( -4) = a21 * b02 - a20 * b04 - a23 * b00;
    spf( -3) = a11 * b07 - a10 * b09 - a12 * b06;
    spf( -2) = a00 * b09 - a01 * b07 + a02 * b06;
    spf( -1) = a31 * b01 - a30 * b03 - a32 * b00;
    spf(  0) = a20 * b03 - a21 * b01 + a22 * b00;
}

static bool InnerRun(const ByteCode* byteCode, const ByteCodeFunction* f, VValue* stack,
                     float* outReturn[], VValue globals[], const float uniforms[],
                     bool stripedOutput, int N, int baseIndex) {
    // Needs to be the first N non-negative integers, at least as large as VecWidth
    static const Interpreter::I32 gLanes = {
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
    };

    VValue* sp = stack + f->fParameterCount + f->fLocalCount - 1;

    #define POP() (*(sp--))
    #define PUSH(v) (sp[1] = v, ++sp)

    const uint8_t* code = f->fCode.data();
    const uint8_t* ip = code;
    std::vector<StackFrame> frames;

    I32 condStack[16];  // Independent condition masks
    I32 maskStack[16];  // Combined masks (eg maskStack[0] & maskStack[1] & ...)
    I32 contStack[16];  // Continue flags for loops
    I32 loopStack[16];  // Loop execution masks
    condStack[0] = maskStack[0] = (gLanes < N);
    contStack[0] = I32( 0);
    loopStack[0] = I32(~0);
    I32* condPtr = condStack;
    I32* maskPtr = maskStack;
    I32* contPtr = contStack;
    I32* loopPtr = loopStack;

    if (f->fConditionCount + 1 > (int)SK_ARRAY_COUNT(condStack) ||
        f->fLoopCount + 1 > (int)SK_ARRAY_COUNT(loopStack)) {
        return false;
    }

    auto mask = [&]() { return *maskPtr & *loopPtr; };

    for (;;) {
#ifdef TRACE
        printf("at %3d ", (int) (ip - code));
        disassemble_instruction(ip);
        printf(" (stack: %d)\n", (int) (sp - stack) + 1);
#endif
        ByteCodeInstruction inst = READ_INST();
        switch (inst) {

            VECTOR_MATRIX_BINARY_OP(kAddF, fFloat, +)
            VECTOR_BINARY_OP(kAddI, fSigned, +)

            // Booleans are integer masks: 0/~0 for false/true. So bitwise ops do what we want:
            case ByteCodeInstruction::kAndB:
                sp[-1] = sp[-1].fSigned & sp[0].fSigned;
                POP();
                continue;
            case ByteCodeInstruction::kNotB4: sp[-3] = ~sp[-3].fSigned; [[fallthrough]];
            case ByteCodeInstruction::kNotB3: sp[-2] = ~sp[-2].fSigned; [[fallthrough]];
            case ByteCodeInstruction::kNotB2: sp[-1] = ~sp[-1].fSigned; [[fallthrough]];
            case ByteCodeInstruction::kNotB:  sp[ 0] = ~sp[ 0].fSigned;
                continue;
            case ByteCodeInstruction::kOrB:
                sp[-1] = sp[-1].fSigned | sp[0].fSigned;
                POP();
                continue;
            case ByteCodeInstruction::kXorB:
                sp[-1] = sp[-1].fSigned ^ sp[0].fSigned;
                POP();
                continue;

            case ByteCodeInstruction::kBranch:
                ip = code + READ16();
                continue;

            case ByteCodeInstruction::kCall: {
                // Precursor code reserved space for the return value, and pushed all parameters to
                // the stack. Update our bottom of stack to point at the first parameter, and our
                // sp to point past those parameters (plus space for locals).
                int target = READ8();
                const ByteCodeFunction* f = byteCode->fFunctions[target].get();
                if (skvx::any(mask())) {
                    frames.push_back({ code, ip, stack, f->fParameterCount });
                    ip = code = f->fCode.data();
                    stack = sp - f->fParameterCount + 1;
                    sp = stack + f->fParameterCount + f->fLocalCount - 1;
                    // As we did in runStriped(), zero locals so they're safe to mask-store into.
                    for (int i = f->fParameterCount; i < f->fParameterCount + f->fLocalCount; i++) {
                        stack[i].fFloat = 0.0f;
                    }
                }
                continue;
            }

            case ByteCodeInstruction::kCallExternal: {
                CallExternal(byteCode, ip, sp, baseIndex, mask());
                continue;
            }

            VECTOR_UNARY_FN(kCeil, skvx::ceil, fFloat)

            case ByteCodeInstruction::kClampIndex: {
                int length = READ8();
                if (skvx::any(mask() & ((sp[0].fSigned < 0) | (sp[0].fSigned >= length)))) {
                    return false;
                }
                continue;
            }

            VECTOR_BINARY_OP(kCompareIEQ, fSigned, ==)
            VECTOR_MATRIX_BINARY_OP(kCompareFEQ, fFloat, ==)
            VECTOR_BINARY_OP(kCompareINEQ, fSigned, !=)
            VECTOR_MATRIX_BINARY_OP(kCompareFNEQ, fFloat, !=)
            VECTOR_BINARY_OP(kCompareSGT, fSigned, >)
            VECTOR_BINARY_OP(kCompareUGT, fUnsigned, >)
            VECTOR_BINARY_OP(kCompareFGT, fFloat, >)
            VECTOR_BINARY_OP(kCompareSGTEQ, fSigned, >=)
            VECTOR_BINARY_OP(kCompareUGTEQ, fUnsigned, >=)
            VECTOR_BINARY_OP(kCompareFGTEQ, fFloat, >=)
            VECTOR_BINARY_OP(kCompareSLT, fSigned, <)
            VECTOR_BINARY_OP(kCompareULT, fUnsigned, <)
            VECTOR_BINARY_OP(kCompareFLT, fFloat, <)
            VECTOR_BINARY_OP(kCompareSLTEQ, fSigned, <=)
            VECTOR_BINARY_OP(kCompareULTEQ, fUnsigned, <=)
            VECTOR_BINARY_OP(kCompareFLTEQ, fFloat, <=)

            case ByteCodeInstruction::kConvertFtoI4:
                sp[-3] = skvx::cast<int>(sp[-3].fFloat);
                [[fallthrough]];
            case ByteCodeInstruction::kConvertFtoI3:
                sp[-2] = skvx::cast<int>(sp[-2].fFloat);
                [[fallthrough]];
            case ByteCodeInstruction::kConvertFtoI2:
                sp[-1] = skvx::cast<int>(sp[-1].fFloat);
                [[fallthrough]];
            case ByteCodeInstruction::kConvertFtoI:
                sp[ 0] = skvx::cast<int>(sp[ 0].fFloat);
                continue;

            case ByteCodeInstruction::kConvertStoF4:
                sp[-3] = skvx::cast<float>(sp[-3].fSigned);
                [[fallthrough]];
            case ByteCodeInstruction::kConvertStoF3:
                sp[-2] = skvx::cast<float>(sp[-2].fSigned);
                [[fallthrough]];
            case ByteCodeInstruction::kConvertStoF2:
                sp[-1] = skvx::cast<float>(sp[-1].fSigned);
                [[fallthrough]];
            case ByteCodeInstruction::kConvertStoF:
                sp[ 0] = skvx::cast<float>(sp[ 0].fSigned);
                continue;

            case ByteCodeInstruction::kConvertUtoF4:
                sp[-3] = skvx::cast<float>(sp[-3].fUnsigned);
                [[fallthrough]];
            case ByteCodeInstruction::kConvertUtoF3:
                sp[-2] = skvx::cast<float>(sp[-2].fUnsigned);
                [[fallthrough]];
            case ByteCodeInstruction::kConvertUtoF2:
                sp[-1] = skvx::cast<float>(sp[-1].fUnsigned);
                [[fallthrough]];
            case ByteCodeInstruction::kConvertUtoF:
                sp[ 0] = skvx::cast<float>(sp[ 0].fUnsigned);
                continue;

            VECTOR_UNARY_FN(kCos, skvx::cos, fFloat)

            VECTOR_BINARY_MASKED_OP(kDivideS, fSigned, /)
            VECTOR_BINARY_MASKED_OP(kDivideU, fUnsigned, /)
            VECTOR_MATRIX_BINARY_OP(kDivideF, fFloat, /)

            case ByteCodeInstruction::kDup4:
                PUSH(sp[(int)inst - (int)ByteCodeInstruction::kDup]);
                [[fallthrough]];
            case ByteCodeInstruction::kDup3:
                PUSH(sp[(int)inst - (int)ByteCodeInstruction::kDup]);
                [[fallthrough]];
            case ByteCodeInstruction::kDup2:
                PUSH(sp[(int)inst - (int)ByteCodeInstruction::kDup]);
                [[fallthrough]];
            case ByteCodeInstruction::kDup :
                PUSH(sp[(int)inst - (int)ByteCodeInstruction::kDup]);
                continue;

            case ByteCodeInstruction::kDupN: {
                int count = READ8();
                memcpy(sp + 1, sp - count + 1, count * sizeof(VValue));
                sp += count;
                continue;
            }

            VECTOR_UNARY_FN(kFloor, skvx::floor, fFloat)
            VECTOR_UNARY_FN(kFract, skvx::fract, fFloat)

            case ByteCodeInstruction::kInverse2x2:
                Inverse2x2(sp);
                continue;
            case ByteCodeInstruction::kInverse3x3:
                Inverse3x3(sp);
                continue;
            case ByteCodeInstruction::kInverse4x4:
                Inverse4x4(sp);
                continue;

            case ByteCodeInstruction::kLerp4:
            case ByteCodeInstruction::kLerp3:
            case ByteCodeInstruction::kLerp2:
            case ByteCodeInstruction::kLerp: {
                int count = (int)ByteCodeInstruction::kLerp - (int)inst + 1;
                VValue* T = sp - count + 1,
                      * B = T - count,
                      * A = B - count;
                for (int i = count; i --> 0; ) {
                    A[i].fFloat += (B[i].fFloat - A[i].fFloat) * T[i].fFloat;
                }
                sp -= 2 * count;
                continue;
            }

            case ByteCodeInstruction::kLoad4: sp[4] = stack[*ip + 3]; [[fallthrough]];
            case ByteCodeInstruction::kLoad3: sp[3] = stack[*ip + 2]; [[fallthrough]];
            case ByteCodeInstruction::kLoad2: sp[2] = stack[*ip + 1]; [[fallthrough]];
            case ByteCodeInstruction::kLoad:  sp[1] = stack[*ip + 0];
                        ++ip;
                        sp += (int)ByteCodeInstruction::kLoad - (int)inst + 1;
                        continue;

            case ByteCodeInstruction::kLoadGlobal4: sp[4] = globals[*ip + 3]; [[fallthrough]];
            case ByteCodeInstruction::kLoadGlobal3: sp[3] = globals[*ip + 2]; [[fallthrough]];
            case ByteCodeInstruction::kLoadGlobal2: sp[2] = globals[*ip + 1]; [[fallthrough]];
            case ByteCodeInstruction::kLoadGlobal:  sp[1] = globals[*ip + 0];
                                ++ip;
                                sp += (int)ByteCodeInstruction::kLoadGlobal - (int)inst + 1;
                                continue;

            case ByteCodeInstruction::kLoadUniform4:
                sp[4].fFloat = uniforms[*ip + 3];
                [[fallthrough]];
            case ByteCodeInstruction::kLoadUniform3:
                sp[3].fFloat = uniforms[*ip + 2];
                [[fallthrough]];
            case ByteCodeInstruction::kLoadUniform2:
                sp[2].fFloat = uniforms[*ip + 1];
                [[fallthrough]];
            case ByteCodeInstruction::kLoadUniform:
                sp[1].fFloat = uniforms[*ip + 0];
                ++ip;
                sp += (int)ByteCodeInstruction::kLoadUniform - (int)inst + 1;
                continue;

            case ByteCodeInstruction::kLoadExtended: {
                int count = READ8();
                I32 src = POP().fSigned;
                I32 m = mask();
                for (int i = 0; i < count; ++i) {
                    for (int j = 0; j < VecWidth; ++j) {
                        if (m[j]) {
                            sp[i + 1].fSigned[j] = stack[src[j] + i].fSigned[j];
                        }
                    }
                }
                sp += count;
                continue;
            }

            case ByteCodeInstruction::kLoadExtendedGlobal: {
                int count = READ8();
                I32 src = POP().fSigned;
                I32 m = mask();
                for (int i = 0; i < count; ++i) {
                    for (int j = 0; j < VecWidth; ++j) {
                        if (m[j]) {
                            sp[i + 1].fSigned[j] = globals[src[j] + i].fSigned[j];
                        }
                    }
                }
                sp += count;
                continue;
            }

            case ByteCodeInstruction::kLoadExtendedUniform: {
                int count = READ8();
                I32 src = POP().fSigned;
                I32 m = mask();
                for (int i = 0; i < count; ++i) {
                    for (int j = 0; j < VecWidth; ++j) {
                        if (m[j]) {
                            sp[i + 1].fFloat[j] = uniforms[src[j] + i];
                        }
                    }
                }
                sp += count;
                continue;
            }

            case ByteCodeInstruction::kMatrixToMatrix: {
                int srcCols = READ8();
                int srcRows = READ8();
                int dstCols = READ8();
                int dstRows = READ8();
                SkASSERT(srcCols >= 2 && srcCols <= 4);
                SkASSERT(srcRows >= 2 && srcRows <= 4);
                SkASSERT(dstCols >= 2 && dstCols <= 4);
                SkASSERT(dstRows >= 2 && dstRows <= 4);
                F32 tmp[16];
                memset(tmp, 0, sizeof(tmp));
                tmp[0] = tmp[5] = tmp[10] = tmp[15] = F32(1.0f);
                for (int c = srcCols - 1; c >= 0; --c) {
                    for (int r = srcRows - 1; r >= 0; --r) {
                        tmp[c*4 + r] = POP().fFloat;
                    }
                }
                for (int c = 0; c < dstCols; ++c) {
                    for (int r = 0; r < dstRows; ++r) {
                        PUSH(tmp[c*4 + r]);
                    }
                }
                continue;
            }

            case ByteCodeInstruction::kMatrixMultiply: {
                int lCols = READ8();
                int lRows = READ8();
                int rCols = READ8();
                int rRows = lCols;
                F32 tmp[16] = { 0.0f };
                F32* B = &(sp - (rCols * rRows) + 1)->fFloat;
                F32* A = B - (lCols * lRows);
                for (int c = 0; c < rCols; ++c) {
                    for (int r = 0; r < lRows; ++r) {
                        for (int j = 0; j < lCols; ++j) {
                            tmp[c*lRows + r] += A[j*lRows + r] * B[c*rRows + j];
                        }
                    }
                }
                sp -= (lCols * lRows) + (rCols * rRows);
                memcpy(sp + 1, tmp, rCols * lRows * sizeof(VValue));
                sp += (rCols * lRows);
                continue;
            }

            VECTOR_BINARY_FN(kMaxF, fFloat, skvx::max)
            VECTOR_BINARY_FN(kMaxS, fSigned, skvx::max)
            VECTOR_BINARY_FN(kMinF, fFloat, skvx::min)
            VECTOR_BINARY_FN(kMinS, fSigned, skvx::min)

            case ByteCodeInstruction::kMix4:
            case ByteCodeInstruction::kMix3:
            case ByteCodeInstruction::kMix2:
            case ByteCodeInstruction::kMix: {
                int count = (int)ByteCodeInstruction::kMix - (int)inst + 1;
                for (int i = count; i --> 0; ) {
                    // GLSL's arguments are mix(else, true, cond)
                    sp[-(2*count + i)] = skvx::if_then_else(sp[-(          i)].fSigned,
                                                            sp[-(  count + i)].fFloat,
                                                            sp[-(2*count + i)].fFloat);
                }
                sp -= 2 * count;
                continue;
            }

            VECTOR_BINARY_OP(kMultiplyI, fSigned, *)
            VECTOR_MATRIX_BINARY_OP(kMultiplyF, fFloat, *)

            case ByteCodeInstruction::kNegateF4: sp[-3] = -sp[-3].fFloat; [[fallthrough]];
            case ByteCodeInstruction::kNegateF3: sp[-2] = -sp[-2].fFloat; [[fallthrough]];
            case ByteCodeInstruction::kNegateF2: sp[-1] = -sp[-1].fFloat; [[fallthrough]];
            case ByteCodeInstruction::kNegateF:  sp[ 0] = -sp[ 0].fFloat;
                                                 continue;

            case ByteCodeInstruction::kNegateFN: {
                int count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    sp[-i] = -sp[-i].fFloat;
                }
                continue;
            }

            case ByteCodeInstruction::kNegateI4: sp[-3] = -sp[-3].fSigned; [[fallthrough]];
            case ByteCodeInstruction::kNegateI3: sp[-2] = -sp[-2].fSigned; [[fallthrough]];
            case ByteCodeInstruction::kNegateI2: sp[-1] = -sp[-1].fSigned; [[fallthrough]];
            case ByteCodeInstruction::kNegateI:  sp[ 0] = -sp[ 0].fSigned;
                                                 continue;

            case ByteCodeInstruction::kPop4: POP(); [[fallthrough]];
            case ByteCodeInstruction::kPop3: POP(); [[fallthrough]];
            case ByteCodeInstruction::kPop2: POP(); [[fallthrough]];
            case ByteCodeInstruction::kPop:  POP();
                                             continue;

            case ByteCodeInstruction::kPopN:
                sp -= READ8();
                continue;

            VECTOR_BINARY_FN(kPow, fFloat, skvx::pow)

            case ByteCodeInstruction::kPushImmediate:
                PUSH(U32(READ32()));
                continue;

            case ByteCodeInstruction::kReadExternal:
            case ByteCodeInstruction::kReadExternal2:
            case ByteCodeInstruction::kReadExternal3:
            case ByteCodeInstruction::kReadExternal4: {
                int count = (int)ByteCodeInstruction::kReadExternal - (int)inst + 1;
                int src = READ8();
                float tmp[4];
                I32 m = mask();
                for (int i = 0; i < VecWidth; ++i) {
                    if (m[i]) {
                        byteCode->fExternalValues[src]->read(baseIndex + i, tmp);
                        for (int j = 0; j < count; ++j) {
                            sp[j + 1].fFloat[i] = tmp[j];
                        }
                    }
                }
                sp += count;
                continue;
            }

            VECTOR_BINARY_FN(kRemainderF, fFloat, VecMod)
            VECTOR_BINARY_MASKED_OP(kRemainderS, fSigned, %)
            VECTOR_BINARY_MASKED_OP(kRemainderU, fUnsigned, %)

            case ByteCodeInstruction::kReserve:
                sp += READ8();
                continue;

            case ByteCodeInstruction::kReturn: {
                int count = READ8();
                if (frames.empty()) {
                    if (outReturn) {
                        VValue* src = sp - count + 1;
                        if (stripedOutput) {
                            for (int i = 0; i < count; ++i) {
                                memcpy(outReturn[i], &src->fFloat, N * sizeof(float));
                                ++src;
                            }
                        } else {
                            float* outPtr = outReturn[0];
                            for (int i = 0; i < count; ++i) {
                                for (int j = 0; j < N; ++j) {
                                    outPtr[count * j] = src->fFloat[j];
                                }
                                ++outPtr;
                                ++src;
                            }
                        }
                    }
                    return true;
                } else {
                    // When we were called, the caller reserved stack space for their copy of our
                    // return value, then 'stack' was positioned after that, where our parameters
                    // were placed. Copy our return values to their reserved area.
                    memcpy(stack - count, sp - count + 1, count * sizeof(VValue));

                    // Now move the stack pointer to the end of the passed-in parameters. This odd
                    // calling convention requires the caller to pop the arguments after calling,
                    // but allows them to store any out-parameters back during that unwinding.
                    // After that sequence finishes, the return value will be the top of the stack.
                    const StackFrame& frame(frames.back());
                    sp = stack + frame.fParameterCount - 1;
                    stack = frame.fStack;
                    code = frame.fCode;
                    ip = frame.fIP;
                    frames.pop_back();
                    continue;
                }
            }

            case ByteCodeInstruction::kScalarToMatrix: {
                int cols = READ8();
                int rows = READ8();
                VValue v = POP();
                for (int c = 0; c < cols; ++c) {
                    for (int r = 0; r < rows; ++r) {
                        PUSH(c == r ? v : F32(0.0f));
                    }
                }
                continue;
            }

            case ByteCodeInstruction::kShiftLeft:
                sp[0] = sp[0].fSigned << READ8();
                continue;
            case ByteCodeInstruction::kShiftRightS:
                sp[0] = sp[0].fSigned >> READ8();
                continue;
            case ByteCodeInstruction::kShiftRightU:
                sp[0] = sp[0].fUnsigned >> READ8();
                continue;

            VECTOR_UNARY_FN(kSin, skvx::sin, fFloat)
            VECTOR_UNARY_FN(kSqrt, skvx::sqrt, fFloat)

            case ByteCodeInstruction::kStore4:
                stack[*ip+3] = skvx::if_then_else(mask(), POP().fFloat, stack[*ip+3].fFloat);
                [[fallthrough]];
            case ByteCodeInstruction::kStore3:
                stack[*ip+2] = skvx::if_then_else(mask(), POP().fFloat, stack[*ip+2].fFloat);
                [[fallthrough]];
            case ByteCodeInstruction::kStore2:
                stack[*ip+1] = skvx::if_then_else(mask(), POP().fFloat, stack[*ip+1].fFloat);
                [[fallthrough]];
            case ByteCodeInstruction::kStore:
                stack[*ip+0] = skvx::if_then_else(mask(), POP().fFloat, stack[*ip+0].fFloat);
                ++ip;
                continue;

            case ByteCodeInstruction::kStoreGlobal4:
                globals[*ip+3] = skvx::if_then_else(mask(), POP().fFloat, globals[*ip+3].fFloat);
                [[fallthrough]];
            case ByteCodeInstruction::kStoreGlobal3:
                globals[*ip+2] = skvx::if_then_else(mask(), POP().fFloat, globals[*ip+2].fFloat);
                [[fallthrough]];
            case ByteCodeInstruction::kStoreGlobal2:
                globals[*ip+1] = skvx::if_then_else(mask(), POP().fFloat, globals[*ip+1].fFloat);
                [[fallthrough]];
            case ByteCodeInstruction::kStoreGlobal:
                globals[*ip+0] = skvx::if_then_else(mask(), POP().fFloat, globals[*ip+0].fFloat);
                ++ip;
                continue;

            case ByteCodeInstruction::kStoreExtended: {
                int count = READ8();
                I32 target = POP().fSigned;
                VValue* src = sp - count + 1;
                I32 m = mask();
                for (int i = 0; i < count; ++i) {
                    for (int j = 0; j < VecWidth; ++j) {
                        if (m[j]) {
                            stack[target[j] + i].fSigned[j] = src[i].fSigned[j];
                        }
                    }
                }
                sp -= count;
                continue;
            }
            case ByteCodeInstruction::kStoreExtendedGlobal: {
                int count = READ8();
                I32 target = POP().fSigned;
                VValue* src = sp - count + 1;
                I32 m = mask();
                for (int i = 0; i < count; ++i) {
                    for (int j = 0; j < VecWidth; ++j) {
                        if (m[j]) {
                            globals[target[j] + i].fSigned[j] = src[i].fSigned[j];
                        }
                    }
                }
                sp -= count;
                continue;
            }

            VECTOR_BINARY_OP(kSubtractI, fSigned, -)
            VECTOR_MATRIX_BINARY_OP(kSubtractF, fFloat, -)

            case ByteCodeInstruction::kSwizzle: {
                VValue tmp[4];
                for (int i = READ8() - 1; i >= 0; --i) {
                    tmp[i] = POP();
                }
                for (int i = READ8() - 1; i >= 0; --i) {
                    PUSH(tmp[READ8()]);
                }
                continue;
            }

            VECTOR_UNARY_FN(kATan, skvx::atan, fFloat)
            VECTOR_UNARY_FN(kTan, skvx::tan, fFloat)

            case ByteCodeInstruction::kWriteExternal4:
            case ByteCodeInstruction::kWriteExternal3:
            case ByteCodeInstruction::kWriteExternal2:
            case ByteCodeInstruction::kWriteExternal: {
                int count = (int)ByteCodeInstruction::kWriteExternal - (int)inst + 1;
                int target = READ8();
                float tmp[4];
                I32 m = mask();
                sp -= count;
                for (int i = 0; i < VecWidth; ++i) {
                    if (m[i]) {
                        for (int j = 0; j < count; ++j) {
                            tmp[j] = sp[j + 1].fFloat[i];
                        }
                        byteCode->fExternalValues[target]->write(baseIndex + i, tmp);
                    }
                }
                continue;
            }

            case ByteCodeInstruction::kMaskPush:
                condPtr[1] = POP().fSigned;
                maskPtr[1] = maskPtr[0] & condPtr[1];
                ++condPtr; ++maskPtr;
                continue;
            case ByteCodeInstruction::kMaskPop:
                --condPtr; --maskPtr;
                continue;
            case ByteCodeInstruction::kMaskNegate:
                maskPtr[0] = maskPtr[-1] & ~condPtr[0];
                continue;
            case ByteCodeInstruction::kMaskBlend: {
                int count = READ8();
                I32 m = condPtr[0];
                --condPtr; --maskPtr;
                for (int i = 0; i < count; ++i) {
                    sp[-count] = skvx::if_then_else(m, sp[-count].fFloat, sp[0].fFloat);
                    --sp;
                }
                continue;
            }
            case ByteCodeInstruction::kBranchIfAllFalse: {
                int target = READ16();
                if (!skvx::any(mask())) {
                    ip = code + target;
                }
                continue;
            }

            case ByteCodeInstruction::kLoopBegin:
                contPtr[1] = 0;
                loopPtr[1] = loopPtr[0];
                ++contPtr; ++loopPtr;
                continue;
            case ByteCodeInstruction::kLoopNext:
                *loopPtr |= *contPtr;
                *contPtr = 0;
                continue;
            case ByteCodeInstruction::kLoopMask:
                *loopPtr &= POP().fSigned;
                continue;
            case ByteCodeInstruction::kLoopEnd:
                --contPtr; --loopPtr;
                continue;
            case ByteCodeInstruction::kLoopBreak:
                *loopPtr &= ~mask();
                continue;
            case ByteCodeInstruction::kLoopContinue: {
                I32 m = mask();
                *contPtr |=  m;
                *loopPtr &= ~m;
                continue;
            }

            case ByteCodeInstruction::kLoadFragCoord:
            case ByteCodeInstruction::kSampleExplicit:
            case ByteCodeInstruction::kSampleMatrix:
            default:
                // TODO: Support these?
                SkASSERT(false);
                return false;
        }
    }
}

}; // class Interpreter

#endif // SK_ENABLE_SKSL_INTERPRETER

#undef spf

void ByteCodeFunction::disassemble() const {
#if defined(SK_ENABLE_SKSL_INTERPRETER)
    const uint8_t* ip = fCode.data();
    while (ip < fCode.data() + fCode.size()) {
        printf("%d: ", (int)(ip - fCode.data()));
        ip = Interpreter::DisassembleInstruction(ip);
        printf("\n");
    }
#endif
}

bool ByteCode::run(const ByteCodeFunction* f,
                   float* args, int argCount,
                   float* outReturn, int returnCount,
                   const float* uniforms, int uniformCount) const {
#if defined(SK_ENABLE_SKSL_INTERPRETER)
    Interpreter::VValue stack[128];
    int stackNeeded = f->fParameterCount + f->fLocalCount + f->fStackCount;
    if (stackNeeded > (int)SK_ARRAY_COUNT(stack)) {
        return false;
    }

    if (argCount != f->fParameterCount ||
        returnCount != f->fReturnCount ||
        uniformCount != fUniformSlotCount) {
        return false;
    }

    Interpreter::VValue globals[32];
    if (fGlobalSlotCount > (int)SK_ARRAY_COUNT(globals)) {
        return false;
    }

    // Transpose args into stack
    {
        float* src = args;
        float* dst = (float*)stack;
        for (int i = 0; i < argCount; ++i) {
            *dst = *src++;
            dst += VecWidth;
        }
    }

    bool stripedOutput = false;
    float** outArray = outReturn ? &outReturn : nullptr;
    if (!Interpreter::InnerRun(this, f, stack, outArray, globals, uniforms, stripedOutput, 1, 0)) {
        return false;
    }

    // Transpose out parameters back
    {
        float* dst = args;
        float* src = (float*)stack;
        for (const auto& p : f->fParameters) {
            if (p.fIsOutParameter) {
                for (int i = p.fSlotCount; i > 0; --i) {
                    *dst++ = *src;
                    src += VecWidth;
                }
            } else {
                dst += p.fSlotCount;
                src += p.fSlotCount * VecWidth;
            }
        }
    }

    return true;
#else
    SkDEBUGFAIL("ByteCode interpreter not enabled");
    return false;
#endif
}

bool ByteCode::runStriped(const ByteCodeFunction* f, int N,
                          float* args[], int argCount,
                          float* outReturn[], int returnCount,
                          const float* uniforms, int uniformCount) const {
#if defined(SK_ENABLE_SKSL_INTERPRETER)
    Interpreter::VValue stack[192];
    int stackNeeded = f->fParameterCount + f->fLocalCount + f->fStackCount;
    if (stackNeeded > (int)SK_ARRAY_COUNT(stack)) {
        return false;
    }

    if (argCount != f->fParameterCount ||
        returnCount != f->fReturnCount ||
        uniformCount != fUniformSlotCount) {
        return false;
    }

    Interpreter::VValue globals[32];
    if (fGlobalSlotCount > (int)SK_ARRAY_COUNT(globals)) {
        return false;
    }

    // innerRun just takes outArgs, so clear it if the count is zero
    if (returnCount == 0) {
        outReturn = nullptr;
    }

    // The instructions to store to locals and globals mask in the original value,
    // so they technically need to be initialized (to any value).
    for (int i = f->fParameterCount; i < f->fParameterCount + f->fLocalCount; i++) {
        stack[i].fFloat = 0.0f;
    }
    for (int i = 0; i < fGlobalSlotCount; i++) {
        globals[i].fFloat = 0.0f;
    }

    int baseIndex = 0;

    while (N) {
        int w = std::min(N, VecWidth);

        // Copy args into stack
        for (int i = 0; i < argCount; ++i) {
            memcpy((void*)(stack + i), args[i], w * sizeof(float));
        }

        bool stripedOutput = true;
        if (!Interpreter::InnerRun(this, f, stack, outReturn, globals, uniforms, stripedOutput, w,
                                   baseIndex)) {
            return false;
        }

        // Copy out parameters back
        int slot = 0;
        for (const auto& p : f->fParameters) {
            if (p.fIsOutParameter) {
                for (int i = slot; i < slot + p.fSlotCount; ++i) {
                    memcpy(args[i], stack + i, w * sizeof(float));
                }
            }
            slot += p.fSlotCount;
        }

        // Step each argument pointer ahead
        for (int i = 0; i < argCount; ++i) {
            args[i] += w;
        }
        N -= w;
        baseIndex += w;
    }

    return true;
#else
    SkDEBUGFAIL("ByteCode interpreter not enabled");
    return false;
#endif
}

} // namespace SkSL

#endif
