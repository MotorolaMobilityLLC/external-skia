/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKVX_DEFINED
#define SKVX_DEFINED

// skvx::Vec<N,T> are SIMD vectors of N T's, a v1.5 successor to SkNx<N,T>.
//
// This time we're leaning a bit less on platform-specific intrinsics and a bit
// more on Clang/GCC vector extensions, but still keeping the option open to
// drop in platform-specific intrinsics, actually more easily than before.
//
// We've also fixed a few of the caveats that used to make SkNx awkward to work
// with across translation units.  skvx::Vec<N,T> always has N*sizeof(T) size
// and alignment and is safe to use across translation units freely.
// Ideally we'd only align to T, but that tanks ARMv7 NEON codegen.

// Please try to keep this file independent of Skia headers.
#include <algorithm>         // std::min, std::max
#include <cmath>             // std::ceil, std::floor, std::trunc, std::round, std::sqrt, etc.
#include <cstdint>           // intXX_t
#include <cstring>           // memcpy()
#include <initializer_list>  // std::initializer_list

#if defined(__SSE__) || defined(__AVX__) || defined(__AVX2__)
    #include <immintrin.h>
#elif defined(__ARM_NEON)
    #include <arm_neon.h>
#elif defined(__wasm_simd128__)
    #include <wasm_simd128.h>
#endif

// To avoid ODR violations, all methods must be force-inlined...
#if defined(_MSC_VER)
    #define SKVX_ALWAYS_INLINE __forceinline
#else
    #define SKVX_ALWAYS_INLINE __attribute__((always_inline))
#endif

// ... and all standalone functions must be static.  Please use these helpers:
#define SI    static inline
#define SIT   template <       typename T> SI
#define SIN   template <int N            > SI
#define SINT  template <int N, typename T> SI
#define SINTU template <int N, typename T, typename U, \
                        typename=std::enable_if_t<std::is_convertible<U,T>::value>> SI

namespace skvx {

// All Vec have the same simple memory layout, the same as `T vec[N]`.
template <int N, typename T>
struct alignas(N*sizeof(T)) Vec {
    static_assert((N & (N-1)) == 0,        "N must be a power of 2.");
    static_assert(sizeof(T) >= alignof(T), "What kind of crazy T is this?");

    Vec<N/2,T> lo, hi;

    // Methods belong here in the class declaration of Vec only if:
    //   - they must be here, like constructors or operator[];
    //   - they'll definitely never want a specialized implementation.
    // Other operations on Vec should be defined outside the type.

    SKVX_ALWAYS_INLINE Vec() = default;

    template <typename U, typename=std::enable_if_t<std::is_convertible<U,T>::value>>
    SKVX_ALWAYS_INLINE
    Vec(U x) : lo(x), hi(x) {}

    SKVX_ALWAYS_INLINE Vec(std::initializer_list<T> xs) {
        T vals[N] = {0};
        memcpy(vals, xs.begin(), std::min(xs.size(), (size_t)N)*sizeof(T));

        lo = Vec<N/2,T>::Load(vals +   0);
        hi = Vec<N/2,T>::Load(vals + N/2);
    }

    SKVX_ALWAYS_INLINE T  operator[](int i) const { return i < N/2 ? lo[i] : hi[i-N/2]; }
    SKVX_ALWAYS_INLINE T& operator[](int i)       { return i < N/2 ? lo[i] : hi[i-N/2]; }

    SKVX_ALWAYS_INLINE static Vec Load(const void* ptr) {
        Vec v;
        memcpy(&v, ptr, sizeof(Vec));
        return v;
    }
    SKVX_ALWAYS_INLINE void store(void* ptr) const {
        memcpy(ptr, this, sizeof(Vec));
    }
};

template <typename T>
struct Vec<1,T> {
    T val;

    SKVX_ALWAYS_INLINE Vec() = default;

    template <typename U, typename=std::enable_if_t<std::is_convertible<U,T>::value>>
    SKVX_ALWAYS_INLINE
    Vec(U x) : val(x) {}

    SKVX_ALWAYS_INLINE Vec(std::initializer_list<T> xs) : val(xs.size() ? *xs.begin() : 0) {}

    SKVX_ALWAYS_INLINE T  operator[](int) const { return val; }
    SKVX_ALWAYS_INLINE T& operator[](int)       { return val; }

