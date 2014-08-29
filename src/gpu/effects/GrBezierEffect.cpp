/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrBezierEffect.h"

#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLVertexEffect.h"
#include "GrTBackendEffectFactory.h"

class GrGLConicEffect : public GrGLVertexEffect {
public:
    GrGLConicEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrDrawEffect&) SK_OVERRIDE {}

private:
    GrEffectEdgeType fEdgeType;

    typedef GrGLVertexEffect INHERITED;
};

GrGLConicEffect::GrGLConicEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    const GrConicEffect& ce = drawEffect.castEffect<GrConicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLConicEffect::emitCode(GrGLFullProgramBuilder* builder,
                               const GrDrawEffect& drawEffect,
                               const GrEffectKey& key,
                               const char* outputColor,
                               const char* inputColor,
                               const TransformedCoordsArray&,
                               const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;

    builder->addVarying(kVec4f_GrSLType, "ConicCoeffs",
                              &vsName, &fsName);

    GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
    const SkString* attr0Name =
        vsBuilder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
    vsBuilder->codeAppendf("%s = %s;", vsName, attr0Name->c_str());

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();

    GrGLShaderVar edgeAlpha("edgeAlpha", kFloat_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar dklmdx("dklmdx", kVec3f_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar dklmdy("dklmdy", kVec3f_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar dfdx("dfdx", kFloat_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar dfdy("dfdy", kFloat_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar gF("gF", kVec2f_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar gFM("gFM", kFloat_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar func("func", kFloat_GrSLType, 0, GrGLShaderVar::kHigh_Precision);

    fsBuilder->declAppend(edgeAlpha);
    fsBuilder->declAppend(dklmdx);
    fsBuilder->declAppend(dklmdy);
    fsBuilder->declAppend(dfdx);
    fsBuilder->declAppend(dfdy);
    fsBuilder->declAppend(gF);
    fsBuilder->declAppend(gFM);
    fsBuilder->declAppend(func);

    switch (fEdgeType) {
        case kHairlineAA_GrEffectEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), fsName);
            fsBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), fsName);
            fsBuilder->codeAppendf("%s = 2.0 * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdx.c_str(), fsName, dklmdx.c_str(), fsName, dklmdx.c_str(),
                                   fsName, dklmdx.c_str());
            fsBuilder->codeAppendf("%s = 2.0 * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdy.c_str(), fsName, dklmdy.c_str(), fsName, dklmdy.c_str(),
                                   fsName, dklmdy.c_str());
            fsBuilder->codeAppendf("%s = vec2(%s, %s);",
                                   gF.c_str(), dfdx.c_str(), dfdy.c_str());
            fsBuilder->codeAppendf("%s = sqrt(dot(%s, %s));",
                                   gFM.c_str(), gF.c_str(), gF.c_str());
            fsBuilder->codeAppendf("%s = %s.x * %s.x - %s.y * %s.z;",
                                   func.c_str(), fsName, fsName, fsName, fsName);
            fsBuilder->codeAppendf("%s = abs(%s);", func.c_str(), func.c_str());
            fsBuilder->codeAppendf("%s = %s / %s;",
                                   edgeAlpha.c_str(), func.c_str(), gFM.c_str());
            fsBuilder->codeAppendf("%s = max(1.0 - %s, 0.0);",
                                   edgeAlpha.c_str(), edgeAlpha.c_str());
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppendf("%s = %s * %s * (3.0 - 2.0 * %s);",
            //                        edgeAlpha.c_str(), edgeAlpha.c_str(), edgeAlpha.c_str(),
            //                        edgeAlpha.c_str());
            break;
        }
        case kFillAA_GrEffectEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), fsName);
            fsBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), fsName);
            fsBuilder->codeAppendf("%s = 2.0 * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdx.c_str(), fsName, dklmdx.c_str(), fsName, dklmdx.c_str(),
                                   fsName, dklmdx.c_str());
            fsBuilder->codeAppendf("%s = 2.0 * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdy.c_str(), fsName, dklmdy.c_str(), fsName, dklmdy.c_str(),
                                   fsName, dklmdy.c_str());
            fsBuilder->codeAppendf("%s = vec2(%s, %s);",
                                   gF.c_str(), dfdx.c_str(), dfdy.c_str());
            fsBuilder->codeAppendf("%s = sqrt(dot(%s, %s));",
                                   gFM.c_str(), gF.c_str(), gF.c_str());
            fsBuilder->codeAppendf("%s = %s.x * %s.x - %s.y * %s.z;",
                                   func.c_str(), fsName, fsName, fsName, fsName);
            fsBuilder->codeAppendf("%s = %s / %s;",
                                   edgeAlpha.c_str(), func.c_str(), gFM.c_str());
            fsBuilder->codeAppendf("%s = clamp(1.0 - %s, 0.0, 1.0);",
                                   edgeAlpha.c_str(), edgeAlpha.c_str());
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppendf("%s = %s * %s * (3.0 - 2.0 * %s);",
            //                        edgeAlpha.c_str(), edgeAlpha.c_str(), edgeAlpha.c_str(),
            //                        edgeAlpha.c_str());
            break;
        }
        case kFillBW_GrEffectEdgeType: {
            fsBuilder->codeAppendf("%s = %s.x * %s.x - %s.y * %s.z;",
                                   edgeAlpha.c_str(), fsName, fsName, fsName, fsName);
            fsBuilder->codeAppendf("%s = float(%s < 0.0);",
                                  edgeAlpha.c_str(), edgeAlpha.c_str());
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    fsBuilder->codeAppendf("%s = %s;", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1(edgeAlpha.c_str())).c_str());
}

void GrGLConicEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                             GrEffectKeyBuilder* b) {
    const GrConicEffect& ce = drawEffect.castEffect<GrConicEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrConicEffect::~GrConicEffect() {}

const GrBackendEffectFactory& GrConicEffect::getFactory() const {
    return GrTBackendEffectFactory<GrConicEffect>::getInstance();
}

GrConicEffect::GrConicEffect(GrEffectEdgeType edgeType) : GrVertexEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
    fEdgeType = edgeType;
}

bool GrConicEffect::onIsEqual(const GrEffect& other) const {
    const GrConicEffect& ce = CastEffect<GrConicEffect>(other);
    return (ce.fEdgeType == fEdgeType);
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrConicEffect);

GrEffect* GrConicEffect::TestCreate(SkRandom* random,
                                    GrContext*,
                                    const GrDrawTargetCaps& caps,
                                    GrTexture*[]) {
    GrEffect* effect;
    do {
        GrEffectEdgeType edgeType = static_cast<GrEffectEdgeType>(
                                                    random->nextULessThan(kGrEffectEdgeTypeCnt));
        effect = GrConicEffect::Create(edgeType, caps);
    } while (NULL == effect);
    return effect;
}

//////////////////////////////////////////////////////////////////////////////
// Quad
//////////////////////////////////////////////////////////////////////////////

class GrGLQuadEffect : public GrGLVertexEffect {
public:
    GrGLQuadEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrDrawEffect&) SK_OVERRIDE {}

private:
    GrEffectEdgeType fEdgeType;

    typedef GrGLVertexEffect INHERITED;
};

