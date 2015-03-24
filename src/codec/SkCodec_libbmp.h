/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkColorTable.h"
#include "SkEncodedFormat.h"
#include "SkImageInfo.h"
#include "SkMaskSwizzler.h"
#include "SkStream.h"
#include "SkSwizzler.h"
#include "SkTypes.h"

// TODO: rename SkCodec_libbmp files to SkBmpCodec
// TODO: define a wrapper for SkDebugf that doesn't always print
/*
 *
 * This class implements the decoding for bmp images
 *
 */
class SkBmpCodec : public SkCodec {
public:

    /*
     *
     * Describes if rows of the input start at the top or bottom of the image
     *
     */
    enum RowOrder {
        kTopDown_RowOrder,
        kBottomUp_RowOrder
    };

    /*
     *
     * Checks the start of the stream to see if the image is a bitmap
     *
     */
    static bool IsBmp(SkStream*);

    /*
     *
     * Assumes IsBmp was called and returned true
     * Creates a bitmap decoder
     * Reads enough of the stream to determine the image format
     *
     */
    static SkCodec* NewFromStream(SkStream*);

protected:

    /*
     *
     * Initiates the bitmap decode
     *
     */
    virtual Result onGetPixels(const SkImageInfo& dstInfo, void* dst,
                               size_t dstRowBytes, const Options&, SkPMColor*,
                               int*) SK_OVERRIDE;

    SkEncodedFormat onGetEncodedFormat() const SK_OVERRIDE { return kBMP_SkEncodedFormat; }
private:

    /*
     *
     * Used to define the input format of the bitmap
     *
     */
    enum BitmapInputFormat {
        kStandard_BitmapInputFormat,
        kRLE_BitmapInputFormat,
        kBitMask_BitmapInputFormat,
        kUnknown_BitmapInputFormat
    };

    /*
     *
     * Creates the color table
     *
     */
     bool createColorTable(SkAlphaType alphaType);

    /*
     *
     * Performs the bitmap decoding for bit masks input format
     *
     */
    Result decodeMask(const SkImageInfo& dstInfo, void* dst,
                      size_t dstRowBytes);

    /*
     *
     * Set an RLE pixel using the color table
     *
     */
    void setRLEPixel(SkPMColor* dst, size_t dstRowBytes,
                     const SkImageInfo& dstInfo, uint32_t x, uint32_t y,
                     uint8_t index);
    /*
     *
     * Set an RLE24 pixel from R, G, B values
     *
     */
    void setRLE24Pixel(SkPMColor* dst, size_t dstRowBytes,
                       const SkImageInfo& dstInfo, uint32_t x, uint32_t y,
                       uint8_t red, uint8_t green, uint8_t blue);

    /*
     *
     * Performs the bitmap decoding for RLE input format
     *
     */
    Result decodeRLE(const SkImageInfo& dstInfo, void* dst,
                     size_t dstRowBytes);

    /*
     *
     * Performs the bitmap decoding for standard input format
     *
     */
    Result decode(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes);

    /*
     *
     * Creates an instance of the decoder
     * Called only by NewFromStream
     *
     * @param srcInfo contains the source width and height
     * @param stream the stream of image data
     * @param bitsPerPixel the number of bits used to store each pixel
     * @param format the format of the bmp file
     * @param masks optional color masks for certain bmp formats, passes
                    ownership to SkBmpCodec
     * @param numColors the number of colors in the color table
     * @param bytesPerColor the number of bytes in the stream used to represent
                            each color in the color table
     * @param offset the offset of the image pixel data from the end of the
     *               headers
     * @param rowOrder indicates whether rows are ordered top-down or bottom-up
     * @param RLEBytes used only for RLE decodes, as we must decode all
     *                  of the data at once rather than row by row
     *                  it indicates the amount of data left in the stream
     *                  after decoding the headers
     *
     */
    SkBmpCodec(const SkImageInfo& srcInfo, SkStream* stream,
               uint16_t bitsPerPixel, BitmapInputFormat format,
               SkMasks* masks, uint32_t numColors, uint32_t bytesPerColor,
               uint32_t offset, RowOrder rowOrder, size_t RLEByes);

    // Fields
    const uint16_t                      fBitsPerPixel;
    const BitmapInputFormat             fInputFormat;
    SkAutoTDelete<SkMasks>              fMasks;          // owned
    SkAutoTDelete<SkColorTable>         fColorTable;     // owned
    uint32_t                            fNumColors;
    const uint32_t                      fBytesPerColor;
    const uint32_t                      fOffset;
    const RowOrder                      fRowOrder;
    const size_t                        fRLEBytes;

    typedef SkCodec INHERITED;
};
