/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4px_DEFINED
#define Sk4px_DEFINED

#include "SkNx.h"
#include "SkColor.h"

// 1, 2 or 4 SkPMColors, generally vectorized.
class Sk4px : public Sk16b {
public:
    Sk4px(SkPMColor);  // Duplicate 4x.
    Sk4px(const Sk16b& v) : Sk16b(v) {}

    // When loading or storing fewer than 4 SkPMColors, we use the low lanes.
    static Sk4px Load4(const SkPMColor[4]);
    static Sk4px Load2(const SkPMColor[2]);
    static Sk4px Load1(const SkPMColor[1]);

    void store4(SkPMColor[4]) const;
    void store2(SkPMColor[2]) const;
    void store1(SkPMColor[1]) const;

    // 1, 2, or 4 SkPMColors with 16-bit components.
    // This is most useful as the result of a multiply, e.g. from mulWiden().
    class Wide : public Sk16h {
    public:
        Wide(const Sk16h& v) : Sk16h(v) {}

        // Pack the top byte of each component back down into 4 SkPMColors.
        Sk4px addNarrowHi(const Sk16h&) const;

        Sk4px div255TruncNarrow() const { return this->addNarrowHi(*this >> 8); }
        Sk4px div255RoundNarrow() const {
            return Sk4px::Wide(*this + Sk16h(128)).div255TruncNarrow();
        }

    private:
        typedef Sk16h INHERITED;
    };

    Wide widenLo() const;               // ARGB -> 0A 0R 0G 0B
    Wide widenHi() const;               // ARGB -> A0 R0 G0 B0
    Wide mulWiden(const Sk16b&) const;  // 8-bit x 8-bit -> 16-bit components.

    // A generic driver that maps fn over a src array into a dst array.
    // fn should take an Sk4px (4 src pixels) and return an Sk4px (4 dst pixels).
    template <typename Fn>
    static void MapSrc(int count, SkPMColor* dst, const SkPMColor* src, Fn fn) {
        // This looks a bit odd, but it helps loop-invariant hoisting across different calls to fn.
        // Basically, we need to make sure we keep things inside a single loop.
        while (count > 0) {
            if (count >= 8) {
                Sk4px dst0 = fn(Load4(src+0)),
                      dst4 = fn(Load4(src+4));
                dst0.store4(dst+0);
                dst4.store4(dst+4);
                dst += 8; src += 8; count -= 8;
                continue;  // Keep our stride at 8 pixels as long as possible.
            }
            SkASSERT(count <= 7);
            if (count >= 4) {
                fn(Load4(src)).store4(dst);
                dst += 4; src += 4; count -= 4;
            }
            if (count >= 2) {
                fn(Load2(src)).store2(dst);
                dst += 2; src += 2; count -= 2;
            }
            if (count >= 1) {
                fn(Load1(src)).store1(dst);
            }
            break;
        }
    }

    // As above, but with dst4' = fn(dst4, src4).
    template <typename Fn>
    static void MapDstSrc(int count, SkPMColor* dst, const SkPMColor* src, Fn fn) {
        while (count > 0) {
            if (count >= 8) {
                Sk4px dst0 = fn(Load4(dst+0), Load4(src+0)),
                      dst4 = fn(Load4(dst+4), Load4(src+4));
                dst0.store4(dst+0);
                dst4.store4(dst+4);
                dst += 8; src += 8; count -= 8;
                continue;  // Keep our stride at 8 pixels as long as possible.
            }
            SkASSERT(count <= 7);
            if (count >= 4) {
                fn(Load4(dst), Load4(src)).store4(dst);
                dst += 4; src += 4; count -= 4;
            }
            if (count >= 2) {
                fn(Load2(dst), Load2(src)).store2(dst);
                dst += 2; src += 2; count -= 2;
            }
            if (count >= 1) {
                fn(Load1(dst), Load1(src)).store1(dst);
            }
            break;
        }
    }

    // As above, but with dst4' = fn(dst4, src4, alpha4).
    template <typename Fn>
    static void MapDstSrcAlpha(
            int count, SkPMColor* dst, const SkPMColor* src, const SkAlpha* a, Fn fn) {
        // TODO: find a terser / faster way to construct Sk16b alphas.
        while (count > 0) {
            if (count >= 8) {
                Sk16b alpha0(a[0],a[0],a[0],a[0], a[1],a[1],a[1],a[1],
                             a[2],a[2],a[2],a[2], a[3],a[3],a[3],a[3]),
                      alpha4(a[4],a[4],a[4],a[4], a[5],a[5],a[5],a[5],
                             a[6],a[6],a[6],a[6], a[7],a[7],a[7],a[7]);
                Sk4px dst0 = fn(Load4(dst+0), Load4(src+0), alpha0),
                      dst4 = fn(Load4(dst+4), Load4(src+4), alpha4);
                dst0.store4(dst+0);
                dst4.store4(dst+4);
                dst += 8; src += 8; a += 8; count -= 8;
                continue;  // Keep our stride at 8 pixels as long as possible.
            }
            SkASSERT(count <= 7);
            if (count >= 4) {
                Sk16b alpha(a[0],a[0],a[0],a[0], a[1],a[1],a[1],a[1],
                            a[2],a[2],a[2],a[2], a[3],a[3],a[3],a[3]);
                fn(Load4(dst), Load4(src), alpha).store4(dst);
                dst += 4; src += 4; a += 4; count -= 4;
            }
            if (count >= 2) {
                Sk16b alpha(a[0],a[0],a[0],a[0], a[1],a[1],a[1],a[1], 0,0,0,0, 0,0,0,0);
                fn(Load2(dst), Load2(src), alpha).store2(dst);
                dst += 2; src += 2; a += 2; count -= 2;
            }
            if (count >= 1) {
                Sk16b alpha(a[0],a[0],a[0],a[0], 0,0,0,0, 0,0,0,0, 0,0,0,0);
                fn(Load1(dst), Load1(src), alpha).store1(dst);
            }
            break;
        }
    }

private:
    typedef Sk16b INHERITED;
};

#ifdef SKNX_NO_SIMD
    #include "../opts/Sk4px_none.h"
#else
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
        #include "../opts/Sk4px_SSE2.h"
    #elif defined(SK_ARM_HAS_NEON)
        #include "../opts/Sk4px_NEON.h"
    #else
        #include "../opts/Sk4px_none.h"
    #endif
#endif

#endif//Sk4px_DEFINED
