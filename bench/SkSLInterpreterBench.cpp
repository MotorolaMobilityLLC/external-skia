/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/utils/SkRandom.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLInterpreter.h"

// Benchmarks the interpreter with a function that has a color-filter style signature
class SkSLInterpreterCFBench : public Benchmark {
public:
    SkSLInterpreterCFBench(SkSL::String name, int pixels, const char* src)
        : fName(SkStringPrintf("sksl_interp_cf_%d_%s", pixels, name.c_str()))
        , fSrc(src)
        , fCount(pixels) {}

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDelayedSetup() override {
        SkSL::Compiler compiler;
        SkSL::Program::Settings settings;
        auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind, fSrc, settings);
        SkASSERT(compiler.errorCount() == 0);
        fByteCode = compiler.toByteCode(*program);
        SkASSERT(compiler.errorCount() == 0);
        fMain = fByteCode->getFunction("main");

        SkRandom rnd;
        fPixels.resize(fCount);
        for (int i = 0; i < fCount; ++i) {
            fPixels[i] = SkColor4f::FromColor(rnd.nextU());
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            SkSL::Interpreter::VecRun(fByteCode.get(), fMain,
                                      (SkSL::Interpreter::Value*)fPixels.data(), nullptr, fCount,
                                      nullptr, 0);
        }
    }

private:
    SkString fName;
    SkSL::String fSrc;
    std::unique_ptr<SkSL::ByteCode> fByteCode;
    const SkSL::ByteCodeFunction* fMain;

    int fCount;
    std::vector<SkColor4f> fPixels;

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new SkSLInterpreterCFBench("lumaToAlpha", 256, R"(
    void main(inout float4 color) {
        color.a = color.r*0.3 + color.g*0.6 + color.b*0.1;
        color.r = 0;
        color.g = 0;
        color.b = 0;
    }
)"));

DEF_BENCH(return new SkSLInterpreterCFBench("hcf", 256, R"(
    half ucontrast_Stage2;
    half hue2rgb_Stage2(half p, half q, half t) {
        if (t < 0)  t += 1;
        if (t > 1)  t -= 1;
        if (t < 1 / 6.)  return p + (q - p) * 6 * t;
        if (t < 1 / 2.)  return q;
        if (t < 2 / 3.)  return p + (q - p) * (2 / 3. - t) * 6;
        return p;
    }
    half max(half a, half b) { return a > b ? a : b; }
    half min(half a, half b) { return a < b ? a : b; }
    void main(inout half4 color) {
        ucontrast_Stage2 = 0.2;

        // HighContrastFilter
        half nonZeroAlpha = max(color.a, 0.0001);
        color = half4(color.rgb / nonZeroAlpha, nonZeroAlpha);
        color.rgb = color.rgb * color.rgb;
        half fmax = max(color.r, max(color.g, color.b));
        half fmin = min(color.r, min(color.g, color.b));
        half l = (fmax + fmin) / 2;
        half h;
        half s;
        if (fmax == fmin) {
            h = 0;
            s = 0;
        } else {
            half d = fmax - fmin;
            s = l > 0.5 ? d / (2 - fmax - fmin) : d / (fmax + fmin);
            if (color.r >= color.g && color.r >= color.b) {
                h = (color.g - color.b) / d + (color.g < color.b ? 6 : 0);
            } else if (color.g >= color.b) {
                h = (color.b - color.r) / d + 2;
            } else {
                h = (color.r - color.g) / d + 4;
            }
        }
        h /= 6;
        l = 1.0 - l;
        if (s == 0) {
            color = half4(l, l, l, 0);
        } else {
            half q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            half p = 2 * l - q;
            color.r = hue2rgb_Stage2(p, q, h + 1 / 3.);
            color.g = hue2rgb_Stage2(p, q, h);
            color.b = hue2rgb_Stage2(p, q, h - 1 / 3.);
        }
        if (ucontrast_Stage2 != 0) {
            half m = (1 + ucontrast_Stage2) / (1 - ucontrast_Stage2);
            half off = (-0.5 * m + 0.5);
            color = m * color + off;
        }
        // color = saturate(color);
        color.rgb = sqrt(color.rgb);
        color.rgb *= color.a;
    }
)"));

class SkSLInterpreterSortBench : public Benchmark {
public:
    SkSLInterpreterSortBench(int groups, int values, const char* src)
        : fName(SkStringPrintf("sksl_interp_sort_%dx%d", groups, values))
        , fCode(src)
        , fGroups(groups)
        , fValues(values) {
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDelayedSetup() override {
        SkSL::Compiler compiler;
        SkSL::Program::Settings settings;
        auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind, fCode, settings);
        SkASSERT(compiler.errorCount() == 0);
        fByteCode = compiler.toByteCode(*program);
        SkASSERT(compiler.errorCount() == 0);
        fMain = fByteCode->getFunction("main");

        fSrc.resize(fGroups * fValues);
        fDst.resize(fGroups * fValues);

        SkRandom rnd;
        for (auto& x : fSrc) {
            x = rnd.nextS();
        }

        // Trigger one run now to check correctness
        SkSL::Interpreter::VecRun(fByteCode.get(), fMain,
                                  (SkSL::Interpreter::Value*)fSrc.data(),
                                  (SkSL::Interpreter::Value*)fDst.data(),
                                  fGroups,
                                  nullptr, 0);
        for (int i = 0; i < fGroups; ++i) {
            for (int j = 1; j < fValues; ++j) {
                SkASSERT(fDst[i * fValues + j] >= fDst[i * fValues + j - 1]);
            }
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            SkSL::Interpreter::VecRun(fByteCode.get(), fMain,
                                      (SkSL::Interpreter::Value*)fSrc.data(),
                                      (SkSL::Interpreter::Value*)fDst.data(),
                                      fGroups,
                                      nullptr, 0);
        }
    }

private:
    SkString fName;
    SkSL::String fCode;
    std::unique_ptr<SkSL::ByteCode> fByteCode;
    const SkSL::ByteCodeFunction* fMain;

    int fGroups;
    int fValues;
    std::vector<int32_t> fSrc;
    std::vector<int32_t> fDst;

    typedef Benchmark INHERITED;
};

// Currently, this exceeds the interpreter's stack. Consider it a test case for some eventual
// bounds checking.
#if 0
DEF_BENCH(return new SkSLInterpreterSortBench(1024, 32, R"(
    int[32] main(int v[32]) {
        for (int i = 1; i < 32; ++i) {
            for (int j = i; j > 0 && v[j-1] > v[j]; --j) {
                int t = v[j];
                v[j] = v[j-1];
                v[j-1] = t;
            }
        }
        return v;
    }
)"));
#endif
