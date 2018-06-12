/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFBitmap.h"

#include "SkColorData.h"
#include "SkData.h"
#include "SkDeflate.h"
#include "SkImage.h"
#include "SkJpegInfo.h"
#include "SkPDFCanon.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkTo.h"
#include "SkUnPreMultiply.h"

bool image_compute_is_opaque(const SkImage* image) {
    if (image->isOpaque()) {
        return true;
    }
    // keep output PDF small at cost of possible resource use.
    SkBitmap bm;
    // if image can not be read, treat as transparent.
    return SkPDFUtils::ToBitmap(image, &bm) && SkBitmap::ComputeIsOpaque(bm);
}

////////////////////////////////////////////////////////////////////////////////

static const char kStreamBegin[] = " stream\n";

static const char kStreamEnd[] = "\nendstream";

////////////////////////////////////////////////////////////////////////////////

// write a single byte to a stream n times.
static void fill_stream(SkWStream* out, char value, size_t n) {
    char buffer[4096];
    memset(buffer, value, sizeof(buffer));
    for (size_t i = 0; i < n / sizeof(buffer); ++i) {
        out->write(buffer, sizeof(buffer));
    }
    out->write(buffer, n % sizeof(buffer));
}

// TODO(reed@): Decide if these five functions belong in SkColorData.h
static bool SkIsBGRA(SkColorType ct) {
    SkASSERT(kBGRA_8888_SkColorType == ct || kRGBA_8888_SkColorType == ct);
    return kBGRA_8888_SkColorType == ct;
}

// Interpret value as the given 4-byte SkColorType (BGRA_8888 or
// RGBA_8888) and return the appropriate component.  Each component
// should be interpreted according to the associated SkAlphaType and
// SkColorProfileType.
static U8CPU SkGetA32Component(uint32_t value, SkColorType ct) {
    return (value >> (SkIsBGRA(ct) ? SK_BGRA_A32_SHIFT : SK_RGBA_A32_SHIFT)) & 0xFF;
}
static U8CPU SkGetR32Component(uint32_t value, SkColorType ct) {
    return (value >> (SkIsBGRA(ct) ? SK_BGRA_R32_SHIFT : SK_RGBA_R32_SHIFT)) & 0xFF;
}
static U8CPU SkGetG32Component(uint32_t value, SkColorType ct) {
    return (value >> (SkIsBGRA(ct) ? SK_BGRA_G32_SHIFT : SK_RGBA_G32_SHIFT)) & 0xFF;
}
static U8CPU SkGetB32Component(uint32_t value, SkColorType ct) {
    return (value >> (SkIsBGRA(ct) ? SK_BGRA_B32_SHIFT : SK_RGBA_B32_SHIFT)) & 0xFF;
}

// unpremultiply and extract R, G, B components.
static void pmcolor_to_rgb24(uint32_t color, uint8_t* rgb, SkColorType ct) {
    SkPMColorAssert(color);
    uint32_t s = SkUnPreMultiply::GetScale(SkGetA32Component(color, ct));
    rgb[0] = SkUnPreMultiply::ApplyScale(s, SkGetR32Component(color, ct));
    rgb[1] = SkUnPreMultiply::ApplyScale(s, SkGetG32Component(color, ct));
    rgb[2] = SkUnPreMultiply::ApplyScale(s, SkGetB32Component(color, ct));
}

/* It is necessary to average the color component of transparent
   pixels with their surrounding neighbors since the PDF renderer may
   separately re-sample the alpha and color channels when the image is
   not displayed at its native resolution. Since an alpha of zero
   gives no information about the color component, the pathological
   case is a white image with sharp transparency bounds - the color
   channel goes to black, and the should-be-transparent pixels are
   rendered as grey because of the separate soft mask and color
   resizing. e.g.: gm/bitmappremul.cpp */
static void get_neighbor_avg_color(const SkBitmap& bm,
                                   int xOrig,
                                   int yOrig,
                                   uint8_t rgb[3],
                                   SkColorType ct) {
    unsigned a = 0, r = 0, g = 0, b = 0;
    // Clamp the range to the edge of the bitmap.
    int ymin = SkTMax(0, yOrig - 1);
    int ymax = SkTMin(yOrig + 1, bm.height() - 1);
    int xmin = SkTMax(0, xOrig - 1);
    int xmax = SkTMin(xOrig + 1, bm.width() - 1);
    for (int y = ymin; y <= ymax; ++y) {
        uint32_t* scanline = bm.getAddr32(0, y);
        for (int x = xmin; x <= xmax; ++x) {
            uint32_t color = scanline[x];
            SkPMColorAssert(color);
            a += SkGetA32Component(color, ct);
            r += SkGetR32Component(color, ct);
            g += SkGetG32Component(color, ct);
            b += SkGetB32Component(color, ct);
        }
    }
    if (a > 0) {
        rgb[0] = SkToU8(255 * r / a);
        rgb[1] = SkToU8(255 * g / a);
        rgb[2] = SkToU8(255 * b / a);
    } else {
        rgb[0] = rgb[1] = rgb[2] = 0;
    }
}

