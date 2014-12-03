/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOptDrawState.h"

#include "GrDrawState.h"
#include "GrDrawTargetCaps.h"
#include "GrGpu.h"
#include "GrProcOptInfo.h"

GrOptDrawState::GrOptDrawState(const GrDrawState& drawState,
                               GrGpu* gpu,
                               const ScissorState& scissorState,
                               const GrDeviceCoordTexture* dstCopy,
                               GrGpu::DrawType drawType) {

    GrBlendCoeff optSrcCoeff;
    GrBlendCoeff optDstCoeff;
    GrDrawState::BlendOpt blendOpt = drawState.getBlendOpt(false, &optSrcCoeff, &optDstCoeff);

    // When path rendering the stencil settings are not always set on the draw state
    // so we must check the draw type. In cases where we will skip drawing we simply return a
    // null GrOptDrawState.
    if (GrDrawState::kSkipDraw_BlendOpt == blendOpt && GrGpu::kStencilPath_DrawType != drawType) {
        // Set the fields that don't default init and return. The lack of a render target will
        // indicate that this can be skipped.
        fFlags = 0;
        fDrawFace = GrDrawState::kInvalid_DrawFace;
        fSrcBlend = kZero_GrBlendCoeff;
        fDstBlend = kZero_GrBlendCoeff;
        fBlendConstant = 0x0;
        fViewMatrix.reset();
        return;
    }

    fRenderTarget.reset(drawState.fRenderTarget.get());
    SkASSERT(fRenderTarget);
    fScissorState = scissorState;
    fViewMatrix = drawState.getViewMatrix();
    fBlendConstant = drawState.getBlendConstant();
    fStencilSettings = drawState.getStencil();
    fDrawFace = drawState.getDrawFace();
    fSrcBlend = optSrcCoeff;
    fDstBlend = optDstCoeff;

    // TODO move this out of optDrawState
    if (dstCopy) {
        fDstCopy = *dstCopy;
    }

    GrProgramDesc::DescInfo descInfo;

    fFlags = 0;
    if (drawState.isHWAntialias()) {
        fFlags |= kHWAA_Flag;
    }
    if (drawState.isColorWriteDisabled()) {
        fFlags |= kDisableColorWrite_Flag;
    }
    if (drawState.isDither()) {
        fFlags |= kDither_Flag;
    }

    descInfo.fHasVertexColor = drawState.hasGeometryProcessor() &&
                               drawState.getGeometryProcessor()->hasVertexColor();

    descInfo.fHasVertexCoverage = drawState.hasGeometryProcessor() &&
                                 drawState.getGeometryProcessor()->hasVertexCoverage();

    bool hasLocalCoords = drawState.hasGeometryProcessor() &&
                          drawState.getGeometryProcessor()->hasLocalCoords();

    const GrProcOptInfo& colorPOI = drawState.colorProcInfo();
    int firstColorStageIdx = colorPOI.firstEffectiveStageIndex();
    descInfo.fInputColorIsUsed = colorPOI.inputColorIsUsed();
    fColor = colorPOI.inputColorToEffectiveStage();
    if (colorPOI.removeVertexAttrib()) {
        descInfo.fHasVertexColor = false;
    }

    // TODO: Once we can handle single or four channel input into coverage stages then we can use
    // drawState's coverageProcInfo (like color above) to set this initial information.
    int firstCoverageStageIdx = 0;
    descInfo.fInputCoverageIsUsed = true;
    fCoverage = drawState.getCoverage();

    this->adjustProgramForBlendOpt(drawState, blendOpt, &descInfo, &firstColorStageIdx,
                                   &firstCoverageStageIdx);

    this->getStageStats(drawState, firstColorStageIdx, firstCoverageStageIdx, hasLocalCoords,
                        &descInfo);

    // Copy GeometryProcesssor from DS or ODS
    SkASSERT(GrGpu::IsPathRenderingDrawType(drawType) ||
             GrGpu::kStencilPath_DrawType ||
             drawState.hasGeometryProcessor());
    fGeometryProcessor.reset(drawState.getGeometryProcessor());

    // Copy Stages from DS to ODS
    for (int i = firstColorStageIdx; i < drawState.numColorStages(); ++i) {
        SkNEW_APPEND_TO_TARRAY(&fFragmentStages,
                               GrPendingFragmentStage,
                               (drawState.fColorStages[i], hasLocalCoords));
    }
    fNumColorStages = fFragmentStages.count();
    for (int i = firstCoverageStageIdx; i < drawState.numCoverageStages(); ++i) {
        SkNEW_APPEND_TO_TARRAY(&fFragmentStages,
                               GrPendingFragmentStage,
                               (drawState.fCoverageStages[i], hasLocalCoords));
    }

    this->setOutputStateInfo(drawState, blendOpt, *gpu->caps(), &descInfo);

    // now create a key
    gpu->buildProgramDesc(*this, descInfo, drawType, &fDesc);
};

