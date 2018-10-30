
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrColor_DEFINED
#define GrColor_DEFINED

#include "GrTypes.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkHalf.h"
#include "SkUnPreMultiply.h"

/**
 * GrColor is 4 bytes for R, G, B, A, in a specific order defined below. Whether the color is
 * premultiplied or not depends on the context in which it is being used.
 */
typedef uint32_t GrColor;

// shift amount to assign a component to a GrColor int
// These shift values are chosen for compatibility with GL attrib arrays
// ES doesn't allow BGRA vertex attrib order so if they were not in this order
// we'd have to swizzle in shaders.
#ifdef SK_CPU_BENDIAN
    #define GrColor_SHIFT_R     24
    #define GrColor_SHIFT_G     16
    #define GrColor_SHIFT_B     8
    #define GrColor_SHIFT_A     0
#else
    #define GrColor_SHIFT_R     0
    #define GrColor_SHIFT_G     8
    #define GrColor_SHIFT_B     16
    #define GrColor_SHIFT_A     24
#endif

/**
 *  Pack 4 components (RGBA) into a GrColor int
 */
static inline GrColor GrColorPackRGBA(unsigned r, unsigned g, unsigned b, unsigned a) {
    SkASSERT((uint8_t)r == r);
    SkASSERT((uint8_t)g == g);
    SkASSERT((uint8_t)b == b);
    SkASSERT((uint8_t)a == a);
    return  (r << GrColor_SHIFT_R) |
            (g << GrColor_SHIFT_G) |
            (b << GrColor_SHIFT_B) |
            (a << GrColor_SHIFT_A);
}

/**
 *  Packs a color with an alpha channel replicated across all four channels.
 */
static inline GrColor GrColorPackA4(unsigned a) {
    SkASSERT((uint8_t)a == a);
    return  (a << GrColor_SHIFT_R) |
            (a << GrColor_SHIFT_G) |
            (a << GrColor_SHIFT_B) |
            (a << GrColor_SHIFT_A);
}

// extract a component (byte) from a GrColor int

#define GrColorUnpackR(color)   (((color) >> GrColor_SHIFT_R) & 0xFF)
#define GrColorUnpackG(color)   (((color) >> GrColor_SHIFT_G) & 0xFF)
#define GrColorUnpackB(color)   (((color) >> GrColor_SHIFT_B) & 0xFF)
#define GrColorUnpackA(color)   (((color) >> GrColor_SHIFT_A) & 0xFF)

/**
 *  Since premultiplied means that alpha >= color, we construct a color with
 *  each component==255 and alpha == 0 to be "illegal"
 */
#define GrColor_ILLEGAL     (~(0xFF << GrColor_SHIFT_A))

#define GrColor_WHITE 0xFFFFFFFF
#define GrColor_TRANSPARENT_BLACK 0x0

/**
 * Assert in debug builds that a GrColor is premultiplied.
 */
static inline void GrColorIsPMAssert(GrColor SkDEBUGCODE(c)) {
#ifdef SK_DEBUG
    unsigned a = GrColorUnpackA(c);
    unsigned r = GrColorUnpackR(c);
    unsigned g = GrColorUnpackG(c);
    unsigned b = GrColorUnpackB(c);

    SkASSERT(r <= a);
    SkASSERT(g <= a);
    SkASSERT(b <= a);
#endif
}

static inline GrColor GrColorMul(GrColor c0, GrColor c1) {
    U8CPU r = SkMulDiv255Round(GrColorUnpackR(c0), GrColorUnpackR(c1));
    U8CPU g = SkMulDiv255Round(GrColorUnpackG(c0), GrColorUnpackG(c1));
    U8CPU b = SkMulDiv255Round(GrColorUnpackB(c0), GrColorUnpackB(c1));
    U8CPU a = SkMulDiv255Round(GrColorUnpackA(c0), GrColorUnpackA(c1));
    return GrColorPackRGBA(r, g, b, a);
}

/** Converts a GrColor to an rgba array of GrGLfloat */
static inline void GrColorToRGBAFloat(GrColor color, float rgba[4]) {
    static const float ONE_OVER_255 = 1.f / 255.f;
    rgba[0] = GrColorUnpackR(color) * ONE_OVER_255;
    rgba[1] = GrColorUnpackG(color) * ONE_OVER_255;
    rgba[2] = GrColorUnpackB(color) * ONE_OVER_255;
    rgba[3] = GrColorUnpackA(color) * ONE_OVER_255;
}

/** Normalizes and coverts an uint8_t to a float. [0, 255] -> [0.0, 1.0] */
static inline float GrNormalizeByteToFloat(uint8_t value) {
    static const float ONE_OVER_255 = 1.f / 255.f;
    return value * ONE_OVER_255;
}

