/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoverageSetOpXP_DEFINED
#define GrCoverageSetOpXP_DEFINED

#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "SkRegion.h"

class GrProcOptInfo;

class GrCoverageSetOpXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create(SkRegion::Op regionOp, bool invertCoverage = false);

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const SK_OVERRIDE {
        return true;
    }

    bool canApplyCoverage(const GrProcOptInfo& colorPOI,
                          const GrProcOptInfo& coveragePOI) const SK_OVERRIDE {
        return true;
    }

    bool canTweakAlphaForCoverage() const SK_OVERRIDE { return false; }

    void getInvariantOutput(const GrProcOptInfo& colorPOI, const GrProcOptInfo& coveragePOI,
                            GrXPFactory::InvariantOutput*) const SK_OVERRIDE;

private:
    GrCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage);

    GrXferProcessor* onCreateXferProcessor(const GrProcOptInfo& colorPOI,
                                           const GrProcOptInfo& coveragePOI,
                                           const GrDeviceCoordTexture* dstCopy) const SK_OVERRIDE;

    bool willReadDstColor(const GrProcOptInfo& colorPOI,
                          const GrProcOptInfo& coveragePOI) const SK_OVERRIDE {
        return false;
    }

    bool onIsEqual(const GrXPFactory& xpfBase) const SK_OVERRIDE {
        const GrCoverageSetOpXPFactory& xpf = xpfBase.cast<GrCoverageSetOpXPFactory>();
        return fRegionOp == xpf.fRegionOp;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    SkRegion::Op fRegionOp;
    bool         fInvertCoverage;

    typedef GrXPFactory INHERITED;
};
#endif