    SKVX_ALWAYS_INLINE static Vec Load(const void* ptr) {
        Vec v;
        memcpy(&v, ptr, sizeof(Vec));
        return v;
    }
    SKVX_ALWAYS_INLINE void store(void* ptr) const {
        memcpy(ptr, this, sizeof(Vec));
    }
};

template <typename D, typename S>
SI D unchecked_bit_pun(const S& s) {
    D d;
    memcpy(&d, &s, sizeof(D));
    return d;
}

template <typename D, typename S>
SI D bit_pun(const S& s) {
    static_assert(sizeof(D) == sizeof(S), "");
    return unchecked_bit_pun<D>(s);
}

// Translate from a value type T to its corresponding Mask, the result of a comparison.
template <typename T> struct Mask { using type = T; };
template <> struct Mask<float > { using type = int32_t; };
template <> struct Mask<double> { using type = int64_t; };
template <typename T> using M = typename Mask<T>::type;

// Join two Vec<N,T> into one Vec<2N,T>.
SINT Vec<2*N,T> join(const Vec<N,T>& lo, const Vec<N,T>& hi) {
    Vec<2*N,T> v;
    v.lo = lo;
    v.hi = hi;
    return v;
}

// We have two default strategies for implementing most operations:
//    1) lean on Clang/GCC vector extensions when available;
//    2) recurse to scalar portable implementations when not.
// At the end we can drop in platform-specific implementations that override either default.

#if !defined(SKNX_NO_SIMD) && (defined(__clang__) || defined(__GNUC__))

    // VExt<N,T> types have the same size as Vec<N,T> and support most operations directly.
    // N.B. VExt<N,T> alignment is N*alignof(T), stricter than Vec<N,T>'s alignof(T).
    #if defined(__clang__)
        template <int N, typename T>
        using VExt = T __attribute__((ext_vector_type(N)));

    #elif defined(__GNUC__)
        template <int N, typename T>
        struct VExtHelper {
            typedef T __attribute__((vector_size(N*sizeof(T)))) type;
        };

        template <int N, typename T>
        using VExt = typename VExtHelper<N,T>::type;

        // For some reason some (new!) versions of GCC cannot seem to deduce N in the generic
        // to_vec<N,T>() below for N=4 and T=float.  This workaround seems to help...
        SI Vec<4,float> to_vec(VExt<4,float> v) { return bit_pun<Vec<4,float>>(v); }
    #endif

    SINT VExt<N,T> to_vext(const Vec<N,T>& v) { return bit_pun<VExt<N,T>>(v); }
    SINT Vec <N,T> to_vec(const VExt<N,T>& v) { return bit_pun<Vec <N,T>>(v); }

    SINT Vec<N,T> operator+(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) + to_vext(y));
    }
    SINT Vec<N,T> operator-(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) - to_vext(y));
    }
    SINT Vec<N,T> operator*(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) * to_vext(y));
    }
    SINT Vec<N,T> operator/(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) / to_vext(y));
    }

    SINT Vec<N,T> operator^(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) ^ to_vext(y));
    }
    SINT Vec<N,T> operator&(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) & to_vext(y));
    }
    SINT Vec<N,T> operator|(const Vec<N,T>& x, const Vec<N,T>& y) {
        return to_vec<N,T>(to_vext(x) | to_vext(y));
    }

    SINT Vec<N,T> operator!(const Vec<N,T>& x) { return to_vec<N,T>(!to_vext(x)); }
    SINT Vec<N,T> operator-(const Vec<N,T>& x) { return to_vec<N,T>(-to_vext(x)); }
    SINT Vec<N,T> operator~(const Vec<N,T>& x) { return to_vec<N,T>(~to_vext(x)); }

    SINT Vec<N,T> operator<<(const Vec<N,T>& x, int k) { return to_vec<N,T>(to_vext(x) << k); }
    SINT Vec<N,T> operator>>(const Vec<N,T>& x, int k) { return to_vec<N,T>(to_vext(x) >> k); }

    SINT Vec<N,M<T>> operator==(const Vec<N,T>& x, const Vec<N,T>& y) {
        return bit_pun<Vec<N,M<T>>>(to_vext(x) == to_vext(y));
    }
    SINT Vec<N,M<T>> operator!=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return bit_pun<Vec<N,M<T>>>(to_vext(x) != to_vext(y));
    }
    SINT Vec<N,M<T>> operator<=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return bit_pun<Vec<N,M<T>>>(to_vext(x) <= to_vext(y));
    }
    SINT Vec<N,M<T>> operator>=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return bit_pun<Vec<N,M<T>>>(to_vext(x) >= to_vext(y));
    }
    SINT Vec<N,M<T>> operator< (const Vec<N,T>& x, const Vec<N,T>& y) {
        return bit_pun<Vec<N,M<T>>>(to_vext(x) <  to_vext(y));
    }
    SINT Vec<N,M<T>> operator> (const Vec<N,T>& x, const Vec<N,T>& y) {
        return bit_pun<Vec<N,M<T>>>(to_vext(x) >  to_vext(y));
    }

