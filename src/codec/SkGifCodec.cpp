/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "SkCodecAnimation.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkColorTable.h"
#include "SkGifCodec.h"
#include "SkStream.h"
#include "SkSwizzler.h"

#include <algorithm>

#define GIF87_STAMP "GIF87a"
#define GIF89_STAMP "GIF89a"
#define GIF_STAMP_LEN 6

/*
 * Checks the start of the stream to see if the image is a gif
 */
bool SkGifCodec::IsGif(const void* buf, size_t bytesRead) {
    if (bytesRead >= GIF_STAMP_LEN) {
        if (memcmp(GIF87_STAMP, buf, GIF_STAMP_LEN) == 0 ||
            memcmp(GIF89_STAMP, buf, GIF_STAMP_LEN) == 0)
        {
            return true;
        }
    }
    return false;
}

/*
 * Error function
 */
static SkCodec::Result gif_error(const char* msg, SkCodec::Result result = SkCodec::kInvalidInput) {
    SkCodecPrintf("Gif Error: %s\n", msg);
    return result;
}

/*
 * Assumes IsGif was called and returned true
 * Creates a gif decoder
 * Reads enough of the stream to determine the image format
 */
SkCodec* SkGifCodec::NewFromStream(SkStream* stream) {
    std::unique_ptr<GIFImageReader> reader(new GIFImageReader(stream));
    if (!reader->parse(GIFImageReader::GIFSizeQuery)) {
        // Not enough data to determine the size.
        return nullptr;
    }

    if (0 == reader->screenWidth() || 0 == reader->screenHeight()) {
        return nullptr;
    }

    const auto alpha = reader->firstFrameHasAlpha() ? SkEncodedInfo::kBinary_Alpha
                                                    : SkEncodedInfo::kOpaque_Alpha;
    // Use kPalette since Gifs are encoded with a color table.
    // FIXME: Gifs can actually be encoded with 4-bits per pixel. Using 8 works, but we could skip
    //        expanding to 8 bits and take advantage of the SkSwizzler to work from 4.
    const auto encodedInfo = SkEncodedInfo::Make(SkEncodedInfo::kPalette_Color, alpha, 8);

    // Although the encodedInfo is always kPalette_Color, it is possible that kIndex_8 is
    // unsupported if the frame is subset and there is no transparent pixel.
    const auto colorType = reader->firstFrameSupportsIndex8() ? kIndex_8_SkColorType
                                                              : kN32_SkColorType;
    // The choice of unpremul versus premul is arbitrary, since all colors are either fully
    // opaque or fully transparent (i.e. kBinary), but we stored the transparent colors as all
    // zeroes, which is arguably premultiplied.
    const auto alphaType = reader->firstFrameHasAlpha() ? kUnpremul_SkAlphaType
                                                        : kOpaque_SkAlphaType;
    // FIXME: GIF should default to SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named).
    const auto imageInfo = SkImageInfo::Make(reader->screenWidth(), reader->screenHeight(),
                                             colorType, alphaType);
    return new SkGifCodec(encodedInfo, imageInfo, reader.release());
}

bool SkGifCodec::onRewind() {
    fReader->clearDecodeState();
    return true;
}

SkGifCodec::SkGifCodec(const SkEncodedInfo& encodedInfo, const SkImageInfo& imageInfo,
                       GIFImageReader* reader)
    : INHERITED(encodedInfo, imageInfo, nullptr)
    , fReader(reader)
    , fTmpBuffer(nullptr)
    , fSwizzler(nullptr)
    , fCurrColorTable(nullptr)
    , fCurrColorTableIsReal(false)
    , fFilledBackground(false)
    , fFirstCallToIncrementalDecode(false)
    , fDst(nullptr)
    , fDstRowBytes(0)
    , fRowsDecoded(0)
{
    reader->setClient(this);
}

