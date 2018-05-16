/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../skcms.h"
#include "Curve.h"
#include "GaussNewton.h"
#include "Macros.h"
#include "PortableMath.h"
#include "TransferFunction.h"
#include <limits.h>
#include <stdlib.h>

// f(x) = skcms_PolyTF{A,B,C,D}(x) =
//     Cx                       x < D
//     A(x^3-1) + B(x^2-1) + 1  x ≥ D
//
// We'll fit C and D directly, and then hold them constant
// and fit the other part using Gauss-Newton, subject to
// the constraint that both parts meet at x=D:
//
//     CD = A(D^3-1) + B(D^2-1) + 1
//
// This lets us solve for B, reducing the optimization problem
// for that part down to just a single parameter A:
//
//         CD - A(D^3-1) - 1
//     B = -----------------
//               D^2-1
//
//                    x^2-1
//  f(x) = A(x^3-1) + ----- [CD - A(D^3-1) - 1] + 1
//                    D^2-1
//
//                  (x^2-1) (D^3-1)
//  ∂f/∂A = x^3-1 - ---------------
//                       D^2-1
//
// It's important to evaluate as f(x) as A(x^3-1) + B(x^2-1) + 1
// and not Ax^3 + Bx^2 + (1-A-B) to ensure that f(1.0f) == 1.0f.


static float eval_poly_tf(float A, float B, float C, float D,
                          float x) {
    return x < D ? C*x
                 : A*(x*x*x-1) + B*(x*x-1) + 1;
}

typedef struct {
    const skcms_Curve*  curve;
    const skcms_PolyTF* tf;
} rg_poly_tf_arg;

static float rg_poly_tf(float x, const void* ctx, const float P[3], float dfdP[3]) {
    const rg_poly_tf_arg* arg = (const rg_poly_tf_arg*)ctx;
    const skcms_PolyTF* tf = arg->tf;

    float A = P[0],
          C = tf->C,
          D = tf->D;
    float B = (C*D - A*(D*D*D - 1) - 1) / (D*D - 1);

    dfdP[0] = (x*x*x - 1) - (x*x-1)*(D*D*D-1)/(D*D-1);

    return skcms_eval_curve(arg->curve, x)
         -     eval_poly_tf(A,B,C,D,    x);
}

static bool fit_poly_tf(const skcms_Curve* curve, skcms_PolyTF* tf) {
    if (curve->table_entries > (uint32_t)INT_MAX) {
        return false;
    }

    const int N = curve->table_entries == 0 ? 256
                                            : (int)curve->table_entries;

    // We'll test the quality of our fit by roundtripping through a skcms_TransferFunction,
    // either the inverse of the curve itself if it is parametric, or of its approximation if not.
    skcms_TransferFunction baseline;
    float err;
    if (curve->table_entries == 0) {
        baseline = curve->parametric;
    } else if (!skcms_ApproximateCurve(curve, &baseline, &err)) {
        return false;
    }

    // We'll borrow the linear section from baseline, which is either
    // exactly correct, or already the approximation we'd use anyway.
    tf->C = baseline.c;
    tf->D = baseline.d;
    if (baseline.f != 0) {
        return false;  // Can't fit this (rare) kind of curve here.
    }

    // Detect linear baseline: (ax + b)^g + e --> ax ~~> Cx
    if (baseline.g == 1 && baseline.d == 0) {
        if (baseline.b + baseline.e == 0) {
            tf->A = 0;
            tf->B = 0;
            tf->C = baseline.a;
            tf->D = INFINITY_;   // Always use Cx, never Ax^3+Bx^2+(1-A-B)
            return true;
        } else {
            return false;  // Just like baseline.f != 0 above, can't represent this offset.
        }
    }

    // This case is less likely, but also guards against divide by zero below.
    if (tf->D == 1) {
        tf->A = 0;
        tf->B = 0;
        return true;
    }

    // Number of points already fit in the linear section.
    // If the curve isn't parametric and we approximated instead, this should be exact.
    const int L = (int)(tf->D * (N-1)) + 1;

    if (L == N-1) {
        // All points but one fit the linear section.
        // We could connect the last point with a quadratic, but let's just be lazy.
        return false;
    }

    skcms_TransferFunction inv;
    if (!skcms_TransferFunction_invert(&baseline, &inv)) {
        return false;
    }

    // Start with guess A = 0, i.e. f(x) ≈ x^2.
    float P[3] = {0, 0,0};
    for (int i = 0; i < 3; i++) {
        rg_poly_tf_arg arg = { curve, tf };
        if (!skcms_gauss_newton_step(rg_poly_tf, &arg,
                                     P,
                                     tf->D, 1, N-L)) {
            return false;
        }
    }

    float A = tf->A = P[0],
          C = tf->C,
          D = tf->D,
          B = tf->B = (C*D - A*(D*D*D - 1) - 1) / (D*D - 1);

    for (int i = 0; i < N; i++) {
        float x = i * (1.0f/(N-1));

        float rt = skcms_TransferFunction_eval(&inv, eval_poly_tf(A,B,C,D, x))
                 * (N-1) + 0.5f;
        if (!isfinitef_(rt) || rt >= N || rt < 0) {
            return false;
        }

        const int tol = (i == 0 || i == N-1) ? 0
                                             : N/256;
        if (abs(i - (int)rt) > tol) {
            return false;
        }
    }
    return true;
}

void skcms_OptimizeForSpeed(skcms_ICCProfile* profile) {
    for (int i = 0; profile->has_trc && i < 3; i++) {
        if (!profile->has_poly_tf[i]) {
             profile->has_poly_tf[i] = fit_poly_tf(&profile->trc[i], &profile->poly_tf[i]);
        }
    }
}
