/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_DEFINED
#define SkImage_DEFINED

#include "SkFilterQuality.h"
#include "SkImageInfo.h"
#include "SkImageEncoder.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkShader.h"

class SkData;
class SkCanvas;
class SkColorTable;
class SkImageGenerator;
class SkPaint;
class SkPicture;
class SkPixelSerializer;
class SkString;
class SkSurface;
class SkSurfaceProps;
class GrContext;
class GrTexture;

/**
 *  SkImage is an abstraction for drawing a rectagle of pixels, though the
 *  particular type of image could be actually storing its data on the GPU, or
 *  as drawing commands (picture or PDF or otherwise), ready to be played back
 *  into another canvas.
 *
 *  The content of SkImage is always immutable, though the actual storage may
 *  change, if for example that image can be re-created via encoded data or
 *  other means.
 *
 *  SkImage always has a non-zero dimensions. If there is a request to create a new image, either
 *  directly or via SkSurface, and either of the requested dimensions are zero, then NULL will be
 *  returned.
 */
class SK_API SkImage : public SkRefCnt {
public:
    typedef SkImageInfo Info;
    typedef void* ReleaseContext;

    static SkImage* NewRasterCopy(const Info&, const void* pixels, size_t rowBytes,
                                  SkColorTable* ctable = NULL);
    static SkImage* NewRasterData(const Info&, SkData* pixels, size_t rowBytes);

    typedef void (*RasterReleaseProc)(const void* pixels, ReleaseContext);

    /**
     *  Return a new Image referencing the specified pixels. These must remain valid and unchanged
     *  until the specified release-proc is called, indicating that Skia no longer has a reference
     *  to the pixels.
     *
     *  Returns NULL if the requested Info is unsupported.
     */
    static SkImage* NewFromRaster(const Info&, const void* pixels, size_t rowBytes,
                                  RasterReleaseProc, ReleaseContext);

    /**
     *  Construct a new image from the specified bitmap. If the bitmap is marked immutable, and
     *  its pixel memory is shareable, it may be shared instead of copied.
     */
    static SkImage* NewFromBitmap(const SkBitmap&);
    
    /**
     *  Construct a new SkImage based on the given ImageGenerator.
     *  This function will always take ownership of the passed
     *  ImageGenerator.  Returns NULL on error.
     *
     *  If a subset is specified, it must be contained within the generator's bounds.
     */
    static SkImage* NewFromGenerator(SkImageGenerator*, const SkIRect* subset = NULL);

    /**
     *  Construct a new SkImage based on the specified encoded data. Returns NULL on failure,
     *  which can mean that the format of the encoded data was not recognized/supported.
     *
     *  If a subset is specified, it must be contained within the encoded data's bounds.
     *
     *  Regardless of success or failure, the caller is responsible for managing their ownership
     *  of the data.
     */
    static SkImage* NewFromEncoded(SkData* encoded, const SkIRect* subset = NULL);

    /**
     *  Create a new image from the specified descriptor. Note - the caller is responsible for
     *  managing the lifetime of the underlying platform texture.
     *
     *  Will return NULL if the specified descriptor is unsupported.
     */
    static SkImage* NewFromTexture(GrContext* ctx, const GrBackendTextureDesc& desc) {
        return NewFromTexture(ctx, desc, kPremul_SkAlphaType, NULL, NULL);
    }

    static SkImage* NewFromTexture(GrContext* ctx, const GrBackendTextureDesc& de, SkAlphaType at) {
        return NewFromTexture(ctx, de, at, NULL, NULL);
    }

    typedef void (*TextureReleaseProc)(ReleaseContext);

    /**
     *  Create a new image from the specified descriptor. The underlying platform texture must stay
     *  valid and unaltered until the specified release-proc is invoked, indicating that Skia
     *  no longer is holding a reference to it.
     *
     *  Will return NULL if the specified descriptor is unsupported.
     */
    static SkImage* NewFromTexture(GrContext*, const GrBackendTextureDesc&, SkAlphaType,
                                   TextureReleaseProc, ReleaseContext);