/** Determines whether the color is opaque or not. */
static inline bool GrColorIsOpaque(GrColor color) {
    return (color & (0xFFU << GrColor_SHIFT_A)) == (0xFFU << GrColor_SHIFT_A);
}

/** Returns an unpremuled version of the GrColor. */
static inline GrColor GrUnpremulColor(GrColor color) {
    GrColorIsPMAssert(color);
    unsigned r = GrColorUnpackR(color);
    unsigned g = GrColorUnpackG(color);
    unsigned b = GrColorUnpackB(color);
    unsigned a = GrColorUnpackA(color);
    SkPMColor colorPM = SkPackARGB32(a, r, g, b);
    SkColor colorUPM = SkUnPreMultiply::PMColorToColor(colorPM);

    r = SkColorGetR(colorUPM);
    g = SkColorGetG(colorUPM);
    b = SkColorGetB(colorUPM);
    a = SkColorGetA(colorUPM);

    return GrColorPackRGBA(r, g, b, a);
}

/**
 * GrColor4h is 8 bytes (4 half-floats) for R, G, B, A, in that order. This is intended for
 * storing wide-gamut (non-normalized) colors in ops, vertex attributes.
 */
struct GrColor4h {
    static GrColor4h FromFloats(const float* rgba) {
        SkASSERT(rgba[3] >= 0 && rgba[3] <= 1);
#ifndef SK_LEGACY_OP_COLOR_AS_BYTES
        uint64_t packed;
        SkFloatToHalf_finite_ftz(Sk4f::Load(rgba)).store(&packed);
        return { packed };
#else
        return { SkColor4f{ rgba[0], rgba[1], rgba[2], rgba[3] }.toBytes_RGBA() };
#endif
    }

    static GrColor4h FromGrColor(GrColor color) {
#ifndef SK_LEGACY_OP_COLOR_AS_BYTES
        SkColor4f c4f = SkColor4f::FromBytes_RGBA(color);
        return FromFloats(c4f.vec());
#else
        return { color };
#endif
    }

    bool isNormalized() const {
#ifndef SK_LEGACY_OP_COLOR_AS_BYTES
        auto half_is_normalized = [](SkHalf h) {
            return h <= SK_Half1 ||  // All positive values in [0, 1]
                   h == 0x8000;      // Negative 0
        };

        // Check R, G, B are in [0, 1]
        return half_is_normalized(this->vec()[0]) &&
               half_is_normalized(this->vec()[1]) &&
               half_is_normalized(this->vec()[2]);
#else
        return true;
#endif
    }

    bool isOpaque() const {
#ifndef SK_LEGACY_OP_COLOR_AS_BYTES
        return this->vec()[3] == SK_Half1;
#else
        return GrColorIsOpaque(fRGBA);
#endif
    }

    bool operator==(GrColor4h that) const {
        return fRGBA == that.fRGBA;
    }

    bool operator!=(GrColor4h that) const {
        return fRGBA != that.fRGBA;
    }

    void toFloats(float* rgba) const {
#ifndef SK_LEGACY_OP_COLOR_AS_BYTES
        SkHalfToFloat_finite_ftz(fRGBA).store(rgba);
#else
        GrColorToRGBAFloat(fRGBA, rgba);
#endif
    }

    GrColor toGrColor() const {
#ifndef SK_LEGACY_OP_COLOR_AS_BYTES
        SkColor4f c4f;
        this->toFloats(c4f.vec());
        return c4f.toBytes_RGBA();
#else
        return fRGBA;
#endif
    }

#ifndef SK_LEGACY_OP_COLOR_AS_BYTES
    const SkHalf* vec() const {
        return reinterpret_cast<const SkHalf*>(&fRGBA);
    }
#endif

#ifndef SK_LEGACY_OP_COLOR_AS_BYTES
    uint64_t fRGBA;
#else
    uint32_t fRGBA;
#endif
};

#ifndef SK_LEGACY_OP_COLOR_AS_BYTES
constexpr GrColor4h GrColor4h_WHITE       = { 0x3C003C003C003C00ull };  // 1, 1, 1, 1
constexpr GrColor4h GrColor4h_TRANSPARENT = { 0 };                      // 0, 0, 0, 0
constexpr GrColor4h GrColor4h_ILLEGAL     = { 0xFFFFFFFFFFFFFFFFull };  // -NaN. -NaN, -NaN, -NaN
#else
constexpr GrColor4h GrColor4h_WHITE       = { GrColor_WHITE };
constexpr GrColor4h GrColor4h_TRANSPARENT = { GrColor_TRANSPARENT_BLACK };
constexpr GrColor4h GrColor4h_ILLEGAL     = { GrColor_ILLEGAL };
#endif

#endif
