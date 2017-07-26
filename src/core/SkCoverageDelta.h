/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCoverageDelta_DEFINED
#define SkCoverageDelta_DEFINED

#include "SkArenaAlloc.h"
#include "SkFixed.h"
#include "SkMask.h"
#include "SkTSort.h"
#include "SkUtils.h"

// Future todo: maybe we can make fX and fDelta 16-bit long to speed it up a little bit.
struct SkCoverageDelta {
    int     fX;     // the y coordinate will be implied in SkCoverageDeltaList
    SkFixed fDelta; // the amount that the alpha changed

    // Sort according to fX
    bool operator<(const SkCoverageDelta& other) const {
        return fX < other.fX;
    }
};

// All the arguments needed for SkBlitter::blitAntiRect
struct SkAntiRect {
    int     fX;
    int     fY;
    int     fWidth;
    int     fHeight;
    SkAlpha fLeftAlpha;
    SkAlpha fRightAlpha;
};

using SkCoverageDeltaAllocator = SkSTArenaAlloc<256>;

// A list of SkCoverageDelta with y from top() to bottom().
// For each row y, there are count(y) number of deltas.
// You can ask whether they are sorted or not by sorted(y), and you can sort them by sort(y).
// Once sorted, getDelta(y, i) should return the i-th leftmost delta on row y.
class SkCoverageDeltaList {
public:
    // We can store INIT_ROW_SIZE deltas per row (i.e., per y-scanline) initially
#ifndef GOOGLE3
    static constexpr int INIT_ROW_SIZE = 32;
#else
    static constexpr int INIT_ROW_SIZE = 8; // google3 has 16k stack limit
#endif
    static constexpr int RESERVED_HEIGHT = 128; // reserve this many rows on stack memory

    SkCoverageDeltaList(SkCoverageDeltaAllocator* alloc, int top, int bottom, bool forceRLE);

    inline int  top() const { return fTop; }
    inline int  bottom() const { return fBottom; }
    inline bool forceRLE() const { return fForceRLE; }
    inline int  count(int y) const { this->checkY(y); return fCounts[y]; }
    inline bool sorted(int y) const { this->checkY(y); return fSorted[y]; }
    inline void addDelta(int x, int y, SkFixed delta) { this->push_back(y, {x, delta}); }

    inline const SkCoverageDelta& getDelta(int y, int i) const {
        this->checkY(y);
        SkASSERT(i < fCounts[y]);
        return fRows[y][i];
    }

    // It might be better to sort right before blitting to make the memory hot
    inline void sort(int y) {
        this->checkY(y);
        if (!fSorted[y]) {
            SkTQSort(fRows[y], fRows[y] + fCounts[y] - 1);
            fSorted[y] = true;
        }
    }

    inline const SkAntiRect& getAntiRect() const { return fAntiRect; }
    inline void setAntiRect(int x, int y, int width, int height,
            SkAlpha leftAlpha, SkAlpha rightAlpha) {
        fAntiRect = {x, y, width, height, leftAlpha, rightAlpha};
    }

    inline void push_back(int y, const SkCoverageDelta& delta) {
        this->checkY(y);
        if (fCounts[y] == fMaxCounts[y]) {
            fMaxCounts[y] *= 2;
            SkCoverageDelta* newRow = fAlloc->makeArrayDefault<SkCoverageDelta>(fMaxCounts[y]);
            memcpy(newRow, fRows[y], sizeof(SkCoverageDelta) * fCounts[y]);
            fRows[y] = newRow;
        }
        SkASSERT(fCounts[y] < fMaxCounts[y]);
        fRows[y][fCounts[y]++] = delta;
        fSorted[y] = fSorted[y] && (fCounts[y] == 1 || delta.fX >= fRows[y][fCounts[y] - 2].fX);
    }

private:
    SkCoverageDeltaAllocator*   fAlloc;
    SkCoverageDelta**           fRows;
    bool*                       fSorted;
    int*                        fCounts;
    int*                        fMaxCounts;
    int                         fTop;
    int                         fBottom;
    SkAntiRect                  fAntiRect;
    bool                        fForceRLE;

    SkCoverageDelta             fReservedStorage[RESERVED_HEIGHT * INIT_ROW_SIZE];
    SkCoverageDelta*            fReservedRows[RESERVED_HEIGHT];
    bool                        fReservedSorted[RESERVED_HEIGHT];
    int                         fReservedCounts[RESERVED_HEIGHT];
    int                         fReservedMaxCounts[RESERVED_HEIGHT];

    inline void checkY(int y) const { SkASSERT(y >= fTop && y < fBottom); }
};