void GrOptDrawState::setOutputStateInfo(const GrDrawState& ds,
                                        GrDrawState::BlendOpt blendOpt,
                                        const GrDrawTargetCaps& caps,
                                        GrProgramDesc::DescInfo* descInfo) {
    // Set this default and then possibly change our mind if there is coverage.
    descInfo->fPrimaryOutputType = GrProgramDesc::kModulate_PrimaryOutputType;
    descInfo->fSecondaryOutputType = GrProgramDesc::kNone_SecondaryOutputType;

    // Determine whether we should use dual source blending or shader code to keep coverage
    // separate from color.
    bool keepCoverageSeparate = !(GrDrawState::kCoverageAsAlpha_BlendOpt == blendOpt ||
                                  GrDrawState::kEmitCoverage_BlendOpt == blendOpt);
    if (keepCoverageSeparate && !ds.hasSolidCoverage()) {
        if (caps.dualSourceBlendingSupport()) {
            if (kZero_GrBlendCoeff == fDstBlend) {
                // write the coverage value to second color
                descInfo->fSecondaryOutputType = GrProgramDesc::kCoverage_SecondaryOutputType;
                fDstBlend = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
            } else if (kSA_GrBlendCoeff == fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                descInfo->fSecondaryOutputType = GrProgramDesc::kCoverageISA_SecondaryOutputType;
                fDstBlend = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
            } else if (kSC_GrBlendCoeff == fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                descInfo->fSecondaryOutputType = GrProgramDesc::kCoverageISC_SecondaryOutputType;
                fDstBlend = (GrBlendCoeff)GrGpu::kIS2C_GrBlendCoeff;
            }
        } else if (descInfo->fReadsDst &&
                   kOne_GrBlendCoeff == fSrcBlend &&
                   kZero_GrBlendCoeff == fDstBlend) {
            descInfo->fPrimaryOutputType = GrProgramDesc::kCombineWithDst_PrimaryOutputType;
        }
    }
}

void GrOptDrawState::adjustProgramForBlendOpt(const GrDrawState& ds,
                                              GrDrawState::BlendOpt blendOpt,
                                              GrProgramDesc::DescInfo* descInfo,
                                              int* firstColorStageIdx,
                                              int* firstCoverageStageIdx) {
    switch (blendOpt) {
        case GrDrawState::kNone_BlendOpt:
        case GrDrawState::kSkipDraw_BlendOpt:
        case GrDrawState::kCoverageAsAlpha_BlendOpt:
            break;
        case GrDrawState::kEmitCoverage_BlendOpt:
            fColor = 0xffffffff;
            descInfo->fInputColorIsUsed = true;
            *firstColorStageIdx = ds.numColorStages();
            descInfo->fHasVertexColor = false;
            break;
        case GrDrawState::kEmitTransBlack_BlendOpt:
            fColor = 0;
            fCoverage = 0xff;
            descInfo->fInputColorIsUsed = true;
            descInfo->fInputCoverageIsUsed = true;
            *firstColorStageIdx = ds.numColorStages();
            *firstCoverageStageIdx = ds.numCoverageStages();
            descInfo->fHasVertexColor = false;
            descInfo->fHasVertexCoverage = false;
            break;
    }
}

