/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawState_DEFINED
#define GrDrawState_DEFINED


#include "GrBlend.h"
#include "GrDrawTargetCaps.h"
#include "GrGeometryProcessor.h"
#include "GrGpuResourceRef.h"
#include "GrFragmentStage.h"
#include "GrProcOptInfo.h"
#include "GrRenderTarget.h"
#include "GrStencil.h"
#include "GrXferProcessor.h"
#include "SkMatrix.h"
#include "effects/GrCoverageSetOpXP.h"
#include "effects/GrDisableColorXP.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

class GrDrawTargetCaps;
class GrPaint;
class GrTexture;

class GrDrawState {
public:
    GrDrawState() {
        SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
        this->reset();
    }

    GrDrawState(const SkMatrix& initialViewMatrix) {
        SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
        this->reset(initialViewMatrix);
    }

    /**
     * Copies another draw state.
     **/
    GrDrawState(const GrDrawState& state) {
        SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
        *this = state;
    }

    /**
     * Copies another draw state with a preconcat to the view matrix.
     **/
    GrDrawState(const GrDrawState& state, const SkMatrix& preConcatMatrix);

    virtual ~GrDrawState();

    /**
     * Resets to the default state. GrProcessors will be removed from all stages.
     */
    void reset() { this->onReset(NULL); }

    void reset(const SkMatrix& initialViewMatrix) { this->onReset(&initialViewMatrix); }

    /**
     * Initializes the GrDrawState based on a GrPaint, view matrix and render target. Note that
     * GrDrawState encompasses more than GrPaint. Aspects of GrDrawState that have no GrPaint
     * equivalents are set to default values with the exception of vertex attribute state which
     * is unmodified by this function and clipping which will be enabled.
     */
    void setFromPaint(const GrPaint& , const SkMatrix& viewMatrix, GrRenderTarget*);

    /// @}

    /**
     * Depending on features available in the underlying 3D API and the color blend mode requested
     * it may or may not be possible to correctly blend with fractional pixel coverage generated by
     * the fragment shader.
     *
     * This function considers the current draw state and the draw target's capabilities to
     * determine whether coverage can be handled correctly. This function assumes that the caller
     * intends to specify fractional pixel coverage via a primitive processor but may not have
     * specified it yet.
     */
    bool canUseFracCoveragePrimProc(GrColor color, const GrDrawTargetCaps& caps) const;

    /**
     * This function returns true if the render target destination pixel values will be read for
     * blending during draw.
     */
    bool willBlendWithDst(const GrPrimitiveProcessor*) const;

    ///////////////////////////////////////////////////////////////////////////
    /// @name Effect Stages
    /// Each stage hosts a GrProcessor. The effect produces an output color or coverage in the
    /// fragment shader. Its inputs are the output from the previous stage as well as some variables
    /// available to it in the fragment and vertex shader (e.g. the vertex position, the dst color,
    /// the fragment position, local coordinates).
    ///
    /// The stages are divided into two sets, color-computing and coverage-computing. The final
    /// color stage produces the final pixel color. The coverage-computing stages function exactly
    /// as the color-computing but the output of the final coverage stage is treated as a fractional
    /// pixel coverage rather than as input to the src/dst color blend step.
    ///
    /// The input color to the first color-stage is either the constant color or interpolated
    /// per-vertex colors. The input to the first coverage stage is either a constant coverage
    /// (usually full-coverage) or interpolated per-vertex coverage.
    ///
    /// See the documentation of kCoverageDrawing_StateBit for information about disabling the
    /// the color / coverage distinction.
    ////

    int numColorStages() const { return fColorStages.count(); }
    int numCoverageStages() const { return fCoverageStages.count(); }
    int numFragmentStages() const { return this->numColorStages() + this->numCoverageStages(); }

    const GrXPFactory* getXPFactory() const { return fXPFactory.get(); }

    const GrFragmentStage& getColorStage(int idx) const { return fColorStages[idx]; }
    const GrFragmentStage& getCoverageStage(int idx) const { return fCoverageStages[idx]; }

    /**
     * Checks whether any of the effects will read the dst pixel color.
     * TODO remove when we have an XP
     */
    bool willEffectReadDstColor(const GrPrimitiveProcessor*) const;

