/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPM4f_DEFINED
#define SkPM4f_DEFINED

#include "SkColorData.h"
#include "SkNx.h"

static inline Sk4f swizzle_rb(const Sk4f& x) {
    return SkNx_shuffle<2, 1, 0, 3>(x);
}

static inline Sk4f swizzle_rb_if_bgra(const Sk4f& x) {
#ifdef SK_PMCOLOR_IS_BGRA
    return swizzle_rb(x);
#else
    return x;
#endif
}

static inline Sk4f Sk4f_fromL32(uint32_t px) {
    return SkNx_cast<float>(Sk4b::Load(&px)) * (1/255.0f);
}

static inline uint32_t Sk4f_toL32(const Sk4f& px) {
    Sk4f v = px;

#if !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    // SkNx_cast<uint8_t, int32_t>() pins, and we don't anticipate giant floats
#elif !defined(SKNX_NO_SIMD) && defined(SK_ARM_HAS_NEON)
    // SkNx_cast<uint8_t, int32_t>() pins, and so does Sk4f_round().
#else
    // No guarantee of a pin.
    v = Sk4f::Max(0, Sk4f::Min(v, 1));
#endif

    uint32_t l32;
    SkNx_cast<uint8_t>(Sk4f_round(v * 255.0f)).store(&l32);
    return l32;
}

/*
 *  The float values are 0...1 premultiplied in RGBA order (regardless of SkPMColor order)
 */
struct SK_API SkPM4f {
    enum {
        R, G, B, A,
    };
    float fVec[4];

    float r() const { return fVec[R]; }
    float g() const { return fVec[G]; }
    float b() const { return fVec[B]; }
    float a() const { return fVec[A]; }

    static SkPM4f FromPremulRGBA(float r, float g, float b, float a) {
        SkPM4f p;
        p.fVec[R] = r;
        p.fVec[G] = g;
        p.fVec[B] = b;
        p.fVec[A] = a;
        return p;
    }

    static SkPM4f From4f(const Sk4f& x) {
        SkPM4f pm;
        x.store(pm.fVec);
        return pm;
    }
    static SkPM4f FromF16(const uint16_t[4]);
    static SkPM4f FromPMColor(SkPMColor);

    Sk4f to4f() const { return Sk4f::Load(fVec); }
    Sk4f to4f_rgba() const { return this->to4f(); }
    Sk4f to4f_bgra() const { return swizzle_rb(this->to4f()); }
    Sk4f to4f_pmorder() const { return swizzle_rb_if_bgra(this->to4f()); }

    SkPMColor toPMColor() const {
        return Sk4f_toL32(swizzle_rb_if_bgra(this->to4f()));
    }

    void toF16(uint16_t[4]) const;
    uint64_t toF16() const; // 4 float16 values packed into uint64_t

    SkColor4f unpremul() const;

#ifdef SK_DEBUG
    void assertIsUnit() const;
#else
    void assertIsUnit() const {}
#endif
};

#endif