static size_t pixel_count(const SkBitmap& bm) {
    return SkToSizeT(bm.width()) * SkToSizeT(bm.height());
}

static const SkBitmap& supported_colortype(const SkBitmap& input, SkBitmap* copy) {
    switch (input.colorType()) {
        case kUnknown_SkColorType:
            SkDEBUGFAIL("kUnknown_SkColorType");
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kGray_8_SkColorType:
            return input;  // supported
        default:
            // if other colortypes are introduced in the future,
            // they will hit this code.
            break;
    }
    // Fallback for rarely used ARGB_4444 and ARGB_F16: do a wasteful tmp copy.
    copy->allocPixels(input.info().makeColorType(kN32_SkColorType));
    SkAssertResult(input.readPixels(copy->info(), copy->getPixels(), copy->rowBytes(), 0, 0));
    copy->setImmutable();
    return *copy;
}

static size_t pdf_color_component_count(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:
            SkDEBUGFAIL("kUnknown_SkColorType");
        case kAlpha_8_SkColorType:
        case kGray_8_SkColorType:
            return 1;
        case kRGB_565_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        default:  // converted to N32
            return 3;
    }
}

static void bitmap_to_pdf_pixels(const SkBitmap& bitmap, SkWStream* out) {
    if (!bitmap.getPixels()) {
        size_t size = pixel_count(bitmap) *
                      pdf_color_component_count(bitmap.colorType());
        fill_stream(out, '\x00', size);
        return;
    }
    SkBitmap copy;
    const SkBitmap& bm = supported_colortype(bitmap, &copy);
    SkColorType colorType = bm.colorType();
    SkAlphaType alphaType = bm.alphaType();
    switch (colorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType: {
            SkASSERT(3 == pdf_color_component_count(colorType));
            SkAutoTMalloc<uint8_t> scanline(3 * bm.width());
            for (int y = 0; y < bm.height(); ++y) {
                const uint32_t* src = bm.getAddr32(0, y);
                uint8_t* dst = scanline.get();
                for (int x = 0; x < bm.width(); ++x) {
                    if (alphaType == kPremul_SkAlphaType) {
                        uint32_t color = *src++;
                        U8CPU alpha = SkGetA32Component(color, colorType);
                        if (alpha != SK_AlphaTRANSPARENT) {
                            pmcolor_to_rgb24(color, dst, colorType);
                        } else {
                            get_neighbor_avg_color(bm, x, y, dst, colorType);
                        }
                        dst += 3;
                    } else {
                        uint32_t color = *src++;
                        *dst++ = SkGetR32Component(color, colorType);
                        *dst++ = SkGetG32Component(color, colorType);
                        *dst++ = SkGetB32Component(color, colorType);
                    }
                }
                out->write(scanline.get(), 3 * bm.width());
            }
            return;
        }
        case kRGB_565_SkColorType: {
            SkASSERT(3 == pdf_color_component_count(colorType));
            SkAutoTMalloc<uint8_t> scanline(3 * bm.width());
            for (int y = 0; y < bm.height(); ++y) {
                const uint16_t* src = bm.getAddr16(0, y);
                uint8_t* dst = scanline.get();
                for (int x = 0; x < bm.width(); ++x) {
                    U16CPU color565 = *src++;
                    *dst++ = SkPacked16ToR32(color565);
                    *dst++ = SkPacked16ToG32(color565);
                    *dst++ = SkPacked16ToB32(color565);
                }
                out->write(scanline.get(), 3 * bm.width());
            }
            return;
        }
        case kAlpha_8_SkColorType:
            SkASSERT(1 == pdf_color_component_count(colorType));
            fill_stream(out, '\x00', pixel_count(bm));
            return;
        case kGray_8_SkColorType:
            SkASSERT(1 == pdf_color_component_count(colorType));
            // these two formats need no transformation to serialize.
            for (int y = 0; y < bm.height(); ++y) {
                out->write(bm.getAddr8(0, y), bm.width());
            }
            return;
        case kUnknown_SkColorType:
        case kARGB_4444_SkColorType:
        default:
            SkDEBUGFAIL("unexpected color type");
    }
}

