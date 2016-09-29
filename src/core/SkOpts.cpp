/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCpu.h"
#include "SkHalf.h"
#include "SkOnce.h"
#include "SkOpts.h"

#if defined(SK_ARM_HAS_NEON)
    #if defined(SK_ARM_HAS_CRC32)
        #define SK_OPTS_NS neon_and_crc32
    #else
        #define SK_OPTS_NS neon
    #endif
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    #define SK_OPTS_NS avx2
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
    #define SK_OPTS_NS avx
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE42
    #define SK_OPTS_NS sse42
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    #define SK_OPTS_NS sse41
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    #define SK_OPTS_NS ssse3
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE3
    #define SK_OPTS_NS sse3
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #define SK_OPTS_NS sse2
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    #define SK_OPTS_NS sse
#else
    #define SK_OPTS_NS portable
#endif

#include "SkBlend_opts.h"
#include "SkBlitMask_opts.h"
#include "SkBlitRow_opts.h"
#include "SkBlurImageFilter_opts.h"
#include "SkChecksum_opts.h"
#include "SkColorCubeFilter_opts.h"
#include "SkMorphologyImageFilter_opts.h"
#include "SkRasterPipeline_opts.h"
#include "SkSwizzler_opts.h"
#include "SkTextureCompressor_opts.h"
#include "SkXfermode_opts.h"

namespace SkOpts {
    // Define default function pointer values here...
    // If our global compile options are set high enough, these defaults might even be
    // CPU-specialized, e.g. a typical x86-64 machine might start with SSE2 defaults.
    // They'll still get a chance to be replaced with even better ones, e.g. using SSE4.1.
#define DEFINE_DEFAULT(name) decltype(name) name = SK_OPTS_NS::name
    DEFINE_DEFAULT(create_xfermode);
    DEFINE_DEFAULT(color_cube_filter_span);

    DEFINE_DEFAULT(box_blur_xx);
    DEFINE_DEFAULT(box_blur_xy);
    DEFINE_DEFAULT(box_blur_yx);

    DEFINE_DEFAULT(dilate_x);
    DEFINE_DEFAULT(dilate_y);
    DEFINE_DEFAULT( erode_x);
    DEFINE_DEFAULT( erode_y);

    DEFINE_DEFAULT(texture_compressor);
    DEFINE_DEFAULT(fill_block_dimensions);

    DEFINE_DEFAULT(blit_mask_d32_a8);

    DEFINE_DEFAULT(blit_row_color32);
    DEFINE_DEFAULT(blit_row_s32a_opaque);

    DEFINE_DEFAULT(RGBA_to_BGRA);
    DEFINE_DEFAULT(RGBA_to_rgbA);
    DEFINE_DEFAULT(RGBA_to_bgrA);
    DEFINE_DEFAULT(RGB_to_RGB1);
    DEFINE_DEFAULT(RGB_to_BGR1);
    DEFINE_DEFAULT(gray_to_RGB1);
    DEFINE_DEFAULT(grayA_to_RGBA);
    DEFINE_DEFAULT(grayA_to_rgbA);
    DEFINE_DEFAULT(inverted_CMYK_to_RGB1);
    DEFINE_DEFAULT(inverted_CMYK_to_BGR1);

    DEFINE_DEFAULT(srcover_srgb_srgb);

    DEFINE_DEFAULT(hash_fn);
#undef DEFINE_DEFAULT

// Stages that are not sensitive to the tail parameter can be represented by one function.
#define DEFINE_DEFAULT(stage, kCallNext) \
    decltype(stage) stage = body<SK_OPTS_NS::stage, kCallNext>

    DEFINE_DEFAULT(srcover, true);
    DEFINE_DEFAULT(constant_color, true);
    DEFINE_DEFAULT(lerp_constant_float, true);
#undef DEFINE_DEFAULT

// Stages that are sensitive to the tail parameter need two versions, _body and _tail.
#define DEFINE_DEFAULT(stage, kCallNext)                                      \
    decltype(stage##_body) stage##_body = body<SK_OPTS_NS::stage, kCallNext>; \
    decltype(stage##_tail) stage##_tail = tail<SK_OPTS_NS::stage, kCallNext>

    DEFINE_DEFAULT(load_d_srgb, true);
    DEFINE_DEFAULT(load_s_srgb, true);
    DEFINE_DEFAULT( store_srgb, false);

    DEFINE_DEFAULT(load_d_f16, true);
    DEFINE_DEFAULT(load_s_f16, true);
    DEFINE_DEFAULT( store_f16, false);

    DEFINE_DEFAULT(load_d_565, true);
    DEFINE_DEFAULT(load_s_565, true);
    DEFINE_DEFAULT( store_565, false);

    DEFINE_DEFAULT(scale_u8, true);

    DEFINE_DEFAULT(lerp_u8,  true);
    DEFINE_DEFAULT(lerp_565, true);
#undef DEFINE_DEFAULT

    // Each Init_foo() is defined in src/opts/SkOpts_foo.cpp.
    void Init_ssse3();
    void Init_sse41();
    void Init_sse42();
    void Init_avx();
    void Init_avx2() {}
    void Init_crc32();

    static void init() {
#if !defined(SK_BUILD_NO_OPTS)
    #if defined(SK_CPU_X86)
        if (SkCpu::Supports(SkCpu::SSSE3)) { Init_ssse3(); }
        if (SkCpu::Supports(SkCpu::SSE41)) { Init_sse41(); }
        if (SkCpu::Supports(SkCpu::SSE42)) { Init_sse42(); }
        if (SkCpu::Supports(SkCpu::AVX  )) { Init_avx();   }
        if (SkCpu::Supports(SkCpu::AVX2 )) { Init_avx2();  }

    #elif defined(SK_CPU_ARM64)
        if (SkCpu::Supports(SkCpu::CRC32)) { Init_crc32(); }

    #endif
#endif
    }

    void Init() {
        static SkOnce once;
        once(init);
    }
}  // namespace SkOpts
