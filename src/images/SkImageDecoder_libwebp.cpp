/*
 * Copyright 2010, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkUtils.h"

// A WebP decoder only, on top of (subset of) libwebp
// For more information on WebP image format, and libwebp library, see:
//   http://code.google.com/speed/webp/
//   http://www.webmproject.org/code/#libwebp_webp_image_decoder_library
//   http://review.webmproject.org/gitweb?p=libwebp.git

#include <stdio.h>
extern "C" {
// If moving libwebp out of skia source tree, path for webp headers must be updated accordingly.
// Here, we enforce using local copy in webp sub-directory.
#include "webp/decode.h"
#include "webp/decode_vp8.h"
}

/* If defined, work around missing padding byte in content generated by webpconv */
#define WEBPCONV_MISSING_PADDING 1

#ifdef ANDROID
#include <cutils/properties.h>

// Key to lookup the size of memory buffer set in system property
static const char KEY_MEM_CAP[] = "ro.media.dec.webp.memcap";
#endif

// this enables timing code to report milliseconds for a decode
//#define TIME_DECODE

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Define VP8 I/O on top of Skia stream

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// An helper to extract a integer (little endian) from byte array. This is
// called only once per decoding, so no real need to optimize it in any way
static uint32_t getint32l(unsigned char *in) {
    int result;
    unsigned char *buffer = (unsigned char*) in;

    if (buffer == NULL) {
        return 0;
    }

    result = buffer[3];
    result = (result << 8) + buffer[2];
    result = (result << 8) + buffer[1];
    result = (result << 8) + buffer[0];

    return result;
}

// Parse headers of RIFF container, and check for valid Webp (VP8) content
// return VP8 chunk content size on success, 0 on error.
static const size_t WEBP_HEADER_SIZE = 20;
static const size_t VP8_HEADER_SIZE = 10;

static uint32_t webp_parse_header(SkStream* stream) {
    unsigned char buffer[WEBP_HEADER_SIZE];
    size_t len;
    uint32_t totalSize;
    uint32_t contentSize;

    // RIFF container for WEBP image should always have:
    // 0  "RIFF" 4-byte tag
    // 4  size of image data (including metadata) starting at offset 8
    // 8  "WEBP" the form-type signature
    // 12 "VP8 " 4-bytes tags, describing the raw video format used
    // 16 size of the raw VP8 image data, starting at offset 20
    // 20 the VP8 bytes
    // First check for RIFF top chunk, consuming only 8 bytes
    len = stream->read(buffer, 8);
    if (len != 8) {
        return 0; // can't read enough
    }
    // Inline magic matching instead of memcmp()
    if (buffer[0] != 'R' || buffer[1] != 'I' || buffer[2] != 'F' || buffer[3] != 'F') {
        return 0;
    }

    totalSize = getint32l(buffer + 4);
    if (totalSize < (int) (WEBP_HEADER_SIZE - 8)) {
        return 0;
    }

    // If RIFF header found, check for RIFF content to start with WEBP/VP8 chunk
    len = stream->read(buffer + 8, WEBP_HEADER_SIZE - 8);
    if (len != (int) (WEBP_HEADER_SIZE - 8)) {
        return 0;
    }
    if (buffer[8] != 'W' || buffer[9] != 'E' || buffer[10] != 'B' || buffer[11] != 'P') {
        return 0;
    }
    if (buffer[12] != 'V' || buffer[13] != 'P' || buffer[14] != '8' || buffer[15] != ' ') {
        return 0;
    }

    // Magic matches, extract content size
    contentSize = getint32l(buffer + 16);

    // Check consistency of reported sizes
    if (contentSize <= 0 || contentSize > 0x7fffffff) {
        return 0;
    }
    if (totalSize < 12 + contentSize) {
        return 0;
    }
    if (contentSize & 1) {
        return 0;
    }

    return contentSize;
}

class SkWEBPImageDecoder: public SkImageDecoder {
public:
    virtual Format getFormat() const {
        return kWEBP_Format;
    }

protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode);
};

//////////////////////////////////////////////////////////////////////////

#include "SkTime.h"

