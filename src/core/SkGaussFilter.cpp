/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGaussFilter.h"

#include <cmath>
#include "SkTypes.h"

static constexpr double kPi = 3.14159265358979323846264338327950288;

// The value when we can stop expanding the filter. The spec implies that 3% is acceptable, but
// we just use 1%.
static constexpr double kGoodEnough = 1.0 / 100.0;

// Normalize the values of gauss to 1.0, and make sure they add to one.
// NB if n == 1, then this will force gauss[0] == 1.
static void normalize(int n, double* gauss) {
    // Carefully add from smallest to largest to calculate the normalizing sum.
    double sum = 0;
    for (int i = n-1; i >= 1; i--) {
        sum += 2 * gauss[i];
    }
    sum += gauss[0];

    // Normalize gauss.
    for (int i = 0; i < n; i++) {
        gauss[i] /= sum;
    }

    // The factors should sum to 1. Take any remaining slop, and add it to gauss[0]. Add the
    // values in such a way to maintain the most accuracy.
    sum = 0;
    for (int i = n - 1; i >= 1; i--) {
        sum += 2 * gauss[i];
    }

    gauss[0] = 1 - sum;
}

static int calculate_bessel_factors(double sigma, double *gauss) {
    auto var = sigma * sigma;

    // The two functions below come from the equations in "Handbook of Mathematical Functions"
    // by Abramowitz and Stegun. Specifically, equation 9.6.10 on page 375. Bessel0 is given
    // explicitly as 9.6.12
    // BesselI_0 for 0 <= sigma < 2.
    // NB the k = 0 factor is just sum = 1.0.
    auto besselI_0 = [](double t) -> double {
        auto tSquaredOver4 = t * t / 4.0;
        auto sum = 1.0;
        auto factor = 1.0;
        auto k = 1;
        // Use a variable number of loops. When sigma is small, this only requires 3-4 loops, but
        // when sigma is near 2, it could require 10 loops. The same holds for BesselI_1.
        while(factor > 1.0/1000000.0) {
            factor *= tSquaredOver4 / (k * k);
            sum += factor;
            k += 1;
        }
        return sum;
    };
    // BesselI_1 for 0 <= sigma < 2.
    auto besselI_1 = [](double t) -> double {
        auto tSquaredOver4 = t * t / 4.0;
        auto sum = t / 2.0;
        auto factor = sum;
        auto k = 1;
        while (factor > 1.0/1000000.0) {
            factor *= tSquaredOver4 / (k * (k + 1));
            sum += factor;
            k += 1;
        }
        return sum;
    };

    // The following formula for calculating the Gaussian kernel is from
    // "Scale-Space for Discrete Signals" by Tony Lindeberg.
    // gauss(n; var) = besselI_n(var) / (e^var)
    auto d = std::exp(var);
    double b[6] = {besselI_0(var), besselI_1(var)};
    gauss[0] = b[0]/d;
    gauss[1] = b[1]/d;

    int n = 1;
    // The recurrence relation below is from "Numerical Recipes" 3rd Edition.
    // Equation 6.5.16 p.282
    while (gauss[n] > kGoodEnough) {
        b[n+1] = -(2*n/var) * b[n] + b[n-1];
        gauss[n+1] = b[n+1] / d;
        n += 1;
    }

    normalize(n, gauss);

    return n;
}

static int calculate_gauss_factors(double sigma, double* gauss) {
    SkASSERT(0 <= sigma && sigma < 2);

    // From the SVG blur spec: 8.13 Filter primitive <feGaussianBlur>.
    // H(x) = exp(-x^2/ (2s^2)) / sqrt(2π * s^2)
    auto var = sigma * sigma;
    auto expGaussDenom = -2 * var;
    auto normalizeDenom = std::sqrt(2 * kPi) * sigma;

    // Use the recursion relation from "Incremental Computation of the Gaussian"  by Ken
    // Turkowski in GPUGems 3. Page 877.
    double g0 = 1.0 / normalizeDenom;
    double g1 = std::exp(1.0 / expGaussDenom);
    double g2 = g1 * g1;

    gauss[0] = g0;
    g0 *= g1;
    g1 *= g2;
    gauss[1] = g0;

    int n = 1;
    while (gauss[n] > kGoodEnough) {
        g0 *= g1;
        g1 *= g2;
        gauss[n+1] = g0;
        n += 1;
    }

    normalize(n, gauss);

    return n;
}

SkGaussFilter::SkGaussFilter(double sigma, Type type) {
    SkASSERT(0 <= sigma && sigma < 2);

    if (type == Type::Bessel) {
        fN = calculate_bessel_factors(sigma, fBasis);
    } else {
        fN = calculate_gauss_factors(sigma, fBasis);
    }
}
