/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformSteps_DEFINED
#define SkColorSpaceXformSteps_DEFINED

#include "SkColorSpace.h"
#include "SkImageInfo.h"

struct SkColorSpaceXformSteps {
    SkColorSpaceXformSteps(SkColorSpace* src, SkAlphaType srcAT,
                           SkColorSpace* dst);

    static SkColorSpaceXformSteps UnpremulToUnpremul(SkColorSpace* src, SkColorSpace* dst) {
        // The need to transform unpremul to unpremul comes up often enough that it's
        // nice to centralize it here, especially because this use of kOpaque_SkAlphaType
        // isn't the most intuitive.  We basically want to always skip unpremul and premul.
        return SkColorSpaceXformSteps(src, kOpaque_SkAlphaType, dst);
    }

    bool unpremul;
    bool linearize;
    bool gamut_transform;
    bool encode;
    bool premul;

    SkColorSpaceTransferFn srcTF,     // Apply for linearize.
                           dstTFInv;  // Apply for encode.
    float src_to_dst_matrix[9];       // Apply this 3x3 row-major matrix for gamut_transform.
};



#endif//SkColorSpaceXformSteps_DEFINED
