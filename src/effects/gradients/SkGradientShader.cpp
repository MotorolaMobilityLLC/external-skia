/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGradientShaderPriv.h"
#include "SkLinearGradient.h"
#include "SkRadialGradient.h"
#include "SkTwoPointRadialGradient.h"
#include "SkTwoPointConicalGradient.h"
#include "SkSweepGradient.h"

SkGradientShaderBase::SkGradientShaderBase(const SkColor colors[], const SkScalar pos[],
             int colorCount, SkShader::TileMode mode, SkUnitMapper* mapper) {
    SkASSERT(colorCount > 1);

    fCacheAlpha = 256;  // init to a value that paint.getAlpha() can't return

    fMapper = mapper;
    SkSafeRef(mapper);

    SkASSERT((unsigned)mode < SkShader::kTileModeCount);
    SkASSERT(SkShader::kTileModeCount == SK_ARRAY_COUNT(gTileProcs));
    fTileMode = mode;
    fTileProc = gTileProcs[mode];

    fCache16 = fCache16Storage = NULL;
    fCache32 = NULL;
    fCache32PixelRef = NULL;

    /*  Note: we let the caller skip the first and/or last position.
        i.e. pos[0] = 0.3, pos[1] = 0.7
        In these cases, we insert dummy entries to ensure that the final data
        will be bracketed by [0, 1].
        i.e. our_pos[0] = 0, our_pos[1] = 0.3, our_pos[2] = 0.7, our_pos[3] = 1

        Thus colorCount (the caller's value, and fColorCount (our value) may
        differ by up to 2. In the above example:
            colorCount = 2
            fColorCount = 4
     */
    fColorCount = colorCount;
    // check if we need to add in dummy start and/or end position/colors
    bool dummyFirst = false;
    bool dummyLast = false;
    if (pos) {
        dummyFirst = pos[0] != 0;
        dummyLast = pos[colorCount - 1] != SK_Scalar1;
        fColorCount += dummyFirst + dummyLast;
    }

    if (fColorCount > kColorStorageCount) {
        size_t size = sizeof(SkColor) + sizeof(Rec);
        fOrigColors = reinterpret_cast<SkColor*>(
                                        sk_malloc_throw(size * fColorCount));
    }
    else {
        fOrigColors = fStorage;
    }

    // Now copy over the colors, adding the dummies as needed
    {
        SkColor* origColors = fOrigColors;
        if (dummyFirst) {
            *origColors++ = colors[0];
        }
        memcpy(origColors, colors, colorCount * sizeof(SkColor));
        if (dummyLast) {
            origColors += colorCount;
            *origColors = colors[colorCount - 1];
        }
    }

    fRecs = (Rec*)(fOrigColors + fColorCount);
    if (fColorCount > 2) {
        Rec* recs = fRecs;
        recs->fPos = 0;
        //  recs->fScale = 0; // unused;
        recs += 1;
        if (pos) {
            /*  We need to convert the user's array of relative positions into
                fixed-point positions and scale factors. We need these results
                to be strictly monotonic (no two values equal or out of order).
                Hence this complex loop that just jams a zero for the scale
                value if it sees a segment out of order, and it assures that
                we start at 0 and end at 1.0
            */
            SkFixed prev = 0;
            int startIndex = dummyFirst ? 0 : 1;
            int count = colorCount + dummyLast;
            for (int i = startIndex; i < count; i++) {
                // force the last value to be 1.0
                SkFixed curr;
                if (i == colorCount) {  // we're really at the dummyLast
                    curr = SK_Fixed1;
                } else {
                    curr = SkScalarToFixed(pos[i]);
                }
                // pin curr withing range
                if (curr < 0) {
                    curr = 0;
                } else if (curr > SK_Fixed1) {
                    curr = SK_Fixed1;
                }
                recs->fPos = curr;
                if (curr > prev) {
                    recs->fScale = (1 << 24) / (curr - prev);
                } else {
                    recs->fScale = 0; // ignore this segment
                }
                // get ready for the next value
                prev = curr;
                recs += 1;
            }
        } else {    // assume even distribution
            SkFixed dp = SK_Fixed1 / (colorCount - 1);
            SkFixed p = dp;
            SkFixed scale = (colorCount - 1) << 8;  // (1 << 24) / dp
            for (int i = 1; i < colorCount; i++) {
                recs->fPos   = p;
                recs->fScale = scale;
                recs += 1;
                p += dp;
            }
        }
    }
    this->initCommon();
}

