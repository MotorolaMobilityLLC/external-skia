/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicture_DEFINED
#define SkPicture_DEFINED

#include "SkRefCnt.h"
#include "SkRect.h"
#include "SkTypes.h"

class GrContext;
class SkBigPicture;
class SkBitmap;
class SkCanvas;
class SkData;
struct SkDeserialProcs;
class SkImage;
class SkPath;
class SkPictureData;
class SkReadBuffer;
class SkRefCntSet;
struct SkSerialProcs;
class SkStream;
class SkTypefacePlayback;
class SkWStream;
class SkWriteBuffer;
struct SkPictInfo;

/** \class SkPicture

    An SkPicture records drawing commands made to a canvas to be played back at a later time.
    This base class handles serialization and a few other miscellany.
*/
class SK_API SkPicture : public SkRefCnt {
public:
    /**
     *  Function signature defining a function that sets up an SkBitmap from encoded data. On
     *  success, the SkBitmap should have its Config, width, height, rowBytes and pixelref set.
     *  If the installed pixelref has decoded the data into pixels, then the src buffer need not be
     *  copied. If the pixelref defers the actual decode until its lockPixels() is called, then it
     *  must make a copy of the src buffer.
     *  @param src Encoded data.
     *  @param length Size of the encoded data, in bytes.
     *  @param dst SkBitmap to install the pixel ref on.
     *  @param bool Whether or not a pixel ref was successfully installed.
     */
    typedef bool (*InstallPixelRefProc)(const void* src, size_t length, SkBitmap* dst);

    /**
     *  Recreate a picture that was serialized into a stream or data.
     */

    static sk_sp<SkPicture> MakeFromStream(SkStream*);
    static sk_sp<SkPicture> MakeFromData(const SkData* data);
    static sk_sp<SkPicture> MakeFromData(const void* data, size_t size);

    static sk_sp<SkPicture> MakeFromStream(SkStream*, const SkDeserialProcs& procs);
    static sk_sp<SkPicture> MakeFromData(const SkData* data, const SkDeserialProcs& procs);
    static sk_sp<SkPicture> MakeFromData(sk_sp<SkData> data, const SkDeserialProcs& procs);

    /**
     *  Recreate a picture that was serialized into a buffer. If the creation requires bitmap
     *  decoding, the decoder must be set on the SkReadBuffer parameter by calling
     *  SkReadBuffer::setBitmapDecoder() before calling SkPicture::CreateFromBuffer().
     *  @param SkReadBuffer Serialized picture data.
     *  @return A new SkPicture representing the serialized data, or NULL if the buffer is
     *          invalid.
     */
    static sk_sp<SkPicture> MakeFromBuffer(SkReadBuffer&);

    /**
    *  Subclasses of this can be passed to playback(). During the playback
    *  of the picture, this callback will periodically be invoked. If its
    *  abort() returns true, then picture playback will be interrupted.
    *
    *  The resulting drawing is undefined, as there is no guarantee how often the
    *  callback will be invoked. If the abort happens inside some level of nested
    *  calls to save(), restore will automatically be called to return the state
    *  to the same level it was before the playback call was made.
    */
    class SK_API AbortCallback {
    public:
        AbortCallback() {}
        virtual ~AbortCallback() {}
        virtual bool abort() = 0;
    };

    /** Replays the drawing commands on the specified canvas. Note that
        this has the effect of unfurling this picture into the destination
        canvas. Using the SkCanvas::drawPicture entry point gives the destination
        canvas the option of just taking a ref.
        @param canvas the canvas receiving the drawing commands.
        @param callback a callback that allows interruption of playback
    */
    virtual void playback(SkCanvas*, AbortCallback* = nullptr) const = 0;

    /** Return a cull rect for this picture.
        Ops recorded into this picture that attempt to draw outside the cull might not be drawn.
     */
    virtual SkRect cullRect() const = 0;

    /** Returns a non-zero value unique among all pictures. */
    uint32_t uniqueID() const;

    sk_sp<SkData> serialize() const;
    void serialize(SkWStream*) const;
    sk_sp<SkData> serialize(const SkSerialProcs&) const;

    /**
     *  Serialize to a buffer.
     */
    void flatten(SkWriteBuffer&) const;

    /**
     * Returns true if any bitmaps may be produced when this SkPicture
     * is replayed.
     */
    virtual bool willPlayBackBitmaps() const = 0;