GrGLQuadEffect::GrGLQuadEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    const GrQuadEffect& ce = drawEffect.castEffect<GrQuadEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLQuadEffect::emitCode(GrGLFullProgramBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              const GrEffectKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;
    builder->addVarying(kVec4f_GrSLType, "HairQuadEdge", &vsName, &fsName);

    GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
    const SkString* attrName =
        vsBuilder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
    vsBuilder->codeAppendf("%s = %s;", vsName, attrName->c_str());

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    fsBuilder->codeAppendf("float edgeAlpha;");

    switch (fEdgeType) {
        case kHairlineAA_GrEffectEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec2 duvdx = dFdx(%s.xy);", fsName);
            fsBuilder->codeAppendf("vec2 duvdy = dFdy(%s.xy);", fsName);
            fsBuilder->codeAppendf("vec2 gF = vec2(2.0 * %s.x * duvdx.x - duvdx.y,"
                                   "               2.0 * %s.x * duvdy.x - duvdy.y);",
                                   fsName, fsName);
            fsBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);", fsName, fsName, fsName);
            fsBuilder->codeAppend("edgeAlpha = sqrt(edgeAlpha * edgeAlpha / dot(gF, gF));");
            fsBuilder->codeAppend("edgeAlpha = max(1.0 - edgeAlpha, 0.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillAA_GrEffectEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec2 duvdx = dFdx(%s.xy);", fsName);
            fsBuilder->codeAppendf("vec2 duvdy = dFdy(%s.xy);", fsName);
            fsBuilder->codeAppendf("vec2 gF = vec2(2.0 * %s.x * duvdx.x - duvdx.y,"
                                   "               2.0 * %s.x * duvdy.x - duvdy.y);",
                                   fsName, fsName);
            fsBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);", fsName, fsName, fsName);
            fsBuilder->codeAppend("edgeAlpha = edgeAlpha / sqrt(dot(gF, gF));");
            fsBuilder->codeAppend("edgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillBW_GrEffectEdgeType: {
            fsBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);", fsName, fsName, fsName);
            fsBuilder->codeAppend("edgeAlpha = float(edgeAlpha < 0.0);");
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    fsBuilder->codeAppendf("%s = %s;", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("edgeAlpha")).c_str());
}

void GrGLQuadEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                            GrEffectKeyBuilder* b) {
    const GrQuadEffect& ce = drawEffect.castEffect<GrQuadEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrQuadEffect::~GrQuadEffect() {}

const GrBackendEffectFactory& GrQuadEffect::getFactory() const {
    return GrTBackendEffectFactory<GrQuadEffect>::getInstance();
}

GrQuadEffect::GrQuadEffect(GrEffectEdgeType edgeType) : GrVertexEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
    fEdgeType = edgeType;
}

bool GrQuadEffect::onIsEqual(const GrEffect& other) const {
    const GrQuadEffect& ce = CastEffect<GrQuadEffect>(other);
    return (ce.fEdgeType == fEdgeType);
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrQuadEffect);

GrEffect* GrQuadEffect::TestCreate(SkRandom* random,
                                   GrContext*,
                                   const GrDrawTargetCaps& caps,
                                   GrTexture*[]) {
    GrEffect* effect;
    do {
        GrEffectEdgeType edgeType = static_cast<GrEffectEdgeType>(
                                                    random->nextULessThan(kGrEffectEdgeTypeCnt));
        effect = GrQuadEffect::Create(edgeType, caps);
    } while (NULL == effect);
    return effect;
}

//////////////////////////////////////////////////////////////////////////////
// Cubic
//////////////////////////////////////////////////////////////////////////////

class GrGLCubicEffect : public GrGLVertexEffect {
public:
    GrGLCubicEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrDrawEffect&) SK_OVERRIDE {}

private:
    GrEffectEdgeType fEdgeType;

    typedef GrGLVertexEffect INHERITED;
};