std::vector<SkCodec::FrameInfo> SkGifCodec::onGetFrameInfo() {
    fReader->parse(GIFImageReader::GIFFrameCountQuery);
    const size_t size = fReader->imagesCount();
    std::vector<FrameInfo> result(size);
    for (size_t i = 0; i < size; i++) {
        const GIFFrameContext* frameContext = fReader->frameContext(i);
        result[i].fDuration = frameContext->delayTime();
        result[i].fRequiredFrame = frameContext->getRequiredFrame();
    }
    return result;
}

void SkGifCodec::initializeColorTable(const SkImageInfo& dstInfo, size_t frameIndex,
        SkPMColor* inputColorPtr, int* inputColorCount) {
    fCurrColorTable = fReader->getColorTable(dstInfo.colorType(), frameIndex);
    fCurrColorTableIsReal = fCurrColorTable;
    if (!fCurrColorTable) {
        // This is possible for an empty frame. Create a dummy with one value (transparent).
        SkPMColor color = SK_ColorTRANSPARENT;
        fCurrColorTable.reset(new SkColorTable(&color, 1));
    }

    if (inputColorCount) {
        *inputColorCount = fCurrColorTable->count();
    }

    copy_color_table(dstInfo, fCurrColorTable.get(), inputColorPtr, inputColorCount);
}


SkCodec::Result SkGifCodec::prepareToDecode(const SkImageInfo& dstInfo, SkPMColor* inputColorPtr,
        int* inputColorCount, const Options& opts) {
    // Check for valid input parameters
    if (!conversion_possible_ignore_color_space(dstInfo, this->getInfo())) {
        return gif_error("Cannot convert input type to output type.\n", kInvalidConversion);
    }

    if (dstInfo.colorType() == kRGBA_F16_SkColorType) {
        // FIXME: This should be supported.
        return gif_error("GIF does not yet support F16.\n", kInvalidConversion);
    }

    if (opts.fSubset) {
        return gif_error("Subsets not supported.\n", kUnimplemented);
    }

    const size_t frameIndex = opts.fFrameIndex;
    if (frameIndex > 0 && dstInfo.colorType() == kIndex_8_SkColorType) {
        // FIXME: It is possible that a later frame can be decoded to index8, if it does one of the
        // following:
        // - Covers the entire previous frame
        // - Shares a color table (and transparent index) with any prior frames that are showing.
        // We must support index8 for the first frame to be backwards compatible on Android, but
        // we do not (currently) need to support later frames as index8.
        return gif_error("Cannot decode multiframe gif (except frame 0) as index 8.\n",
                         kInvalidConversion);
    }

    fReader->parse((GIFImageReader::GIFParseQuery) frameIndex);

    if (frameIndex >= fReader->imagesCount()) {
        return gif_error("frame index out of range!\n", kIncompleteInput);
    }

    fTmpBuffer.reset(new uint8_t[dstInfo.minRowBytes()]);

    // Initialize color table and copy to the client if necessary
    this->initializeColorTable(dstInfo, frameIndex, inputColorPtr, inputColorCount);
    this->initializeSwizzler(dstInfo, frameIndex);
    return kSuccess;
}

void SkGifCodec::initializeSwizzler(const SkImageInfo& dstInfo, size_t frameIndex) {
    const GIFFrameContext* frame = fReader->frameContext(frameIndex);
    // This is only called by prepareToDecode, which ensures frameIndex is in range.
    SkASSERT(frame);

    const int xBegin = frame->xOffset();
    const int xEnd = std::min(static_cast<int>(frame->xOffset() + frame->width()),
                              static_cast<int>(fReader->screenWidth()));

    // CreateSwizzler only reads left and right of the frame. We cannot use the frame's raw
    // frameRect, since it might extend beyond the edge of the frame.
    SkIRect swizzleRect = SkIRect::MakeLTRB(xBegin, 0, xEnd, 0);

    // The default Options should be fine:
    // - we'll ignore if the memory is zero initialized - unless we're the first frame, this won't
    //   matter anyway.
    // - subsets are not supported for gif
    // - the swizzler does not need to know about the frame.
    // We may not be able to use the real Options anyway, since getPixels does not store it (due to
    // a bug).
    fSwizzler.reset(SkSwizzler::CreateSwizzler(this->getEncodedInfo(),
                    fCurrColorTable->readColors(), dstInfo, Options(), &swizzleRect));
    SkASSERT(fSwizzler.get());
}

