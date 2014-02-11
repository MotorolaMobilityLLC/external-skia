/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMatrixClipStateMgr_DEFINED
#define SkMatrixClipStateMgr_DEFINED

#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkRegion.h"
#include "SkRRect.h"
#include "SkTypes.h"
#include "SkTArray.h"

class SkPictureRecord;
class SkWriter32;

// The SkMatrixClipStateMgr collapses the matrix/clip state of an SkPicture into
// a series of save/restore blocks of consistent matrix clip state, e.g.:
//
//   save
//     clip(s)
//     concat
//     ... draw ops ...
//   restore
//
// SaveLayers simply add another level, e.g.:
//
//   save
//     clip(s)
//     concat
//     ... draw ops ...
//     saveLayer
//       save
//         clip(s)
//         concat
//         ... draw ops ...
//       restore
//     restore
//   restore
//
// As a side effect of this process all saves and saveLayers will become
// kMatrixClip_SaveFlag style saves/saveLayers.

// The SkMatrixClipStateMgr works by intercepting all the save*, restore, clip*,
// and matrix calls sent to SkCanvas in order to track the current matrix/clip
// state. All the other canvas calls get funnelled into a generic "call" entry 
// point that signals that a state block is required.
class SkMatrixClipStateMgr {
public:
    static const int32_t kIdentityWideOpenStateID = 0;

    class MatrixClipState {
    public:
        class MatrixInfo {
        public:
            SkMatrix fMatrix;
            // TODO: add an internal dictionary and an ID here
        };

        class ClipInfo : public SkNoncopyable {
        public:
            ClipInfo() {}

            bool clipRect(const SkRect& rect, 
                          SkRegion::Op op, 
                          bool doAA, 
                          const SkMatrix& matrix) {
                ClipOp& newClip = fClips.push_back();
                newClip.fClipType = kRect_ClipType;
                newClip.fGeom.fRRect.setRect(rect);   // storing the clipRect in the RRect
                newClip.fOp = op;
                newClip.fDoAA = doAA;
                newClip.fMatrix = matrix;
                newClip.fOffset = kInvalidJumpOffset;
                return false;
            }

            bool clipRRect(const SkRRect& rrect, 
                           SkRegion::Op op, 
                           bool doAA, 
                           const SkMatrix& matrix) {
                ClipOp& newClip = fClips.push_back();
                newClip.fClipType = kRRect_ClipType;
                newClip.fGeom.fRRect = rrect;
                newClip.fOp = op;
                newClip.fDoAA = doAA;
                newClip.fMatrix = matrix;
                newClip.fOffset = kInvalidJumpOffset;
                return false;
            }

            bool clipPath(SkPictureRecord* picRecord, 
                          const SkPath& path, 
                          SkRegion::Op op, 
                          bool doAA, 
                          const SkMatrix& matrix);
            bool clipRegion(SkPictureRecord* picRecord, 
                            const SkRegion& region, 
                            SkRegion::Op op, 
                            const SkMatrix& matrix);
            void writeClip(SkMatrix* curMat, 
                           SkPictureRecord* picRecord,
                           bool* overrideFirstOp);
            void fillInSkips(SkWriter32* writer, int32_t restoreOffset);

#ifdef SK_DEBUG
            void checkOffsetNotEqual(int32_t offset) {
                for (int i = 0; i < fClips.count(); ++i) {
                    ClipOp& curClip = fClips[i];
                    SkASSERT(offset != curClip.fOffset);
                }
            }
#endif
        private:
            enum ClipType {
                kRect_ClipType,
                kRRect_ClipType,
                kPath_ClipType,
                kRegion_ClipType
            };

            static const int kInvalidJumpOffset = -1;

            class ClipOp {
            public:
                ClipOp() {}
                ~ClipOp() {
                    if (kRegion_ClipType == fClipType) {
                        SkDELETE(fGeom.fRegion);
                    }
                }

                ClipType     fClipType;

                union {
                    SkRRect         fRRect;        // also stores clipRect
                    int             fPathID;
                    // TODO: add internal dictionary of regions
                    // This parameter forces us to have a dtor and thus use
                    // SkTArray rather then SkTDArray!
                    const SkRegion* fRegion;
                } fGeom;

                bool         fDoAA;
                SkRegion::Op fOp;

                // The CTM in effect when this clip call was issued
                // TODO: add an internal dictionary and replace with ID
                SkMatrix     fMatrix;

                // The offset of this clipOp's "jump-to-offset" location in the skp.
                // -1 means the offset hasn't been written.
                int32_t      fOffset;
            };

            SkTArray<ClipOp> fClips;

            typedef SkNoncopyable INHERITED;
        };

        MatrixClipState(MatrixClipState* prev, int flags) 
#ifdef SK_DEBUG
            : fPrev(prev)
#endif
        {
            if (NULL == prev) {
                fLayerID = 0;

                fMatrixInfoStorage.fMatrix.reset();
                fMatrixInfo = &fMatrixInfoStorage;
                fClipInfo = &fClipInfoStorage;  // ctor handles init of fClipInfoStorage

                // The identity/wide-open-clip state is current by default
                fMCStateID = kIdentityWideOpenStateID;
            } 
            else {
                fLayerID = prev->fLayerID;

                if (flags & SkCanvas::kMatrix_SaveFlag) {
                    fMatrixInfoStorage = *prev->fMatrixInfo;
                    fMatrixInfo = &fMatrixInfoStorage;
                } else {
                    fMatrixInfo = prev->fMatrixInfo;
                }

                if (flags & SkCanvas::kClip_SaveFlag) {
                    // We don't copy the ClipOps of the previous clip states
                    fClipInfo = &fClipInfoStorage;
                } else {
                    fClipInfo = prev->fClipInfo;
                }

                // Initially a new save/saveLayer represents the same MC state
                // as its predecessor.
                fMCStateID = prev->fMCStateID;
            }

            fIsSaveLayer = false;
        }