    /**
     *  Create a new image from the specified descriptor. Note - Skia will delete or recycle the
     *  texture when the image is released.
     *
     *  Will return NULL if the specified descriptor is unsupported.
     */
    static SkImage* NewFromAdoptedTexture(GrContext*, const GrBackendTextureDesc&,
                                          SkAlphaType = kPremul_SkAlphaType);

    /**
     *  Create a new image by copying the pixels from the specified descriptor. No reference is
     *  kept to the original platform texture.
     *
     *  Will return NULL if the specified descriptor is unsupported.
     */
    static SkImage* NewFromTextureCopy(GrContext*, const GrBackendTextureDesc&,
                                       SkAlphaType = kPremul_SkAlphaType);

    /**
     *  Create a new image by copying the pixels from the specified y, u, v textures. The data
     *  from the textures is immediately ingested into the image and the textures can be modified or
     *  deleted after the function returns. The image will have the dimensions of the y texture.
     */
    static SkImage* NewFromYUVTexturesCopy(GrContext*, SkYUVColorSpace,
                                           const GrBackendObject yuvTextureHandles[3],
                                           const SkISize yuvSizes[3],
                                           GrSurfaceOrigin);

    static SkImage* NewFromPicture(const SkPicture*, const SkISize& dimensions,
                                   const SkMatrix*, const SkPaint*);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    uint32_t uniqueID() const { return fUniqueID; }
    virtual bool isOpaque() const { return false; }

    virtual SkShader* newShader(SkShader::TileMode,
                                SkShader::TileMode,
                                const SkMatrix* localMatrix = NULL) const;

    /**
     *  If the image has direct access to its pixels (i.e. they are in local
     *  RAM) return the (const) address of those pixels, and if not null, return
     *  the ImageInfo and rowBytes. The returned address is only valid while
     *  the image object is in scope.
     *
     *  On failure, returns NULL and the info and rowBytes parameters are
     *  ignored.
     */
    const void* peekPixels(SkImageInfo* info, size_t* rowBytes) const;

    /**
     *  If the image has direct access to its pixels (i.e. they are in local
     *  RAM) return the (const) address of those pixels, and if not null, return
     *  true, and if pixmap is not NULL, set it to point into the image.
     *
     *  On failure, return false and ignore the pixmap parameter.
     */
    bool peekPixels(SkPixmap* pixmap) const;

    /**
     *  Some images have to perform preliminary work in preparation for drawing. This can be
     *  decoding, uploading to a GPU, or other tasks. These happen automatically when an image
     *  is drawn, and often they are cached so that the cost is only paid the first time.
     *
     *  Preroll() can be called before drawing to try to perform this prepatory work ahead of time.
     *  For images that have no such work, this returns instantly. Others may do some thing to
     *  prepare their cache and then return.
     *
     *  If the image will drawn to a GPU-backed canvas or surface, pass the associated GrContext.
     *  If the image will be drawn to any other type of canvas or surface, pass null.
     */
    void preroll(GrContext* = nullptr) const;

    // DEPRECATED
    GrTexture* getTexture() const;

    /**
     *  Returns true if the image is texture backed.
     */
    bool isTextureBacked() const;

    /**
     *  Retrieves the backend API handle of the texture. If flushPendingGrContextIO then the
     *  GrContext will issue to the backend API any deferred IO operations on the texture before
     *  returning.
     */
    GrBackendObject getTextureHandle(bool flushPendingGrContextIO) const;

