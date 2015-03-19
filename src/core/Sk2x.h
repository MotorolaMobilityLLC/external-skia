/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk2x_DEFINED
#define Sk2x_DEFINED

#include "SkTypes.h"

#define SK2X_PREAMBLE 1
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
        #include "../opts/Sk2x_sse.h"
    #else
        #include "../opts/Sk2x_none.h"
    #endif
#undef SK2X_PREAMBLE

template <typename T> class Sk2x;
typedef Sk2x<float>  Sk2f;
typedef Sk2x<double> Sk2d;

#if SK_SCALAR_IS_FLOAT
    typedef Sk2f Sk2s;
#elif SK_SCALAR_IS_DOUBLE
    typedef Sk2d Sk2s;
#endif

// This API is meant to be manageably small, not comprehensive.
// Please talk to mtklein if you find yourself wanting more.
template <typename T> class Sk2x {
public:
    Sk2x();  // Uninitialized; use Sk2x(0) for zero.
    explicit Sk2x(T);  // Same as Sk2x(T,T);
    Sk2x(T, T);

    Sk2x(const Sk2x& o) { *this = o; }
    Sk2x& operator=(const Sk2x&);

    // These assume no particular alignment.
    static Sk2x Load(const T[2]);
    void store(T[2]) const;

    Sk2x      add(const Sk2x&) const;
    Sk2x subtract(const Sk2x&) const;
    Sk2x multiply(const Sk2x&) const;

    Sk2x rsqrt() const;   // Approximate 1/this->sqrt().
    Sk2x  sqrt() const;   // this->multiply(this->rsqrt()) may be faster, but less precise.

    static Sk2x Min(const Sk2x&, const Sk2x&);
    static Sk2x Max(const Sk2x&, const Sk2x&);

private:
#define SK2X_PRIVATE 1
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
        #include "../opts/Sk2x_sse.h"
    #else
        #include "../opts/Sk2x_none.h"
    #endif
#undef SK2X_PRIVATE
};

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include "../opts/Sk2x_sse.h"
#else
    #include "../opts/Sk2x_none.h"
#endif

#endif//Sk2x_DEFINED
