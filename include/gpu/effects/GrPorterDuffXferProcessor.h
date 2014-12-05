/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPorterDuffXferProcessor_DEFINED
#define GrPorterDuffXferProcessor_DEFINED

#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "SkXfermode.h"

class GrDrawState;
class GrInvariantOutput;

class GrPorterDuffXferProcessor : public GrXferProcessor {
public:
    static GrXferProcessor* Create(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend) {
        return SkNEW_ARGS(GrPorterDuffXferProcessor, (srcBlend, dstBlend));
    }

    virtual ~GrPorterDuffXferProcessor();

    virtual const char* name() const { return "Porter Duff"; }

    virtual void getGLProcessorKey(const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    virtual GrGLFragmentProcessor* createGLInstance() const SK_OVERRIDE;

private:
    GrPorterDuffXferProcessor(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend);

    virtual bool onIsEqual(const GrFragmentProcessor& fpBase) const SK_OVERRIDE {
        const GrPorterDuffXferProcessor& xp = fpBase.cast<GrPorterDuffXferProcessor>();
        if (fSrcBlend != xp.fSrcBlend || fDstBlend != xp.fDstBlend) {
            return false;
        }
        return true;
    }

    virtual void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE;

    GrBlendCoeff fSrcBlend;
    GrBlendCoeff fDstBlend;
    
    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrPorterDuffXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create(SkXfermode::Mode mode); 

    static GrXPFactory* Create(SkXfermode::Coeff src, SkXfermode::Coeff dst) {
        return SkNEW_ARGS(GrPorterDuffXPFactory, ((GrBlendCoeff)(src), (GrBlendCoeff)(dst)));
    }

    static GrXPFactory* Create(GrBlendCoeff src, GrBlendCoeff dst) {
        return SkNEW_ARGS(GrPorterDuffXPFactory, (src, dst));
    }

    const GrXferProcessor* createXferProcessor() const SK_OVERRIDE;

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const SK_OVERRIDE;

private:
    GrPorterDuffXPFactory(GrBlendCoeff src, GrBlendCoeff dst); 

    bool onIsEqual(const GrXPFactory& xpfBase) const SK_OVERRIDE {
        const GrPorterDuffXPFactory& xpf = xpfBase.cast<GrPorterDuffXPFactory>();
        return (fSrc == xpf.fSrc && fDst == xpf.fDst);
    }

    GrBlendCoeff fSrc;
    GrBlendCoeff fDst;

    typedef GrXPFactory INHERITED;
};

#endif
