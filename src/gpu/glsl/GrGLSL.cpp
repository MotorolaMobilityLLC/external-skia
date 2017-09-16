/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShaderCaps.h"
#include "SkString.h"
#include "../private/GrGLSL.h"

bool GrGLSLSupportsNamedFragmentShaderOutputs(GrGLSLGeneration gen) {
    switch (gen) {
        case k110_GrGLSLGeneration:
            return false;
        case k130_GrGLSLGeneration:
        case k140_GrGLSLGeneration:
        case k150_GrGLSLGeneration:
        case k330_GrGLSLGeneration:
        case k400_GrGLSLGeneration:
        case k420_GrGLSLGeneration:
        case k310es_GrGLSLGeneration:
        case k320es_GrGLSLGeneration:
            return true;
    }
    return false;
}

const char* GrGLSLTypeString(const GrShaderCaps* shaderCaps, GrSLType t) {
    switch (t) {
        case kVoid_GrSLType:
            return "void";
        case kHalf_GrSLType:
            return "half";
        case kHalf2_GrSLType:
            return "half2";
        case kHalf3_GrSLType:
            return "half3";
        case kHalf4_GrSLType:
            return "half4";
        case kHighFloat_GrSLType:
            return "highfloat";
        case kHighFloat2_GrSLType:
            return "highfloat2";
        case kHighFloat3_GrSLType:
            return "highfloat3";
        case kHighFloat4_GrSLType:
            return "highfloat4";
        case kUint2_GrSLType:
            if (shaderCaps->integerSupport()) {
                return "uint2";
            } else {
                // uint2 (aka uvec2) isn't supported in GLSL ES 1.00/GLSL 1.20
                return "highfloat2";
            }
        case kInt2_GrSLType:
            return "int2";
        case kInt3_GrSLType:
            return "int3";
        case kInt4_GrSLType:
            return "int4";
        case kHighFloat2x2_GrSLType:
            return "highfloat2x2";
        case kHighFloat3x3_GrSLType:
            return "highfloat3x3";
        case kHighFloat4x4_GrSLType:
            return "highfloat4x4";
        case kHalf2x2_GrSLType:
            return "half2x2";
        case kHalf3x3_GrSLType:
            return "half3x3";
        case kHalf4x4_GrSLType:
            return "half4x4";
        case kTexture2DSampler_GrSLType:
            return "sampler2D";
        case kITexture2DSampler_GrSLType:
            return "isampler2D";
        case kTextureExternalSampler_GrSLType:
            return "samplerExternalOES";
        case kTexture2DRectSampler_GrSLType:
            return "sampler2DRect";
        case kBufferSampler_GrSLType:
            return "samplerBuffer";
        case kBool_GrSLType:
            return "bool";
        case kInt_GrSLType:
            return "int";
        case kUint_GrSLType:
            return "uint";
        case kShort_GrSLType:
            return "short";
        case kUShort_GrSLType:
            return "ushort";
        case kTexture2D_GrSLType:
            return "texture2D";
        case kSampler_GrSLType:
            return "sampler";
        case kImageStorage2D_GrSLType:
            return "image2D";
        case kIImageStorage2D_GrSLType:
            return "iimage2D";
    }
    SK_ABORT("Unknown shader var type.");
    return ""; // suppress warning
}

void GrGLSLAppendDefaultFloatPrecisionDeclaration(GrSLPrecision p,
                                                  const GrShaderCaps& shaderCaps,
                                                  SkString* out) {
    if (shaderCaps.usesPrecisionModifiers()) {
        switch (p) {
            case kHigh_GrSLPrecision:
                out->append("precision highp float;\n");
                break;
            case kMedium_GrSLPrecision:
                out->append("precision mediump float;\n");
                break;
            case kLow_GrSLPrecision:
                out->append("precision lowp float;\n");
                break;
            default:
                SK_ABORT("Unknown precision value.");
        }
    }
}