/*
 * Initiates the gif decode
 */
SkCodec::Result SkGifCodec::onGetPixels(const SkImageInfo& dstInfo,
                                        void* pixels, size_t dstRowBytes,
                                        const Options& opts,
                                        SkPMColor* inputColorPtr,
                                        int* inputColorCount,
                                        int* rowsDecoded) {
    Result result = this->prepareToDecode(dstInfo, inputColorPtr, inputColorCount, opts);
    if (kSuccess != result) {
        return result;
    }

    if (dstInfo.dimensions() != this->getInfo().dimensions()) {
        return gif_error("Scaling not supported.\n", kInvalidScale);
    }

    fDst = pixels;
    fDstRowBytes = dstRowBytes;

    return this->decodeFrame(true, opts, rowsDecoded);
}

SkCodec::Result SkGifCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                     void* pixels, size_t dstRowBytes,
                                                     const SkCodec::Options& opts,
                                                     SkPMColor* inputColorPtr,
                                                     int* inputColorCount) {
    Result result = this->prepareToDecode(dstInfo, inputColorPtr, inputColorCount, opts);
    if (result != kSuccess) {
        return result;
    }

    fDst = pixels;
    fDstRowBytes = dstRowBytes;

    fFirstCallToIncrementalDecode = true;

    return kSuccess;
}

SkCodec::Result SkGifCodec::onIncrementalDecode(int* rowsDecoded) {
    // It is possible the client has appended more data. Parse, if needed.
    const auto& options = this->options();
    const size_t frameIndex = options.fFrameIndex;
    fReader->parse((GIFImageReader::GIFParseQuery) frameIndex);

    const bool firstCallToIncrementalDecode = fFirstCallToIncrementalDecode;
    fFirstCallToIncrementalDecode = false;
    return this->decodeFrame(firstCallToIncrementalDecode, options, rowsDecoded);
}