class AutoTimeMillis {
public:
    AutoTimeMillis(const char label[]) :
        fLabel(label) {
        if (!fLabel) {
            fLabel = "";
        }
        fNow = SkTime::GetMSecs();
    }
    ~AutoTimeMillis() {
        SkDebugf("---- Time (ms): %s %d\n", fLabel, SkTime::GetMSecs() - fNow);
    }
private:
    const char* fLabel;
    SkMSec fNow;
};

///////////////////////////////////////////////////////////////////////////////

// This guy exists just to aid in debugging, as it allows debuggers to just
// set a break-point in one place to see all error exists.
static bool return_false(const SkBitmap& bm, const char msg[]) {
#if 0
    SkDebugf("libwebp error %s [%d %d]", msg, bm.width(), bm.height());
#endif
    return false; // must always return false
}

typedef struct {
    SkBitmap* image;
    SkStream* stream;
} WEBPImage;

// WebP library embeds its own YUV to RGB converter. However, High-level API doesn't take benefit
// of (U,v) clipped values being valid for up to 4 pixels, and so there is a significant improvement
// in performance in handling this on our own.
// TODO: use architecture-optimized (eventually hardware-accelerated) YUV converters
#define YUV_HALF (1 << (YUV_FIX - 1))
#define YUV_FIX 16                   // fixed-point precision
#define YUV_RANGE_MIN (-227)         // min value of r/g/b output
#define YUV_RANGE_MAX (256 + 226)    // max value of r/g/b output
static int16_t VP8kVToR[256], VP8kUToB[256];
static int32_t VP8kVToG[256], VP8kUToG[256];
static uint8_t VP8kClip[YUV_RANGE_MAX - YUV_RANGE_MIN];

static void yuv_init_tables() {
    int i;

    for (i = 0; i < 256; ++i) {
        VP8kVToR[i] = (89858 * (i - 128) + YUV_HALF) >> YUV_FIX;
        VP8kUToG[i] = -22014 * (i - 128) + YUV_HALF;
        VP8kVToG[i] = -45773 * (i - 128);
        VP8kUToB[i] = (113618 * (i - 128) + YUV_HALF) >> YUV_FIX;
    }
    for (i = YUV_RANGE_MIN; i < YUV_RANGE_MAX; ++i) {
        const int k = ((i - 16) * 76283 + YUV_HALF) >> YUV_FIX;
        VP8kClip[i - YUV_RANGE_MIN] = (k < 0) ? 0 : (k > 255) ? 255 : k;
    }
}

// Static global mutex to protect Webp initialization
static SkMutex gYUVMutex;
static bool gYUVReady = false;

static bool yuv_init() {
    if (!gYUVReady) {
        gYUVMutex.acquire();
        if (!gYUVReady) {
            yuv_init_tables();
            gYUVReady = true;
        }
        gYUVMutex.release();
    }

    return gYUVReady;
}

#define PutRGBA(p,r,g,b) (((SkPMColor*) (p))[0] = SkPackARGB32(0xff,(r),(g),(b)))
#define PutRGB565(p,r,g,b) (((SkPMColor16*) (p))[0] = SkPackRGB16((r)>>3,(g)>>2,(b)>>3))
#define PutRGBA4444(p,r,g,b) (((SkPMColor16*) (p))[0] = SkPackARGB4444(0xf,(r)>>4,(g)>>4,(b)>>4))

#define CRGBA(p,y,roff,goff,boff) PutRGBA(p,         \
          VP8kClip[(y) + (roff) - YUV_RANGE_MIN],    \
          VP8kClip[(y) + (goff) - YUV_RANGE_MIN],    \
          VP8kClip[(y) + (boff) - YUV_RANGE_MIN])
#define CRGB565(p,y,roff,goff,boff) PutRGB565(p,     \
          VP8kClip[(y) + (roff) - YUV_RANGE_MIN],    \
          VP8kClip[(y) + (goff) - YUV_RANGE_MIN],    \
          VP8kClip[(y) + (boff) - YUV_RANGE_MIN])
