/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShader_DEFINED
#define SkShader_DEFINED

#include "SkBlendMode.h"
#include "SkColor.h"
#include "SkFilterQuality.h"
#include "SkFlattenable.h"
#include "SkImageInfo.h"
#include "SkMatrix.h"
#include "SkTileMode.h"

class SkArenaAlloc;
class SkBitmap;
class SkColorFilter;
class SkColorSpace;
class SkImage;
class SkMixer;
class SkPath;
class SkPicture;
class SkRasterPipeline;
class GrContext;
class GrFragmentProcessor;

#ifndef SK_SUPPORT_LEGACY_BITMAPSHADER_FACTORY
#define SK_SUPPORT_LEGACY_BITMAPSHADER_FACTORY
#endif

/** \class SkShader
 *
 *  Shaders specify the source color(s) for what is being drawn. If a paint
 *  has no shader, then the paint's color is used. If the paint has a
 *  shader, then the shader's color(s) are use instead, but they are
 *  modulated by the paint's alpha. This makes it easy to create a shader
 *  once (e.g. bitmap tiling or gradient) and then change its transparency
 *  w/o having to modify the original shader... only the paint's alpha needs
 *  to be modified.
 */
class SK_API SkShader : public SkFlattenable {
public:
#ifdef SK_SUPPORT_LEGACY_TILEMODE_ENUM
    enum TileMode {
        /**
         *  Replicate the edge color if the shader draws outside of its
         *  original bounds.
         */
        kClamp_TileMode,

        /**
         *  Repeat the shader's image horizontally and vertically.
         */
        kRepeat_TileMode,

        /**
         *  Repeat the shader's image horizontally and vertically, alternating
         *  mirror images so that adjacent images always seam.
         */
        kMirror_TileMode,

        /**
         *  Only draw within the original domain, return transparent-black everywhere else.
         */
        kDecal_TileMode,

        kLast_TileMode = kDecal_TileMode,
    };

    static constexpr int kTileModeCount = kLast_TileMode + 1;
#endif

    /**
     *  Returns true if the shader is guaranteed to produce only opaque
     *  colors, subject to the SkPaint using the shader to apply an opaque
     *  alpha value. Subclasses should override this to allow some
     *  optimizations.
     */
    virtual bool isOpaque() const { return false; }

    /**
     *  Iff this shader is backed by a single SkImage, return its ptr (the caller must ref this
     *  if they want to keep it longer than the lifetime of the shader). If not, return nullptr.
     */
    SkImage* isAImage(SkMatrix* localMatrix, SkTileMode xy[2]) const;
#ifdef SK_SUPPORT_LEGACY_TILEMODE_ENUM
    SkImage* isAImage(SkMatrix* localMatrix, TileMode xy[2]) const {
        return this->isAImage(localMatrix, (SkTileMode*)xy);
    }
#endif

    bool isAImage() const {
        return this->isAImage(nullptr, (SkTileMode*)nullptr) != nullptr;
    }

    /**
     *  If the shader subclass can be represented as a gradient, asAGradient
     *  returns the matching GradientType enum (or kNone_GradientType if it
     *  cannot). Also, if info is not null, asAGradient populates info with
     *  the relevant (see below) parameters for the gradient.  fColorCount
     *  is both an input and output parameter.  On input, it indicates how
     *  many entries in fColors and fColorOffsets can be used, if they are
     *  non-NULL.  After asAGradient has run, fColorCount indicates how
     *  many color-offset pairs there are in the gradient.  If there is
     *  insufficient space to store all of the color-offset pairs, fColors
     *  and fColorOffsets will not be altered.  fColorOffsets specifies
     *  where on the range of 0 to 1 to transition to the given color.
     *  The meaning of fPoint and fRadius is dependant on the type of gradient.
     *
     *  None:
     *      info is ignored.
     *  Color:
     *      fColorOffsets[0] is meaningless.
     *  Linear:
     *      fPoint[0] and fPoint[1] are the end-points of the gradient
     *  Radial:
     *      fPoint[0] and fRadius[0] are the center and radius
     *  Conical:
     *      fPoint[0] and fRadius[0] are the center and radius of the 1st circle
     *      fPoint[1] and fRadius[1] are the center and radius of the 2nd circle
     *  Sweep:
     *      fPoint[0] is the center of the sweep.
     */

    enum GradientType {
        kNone_GradientType,
        kColor_GradientType,
        kLinear_GradientType,
        kRadial_GradientType,
        kSweep_GradientType,
        kConical_GradientType,
        kLast_GradientType = kConical_GradientType,
    };

    struct GradientInfo {
        int         fColorCount;    //!< In-out parameter, specifies passed size
                                    //   of fColors/fColorOffsets on input, and
                                    //   actual number of colors/offsets on
                                    //   output.
        SkColor*    fColors;        //!< The colors in the gradient.
        SkScalar*   fColorOffsets;  //!< The unit offset for color transitions.
        SkPoint     fPoint[2];      //!< Type specific, see above.
        SkScalar    fRadius[2];     //!< Type specific, see above.
#ifdef SK_SUPPORT_LEGACY_TILEMODE_ENUM
        TileMode    fTileMode;      //!< The tile mode used.
#else
        SkTileMode  fTileMode;
#endif
        uint32_t    fGradientFlags; //!< see SkGradientShader::Flags
    };