SkCodec::Result SkGifCodec::decodeFrame(bool firstAttempt, const Options& opts, int* rowsDecoded) {
    const SkImageInfo& dstInfo = this->dstInfo();
    const size_t frameIndex = opts.fFrameIndex;
    SkASSERT(frameIndex < fReader->imagesCount());
    const GIFFrameContext* frameContext = fReader->frameContext(frameIndex);
    if (firstAttempt) {
        // rowsDecoded reports how many rows have been initialized, so a layer above
        // can fill the rest. In some cases, we fill the background before decoding
        // (or it is already filled for us), so we report rowsDecoded to be the full
        // height.
        bool filledBackground = false;
        if (frameContext->getRequiredFrame() == kNone) {
            // We may need to clear to transparent for one of the following reasons:
            // - The frameRect does not cover the full bounds. haveDecodedRow will
            //   only draw inside the frameRect, so we need to clear the rest.
            // - There is a valid transparent pixel value. (FIXME: I'm assuming
            //   writeTransparentPixels will be false in this case, based on
            //   Chromium's assumption that it would already be zeroed. If we
            //   change that behavior, could we skip Filling here?)
            // - The frame is interlaced. There is no obvious way to fill
            //   afterwards for an incomplete image. (FIXME: Does the first pass
            //   cover all rows? If so, we do not have to fill here.)
            if (frameContext->frameRect() != this->getInfo().bounds()
                    || frameContext->transparentPixel() < MAX_COLORS
                    || frameContext->interlaced()) {
                // fill ignores the width (replaces it with the actual, scaled width).
                // But we need to scale in Y.
                const int scaledHeight = get_scaled_dimension(dstInfo.height(),
                                                              fSwizzler->sampleY());
                auto fillInfo = dstInfo.makeWH(0, scaledHeight);
                fSwizzler->fill(fillInfo, fDst, fDstRowBytes, this->getFillValue(dstInfo),
                                opts.fZeroInitialized);
                filledBackground = true;
            }
        } else {
            // Not independent
            if (!opts.fHasPriorFrame) {
                // Decode that frame into pixels.
                Options prevFrameOpts(opts);
                prevFrameOpts.fFrameIndex = frameContext->getRequiredFrame();
                prevFrameOpts.fHasPriorFrame = false;
                const Result prevResult = this->decodeFrame(true, prevFrameOpts, nullptr);
                switch (prevResult) {
                    case kSuccess:
                        // Prior frame succeeded. Carry on.
                        break;
                    case kIncompleteInput:
                        // Prior frame was incomplete. So this frame cannot be decoded.
                        return kInvalidInput;
                    default:
                        return prevResult;
                }
            }
            const auto* prevFrame = fReader->frameContext(frameContext->getRequiredFrame());
            if (prevFrame->getDisposalMethod() == SkCodecAnimation::RestoreBGColor_DisposalMethod) {
                const SkIRect prevRect = prevFrame->frameRect();
                auto left = get_scaled_dimension(prevRect.fLeft, fSwizzler->sampleX());
                auto top = get_scaled_dimension(prevRect.fTop, fSwizzler->sampleY());
                void* const eraseDst = SkTAddOffset<void>(fDst, top * fDstRowBytes
                        + left * SkColorTypeBytesPerPixel(dstInfo.colorType()));
                auto width = get_scaled_dimension(prevRect.width(), fSwizzler->sampleX());
                auto height = get_scaled_dimension(prevRect.height(), fSwizzler->sampleY());
                // fSwizzler->fill() would fill to the scaled width of the frame, but we want to
                // fill to the scaled with of the width of the PRIOR frame, so we do all the scaling
                // ourselves and call the static version.
                SkSampler::Fill(dstInfo.makeWH(width, height), eraseDst,
                                fDstRowBytes, this->getFillValue(dstInfo), kNo_ZeroInitialized);
            }
            filledBackground = true;
        }

        fFilledBackground = filledBackground;
        if (filledBackground) {
            // Report the full (scaled) height, since the client will never need to fill.
            fRowsDecoded = get_scaled_dimension(dstInfo.height(), fSwizzler->sampleY());
        } else {
            // This will be updated by haveDecodedRow.
            fRowsDecoded = 0;
        }
    }

    // Note: there is a difference between the following call to GIFImageReader::decode
    // returning false and leaving frameDecoded false:
    // - If the method returns false, there was an error in the stream. We still treat this as
    //   incomplete, since we have already decoded some rows.
    // - If frameDecoded is false, that just means that we do not have enough data. If more data
    //   is supplied, we may be able to continue decoding this frame. We also treat this as
    //   incomplete.
    // FIXME: Ensure that we do not attempt to continue decoding if the method returns false and
    // more data is supplied.
    bool frameDecoded = false;
    if (!fReader->decode(frameIndex, &frameDecoded) || !frameDecoded) {
        if (rowsDecoded) {
            *rowsDecoded = fRowsDecoded;
        }
        return kIncompleteInput;
    }

    return kSuccess;
}