////////////////////////////////////////////////////////////////////////////////

static void bitmap_alpha_to_a8(const SkBitmap& bitmap, SkWStream* out) {
    if (!bitmap.getPixels()) {
        fill_stream(out, '\xFF', pixel_count(bitmap));
        return;
    }
    SkBitmap copy;
    const SkBitmap& bm = supported_colortype(bitmap, &copy);
    SkColorType colorType = bm.colorType();
    switch (colorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType: {
            SkAutoTMalloc<uint8_t> scanline(bm.width());
            for (int y = 0; y < bm.height(); ++y) {
                uint8_t* dst = scanline.get();
                const SkPMColor* src = bm.getAddr32(0, y);
                for (int x = 0; x < bm.width(); ++x) {
                    *dst++ = SkGetA32Component(*src++, colorType);
                }
                out->write(scanline.get(), bm.width());
            }
            return;
        }
        case kAlpha_8_SkColorType:
            for (int y = 0; y < bm.height(); ++y) {
                out->write(bm.getAddr8(0, y), bm.width());
            }
            return;
        case kRGB_565_SkColorType:
        case kGray_8_SkColorType:
            SkDEBUGFAIL("color type has no alpha");
            return;
        case kARGB_4444_SkColorType:
            SkDEBUGFAIL("4444 color type should have been converted to N32");
            return;
        case kUnknown_SkColorType:
        default:
            SkDEBUGFAIL("unexpected color type");
    }
}

static void emit_image_xobject(SkWStream* stream,
                               const SkImage* image,
                               bool alpha,
                               const sk_sp<SkPDFObject>& smask,
                               const SkPDFObjNumMap& objNumMap) {
    SkBitmap bitmap;
    if (!SkPDFUtils::ToBitmap(image, &bitmap)) {
        // no pixels or wrong size: fill with zeros.
        bitmap.setInfo(SkImageInfo::MakeN32(image->width(), image->height(), image->alphaType()));
    }

    // Write to a temporary buffer to get the compressed length.
    SkDynamicMemoryWStream buffer;
    SkDeflateWStream deflateWStream(&buffer);
    if (alpha) {
        bitmap_alpha_to_a8(bitmap, &deflateWStream);
    } else {
        bitmap_to_pdf_pixels(bitmap, &deflateWStream);
    }
    deflateWStream.finalize();  // call before buffer.bytesWritten().

    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", bitmap.width());
    pdfDict.insertInt("Height", bitmap.height());
    if (alpha) {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    } else if (1 == pdf_color_component_count(bitmap.colorType())) {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    } else {
        pdfDict.insertName("ColorSpace", "DeviceRGB");
    }
    if (smask) {
        pdfDict.insertObjRef("SMask", smask);
    }
    pdfDict.insertInt("BitsPerComponent", 8);
    pdfDict.insertName("Filter", "FlateDecode");
    pdfDict.insertInt("Length", buffer.bytesWritten());
    pdfDict.emitObject(stream, objNumMap);

    stream->writeText(kStreamBegin);
    buffer.writeToAndReset(stream);
    stream->writeText(kStreamEnd);
}

////////////////////////////////////////////////////////////////////////////////

