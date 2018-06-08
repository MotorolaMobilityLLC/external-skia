/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Generated by tools/bookmaker from include/core/SkColor.h and docs/SkColor_Reference.bmh
   on 2018-06-08 11:48:28. Additional documentation and examples can be found at:
   https://skia.org/user/api/SkColor_Reference

   You may edit either file directly. Structural changes to public interfaces require
   editing both files. After editing docs/SkColor_Reference.bmh, run:
       bookmaker -b docs -i include/core/SkColor.h -p
   to create an updated version of this file.
 */

#ifndef SkColor_DEFINED
#define SkColor_DEFINED

#include "SkScalar.h"
#include "SkTypes.h"

/** \file SkColor.h

    Types, consts, functions, and macros for colors.
*/

/** 8-bit type for an alpha value. 255 is 100% opaque, zero is 100% transparent.
*/
typedef uint8_t SkAlpha;

/** 32-bit ARGB color value, unpremultiplied. Color components are always in
    a known order. This is different from SkPMColor, which has its bytes in a configuration
    dependent order, to match the format of kBGRA_8888_SkColorType bitmaps. SkColor
    is the type used to specify colors in SkPaint and in gradients.

    Color that is premultiplied has the same component values as color
    that is unpremultiplied if alpha is 255, fully opaque, although may have the
    component values in a different order.
*/
typedef uint32_t SkColor;

/** Returns color value from 8-bit component values. Asserts if SK_DEBUG is defined
    if a, r, g, or b exceed 255. Since color is unpremultiplied, a may be smaller
    than the largest of r, g, and b.

    @param a  amount of alpha, from fully transparent (0) to fully opaque (255)
    @param r  amount of red, from no red (0) to full red (255)
    @param g  amount of green, from no green (0) to full green (255)
    @param b  amount of blue, from no blue (0) to full blue (255)
    @return   color and alpha, unpremultiplied
*/
static constexpr inline SkColor SkColorSetARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    return SkASSERT(a <= 255 && r <= 255 && g <= 255 && b <= 255),
           (a << 24) | (r << 16) | (g << 8) | (b << 0);
}

/** Returns color value from 8-bit component values, with alpha set
    fully opaque to 255.
*/
#define SkColorSetRGB(r, g, b)  SkColorSetARGB(0xFF, r, g, b)

/** Returns alpha byte from color value.
*/
#define SkColorGetA(color)      (((color) >> 24) & 0xFF)

/** Returns red component of color, from zero to 255.
*/
#define SkColorGetR(color)      (((color) >> 16) & 0xFF)

/** Returns green component of color, from zero to 255.
*/
#define SkColorGetG(color)      (((color) >>  8) & 0xFF)

/** Returns blue component of color, from zero to 255.
*/
#define SkColorGetB(color)      (((color) >>  0) & 0xFF)

/** Returns unpremultiplied color with red, blue, and green set from c; and alpha set
    from a. Alpha component of c is ignored and is replaced by a in result.

    @param c  packed RGB, eight bits per component
    @param a  alpha: transparent at zero, fully opaque at 255
    @return   color with transparency
*/
static constexpr inline SkColor SkColorSetA(SkColor c, U8CPU a) {
    return (c & 0x00FFFFFF) | (a << 24);
}

/** Represents fully transparent SkAlpha value. SkAlpha ranges from zero,
    fully transparent; to 255, fully opaque.
*/
constexpr SkAlpha SK_AlphaTRANSPARENT = 0x00;

/** Represents fully opaque SkAlpha value. SkAlpha ranges from zero,
    fully transparent; to 255, fully opaque.
*/
constexpr SkAlpha SK_AlphaOPAQUE      = 0xFF;

/** Represents fully transparent SkColor. May be used to initialize a destination
    containing a mask or a non-rectangular image.
*/
constexpr SkColor SK_ColorTRANSPARENT = SkColorSetARGB(0x00, 0x00, 0x00, 0x00);

