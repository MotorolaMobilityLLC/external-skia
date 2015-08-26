/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBmpRLECodec.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkScanlineDecoder.h"
#include "SkStream.h"

/*
 * Creates an instance of the decoder
 * Called only by NewFromStream
 */
SkBmpRLECodec::SkBmpRLECodec(const SkImageInfo& info,
                             SkStream* stream,
                             uint16_t bitsPerPixel,
                             uint32_t numColors,
                             uint32_t bytesPerColor,
                             uint32_t offset,
                             SkBmpCodec::RowOrder rowOrder,
                             size_t RLEBytes)
    : INHERITED(info, stream, bitsPerPixel, rowOrder)
    , fColorTable(NULL)
    , fNumColors(this->computeNumColors(numColors))
    , fBytesPerColor(bytesPerColor)
    , fOffset(offset)
    , fStreamBuffer(new uint8_t[RLEBytes])
    , fRLEBytes(RLEBytes)
    , fCurrRLEByte(0) {}

/*
 * Initiates the bitmap decode
 */
SkCodec::Result SkBmpRLECodec::onGetPixels(const SkImageInfo& dstInfo,
                                        void* dst, size_t dstRowBytes,
                                        const Options& opts,
                                        SkPMColor* inputColorPtr,
                                        int* inputColorCount) {
    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
    }
    if (opts.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }
    if (dstInfo.dimensions() != this->getInfo().dimensions()) {
        SkCodecPrintf("Error: scaling not supported.\n");
        return kInvalidScale;
    }
    if (!conversion_possible(dstInfo, this->getInfo())) {
        SkCodecPrintf("Error: cannot convert input type to output type.\n");
        return kInvalidConversion;
    }

    // Create the color table if necessary and prepare the stream for decode
    // Note that if it is non-NULL, inputColorCount will be modified
    if (!this->createColorTable(inputColorCount)) {
        SkCodecPrintf("Error: could not create color table.\n");
        return kInvalidInput;
    }

    // Copy the color table to the client if necessary
    copy_color_table(dstInfo, fColorTable, inputColorPtr, inputColorCount);

    // Initialize a swizzler if necessary
    if (!this->initializeStreamBuffer()) {
        SkCodecPrintf("Error: cannot initialize swizzler.\n");
        return kInvalidConversion;
    }

    // Perform the decode
    return decode(dstInfo, dst, dstRowBytes, opts);
}

/*
 * Process the color table for the bmp input
 */
 bool SkBmpRLECodec::createColorTable(int* numColors) {
    // Allocate memory for color table
    uint32_t colorBytes = 0;
    SkPMColor colorTable[256];
    if (this->bitsPerPixel() <= 8) {
        // Inform the caller of the number of colors
        uint32_t maxColors = 1 << this->bitsPerPixel();
        if (NULL != numColors) {
            // We set the number of colors to maxColors in order to ensure
            // safe memory accesses.  Otherwise, an invalid pixel could
            // access memory outside of our color table array.
            *numColors = maxColors;
        }

        // Read the color table from the stream
        colorBytes = fNumColors * fBytesPerColor;
        SkAutoTDeleteArray<uint8_t> cBuffer(new uint8_t[colorBytes]);
        if (stream()->read(cBuffer.get(), colorBytes) != colorBytes) {
            SkCodecPrintf("Error: unable to read color table.\n");
            return false;
        }

        // Fill in the color table
        uint32_t i = 0;
        for (; i < fNumColors; i++) {
            uint8_t blue = get_byte(cBuffer.get(), i*fBytesPerColor);
            uint8_t green = get_byte(cBuffer.get(), i*fBytesPerColor + 1);
            uint8_t red = get_byte(cBuffer.get(), i*fBytesPerColor + 2);
            colorTable[i] = SkPackARGB32NoCheck(0xFF, red, green, blue);
        }

        // To avoid segmentation faults on bad pixel data, fill the end of the
        // color table with black.  This is the same the behavior as the
        // chromium decoder.
        for (; i < maxColors; i++) {
            colorTable[i] = SkPackARGB32NoCheck(0xFF, 0, 0, 0);
        }

        // Set the color table
        fColorTable.reset(new SkColorTable(colorTable, maxColors));
    }

    // Check that we have not read past the pixel array offset
    if(fOffset < colorBytes) {
        // This may occur on OS 2.1 and other old versions where the color
        // table defaults to max size, and the bmp tries to use a smaller
        // color table.  This is invalid, and our decision is to indicate
        // an error, rather than try to guess the intended size of the
        // color table.
        SkCodecPrintf("Error: pixel data offset less than color table size.\n");
        return false;
    }

    // After reading the color table, skip to the start of the pixel array
    if (stream()->skip(fOffset - colorBytes) != fOffset - colorBytes) {
        SkCodecPrintf("Error: unable to skip to image data.\n");
        return false;
    }

    // Return true on success
    return true;
}