uint64_t SkGifCodec::onGetFillValue(const SkImageInfo& dstInfo) const {
    // Note: Using fCurrColorTable relies on having called initializeColorTable already.
    // This is (currently) safe because this method is only called when filling, after
    // initializeColorTable has been called.
    // FIXME: Is there a way to make this less fragile?
    if (dstInfo.colorType() == kIndex_8_SkColorType && fCurrColorTableIsReal) {
        // We only support index 8 for the first frame, for backwards
        // compatibity on Android, so we are using the color table for the first frame.
        SkASSERT(this->options().fFrameIndex == 0);
        // Use the transparent index for the first frame.
        const size_t transPixel = fReader->frameContext(0)->transparentPixel();
        if (transPixel < (size_t) fCurrColorTable->count()) {
            return transPixel;
        }
        // Fall through to return SK_ColorTRANSPARENT (i.e. 0). This choice is arbitrary,
        // but we have to pick something inside the color table, and this one is as good
        // as any.
    }
    // Using transparent as the fill value matches the behavior in Chromium,
    // which ignores the background color.
    // If the colorType is kIndex_8, and there was no color table (i.e.
    // fCurrColorTableIsReal is false), this value (zero) corresponds to the
    // only entry in the dummy color table provided to the client.
    return SK_ColorTRANSPARENT;
}