SkGradientShaderBase::SkGradientShaderBase(SkFlattenableReadBuffer& buffer) :
    INHERITED(buffer) {
    fCacheAlpha = 256;

    fMapper = buffer.readFlattenableT<SkUnitMapper>();

    fCache16 = fCache16Storage = NULL;
    fCache32 = NULL;
    fCache32PixelRef = NULL;

    int colorCount = fColorCount = buffer.getArrayCount();
    if (colorCount > kColorStorageCount) {
        size_t size = sizeof(SkColor) + sizeof(SkPMColor) + sizeof(Rec);
        fOrigColors = (SkColor*)sk_malloc_throw(size * colorCount);
    } else {
        fOrigColors = fStorage;
    }
    buffer.readColorArray(fOrigColors);

    fTileMode = (TileMode)buffer.readUInt();
    fTileProc = gTileProcs[fTileMode];
    fRecs = (Rec*)(fOrigColors + colorCount);
    if (colorCount > 2) {
        Rec* recs = fRecs;
        recs[0].fPos = 0;
        for (int i = 1; i < colorCount; i++) {
            recs[i].fPos = buffer.readInt();
            recs[i].fScale = buffer.readUInt();
        }
    }
    buffer.readMatrix(&fPtsToUnit);
    this->initCommon();
}

SkGradientShaderBase::~SkGradientShaderBase() {
    if (fCache16Storage) {
        sk_free(fCache16Storage);
    }
    SkSafeUnref(fCache32PixelRef);
    if (fOrigColors != fStorage) {
        sk_free(fOrigColors);
    }
    SkSafeUnref(fMapper);
}

void SkGradientShaderBase::initCommon() {
    fFlags = 0;
    unsigned colorAlpha = 0xFF;
    for (int i = 0; i < fColorCount; i++) {
        colorAlpha &= SkColorGetA(fOrigColors[i]);
    }
    fColorsAreOpaque = colorAlpha == 0xFF;
}

void SkGradientShaderBase::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fMapper);
    buffer.writeColorArray(fOrigColors, fColorCount);
    buffer.writeUInt(fTileMode);
    if (fColorCount > 2) {
        Rec* recs = fRecs;
        for (int i = 1; i < fColorCount; i++) {
            buffer.writeInt(recs[i].fPos);
            buffer.writeUInt(recs[i].fScale);
        }
    }
    buffer.writeMatrix(fPtsToUnit);
}

bool SkGradientShaderBase::isOpaque() const {
    return fColorsAreOpaque;
}

bool SkGradientShaderBase::setContext(const SkBitmap& device,
                                 const SkPaint& paint,
                                 const SkMatrix& matrix) {
    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    const SkMatrix& inverse = this->getTotalInverse();

    if (!fDstToIndex.setConcat(fPtsToUnit, inverse)) {
        // need to keep our set/end context calls balanced.
        this->INHERITED::endContext();
        return false;
    }

    fDstToIndexProc = fDstToIndex.getMapXYProc();
    fDstToIndexClass = (uint8_t)SkShader::ComputeMatrixClass(fDstToIndex);

    // now convert our colors in to PMColors
    unsigned paintAlpha = this->getPaintAlpha();

    fFlags = this->INHERITED::getFlags();
    if (fColorsAreOpaque && paintAlpha == 0xFF) {
        fFlags |= kOpaqueAlpha_Flag;
    }
    // we can do span16 as long as our individual colors are opaque,
    // regardless of the paint's alpha
    if (fColorsAreOpaque) {
        fFlags |= kHasSpan16_Flag;
    }

    this->setCacheAlpha(paintAlpha);
    return true;
}

void SkGradientShaderBase::setCacheAlpha(U8CPU alpha) const {
    // if the new alpha differs from the previous time we were called, inval our cache
    // this will trigger the cache to be rebuilt.
    // we don't care about the first time, since the cache ptrs will already be NULL
    if (fCacheAlpha != alpha) {
        fCache16 = NULL;            // inval the cache
        fCache32 = NULL;            // inval the cache
        fCacheAlpha = alpha;        // record the new alpha
        // inform our subclasses
        if (fCache32PixelRef) {
            fCache32PixelRef->notifyPixelsChanged();
        }
    }
}

