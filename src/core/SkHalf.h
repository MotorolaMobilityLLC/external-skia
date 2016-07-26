/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHalf_DEFINED
#define SkHalf_DEFINED

#include "SkNx.h"
#include "SkTypes.h"

// 16-bit floating point value
// format is 1 bit sign, 5 bits exponent, 10 bits mantissa
// only used for storage
typedef uint16_t SkHalf;

static constexpr uint16_t SK_HalfMin     = 0x0400; // 2^-24  (minimum positive normal value)
static constexpr uint16_t SK_HalfMax     = 0x7bff; // 65504
static constexpr uint16_t SK_HalfEpsilon = 0x1400; // 2^-10
static constexpr uint16_t SK_Half1       = 0x3C00; // 1

// convert between half and single precision floating point
float SkHalfToFloat(SkHalf h);
SkHalf SkFloatToHalf(float f);

// Convert between half and single precision floating point,
// assuming inputs and outputs are both finite.
static inline Sk4f SkHalfToFloat_finite(uint64_t);
static inline Sk4h SkFloatToHalf_finite(const Sk4f&);

// ~~~~~~~~~~~ impl ~~~~~~~~~~~~~~ //

// Like the serial versions in SkHalf.cpp, these are based on
// https://fgiesen.wordpress.com/2012/03/28/half-to-float-done-quic/

// GCC 4.9 lacks the intrinsics to use ARMv8 f16<->f32 instructions, so we use inline assembly.

static inline Sk4f SkHalfToFloat_finite(const Sk4h& hs) {
#if !defined(SKNX_NO_SIMD) && defined(SK_CPU_ARM64)
    float32x4_t fs;
    asm ("fcvtl %[fs].4s, %[hs].4h   \n"   // vcvt_f32_f16(...)
        : [fs] "=w" (fs)                   // =w: write-only NEON register
        : [hs] "w" (hs.fVec));             //  w: read-only NEON register
    return fs;
#else
    Sk4i bits      = SkNx_cast<int>(hs),   // Expand to 32 bit.
         sign      = bits & 0x00008000,    // Save the sign bit for later...
         positive  = bits ^ sign,          // ...but strip it off for now.
         is_denorm = positive < (1<<10);   // Exponent == 0?

    // For normal half floats, extend the mantissa by 13 zero bits,
    // then adjust the exponent from 15 bias to 127 bias.
    Sk4i norm = (positive << 13) + ((127 - 15) << 23);

    // For denorm half floats, mask in the exponent-only float K that turns our
    // denorm value V*2^-14 into a normalized float K + V*2^-14.  Then subtract off K.
    const Sk4i K = ((127-15) + (23-10) + 1) << 23;
    Sk4i mask_K = positive | K;
    Sk4f denorm = Sk4f::Load(&mask_K) - Sk4f::Load(&K);

    Sk4i merged = (sign << 16) | is_denorm.thenElse(Sk4i::Load(&denorm), norm);
    return Sk4f::Load(&merged);
#endif
}

static inline Sk4f SkHalfToFloat_finite(uint64_t hs) {
    return SkHalfToFloat_finite(Sk4h::Load(&hs));
}

static inline Sk4h SkFloatToHalf_finite(const Sk4f& fs) {
#if !defined(SKNX_NO_SIMD) && defined(SK_CPU_ARM64)
    float32x4_t vec = fs.fVec;
    asm ("fcvtn %[vec].4h, %[vec].4s  \n"   // vcvt_f16_f32(vec)
        : [vec] "+w" (vec));                // +w: read-write NEON register
    return vreinterpret_u16_f32(vget_low_f32(vec));
#else
    Sk4i bits           = Sk4i::Load(&fs),
         sign           = bits & 0x80000000,              // Save the sign bit for later...
         positive       = bits ^ sign,                    // ...but strip it off for now.
         will_be_denorm = positive < ((127-15+1) << 23);  // positve < smallest normal half?

    // For normal half floats, adjust the exponent from 127 bias to 15 bias,
    // then drop the bottom 13 mantissa bits.
    Sk4i norm = (positive - ((127 - 15) << 23)) >> 13;

    // This mechanically inverts the denorm half -> normal float conversion above.
    // Knowning that and reading its explanation will leave you feeling more confident
    // than reading my best attempt at explaining this directly.
    const Sk4i K = ((127-15) + (23-10) + 1) << 23;
    Sk4f plus_K = Sk4f::Load(&positive) + Sk4f::Load(&K);
    Sk4i denorm = Sk4i::Load(&plus_K) ^ K;

    Sk4i merged = (sign >> 16) | will_be_denorm.thenElse(denorm, norm);
    return SkNx_cast<uint16_t>(merged);
#endif
}

#endif
