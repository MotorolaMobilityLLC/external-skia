/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrShaderVar.h"
#include "glsl/GrGLSLCaps.h"

static const char* type_modifier_string(GrShaderVar::TypeModifier t) {
    switch (t) {
        case GrShaderVar::kNone_TypeModifier: return "";
        case GrShaderVar::kIn_TypeModifier: return "in";
        case GrShaderVar::kInOut_TypeModifier: return "inout";
        case GrShaderVar::kOut_TypeModifier: return "out";
        case GrShaderVar::kUniform_TypeModifier: return "uniform";
    }
    SkFAIL("Unknown shader variable type modifier.");
    return "";
}

void GrShaderVar::setImageStorageFormat(GrImageStorageFormat format) {
    const char* formatStr = nullptr;
    switch (format) {
        case GrImageStorageFormat::kRGBA8:
            formatStr = "rgba8";
            break;
        case GrImageStorageFormat::kRGBA8i:
            formatStr = "rgba8i";
            break;
        case GrImageStorageFormat::kRGBA16f:
            formatStr = "rgba16f";
            break;
        case GrImageStorageFormat::kRGBA32f:
            formatStr = "rgba32f";
            break;
    }
    this->addLayoutQualifier(formatStr);
    SkASSERT(formatStr);
}

void GrShaderVar::appendDecl(const GrGLSLCaps* glslCaps, SkString* out) const {
    SkASSERT(kDefault_GrSLPrecision == fPrecision || GrSLTypeAcceptsPrecision(fType));
    SkString layout = fLayoutQualifier;
    if (!fLayoutQualifier.isEmpty()) {
        out->appendf("layout(%s) ", fLayoutQualifier.c_str());
    }
    out->append(fExtraModifiers);
    if (this->getTypeModifier() != kNone_TypeModifier) {
        out->append(type_modifier_string(this->getTypeModifier()));
        out->append(" ");
    }
    GrSLType effectiveType = this->getType();
    if (glslCaps->usesPrecisionModifiers() && GrSLTypeAcceptsPrecision(effectiveType)) {
        // Desktop GLSL has added precision qualifiers but they don't do anything.
        out->appendf("%s ", GrGLSLPrecisionString(fPrecision));
    }
    if (this->isArray()) {
        if (this->isUnsizedArray()) {
            out->appendf("%s %s[]",
                         GrGLSLTypeString(effectiveType),
                         this->getName().c_str());
        } else {
            SkASSERT(this->getArrayCount() > 0);
            out->appendf("%s %s[%d]",
                         GrGLSLTypeString(effectiveType),
                         this->getName().c_str(),
                         this->getArrayCount());
        }
    } else {
        out->appendf("%s %s",
                     GrGLSLTypeString(effectiveType),
                     this->getName().c_str());
    }
}
