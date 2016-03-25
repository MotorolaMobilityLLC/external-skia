/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyph_DEFINED
#define SkGlyph_DEFINED

#include "SkChecksum.h"
#include "SkTypes.h"
#include "SkFixed.h"
#include "SkMask.h"

class SkPath;
class SkGlyphCache;

// needs to be != to any valid SkMask::Format
#define MASK_FORMAT_UNKNOWN         (0xFF)
#define MASK_FORMAT_JUST_ADVANCE    MASK_FORMAT_UNKNOWN

#define kMaxGlyphWidth (1<<13)

SK_BEGIN_REQUIRE_DENSE
class SkGlyph {
    enum {
        kSubBits = 2,
        kSubMask = ((1 << kSubBits) - 1),
        kSubShift = 24, // must be large enough for glyphs and unichars
        kCodeMask = ((1 << kSubShift) - 1),
        // relative offsets for X and Y subpixel bits
        kSubShiftX = kSubBits,
        kSubShiftY = 0
    };

    // Support horizontal and vertical skipping strike-through / underlines.
    // The caller walks the linked list looking for a match. For a horizontal underline,
    // the fBounds contains the top and bottom of the underline. The fInterval pair contains the
    // beginning and end of of the intersection of the bounds and the glyph's path.
    // If interval[0] >= interval[1], no intesection was found.
    struct Intercept {
        Intercept* fNext;
        SkScalar   fBounds[2];    // for horz underlines, the boundaries in Y
        SkScalar   fInterval[2];  // the outside intersections of the axis and the glyph
    };

    struct PathData {
        Intercept* fIntercept;
        SkPath*    fPath;
    };

public:
    static const SkFixed kSubpixelRound = SK_FixedHalf >> SkGlyph::kSubBits;
    // A value that can never be generated by MakeID.
    static const uint32_t kImpossibleID = ~0;
    void*       fImage;
    PathData*   fPathData;
    float       fAdvanceX, fAdvanceY;

    uint16_t    fWidth, fHeight;
    int16_t     fTop, fLeft;

    uint8_t     fMaskFormat;
    int8_t      fRsbDelta, fLsbDelta;  // used by auto-kerning
    int8_t      fForceBW;

    void initWithGlyphID(uint32_t glyph_id) {
        this->initCommon(MakeID(glyph_id));
    }

    void initGlyphIdFrom(const SkGlyph& glyph) {
        this->initCommon(glyph.fID);
    }

    void initGlyphFromCombinedID(uint32_t combined_id) {
      this->initCommon(combined_id);
    }

    /**
     *  Compute the rowbytes for the specified width and mask-format.
     */
    static unsigned ComputeRowBytes(unsigned width, SkMask::Format format) {
        unsigned rb = width;
        if (SkMask::kBW_Format == format) {
            rb = (rb + 7) >> 3;
        } else if (SkMask::kARGB32_Format == format) {
            rb <<= 2;
        } else if (SkMask::kLCD16_Format == format) {
            rb = SkAlign4(rb << 1);
        } else {
            rb = SkAlign4(rb);
        }
        return rb;
    }

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // Temporary accessor for Android.
    SkFixed advanceXFixed() const { return fAdvanceX; }
    // Temporary accessor for Android.
    SkFixed advanceYFixed() const { return fAdvanceY; }
#endif

    unsigned rowBytes() const {
        return ComputeRowBytes(fWidth, (SkMask::Format)fMaskFormat);
    }

    bool isJustAdvance() const {
        return MASK_FORMAT_JUST_ADVANCE == fMaskFormat;
    }

    bool isFullMetrics() const {
        return MASK_FORMAT_JUST_ADVANCE != fMaskFormat;
    }

    uint16_t getGlyphID() const {
        return ID2Code(fID);
    }

    unsigned getSubX() const {
        return ID2SubX(fID);
    }

    SkFixed getSubXFixed() const {
        return SubToFixed(ID2SubX(fID));
    }

    SkFixed getSubYFixed() const {
        return SubToFixed(ID2SubY(fID));
    }

    size_t computeImageSize() const;

    /** Call this to set all of the metrics fields to 0 (e.g. if the scaler
        encounters an error measuring a glyph). Note: this does not alter the
        fImage, fPath, fID, fMaskFormat fields.
     */
    void zeroMetrics();

    void toMask(SkMask* mask) const;

    class HashTraits {
    public:
        static uint32_t GetKey(const SkGlyph& glyph) {
            return glyph.fID;
        }
        static uint32_t Hash(uint32_t glyphId) {
            return SkChecksum::CheapMix(glyphId);
        }
    };

 private:
    // TODO(herb) remove friend statement after SkGlyphCache cleanup.
    friend class SkGlyphCache;

    void initCommon(uint32_t id) {
        fID             = id;
        fImage          = nullptr;
        fPathData       = nullptr;
        fMaskFormat     = MASK_FORMAT_UNKNOWN;
        fForceBW        = 0;
    }

    static unsigned ID2Code(uint32_t id) {
        return id & kCodeMask;
    }

    static unsigned ID2SubX(uint32_t id) {
        return id >> (kSubShift + kSubShiftX);
    }

    static unsigned ID2SubY(uint32_t id) {
        return (id >> (kSubShift + kSubShiftY)) & kSubMask;
    }

    static unsigned FixedToSub(SkFixed n) {
        return (n >> (16 - kSubBits)) & kSubMask;
    }

    static SkFixed SubToFixed(unsigned sub) {
        SkASSERT(sub <= kSubMask);
        return sub << (16 - kSubBits);
    }

    static uint32_t MakeID(unsigned code) {
        SkASSERT(code <= kCodeMask);
        SkASSERT(code != kImpossibleID);
        return code;
    }

    static uint32_t MakeID(unsigned code, SkFixed x, SkFixed y) {
        SkASSERT(code <= kCodeMask);
        x = FixedToSub(x);
        y = FixedToSub(y);
        uint32_t ID = (x << (kSubShift + kSubShiftX)) |
                      (y << (kSubShift + kSubShiftY)) |
                      code;
        SkASSERT(ID != kImpossibleID);
        return ID;
    }

    // FIXME - This is needed because the Android frame work directly
  // accesses fID. Remove when fID accesses are cleaned up.
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
  public:
#endif
    uint32_t    fID;
};
SK_END_REQUIRE_DENSE

#endif
