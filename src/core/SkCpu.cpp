/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCpu.h"
#include "SkOnce.h"

#if !defined(__has_include)
    #define __has_include(x) 0
#endif

#if defined(SK_CPU_X86)
    #if defined(SK_BUILD_FOR_WIN32)
        #include <intrin.h>
        static void cpuid (uint32_t abcd[4]) { __cpuid  ((int*)abcd, 1);    }
        static void cpuid7(uint32_t abcd[4]) { __cpuidex((int*)abcd, 7, 0); }
        static uint64_t xgetbv(uint32_t xcr) { return _xgetbv(xcr); }
    #else
        #include <cpuid.h>
        #if !defined(__cpuid_count)  // Old Mac Clang doesn't have this defined.
            #define  __cpuid_count(eax, ecx, a, b, c, d) \
                __asm__("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "0"(eax), "2"(ecx))
        #endif
        static void cpuid (uint32_t abcd[4]) { __get_cpuid(1, abcd+0, abcd+1, abcd+2, abcd+3); }
        static void cpuid7(uint32_t abcd[4]) {
            __cpuid_count(7, 0, abcd[0], abcd[1], abcd[2], abcd[3]);
        }
        static uint64_t xgetbv(uint32_t xcr) {
            uint32_t eax, edx;
            __asm__ __volatile__ ( "xgetbv" : "=a"(eax), "=d"(edx) : "c"(xcr));
            return (uint64_t)(edx) << 32 | eax;
        }
    #endif

    static uint32_t read_cpu_features() {
        uint32_t features = 0;
        uint32_t abcd[4] = {0,0,0,0};

        // You might want to refer to http://www.sandpile.org/x86/cpuid.htm

        cpuid(abcd);
        if (abcd[3] & (1<<25)) { features |= SkCpu:: SSE1; }
        if (abcd[3] & (1<<26)) { features |= SkCpu:: SSE2; }
        if (abcd[2] & (1<< 0)) { features |= SkCpu:: SSE3; }
        if (abcd[2] & (1<< 9)) { features |= SkCpu::SSSE3; }
        if (abcd[2] & (1<<19)) { features |= SkCpu::SSE41; }
        if (abcd[2] & (1<<20)) { features |= SkCpu::SSE42; }

        if ((abcd[2] & (3<<26)) == (3<<26) && (xgetbv(0) & 6) == 6) {  // XSAVE + OSXSAVE
            if (abcd[2] & (1<<28)) { features |= SkCpu:: AVX; }
            if (abcd[2] & (1<<29)) { features |= SkCpu::F16C; }
            if (abcd[2] & (1<<12)) { features |= SkCpu:: FMA; }

            cpuid7(abcd);
            if (abcd[1] & (1<<5)) { features |= SkCpu::AVX2; }
            if (abcd[1] & (1<<3)) { features |= SkCpu::BMI1; }
            if (abcd[1] & (1<<8)) { features |= SkCpu::BMI2; }
        }
        return features;
    }

#elif defined(SK_CPU_ARM64) && __has_include(<asm/hwcap.h>) && __has_include(<sys/auxv.h>)
    #include <asm/hwcap.h>
    #include <sys/auxv.h>

    static uint32_t read_cpu_features() {
        uint32_t features = 0;
        uint32_t hwcaps = getauxval(AT_HWCAP);
        if (hwcaps & HWCAP_CRC32) { features |= SkCpu::CRC32; }
        return features;
    }

#elif defined(SK_CPU_ARM32) && __has_include(<asm/hwcap.h>) && __has_include(<sys/auxv.h>)
    // asm/hwcap.h and sys/auxv.h won't be present on NDK builds before API v21.
    #include <asm/hwcap.h>
    #include <sys/auxv.h>

    static uint32_t read_cpu_features() {
        uint32_t features = 0;
        uint32_t hwcaps = getauxval(AT_HWCAP);
        if (hwcaps & HWCAP_VFPv4) { features |= SkCpu::NEON|SkCpu::NEON_FMA|SkCpu::VFP_FP16; }
        return features;
    }

#elif defined(SK_CPU_ARM32) && __has_include(<cpu-features.h>)
    #include <cpu-features.h>

    static uint32_t read_cpu_features() {
        uint32_t features = 0;
        uint64_t cpu_features = android_getCpuFeatures();
        if (cpu_features & ANDROID_CPU_ARM_FEATURE_NEON)     { features |= SkCpu::NEON; }
        if (cpu_features & ANDROID_CPU_ARM_FEATURE_NEON_FMA) { features |= SkCpu::NEON_FMA; }
        if (cpu_features & ANDROID_CPU_ARM_FEATURE_VFP_FP16) { features |= SkCpu::VFP_FP16; }
        return features;
    }

#else
    static uint32_t read_cpu_features() {
        return 0;
    }

#endif

uint32_t SkCpu::gCachedFeatures = 0;

void SkCpu::CacheRuntimeFeatures() {
    static SkOnce once;
    once([] { gCachedFeatures = read_cpu_features(); });
}