#define CRGBA4444(p,y,roff,goff,boff) PutRGBA4444(p, \
          VP8kClip[(y) + (roff) - YUV_RANGE_MIN],    \
          VP8kClip[(y) + (goff) - YUV_RANGE_MIN],    \
          VP8kClip[(y) + (boff) - YUV_RANGE_MIN])

static int block_put(const VP8Io* io) {
    WEBPImage *p = (WEBPImage*) io->opaque;
    SkBitmap* decodedBitmap = p->image;

    const int w = io->width;
    const int mb_h = io->mb_h;

    const uint8_t *y, *y2, *u, *v;
    const uint8_t *py, *py2, *pu, *pv;

    uint8_t* pout;
    uint8_t* pout2;

    int i, j;
    const int ystride2 = io->y_stride * 2;
    int bpp;
    SkBitmap::Config config = decodedBitmap->config();

    //SkASSERT(!(io->mb_y & 1));

    y = io->y;
    u = io->u;
    v = io->v;

    switch (config) {
        case SkBitmap::kARGB_8888_Config:
            bpp = 4;
            break;
        case SkBitmap::kRGB_565_Config:
            bpp = 2;
            break;
        case SkBitmap::kARGB_4444_Config:
            bpp = 2;
            break;
        default:
            // Unsupported config
            return 0;
    }

    for (j = 0; j < mb_h;) {
        pout = decodedBitmap->getAddr8(0, io->mb_y + j);
        if (j + 1 < mb_h) {
            y2 = y + io->y_stride;
            pout2 = decodedBitmap->getAddr8(0, io->mb_y + j + 1);
        } else {
            y2 = NULL;
            pout2 = NULL;
        }

        // Copy YUV into target buffer
        py = y;
        pu = u;
        pv = v;

        py2 = y2;

        // Leave test for config out of inner loop. This implies some redundancy in code,
        // but help in supporting several configs without degrading performance.
        // As a reminder, one must *NOT* put py increment into parameters (i.e. *py++) in the hope to
        // improve performance or code readability. Since it is used as argument of a macro which uses it
        // several times in its expression, so this would end up in having it too much incremented
        switch (config) {
            case SkBitmap::kARGB_8888_Config:
                for (i = 0; i < w; i += 2) {
                    // U and V are common for up to 4 pixels
                    const int r_off = VP8kVToR[*pv];
                    const int g_off = (VP8kVToG[*pv] + VP8kUToG[*pu]) >> YUV_FIX;
                    const int b_off = VP8kUToB[*pu];

                    CRGBA(pout, *py, r_off, g_off, b_off);
                    pout += bpp;
                    py++;

                    // Width shouldn't be odd, so this should always be true
                    if (i + 1 < w) {
                        CRGBA(pout, *py, r_off, g_off, b_off);
                        pout += bpp;
                        py++;
                    }

                    if (pout2) {
                        CRGBA(pout2, *py2, r_off, g_off, b_off);
                        pout2 += bpp;
                        py2++;

                        // Width shouldn't be odd, so this should always be true
                        if (i + 1 < w) {
                            CRGBA(pout2, *py2, r_off, g_off, b_off);
                            pout2 += bpp;
                            py2++;
                        }
                    }

                    pu++;
                    pv++;
                }
                break;
            case SkBitmap::kRGB_565_Config:
                for (i = 0; i < w; i += 2) {
                    // U and V are common for up to 4 pixels
                    const int r_off = VP8kVToR[*pv];
                    const int g_off = (VP8kVToG[*pv] + VP8kUToG[*pu]) >> YUV_FIX;
                    const int b_off = VP8kUToB[*pu];

                    CRGB565(pout, *py, r_off, g_off, b_off);
                    pout += bpp;
                    py++;

                    // Width shouldn't be odd, so this should always be true
                    if (i + 1 < w) {
                        CRGB565(pout, *py, r_off, g_off, b_off);
                        pout += bpp;
                        py++;
                    }

                    if (pout2) {
                        CRGB565(pout2, *py2, r_off, g_off, b_off);
                        pout2 += bpp;
                        py2++;

                        // Width shouldn't be odd, so this should always be true
                        if (i + 1 < w) {
                            CRGB565(pout2, *py2, r_off, g_off, b_off);
                            pout2 += bpp;
                            py2++;
                        }
                    }

                    pu++;
                    pv++;
                }
                break;
            case SkBitmap::kARGB_4444_Config:
                for (i = 0; i < w; i += 2) {
                    // U and V are common for up to 4 pixels
                    const int r_off = VP8kVToR[*pv];
                    const int g_off = (VP8kVToG[*pv] + VP8kUToG[*pu]) >> YUV_FIX;
                    const int b_off = VP8kUToB[*pu];

                    CRGBA4444(pout, *py, r_off, g_off, b_off);
                    pout += bpp;
                    py++;

                    // Width shouldn't be odd, so this should always be true
                    if (i + 1 < w) {
                        CRGBA4444(pout, *py, r_off, g_off, b_off);
                        pout += bpp;
                        py++;
                    }

                    if (pout2) {
                        CRGBA4444(pout2, *py2, r_off, g_off, b_off);
                        pout2 += bpp;
                        py2++;

                        // Width shouldn't be odd, so this should always be true
                        if (i + 1 < w) {
                            CRGBA4444(pout2, *py2, r_off, g_off, b_off);
                            pout2 += bpp;
                            py2++;
                        }
                    }

                    pu++;
                    pv++;
                }
                break;
            default:
                // Unsupported config (can't happen, but prevents compiler warning)
                SkASSERT(0);
                break;
        }

        if (y2) {
            // Scanned and populated two rows
            y += ystride2;
            y2 += ystride2;
            j += 2;
        } else {
            // Skip to next row
            y += io->y_stride;
            j++;
        }

        u += io->uv_stride;
        v += io->uv_stride;
    }

    return 1;
}

