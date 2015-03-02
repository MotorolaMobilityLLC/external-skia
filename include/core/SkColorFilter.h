
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkColorFilter_DEFINED
#define SkColorFilter_DEFINED

#include "SkColor.h"
#include "SkFlattenable.h"
#include "SkXfermode.h"

class SkBitmap;
class GrProcessor;
class GrContext;

/**
 *  ColorFilters are optional objects in the drawing pipeline. When present in
 *  a paint, they are called with the "src" colors, and return new colors, which
 *  are then passed onto the next stage (either ImageFilter or Xfermode).
 *
 *  All subclasses are required to be reentrant-safe : it must be legal to share
 *  the same instance between several threads.
 */
class SK_API SkColorFilter : public SkFlattenable {
public:
    SK_DECLARE_INST_COUNT(SkColorFilter)

    /**
     *  If the filter can be represented by a source color plus Mode, this
     *  returns true, and sets (if not NULL) the color and mode appropriately.
     *  If not, this returns false and ignores the parameters.
     */
    virtual bool asColorMode(SkColor* color, SkXfermode::Mode* mode) const;

    /**
     *  If the filter can be represented by a 5x4 matrix, this
     *  returns true, and sets the matrix appropriately.
     *  If not, this returns false and ignores the parameter.
     */
    virtual bool asColorMatrix(SkScalar matrix[20]) const;

    /**
     *  If the filter can be represented by per-component table, return true,
     *  and if table is not null, copy the bitmap containing the table into it.
     *
     *  The table bitmap will be in SkBitmap::kA8_Config. Each row corresponding
     *  to each component in ARGB order. e.g. row[0] == alpha, row[1] == red,
     *  etc. To transform a color, you (logically) perform the following:
     *
     *      a' = *table.getAddr8(a, 0);
     *      r' = *table.getAddr8(r, 1);
     *      g' = *table.getAddr8(g, 2);
     *      b' = *table.getAddr8(b, 3);
     *
     *  The original component value is the horizontal index for a given row,
     *  and the stored value at that index is the new value for that component.
     */
    virtual bool asComponentTable(SkBitmap* table) const;

    /** Called with a scanline of colors, as if there was a shader installed.
        The implementation writes out its filtered version into result[].
        Note: shader and result may be the same buffer.
        @param src      array of colors, possibly generated by a shader
        @param count    the number of entries in the src[] and result[] arrays
        @param result   written by the filter
    */
    virtual void filterSpan(const SkPMColor src[], int count,
                            SkPMColor result[]) const = 0;
    /** Called with a scanline of colors, as if there was a shader installed.
        The implementation writes out its filtered version into result[].
        Note: shader and result may be the same buffer.
        @param src      array of colors, possibly generated by a shader
        @param count    the number of entries in the src[] and result[] arrays
        @param result   written by the filter
    */
    virtual void filterSpan16(const uint16_t shader[], int count,
                              uint16_t result[]) const;

    enum Flags {
        /** If set the filter methods will not change the alpha channel of the
            colors.
        */
        kAlphaUnchanged_Flag = 0x01,
        /** If set, this subclass implements filterSpan16(). If this flag is
            set, then kAlphaUnchanged_Flag must also be set.
        */
        kHasFilter16_Flag    = 0x02
    };

    /** Returns the flags for this filter. Override in subclasses to return
        custom flags.
    */
    virtual uint32_t getFlags() const { return 0; }

    /**
     *  Apply this colorfilter to the specified SkColor. This routine handles
     *  converting to SkPMColor, calling the filter, and then converting back
     *  to SkColor. This method is not virtual, but will call filterSpan()
     *   which is virtual.
     */
    SkColor filterColor(SkColor) const;

    /** Create a colorfilter that uses the specified color and mode.
        If the Mode is DST, this function will return NULL (since that
        mode will have no effect on the result).
        @param c    The source color used with the specified mode
        @param mode The xfermode mode that is applied to each color in
                        the colorfilter's filterSpan[16,32] methods
        @return colorfilter object that applies the src color and mode,
                    or NULL if the mode will have no effect.
    */
    static SkColorFilter* CreateModeFilter(SkColor c, SkXfermode::Mode mode);

    /** Create a colorfilter that multiplies the RGB channels by one color, and
        then adds a second color, pinning the result for each component to
        [0..255]. The alpha components of the mul and add arguments
        are ignored.
    */
    static SkColorFilter* CreateLightingFilter(SkColor mul, SkColor add);

    /** Construct a colorfilter whose effect is to first apply the inner filter and then apply
     *  the outer filter to the result of the inner's.
     *  The reference counts for outer and inner are incremented.
     */
    static SkColorFilter* CreateComposeFilter(SkColorFilter* outer, SkColorFilter* inner);

    /** A subclass may implement this factory function to work with the GPU backend. If the return
        is non-NULL then the caller owns a ref on the returned object.
     */
    virtual GrFragmentProcessor* asFragmentProcessor(GrContext*) const;

    SK_TO_STRING_PUREVIRT()

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
    SK_DEFINE_FLATTENABLE_TYPE(SkColorFilter)

protected:
    SkColorFilter() {}

private:
    typedef SkFlattenable INHERITED;
};

#endif