#else

    // Either SKNX_NO_SIMD is defined, or Clang/GCC vector extensions are not available.
    // We'll implement things portably, in a way that should be easily autovectorizable.

    // N == 1 scalar implementations.
    SIT Vec<1,T> operator+(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val + y.val; }
    SIT Vec<1,T> operator-(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val - y.val; }
    SIT Vec<1,T> operator*(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val * y.val; }
    SIT Vec<1,T> operator/(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val / y.val; }

    SIT Vec<1,T> operator^(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val ^ y.val; }
    SIT Vec<1,T> operator&(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val & y.val; }
    SIT Vec<1,T> operator|(const Vec<1,T>& x, const Vec<1,T>& y) { return x.val | y.val; }

    SIT Vec<1,T> operator!(const Vec<1,T>& x) { return !x.val; }
    SIT Vec<1,T> operator-(const Vec<1,T>& x) { return -x.val; }
    SIT Vec<1,T> operator~(const Vec<1,T>& x) { return ~x.val; }

    SIT Vec<1,T> operator<<(const Vec<1,T>& x, int k) { return x.val << k; }
    SIT Vec<1,T> operator>>(const Vec<1,T>& x, int k) { return x.val >> k; }

    SIT Vec<1,M<T>> operator==(const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val == y.val ? ~0 : 0;
    }
    SIT Vec<1,M<T>> operator!=(const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val != y.val ? ~0 : 0;
    }
    SIT Vec<1,M<T>> operator<=(const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val <= y.val ? ~0 : 0;
    }
    SIT Vec<1,M<T>> operator>=(const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val >= y.val ? ~0 : 0;
    }
    SIT Vec<1,M<T>> operator< (const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val <  y.val ? ~0 : 0;
    }
    SIT Vec<1,M<T>> operator> (const Vec<1,T>& x, const Vec<1,T>& y) {
        return x.val >  y.val ? ~0 : 0;
    }

    // All default N != 1 implementations just recurse on lo and hi halves.
    SINT Vec<N,T> operator+(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo + y.lo, x.hi + y.hi);
    }
    SINT Vec<N,T> operator-(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo - y.lo, x.hi - y.hi);
    }
    SINT Vec<N,T> operator*(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo * y.lo, x.hi * y.hi);
    }
    SINT Vec<N,T> operator/(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo / y.lo, x.hi / y.hi);
    }

    SINT Vec<N,T> operator^(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo ^ y.lo, x.hi ^ y.hi);
    }
    SINT Vec<N,T> operator&(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo & y.lo, x.hi & y.hi);
    }
    SINT Vec<N,T> operator|(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo | y.lo, x.hi | y.hi);
    }

    SINT Vec<N,T> operator!(const Vec<N,T>& x) { return join(!x.lo, !x.hi); }
    SINT Vec<N,T> operator-(const Vec<N,T>& x) { return join(-x.lo, -x.hi); }
    SINT Vec<N,T> operator~(const Vec<N,T>& x) { return join(~x.lo, ~x.hi); }

    SINT Vec<N,T> operator<<(const Vec<N,T>& x, int k) { return join(x.lo << k, x.hi << k); }
    SINT Vec<N,T> operator>>(const Vec<N,T>& x, int k) { return join(x.lo >> k, x.hi >> k); }

    SINT Vec<N,M<T>> operator==(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo == y.lo, x.hi == y.hi);
    }
    SINT Vec<N,M<T>> operator!=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo != y.lo, x.hi != y.hi);
    }
    SINT Vec<N,M<T>> operator<=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo <= y.lo, x.hi <= y.hi);
    }
    SINT Vec<N,M<T>> operator>=(const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo >= y.lo, x.hi >= y.hi);
    }
    SINT Vec<N,M<T>> operator< (const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo <  y.lo, x.hi <  y.hi);
    }
    SINT Vec<N,M<T>> operator> (const Vec<N,T>& x, const Vec<N,T>& y) {
        return join(x.lo >  y.lo, x.hi >  y.hi);
    }
#endif

// Some operations we want are not expressible with Clang/GCC vector
// extensions, so we implement them using the recursive approach.

// N == 1 scalar implementations.
SIT Vec<1,T> if_then_else(const Vec<1,M<T>>& cond, const Vec<1,T>& t, const Vec<1,T>& e) {
    // In practice this scalar implementation is unlikely to be used.  See if_then_else() below.
    return bit_pun<Vec<1,T>>(( cond & bit_pun<Vec<1, M<T>>>(t)) |
                             (~cond & bit_pun<Vec<1, M<T>>>(e)) );
}