bool SkBmpRLECodec::initializeStreamBuffer() {
    // Setup a buffer to contain the full input stream
    size_t totalBytes = this->stream()->read(fStreamBuffer.get(), fRLEBytes);
    if (totalBytes < fRLEBytes) {
        fRLEBytes = totalBytes;
        SkCodecPrintf("Warning: incomplete RLE file.\n");
    }
    if (fRLEBytes == 0) {
        SkCodecPrintf("Error: could not read RLE image data.\n");
        return false;
    }
    return true;
}

/*
 * Before signalling kIncompleteInput, we should attempt to load the
 * stream buffer with additional data.
 *
 * @return the number of bytes remaining in the stream buffer after
 *         attempting to read more bytes from the stream
 */
size_t SkBmpRLECodec::checkForMoreData() {
    const size_t remainingBytes = fRLEBytes - fCurrRLEByte;
    uint8_t* buffer = fStreamBuffer.get();

    // We will be reusing the same buffer, starting over from the beginning.
    // Move any remaining bytes to the start of the buffer.
    // We use memmove() instead of memcpy() because there is risk that the dst
    // and src memory will overlap in corrupt images.
    memmove(buffer, SkTAddOffset<uint8_t>(buffer, fCurrRLEByte), remainingBytes);

    // Adjust the buffer ptr to the start of the unfilled data.
    buffer += remainingBytes;

    // Try to read additional bytes from the stream.  There are fCurrRLEByte
    // bytes of additional space remaining in the buffer, assuming that we
    // have already copied remainingBytes to the start of the buffer.
    size_t additionalBytes = this->stream()->read(buffer, fCurrRLEByte);

    // Update counters and return the number of bytes we currently have
    // available.  We are at the start of the buffer again.
    fCurrRLEByte = 0;
    // If we were unable to fill the buffer, fRLEBytes is no longer equal to
    // the size of the buffer.  There will be unused space at the end.  This
    // should be fine, given that there are no more bytes in the stream.
    fRLEBytes = remainingBytes + additionalBytes;
    return fRLEBytes;
}

/*
 * Set an RLE pixel using the color table
 */
void SkBmpRLECodec::setPixel(void* dst, size_t dstRowBytes,
                             const SkImageInfo& dstInfo, uint32_t x, uint32_t y,
                             uint8_t index) {
    // Set the row
    int height = dstInfo.height();
    int row;
    if (SkBmpCodec::kBottomUp_RowOrder == this->rowOrder()) {
        row = height - y - 1;
    } else {
        row = y;
    }

    // Set the pixel based on destination color type
    switch (dstInfo.colorType()) {
        case kN32_SkColorType: {
            SkPMColor* dstRow = SkTAddOffset<SkPMColor>((SkPMColor*) dst,
                    row * (int) dstRowBytes);
            dstRow[x] = fColorTable->operator[](index);
            break;
        }
        case kRGB_565_SkColorType: {
            uint16_t* dstRow = SkTAddOffset<uint16_t>(dst, row * (int) dstRowBytes);
            dstRow[x] = SkPixel32ToPixel16(fColorTable->operator[](index));
            break;
        }
        default:
            // This case should not be reached.  We should catch an invalid
            // color type when we check that the conversion is possible.
            SkASSERT(false);
            break;
    }
}

/*
 * Set an RLE pixel from R, G, B values
 */
