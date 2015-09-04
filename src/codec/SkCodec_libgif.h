/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkColorTable.h"
#include "SkImageInfo.h"
#include "SkScanlineDecoder.h"
#include "SkSwizzler.h"

#include "gif_lib.h"

/*
 *
 * This class implements the decoding for gif images
 *
 */
class SkGifCodec : public SkCodec {
public:

    /*
     * Checks the start of the stream to see if the image is a gif
     */
    static bool IsGif(SkStream*);

    /*
     * Assumes IsGif was called and returned true
     * Creates a gif decoder
     * Reads enough of the stream to determine the image format
     */
    static SkCodec* NewFromStream(SkStream*);

    static SkScanlineDecoder* NewSDFromStream(SkStream* stream);

protected:

    /*
     * Read enough of the stream to initialize the SkGifCodec.
     * Returns a bool representing success or failure.
     *
     * @param codecOut
     * If it returned true, and codecOut was not nullptr,
     * codecOut will be set to a new SkGifCodec.
     *
     * @param gifOut
     * If it returned true, and codecOut was nullptr,
     * gifOut must be non-nullptr and gifOut will be set to a new
     * GifFileType pointer.
     *
     * @param stream
     * Deleted on failure.
     * codecOut will take ownership of it in the case where we created a codec.
     * Ownership is unchanged when we returned a gifOut.
     *
     */
    static bool ReadHeader(SkStream* stream, SkCodec** codecOut,
            GifFileType** gifOut);

    /*
     * Performs the full gif decode
     */
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&,
            SkPMColor*, int32_t*) override;

    SkEncodedFormat onGetEncodedFormat() const override {
        return kGIF_SkEncodedFormat;
    }

    bool onRewind() override;

private:

    /*
     * A gif can contain multiple image frames.  We will only decode the first
     * frame.  This function reads up to the first image frame, processing
     * transparency and/or animation information that comes before the image
     * data.
     *
     * @param gif        Pointer to the library type that manages the gif decode
     * @param transIndex This call will set the transparent index based on the
     *                   extension data.
     */
     static SkCodec::Result ReadUpToFirstImage(GifFileType* gif, uint32_t* transIndex);

     /*
      * A gif may contain many image frames, all of different sizes.
      * This function checks if the frame dimensions are valid and corrects
      * them if necessary.  It then sets fFrameDims to the corrected
      * dimensions.
      *
      * @param desc The image frame descriptor
      */
     bool setFrameDimensions(const GifImageDesc& desc);

    /*
     * Initializes the color table that we will use for decoding.
     *
     * @param dstInfo         Contains the requested dst color type.
     * @param inputColorPtr   Copies the encoded color table to the client's
     *                        input color table if the client requests kIndex8.
     * @param inputColorCount If the client requests kIndex8, sets
     *                        inputColorCount to 256.  Since gifs always
     *                        contain 8-bit indices, we need a 256 entry color
     *                        table to ensure that indexing is always in
     *                        bounds.
     */
    void initializeColorTable(const SkImageInfo& dstInfo, SkPMColor* colorPtr,
            int* inputColorCount);

   /*
    * Checks for invalid inputs and calls rewindIfNeeded(), setFramDimensions(), and
    * initializeColorTable() in the proper sequence.
    */
    SkCodec::Result prepareToDecode(const SkImageInfo& dstInfo, SkPMColor* inputColorPtr,
            int* inputColorCount, const Options& opts);

    /*
     * Initializes the swizzler.
     *
     * @param dstInfo  Output image information.  Dimensions may have been
     *                 adjusted if the image frame size does not match the size
     *                 indicated in the header.
     * @param zeroInit Indicates if destination memory is zero initialized.
     */
    SkCodec::Result initializeSwizzler(const SkImageInfo& dstInfo,
            ZeroInitialized zeroInit);

    /*
     * @return kSuccess if the read is successful and kIncompleteInput if the
     *         read fails.
     */
    SkCodec::Result readRow();

    /*
     * This function cleans up the gif object after the decode completes
     * It is used in a SkAutoTCallIProc template
     */
    static void CloseGif(GifFileType* gif);

    /*
     * Frees any extension data used in the decode
     * Used in a SkAutoTCallVProc
     */
    static void FreeExtension(SavedImage* image);

    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     *
     * @param srcInfo contains the source width and height
     * @param stream the stream of image data
     * @param gif pointer to library type that manages gif decode
     *            takes ownership
     * @param transIndex  The transparent index.  An invalid value
     *            indicates that there is no transparent index.
     */
    SkGifCodec(const SkImageInfo& srcInfo, SkStream* stream, GifFileType* gif, uint32_t transIndex);

    SkAutoTCallVProc<GifFileType, CloseGif> fGif; // owned
    SkAutoTDeleteArray<uint8_t>             fSrcBuffer;
    SkIRect                                 fFrameDims;
    const uint32_t                          fTransIndex;
    uint32_t                                fFillIndex;
    bool                                    fFrameIsSubset;
    SkAutoTDelete<SkSwizzler>               fSwizzler;
    SkAutoTUnref<SkColorTable>              fColorTable;

    friend class SkGifScanlineDecoder;

    typedef SkCodec INHERITED;
};
