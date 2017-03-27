/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProcessorSet.h"
#include "GrAppliedClip.h"
#include "GrCaps.h"
#include "GrPipelineAnalysis.h"

GrProcessorSet::GrProcessorSet(GrPaint&& paint) {
    fXPFactory = paint.fXPFactory;
    fFlags = 0;
    if (paint.numColorFragmentProcessors() <= kMaxColorProcessors) {
        fColorFragmentProcessorCnt = paint.numColorFragmentProcessors();
        fFragmentProcessors.reset(paint.numTotalFragmentProcessors());
        int i = 0;
        for (auto& fp : paint.fColorFragmentProcessors) {
            fFragmentProcessors[i++] = fp.release();
        }
        for (auto& fp : paint.fCoverageFragmentProcessors) {
            fFragmentProcessors[i++] = fp.release();
        }
        if (paint.usesDistanceVectorField()) {
            fFlags |= kUseDistanceVectorField_Flag;
        }
    } else {
        SkDebugf("Insane number of color fragment processors in paint. Dropping all processors.");
        fColorFragmentProcessorCnt = 0;
    }
    if (paint.getDisableOutputConversionToSRGB()) {
        fFlags |= kDisableOutputConversionToSRGB_Flag;
    }
    if (paint.getAllowSRGBInputs()) {
        fFlags |= kAllowSRGBInputs_Flag;
    }
}

GrProcessorSet::~GrProcessorSet() {
    for (int i = fFragmentProcessorOffset; i < fFragmentProcessors.count(); ++i) {
        if (this->isPendingExecution()) {
            fFragmentProcessors[i]->completedExecution();
        } else {
            fFragmentProcessors[i]->unref();
        }
    }
}

void GrProcessorSet::makePendingExecution() {
    SkASSERT(!(kPendingExecution_Flag & fFlags));
    fFlags |= kPendingExecution_Flag;
    for (int i = fFragmentProcessorOffset; i < fFragmentProcessors.count(); ++i) {
        fFragmentProcessors[i]->addPendingExecution();
        fFragmentProcessors[i]->unref();
    }
}