SIT bool any(const Vec<1,T>& x) { return x.val != 0; }
SIT bool all(const Vec<1,T>& x) { return x.val != 0; }

SIT Vec<1,T> pow(const Vec<1,T>& x, const Vec<1,T>& y) { return std::pow(x.val, y.val); }

// All default N != 1 implementations just recurse on lo and hi halves.

// Clang can reason about naive_if_then_else() and optimize through it better
// than if_then_else(), so it's sometimes useful to call it directly when we
// think an entire expression should optimize away, e.g. min()/max().
SINT Vec<N,T> naive_if_then_else(const Vec<N,M<T>>& cond, const Vec<N,T>& t, const Vec<N,T>& e) {
    return bit_pun<Vec<N,T>>(( cond & bit_pun<Vec<N, M<T>>>(t)) |
                             (~cond & bit_pun<Vec<N, M<T>>>(e)) );
}

SINT Vec<N,T> if_then_else(const Vec<N,M<T>>& cond, const Vec<N,T>& t, const Vec<N,T>& e) {
    // Specializations inline here so they can generalize what types the apply to.
    // (This header is used in C++14 contexts, so we have to kind of fake constexpr if.)
#if defined(__AVX2__)
    if /*constexpr*/ (N*sizeof(T) == 32) {
        return unchecked_bit_pun<Vec<N,T>>(_mm256_blendv_epi8(unchecked_bit_pun<__m256i>(e),
                                                              unchecked_bit_pun<__m256i>(t),
                                                              unchecked_bit_pun<__m256i>(cond)));
    }
#endif
#if defined(__SSE4_1__)
    if /*constexpr*/ (N*sizeof(T) == 16) {
        return unchecked_bit_pun<Vec<N,T>>(_mm_blendv_epi8(unchecked_bit_pun<__m128i>(e),
                                                           unchecked_bit_pun<__m128i>(t),
                                                           unchecked_bit_pun<__m128i>(cond)));
    }
#endif
#if defined(__ARM_NEON)
    if /*constexpr*/ (N*sizeof(T) == 16) {
        return unchecked_bit_pun<Vec<N,T>>(vbslq_u8(unchecked_bit_pun<uint8x16_t>(cond),
                                                    unchecked_bit_pun<uint8x16_t>(t),
                                                    unchecked_bit_pun<uint8x16_t>(e)));
    }
#endif
    // Recurse for large vectors to try to hit the specializations above.
    if /*constexpr*/ (N*sizeof(T) > 16) {
        return join(if_then_else(cond.lo, t.lo, e.lo),
                    if_then_else(cond.hi, t.hi, e.hi));
    }
    // This default can lead to better code than the recursing onto scalars.
    return naive_if_then_else(cond, t, e);
}

SINT bool any(const Vec<N,T>& x) { return any(x.lo) || any(x.hi); }
SINT bool all(const Vec<N,T>& x) { return all(x.lo) && all(x.hi); }

SINT Vec<N,T> pow(const Vec<N,T>& x, const Vec<N,T>& y) {
    return join(pow(x.lo, y.lo), pow(x.hi, y.hi));
}

// Scalar/vector operations just splat the scalar to a vector...
SINTU Vec<N,T>    operator+ (U x, const Vec<N,T>& y) { return Vec<N,T>(x) +  y; }
SINTU Vec<N,T>    operator- (U x, const Vec<N,T>& y) { return Vec<N,T>(x) -  y; }
SINTU Vec<N,T>    operator* (U x, const Vec<N,T>& y) { return Vec<N,T>(x) *  y; }
SINTU Vec<N,T>    operator/ (U x, const Vec<N,T>& y) { return Vec<N,T>(x) /  y; }
SINTU Vec<N,T>    operator^ (U x, const Vec<N,T>& y) { return Vec<N,T>(x) ^  y; }
SINTU Vec<N,T>    operator& (U x, const Vec<N,T>& y) { return Vec<N,T>(x) &  y; }
SINTU Vec<N,T>    operator| (U x, const Vec<N,T>& y) { return Vec<N,T>(x) |  y; }
SINTU Vec<N,M<T>> operator==(U x, const Vec<N,T>& y) { return Vec<N,T>(x) == y; }
SINTU Vec<N,M<T>> operator!=(U x, const Vec<N,T>& y) { return Vec<N,T>(x) != y; }
SINTU Vec<N,M<T>> operator<=(U x, const Vec<N,T>& y) { return Vec<N,T>(x) <= y; }
SINTU Vec<N,M<T>> operator>=(U x, const Vec<N,T>& y) { return Vec<N,T>(x) >= y; }
SINTU Vec<N,M<T>> operator< (U x, const Vec<N,T>& y) { return Vec<N,T>(x) <  y; }
SINTU Vec<N,M<T>> operator> (U x, const Vec<N,T>& y) { return Vec<N,T>(x) >  y; }
SINTU Vec<N,T>           pow(U x, const Vec<N,T>& y) { return pow(Vec<N,T>(x), y); }