static int block_setup(VP8Io* io) {
    yuv_init();
    return 1;
}

static void block_teardown(const VP8Io* io) {
}

bool SkWEBPImageDecoder::onDecode(SkStream* stream, SkBitmap* decodedBitmap, Mode mode) {
#ifdef TIME_DECODE
    AutoTimeMillis atm("WEBP Decode");
#endif

    // libwebp doesn't provide a way to override all I/O with a custom
    // implementation. For initial implementation, let's go the "dirty"
    // way, by loading image file content in memory before decoding it
    int origWidth, origHeight;
    size_t len;

    bool hasAlpha = false;

    uint32_t contentSize;
    unsigned char buffer[VP8_HEADER_SIZE];
    unsigned char *input;

    // Check header
    contentSize = webp_parse_header(stream);
    if (contentSize <= VP8_HEADER_SIZE) {
        return false;
    }

    //* Extract information
    len = stream->read(buffer, VP8_HEADER_SIZE);
    if (len != VP8_HEADER_SIZE) {
        return false;
    }

    // check signature
    if (buffer[3] != 0x9d || buffer[4] != 0x01 || buffer[5] != 0x2a) {
        return false;
    }

    const uint32_t bits = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16);
    const int key_frame = !(bits & 1);

    origWidth = ((buffer[7] << 8) | buffer[6]) & 0x3fff;
    origHeight = ((buffer[9] << 8) | buffer[8]) & 0x3fff;

    if (origWidth <= 0 || origHeight <= 0) {
        return false;
    }

    if (!key_frame) {
        // Not a keyframe
        return false;
    }

    if (((bits >> 1) & 7) > 3) {
        // unknown profile
        return false;
    }
    if (!((bits >> 4) & 1)) {
        // first frame is invisible
        return false;
    }
    if (((bits >> 5)) >= contentSize) {
        // partition_length inconsistent size information
        return false;
    }

    SkBitmap::Config config;
    config = this->getPrefConfig(k32Bit_SrcDepth, hasAlpha);

    // only accept prefConfig if it makes sense for us. YUV converter
    // supports output in RGB565, RGBA4444 and RGBA8888 formats.
    if (hasAlpha) {
        if (config != SkBitmap::kARGB_4444_Config) {
            config = SkBitmap::kARGB_8888_Config;
        }
    } else {
        if (config != SkBitmap::kRGB_565_Config && config != SkBitmap::kARGB_4444_Config) {
            config = SkBitmap::kARGB_8888_Config;
        }
    }

    // sanity check for size
    {
        Sk64 size;
        size.setMul(origWidth, origHeight);
        if (size.isNeg() || !size.is32()) {
            return false;
        }
        // now check that if we are 4-bytes per pixel, we also don't overflow
        if (size.get32() > (0x7FFFFFFF >> 2)) {
            return false;
        }
    }

    if (!this->chooseFromOneChoice(config, origWidth, origHeight)) {
        return false;
    }

    // TODO: may add support for sampler, to load previeww/thumbnails faster
    // Note that, as documented, an image decoder may decide to ignore sample hint from requested
    // config, so this implementation is still valid and safe even without handling it at all. Several
    // other Skia image decoders just ignore this optional feature as well.