    /**
     * The xfer processor factory.
     */
    const GrXPFactory* setXPFactory(const GrXPFactory* xpFactory) {
        fXPFactory.reset(SkRef(xpFactory));
        return xpFactory;
    }

    void setPorterDuffXPFactory(SkXfermode::Mode mode) {
        fXPFactory.reset(GrPorterDuffXPFactory::Create(mode));
    }

    void setPorterDuffXPFactory(GrBlendCoeff src, GrBlendCoeff dst) {
        fXPFactory.reset(GrPorterDuffXPFactory::Create(src, dst));
    }

    void setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage = false) {
        fXPFactory.reset(GrCoverageSetOpXPFactory::Create(regionOp, invertCoverage));
    }

    void setDisableColorXPFactory() {
        fXPFactory.reset(GrDisableColorXPFactory::Create());
    }

    const GrFragmentProcessor* addColorProcessor(const GrFragmentProcessor* effect) {
        SkASSERT(effect);
        SkNEW_APPEND_TO_TARRAY(&fColorStages, GrFragmentStage, (effect));
        fColorProcInfoValid = false;
        return effect;
    }

    const GrFragmentProcessor* addCoverageProcessor(const GrFragmentProcessor* effect) {
        SkASSERT(effect);
        SkNEW_APPEND_TO_TARRAY(&fCoverageStages, GrFragmentStage, (effect));
        fCoverageProcInfoValid = false;
        return effect;
    }

    /**
     * Creates a GrSimpleTextureEffect that uses local coords as texture coordinates.
     */
    void addColorTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
        this->addColorProcessor(GrSimpleTextureEffect::Create(texture, matrix))->unref();
    }

    void addCoverageTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
        this->addCoverageProcessor(GrSimpleTextureEffect::Create(texture, matrix))->unref();
    }

    void addColorTextureProcessor(GrTexture* texture,
                                  const SkMatrix& matrix,
                                  const GrTextureParams& params) {
        this->addColorProcessor(GrSimpleTextureEffect::Create(texture, matrix, params))->unref();
    }

    void addCoverageTextureProcessor(GrTexture* texture,
                                     const SkMatrix& matrix,
                                     const GrTextureParams& params) {
        this->addCoverageProcessor(GrSimpleTextureEffect::Create(texture, matrix, params))->unref();
    }

    /**
     * When this object is destroyed it will remove any color/coverage effects from the draw state
     * that were added after its constructor.
     *
     * This class has strange behavior around geometry processor. If there is a GP on the draw state
     * it will assert that the GP is not modified until after the destructor of the ARE. If the
     * draw state has a NULL GP when the ARE is constructed then it will reset it to null in the
     * destructor.
     *
     * TODO: We'd prefer for the ARE to just save and restore the GP. However, this would add
     * significant complexity to the multi-ref architecture for deferred drawing. Once GrDrawState
     * and GrOptDrawState are fully separated then GrDrawState will never be in the deferred
     * execution state and GrOptDrawState always will be (and will be immutable and therefore
     * unable to have an ARE). At this point we can restore sanity and have the ARE save and restore
     * the GP.
     */
    class AutoRestoreEffects : public ::SkNoncopyable {
    public:
        AutoRestoreEffects() 
            : fDrawState(NULL)
            , fColorEffectCnt(0)
            , fCoverageEffectCnt(0) {}

        AutoRestoreEffects(GrDrawState* ds)
            : fDrawState(NULL)
            , fColorEffectCnt(0)
            , fCoverageEffectCnt(0) {
            this->set(ds);
        }

        ~AutoRestoreEffects() { this->set(NULL); }

        void set(GrDrawState* ds);

        bool isSet() const { return SkToBool(fDrawState); }

    private:
        GrDrawState*    fDrawState;
        int             fColorEffectCnt;
        int             fCoverageEffectCnt;
    };

    /**
     * AutoRestoreStencil
     *
     * This simple struct saves and restores the stencil settings
     */
    class AutoRestoreStencil : public ::SkNoncopyable {
    public:
        AutoRestoreStencil() : fDrawState(NULL) {}

        AutoRestoreStencil(GrDrawState* ds) : fDrawState(NULL) { this->set(ds); }

        ~AutoRestoreStencil() { this->set(NULL); }

        void set(GrDrawState* ds) {
            if (fDrawState) {
                fDrawState->setStencil(fStencilSettings);
            }
            fDrawState = ds;
            if (ds) {
                fStencilSettings = ds->getStencil();
            }
        }

        bool isSet() const { return SkToBool(fDrawState); }

    private:
        GrDrawState*       fDrawState;
        GrStencilSettings  fStencilSettings;
    };

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Blending
    ////

    /**
     * Determines whether multiplying the computed per-pixel color by the pixel's fractional
     * coverage before the blend will give the correct final destination color. In general it
     * will not as coverage is applied after blending.
     */
    bool canTweakAlphaForCoverage() const;

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name View Matrix
    ////

    /**
     * Retrieves the current view matrix
     * @return the current view matrix.
     */
    const SkMatrix& getViewMatrix() const { return fViewMatrix; }

    /**
     *  Retrieves the inverse of the current view matrix.
     *
     *  If the current view matrix is invertible, return true, and if matrix
     *  is non-null, copy the inverse into it. If the current view matrix is
     *  non-invertible, return false and ignore the matrix parameter.
     *
     * @param matrix if not null, will receive a copy of the current inverse.
     */
    bool getViewInverse(SkMatrix* matrix) const {
        SkMatrix inverse;
        if (fViewMatrix.invert(&inverse)) {
            if (matrix) {
                *matrix = inverse;
            }
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////

    /**
     * Sets the viewmatrix to identity and restores it in the destructor.
     * TODO remove vm off of drawstate
     */
    class AutoViewMatrixRestore : public ::SkNoncopyable {
    public:
        AutoViewMatrixRestore() {
            fDrawState = NULL;
        }

        AutoViewMatrixRestore(GrDrawState* ds) {
            SkASSERT(ds);
            fDrawState = ds;
            fViewMatrix = fDrawState->fViewMatrix;
            fDrawState->fViewMatrix = SkMatrix::I();
        }

        void setIdentity(GrDrawState* ds) {
            SkASSERT(ds);
            fDrawState = ds;
            fViewMatrix = fDrawState->fViewMatrix;
            fDrawState->fViewMatrix = SkMatrix::I();
        }

        ~AutoViewMatrixRestore() {
            if (fDrawState) {
                fDrawState->fViewMatrix = fViewMatrix;
            }
        }

    private:
        GrDrawState*                                           fDrawState;
        SkMatrix                                               fViewMatrix;
    };


    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Render Target
    ////

    /**
     * Retrieves the currently set render-target.
     *
     * @return    The currently set render target.
     */
    GrRenderTarget* getRenderTarget() const { return fRenderTarget.get(); }

    /**
     * Sets the render-target used at the next drawing call
     *
     * @param target  The render target to set.
     */
    void setRenderTarget(GrRenderTarget* target) { fRenderTarget.reset(SkSafeRef(target)); }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Stencil
    ////

    const GrStencilSettings& getStencil() const { return fStencilSettings; }

    /**
     * Sets the stencil settings to use for the next draw.
     * Changing the clip has the side-effect of possibly zeroing
     * out the client settable stencil bits. So multipass algorithms
     * using stencil should not change the clip between passes.
     * @param settings  the stencil settings to use.
     */
    void setStencil(const GrStencilSettings& settings) { fStencilSettings = settings; }

    /**
     * Shortcut to disable stencil testing and ops.
     */
    void disableStencil() { fStencilSettings.setDisabled(); }

    GrStencilSettings* stencil() { return &fStencilSettings; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name State Flags
    ////

    /**
     *  Flags that affect rendering. Controlled using enable/disableState(). All
     *  default to disabled.
     */
    enum StateBits {
        /**
         * Perform dithering. TODO: Re-evaluate whether we need this bit
         */
        kDither_StateBit        = 0x01,
        /**
         * Perform HW anti-aliasing. This means either HW FSAA, if supported by the render target,
         * or smooth-line rendering if a line primitive is drawn and line smoothing is supported by
         * the 3D API.
         */
        kHWAntialias_StateBit   = 0x02,
        /**
         * Draws will respect the clip, otherwise the clip is ignored.
         */
        kClip_StateBit          = 0x04,

        kLast_StateBit = kClip_StateBit,
    };

    bool isClipState() const { return 0 != (fFlagBits & kClip_StateBit); }
    bool isDither() const { return 0 != (fFlagBits & kDither_StateBit); }
    bool isHWAntialias() const { return 0 != (fFlagBits & kHWAntialias_StateBit); }

    /**
     * Enable render state settings.
     *
     * @param stateBits bitfield of StateBits specifying the states to enable
     */
    void enableState(uint32_t stateBits) { fFlagBits |= stateBits; }

    /**
     * Disable render state settings.
     *
     * @param stateBits bitfield of StateBits specifying the states to disable
     */
    void disableState(uint32_t stateBits) { fFlagBits &= ~(stateBits); }

    /**
     * Enable or disable stateBits based on a boolean.
     *
     * @param stateBits bitfield of StateBits to enable or disable
     * @param enable    if true enable stateBits, otherwise disable
     */
    void setState(uint32_t stateBits, bool enable) {
        if (enable) {
            this->enableState(stateBits);
        } else {
            this->disableState(stateBits);
        }
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Face Culling
    ////

    enum DrawFace {
        kInvalid_DrawFace = -1,

        kBoth_DrawFace,
        kCCW_DrawFace,
        kCW_DrawFace,
    };

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    DrawFace getDrawFace() const { return fDrawFace; }

    /**
     * Controls whether clockwise, counterclockwise, or both faces are drawn.
     * @param face  the face(s) to draw.
     */
    void setDrawFace(DrawFace face) {
        SkASSERT(kInvalid_DrawFace != face);
        fDrawFace = face;
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////

    GrDrawState& operator= (const GrDrawState& that);

private:
    bool isEqual(const GrDrawState& that, bool explicitLocalCoords) const;

    const GrProcOptInfo& colorProcInfo(const GrPrimitiveProcessor* pp) const {
        this->calcColorInvariantOutput(pp);
        return fColorProcInfo;
    }

    const GrProcOptInfo& coverageProcInfo(const GrPrimitiveProcessor* pp) const {
        this->calcCoverageInvariantOutput(pp);
        return fCoverageProcInfo;
    }

    /**
     * If fColorProcInfoValid is false, function calculates the invariant output for the color
     * stages and results are stored in fColorProcInfo.
     */
    void calcColorInvariantOutput(const GrPrimitiveProcessor*) const;

    /**
     * If fCoverageProcInfoValid is false, function calculates the invariant output for the coverage
     * stages and results are stored in fCoverageProcInfo.
     */
    void calcCoverageInvariantOutput(const GrPrimitiveProcessor*) const;

    /**
     * If fColorProcInfoValid is false, function calculates the invariant output for the color
     * stages and results are stored in fColorProcInfo.
     */
    void calcColorInvariantOutput(GrColor) const;

    /**
     * If fCoverageProcInfoValid is false, function calculates the invariant output for the coverage
     * stages and results are stored in fCoverageProcInfo.
     */
    void calcCoverageInvariantOutput(GrColor) const;

    void onReset(const SkMatrix* initialViewMatrix);

    // Some of the auto restore objects assume that no effects are removed during their lifetime.
    // This is used to assert that this condition holds.
    SkDEBUGCODE(int fBlockEffectRemovalCnt;)

    typedef SkSTArray<4, GrFragmentStage> FragmentStageArray;

    SkAutoTUnref<GrRenderTarget>            fRenderTarget;
    SkMatrix                                fViewMatrix;
    uint32_t                                fFlagBits;
    GrStencilSettings                       fStencilSettings;
    DrawFace                                fDrawFace;
    SkAutoTUnref<const GrXPFactory>         fXPFactory;
    FragmentStageArray                      fColorStages;
    FragmentStageArray                      fCoverageStages;

    mutable GrProcOptInfo fColorProcInfo;
    mutable GrProcOptInfo fCoverageProcInfo;
    mutable bool fColorProcInfoValid;
    mutable bool fCoverageProcInfoValid;
    mutable GrColor fColorCache;
    mutable GrColor fCoverageCache;
    mutable const GrPrimitiveProcessor* fColorPrimProc;
    mutable const GrPrimitiveProcessor* fCoveragePrimProc;

    friend class GrOptDrawState;
};

#endif