// ... and same deal for vector/scalar operations.
SINTU Vec<N,T>    operator+ (const Vec<N,T>& x, U y) { return x +  Vec<N,T>(y); }
SINTU Vec<N,T>    operator- (const Vec<N,T>& x, U y) { return x -  Vec<N,T>(y); }
SINTU Vec<N,T>    operator* (const Vec<N,T>& x, U y) { return x *  Vec<N,T>(y); }
SINTU Vec<N,T>    operator/ (const Vec<N,T>& x, U y) { return x /  Vec<N,T>(y); }
SINTU Vec<N,T>    operator^ (const Vec<N,T>& x, U y) { return x ^  Vec<N,T>(y); }
SINTU Vec<N,T>    operator& (const Vec<N,T>& x, U y) { return x &  Vec<N,T>(y); }
SINTU Vec<N,T>    operator| (const Vec<N,T>& x, U y) { return x |  Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator==(const Vec<N,T>& x, U y) { return x == Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator!=(const Vec<N,T>& x, U y) { return x != Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator<=(const Vec<N,T>& x, U y) { return x <= Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator>=(const Vec<N,T>& x, U y) { return x >= Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator< (const Vec<N,T>& x, U y) { return x <  Vec<N,T>(y); }
SINTU Vec<N,M<T>> operator> (const Vec<N,T>& x, U y) { return x >  Vec<N,T>(y); }
SINTU Vec<N,T>           pow(const Vec<N,T>& x, U y) { return pow(x, Vec<N,T>(y)); }

// The various op= operators, for vectors...
SINT Vec<N,T>& operator+=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x + y); }
SINT Vec<N,T>& operator-=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x - y); }
SINT Vec<N,T>& operator*=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x * y); }
SINT Vec<N,T>& operator/=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x / y); }
SINT Vec<N,T>& operator^=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x ^ y); }
SINT Vec<N,T>& operator&=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x & y); }
SINT Vec<N,T>& operator|=(Vec<N,T>& x, const Vec<N,T>& y) { return (x = x | y); }

// ... for scalars...
SINTU Vec<N,T>& operator+=(Vec<N,T>& x, U y) { return (x = x + Vec<N,T>(y)); }
SINTU Vec<N,T>& operator-=(Vec<N,T>& x, U y) { return (x = x - Vec<N,T>(y)); }
SINTU Vec<N,T>& operator*=(Vec<N,T>& x, U y) { return (x = x * Vec<N,T>(y)); }
SINTU Vec<N,T>& operator/=(Vec<N,T>& x, U y) { return (x = x / Vec<N,T>(y)); }
SINTU Vec<N,T>& operator^=(Vec<N,T>& x, U y) { return (x = x ^ Vec<N,T>(y)); }
SINTU Vec<N,T>& operator&=(Vec<N,T>& x, U y) { return (x = x & Vec<N,T>(y)); }
SINTU Vec<N,T>& operator|=(Vec<N,T>& x, U y) { return (x = x | Vec<N,T>(y)); }

// ... and for shifts.
SINT Vec<N,T>& operator<<=(Vec<N,T>& x, int bits) { return (x = x << bits); }
SINT Vec<N,T>& operator>>=(Vec<N,T>& x, int bits) { return (x = x >> bits); }

// cast() Vec<N,S> to Vec<N,D>, as if applying a C-cast to each lane.
template <typename D, typename S>
SI Vec<1,D> cast(const Vec<1,S>& src) { return (D)src.val; }

template <typename D, int N, typename S>
SI Vec<N,D> cast(const Vec<N,S>& src) {
#if !defined(SKNX_NO_SIMD) && defined(__clang__)
    return to_vec(__builtin_convertvector(to_vext(src), VExt<N,D>));
#else
    return join(cast<D>(src.lo), cast<D>(src.hi));
#endif
}

// min/max match logic of std::min/std::max, which is important when NaN is involved.
SIT  T min(const Vec<1,T>& x) { return x.val; }
SIT  T max(const Vec<1,T>& x) { return x.val; }
SINT T min(const Vec<N,T>& x) { return std::min(min(x.lo), min(x.hi)); }
SINT T max(const Vec<N,T>& x) { return std::max(max(x.lo), max(x.hi)); }