void SkBmpRLECodec::setRGBPixel(void* dst, size_t dstRowBytes,
                                const SkImageInfo& dstInfo, uint32_t x,
                                uint32_t y, uint8_t red, uint8_t green,
                                uint8_t blue) {
    // Set the row
    int height = dstInfo.height();
    int row;
    if (SkBmpCodec::kBottomUp_RowOrder == this->rowOrder()) {
        row = height - y - 1;
    } else {
        row = y;
    }

    // Set the pixel based on destination color type
    switch (dstInfo.colorType()) {
        case kN32_SkColorType: {
            SkPMColor* dstRow = SkTAddOffset<SkPMColor>((SkPMColor*) dst,
                    row * (int) dstRowBytes);
            dstRow[x] = SkPackARGB32NoCheck(0xFF, red, green, blue);
            break;
        }
        case kRGB_565_SkColorType: {
            uint16_t* dstRow = SkTAddOffset<uint16_t>(dst, row * (int) dstRowBytes);
            dstRow[x] = SkPack888ToRGB16(red, green, blue);
            break;
        }
        default:
            // This case should not be reached.  We should catch an invalid
            // color type when we check that the conversion is possible.
            SkASSERT(false);
            break;
    }
}

/*
 * Performs the bitmap decoding for RLE input format
 * RLE decoding is performed all at once, rather than a one row at a time
 */
