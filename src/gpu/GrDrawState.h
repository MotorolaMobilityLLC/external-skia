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
#include "SkMatrix.h"
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

    ///////////////////////////////////////////////////////////////////////////
    /// @name Vertex Attributes
    ////

    // TODO when we move this info off of GrGeometryProcessor, delete these
    bool hasLocalCoordAttribute() const {
        return this->hasGeometryProcessor() && this->getGeometryProcessor()->hasLocalCoords();
    }
    bool hasColorVertexAttribute() const {
        return this->hasGeometryProcessor() && this->getGeometryProcessor()->hasVertexColor();
    }
    bool hasCoverageVertexAttribute() const {
        return this->hasGeometryProcessor() && this->getGeometryProcessor()->hasVertexCoverage();
    }

    /// @}

    /**
     * Depending on features available in the underlying 3D API and the color blend mode requested
     * it may or may not be possible to correctly blend with fractional pixel coverage generated by
     * the fragment shader.
     *
     * This function considers the current draw state and the draw target's capabilities to
     * determine whether coverage can be handled correctly. This function assumes that the caller
     * intends to specify fractional pixel coverage (via setCoverage(), through a coverage vertex
     * attribute, or a coverage effect) but may not have specified it yet.
     */
    bool couldApplyCoverage(const GrDrawTargetCaps& caps) const;

    /**
     * Determines whether the output coverage is guaranteed to be one for all pixels hit by a draw.
     */
    bool hasSolidCoverage() const;

    /**
     * This function returns true if the render target destination pixel values will be read for
     * blending during draw.
     */
    bool willBlendWithDst() const;

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Color
    ////

    GrColor getColor() const { return fColor; }

    /**
     *  Sets color for next draw to a premultiplied-alpha color.
     *
     *  @param color    the color to set.
     */
    void setColor(GrColor color) {
        if (color != fColor) {
            fColor = color;
            fColorProcInfoValid = false;
        }
    }

    /**
     *  Sets the color to be used for the next draw to be
     *  (r,g,b,a) = (alpha, alpha, alpha, alpha).
     *
     *  @param alpha The alpha value to set as the color.
     */
    void setAlpha(uint8_t a) { this->setColor((a << 24) | (a << 16) | (a << 8) | a); }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Coverage
    ////

    uint8_t getCoverage() const { return fCoverage; }

    GrColor getCoverageColor() const {
        return GrColorPackRGBA(fCoverage, fCoverage, fCoverage, fCoverage);
    }

    /**
     * Sets a constant fractional coverage to be applied to the draw. The
     * initial value (after construction or reset()) is 0xff. The constant
     * coverage is ignored when per-vertex coverage is provided.
     */
    void setCoverage(uint8_t coverage) {
        if (coverage != fCoverage) {
            fCoverage = coverage;
            fCoverageProcInfoValid = false;
        }
    }

    /// @}

    /**
     * The geometry processor is the sole element of the skia pipeline which can use the vertex,
     * geometry, and tesselation shaders.  The GP may also compute a coverage in its fragment shader
     * but is never put in the color processing pipeline.
     */

    const GrGeometryProcessor* setGeometryProcessor(const GrGeometryProcessor* geometryProcessor) {
        SkASSERT(geometryProcessor);
        SkASSERT(!this->hasGeometryProcessor());
        fGeometryProcessor.reset(SkRef(geometryProcessor));
        fCoverageProcInfoValid = false;
        return geometryProcessor;
    }

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
    int numTotalStages() const {
         return this->numFragmentStages() + (this->hasGeometryProcessor() ? 1 : 0);
    }

    bool hasGeometryProcessor() const { return SkToBool(fGeometryProcessor.get()); }
    const GrGeometryProcessor* getGeometryProcessor() const { return fGeometryProcessor.get(); }

    const GrXPFactory* getXPFactory() const { return fXPFactory.get(); }

    const GrFragmentStage& getColorStage(int idx) const { return fColorStages[idx]; }
    const GrFragmentStage& getCoverageStage(int idx) const { return fCoverageStages[idx]; }

    /**
     * Checks whether any of the effects will read the dst pixel color.
     */
    bool willEffectReadDstColor() const;

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
            , fOriginalGPID(SK_InvalidUniqueID)
            , fColorEffectCnt(0)
            , fCoverageEffectCnt(0) {}

        AutoRestoreEffects(GrDrawState* ds)
            : fDrawState(NULL)
            , fOriginalGPID(SK_InvalidUniqueID)
            , fColorEffectCnt(0)
            , fCoverageEffectCnt(0) {
            this->set(ds);
        }

        ~AutoRestoreEffects() { this->set(NULL); }

        void set(GrDrawState* ds);

        bool isSet() const { return SkToBool(fDrawState); }

    private:
        GrDrawState*    fDrawState;
        uint32_t        fOriginalGPID;
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

    GrBlendCoeff getSrcBlendCoeff() const { return fSrcBlend; }
    GrBlendCoeff getDstBlendCoeff() const { return fDstBlend; }

    /**
     * Retrieves the last value set by setBlendConstant()
     * @return the blending constant value
     */
    GrColor getBlendConstant() const { return fBlendConstant; }

    /**
     * Determines whether multiplying the computed per-pixel color by the pixel's fractional
     * coverage before the blend will give the correct final destination color. In general it
     * will not as coverage is applied after blending.
     */
    bool canTweakAlphaForCoverage() const;

    /**
     * Sets the blending function coefficients.
     *
     * The blend function will be:
     *    D' = sat(S*srcCoef + D*dstCoef)
     *
     *   where D is the existing destination color, S is the incoming source
     *   color, and D' is the new destination color that will be written. sat()
     *   is the saturation function.
     *
     * @param srcCoef coefficient applied to the src color.
     * @param dstCoef coefficient applied to the dst color.
     */
    void setBlendFunc(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff) {
        fSrcBlend = srcCoeff;
        fDstBlend = dstCoeff;
    #ifdef SK_DEBUG
        if (GrBlendCoeffRefsDst(dstCoeff)) {
            SkDebugf("Unexpected dst blend coeff. Won't work correctly with coverage stages.\n");
        }
        if (GrBlendCoeffRefsSrc(srcCoeff)) {
            SkDebugf("Unexpected src blend coeff. Won't work correctly with coverage stages.\n");
        }
    #endif
    }

    /**
     * Sets the blending function constant referenced by the following blending
     * coefficients:
     *      kConstC_GrBlendCoeff
     *      kIConstC_GrBlendCoeff
     *      kConstA_GrBlendCoeff
     *      kIConstA_GrBlendCoeff
     *
     * @param constant the constant to set
     */
    void setBlendConstant(GrColor constant) { fBlendConstant = constant; }

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

    /**
     * Sets the view matrix to identity and updates any installed effects to compensate for the
     * coord system change.
     */
    bool setIdentityViewMatrix();

    ////////////////////////////////////////////////////////////////////////////

    /**
     * Preconcats the current view matrix and restores the previous view matrix in the destructor.
     * Effect matrices are automatically adjusted to compensate and adjusted back in the destructor.
     */
    class AutoViewMatrixRestore : public ::SkNoncopyable {
    public:
        AutoViewMatrixRestore() : fDrawState(NULL) {}

        AutoViewMatrixRestore(GrDrawState* ds, const SkMatrix& preconcatMatrix) {
            fDrawState = NULL;
            this->set(ds, preconcatMatrix);
        }

        ~AutoViewMatrixRestore() { this->restore(); }

        /**
         * Can be called prior to destructor to restore the original matrix.
         */
        void restore();

        void set(GrDrawState* drawState, const SkMatrix& preconcatMatrix);

        /** Sets the draw state's matrix to identity. This can fail because the current view matrix
            is not invertible. */
        bool setIdentity(GrDrawState* drawState);

    private:
        void doEffectCoordChanges(const SkMatrix& coordChangeMatrix);

        GrDrawState*                                           fDrawState;
        SkMatrix                                               fViewMatrix;
        int                                                    fNumColorStages;
        SkAutoSTArray<8, GrFragmentStage::SavedCoordChange>    fSavedCoordChanges;
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
        /**
         * Disables writing to the color buffer. Useful when performing stencil
         * operations.
         */
        kNoColorWrites_StateBit = 0x08,

        /**
         * Usually coverage is applied after color blending. The color is blended using the coeffs
         * specified by setBlendFunc(). The blended color is then combined with dst using coeffs
         * of src_coverage, 1-src_coverage. Sometimes we are explicitly drawing a coverage mask. In
         * this case there is no distinction between coverage and color and the caller needs direct
         * control over the blend coeffs. When set, there will be a single blend step controlled by
         * setBlendFunc() which will use coverage*color as the src color.
         */
         kCoverageDrawing_StateBit = 0x10,
         kLast_StateBit = kCoverageDrawing_StateBit,
    };

    bool isClipState() const { return 0 != (fFlagBits & kClip_StateBit); }
    bool isColorWriteDisabled() const { return 0 != (fFlagBits & kNoColorWrites_StateBit); }
    bool isCoverageDrawing() const { return 0 != (fFlagBits & kCoverageDrawing_StateBit); }
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
    /// @name Hints
    /// Hints that when provided can enable optimizations.
    ////

    enum Hints {
        kVertexColorsAreOpaque_Hint = 0x1,
        kLast_Hint = kVertexColorsAreOpaque_Hint
    };

    void setHint(Hints hint, bool value) { fHints = value ? (fHints | hint) : (fHints & ~hint); }

    bool vertexColorsAreOpaque() const { return kVertexColorsAreOpaque_Hint & fHints; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////

    GrDrawState& operator= (const GrDrawState& that);

private:
    bool isEqual(const GrDrawState& that) const;

    /**
     * Optimizations for blending / coverage to that can be applied based on the current state.
     */
    enum BlendOpt {
        /**
         * No optimization
         */
        kNone_BlendOpt,
        /**
         * Don't draw at all
         */
        kSkipDraw_BlendOpt,
        /**
         * The coverage value does not have to be computed separately from alpha, the the output
         * color can be the modulation of the two.
         */
        kCoverageAsAlpha_BlendOpt,
        /**
         * Instead of emitting a src color, emit coverage in the alpha channel and r,g,b are
         * "don't cares".
         */
        kEmitCoverage_BlendOpt,
        /**
         * Emit transparent black instead of the src color, no need to compute coverage.
         */
        kEmitTransBlack_BlendOpt
    };

    /**
     * Determines what optimizations can be applied based on the blend. The coefficients may have
     * to be tweaked in order for the optimization to work. srcCoeff and dstCoeff are optional
     * params that receive the tweaked coefficients. Normally the function looks at the current
     * state to see if coverage is enabled. By setting forceCoverage the caller can speculatively
     * determine the blend optimizations that would be used if there was partial pixel coverage.
     *
     * This is used internally and when constructing a GrOptDrawState.
     */
    BlendOpt getBlendOpt(bool forceCoverage = false,
                         GrBlendCoeff* srcCoeff = NULL,
                         GrBlendCoeff* dstCoeff = NULL) const;

    const GrProcOptInfo& colorProcInfo() const { 
        this->calcColorInvariantOutput();
        return fColorProcInfo;
    }

    const GrProcOptInfo& coverageProcInfo() const {
        this->calcCoverageInvariantOutput();
        return fCoverageProcInfo;
    }

    /**
     * Determines whether src alpha is guaranteed to be one for all src pixels
     */
    bool srcAlphaWillBeOne() const;

    /**
     * If fColorProcInfoValid is false, function calculates the invariant output for the color
     * stages and results are stored in fColorProcInfo.
     */
    void calcColorInvariantOutput() const;

    /**
     * If fCoverageProcInfoValid is false, function calculates the invariant output for the coverage
     * stages and results are stored in fCoverageProcInfo.
     */
    void calcCoverageInvariantOutput() const;

    void onReset(const SkMatrix* initialViewMatrix);

    // Some of the auto restore objects assume that no effects are removed during their lifetime.
    // This is used to assert that this condition holds.
    SkDEBUGCODE(int fBlockEffectRemovalCnt;)

    typedef SkSTArray<4, GrFragmentStage> FragmentStageArray;

    SkAutoTUnref<GrRenderTarget>            fRenderTarget;
    GrColor                                 fColor;
    SkMatrix                                fViewMatrix;
    GrColor                                 fBlendConstant;
    uint32_t                                fFlagBits;
    GrStencilSettings                       fStencilSettings;
    uint8_t                                 fCoverage;
    DrawFace                                fDrawFace;
    GrBlendCoeff                            fSrcBlend;
    GrBlendCoeff                            fDstBlend;
    SkAutoTUnref<const GrGeometryProcessor> fGeometryProcessor;
    SkAutoTUnref<const GrXPFactory>         fXPFactory;
    FragmentStageArray                      fColorStages;
    FragmentStageArray                      fCoverageStages;
    uint32_t                                fHints;

    mutable GrProcOptInfo fColorProcInfo;
    mutable GrProcOptInfo fCoverageProcInfo;
    mutable bool fColorProcInfoValid;
    mutable bool fCoverageProcInfoValid;

    friend class GrOptDrawState;
};

#endif