#if 0
    SkScaledBitmapSampler sampler(origWidth, origHeight, getSampleSize());
    decodedBitmap->setConfig(config, sampler.scaledWidth(), sampler.scaledHeight(), 0);
#else
    decodedBitmap->setConfig(config, origWidth, origHeight, 0);
#endif

    // If only bounds are requested, done
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return true;
    }

    // Current WEBP specification has no support for alpha layer.
    decodedBitmap->setIsOpaque(true);

    if (!this->allocPixelRef(decodedBitmap, NULL)) {
        return return_false(*decodedBitmap, "allocPixelRef");
    }
    SkAutoLockPixels alp(*decodedBitmap);

    // libwebp doesn't provide a way to override all I/O with a custom
    // implementation. For initial implementation, let's go the "dirty"
    // way, by loading image file content (actually only VP8 chunk) into
    // memory before decoding it */
    SkAutoMalloc srcStorage(contentSize);
    input = (uint8_t*) srcStorage.get();
    if (input == NULL) {
        return return_false(*decodedBitmap, "failed to allocate read buffer");
    }

    len = stream->read(input + VP8_HEADER_SIZE, contentSize - VP8_HEADER_SIZE);
#ifdef WEBPCONV_MISSING_PADDING
    // Some early (yet widely spread) version of webpconv utility
    // had a bug causing mandatory padding byte to not be written to
    // file when content size was odd, while total size was reporting
    // actual file size correctly. Since many webp around may have been
    // generated with this version of webpconv, work around this issue
    // by adding padding here. */
    // TODO: remove this whenever work-around may be considered obsolete
    if (len == contentSize - VP8_HEADER_SIZE - 1) {
        input[VP8_HEADER_SIZE + len] = 0;
        len++;
    }
#endif
    if (len != contentSize - VP8_HEADER_SIZE) {
        return false;
    }
    memcpy(input, buffer, VP8_HEADER_SIZE);

    WEBPImage pSrc;
    VP8Decoder* dec;
    VP8Io io;

    // Custom Put callback need reference to target image
    pSrc.image = decodedBitmap;

    // Keep reference to input stream, in case we find a way to not preload stream
    // content in memory. So far, stream content has already been consumed, and this
    // won't be used, but this is left for future usage.
    pSrc.stream = stream;

    dec = VP8New();
    if (dec == NULL) {
        return false;
    }

    VP8InitIo(&io);
    io.data = input;
    io.data_size = contentSize;

    io.opaque = (void*) &pSrc;
    io.put = block_put;
    io.setup = block_setup;
    io.teardown = block_teardown;

    if (!VP8GetHeaders(dec, &io)) {
        VP8Delete(dec);
        return false;
    }

    if (!VP8Decode(dec, &io)) {
        VP8Delete(dec);
        return false;
    }

    VP8Delete(dec);

    // SkDebugf("------------------- bm2 size %d [%d %d] %d\n",
    //          bm->getSize(), bm->width(), bm->height(), bm->config());
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkTRegistry.h"

static SkImageDecoder* DFactory(SkStream* stream) {
    if (webp_parse_header(stream) <= 0) {
        return NULL;
    }

    // Magic matches, call decoder
    return SkNEW(SkWEBPImageDecoder);
}

SkImageDecoder* sk_libwebp_dfactory(SkStream* stream) {
    return DFactory(stream);
}

static SkTRegistry<SkImageDecoder*, SkStream*> gDReg(DFactory);
