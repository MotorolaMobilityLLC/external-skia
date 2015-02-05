/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomXfermodePriv_DEFINED
#define GrCustomXfermodePriv_DEFINED

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrTextureAccess.h"
#include "GrXferProcessor.h"
#include "SkXfermode.h"

class GrGLCaps;
class GrGLFragmentProcessor;
class GrInvariantOutput;
class GrProcessorKeyBuilder;
class GrTexture;

///////////////////////////////////////////////////////////////////////////////
// Fragment Processor
///////////////////////////////////////////////////////////////////////////////

class GrCustomXferFP : public GrFragmentProcessor {
public:
    GrCustomXferFP(SkXfermode::Mode mode, GrTexture* background);

    void getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE; 

    GrGLFragmentProcessor* createGLInstance() const SK_OVERRIDE;

    const char* name() const SK_OVERRIDE { return "Custom Xfermode"; }

    SkXfermode::Mode mode() const { return fMode; }
    const GrTextureAccess&  backgroundAccess() const { return fBackgroundAccess; }

private:
    bool onIsEqual(const GrFragmentProcessor& other) const SK_OVERRIDE; 

    void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    SkXfermode::Mode fMode;
    GrCoordTransform fBackgroundTransform;
    GrTextureAccess  fBackgroundAccess;

    typedef GrFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class GrCustomXP : public GrXferProcessor {
public:
    static GrXferProcessor* Create(SkXfermode::Mode mode) {
        if (!GrCustomXfermode::IsSupportedMode(mode)) {
            return NULL;
        } else {
            return SkNEW_ARGS(GrCustomXP, (mode));
        }
    }

    ~GrCustomXP() SK_OVERRIDE {};

    const char* name() const SK_OVERRIDE { return "Custom Xfermode"; }

    void getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    GrGLXferProcessor* createGLInstance() const SK_OVERRIDE;

    bool hasSecondaryOutput() const SK_OVERRIDE { return false; }

    GrXferProcessor::OptFlags getOptimizations(const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& coveragePOI,
                                               bool doesStencilWrite,
                                               GrColor* overrideColor,
                                               const GrDrawTargetCaps& caps) SK_OVERRIDE;

    void getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const SK_OVERRIDE {
        blendInfo->fSrcBlend = kOne_GrBlendCoeff;
        blendInfo->fDstBlend = kZero_GrBlendCoeff;
        blendInfo->fBlendConstant = 0;
    }

    SkXfermode::Mode mode() const { return fMode; }

private:
    GrCustomXP(SkXfermode::Mode mode);

    bool onIsEqual(const GrXferProcessor& xpBase) const SK_OVERRIDE;

    SkXfermode::Mode fMode;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrCustomXPFactory : public GrXPFactory {
public:
    GrCustomXPFactory(SkXfermode::Mode mode); 

    GrXferProcessor* createXferProcessor(const GrProcOptInfo& colorPOI,
                                         const GrProcOptInfo& coveragePOI) const SK_OVERRIDE {
        return GrCustomXP::Create(fMode);
    }

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const SK_OVERRIDE {
        return true;
    }

    bool canApplyCoverage(const GrProcOptInfo& colorPOI,
                          const GrProcOptInfo& coveragePOI) const SK_OVERRIDE {
        return true;
    }

    bool canTweakAlphaForCoverage() const SK_OVERRIDE {
        return false;
    }

    void getInvariantOutput(const GrProcOptInfo& colorPOI, const GrProcOptInfo& coveragePOI,
                            GrXPFactory::InvariantOutput*) const SK_OVERRIDE;

    bool willReadDst() const SK_OVERRIDE { return true; }

private:
    bool onIsEqual(const GrXPFactory& xpfBase) const SK_OVERRIDE {
        const GrCustomXPFactory& xpf = xpfBase.cast<GrCustomXPFactory>();
        return fMode == xpf.fMode;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    SkXfermode::Mode fMode;

    typedef GrXPFactory INHERITED;
};
#endif