    /**
     *  Copy the pixels from the image into the specified buffer (pixels + rowBytes),
     *  converting them into the requested format (dstInfo). The image pixels are read
     *  starting at the specified (srcX,srcY) location.
     *
     *  The specified ImageInfo and (srcX,srcY) offset specifies a source rectangle
     *
     *      srcR.setXYWH(srcX, srcY, dstInfo.width(), dstInfo.height());
     *
     *  srcR is intersected with the bounds of the image. If this intersection is not empty,
     *  then we have two sets of pixels (of equal size). Replace the dst pixels with the
     *  corresponding src pixels, performing any colortype/alphatype transformations needed
     *  (in the case where the src and dst have different colortypes or alphatypes).
     *
     *  This call can fail, returning false, for several reasons:
     *  - If srcR does not intersect the image bounds.
     *  - If the requested colortype/alphatype cannot be converted from the image's types.
     */
    bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY) const;

    bool readPixels(const SkPixmap& dst, int srcX, int srcY) const;

    /**
     *  Encode the image's pixels and return the result as a new SkData, which
     *  the caller must manage (i.e. call unref() when they are done).
     *
     *  If the image type cannot be encoded, or the requested encoder type is
     *  not supported, this will return NULL.
     *
     *  Note: this will attempt to encode the image's pixels in the specified format,
     *  even if the image returns a data from refEncoded(). That data will be ignored.
     */
    SkData* encode(SkImageEncoder::Type, int quality) const;

    /**
     *  Encode the image and return the result as a caller-managed SkData.  This will
     *  attempt to reuse existing encoded data (as returned by refEncoded).
     *
     *  We defer to the SkPixelSerializer both for vetting existing encoded data
     *  (useEncodedData) and for encoding the image (encodePixels) when no such data is
     *  present or is rejected by the serializer.
     *
     *  If not specified, we use a default serializer which 1) always accepts existing data
     *  (in any format) and 2) encodes to PNG.
     *
     *  If no compatible encoded data exists and encoding fails, this method will also
     *  fail (return NULL).
     */
    SkData* encode(SkPixelSerializer* = nullptr) const;

    /**
     *  If the image already has its contents in encoded form (e.g. PNG or JPEG), return a ref
     *  to that data (which the caller must call unref() on). The caller is responsible for calling
     *  unref on the data when they are done.
     *
     *  If the image does not already has its contents in encoded form, return NULL.
     *
     *  Note: to force the image to return its contents as encoded data, try calling encode(...).
     */
    SkData* refEncoded() const;

    /**
     *  Return a new surface that is compatible with this image's internal representation
     *  (e.g. raster or gpu).
     *
     *  If no surfaceprops are specified, the image will attempt to match the props of when it
     *  was created (if it came from a surface).
     */
    SkSurface* newSurface(const SkImageInfo&, const SkSurfaceProps* = NULL) const;

    const char* toString(SkString*) const;

    /**
     *  Return an image that is a rescale of this image (using newWidth, newHeight).
     *
     *  If subset is NULL, then the entire original image is used as the src for the scaling.
     *  If subset is not NULL, then it specifies subset of src-pixels used for scaling. If
     *  subset extends beyond the bounds of the original image, then NULL is returned.
     *
     *  Notes:
     *  - newWidth and newHeight must be > 0 or NULL will be returned.
     *
     *  - it is legal for the returned image to be the same instance as the src image
     *    (if the new dimensions == the src dimensions and subset is NULL or == src dimensions).
     *
     *  - it is legal for the "scaled" image to have changed its SkAlphaType from unpremul
     *    to premul (as required by the impl). The image should draw (nearly) identically,
     *    since during drawing we will "apply the alpha" to the pixels. Future optimizations
     *    may take away this caveat, preserving unpremul.
     */
    SkImage* newImage(int newWidth, int newHeight, const SkIRect* subset = NULL,
                      SkFilterQuality = kNone_SkFilterQuality) const;

    // Helper functions to convert to SkBitmap

    enum LegacyBitmapMode {
        kRO_LegacyBitmapMode,
        kRW_LegacyBitmapMode,
    };

    /**
     *  Attempt to create a bitmap with the same pixels as the image. The result will always be
     *  a raster-backed bitmap (texture-backed bitmaps are DEPRECATED, and not supported here).
     *
     *  If the mode is kRO (read-only), the resulting bitmap will be marked as immutable.
     *
     *  On succcess, returns true. On failure, returns false and the bitmap parameter will be reset
     *  to empty.
     */
    bool asLegacyBitmap(SkBitmap*, LegacyBitmapMode) const;
    
    /**
     *  Returns true if the image is backed by an image-generator or other src that creates
     *  (and caches) its pixels / texture on-demand.
     */
    bool isLazyGenerated() const;

protected:
    SkImage(int width, int height, uint32_t uniqueID);

private:
    const int       fWidth;
    const int       fHeight;
    const uint32_t  fUniqueID;

    typedef SkRefCnt INHERITED;
};

#endif
