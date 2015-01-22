/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawState.h"

#include "GrBlend.h"
#include "GrOptDrawState.h"
#include "GrPaint.h"
#include "GrProcOptInfo.h"
#include "GrXferProcessor.h"
#include "effects/GrPorterDuffXferProcessor.h"

GrDrawState::GrDrawState()
    : fFlagBits(0x0)
    , fDrawFace(kBoth_DrawFace)
    , fColorProcInfoValid(false)
    , fCoverageProcInfoValid(false)
    , fColorCache(GrColor_ILLEGAL)
    , fCoverageCache(GrColor_ILLEGAL)
    , fColorPrimProc(NULL)
    , fCoveragePrimProc(NULL) {
    SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
}

GrDrawState& GrDrawState::operator=(const GrDrawState& that) {
    fRenderTarget.reset(SkSafeRef(that.fRenderTarget.get()));
    fFlagBits = that.fFlagBits;
    fStencilSettings = that.fStencilSettings;
    fDrawFace = that.fDrawFace;
    fXPFactory.reset(SkRef(that.getXPFactory()));
    fColorStages = that.fColorStages;
    fCoverageStages = that.fCoverageStages;

    fColorProcInfoValid = that.fColorProcInfoValid;
    fCoverageProcInfoValid = that.fCoverageProcInfoValid;
    fColorCache = that.fColorCache;
    fCoverageCache = that.fCoverageCache;
    fColorPrimProc = that.fColorPrimProc;
    fCoveragePrimProc = that.fCoveragePrimProc;
    if (fColorProcInfoValid) {
        fColorProcInfo = that.fColorProcInfo;
    }
    if (fCoverageProcInfoValid) {
        fCoverageProcInfo = that.fCoverageProcInfo;
    }
    return *this;
}

void GrDrawState::setFromPaint(const GrPaint& paint, GrRenderTarget* rt) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numFragmentStages());

    fColorStages.reset();
    fCoverageStages.reset();

    for (int i = 0; i < paint.numColorStages(); ++i) {
        fColorStages.push_back(paint.getColorStage(i));
    }

    for (int i = 0; i < paint.numCoverageStages(); ++i) {
        fCoverageStages.push_back(paint.getCoverageStage(i));
    }

    fXPFactory.reset(SkRef(paint.getXPFactory()));

    this->setRenderTarget(rt);

    // These have no equivalent in GrPaint, set them to defaults
    fDrawFace = kBoth_DrawFace;
    fStencilSettings.setDisabled();
    fFlagBits = 0;

    // Enable the clip bit
    this->enableState(GrDrawState::kClip_StateBit);

    this->setState(GrDrawState::kDither_StateBit, paint.isDither());
    this->setState(GrDrawState::kHWAntialias_StateBit, paint.isAntiAlias());

    fColorProcInfoValid = false;
    fCoverageProcInfoValid = false;

    fColorCache = GrColor_ILLEGAL;
    fCoverageCache = GrColor_ILLEGAL;

    fColorPrimProc = NULL;
    fCoveragePrimProc = NULL;
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawState::canUseFracCoveragePrimProc(GrColor color, const GrDrawTargetCaps& caps) const {
    if (caps.dualSourceBlendingSupport()) {
        return true;
    }

    this->calcColorInvariantOutput(color);

    // The coverage isn't actually white, its unknown, but this will produce the same effect
    // TODO we want to cache the result of this call, but we can probably clean up the interface
    // so we don't have to pass in a seemingly known coverage
    this->calcCoverageInvariantOutput(GrColor_WHITE);
    return this->getXPFactory()->canApplyCoverage(fColorProcInfo, fCoverageProcInfo);
}

//////////////////////////////////////////////////////////////////////////////s

bool GrDrawState::willEffectReadDstColor() const {
    return this->getXPFactory()->willReadDst();
}

void GrDrawState::AutoRestoreEffects::set(GrDrawState* ds) {
    if (fDrawState) {
        int m = fDrawState->numColorStages() - fColorEffectCnt;
        SkASSERT(m >= 0);
        fDrawState->fColorStages.pop_back_n(m);

        int n = fDrawState->numCoverageStages() - fCoverageEffectCnt;
        SkASSERT(n >= 0);
        fDrawState->fCoverageStages.pop_back_n(n);
        if (m + n > 0) {
            fDrawState->fColorProcInfoValid = false;
            fDrawState->fCoverageProcInfoValid = false;
        }
        SkDEBUGCODE(--fDrawState->fBlockEffectRemovalCnt;)
    }
    fDrawState = ds;
    if (NULL != ds) {
        fColorEffectCnt = ds->numColorStages();
        fCoverageEffectCnt = ds->numCoverageStages();
        SkDEBUGCODE(++ds->fBlockEffectRemovalCnt;)
    }
}

////////////////////////////////////////////////////////////////////////////////

// Some blend modes allow folding a fractional coverage value into the color's alpha channel, while
// others will blend incorrectly.
bool GrDrawState::canTweakAlphaForCoverage() const {
    return this->getXPFactory()->canTweakAlphaForCoverage();
}

////////////////////////////////////////////////////////////////////////////////

GrDrawState::~GrDrawState() {
    SkASSERT(0 == fBlockEffectRemovalCnt);
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawState::willBlendWithDst(const GrPrimitiveProcessor* pp) const {
    this->calcColorInvariantOutput(pp);
    this->calcCoverageInvariantOutput(pp);
    
    GrXPFactory::InvariantOutput output;
    fXPFactory->getInvariantOutput(fColorProcInfo, fCoverageProcInfo, &output);
    return output.fWillBlendWithDst;
}

void GrDrawState::calcColorInvariantOutput(const GrPrimitiveProcessor* pp) const {
    if (!fColorProcInfoValid || fColorPrimProc != pp) {
        fColorProcInfo.calcColorWithPrimProc(pp, fColorStages.begin(), this->numColorStages());
        fColorProcInfoValid = true;
        fColorPrimProc = pp;
    }
}

void GrDrawState::calcCoverageInvariantOutput(const GrPrimitiveProcessor* pp) const {
    if (!fCoverageProcInfoValid ||  fCoveragePrimProc != pp) {
        fCoverageProcInfo.calcCoverageWithPrimProc(pp, fCoverageStages.begin(),
                                                   this->numCoverageStages());
        fCoverageProcInfoValid = true;
        fCoveragePrimProc = pp;
    }
}

void GrDrawState::calcColorInvariantOutput(GrColor color) const {
    if (!fColorProcInfoValid || color != fColorCache) {
        GrColorComponentFlags flags = kRGBA_GrColorComponentFlags;
        fColorProcInfo.calcWithInitialValues(fColorStages.begin(), this->numColorStages(), color,
                                             flags, false);
        fColorProcInfoValid = true;
        fColorCache = color;
    }
}

void GrDrawState::calcCoverageInvariantOutput(GrColor coverage) const {
    if (!fCoverageProcInfoValid || coverage != fCoverageCache) {
        GrColorComponentFlags flags = kRGBA_GrColorComponentFlags;
        fCoverageProcInfo.calcWithInitialValues(fCoverageStages.begin(),
                                                this->numCoverageStages(), coverage, flags,
                                                true);
        fCoverageProcInfoValid = true;
        fCoverageCache = coverage;
    }
}