#define Fixed_To_Dot8(x)        (((x) + 0x80) >> 8)

/** We take the original colors, not our premultiplied PMColors, since we can
    build a 16bit table as long as the original colors are opaque, even if the
    paint specifies a non-opaque alpha.
*/
void SkGradientShaderBase::Build16bitCache(uint16_t cache[], SkColor c0, SkColor c1,
                                      int count) {
    SkASSERT(count > 1);
    SkASSERT(SkColorGetA(c0) == 0xFF);
    SkASSERT(SkColorGetA(c1) == 0xFF);

    SkFixed r = SkColorGetR(c0);
    SkFixed g = SkColorGetG(c0);
    SkFixed b = SkColorGetB(c0);

    SkFixed dr = SkIntToFixed(SkColorGetR(c1) - r) / (count - 1);
    SkFixed dg = SkIntToFixed(SkColorGetG(c1) - g) / (count - 1);
    SkFixed db = SkIntToFixed(SkColorGetB(c1) - b) / (count - 1);

    r = SkIntToFixed(r) + 0x8000;
    g = SkIntToFixed(g) + 0x8000;
    b = SkIntToFixed(b) + 0x8000;

    do {
        unsigned rr = r >> 16;
        unsigned gg = g >> 16;
        unsigned bb = b >> 16;
        cache[0] = SkPackRGB16(SkR32ToR16(rr), SkG32ToG16(gg), SkB32ToB16(bb));
        cache[kCache16Count] = SkDitherPack888ToRGB16(rr, gg, bb);
        cache += 1;
        r += dr;
        g += dg;
        b += db;
    } while (--count != 0);
}

void SkGradientShaderBase::Build32bitCache(SkPMColor cache[], SkColor c0, SkColor c1,
                                      int count, U8CPU paintAlpha) {
    SkASSERT(count > 1);

    // need to apply paintAlpha to our two endpoints
    SkFixed a = SkMulDiv255Round(SkColorGetA(c0), paintAlpha);
    SkFixed da;
    {
        int tmp = SkMulDiv255Round(SkColorGetA(c1), paintAlpha);
        da = SkIntToFixed(tmp - a) / (count - 1);
    }

    /*
     *  r,g,b used to be SkFixed, but on gcc (4.2.1 mac and 4.6.3 goobuntu) in
     *  release builds, we saw a compiler error where the 0xFF parameter in
     *  SkPackARGB32() was being totally ignored whenever it was called with
     *  a non-zero add (e.g. 0x8000).
     *
     *  We found two work-arounds:
     *      1. change r,g,b to unsigned (or just one of them)
     *      2. change SkPackARGB32 to + its (a << SK_A32_SHIFT) value instead
     *         of using |
     *
     *  We chose #1 just because it was more localized.
     *  See http://code.google.com/p/skia/issues/detail?id=1113
     */
    uint32_t r = SkColorGetR(c0);
    uint32_t g = SkColorGetG(c0);
    uint32_t b = SkColorGetB(c0);

    SkFixed dr = SkIntToFixed(SkColorGetR(c1) - r) / (count - 1);
    SkFixed dg = SkIntToFixed(SkColorGetG(c1) - g) / (count - 1);
    SkFixed db = SkIntToFixed(SkColorGetB(c1) - b) / (count - 1);

    /*  We pre-add 1/8 to avoid having to add this to our [0] value each time
        in the loop. Without this, the bias for each would be
            0x2000  0xA000  0xE000  0x6000
        With this trick, we can add 0 for the first (no-op) and just adjust the
        others.
     */
    r = SkIntToFixed(r) + 0x2000;
    g = SkIntToFixed(g) + 0x2000;
    b = SkIntToFixed(b) + 0x2000;

    /*
     *  Our dither-cell (spatially) is
     *      0 2
     *      3 1
     *  Where
     *      [0] -> [-1/8 ... 1/8 ) values near 0
     *      [1] -> [ 1/8 ... 3/8 ) values near 1/4
     *      [2] -> [ 3/8 ... 5/8 ) values near 1/2
     *      [3] -> [ 5/8 ... 7/8 ) values near 3/4
     */

    if (0xFF == a && 0 == da) {
        do {
            cache[kCache32Count*0] = SkPackARGB32(0xFF, (r + 0     ) >> 16,
                                                        (g + 0     ) >> 16,
                                                        (b + 0     ) >> 16);
            cache[kCache32Count*1] = SkPackARGB32(0xFF, (r + 0x8000) >> 16,
                                                        (g + 0x8000) >> 16,
                                                        (b + 0x8000) >> 16);
            cache[kCache32Count*2] = SkPackARGB32(0xFF, (r + 0xC000) >> 16,
                                                        (g + 0xC000) >> 16,
                                                        (b + 0xC000) >> 16);
            cache[kCache32Count*3] = SkPackARGB32(0xFF, (r + 0x4000) >> 16,
                                                        (g + 0x4000) >> 16,
                                                        (b + 0x4000) >> 16);
            cache += 1;
            r += dr;
            g += dg;
            b += db;
        } while (--count != 0);
    } else {
        a = SkIntToFixed(a) + 0x2000;
        do {
            cache[kCache32Count*0] = SkPremultiplyARGBInline((a + 0     ) >> 16,
                                                             (r + 0     ) >> 16,
                                                             (g + 0     ) >> 16,
                                                             (b + 0     ) >> 16);
            cache[kCache32Count*1] = SkPremultiplyARGBInline((a + 0x8000) >> 16,
                                                             (r + 0x8000) >> 16,
                                                             (g + 0x8000) >> 16,
                                                             (b + 0x8000) >> 16);
            cache[kCache32Count*2] = SkPremultiplyARGBInline((a + 0xC000) >> 16,
                                                             (r + 0xC000) >> 16,
                                                             (g + 0xC000) >> 16,
                                                             (b + 0xC000) >> 16);
            cache[kCache32Count*3] = SkPremultiplyARGBInline((a + 0x4000) >> 16,
                                                             (r + 0x4000) >> 16,
                                                             (g + 0x4000) >> 16,
                                                             (b + 0x4000) >> 16);
            cache += 1;
            a += da;
            r += dr;
            g += dg;
            b += db;
        } while (--count != 0);
    }
}