        MatrixInfo*  fMatrixInfo;
        MatrixInfo   fMatrixInfoStorage;

        ClipInfo*    fClipInfo;
        ClipInfo     fClipInfoStorage;

        // Tracks the current depth of saveLayers to support the isDrawingToLayer call
        int          fLayerID;
        // Does this MC state represent a saveLayer call?
        bool         fIsSaveLayer;

        // The next two fields are only valid when fIsSaveLayer is set.
        int32_t      fSaveLayerBaseStateID;
        bool         fSaveLayerBracketed;

#ifdef SK_DEBUG
        MatrixClipState* fPrev; // debugging aid
#endif

        int32_t     fMCStateID;
    };

    enum CallType {
        kMatrix_CallType,
        kClip_CallType,
        kOther_CallType
    };

    SkMatrixClipStateMgr();

    void init(SkPictureRecord* picRecord) {
        // Note: we're not taking a ref here. It is expected that the SkMatrixClipStateMgr
        // is owned by the SkPictureRecord object
        fPicRecord = picRecord; 
    }

    // TODO: need to override canvas' getSaveCount. Right now we pass the
    // save* and restore calls on to the base SkCanvas in SkPictureRecord but
    // this duplicates effort.
    int getSaveCount() const { return fMatrixClipStack.count(); }

    int save(SkCanvas::SaveFlags flags);

    int saveLayer(const SkRect* bounds, const SkPaint* paint, SkCanvas::SaveFlags flags);

    bool isDrawingToLayer() const {
        return fCurMCState->fLayerID > 0;
    }

    void restore();

    bool translate(SkScalar dx, SkScalar dy) {
        this->call(kMatrix_CallType);
        return fCurMCState->fMatrixInfo->fMatrix.preTranslate(dx, dy);
    }

    bool scale(SkScalar sx, SkScalar sy) {
        this->call(kMatrix_CallType);
        return fCurMCState->fMatrixInfo->fMatrix.preScale(sx, sy);
    }

    bool rotate(SkScalar degrees) {
        this->call(kMatrix_CallType);
        return fCurMCState->fMatrixInfo->fMatrix.preRotate(degrees);
    }

    bool skew(SkScalar sx, SkScalar sy) {
        this->call(kMatrix_CallType);
        return fCurMCState->fMatrixInfo->fMatrix.preSkew(sx, sy);
    }

    bool concat(const SkMatrix& matrix) {
        this->call(kMatrix_CallType);
        return fCurMCState->fMatrixInfo->fMatrix.preConcat(matrix);
    }

    void setMatrix(const SkMatrix& matrix) {
        this->call(kMatrix_CallType);
        fCurMCState->fMatrixInfo->fMatrix = matrix;
    }

    bool clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
        this->call(SkMatrixClipStateMgr::kClip_CallType);
        return fCurMCState->fClipInfo->clipRect(rect, op, doAA,
                                                fCurMCState->fMatrixInfo->fMatrix);
    }

    bool clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
        this->call(SkMatrixClipStateMgr::kClip_CallType);
        return fCurMCState->fClipInfo->clipRRect(rrect, op, doAA,
                                                 fCurMCState->fMatrixInfo->fMatrix);
    }

    bool clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
        this->call(SkMatrixClipStateMgr::kClip_CallType);
        return fCurMCState->fClipInfo->clipPath(fPicRecord, path, op, doAA,
                                                fCurMCState->fMatrixInfo->fMatrix);
    }

    bool clipRegion(const SkRegion& region, SkRegion::Op op) {
        this->call(SkMatrixClipStateMgr::kClip_CallType);
        return fCurMCState->fClipInfo->clipRegion(fPicRecord, region, op,
                                                  fCurMCState->fMatrixInfo->fMatrix);
    }

    bool call(CallType callType);

    void fillInSkips(SkWriter32* writer, int32_t restoreOffset) {
        // Since we write out the entire clip stack at each block start we
        // need to update the skips for the entire stack each time too.
        SkDeque::F2BIter iter(fMatrixClipStack);

        for (const MatrixClipState* state = (const MatrixClipState*) iter.next();
             state != NULL;
             state = (const MatrixClipState*) iter.next()) {
            state->fClipInfo->fillInSkips(writer, restoreOffset);
        }
    }

    void finish();

protected:
    SkPictureRecord* fPicRecord;

    uint32_t         fMatrixClipStackStorage[43]; // sized to fit 2 clip states
    SkDeque          fMatrixClipStack;
    MatrixClipState* fCurMCState;

    // The MCStateID of the state currently in effect in the byte stream. 0 if none.
    int32_t          fCurOpenStateID;

    SkDEBUGCODE(void validate();)

    static void WriteDeltaMat(SkPictureRecord* picRecord,
                              const SkMatrix& current, 
                              const SkMatrix& desired);
    static int32_t   NewMCStateID();
};

#endif
