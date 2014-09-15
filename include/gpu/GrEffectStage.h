
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrEffectStage_DEFINED
#define GrEffectStage_DEFINED

#include "GrBackendEffectFactory.h"
#include "GrEffect.h"
#include "GrProgramElementRef.h"
#include "SkMatrix.h"
#include "SkShader.h"

// TODO: Make two variations on this class: One for GrDrawState that only owns regular refs
// and supports compatibility checks and changing local coords. The second is for GrOptDrawState,
// is immutable, and only owns pending execution refs. This requries removing the common base
// class from GrDrawState and GrOptDrawState called GrRODrawState and converting to GrOptDrawState
// when draws are enqueued in the GrInOrderDrawBuffer.
class GrEffectStage {
public:
    explicit GrEffectStage(const GrEffect* effect)
    : fEffect(SkRef(effect)) {
        fCoordChangeMatrixSet = false;
    }

    GrEffectStage(const GrEffectStage& other) {
        fCoordChangeMatrixSet = other.fCoordChangeMatrixSet;
        if (other.fCoordChangeMatrixSet) {
            fCoordChangeMatrix = other.fCoordChangeMatrix;
        }
        fEffect.initAndRef(other.fEffect);
    }
    
    static bool AreCompatible(const GrEffectStage& a, const GrEffectStage& b,
                              bool usingExplicitLocalCoords) {
        SkASSERT(a.fEffect.get());
        SkASSERT(b.fEffect.get());

        if (!a.getEffect()->isEqual(*b.getEffect())) {
            return false;
        }

        // We always track the coord change matrix, but it has no effect when explicit local coords
        // are used.
        if (usingExplicitLocalCoords) {
            return true;
        }

        if (a.fCoordChangeMatrixSet != b.fCoordChangeMatrixSet) {
            return false;
        }

        if (!a.fCoordChangeMatrixSet) {
            return true;
        }

        return a.fCoordChangeMatrix == b.fCoordChangeMatrix;
    }

    /**
     * This is called when the coordinate system in which the geometry is specified will change.
     *
     * @param matrix    The transformation from the old coord system in which geometry is specified
     *                  to the new one from which it will actually be drawn.
     */
    void localCoordChange(const SkMatrix& matrix) {
        if (fCoordChangeMatrixSet) {
            fCoordChangeMatrix.preConcat(matrix);
        } else {
            fCoordChangeMatrixSet = true;
            fCoordChangeMatrix = matrix;
        }
    }

    class SavedCoordChange {
    public:
        SkDEBUGCODE(SavedCoordChange() : fEffectUniqueID(SK_InvalidUniqueID) {})
    private:
        bool fCoordChangeMatrixSet;
        SkMatrix fCoordChangeMatrix;
        SkDEBUGCODE(mutable uint32_t fEffectUniqueID;)

        friend class GrEffectStage;
    };

    /**
     * This gets the current coordinate system change. It is the accumulation of
     * localCoordChange calls since the effect was installed. It is used when then caller
     * wants to temporarily change the source geometry coord system, draw something, and then
     * restore the previous coord system (e.g. temporarily draw in device coords).
     */
    void saveCoordChange(SavedCoordChange* savedCoordChange) const {
        savedCoordChange->fCoordChangeMatrixSet = fCoordChangeMatrixSet;
        if (fCoordChangeMatrixSet) {
            savedCoordChange->fCoordChangeMatrix = fCoordChangeMatrix;
        }
        SkASSERT(SK_InvalidUniqueID == savedCoordChange->fEffectUniqueID);
        SkDEBUGCODE(savedCoordChange->fEffectUniqueID = fEffect->getUniqueID();)
    }

    /**
     * This balances the saveCoordChange call.
     */
    void restoreCoordChange(const SavedCoordChange& savedCoordChange) {
        fCoordChangeMatrixSet = savedCoordChange.fCoordChangeMatrixSet;
        if (fCoordChangeMatrixSet) {
            fCoordChangeMatrix = savedCoordChange.fCoordChangeMatrix;
        }
        SkASSERT(savedCoordChange.fEffectUniqueID == fEffect->getUniqueID());
        SkDEBUGCODE(savedCoordChange.fEffectUniqueID = SK_InvalidUniqueID);
    }

    /**
     * Gets the matrix representing all changes of coordinate system since the GrEffect was
     * installed in the stage.
     */
    const SkMatrix& getCoordChangeMatrix() const {
        if (fCoordChangeMatrixSet) {
            return fCoordChangeMatrix;
        } else {
            return SkMatrix::I();
        }
    }

    const GrEffect* getEffect() const { return fEffect.get(); }

    void convertToPendingExec() { fEffect.convertToPendingExec(); }

private:
    bool                                fCoordChangeMatrixSet;
    SkMatrix                            fCoordChangeMatrix;
    GrProgramElementRef<const GrEffect> fEffect;
};

#endif