static inline int SkFixedToFFFF(SkFixed x) {
    SkASSERT((unsigned)x <= SK_Fixed1);
    return x - (x >> 16);
}

static inline U16CPU bitsTo16(unsigned x, const unsigned bits) {
    SkASSERT(x < (1U << bits));
    if (6 == bits) {
        return (x << 10) | (x << 4) | (x >> 2);
    }
    if (8 == bits) {
        return (x << 8) | x;
    }
    sk_throw();
    return 0;
}

const uint16_t* SkGradientShaderBase::getCache16() const {
    if (fCache16 == NULL) {
        // double the count for dither entries
        const int entryCount = kCache16Count * 2;
        const size_t allocSize = sizeof(uint16_t) * entryCount;

        if (fCache16Storage == NULL) { // set the storage and our working ptr
            fCache16Storage = (uint16_t*)sk_malloc_throw(allocSize);
        }
        fCache16 = fCache16Storage;
        if (fColorCount == 2) {
            Build16bitCache(fCache16, fOrigColors[0], fOrigColors[1],
                            kCache16Count);
        } else {
            Rec* rec = fRecs;
            int prevIndex = 0;
            for (int i = 1; i < fColorCount; i++) {
                int nextIndex = SkFixedToFFFF(rec[i].fPos) >> kCache16Shift;
                SkASSERT(nextIndex < kCache16Count);

                if (nextIndex > prevIndex)
                    Build16bitCache(fCache16 + prevIndex, fOrigColors[i-1], fOrigColors[i], nextIndex - prevIndex + 1);
                prevIndex = nextIndex;
            }
        }

        if (fMapper) {
            fCache16Storage = (uint16_t*)sk_malloc_throw(allocSize);
            uint16_t* linear = fCache16;         // just computed linear data
            uint16_t* mapped = fCache16Storage;  // storage for mapped data
            SkUnitMapper* map = fMapper;
            for (int i = 0; i < kCache16Count; i++) {
                int index = map->mapUnit16(bitsTo16(i, kCache16Bits)) >> kCache16Shift;
                mapped[i] = linear[index];
                mapped[i + kCache16Count] = linear[index + kCache16Count];
            }
            sk_free(fCache16);
            fCache16 = fCache16Storage;
        }
    }
    return fCache16;
}