GrGLCubicEffect::GrGLCubicEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    const GrCubicEffect& ce = drawEffect.castEffect<GrCubicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLCubicEffect::emitCode(GrGLFullProgramBuilder* builder,
                               const GrDrawEffect& drawEffect,
                               const GrEffectKey& key,
                               const char* outputColor,
                               const char* inputColor,
                               const TransformedCoordsArray&,
                               const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;

    builder->addVarying(kVec4f_GrSLType, "CubicCoeffs",
                              &vsName, &fsName);

    GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
    const SkString* attr0Name =
        vsBuilder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
    vsBuilder->codeAppendf("%s = %s;", vsName, attr0Name->c_str());

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    fsBuilder->codeAppend("float edgeAlpha;");

    switch (fEdgeType) {
        case kHairlineAA_GrEffectEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec3 dklmdx = dFdx(%s.xyz);", fsName);
            fsBuilder->codeAppendf("vec3 dklmdy = dFdy(%s.xyz);", fsName);
            fsBuilder->codeAppendf("float dfdx ="
                                   "3.0*%s.x*%s.x * dklmdx.x - %s.y * dklmdx.z - %s.z * dklmdx.y;",
                                   fsName, fsName, fsName, fsName);
            fsBuilder->codeAppendf("float dfdy ="
                                   "3.0*%s.x*%s.x * dklmdy.x - %s.y * dklmdy.z - %s.z * dklmdy.y;",
                                   fsName, fsName, fsName, fsName);
            fsBuilder->codeAppend("vec2 gF = vec2(dfdx, dfdy);");
            fsBuilder->codeAppend("float gFM = sqrt(dot(gF, gF));");
            fsBuilder->codeAppendf("float func = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                   fsName, fsName, fsName, fsName, fsName);
            fsBuilder->codeAppend("func = abs(func);");
            fsBuilder->codeAppend("edgeAlpha = func / gFM;");
            fsBuilder->codeAppend("edgeAlpha = max(1.0 - edgeAlpha, 0.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillAA_GrEffectEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec3 dklmdx = dFdx(%s.xyz);", fsName);
            fsBuilder->codeAppendf("vec3 dklmdy = dFdy(%s.xyz);", fsName);
            fsBuilder->codeAppendf("float dfdx ="
                                   "3.0*%s.x*%s.x * dklmdx.x - %s.y * dklmdx.z - %s.z * dklmdx.y;",
                                   fsName, fsName, fsName, fsName);
            fsBuilder->codeAppendf("float dfdy ="
                                   "3.0*%s.x*%s.x * dklmdy.x - %s.y * dklmdy.z - %s.z * dklmdy.y;",
                                   fsName, fsName, fsName, fsName);
            fsBuilder->codeAppend("vec2 gF = vec2(dfdx, dfdy);");
            fsBuilder->codeAppend("float gFM = sqrt(dot(gF, gF));");
            fsBuilder->codeAppendf("float func = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                   fsName, fsName, fsName, fsName, fsName);
            fsBuilder->codeAppend("edgeAlpha = func / gFM;");
            fsBuilder->codeAppend("edgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillBW_GrEffectEdgeType: {
            fsBuilder->codeAppendf("edgeAlpha = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                   fsName, fsName, fsName, fsName, fsName);
            fsBuilder->codeAppend("edgeAlpha = float(edgeAlpha < 0.0);");
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    fsBuilder->codeAppendf("%s = %s;", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("edgeAlpha")).c_str());
}

void GrGLCubicEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                             GrEffectKeyBuilder* b) {
    const GrCubicEffect& ce = drawEffect.castEffect<GrCubicEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrCubicEffect::~GrCubicEffect() {}

const GrBackendEffectFactory& GrCubicEffect::getFactory() const {
    return GrTBackendEffectFactory<GrCubicEffect>::getInstance();
}

GrCubicEffect::GrCubicEffect(GrEffectEdgeType edgeType) : GrVertexEffect() {
    this->addVertexAttrib(kVec4f_GrSLType);
    fEdgeType = edgeType;
}

bool GrCubicEffect::onIsEqual(const GrEffect& other) const {
    const GrCubicEffect& ce = CastEffect<GrCubicEffect>(other);
    return (ce.fEdgeType == fEdgeType);
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrCubicEffect);

GrEffect* GrCubicEffect::TestCreate(SkRandom* random,
                                    GrContext*,
                                    const GrDrawTargetCaps& caps,
                                    GrTexture*[]) {
    GrEffect* effect;
    do {
        GrEffectEdgeType edgeType = static_cast<GrEffectEdgeType>(
                                                    random->nextULessThan(kGrEffectEdgeTypeCnt));
        effect = GrCubicEffect::Create(edgeType, caps);
    } while (NULL == effect);
    return effect;
}