SINT Vec<N,T> min(const Vec<N,T>& x, const Vec<N,T>& y) { return naive_if_then_else(y < x, y, x); }
SINT Vec<N,T> max(const Vec<N,T>& x, const Vec<N,T>& y) { return naive_if_then_else(x < y, y, x); }

SINTU Vec<N,T> min(const Vec<N,T>& x, U y) { return min(x, Vec<N,T>(y)); }
SINTU Vec<N,T> max(const Vec<N,T>& x, U y) { return max(x, Vec<N,T>(y)); }
SINTU Vec<N,T> min(U x, const Vec<N,T>& y) { return min(Vec<N,T>(x), y); }
SINTU Vec<N,T> max(U x, const Vec<N,T>& y) { return max(Vec<N,T>(x), y); }


// Shuffle values from a vector pretty arbitrarily:
//    skvx::Vec<4,float> rgba = {R,G,B,A};
//    shuffle<2,1,0,3>        (rgba) ~> {B,G,R,A}
//    shuffle<2,1>            (rgba) ~> {B,G}
//    shuffle<2,1,2,1,2,1,2,1>(rgba) ~> {B,G,B,G,B,G,B,G}
//    shuffle<3,3,3,3>        (rgba) ~> {A,A,A,A}
// The only real restriction is that the output also be a legal N=power-of-two sknx::Vec.
template <int... Ix, int N, typename T>
SI Vec<sizeof...(Ix),T> shuffle(const Vec<N,T>& x) {
#if !defined(SKNX_NO_SIMD) && defined(__clang__)
    return to_vec<sizeof...(Ix),T>(__builtin_shufflevector(to_vext(x), to_vext(x), Ix...));
#else
    return { x[Ix]... };
#endif
}

// fma() delivers a fused mul-add, even if that's really expensive.
SI Vec<1,float> fma(const Vec<1,float>& x, const Vec<1,float>& y, const Vec<1,float>& z) {
    return std::fma(x.val, y.val, z.val);
}
SIN Vec<N,float> fma(const Vec<N,float>& x, const Vec<N,float>& y, const Vec<N,float>& z) {
    return join(fma(x.lo, y.lo, z.lo),
                fma(x.hi, y.hi, z.hi));
}

template <int N, typename T, typename R, std::size_t... I>
SI skvx::Vec<N,R> map_(const skvx::Vec<N,T>& x, R (*fn)(T), std::index_sequence<I...>) {
    return { fn(x[I])... };
}

template <int N, typename T, typename R>
SI skvx::Vec<N,R> map(const skvx::Vec<N,T>& x, R (*fn)(T)) {
    return map_(x, fn, std::make_index_sequence<N>{});
}

SIN Vec<N,float>  atan(const Vec<N,float>& x) { return map(x,  atanf); }
SIN Vec<N,float>  ceil(const Vec<N,float>& x) { return map(x,  ceilf); }
SIN Vec<N,float> floor(const Vec<N,float>& x) { return map(x, floorf); }
SIN Vec<N,float> trunc(const Vec<N,float>& x) { return map(x, truncf); }
SIN Vec<N,float> round(const Vec<N,float>& x) { return map(x, roundf); }
SIN Vec<N,float>  sqrt(const Vec<N,float>& x) { return map(x,  sqrtf); }
SIN Vec<N,float>   abs(const Vec<N,float>& x) { return map(x,  fabsf); }
SIN Vec<N,float>   sin(const Vec<N,float>& x) { return map(x,   sinf); }
SIN Vec<N,float>   cos(const Vec<N,float>& x) { return map(x,   cosf); }
SIN Vec<N,float>   tan(const Vec<N,float>& x) { return map(x,   tanf); }

SI Vec<1,int> lrint(const Vec<1,float>& x) {
    return (int)lrintf(x.val);
}
SIN Vec<N,int> lrint(const Vec<N,float>& x) {
#if defined(__AVX__)
    if /*constexpr*/ (N == 8) {
        return unchecked_bit_pun<Vec<N,int>>(_mm256_cvtps_epi32(unchecked_bit_pun<__m256>(x)));
    }
#endif
#if defined(__SSE__)
    if /*constexpr*/ (N == 4) {
        return unchecked_bit_pun<Vec<N,int>>(_mm_cvtps_epi32(unchecked_bit_pun<__m128>(x)));
    }
#endif
    return join(lrint(x.lo),
                lrint(x.hi));
}

