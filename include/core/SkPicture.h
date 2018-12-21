/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Generated by tools/bookmaker from include/core/SkPicture.h and docs/SkPicture_Reference.bmh
   on 2018-08-10 12:59:44. Additional documentation and examples can be found at:
   https://skia.org/user/api/SkPicture_Reference

   You may edit either file directly. Structural changes to public interfaces require
   editing both files. After editing docs/SkPicture_Reference.bmh, run:
       bookmaker -b docs -i include/core/SkPicture.h -p
   to create an updated version of this file.
 */

#ifndef SkPicture_DEFINED
#define SkPicture_DEFINED

#include "SkRefCnt.h"
#include "SkRect.h"
#include "SkTypes.h"

class SkCanvas;
class SkData;
struct SkDeserialProcs;
class SkImage;
struct SkSerialProcs;
class SkStream;
class SkWStream;

/** \class SkPicture
    SkPicture records drawing commands made to SkCanvas. The command stream may be
    played in whole or in part at a later time.

    SkPicture is an abstract class. SkPicture may be generated by SkPictureRecorder
    or SkDrawable, or from SkPicture previously saved to SkData or SkStream.

    SkPicture may contain any SkCanvas drawing command, as well as one or more
    SkCanvas matrix or SkCanvas clip. SkPicture has a cull SkRect, which is used as
    a bounding box hint. To limit SkPicture bounds, use SkCanvas clip when
    recording or drawing SkPicture.
*/
class SK_API SkPicture : public SkRefCnt {
public:

    /** Recreates SkPicture that was serialized into a stream. Returns constructed SkPicture
        if successful; otherwise, returns nullptr. Fails if data does not permit
        constructing valid SkPicture.

        procs->fPictureProc permits supplying a custom function to decode SkPicture.
        If procs->fPictureProc is nullptr, default decoding is used. procs->fPictureCtx
        may be used to provide user context to procs->fPictureProc; procs->fPictureProc
        is called with a pointer to data, data byte length, and user context.

        @param stream  container for serial data
        @param procs   custom serial data decoders; may be nullptr
        @return        SkPicture constructed from stream data
    */
    static sk_sp<SkPicture> MakeFromStream(SkStream* stream,
                                           const SkDeserialProcs* procs = nullptr);

    /** Recreates SkPicture that was serialized into data. Returns constructed SkPicture
        if successful; otherwise, returns nullptr. Fails if data does not permit
        constructing valid SkPicture.

        procs->fPictureProc permits supplying a custom function to decode SkPicture.
        If procs->fPictureProc is nullptr, default decoding is used. procs->fPictureCtx
        may be used to provide user context to procs->fPictureProc; procs->fPictureProc
        is called with a pointer to data, data byte length, and user context.

        @param data   container for serial data
        @param procs  custom serial data decoders; may be nullptr
        @return       SkPicture constructed from data
    */
    static sk_sp<SkPicture> MakeFromData(const SkData* data,
                                         const SkDeserialProcs* procs = nullptr);

    /**

        @param data   pointer to serial data
        @param size   size of data
        @param procs  custom serial data decoders; may be nullptr
        @return       SkPicture constructed from data
    */
    static sk_sp<SkPicture> MakeFromData(const void* data, size_t size,
                                         const SkDeserialProcs* procs = nullptr);

    /** \class SkPicture::AbortCallback
        AbortCallback is an abstract class. An implementation of AbortCallback may
        passed as a parameter to SkPicture::playback, to stop it before all drawing
        commands have been processed.

        If AbortCallback::abort returns true, SkPicture::playback is interrupted.
    */
    class SK_API AbortCallback {
    public:

        /** Has no effect.

            @return  abstract class cannot be instantiated
        */
        AbortCallback() {}

        /** Has no effect.
        */
        virtual ~AbortCallback() {}

        /** Stops SkPicture playback when some condition is met. A subclass of
            AbortCallback provides an override for abort() that can stop SkPicture::playback.

            The part of SkPicture drawn when aborted is undefined. SkPicture instantiations are
            free to stop drawing at different points during playback.

            If the abort happens inside one or more calls to SkCanvas::save(), stack
            of SkCanvas matrix and SkCanvas clip values is restored to its state before
            SkPicture::playback was called.

            @return  true to stop playback
        */
        virtual bool abort() = 0;
    };

    /** Replays the drawing commands on the specified canvas. In the case that the
        commands are recorded, each command in the SkPicture is sent separately to canvas.

        To add a single command to draw SkPicture to recording canvas, call
        SkCanvas::drawPicture instead.

        @param canvas    receiver of drawing commands
        @param callback  allows interruption of playback
    */
    virtual void playback(SkCanvas* canvas, AbortCallback* callback = nullptr) const = 0;