bool GrProcessorSet::operator==(const GrProcessorSet& that) const {
    int fpCount = this->numFragmentProcessors();
    if (((fFlags ^ that.fFlags) & ~kPendingExecution_Flag) ||
        fpCount != that.numFragmentProcessors() ||
        fColorFragmentProcessorCnt != that.fColorFragmentProcessorCnt) {
        return false;
    }

    for (int i = 0; i < fpCount; ++i) {
        int a = i + fFragmentProcessorOffset;
        int b = i + that.fFragmentProcessorOffset;
        if (!fFragmentProcessors[a]->isEqual(*that.fFragmentProcessors[b])) {
            return false;
        }
    }
    if (fXPFactory != that.fXPFactory) {
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////

void GrProcessorSet::FragmentProcessorAnalysis::internalInit(
        const GrPipelineAnalysisColor& colorInput,
        const GrPipelineAnalysisCoverage coverageInput,
        const GrProcessorSet& processors,
        const GrFragmentProcessor* clipFP,
        const GrCaps& caps) {
    GrColorFragmentProcessorAnalysis colorInfo(colorInput);
    fCompatibleWithCoverageAsAlpha = GrPipelineAnalysisCoverage::kLCD != coverageInput;
    fValidInputColor = colorInput.isConstant(&fInputColor);

    const GrFragmentProcessor* const* fps =
            processors.fFragmentProcessors.get() + processors.fFragmentProcessorOffset;
    colorInfo.analyzeProcessors(fps, processors.fColorFragmentProcessorCnt);
    fCompatibleWithCoverageAsAlpha &= colorInfo.allProcessorsCompatibleWithCoverageAsAlpha();
    fps += processors.fColorFragmentProcessorCnt;
    int n = processors.numCoverageFragmentProcessors();
    bool hasCoverageFP = n > 0;
    fUsesLocalCoords = colorInfo.usesLocalCoords();
    for (int i = 0; i < n; ++i) {
        if (!fps[i]->compatibleWithCoverageAsAlpha()) {
            fCompatibleWithCoverageAsAlpha = false;
            // Other than tests that exercise atypical behavior we expect all coverage FPs to be
            // compatible with the coverage-as-alpha optimization.
            GrCapsDebugf(&caps, "Coverage FP is not compatible with coverage as alpha.\n");
        }
        fUsesLocalCoords |= fps[i]->usesLocalCoords();
    }

    if (clipFP) {
        fCompatibleWithCoverageAsAlpha &= clipFP->compatibleWithCoverageAsAlpha();
        fUsesLocalCoords |= clipFP->usesLocalCoords();
        hasCoverageFP = true;
    }
    fInitialColorProcessorsToEliminate = colorInfo.initialProcessorsToEliminate(&fInputColor);
    fValidInputColor |= SkToBool(fInitialColorProcessorsToEliminate);

    GrPipelineAnalysisColor outputColor = colorInfo.outputColor();
    if (outputColor.isConstant(&fKnownOutputColor)) {
        fOutputColorType = static_cast<unsigned>(outputColor.isOpaque() ? ColorType::kOpaqueConstant
                                                                        : ColorType::kConstant);
    } else if (outputColor.isOpaque()) {
        fOutputColorType = static_cast<unsigned>(ColorType::kOpaque);
    } else {
        fOutputColorType = static_cast<unsigned>(ColorType::kUnknown);
    }

    if (GrPipelineAnalysisCoverage::kLCD == coverageInput) {
        fOutputCoverageType = static_cast<unsigned>(GrPipelineAnalysisCoverage::kLCD);
    } else if (hasCoverageFP || GrPipelineAnalysisCoverage::kSingleChannel == coverageInput) {
        fOutputCoverageType = static_cast<unsigned>(GrPipelineAnalysisCoverage::kSingleChannel);
    } else {
        fOutputCoverageType = static_cast<unsigned>(GrPipelineAnalysisCoverage::kNone);
    }
}

void GrProcessorSet::FragmentProcessorAnalysis::init(const GrPipelineAnalysisColor& colorInput,
                                                     const GrPipelineAnalysisCoverage coverageInput,
                                                     const GrProcessorSet& processors,
                                                     const GrAppliedClip* appliedClip,
                                                     const GrCaps& caps) {
    const GrFragmentProcessor* clipFP =
            appliedClip ? appliedClip->clipCoverageFragmentProcessor() : nullptr;
    this->internalInit(colorInput, coverageInput, processors, clipFP, caps);
    fIsInitializedWithProcessorSet = true;
}

GrProcessorSet::FragmentProcessorAnalysis::FragmentProcessorAnalysis(
        const GrPipelineAnalysisColor& colorInput,
        const GrPipelineAnalysisCoverage coverageInput,
        const GrCaps& caps)
        : FragmentProcessorAnalysis() {
    this->internalInit(colorInput, coverageInput, GrProcessorSet(GrPaint()), nullptr, caps);
}

void GrProcessorSet::analyzeAndEliminateFragmentProcessors(
        FragmentProcessorAnalysis* analysis,
        const GrPipelineAnalysisColor& colorInput,
        const GrPipelineAnalysisCoverage coverageInput,
        const GrAppliedClip* clip,
        const GrCaps& caps) {
    analysis->init(colorInput, coverageInput, *this, clip, caps);
    if (analysis->fInitialColorProcessorsToEliminate > 0) {
        for (unsigned i = 0; i < analysis->fInitialColorProcessorsToEliminate; ++i) {
            if (this->isPendingExecution()) {
                fFragmentProcessors[i + fFragmentProcessorOffset]->completedExecution();
            } else {
                fFragmentProcessors[i + fFragmentProcessorOffset]->unref();
            }
            fFragmentProcessors[i + fFragmentProcessorOffset] = nullptr;
        }
        fFragmentProcessorOffset += analysis->fInitialColorProcessorsToEliminate;
        fColorFragmentProcessorCnt -= analysis->fInitialColorProcessorsToEliminate;
        SkASSERT(fFragmentProcessorOffset + fColorFragmentProcessorCnt <=
                 fFragmentProcessors.count());
        analysis->fInitialColorProcessorsToEliminate = 0;
    }
}
