/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#if defined(SK_CPU_X86)
    #if defined(SK_BUILD_FOR_WIN32)
        #include <intrin.h>
        static void cpuid(uint32_t abcd[4]) { __cpuid((int*)abcd, 1); }
    #else
        #include <cpuid.h>
        static void cpuid(uint32_t abcd[4]) { __get_cpuid(1, abcd+0, abcd+1, abcd+2, abcd+3); }
    #endif
#elif defined(SK_BUILD_FOR_ANDROID)
    #include <cpu-features.h>
#endif

namespace SkOpts {
    // (Define default function pointer values here...)

    // Each Init_foo() is defined in src/opts/SkOpts_foo.cpp.
    void Init_sse2();
    void Init_ssse3();
    void Init_sse41();
    void Init_neon();
    //TODO: _dsp2, _armv7, _armv8, _x86, _x86_64, _sse42, _avx, avx2, ... ?

    void Init() {
    #if defined(SK_CPU_X86)
        uint32_t abcd[] = {0,0,0,0};
        cpuid(abcd);
        if (abcd[3] & (1<<26)) { Init_sse2(); }
        if (abcd[2] & (1<< 9)) { Init_ssse3(); }
        if (abcd[2] & (1<<19)) { Init_sse41(); }

    #elif defined(SK_ARM_HAS_NEON)
        Init_neon();

    #elif defined(SK_BUILD_FOR_ANDROID)
        if (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) { Init_neon(); }

    #endif
    }
}
