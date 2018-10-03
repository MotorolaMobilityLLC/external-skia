/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImagePriv_DEFINED
#define SkImagePriv_DEFINED

#include "SkImage.h"
#include "SkSurface.h"

enum SkCopyPixelsMode {
    kIfMutable_SkCopyPixelsMode,  //!< only copy src pixels if they are marked mutable
    kAlways_SkCopyPixelsMode,     //!< always copy src pixels (even if they are marked immutable)
    kNever_SkCopyPixelsMode,      //!< never copy src pixels (even if they are marked mutable)
};

// A good size for creating shader contexts on the stack.
enum {kSkBlitterContextSize = 3332};

// If alloc is non-nullptr, it will be used to allocate the returned SkShader, and MUST outlive
// the SkShader.
sk_sp<SkShader> SkMakeBitmapShader(const SkBitmap& src, SkShader::TileMode, SkShader::TileMode,
                                   const SkMatrix* localMatrix, SkCopyPixelsMode);

/**
 *  Examines the bitmap to decide if it can share the existing pixelRef, or
 *  if it needs to make a deep-copy of the pixels.
 *
 *  The bitmap's pixelref will be shared if either the bitmap is marked as
 *  immutable, or CopyPixelsMode allows it. Shared pixel refs are also
 *  locked when kLocked_SharedPixelRefMode is specified.
 *
 *  Passing kLocked_SharedPixelRefMode allows the image's peekPixels() method
 *  to succeed, but it will force any lazy decodes/generators to execute if
 *  they exist on the pixelref.
 *
 *  It is illegal to call this with a texture-backed bitmap.
 *
 *  If the bitmap's colortype cannot be converted into a corresponding
 *  SkImageInfo, or the bitmap's pixels cannot be accessed, this will return
 *  nullptr.
 */
extern SK_API sk_sp<SkImage> SkMakeImageFromRasterBitmap(const SkBitmap&, SkCopyPixelsMode);

// Given an image created from SkNewImageFromBitmap, return its pixelref. This
// may be called to see if the surface and the image share the same pixelref,
// in which case the surface may need to perform a copy-on-write.
extern const SkPixelRef* SkBitmapImageGetPixelRef(const SkImage* rasterImage);

/**
 *  Will attempt to upload and lock the contents of the image as a texture, so that subsequent
 *  draws to a gpu-target will come from that texture (and not by looking at the original image
 *  src). In particular this is intended to use the texture even if the image's original content
 *  changes subsequent to this call (i.e. the src is mutable!).
 *
 *  All successful calls must be balanced by an equal number of calls to SkImage_unpinAsTexture().
 *
 *  Once in this "pinned" state, the image has all of the same thread restrictions that exist
 *  for a natively created gpu image (e.g. SkImage::MakeFromTexture)
 *  - all drawing, pinning, unpinning must happen in the same thread as the GrContext.
 *
 *  @return true if the image was successfully uploaded and locked into a texture
 */
bool SkImage_pinAsTexture(const SkImage*, GrContext*);

/**
 *  The balancing call to a successful invokation of SkImage_pinAsTexture.  When a balanced number of
 *  calls have been made, then the "pinned" texture is free to be purged, etc. This also means that a
 *  subsequent "pin" call will look at the original content again, and if its uniqueID/generationID
 *  has changed, then a newer texture will be uploaded/pinned.
 *
 *  The context passed to unpin must match the one passed to pin.
 */
void SkImage_unpinAsTexture(const SkImage*, GrContext*);

/**
 *  Returns the bounds of the image relative to its encoded buffer. For all non-lazy images,
 *  this returns (0,0,width,height). For a lazy-image, it may return a subset of that rect.
 */
SkIRect SkImage_getSubset(const SkImage*);

/**
 *  Returns a new image containing the same pixel values as the source, but with a different color
 *  space assigned. This performs no color space conversion. Primarily used in tests, to visualize
 *  the results of rendering in wide or narrow gamuts.
 */
sk_sp<SkImage> SkImageMakeRasterCopyAndAssignColorSpace(const SkImage*, SkColorSpace*);

/** \enum SkImageInfo::SkImageSourceChannel
    Describes different channels we could read from image source.
*/
enum SkImageSourceChannel {
    /** Describes the red channel; */
    kR_SkImageSourceChannel,

    /** Describes the green channel; */
    kG_SkImageSourceChannel,

    /** Describes the blue channel; */
    kB_SkImageSourceChannel,

    /** Describes the alpha channel; */
    kA_SkImageSourceChannel,

    /** Utility value */
    kLastEnum_SkImageSourceChannel = kA_SkImageSourceChannel,
};

/** \struct SkYUVAIndex
    Describes from which image source and which channel to read each individual YUVA plane.

    SkYUVAIndex contains a index for which image source to read from and a enum for which channel
    to read from.
*/
struct SK_API SkYUVAIndex {
    // Index in the array of SkYUVAIndex
    enum Index {
        kY_Index = 0,
        kU_Index = 1,
        kV_Index = 2,
        kA_Index = 3
    };

    /** The index is a number between -1..3 which definies which image source to read from, where -1
     * means the image source doesn't exist. The assumption is we will always have image sources for
     * each of YUV planes, but optionally have image source for A plane. */
    int fIndex;
    /** The channel describes from which channel to read the info from. Currently we only deal with
     * YUV and NV12 and channel info is ignored. */
    SkImageSourceChannel fChannel;
};

#endif
