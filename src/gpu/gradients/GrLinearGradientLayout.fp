/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in half4x4 gradientMatrix;

@coordTransform {
    gradientMatrix
}

void main() {
    sk_OutColor = half4(sk_TransformedCoords2D[0].x);
}

//////////////////////////////////////////////////////////////////////////////

@header {
    #include "SkLinearGradient.h"
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(const SkLinearGradient& gradient,
                                                     const GrFPArgs& args);
}

@cppEnd {
    std::unique_ptr<GrFragmentProcessor> GrLinearGradientLayout::Make(
            const SkLinearGradient& grad, const GrFPArgs& args) {
        SkMatrix matrix;
        if (!grad.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }
        matrix.postConcat(grad.getGradientMatrix());
        return std::unique_ptr<GrFragmentProcessor>(new GrLinearGradientLayout(matrix));
    }
}