static void get_stage_stats(const GrFragmentStage& stage, bool* readsDst, bool* readsFragPosition) {
    if (stage.getProcessor()->willReadDstColor()) {
        *readsDst = true;
    }
    if (stage.getProcessor()->willReadFragmentPosition()) {
        *readsFragPosition = true;
    }
}

void GrOptDrawState::getStageStats(const GrDrawState& ds, int firstColorStageIdx,
                                   int firstCoverageStageIdx, bool hasLocalCoords,
                                   GrProgramDesc::DescInfo* descInfo) {
    // We will need a local coord attrib if there is one currently set on the optState and we are
    // actually generating some effect code
    descInfo->fRequiresLocalCoordAttrib = hasLocalCoords &&
        ds.numTotalStages() - firstColorStageIdx - firstCoverageStageIdx > 0;

    descInfo->fReadsDst = false;
    descInfo->fReadsFragPosition = false;

    for (int s = firstColorStageIdx; s < ds.numColorStages(); ++s) {
        const GrFragmentStage& stage = ds.getColorStage(s);
        get_stage_stats(stage, &descInfo->fReadsDst, &descInfo->fReadsFragPosition);
    }
    for (int s = firstCoverageStageIdx; s < ds.numCoverageStages(); ++s) {
        const GrFragmentStage& stage = ds.getCoverageStage(s);
        get_stage_stats(stage, &descInfo->fReadsDst, &descInfo->fReadsFragPosition);
    }
    if (ds.hasGeometryProcessor()) {
        const GrGeometryProcessor& gp = *ds.getGeometryProcessor();
        descInfo->fReadsFragPosition = descInfo->fReadsFragPosition || gp.willReadFragmentPosition();
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrOptDrawState::operator== (const GrOptDrawState& that) const {
    if (this->fDesc != that.fDesc) {
        return false;
    }
    bool hasVertexColors = this->fDesc.header().fColorInput == GrProgramDesc::kAttribute_ColorInput;
    if (!hasVertexColors && this->fColor != that.fColor) {
        return false;
    }

    if (this->getRenderTarget() != that.getRenderTarget() ||
        this->fScissorState != that.fScissorState ||
        !this->fViewMatrix.cheapEqualTo(that.fViewMatrix) ||
        this->fSrcBlend != that.fSrcBlend ||
        this->fDstBlend != that.fDstBlend ||
        this->fBlendConstant != that.fBlendConstant ||
        this->fFlags != that.fFlags ||
        this->fStencilSettings != that.fStencilSettings ||
        this->fDrawFace != that.fDrawFace ||
        this->fDstCopy.texture() != that.fDstCopy.texture()) {
        return false;
    }

    bool hasVertexCoverage =
            this->fDesc.header().fCoverageInput == GrProgramDesc::kAttribute_ColorInput;
    if (!hasVertexCoverage && this->fCoverage != that.fCoverage) {
        return false;
    }

    if (this->hasGeometryProcessor()) {
        if (!that.hasGeometryProcessor()) {
            return false;
        } else if (!this->getGeometryProcessor()->isEqual(*that.getGeometryProcessor())) {
            return false;
        }
    } else if (that.hasGeometryProcessor()) {
        return false;
    }

    // The program desc comparison should have already assured that the stage counts match.
    SkASSERT(this->numFragmentStages() == that.numFragmentStages());
    for (int i = 0; i < this->numFragmentStages(); i++) {

        if (this->getFragmentStage(i) != that.getFragmentStage(i)) {
            return false;
        }
    }
    return true;
}