namespace {
// This SkPDFObject only outputs the alpha layer of the given bitmap.
class PDFAlphaBitmap final : public SkPDFObject {
public:
    PDFAlphaBitmap(sk_sp<SkImage> image) : fImage(std::move(image)) { SkASSERT(fImage); }
    void emitObject(SkWStream*  stream,
                    const SkPDFObjNumMap& objNumMap) const override {
        SkASSERT(fImage);
        emit_image_xobject(stream, fImage.get(), true, nullptr, objNumMap);
    }
    void drop() override { fImage = nullptr; }

private:
    sk_sp<SkImage> fImage;
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////

namespace {
class PDFDefaultBitmap final : public SkPDFObject {
public:
    void emitObject(SkWStream* stream,
                    const SkPDFObjNumMap& objNumMap) const override {
        SkASSERT(fImage);
        emit_image_xobject(stream, fImage.get(), false, fSMask, objNumMap);
    }
    void addResources(SkPDFObjNumMap* catalog) const override {
        catalog->addObjectRecursively(fSMask.get());
    }
    void drop() override { fImage = nullptr; fSMask = nullptr; }
    PDFDefaultBitmap(sk_sp<SkImage> image, sk_sp<SkPDFObject> smask)
        : fImage(std::move(image)), fSMask(std::move(smask)) { SkASSERT(fImage); }

private:
    sk_sp<SkImage> fImage;
    sk_sp<SkPDFObject> fSMask;
};
}  // namespace

////////////////////////////////////////////////////////////////////////////////

namespace {
/**
 *  This PDFObject assumes that its constructor was handed YUV or
 *  Grayscale JFIF Jpeg-encoded data that can be directly embedded
 *  into a PDF.
 */
class PDFJpegBitmap final : public SkPDFObject {
public:
    SkISize fSize;
    sk_sp<SkData> fData;
    bool fIsYUV;
    PDFJpegBitmap(SkISize size, sk_sp<SkData> data, bool isYUV)
        : fSize(size), fData(std::move(data)), fIsYUV(isYUV) { SkASSERT(fData); }
    void emitObject(SkWStream*, const SkPDFObjNumMap&) const override;
    void drop() override { fData = nullptr; }
};

void PDFJpegBitmap::emitObject(SkWStream* stream,
                               const SkPDFObjNumMap& objNumMap) const {
    SkASSERT(fData);
    SkPDFDict pdfDict("XObject");
    pdfDict.insertName("Subtype", "Image");
    pdfDict.insertInt("Width", fSize.width());
    pdfDict.insertInt("Height", fSize.height());
    if (fIsYUV) {
        pdfDict.insertName("ColorSpace", "DeviceRGB");
    } else {
        pdfDict.insertName("ColorSpace", "DeviceGray");
    }
    pdfDict.insertInt("BitsPerComponent", 8);
    pdfDict.insertName("Filter", "DCTDecode");
    pdfDict.insertInt("ColorTransform", 0);
    pdfDict.insertInt("Length", SkToInt(fData->size()));
    pdfDict.emitObject(stream, objNumMap);
    stream->writeText(kStreamBegin);
    stream->write(fData->data(), fData->size());
    stream->writeText(kStreamEnd);
}
}  // namespace

////////////////////////////////////////////////////////////////////////////////
sk_sp<PDFJpegBitmap> make_jpeg_bitmap(sk_sp<SkData> data, SkISize size) {
    SkISize jpegSize;
    SkEncodedInfo::Color jpegColorType;
    SkEncodedOrigin exifOrientation;
    if (data && SkGetJpegInfo(data->data(), data->size(), &jpegSize,
                              &jpegColorType, &exifOrientation)) {
        bool yuv = jpegColorType == SkEncodedInfo::kYUV_Color;
        bool goodColorType = yuv || jpegColorType == SkEncodedInfo::kGray_Color;
        if (jpegSize == size  // Sanity check.
                && goodColorType
                && kTopLeft_SkEncodedOrigin == exifOrientation) {
            // hold on to data, not image.
            #ifdef SK_PDF_IMAGE_STATS
            gJpegImageObjects.fetch_add(1);
            #endif
            return sk_make_sp<PDFJpegBitmap>(jpegSize, std::move(data), yuv);
        }
    }
    return nullptr;
}

sk_sp<SkPDFObject> SkPDFCreateBitmapObject(sk_sp<SkImage> image, int encodingQuality) {
    SkASSERT(image);
    SkASSERT(encodingQuality >= 0);
    SkISize dimensions = image->dimensions();
    sk_sp<SkData> data = image->refEncodedData();
    if (auto jpeg = make_jpeg_bitmap(std::move(data), dimensions)) {
        return std::move(jpeg);
    }

    const bool isOpaque = image_compute_is_opaque(image.get());

    if (encodingQuality <= 100 && isOpaque) {
        data = image->encodeToData(SkEncodedImageFormat::kJPEG, encodingQuality);
        if (auto jpeg = make_jpeg_bitmap(std::move(data), dimensions)) {
            return std::move(jpeg);
        }
    }

    sk_sp<SkPDFObject> smask;
    if (!isOpaque) {
        smask = sk_make_sp<PDFAlphaBitmap>(image);
    }
    #ifdef SK_PDF_IMAGE_STATS
    gRegularImageObjects.fetch_add(1);
    #endif
    return sk_make_sp<PDFDefaultBitmap>(std::move(image), std::move(smask));
}
