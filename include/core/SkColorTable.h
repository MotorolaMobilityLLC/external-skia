
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkColorTable_DEFINED
#define SkColorTable_DEFINED

#include "SkColor.h"
#include "SkFlattenable.h"
#include "SkImageInfo.h"

/** \class SkColorTable

    SkColorTable holds an array SkPMColors (premultiplied 32-bit colors) used by
    8-bit bitmaps, where the bitmap bytes are interpreted as indices into the colortable.
*/
class SK_API SkColorTable : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkColorTable)

    /** Makes a deep copy of colors.
     */
    SkColorTable(const SkColorTable& src);
    SkColorTable(const SkPMColor colors[], int count);
    virtual ~SkColorTable();

    /** Returns the number of colors in the table.
    */
    int count() const { return fCount; }

    /** Returns the specified color from the table. In the debug build, this asserts that
        the index is in range (0 <= index < count).
    */
    SkPMColor operator[](int index) const {
        SkASSERT(fColors != NULL && (unsigned)index < (unsigned)fCount);
        return fColors[index];
    }

    /**
     *  Return the array of colors for reading. This must be balanced by a call
     *  to unlockColors().
     */
    const SkPMColor* lockColors() {
        SkDEBUGCODE(sk_atomic_inc(&fColorLockCount);)
        return fColors;
    }

    /**
     *  Balancing call to lockColors().
     */
    void unlockColors();

    /** Similar to lockColors(), lock16BitCache() returns the array of
        RGB16 colors that mirror the 32bit colors. However, this function
        will return null if kColorsAreOpaque_Flag is not set.
        Also, unlike lockColors(), the returned array here cannot be modified.
    */
    const uint16_t* lock16BitCache();
    /** Balancing call to lock16BitCache().
    */
    void unlock16BitCache() {
        SkASSERT(f16BitCacheLockCount > 0);
        SkDEBUGCODE(sk_atomic_dec(&f16BitCacheLockCount);)
    }

    explicit SkColorTable(SkReadBuffer&);
    void writeToBuffer(SkWriteBuffer&) const;

private:
    SkPMColor*  fColors;
    uint16_t*   f16BitCache;
    int         fCount;
    SkDEBUGCODE(int fColorLockCount;)
    SkDEBUGCODE(int f16BitCacheLockCount;)

    void init(const SkPMColor* colors, int count);

    void inval16BitCache();

    typedef SkRefCnt INHERITED;
};

#endif