    /** Returns cull SkRect for this picture, passed in when SkPicture was created.
        Returned SkRect does not specify clipping SkRect for SkPicture; cull is hint
        of SkPicture bounds.

        SkPicture is free to discard recorded drawing commands that fall outside
        cull.

        @return  bounds passed when SkPicture was created
    */
    virtual SkRect cullRect() const = 0;

    /** Returns a non-zero value unique among SkPicture in Skia process.

        @return  identifier for SkPicture
    */
    uint32_t uniqueID() const { return fUniqueID; }

    /** Returns storage containing SkData describing SkPicture, using optional custom
        encoders.

        procs->fPictureProc permits supplying a custom function to encode SkPicture.
        If procs->fPictureProc is nullptr, default encoding is used. procs->fPictureCtx
        may be used to provide user context to procs->fPictureProc; procs->fPictureProc
        is called with a pointer to SkPicture and user context.

        @param procs  custom serial data encoders; may be nullptr
        @return       storage containing serialized SkPicture
    */
    sk_sp<SkData> serialize(const SkSerialProcs* procs = nullptr) const;

    /** Writes picture to stream, using optional custom encoders.

        procs->fPictureProc permits supplying a custom function to encode SkPicture.
        If procs->fPictureProc is nullptr, default encoding is used. procs->fPictureCtx
        may be used to provide user context to procs->fPictureProc; procs->fPictureProc
        is called with a pointer to SkPicture and user context.

        @param stream  writable serial data stream
        @param procs   custom serial data encoders; may be nullptr
    */
    void serialize(SkWStream* stream, const SkSerialProcs* procs = nullptr) const;

    /** Returns a placeholder SkPicture. Result does not draw, and contains only
        cull SkRect, a hint of its bounds. Result is immutable; it cannot be changed
        later. Result identifier is unique.

        Returned placeholder can be intercepted during playback to insert other
        commands into SkCanvas draw stream.

        @param cull  placeholder dimensions
        @return      placeholder with unique identifier
    */
    static sk_sp<SkPicture> MakePlaceholder(SkRect cull);

    /** Returns the approximate number of operations in SkPicture. Returned value
        may be greater or less than the number of SkCanvas calls
        recorded: some calls may be recorded as more than one operation, other
        calls may be optimized away.

        @return  approximate operation count
    */
    virtual int approximateOpCount() const = 0;

    /** Returns the approximate byte size of SkPicture. Does not include large objects
        referenced by SkPicture.

        @return  approximate size
    */
    virtual size_t approximateBytesUsed() const = 0;

private:
    // Subclass whitelist.
    SkPicture();
    friend class SkBigPicture;
    friend class SkEmptyPicture;
    friend class SkPicturePriv;
    template <typename> friend class SkMiniPicture;

    void serialize(SkWStream*, const SkSerialProcs*, class SkRefCntSet* typefaces) const;
    static sk_sp<SkPicture> MakeFromStream(SkStream*, const SkDeserialProcs*,
                                           class SkTypefacePlayback*);
    friend class SkPictureData;

    /** Return true if the SkStream/Buffer represents a serialized picture, and
     fills out SkPictInfo. After this function returns, the data source is not
     rewound so it will have to be manually reset before passing to
     MakeFromStream or MakeFromBuffer. Note, MakeFromStream and
     MakeFromBuffer perform this check internally so these entry points are
     intended for stand alone tools.
     If false is returned, SkPictInfo is unmodified.
     */
    static bool StreamIsSKP(SkStream*, struct SkPictInfo*);
    static bool BufferIsSKP(class SkReadBuffer*, struct SkPictInfo*);
    friend bool SkPicture_StreamIsSKP(SkStream*, struct SkPictInfo*);

    // Returns NULL if this is not an SkBigPicture.
    virtual const class SkBigPicture* asSkBigPicture() const { return nullptr; }

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
    // V60: Remove flags in picture header
    // V61: Change SkDrawPictureRec to take two colors rather than two alphas
    // V62: Don't negate size of custom encoded images (don't write origin x,y either)
    // V63: Store image bounds (including origin) instead of just width/height to support subsets
    // V64: Remove occluder feature from blur maskFilter
    // V65: Float4 paint color
    // V66: Add saveBehind
    // V67: Blobs serialize fonts instead of paints

    // Only SKPs within the min/current picture version range (inclusive) can be read.
    static const uint32_t     MIN_PICTURE_VERSION = 56;     // august 2017
    static const uint32_t CURRENT_PICTURE_VERSION = 67;

    static_assert(MIN_PICTURE_VERSION <= 62, "Remove kFontAxes_bad from SkFontDescriptor.cpp");

    static bool IsValidPictInfo(const struct SkPictInfo& info);
    static sk_sp<SkPicture> Forwardport(const struct SkPictInfo&,
                                        const class SkPictureData*,
                                        class SkReadBuffer* buffer);

    struct SkPictInfo createHeader() const;
    class SkPictureData* backport() const;

    uint32_t fUniqueID;
};

#endif