    // DEPRECATED. skbug.com/8941
    virtual GradientType asAGradient(GradientInfo* info) const;

    //////////////////////////////////////////////////////////////////////////
    //  Methods to create combinations or variants of shaders

    /**
     *  Return a shader that will apply the specified localMatrix to this shader.
     *  The specified matrix will be applied before any matrix associated with this shader.
     */
    sk_sp<SkShader> makeWithLocalMatrix(const SkMatrix&) const;

    /**
     *  Create a new shader that produces the same colors as invoking this shader and then applying
     *  the colorfilter.
     */
    sk_sp<SkShader> makeWithColorFilter(sk_sp<SkColorFilter>) const;

    //////////////////////////////////////////////////////////////////////////
    //  Factory methods for stock shaders

    /**
     *  Call this to create a new "empty" shader, that will not draw anything.
     */
    static sk_sp<SkShader> MakeEmptyShader();

    /**
     *  Call this to create a new shader that just draws the specified color. This should always
     *  draw the same as a paint with this color (and no shader).
     */
    static sk_sp<SkShader> MakeColorShader(SkColor);

    /**
     *  Create a shader that draws the specified color (in the specified colorspace).
     *
     *  This works around the limitation that SkPaint::setColor() only takes byte values, and does
     *  not support specific colorspaces.
     */
    static sk_sp<SkShader> MakeColorShader(const SkColor4f&, sk_sp<SkColorSpace>);

    static sk_sp<SkShader> MakeBlend(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src);

    /*
     *  DEPRECATED: call MakeBlend.
     */
    static sk_sp<SkShader> MakeComposeShader(sk_sp<SkShader> dst, sk_sp<SkShader> src,
                                             SkBlendMode mode) {
        return MakeBlend(mode, std::move(dst), std::move(src));
    }

    /**
     *  Compose two shaders together using a weighted average.
     *
     *  result = dst * (1 - weight) + src * weight
     *
     *  If either shader is nullptr, then this returns nullptr.
     *  If weight is NaN then this returns nullptr, otherwise lerp is clamped to [0..1].
     */
    static sk_sp<SkShader> MakeLerp(float weight, sk_sp<SkShader> dst, sk_sp<SkShader> src);

    static sk_sp<SkShader> MakeMixer(sk_sp<SkShader> dst, sk_sp<SkShader> src, sk_sp<SkMixer>);

#ifdef SK_SUPPORT_LEGACY_BITMAPSHADER_FACTORY
    /** DEPRECATED. call bitmap.makeShader()
     *  Call this to create a new shader that will draw with the specified bitmap.
     *
     *  If the bitmap cannot be used (e.g. has no pixels, or its dimensions
     *  exceed implementation limits (currently at 64K - 1)) then SkEmptyShader
     *  may be returned.
     *
     *  If the src is kA8_Config then that mask will be colorized using the color on
     *  the paint.
     *
     *  @param src  The bitmap to use inside the shader
     *  @param tmx  The tiling mode to use when sampling the bitmap in the x-direction.
     *  @param tmy  The tiling mode to use when sampling the bitmap in the y-direction.
     *  @return     Returns a new shader object. Note: this function never returns null.
    */
    static sk_sp<SkShader> MakeBitmapShader(const SkBitmap& src, SkTileMode tmx, SkTileMode tmy,
                                            const SkMatrix* localMatrix = nullptr);
#endif
#ifdef SK_SUPPORT_LEGACY_TILEMODE_ENUM
    static sk_sp<SkShader> MakeBitmapShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                            const SkMatrix* localMatrix = nullptr) {
        return MakeBitmapShader(src, static_cast<SkTileMode>(tmx), static_cast<SkTileMode>(tmy),
                                localMatrix);
    }

    /** DEPRECATED: call picture->makeShader(...)
     *  Call this to create a new shader that will draw with the specified picture.
     *
     *  @param src  The picture to use inside the shader (if not NULL, its ref count
     *              is incremented). The SkPicture must not be changed after
     *              successfully creating a picture shader.
     *  @param tmx  The tiling mode to use when sampling the bitmap in the x-direction.
     *  @param tmy  The tiling mode to use when sampling the bitmap in the y-direction.
     *  @param tile The tile rectangle in picture coordinates: this represents the subset
     *              (or superset) of the picture used when building a tile. It is not
     *              affected by localMatrix and does not imply scaling (only translation
     *              and cropping). If null, the tile rect is considered equal to the picture
     *              bounds.
     *  @return     Returns a new shader object. Note: this function never returns null.
    */
    static sk_sp<SkShader> MakePictureShader(sk_sp<SkPicture> src, TileMode tmx, TileMode tmy,
                                             const SkMatrix* localMatrix, const SkRect* tile);
#endif

#ifdef SK_SUPPORT_LEGACY_SHADER_LOCALMATRIX
    SkMatrix getLocalMatrix() const;
#endif

private:
    SkShader() = default;
    friend class SkShaderBase;

    typedef SkFlattenable INHERITED;
};

#endif