/** Represents fully opaque black.
*/
constexpr SkColor SK_ColorBLACK       = SkColorSetARGB(0xFF, 0x00, 0x00, 0x00);

/** Represents fully opaque dark gray.
    Note that SVG_darkgray is equivalent to 0xFFA9A9A9.
*/
constexpr SkColor SK_ColorDKGRAY      = SkColorSetARGB(0xFF, 0x44, 0x44, 0x44);

/** Represents fully opaque gray.
    Note that HTML_Gray is equivalent to 0xFF808080.
*/
constexpr SkColor SK_ColorGRAY        = SkColorSetARGB(0xFF, 0x88, 0x88, 0x88);

/** Represents fully opaque light gray. HTML_Silver is equivalent to 0xFFC0C0C0.
    Note that SVG_lightgray is equivalent to 0xFFD3D3D3.
*/
constexpr SkColor SK_ColorLTGRAY      = SkColorSetARGB(0xFF, 0xCC, 0xCC, 0xCC);

/** Represents fully opaque white.
*/
constexpr SkColor SK_ColorWHITE       = SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF);

/** Represents fully opaque red.
*/
constexpr SkColor SK_ColorRED         = SkColorSetARGB(0xFF, 0xFF, 0x00, 0x00);

/** Represents fully opaque green. HTML_Lime is equivalent.
    Note that HTML_Green is equivalent to 0xFF008000.
*/
constexpr SkColor SK_ColorGREEN       = SkColorSetARGB(0xFF, 0x00, 0xFF, 0x00);

/** Represents fully opaque blue.
*/
constexpr SkColor SK_ColorBLUE        = SkColorSetARGB(0xFF, 0x00, 0x00, 0xFF);

/** Represents fully opaque yellow.
*/
constexpr SkColor SK_ColorYELLOW      = SkColorSetARGB(0xFF, 0xFF, 0xFF, 0x00);

/** Represents fully opaque cyan. HTML_Aqua is equivalent.
*/
constexpr SkColor SK_ColorCYAN        = SkColorSetARGB(0xFF, 0x00, 0xFF, 0xFF);

/** Represents fully opaque magenta. HTML_Fuchsia is equivalent.
*/
constexpr SkColor SK_ColorMAGENTA     = SkColorSetARGB(0xFF, 0xFF, 0x00, 0xFF);

/** Converts RGB to its HSV components.
    hsv[0] contains hsv hue, a value from zero to less than 360.
    hsv[1] contains hsv saturation, a value from zero to one.
    hsv[2] contains hsv value, a value from zero to one.

    @param red    red component value from zero to 255
    @param green  green component value from zero to 255
    @param blue   blue component value from zero to 255
    @param hsv    three element array which holds the resulting HSV components
*/
SK_API void SkRGBToHSV(U8CPU red, U8CPU green, U8CPU blue, SkScalar hsv[3]);

/** Converts ARGB to its HSV components. Alpha in ARGB is ignored.
    hsv[0] contains hsv hue, and is assigned a value from zero to less than 360.
    hsv[1] contains hsv saturation, a value from zero to one.
    hsv[2] contains hsv value, a value from zero to one.

    @param color  ARGB color to convert
    @param hsv    three element array which holds the resulting HSV components
*/
static inline void SkColorToHSV(SkColor color, SkScalar hsv[3]) {
    SkRGBToHSV(SkColorGetR(color), SkColorGetG(color), SkColorGetB(color), hsv);
}

/** Converts HSV components to an ARGB color. Alpha is passed through unchanged.
    hsv[0] represents hsv hue, an angle from zero to less than 360.
    hsv[1] represents hsv saturation, and varies from zero to one.
    hsv[2] represents hsv value, and varies from zero to one.

    Out of range hsv values are pinned.

    @param alpha  alpha component of the returned ARGB color
    @param hsv    three element array which holds the input HSV components
    @return       ARGB equivalent to HSV
*/
SK_API SkColor SkHSVToColor(U8CPU alpha, const SkScalar hsv[3]);