SIN Vec<N,float>   rcp(const Vec<N,float>& x) { return 1/x; }
SIN Vec<N,float> rsqrt(const Vec<N,float>& x) { return rcp(sqrt(x)); }
SIN Vec<N,float> fract(const Vec<N,float>& x) { return x - floor(x); }

// The default cases for to_half/from_half are borrowed from skcms,
// and assume inputs are finite and treat/flush denorm half floats as/to zero.
// Key constants to watch for:
//    - a float is 32-bit, 1-8-23 sign-exponent-mantissa, with 127 exponent bias;
//    - a half  is 16-bit, 1-5-10 sign-exponent-mantissa, with  15 exponent bias.
SIN Vec<N,uint16_t> to_half_finite_ftz(const Vec<N,float>& x) {
    Vec<N,uint32_t> sem = bit_pun<Vec<N,uint32_t>>(x),
                    s   = sem & 0x8000'0000,
                     em = sem ^ s,
              is_denorm =  em < 0x3880'0000;
    return cast<uint16_t>(if_then_else(is_denorm, Vec<N,uint32_t>(0)
                                                , (s>>16) + (em>>13) - ((127-15)<<10)));
}
SIN Vec<N,float> from_half_finite_ftz(const Vec<N,uint16_t>& x) {
    Vec<N,uint32_t> wide = cast<uint32_t>(x),
                      s  = wide & 0x8000,
                      em = wide ^ s;
    auto is_denorm = bit_pun<Vec<N,int32_t>>(em < 0x0400);
    return if_then_else(is_denorm, Vec<N,float>(0)
                                 , bit_pun<Vec<N,float>>( (s<<16) + (em<<13) + ((127-15)<<23) ));
}

// Like if_then_else(), these N=1 base cases won't actually be used unless explicitly called.
SI Vec<1,uint16_t> to_half(const Vec<1,float>&    x) { return   to_half_finite_ftz(x); }
SI Vec<1,float>  from_half(const Vec<1,uint16_t>& x) { return from_half_finite_ftz(x); }

SIN Vec<N,uint16_t> to_half(const Vec<N,float>& x) {
#if defined(__F16C__)
    if /*constexpr*/ (N == 8) {
        return unchecked_bit_pun<Vec<N,uint16_t>>(_mm256_cvtps_ph(unchecked_bit_pun<__m256>(x),
                                                                  _MM_FROUND_CUR_DIRECTION));
    }
#endif
#if defined(__aarch64__)
    if /*constexpr*/ (N == 4) {
        return unchecked_bit_pun<Vec<N,uint16_t>>(vcvt_f16_f32(unchecked_bit_pun<float32x4_t>(x)));

    }
#endif
    if /*constexpr*/ (N > 4) {
        return join(to_half(x.lo),
                    to_half(x.hi));
    }
    return to_half_finite_ftz(x);
}

SIN Vec<N,float> from_half(const Vec<N,uint16_t>& x) {
#if defined(__F16C__)
    if /*constexpr*/ (N == 8) {
        return unchecked_bit_pun<Vec<N,float>>(_mm256_cvtph_ps(unchecked_bit_pun<__m128i>(x)));
    }
#endif
#if defined(__aarch64__)
    if /*constexpr*/ (N == 4) {
        return unchecked_bit_pun<Vec<N,float>>(vcvt_f32_f16(unchecked_bit_pun<float16x4_t>(x)));
    }
#endif
    if /*constexpr*/ (N > 4) {
        return join(from_half(x.lo),
                    from_half(x.hi));
    }
    return from_half_finite_ftz(x);
}


// div255(x) = (x + 127) / 255 is a bit-exact rounding divide-by-255, packing down to 8-bit.
SIN Vec<N,uint8_t> div255(const Vec<N,uint16_t>& x) {
    return cast<uint8_t>( (x+127)/255 );
}

// approx_scale(x,y) approximates div255(cast<uint16_t>(x)*cast<uint16_t>(y)) within a bit,
// and is always perfect when x or y is 0 or 255.
SIN Vec<N,uint8_t> approx_scale(const Vec<N,uint8_t>& x, const Vec<N,uint8_t>& y) {
    // All of (x*y+x)/256, (x*y+y)/256, and (x*y+255)/256 meet the criteria above.
    // We happen to have historically picked (x*y+x)/256.
    auto X = cast<uint16_t>(x),
         Y = cast<uint16_t>(y);
    return cast<uint8_t>( (X*Y+X)/256 );
}