bool SkGifCodec::haveDecodedRow(size_t frameIndex, const unsigned char* rowBegin,
                                size_t rowNumber, unsigned repeatCount, bool writeTransparentPixels)
{
    const GIFFrameContext* frameContext = fReader->frameContext(frameIndex);
    // The pixel data and coordinates supplied to us are relative to the frame's
    // origin within the entire image size, i.e.
    // (frameContext->xOffset, frameContext->yOffset). There is no guarantee
    // that width == (size().width() - frameContext->xOffset), so
    // we must ensure we don't run off the end of either the source data or the
    // row's X-coordinates.
    const size_t width = frameContext->width();
    const int xBegin = frameContext->xOffset();
    const int yBegin = frameContext->yOffset() + rowNumber;
    const int xEnd = std::min(static_cast<int>(frameContext->xOffset() + width),
                              this->getInfo().width());
    const int yEnd = std::min(static_cast<int>(frameContext->yOffset() + rowNumber + repeatCount),
                              this->getInfo().height());
    // FIXME: No need to make the checks on width/xBegin/xEnd for every row. We could instead do
    // this once in prepareToDecode.
    if (!width || (xBegin < 0) || (yBegin < 0) || (xEnd <= xBegin) || (yEnd <= yBegin))
        return true;

    // yBegin is the first row in the non-sampled image. dstRow will be the row in the output,
    // after potentially scaling it.
    int dstRow = yBegin;

    const int sampleY = fSwizzler->sampleY();
    if (sampleY > 1) {
        // Check to see whether this row or one that falls in the repeatCount is needed in the
        // output.
        bool foundNecessaryRow = false;
        for (unsigned i = 0; i < repeatCount; i++) {
            const int potentialRow = yBegin + i;
            if (fSwizzler->rowNeeded(potentialRow)) {
                dstRow = potentialRow / sampleY;
                const int scaledHeight = get_scaled_dimension(this->dstInfo().height(), sampleY);
                if (dstRow >= scaledHeight) {
                    return true;
                }

                foundNecessaryRow = true;
                repeatCount -= i;

                repeatCount = (repeatCount - 1) / sampleY + 1;

                // Make sure the repeatCount does not take us beyond the end of the dst
                if (dstRow + (int) repeatCount > scaledHeight) {
                    repeatCount = scaledHeight - dstRow;
                    SkASSERT(repeatCount >= 1);
                }
                break;
            }
        }

        if (!foundNecessaryRow) {
            return true;
        }
    }

    if (!fFilledBackground) {
        // At this point, we are definitely going to write the row, so count it towards the number
        // of rows decoded.
        // We do not consider the repeatCount, which only happens for interlaced, in which case we
        // have already set fRowsDecoded to the proper value (reflecting that we have filled the
        // background).
        fRowsDecoded++;
    }

    if (!fCurrColorTableIsReal) {
        // No color table, so nothing to draw this frame.
        // FIXME: We can abort even earlier - no need to decode this frame.
        return true;
    }

    // The swizzler takes care of offsetting into the dst width-wise.
    void* dstLine = SkTAddOffset<void>(fDst, dstRow * fDstRowBytes);

    // We may or may not need to write transparent pixels to the buffer.
    // If we're compositing against a previous image, it's wrong, and if
    // we're writing atop a cleared, fully transparent buffer, it's
    // unnecessary; but if we're decoding an interlaced gif and
    // displaying it "Haeberli"-style, we must write these for passes
    // beyond the first, or the initial passes will "show through" the
    // later ones.
    const auto dstInfo = this->dstInfo();
    if (writeTransparentPixels || dstInfo.colorType() == kRGB_565_SkColorType) {
        fSwizzler->swizzle(dstLine, rowBegin);
    } else {
        // We cannot swizzle directly into the dst, since that will write the transparent pixels.
        // Instead, swizzle into a temporary buffer, and copy that into the dst.
        {
            void* const memsetDst = fTmpBuffer.get();
            // Although onGetFillValue returns a uint64_t, we only use the low eight bits. The
            // return value is either an 8 bit index (for index8) or SK_ColorTRANSPARENT, which is
            // all zeroes.
            const int fillValue = (uint8_t) this->onGetFillValue(dstInfo);
            const size_t rb = dstInfo.minRowBytes();
            if (fillValue == 0) {
                // FIXME: This special case should be unnecessary, and in fact sk_bzero just calls
                // memset. But without it, the compiler thinks this is trying to pass a zero length
                // to memset, causing an error.
                sk_bzero(memsetDst, rb);
            } else {
                memset(memsetDst, fillValue, rb);
            }
        }
        fSwizzler->swizzle(fTmpBuffer.get(), rowBegin);

        const size_t offsetBytes = fSwizzler->swizzleOffsetBytes();
        switch (dstInfo.colorType()) {
            case kBGRA_8888_SkColorType:
            case kRGBA_8888_SkColorType: {
                uint32_t* dstPixel = SkTAddOffset<uint32_t>(dstLine, offsetBytes);
                uint32_t* srcPixel = SkTAddOffset<uint32_t>(fTmpBuffer.get(), offsetBytes);
                for (int i = 0; i < fSwizzler->swizzleWidth(); i++) {
                    // Technically SK_ColorTRANSPARENT is an SkPMColor, and srcPixel would have
                    // the opposite swizzle for the non-native swizzle, but TRANSPARENT is all
                    // zeroes, which is the same either way.
                    if (*srcPixel != SK_ColorTRANSPARENT) {
                        *dstPixel = *srcPixel;
                    }
                    dstPixel++;
                    srcPixel++;
                }
                break;
            }
            case kIndex_8_SkColorType: {
                uint8_t* dstPixel = SkTAddOffset<uint8_t>(dstLine, offsetBytes);
                uint8_t* srcPixel = SkTAddOffset<uint8_t>(fTmpBuffer.get(), offsetBytes);
                for (int i = 0; i < fSwizzler->swizzleWidth(); i++) {
                    if (*srcPixel != frameContext->transparentPixel()) {
                        *dstPixel = *srcPixel;
                    }
                    dstPixel++;
                    srcPixel++;
                }
                break;
            }
            default:
                SkASSERT(false);
                break;
        }
    }

    // Tell the frame to copy the row data if need be.
    if (repeatCount > 1) {
        const size_t bytesPerPixel = SkColorTypeBytesPerPixel(this->dstInfo().colorType());
        const size_t bytesToCopy = fSwizzler->swizzleWidth() * bytesPerPixel;
        void* copiedLine = SkTAddOffset<void>(dstLine, fSwizzler->swizzleOffsetBytes());
        void* dst = copiedLine;
        for (unsigned i = 1; i < repeatCount; i++) {
            dst = SkTAddOffset<void>(dst, fDstRowBytes);
            memcpy(dst, copiedLine, bytesToCopy);
        }
    }

    return true;
}