/** Converts HSV components to an ARGB color. Alpha is set to 255.
    hsv[0] represents hsv hue, an angle from zero to less than 360.
    hsv[1] represents hsv saturation, and varies from zero to one.
    hsv[2] represents hsv value, and varies from zero to one.

    Out of range hsv values are pinned.

    @param hsv  three element array which holds the input HSV components
    @return     RGB equivalent to HSV
*/
static inline SkColor SkHSVToColor(const SkScalar hsv[3]) {
    return SkHSVToColor(0xFF, hsv);
}

/** 32-bit ARGB color value, premultiplied. The byte order for this value is
    configuration dependent, matching the format of kBGRA_8888_SkColorType bitmaps.
    This is different from SkColor, which is unpremultiplied, and is always in the
    same byte order.
*/
typedef uint32_t SkPMColor;

/** Returns a SkPMColor value from unpremultiplied 8-bit component values.

    @param a  amount of alpha, from fully transparent (0) to fully opaque (255)
    @param r  amount of red, from no red (0) to full red (255)
    @param g  amount of green, from no green (0) to full green (255)
    @param b  amount of blue, from no blue (0) to full blue (255)
    @return   premultiplied color
*/
SK_API SkPMColor SkPreMultiplyARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b);

/** Returns pmcolor closest to color c. Multiplies c RGB components by the c alpha,
    and arranges the bytes to match the format of kN32_SkColorType.

    @param c  unpremultiplied ARGB color
    @return   premultiplied color
*/
SK_API SkPMColor SkPreMultiplyColor(SkColor c);

struct SkPM4f;

/** \struct SkColor4f
    Each component is stored as a 32-bit single precision floating point float value.
    All values are allowed, but only the range from zero to one is meaningful.

    Each component is independent of the others; fA alpha is not premultiplied
    with fG green, fB blue, or fR red.

    Values smaller than zero or larger than one are allowed. Values out of range
    may be used with SkBlendMode so that the final component is in range.
*/
struct SK_API SkColor4f {
    float fR; //!< red component
    float fG; //!< green component
    float fB; //!< blue component
    float fA; //!< alpha component

    /** Compares SkColor4f with other, and returns true if all components are equivalent.

        @param other  SkColor4f to compare
        @return       true if SkColor4f equals other
    */
    bool operator==(const SkColor4f& other) const {
        return fA == other.fA && fR == other.fR && fG == other.fG && fB == other.fB;
    }

    /** Compares SkColor4f with other, and returns true if all components are not
        equivalent.

        @param other  SkColor4f to compare
        @return       true if SkColor4f is not equal to other
    */
    bool operator!=(const SkColor4f& other) const {
        return !(*this == other);
    }

    /** Returns SkColor4f components as a read-only array.

        @return  components as read-only array
    */
    const float* vec() const { return &fR; }

    /** Returns SkColor4f components as a read-only array.

        @return  components as read-only array
    */
    float* vec() { return &fR; }

    /** Constructs and returns SkColor4f with each component pinned from zero to one.

        @param r  red component
        @param g  green component
        @param b  blue component
        @param a  alpha component
        @return   SkColor4f with valid components
    */
    static SkColor4f Pin(float r, float g, float b, float a);

    /** Converts to closest SkColor4f.

        @param SkColor  color with alpha, red, blue, and green components
        @return         SkColor4f equivalent
    */
    static SkColor4f FromColor(SkColor);

    /** Converts to closest SkColor.

        @return  closest color
    */
    SkColor toSkColor() const;

    /** Returns SkColor4f with all components in the range from zero to one.

        @return  SkColor4f with valid components
    */
    SkColor4f pin() const {
        return Pin(fR, fG, fB, fA);
    }

    /** Internal use only.

        @return  premultiplied color
    */
    SkPM4f premul() const;
};

#endif