SkCodec::Result SkBmpRLECodec::decode(const SkImageInfo& dstInfo,
                                      void* dst, size_t dstRowBytes,
                                      const Options& opts) {
    // Set RLE flags
    static const uint8_t RLE_ESCAPE = 0;
    static const uint8_t RLE_EOL = 0;
    static const uint8_t RLE_EOF = 1;
    static const uint8_t RLE_DELTA = 2;

    // Set constant values
    const int width = dstInfo.width();
    const int height = dstInfo.height();

    // Destination parameters
    int x = 0;
    int y = 0;

    // Set the background as transparent.  Then, if the RLE code skips pixels,
    // the skipped pixels will be transparent.
    // Because of the need for transparent pixels, kN32 is the only color
    // type that makes sense for the destination format.
    SkASSERT(kN32_SkColorType == dstInfo.colorType());
    if (kNo_ZeroInitialized == opts.fZeroInitialized) {
        SkSwizzler::Fill(dst, dstInfo, dstRowBytes, height, SK_ColorTRANSPARENT, NULL);
    }

    while (true) {
        // If we have reached a row that is beyond the requested height, we have
        // succeeded.
        if (y >= height) {
            // It would be better to check for the EOF marker before returning
            // success, but we may be performing a scanline decode, which
            // may require us to stop before decoding the full height.
            return kSuccess;
        }

        // Every entry takes at least two bytes
        if ((int) fRLEBytes - fCurrRLEByte < 2) {
            SkCodecPrintf("Warning: might be incomplete RLE input.\n");
            if (this->checkForMoreData() < 2) {
                return kIncompleteInput;
            }
        }

        // Read the next two bytes.  These bytes have different meanings
        // depending on their values.  In the first interpretation, the first
        // byte is an escape flag and the second byte indicates what special
        // task to perform.
        const uint8_t flag = fStreamBuffer.get()[fCurrRLEByte++];
        const uint8_t task = fStreamBuffer.get()[fCurrRLEByte++];

        // Perform decoding
        if (RLE_ESCAPE == flag) {
            switch (task) {
                case RLE_EOL:
                    x = 0;
                    y++;
                    break;
                case RLE_EOF:
                    return kSuccess;
                case RLE_DELTA: {
                    // Two bytes are needed to specify delta
                    if ((int) fRLEBytes - fCurrRLEByte < 2) {
                        SkCodecPrintf("Warning: might be incomplete RLE input.\n");
                        if (this->checkForMoreData() < 2) {
                            return kIncompleteInput;
                        }
                    }
                    // Modify x and y
                    const uint8_t dx = fStreamBuffer.get()[fCurrRLEByte++];
                    const uint8_t dy = fStreamBuffer.get()[fCurrRLEByte++];
                    x += dx;
                    y += dy;
                    if (x > width || y > height) {
                        SkCodecPrintf("Warning: invalid RLE input.\n");
                        return kInvalidInput;
                    }
                    break;
                }
                default: {
                    // If task does not match any of the above signals, it
                    // indicates that we have a sequence of non-RLE pixels.
                    // Furthermore, the value of task is equal to the number
                    // of pixels to interpret.
                    uint8_t numPixels = task;
                    const size_t rowBytes = compute_row_bytes(numPixels,
                            this->bitsPerPixel());
                    // Abort if setting numPixels moves us off the edge of the
                    // image.
                    if (x + numPixels > width) {
                        SkCodecPrintf("Warning: invalid RLE input.\n");
                        return kInvalidInput;
                    }
                    // Also abort if there are not enough bytes
                    // remaining in the stream to set numPixels.
                    if ((int) fRLEBytes - fCurrRLEByte < SkAlign2(rowBytes)) {
                        SkCodecPrintf("Warning: might be incomplete RLE input.\n");
                        if (this->checkForMoreData() < SkAlign2(rowBytes)) {
                            return kIncompleteInput;
                        }
                    }
                    // Set numPixels number of pixels
                    while (numPixels > 0) {
                        switch(this->bitsPerPixel()) {
                            case 4: {
                                SkASSERT(fCurrRLEByte < fRLEBytes);
                                uint8_t val = fStreamBuffer.get()[fCurrRLEByte++];
                                setPixel(dst, dstRowBytes, dstInfo, x++,
                                        y, val >> 4);
                                numPixels--;
                                if (numPixels != 0) {
                                    setPixel(dst, dstRowBytes, dstInfo,
                                            x++, y, val & 0xF);
                                    numPixels--;
                                }
                                break;
                            }
                            case 8:
                                SkASSERT(fCurrRLEByte < fRLEBytes);
                                setPixel(dst, dstRowBytes, dstInfo, x++,
                                        y, fStreamBuffer.get()[fCurrRLEByte++]);
                                numPixels--;
                                break;
                            case 24: {
                                SkASSERT(fCurrRLEByte + 2 < fRLEBytes);
                                uint8_t blue = fStreamBuffer.get()[fCurrRLEByte++];
                                uint8_t green = fStreamBuffer.get()[fCurrRLEByte++];
                                uint8_t red = fStreamBuffer.get()[fCurrRLEByte++];
                                setRGBPixel(dst, dstRowBytes, dstInfo,
                                            x++, y, red, green, blue);
                                numPixels--;
                            }
                            default:
                                SkASSERT(false);
                                return kInvalidInput;
                        }
                    }
                    // Skip a byte if necessary to maintain alignment
                    if (!SkIsAlign2(rowBytes)) {
                        fCurrRLEByte++;
                    }
                    break;
                }
            }
        } else {
            // If the first byte read is not a flag, it indicates the number of
            // pixels to set in RLE mode.
            const uint8_t numPixels = flag;
            const int endX = SkTMin<int>(x + numPixels, width);

            if (24 == this->bitsPerPixel()) {
                // In RLE24, the second byte read is part of the pixel color.
                // There are two more required bytes to finish encoding the
                // color.
                if ((int) fRLEBytes - fCurrRLEByte < 2) {
                    SkCodecPrintf("Warning: might be incomplete RLE input.\n");
                    if (this->checkForMoreData() < 2) {
                        return kIncompleteInput;
                    }
                }

                // Fill the pixels up to endX with the specified color
                uint8_t blue = task;
                uint8_t green = fStreamBuffer.get()[fCurrRLEByte++];
                uint8_t red = fStreamBuffer.get()[fCurrRLEByte++];
                while (x < endX) {
                    setRGBPixel(dst, dstRowBytes, dstInfo, x++, y, red,
                            green, blue);
                }
            } else {
                // In RLE8 or RLE4, the second byte read gives the index in the
                // color table to look up the pixel color.
                // RLE8 has one color index that gets repeated
                // RLE4 has two color indexes in the upper and lower 4 bits of
                // the bytes, which are alternated
                uint8_t indices[2] = { task, task };
                if (4 == this->bitsPerPixel()) {
                    indices[0] >>= 4;
                    indices[1] &= 0xf;
                }

                // Set the indicated number of pixels
                for (int which = 0; x < endX; x++) {
                    setPixel(dst, dstRowBytes, dstInfo, x, y,
                            indices[which]);
                    which = !which;
                }
            }
        }
    }
}