const SkPMColor* SkGradientShaderBase::getCache32() const {
    if (fCache32 == NULL) {
        // double the count for dither entries
        const int entryCount = kCache32Count * 4;
        const size_t allocSize = sizeof(SkPMColor) * entryCount;

        if (NULL == fCache32PixelRef) {
            fCache32PixelRef = SkNEW_ARGS(SkMallocPixelRef,
                                          (NULL, allocSize, NULL));
        }
        fCache32 = (SkPMColor*)fCache32PixelRef->getAddr();
        if (fColorCount == 2) {
            Build32bitCache(fCache32, fOrigColors[0], fOrigColors[1],
                            kCache32Count, fCacheAlpha);
        } else {
            Rec* rec = fRecs;
            int prevIndex = 0;
            for (int i = 1; i < fColorCount; i++) {
                int nextIndex = SkFixedToFFFF(rec[i].fPos) >> kCache32Shift;
                SkASSERT(nextIndex < kCache32Count);

                if (nextIndex > prevIndex)
                    Build32bitCache(fCache32 + prevIndex, fOrigColors[i-1],
                                    fOrigColors[i],
                                    nextIndex - prevIndex + 1, fCacheAlpha);
                prevIndex = nextIndex;
            }
        }

        if (fMapper) {
            SkMallocPixelRef* newPR = SkNEW_ARGS(SkMallocPixelRef,
                                                 (NULL, allocSize, NULL));
            SkPMColor* linear = fCache32;           // just computed linear data
            SkPMColor* mapped = (SkPMColor*)newPR->getAddr();    // storage for mapped data
            SkUnitMapper* map = fMapper;
            for (int i = 0; i < kCache32Count; i++) {
                int index = map->mapUnit16((i << 8) | i) >> 8;
                mapped[i + kCache32Count*0] = linear[index + kCache32Count*0];
                mapped[i + kCache32Count*1] = linear[index + kCache32Count*1];
                mapped[i + kCache32Count*2] = linear[index + kCache32Count*2];
                mapped[i + kCache32Count*3] = linear[index + kCache32Count*3];
            }
            fCache32PixelRef->unref();
            fCache32PixelRef = newPR;
            fCache32 = (SkPMColor*)newPR->getAddr();
        }
    }
    return fCache32;
}

/*
 *  Because our caller might rebuild the same (logically the same) gradient
 *  over and over, we'd like to return exactly the same "bitmap" if possible,
 *  allowing the client to utilize a cache of our bitmap (e.g. with a GPU).
 *  To do that, we maintain a private cache of built-bitmaps, based on our
 *  colors and positions. Note: we don't try to flatten the fMapper, so if one
 *  is present, we skip the cache for now.
 */
void SkGradientShaderBase::getGradientTableBitmap(SkBitmap* bitmap) const {
    // our caller assumes no external alpha, so we ensure that our cache is
    // built with 0xFF
    this->setCacheAlpha(0xFF);

    // don't have a way to put the mapper into our cache-key yet
    if (fMapper) {
        // force our cahce32pixelref to be built
        (void)this->getCache32();
        bitmap->setConfig(SkBitmap::kARGB_8888_Config, kCache32Count, 1);
        bitmap->setPixelRef(fCache32PixelRef);
        return;
    }

    // build our key: [numColors + colors[] + {positions[]} ]
    int count = 1 + fColorCount;
    if (fColorCount > 2) {
        count += fColorCount - 1;    // fRecs[].fPos
    }

    SkAutoSTMalloc<16, int32_t> storage(count);
    int32_t* buffer = storage.get();

    *buffer++ = fColorCount;
    memcpy(buffer, fOrigColors, fColorCount * sizeof(SkColor));
    buffer += fColorCount;
    if (fColorCount > 2) {
        for (int i = 1; i < fColorCount; i++) {
            *buffer++ = fRecs[i].fPos;
        }
    }
    SkASSERT(buffer - storage.get() == count);

    ///////////////////////////////////

    SK_DECLARE_STATIC_MUTEX(gMutex);
    static SkBitmapCache* gCache;
    // each cache cost 1K of RAM, since each bitmap will be 1x256 at 32bpp
    static const int MAX_NUM_CACHED_GRADIENT_BITMAPS = 32;
    SkAutoMutexAcquire ama(gMutex);

    if (NULL == gCache) {
        gCache = SkNEW_ARGS(SkBitmapCache, (MAX_NUM_CACHED_GRADIENT_BITMAPS));
    }
    size_t size = count * sizeof(int32_t);

    if (!gCache->find(storage.get(), size, bitmap)) {
        // force our cahce32pixelref to be built
        (void)this->getCache32();
        bitmap->setConfig(SkBitmap::kARGB_8888_Config, kCache32Count, 1);
        bitmap->setPixelRef(fCache32PixelRef);

        gCache->add(storage.get(), size, *bitmap);
    }
}