class SkCoverageDeltaMask {
public:
    // 1 for precision error, 1 for boundary delta (e.g., -SK_Fixed1 at fBounds.fRight + 1)
    static constexpr int PADDING        = 2;

    static constexpr int SIMD_WIDTH     = 8;
    static constexpr int SUITABLE_WIDTH = 32;
    static constexpr int MAX_MASK_SIZE  = 2048;

    // Expand PADDING on both sides, and make it a multiple of SIMD_WIDTH
    static int  ExpandWidth(int width);
    static bool CanHandle(const SkIRect& bounds);   // whether bounds fits into MAX_MASK_SIZE
    static bool Suitable(const SkIRect& bounds);    // CanHandle(bounds) && width <= SUITABLE_WIDTH

    SkCoverageDeltaMask(const SkIRect& bounds);

    inline int              top()       const { return fBounds.fTop; }
    inline int              bottom()    const { return fBounds.fBottom; }
    inline SkAlpha*         getMask()         { return fMask; }
    inline const SkIRect&   getBounds() const { return fBounds; }

    inline void             addDelta (int x, int y, SkFixed delta) { this->delta(x, y) += delta; }
    inline SkFixed&         delta    (int x, int y) {
        this->checkX(x);
        this->checkY(y);
        return fDeltas[this->index(x, y)];
    }

    inline void setAntiRect(int x, int y, int width, int height,
                            SkAlpha leftAlpha, SkAlpha rightAlpha) {
        fAntiRect = {x, y, width, height, leftAlpha, rightAlpha};
    }

    inline SkMask prepareSkMask() {
        SkMask mask;
        mask.fImage     = fMask;
        mask.fBounds    = fBounds;
        mask.fRowBytes  = fBounds.width();
        mask.fFormat    = SkMask::kA8_Format;
        return mask;
    }

    void convertCoverageToAlpha(bool isEvenOdd, bool isInverse, bool isConvex);

private:
    SkIRect     fBounds;
    SkFixed     fDeltaStorage[MAX_MASK_SIZE];
    SkFixed*    fDeltas;
    SkAlpha     fMask[MAX_MASK_SIZE];
    int         fExpandedWidth;
    SkAntiRect  fAntiRect;

    inline int  index(int x, int y) const { return y * fExpandedWidth + x; }
    inline void checkY(int y) const { SkASSERT(y >= fBounds.fTop && y < fBounds.fBottom); }
    inline void checkX(int x) const {
        SkASSERT(x >= fBounds.fLeft - PADDING && x < fBounds.fRight + PADDING);
    }
};

static SK_ALWAYS_INLINE SkAlpha CoverageToAlpha(SkFixed coverage, bool isEvenOdd, bool isInverse) {
    SkAlpha result;
    if (isEvenOdd) {
        SkFixed mod17 = coverage & 0x1ffff;
        SkFixed mod16 = coverage & 0xffff;
        result = SkTPin(SkAbs32((mod16 << 1) - mod17) >> 8, 0, 255);
    } else {
        result = SkTPin(SkAbs32(coverage) >> 8, 0, 255);
    }
    return isInverse ? 255 - result : result;
}

template<typename T>
static SK_ALWAYS_INLINE T CoverageToAlpha(T coverage, bool isEvenOdd, bool isInverse) {
    T t0(0), t255(255);
    T result;
    if (isEvenOdd) {
        T mod17 = coverage & 0x1ffff;
        T mod16 = coverage & 0xffff;
        result = ((mod16 << 1) - mod17).abs() >> 8;
    } else {
        result = coverage.abs() >> 8;;
    }
    result = T::Min(result, t255);
    result = T::Max(result, t0);
    return isInverse ? 255 - result : result;
}

// For convex paths (including inverse mode), the coverage is guaranteed to be
// between [-SK_Fixed1, SK_Fixed1] so we can skip isEvenOdd and SkTPin.
static SK_ALWAYS_INLINE SkAlpha ConvexCoverageToAlpha(SkFixed coverage, bool isInverse) {
    SkASSERT(coverage >= -SK_Fixed1 && coverage <= SK_Fixed1);
    int result = SkAbs32(coverage) >> 8;
    result -= (result >> 8); // 256 to 255
    return isInverse ? 255 - result : result;
}

template<typename T>
static SK_ALWAYS_INLINE T ConvexCoverageToAlpha(T coverage, bool isInverse) {
    // allTrue is not implemented
    // SkASSERT((coverage >= 0).allTrue() && (coverage <= SK_Fixed1).allTrue());
    T result = coverage.abs() >> 8;
    result -= (result >> 8); // 256 to 255
    return isInverse ? 255 - result : result;
}

#endif