    /** Return the approximate number of operations in this picture.  This
     *  number may be greater or less than the number of SkCanvas calls
     *  recorded: some calls may be recorded as more than one operation, or some
     *  calls may be optimized away.
     */
    virtual int approximateOpCount() const = 0;

    /** Returns the approximate byte size of this picture, not including large ref'd objects. */
    virtual size_t approximateBytesUsed() const = 0;

#ifdef SK_SUPPORT_LEGACY_PICTURE_GPUVETO
    /** Return true if the picture is suitable for rendering on the GPU.  */
    bool suitableForGpuRasterization(GrContext*, const char** whyNot = nullptr) const;
#endif

    // Returns NULL if this is not an SkBigPicture.
    virtual const SkBigPicture* asSkBigPicture() const { return nullptr; }

    static bool PictureIOSecurityPrecautionsEnabled();

private:
    // Subclass whitelist.
    SkPicture();
    friend class SkBigPicture;
    friend class SkEmptyPicture;
    template <typename> friend class SkMiniPicture;

    void serialize(SkWStream*, const SkSerialProcs&, SkRefCntSet* typefaces) const;
    static sk_sp<SkPicture> MakeFromStream(SkStream*, const SkDeserialProcs&, SkTypefacePlayback*);
    friend class SkPictureData;

    /** Return true if the SkStream/Buffer represents a serialized picture, and
     fills out SkPictInfo. After this function returns, the data source is not
     rewound so it will have to be manually reset before passing to
     CreateFromStream or CreateFromBuffer. Note, CreateFromStream and
     CreateFromBuffer perform this check internally so these entry points are
     intended for stand alone tools.
     If false is returned, SkPictInfo is unmodified.
     */
    static bool StreamIsSKP(SkStream*, SkPictInfo*);
    static bool BufferIsSKP(SkReadBuffer*, SkPictInfo*);
    friend bool SkPicture_StreamIsSKP(SkStream*, SkPictInfo*);

    virtual int numSlowPaths() const = 0;
    friend class SkPictureGpuAnalyzer;
    friend struct SkPathCounter;

    // V35: Store SkRect (rather then width & height) in header
    // V36: Remove (obsolete) alphatype from SkColorTable
    // V37: Added shadow only option to SkDropShadowImageFilter (last version to record CLEAR)
    // V38: Added PictureResolution option to SkPictureImageFilter
    // V39: Added FilterLevel option to SkPictureImageFilter
    // V40: Remove UniqueID serialization from SkImageFilter.
    // V41: Added serialization of SkBitmapSource's filterQuality parameter
    // V42: Added a bool to SkPictureShader serialization to indicate did-we-serialize-a-picture?
    // V43: Added DRAW_IMAGE and DRAW_IMAGE_RECT opt codes to serialized data
    // V44: Move annotations from paint to drawAnnotation
    // V45: Add invNormRotation to SkLightingShader.
    // V46: Add drawTextRSXform
    // V47: Add occluder rect to SkBlurMaskFilter
    // V48: Read and write extended SkTextBlobs.
    // V49: Gradients serialized as SkColor4f + SkColorSpace
    // V50: SkXfermode -> SkBlendMode
    // V51: more SkXfermode -> SkBlendMode
    // V52: Remove SkTextBlob::fRunCount
    // V53: SaveLayerRec clip mask
    // V54: ComposeShader can use a Mode or a Lerp
    // V55: Drop blendmode[] from MergeImageFilter
    // V56: Add TileMode in SkBlurImageFilter.
    // V57: Sweep tiling info.
    // V58: No more 2pt conical flipping.
    // V59: No more LocalSpace option on PictureImageFilter

    // Only SKPs within the min/current picture version range (inclusive) can be read.
    static const uint32_t     MIN_PICTURE_VERSION = 51;     // Produced by Chrome ~M56.
    static const uint32_t CURRENT_PICTURE_VERSION = 59;

    static bool IsValidPictInfo(const SkPictInfo& info);
    static sk_sp<SkPicture> Forwardport(const SkPictInfo&,
                                        const SkPictureData*,
                                        SkReadBuffer* buffer);

    SkPictInfo createHeader() const;
    SkPictureData* backport() const;

    mutable uint32_t fUniqueID;
};

#endif