void SkGradientShaderBase::commonAsAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColorCount >= fColorCount) {
            if (info->fColors) {
                memcpy(info->fColors, fOrigColors, fColorCount * sizeof(SkColor));
            }
            if (info->fColorOffsets) {
                if (fColorCount == 2) {
                    info->fColorOffsets[0] = 0;
                    info->fColorOffsets[1] = SK_Scalar1;
                } else if (fColorCount > 2) {
                    for (int i = 0; i < fColorCount; ++i) {
                        info->fColorOffsets[i] = SkFixedToScalar(fRecs[i].fPos);
                    }
                }
            }
        }
        info->fColorCount = fColorCount;
        info->fTileMode = fTileMode;
    }
}

#ifdef SK_DEVELOPER
void SkGradientShaderBase::toString(SkString* str) const {

    str->appendf("%d colors: ", fColorCount);

    for (int i = 0; i < fColorCount; ++i) {
        str->appendHex(fOrigColors[i]);
        if (i < fColorCount-1) {
            str->append(", ");
        }
    }

    if (fColorCount > 2) {
        str->append(" points: (");
        for (int i = 0; i < fColorCount; ++i) {
            str->appendScalar(SkFixedToScalar(fRecs[i].fPos));
            if (i < fColorCount-1) {
                str->append(", ");
            }
        }
        str->append(")");
    }

    static const char* gTileModeName[SkShader::kTileModeCount] = {
        "clamp", "repeat", "mirror"
    };

    str->append(" ");
    str->append(gTileModeName[fTileMode]);

    // TODO: add "fMapper->toString(str);" when SkUnitMapper::toString is added

    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkEmptyShader.h"

// assumes colors is SkColor* and pos is SkScalar*
#define EXPAND_1_COLOR(count)               \
    SkColor tmp[2];                         \
    do {                                    \
        if (1 == count) {                   \
            tmp[0] = tmp[1] = colors[0];    \
            colors = tmp;                   \
            pos = NULL;                     \
            count = 2;                      \
        }                                   \
    } while (0)

SkShader* SkGradientShader::CreateLinear(const SkPoint pts[2],
                                         const SkColor colors[],
                                         const SkScalar pos[], int colorCount,
                                         SkShader::TileMode mode,
                                         SkUnitMapper* mapper) {
    if (NULL == pts || NULL == colors || colorCount < 1) {
        return NULL;
    }
    EXPAND_1_COLOR(colorCount);

    return SkNEW_ARGS(SkLinearGradient,
                      (pts, colors, pos, colorCount, mode, mapper));
}

SkShader* SkGradientShader::CreateRadial(const SkPoint& center, SkScalar radius,
                                         const SkColor colors[],
                                         const SkScalar pos[], int colorCount,
                                         SkShader::TileMode mode,
                                         SkUnitMapper* mapper) {
    if (radius <= 0 || NULL == colors || colorCount < 1) {
        return NULL;
    }
    EXPAND_1_COLOR(colorCount);

    return SkNEW_ARGS(SkRadialGradient,
                      (center, radius, colors, pos, colorCount, mode, mapper));
}

SkShader* SkGradientShader::CreateTwoPointRadial(const SkPoint& start,
                                                 SkScalar startRadius,
                                                 const SkPoint& end,
                                                 SkScalar endRadius,
                                                 const SkColor colors[],
                                                 const SkScalar pos[],
                                                 int colorCount,
                                                 SkShader::TileMode mode,
                                                 SkUnitMapper* mapper) {
    if (startRadius < 0 || endRadius < 0 || NULL == colors || colorCount < 1) {
        return NULL;
    }
    EXPAND_1_COLOR(colorCount);

    return SkNEW_ARGS(SkTwoPointRadialGradient,
                      (start, startRadius, end, endRadius, colors, pos,
                       colorCount, mode, mapper));
}

SkShader* SkGradientShader::CreateTwoPointConical(const SkPoint& start,
                                                 SkScalar startRadius,
                                                 const SkPoint& end,
                                                 SkScalar endRadius,
                                                 const SkColor colors[],
                                                 const SkScalar pos[],
                                                 int colorCount,
                                                 SkShader::TileMode mode,
                                                 SkUnitMapper* mapper) {
    if (startRadius < 0 || endRadius < 0 || NULL == colors || colorCount < 1) {
        return NULL;
    }
    if (start == end && startRadius == endRadius) {
        return SkNEW(SkEmptyShader);
    }
    EXPAND_1_COLOR(colorCount);

    return SkNEW_ARGS(SkTwoPointConicalGradient,
                      (start, startRadius, end, endRadius, colors, pos,
                       colorCount, mode, mapper));
}

SkShader* SkGradientShader::CreateSweep(SkScalar cx, SkScalar cy,
                                        const SkColor colors[],
                                        const SkScalar pos[],
                                        int count, SkUnitMapper* mapper) {
    if (NULL == colors || count < 1) {
        return NULL;
    }
    EXPAND_1_COLOR(count);

    return SkNEW_ARGS(SkSweepGradient, (cx, cy, colors, pos, count, mapper));
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkGradientShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLinearGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkRadialGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSweepGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTwoPointRadialGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTwoPointConicalGradient)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "effects/GrTextureStripAtlas.h"
#include "GrTBackendEffectFactory.h"
#include "SkGr.h"

GrGLGradientEffect::GrGLGradientEffect(const GrBackendEffectFactory& factory)
    : INHERITED(factory)
    , fCachedYCoord(SK_ScalarMax)
    , fFSYUni(GrGLUniformManager::kInvalidUniformHandle)
    , fEffectMatrix(kCoordsType) {
}

GrGLGradientEffect::~GrGLGradientEffect() { }

void GrGLGradientEffect::emitYCoordUniform(GrGLShaderBuilder* builder) {
    fFSYUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                  kFloat_GrSLType, "GradientYCoordFS");
}