#if !defined(SKNX_NO_SIMD) && defined(__ARM_NEON)
    // With NEON we can do eight u8*u8 -> u16 in one instruction, vmull_u8 (read, mul-long).
    SI Vec<8,uint16_t> mull(const Vec<8,uint8_t>& x,
                            const Vec<8,uint8_t>& y) {
        return to_vec<8,uint16_t>(vmull_u8(to_vext(x),
                                           to_vext(y)));
    }

    SIN std::enable_if_t<(N < 8), Vec<N,uint16_t>> mull(const Vec<N,uint8_t>& x,
                                                        const Vec<N,uint8_t>& y) {
        // N < 8 --> double up data until N == 8, returning the part we need.
        return mull(join(x,x),
                    join(y,y)).lo;
    }

    SIN std::enable_if_t<(N > 8), Vec<N,uint16_t>> mull(const Vec<N,uint8_t>& x,
                                                        const Vec<N,uint8_t>& y) {
        // N > 8 --> usual join(lo,hi) strategy to recurse down to N == 8.
        return join(mull(x.lo, y.lo),
                    mull(x.hi, y.hi));
    }
#else
    // Nothing special when we don't have NEON... just cast up to 16-bit and multiply.
    SIN Vec<N,uint16_t> mull(const Vec<N,uint8_t>& x,
                             const Vec<N,uint8_t>& y) {
        return cast<uint16_t>(x)
             * cast<uint16_t>(y);
    }
#endif

#if !defined(SKNX_NO_SIMD)

    // Platform-specific specializations and overloads can now drop in here.

    #if defined(__AVX__)
        SI Vec<8,float> rsqrt(const Vec<8,float>& x) {
            return bit_pun<Vec<8,float>>(_mm256_rsqrt_ps(bit_pun<__m256>(x)));
        }
        SI Vec<8,float> rcp(const Vec<8,float>& x) {
            return bit_pun<Vec<8,float>>(_mm256_rcp_ps(bit_pun<__m256>(x)));
        }
    #endif

    #if defined(__SSE__)
        SI Vec<4,float> rsqrt(const Vec<4,float>& x) {
            return bit_pun<Vec<4,float>>(_mm_rsqrt_ps(bit_pun<__m128>(x)));
        }
        SI Vec<4,float> rcp(const Vec<4,float>& x) {
            return bit_pun<Vec<4,float>>(_mm_rcp_ps(bit_pun<__m128>(x)));
        }

        SI Vec<2,float> rsqrt(const Vec<2,float>& x) {
            return shuffle<0,1>(rsqrt(shuffle<0,1,0,1>(x)));
        }
        SI Vec<2,float>   rcp(const Vec<2,float>& x) {
            return shuffle<0,1>(  rcp(shuffle<0,1,0,1>(x)));
        }
    #endif

    #if defined(__AVX2__)
        SI Vec<4,float> fma(const Vec<4,float>& x, const Vec<4,float>& y, const Vec<4,float>& z) {
            return bit_pun<Vec<4,float>>(_mm_fmadd_ps(bit_pun<__m128>(x),
                                                      bit_pun<__m128>(y),
                                                      bit_pun<__m128>(z)));
        }

        SI Vec<8,float> fma(const Vec<8,float>& x, const Vec<8,float>& y, const Vec<8,float>& z) {
            return bit_pun<Vec<8,float>>(_mm256_fmadd_ps(bit_pun<__m256>(x),
                                                         bit_pun<__m256>(y),
                                                         bit_pun<__m256>(z)));
        }
    #elif defined(__aarch64__)
        SI Vec<4,float> fma(const Vec<4,float>& x, const Vec<4,float>& y, const Vec<4,float>& z) {
            // These instructions tend to work like z += xy, so the order here is z,x,y.
            return bit_pun<Vec<4,float>>(vfmaq_f32(bit_pun<float32x4_t>(z),
                                                   bit_pun<float32x4_t>(x),
                                                   bit_pun<float32x4_t>(y)));
        }
    #endif

    #if defined __wasm_simd128__
        SI bool any(const Vec<4, int32_t>& x) { return wasm_i32x4_any_true(to_vext(x)); }
        SI bool any(const Vec<4,uint32_t>& x) { return wasm_i32x4_any_true(to_vext(x)); }
        SI bool all(const Vec<4, int32_t>& x) { return wasm_i32x4_all_true(to_vext(x)); }
        SI bool all(const Vec<4,uint32_t>& x) { return wasm_i32x4_all_true(to_vext(x)); }
    #endif

#endif // !defined(SKNX_NO_SIMD)

}  // namespace skvx

#undef SINTU
#undef SINT
#undef SIT
#undef SI

#endif//SKVX_DEFINED