void GrGLGradientEffect::setData(const GrGLUniformManager& uman,
                                 const GrDrawEffect& drawEffect) {
    const GrGradientEffect& e = drawEffect.castEffect<GrGradientEffect>();
    const GrTexture* texture = e.texture(0);
    fEffectMatrix.setData(uman, e.getMatrix(), drawEffect, texture);

    SkScalar yCoord = e.getYCoord();
    if (yCoord != fCachedYCoord) {
        uman.set1f(fFSYUni, yCoord);
        fCachedYCoord = yCoord;
    }
}

GrGLEffect::EffectKey GrGLGradientEffect::GenMatrixKey(const GrDrawEffect& drawEffect) {
    const GrGradientEffect& e = drawEffect.castEffect<GrGradientEffect>();
    const GrTexture* texture = e.texture(0);
    return GrGLEffectMatrix::GenKey(e.getMatrix(), drawEffect, kCoordsType, texture);
}

void GrGLGradientEffect::setupMatrix(GrGLShaderBuilder* builder,
                                     EffectKey key,
                                     const char** fsCoordName,
                                     const char** vsVaryingName,
                                     GrSLType* vsVaryingType) {
    fEffectMatrix.emitCodeMakeFSCoords2D(builder,
                                         key & kMatrixKeyMask,
                                         fsCoordName,
                                         vsVaryingName,
                                         vsVaryingType);
}

void GrGLGradientEffect::emitColorLookup(GrGLShaderBuilder* builder,
                                         const char* gradientTValue,
                                         const char* outputColor,
                                         const char* inputColor,
                                         const GrGLShaderBuilder::TextureSampler& sampler) {

    builder->fsCodeAppendf("\tvec2 coord = vec2(%s, %s);\n",
                           gradientTValue,
                           builder->getUniformVariable(fFSYUni).c_str());
    builder->fsCodeAppendf("\t%s = ", outputColor);
    builder->appendTextureLookupAndModulate(GrGLShaderBuilder::kFragment_ShaderType,
                                            inputColor,
                                            sampler,
                                            "coord");
    builder->fsCodeAppend(";\n");
}

/////////////////////////////////////////////////////////////////////

GrGradientEffect::GrGradientEffect(GrContext* ctx,
                                   const SkGradientShaderBase& shader,
                                   const SkMatrix& matrix,
                                   SkShader::TileMode tileMode) {
    // TODO: check for simple cases where we don't need a texture:
    //GradientInfo info;
    //shader.asAGradient(&info);
    //if (info.fColorCount == 2) { ...

    fMatrix = matrix;

    SkBitmap bitmap;
    shader.getGradientTableBitmap(&bitmap);

    fIsOpaque = shader.isOpaque();

    GrTextureStripAtlas::Desc desc;
    desc.fWidth  = bitmap.width();
    desc.fHeight = 32;
    desc.fRowHeight = bitmap.height();
    desc.fContext = ctx;
    desc.fConfig = SkBitmapConfig2GrPixelConfig(bitmap.config());
    fAtlas = GrTextureStripAtlas::GetAtlas(desc);
    GrAssert(NULL != fAtlas);

    // We always filter the gradient table. Each table is one row of a texture, so always y-clamp.
    GrTextureParams params;
    params.setBilerp(true);
    params.setTileModeX(tileMode);

    fRow = fAtlas->lockRow(bitmap);
    if (-1 != fRow) {
        fYCoord = fAtlas->getYOffset(fRow) + SK_ScalarHalf *
                  fAtlas->getVerticalScaleFactor();
        fTextureAccess.reset(fAtlas->getTexture(), params);
    } else {
        GrTexture* texture = GrLockAndRefCachedBitmapTexture(ctx, bitmap, &params);
        fTextureAccess.reset(texture, params);
        fYCoord = SK_ScalarHalf;

        // Unlock immediately, this is not great, but we don't have a way of
        // knowing when else to unlock it currently, so it may get purged from
        // the cache, but it'll still be ref'd until it's no longer being used.
        GrUnlockAndUnrefCachedBitmapTexture(texture);
    }
    this->addTextureAccess(&fTextureAccess);
}

GrGradientEffect::~GrGradientEffect() {
    if (this->useAtlas()) {
        fAtlas->unlockRow(fRow);
    }
}

bool GrGradientEffect::onIsEqual(const GrEffect& effect) const {
    const GrGradientEffect& s = CastEffect<GrGradientEffect>(effect);
    return fTextureAccess.getTexture() == s.fTextureAccess.getTexture()  &&
           fTextureAccess.getParams().getTileModeX() ==
                s.fTextureAccess.getParams().getTileModeX() &&
           this->useAtlas() == s.useAtlas() &&
           fYCoord == s.getYCoord() &&
           fMatrix.cheapEqualTo(s.getMatrix());
}

void GrGradientEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    if (fIsOpaque && (kA_ValidComponentFlag & *validFlags) && 0xff == GrColorUnpackA(*color)) {
        *validFlags = kA_ValidComponentFlag;
    } else {
        *validFlags = 0;
    }
}

int GrGradientEffect::RandomGradientParams(SkMWCRandom* random,
                                           SkColor colors[],
                                           SkScalar** stops,
                                           SkShader::TileMode* tm) {
    int outColors = random->nextRangeU(1, kMaxRandomGradientColors);

    // if one color, omit stops, otherwise randomly decide whether or not to
    if (outColors == 1 || (outColors >= 2 && random->nextBool())) {
        *stops = NULL;
    }

    SkScalar stop = 0.f;
    for (int i = 0; i < outColors; ++i) {
        colors[i] = random->nextU();
        if (NULL != *stops) {
            (*stops)[i] = stop;
            stop = i < outColors - 1 ? stop + random->nextUScalar1() * (1.f - stop) : 1.f;
        }
    }
    *tm = static_cast<SkShader::TileMode>(random->nextULessThan(SkShader::kTileModeCount));

    return outColors;
}

#endif
