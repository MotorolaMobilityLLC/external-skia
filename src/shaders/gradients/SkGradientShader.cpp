/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include "Sk4fLinearGradient.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpaceXformer.h"
#include "SkFloatBits.h"
#include "SkGradientBitmapCache.h"
#include "SkGradientShaderPriv.h"
#include "SkHalf.h"
#include "SkLinearGradient.h"
#include "SkMallocPixelRef.h"
#include "SkRadialGradient.h"
#include "SkReadBuffer.h"
#include "SkSweepGradient.h"
#include "SkTwoPointConicalGradient.h"
#include "SkWriteBuffer.h"
#include "../../jumper/SkJumper.h"


enum GradientSerializationFlags {
    // Bits 29:31 used for various boolean flags
    kHasPosition_GSF    = 0x80000000,
    kHasLocalMatrix_GSF = 0x40000000,
    kHasColorSpace_GSF  = 0x20000000,

    // Bits 12:28 unused

    // Bits 8:11 for fTileMode
    kTileModeShift_GSF  = 8,
    kTileModeMask_GSF   = 0xF,

    // Bits 0:7 for fGradFlags (note that kForce4fContext_PrivateFlag is 0x80)
    kGradFlagsShift_GSF = 0,
    kGradFlagsMask_GSF  = 0xFF,
};

void SkGradientShaderBase::Descriptor::flatten(SkWriteBuffer& buffer) const {
    uint32_t flags = 0;
    if (fPos) {
        flags |= kHasPosition_GSF;
    }
    if (fLocalMatrix) {
        flags |= kHasLocalMatrix_GSF;
    }
    sk_sp<SkData> colorSpaceData = fColorSpace ? fColorSpace->serialize() : nullptr;
    if (colorSpaceData) {
        flags |= kHasColorSpace_GSF;
    }
    SkASSERT(static_cast<uint32_t>(fTileMode) <= kTileModeMask_GSF);
    flags |= (fTileMode << kTileModeShift_GSF);
    SkASSERT(fGradFlags <= kGradFlagsMask_GSF);
    flags |= (fGradFlags << kGradFlagsShift_GSF);

    buffer.writeUInt(flags);

    buffer.writeColor4fArray(fColors, fCount);
    if (colorSpaceData) {
        buffer.writeDataAsByteArray(colorSpaceData.get());
    }
    if (fPos) {
        buffer.writeScalarArray(fPos, fCount);
    }
    if (fLocalMatrix) {
        buffer.writeMatrix(*fLocalMatrix);
    }
}

bool SkGradientShaderBase::DescriptorScope::unflatten(SkReadBuffer& buffer) {
    // New gradient format. Includes floating point color, color space, densely packed flags
    uint32_t flags = buffer.readUInt();

    fTileMode = (SkShader::TileMode)((flags >> kTileModeShift_GSF) & kTileModeMask_GSF);
    fGradFlags = (flags >> kGradFlagsShift_GSF) & kGradFlagsMask_GSF;

    fCount = buffer.getArrayCount();
    if (fCount > kStorageCount) {
        size_t allocSize = (sizeof(SkColor4f) + sizeof(SkScalar)) * fCount;
        fDynamicStorage.reset(allocSize);
        fColors = (SkColor4f*)fDynamicStorage.get();
        fPos = (SkScalar*)(fColors + fCount);
    } else {
        fColors = fColorStorage;
        fPos = fPosStorage;
    }
    if (!buffer.readColor4fArray(mutableColors(), fCount)) {
        return false;
    }
    if (SkToBool(flags & kHasColorSpace_GSF)) {
        sk_sp<SkData> data = buffer.readByteArrayAsData();
        fColorSpace = SkColorSpace::Deserialize(data->data(), data->size());
    } else {
        fColorSpace = nullptr;
    }
    if (SkToBool(flags & kHasPosition_GSF)) {
        if (!buffer.readScalarArray(mutablePos(), fCount)) {
            return false;
        }
    } else {
        fPos = nullptr;
    }
    if (SkToBool(flags & kHasLocalMatrix_GSF)) {
        fLocalMatrix = &fLocalMatrixStorage;
        buffer.readMatrix(&fLocalMatrixStorage);
    } else {
        fLocalMatrix = nullptr;
    }
    return buffer.isValid();
}

////////////////////////////////////////////////////////////////////////////////////////////

SkGradientShaderBase::SkGradientShaderBase(const Descriptor& desc, const SkMatrix& ptsToUnit)
    : INHERITED(desc.fLocalMatrix)
    , fPtsToUnit(ptsToUnit)
    , fColorsAreOpaque(true)
{
    fPtsToUnit.getType();  // Precache so reads are threadsafe.
    SkASSERT(desc.fCount > 1);

    fGradFlags = static_cast<uint8_t>(desc.fGradFlags);

    SkASSERT((unsigned)desc.fTileMode < SkShader::kTileModeCount);
    fTileMode = desc.fTileMode;

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
    fColorCount = desc.fCount;
    // check if we need to add in dummy start and/or end position/colors
    bool dummyFirst = false;
    bool dummyLast = false;
    if (desc.fPos) {
        dummyFirst = desc.fPos[0] != 0;
        dummyLast = desc.fPos[desc.fCount - 1] != SK_Scalar1;
        fColorCount += dummyFirst + dummyLast;
    }

    if (fColorCount > kColorStorageCount) {
        size_t size = sizeof(SkColor4f);
        if (desc.fPos) {
            size += sizeof(SkScalar);
        }
        fOrigColors4f = reinterpret_cast<SkColor4f*>(sk_malloc_throw(size * fColorCount));
    }
    else {
        fOrigColors4f = fStorage;
    }

    // Now copy over the colors, adding the dummies as needed
    SkColor4f* origColors = fOrigColors4f;
    if (dummyFirst) {
        *origColors++ = desc.fColors[0];
    }
    for (int i = 0; i < desc.fCount; ++i) {
        origColors[i] = desc.fColors[i];
        fColorsAreOpaque = fColorsAreOpaque && (desc.fColors[i].fA == 1);
    }
    if (dummyLast) {
        origColors += desc.fCount;
        *origColors = desc.fColors[desc.fCount - 1];
    }

    if (!desc.fColorSpace) {
        // This happens if we were constructed from SkColors, so our colors are really sRGB
        fColorSpace = SkColorSpace::MakeSRGBLinear();
    } else {
        // The color space refers to the float colors, so it must be linear gamma
        // TODO: GPU code no longer requires this (see GrGradientEffect). Remove this restriction?
        SkASSERT(desc.fColorSpace->gammaIsLinear());
        fColorSpace = desc.fColorSpace;
    }

    if (desc.fPos && fColorCount) {
        fOrigPos = (SkScalar*)(fOrigColors4f + fColorCount);
    } else {
        fOrigPos = nullptr;
    }

    if (fColorCount > 2) {
        if (desc.fPos) {
            SkScalar* origPosPtr = fOrigPos;
            *origPosPtr++ = 0;

            int startIndex = dummyFirst ? 0 : 1;
            int count = desc.fCount + dummyLast;
            for (int i = startIndex; i < count; i++) {
                // force the last value to be 1.0
                SkScalar curr;
                if (i == desc.fCount) {  // we're really at the dummyLast
                    curr = 1;
                } else {
                    curr = SkScalarPin(desc.fPos[i], 0, 1);
                }
                *origPosPtr++ = curr;
            }
        }
    } else if (desc.fPos) {
        SkASSERT(2 == fColorCount);
        fOrigPos[0] = SkScalarPin(desc.fPos[0], 0, 1);
        fOrigPos[1] = SkScalarPin(desc.fPos[1], fOrigPos[0], 1);
        if (0 == fOrigPos[0] && 1 == fOrigPos[1]) {
            fOrigPos = nullptr;
        }
    }
}

SkGradientShaderBase::~SkGradientShaderBase() {
    if (fOrigColors4f != fStorage) {
        sk_free(fOrigColors4f);
    }
}

void SkGradientShaderBase::flatten(SkWriteBuffer& buffer) const {
    Descriptor desc;
    desc.fColors = fOrigColors4f;
    desc.fColorSpace = fColorSpace;
    desc.fPos = fOrigPos;
    desc.fCount = fColorCount;
    desc.fTileMode = fTileMode;
    desc.fGradFlags = fGradFlags;

    const SkMatrix& m = this->getLocalMatrix();
    desc.fLocalMatrix = m.isIdentity() ? nullptr : &m;
    desc.flatten(buffer);
}

static void add_stop_color(SkJumper_GradientCtx* ctx, size_t stop, SkPM4f Fs, SkPM4f Bs) {
    (ctx->fs[0])[stop] = Fs.r();
    (ctx->fs[1])[stop] = Fs.g();
    (ctx->fs[2])[stop] = Fs.b();
    (ctx->fs[3])[stop] = Fs.a();
    (ctx->bs[0])[stop] = Bs.r();
    (ctx->bs[1])[stop] = Bs.g();
    (ctx->bs[2])[stop] = Bs.b();
    (ctx->bs[3])[stop] = Bs.a();
}

static void add_const_color(SkJumper_GradientCtx* ctx, size_t stop, SkPM4f color) {
    add_stop_color(ctx, stop, SkPM4f::FromPremulRGBA(0,0,0,0), color);
}

// Calculate a factor F and a bias B so that color = F*t + B when t is in range of
// the stop. Assume that the distance between stops is 1/gapCount.
static void init_stop_evenly(
    SkJumper_GradientCtx* ctx, float gapCount, size_t stop, SkPM4f c_l, SkPM4f c_r) {
    // Clankium's GCC 4.9 targeting ARMv7 is barfing when we use Sk4f math here, so go scalar...
    SkPM4f Fs = {{
        (c_r.r() - c_l.r()) * gapCount,
        (c_r.g() - c_l.g()) * gapCount,
        (c_r.b() - c_l.b()) * gapCount,
        (c_r.a() - c_l.a()) * gapCount,
    }};
    SkPM4f Bs = {{
        c_l.r() - Fs.r()*(stop/gapCount),
        c_l.g() - Fs.g()*(stop/gapCount),
        c_l.b() - Fs.b()*(stop/gapCount),
        c_l.a() - Fs.a()*(stop/gapCount),
    }};
    add_stop_color(ctx, stop, Fs, Bs);
}

// For each stop we calculate a bias B and a scale factor F, such that
// for any t between stops n and n+1, the color we want is B[n] + F[n]*t.
static void init_stop_pos(
    SkJumper_GradientCtx* ctx, size_t stop, float t_l, float t_r, SkPM4f c_l, SkPM4f c_r) {
    // See note about Clankium's old compiler in init_stop_evenly().
    SkPM4f Fs = {{
        (c_r.r() - c_l.r()) / (t_r - t_l),
        (c_r.g() - c_l.g()) / (t_r - t_l),
        (c_r.b() - c_l.b()) / (t_r - t_l),
        (c_r.a() - c_l.a()) / (t_r - t_l),
    }};
    SkPM4f Bs = {{
        c_l.r() - Fs.r()*t_l,
        c_l.g() - Fs.g()*t_l,
        c_l.b() - Fs.b()*t_l,
        c_l.a() - Fs.a()*t_l,
    }};
    ctx->ts[stop] = t_l;
    add_stop_color(ctx, stop, Fs, Bs);
}

bool SkGradientShaderBase::onAppendStages(const StageRec& rec) const {
    SkRasterPipeline* p = rec.fPipeline;
    SkArenaAlloc* alloc = rec.fAlloc;
    SkColorSpace* dstCS = rec.fDstCS;

    SkMatrix matrix;
    if (!this->computeTotalInverse(rec.fCTM, rec.fLocalM, &matrix)) {
        return false;
    }
    matrix.postConcat(fPtsToUnit);

    SkRasterPipeline_<256> postPipeline;

    p->append_seed_shader();
    p->append_matrix(alloc, matrix);
    this->appendGradientStages(alloc, p, &postPipeline);

    switch(fTileMode) {
        case kMirror_TileMode: p->append(SkRasterPipeline::mirror_x_1); break;
        case kRepeat_TileMode: p->append(SkRasterPipeline::repeat_x_1); break;
        case kClamp_TileMode:
            if (!fOrigPos) {
                // We clamp only when the stops are evenly spaced.
                // If not, there may be hard stops, and clamping ruins hard stops at 0 and/or 1.
                // In that case, we must make sure we're using the general "gradient" stage,
                // which is the only stage that will correctly handle unclamped t.
                p->append(SkRasterPipeline::clamp_x_1);
            }
    }

    const bool premulGrad = fGradFlags & SkGradientShader::kInterpolateColorsInPremul_Flag;
    auto prepareColor = [premulGrad, dstCS, this](int i) {
        SkColor4f c = this->getXformedColor(i, dstCS);
        return premulGrad ? c.premul()
                          : SkPM4f::From4f(Sk4f::Load(&c));
    };

    // The two-stop case with stops at 0 and 1.
    if (fColorCount == 2 && fOrigPos == nullptr) {
        const SkPM4f c_l = prepareColor(0),
                     c_r = prepareColor(1);

        // See F and B below.
        auto* f_and_b = alloc->makeArrayDefault<SkPM4f>(2);
        f_and_b[0] = SkPM4f::From4f(c_r.to4f() - c_l.to4f());
        f_and_b[1] = c_l;

        p->append(SkRasterPipeline::evenly_spaced_2_stop_gradient, f_and_b);
    } else {
        auto* ctx = alloc->make<SkJumper_GradientCtx>();

        // Note: In order to handle clamps in search, the search assumes a stop conceptully placed
        // at -inf. Therefore, the max number of stops is fColorCount+1.
        for (int i = 0; i < 4; i++) {
            // Allocate at least at for the AVX2 gather from a YMM register.
            ctx->fs[i] = alloc->makeArray<float>(std::max(fColorCount+1, 8));
            ctx->bs[i] = alloc->makeArray<float>(std::max(fColorCount+1, 8));
        }

        if (fOrigPos == nullptr) {
            // Handle evenly distributed stops.

            size_t stopCount = fColorCount;
            float gapCount = stopCount - 1;

            SkPM4f c_l = prepareColor(0);
            for (size_t i = 0; i < stopCount - 1; i++) {
                SkPM4f c_r = prepareColor(i + 1);
                init_stop_evenly(ctx, gapCount, i, c_l, c_r);
                c_l = c_r;
            }
            add_const_color(ctx, stopCount - 1, c_l);

            ctx->stopCount = stopCount;
            p->append(SkRasterPipeline::evenly_spaced_gradient, ctx);
        } else {
            // Handle arbitrary stops.

            ctx->ts = alloc->makeArray<float>(fColorCount+1);

            // Remove the dummy stops inserted by SkGradientShaderBase::SkGradientShaderBase
            // because they are naturally handled by the search method.
            int firstStop;
            int lastStop;
            if (fColorCount > 2) {
                firstStop = fOrigColors4f[0] != fOrigColors4f[1] ? 0 : 1;
                lastStop = fOrigColors4f[fColorCount - 2] != fOrigColors4f[fColorCount - 1]
                           ? fColorCount - 1 : fColorCount - 2;
            } else {
                firstStop = 0;
                lastStop = 1;
            }

            size_t stopCount = 0;
            float  t_l = fOrigPos[firstStop];
            SkPM4f c_l = prepareColor(firstStop);
            add_const_color(ctx, stopCount++, c_l);
            // N.B. lastStop is the index of the last stop, not one after.
            for (int i = firstStop; i < lastStop; i++) {
                float  t_r = fOrigPos[i + 1];
                SkPM4f c_r = prepareColor(i + 1);
                if (t_l < t_r) {
                    init_stop_pos(ctx, stopCount, t_l, t_r, c_l, c_r);
                    stopCount += 1;
                }
                t_l = t_r;
                c_l = c_r;
            }

            ctx->ts[stopCount] = t_l;
            add_const_color(ctx, stopCount++, c_l);

            ctx->stopCount = stopCount;
            p->append(SkRasterPipeline::gradient, ctx);
        }
    }

    if (!premulGrad && !this->colorsAreOpaque()) {
        p->append(SkRasterPipeline::premul);
    }

    p->extend(postPipeline);

    return true;
}


bool SkGradientShaderBase::isOpaque() const {
    return fColorsAreOpaque;
}

static unsigned rounded_divide(unsigned numer, unsigned denom) {
    return (numer + (denom >> 1)) / denom;
}

bool SkGradientShaderBase::onAsLuminanceColor(SkColor* lum) const {
    // we just compute an average color.
    // possibly we could weight this based on the proportional width for each color
    //   assuming they are not evenly distributed in the fPos array.
    int r = 0;
    int g = 0;
    int b = 0;
    const int n = fColorCount;
    // TODO: use linear colors?
    for (int i = 0; i < n; ++i) {
        SkColor c = this->getLegacyColor(i);
        r += SkColorGetR(c);
        g += SkColorGetG(c);
        b += SkColorGetB(c);
    }
    *lum = SkColorSetRGB(rounded_divide(r, n), rounded_divide(g, n), rounded_divide(b, n));
    return true;
}

SkGradientShaderBase::AutoXformColors::AutoXformColors(const SkGradientShaderBase& grad,
                                                       SkColorSpaceXformer* xformer)
    : fColors(grad.fColorCount) {
    // TODO: stay in 4f to preserve precision?

    SkAutoSTMalloc<8, SkColor> origColors(grad.fColorCount);
    for (int i = 0; i < grad.fColorCount; ++i) {
        origColors[i] = grad.getLegacyColor(i);
    }

    xformer->apply(fColors.get(), origColors.get(), grad.fColorCount);
}

static constexpr int kGradientTextureSize = 256;

void SkGradientShaderBase::initLinearBitmap(SkBitmap* bitmap, GradientBitmapType bitmapType) const {
    const bool interpInPremul = SkToBool(fGradFlags &
                                         SkGradientShader::kInterpolateColorsInPremul_Flag);
    SkHalf* pixelsF16 = reinterpret_cast<SkHalf*>(bitmap->getPixels());
    uint32_t* pixels32 = reinterpret_cast<uint32_t*>(bitmap->getPixels());

    typedef std::function<void(const Sk4f&, int)> pixelWriteFn_t;

    pixelWriteFn_t writeF16Pixel = [&](const Sk4f& x, int index) {
        Sk4h c = SkFloatToHalf_finite_ftz(x);
        pixelsF16[4*index+0] = c[0];
        pixelsF16[4*index+1] = c[1];
        pixelsF16[4*index+2] = c[2];
        pixelsF16[4*index+3] = c[3];
    };
    pixelWriteFn_t writeS32Pixel = [&](const Sk4f& c, int index) {
        pixels32[index] = Sk4f_toS32(c);
    };
    pixelWriteFn_t writeL32Pixel = [&](const Sk4f& c, int index) {
        pixels32[index] = Sk4f_toL32(c);
    };

    pixelWriteFn_t writeSizedPixel =
        (bitmapType == GradientBitmapType::kHalfFloat) ? writeF16Pixel :
        (bitmapType == GradientBitmapType::kSRGB     ) ? writeS32Pixel : writeL32Pixel;
    pixelWriteFn_t writeUnpremulPixel = [&](const Sk4f& c, int index) {
        writeSizedPixel(c * Sk4f(c[3], c[3], c[3], 1.0f), index);
    };

    pixelWriteFn_t writePixel = interpInPremul ? writeSizedPixel : writeUnpremulPixel;

    // When not in legacy mode, we just want the original 4f colors - so we pass in
    // our own CS for identity/no transform.
    auto* cs = bitmapType != GradientBitmapType::kLegacy ? fColorSpace.get() : nullptr;

    int prevIndex = 0;
    for (int i = 1; i < fColorCount; i++) {
        // Historically, stops have been mapped to [0, 256], with 256 then nudged to the
        // next smaller value, then truncate for the texture index. This seems to produce
        // the best results for some common distributions, so we preserve the behavior.
        int nextIndex = SkTMin(this->getPos(i) * kGradientTextureSize,
                               SkIntToScalar(kGradientTextureSize - 1));

        if (nextIndex > prevIndex) {
            SkColor4f color0 = this->getXformedColor(i - 1, cs),
                      color1 = this->getXformedColor(i    , cs);
            Sk4f          c0 = Sk4f::Load(color0.vec()),
                          c1 = Sk4f::Load(color1.vec());

            if (interpInPremul) {
                c0 = c0 * Sk4f(c0[3], c0[3], c0[3], 1.0f);
                c1 = c1 * Sk4f(c1[3], c1[3], c1[3], 1.0f);
            }

            Sk4f step = Sk4f(1.0f / static_cast<float>(nextIndex - prevIndex));
            Sk4f delta = (c1 - c0) * step;

            for (int curIndex = prevIndex; curIndex <= nextIndex; ++curIndex) {
                writePixel(c0, curIndex);
                c0 += delta;
            }
        }
        prevIndex = nextIndex;
    }
    SkASSERT(prevIndex == kGradientTextureSize - 1);
}

SkColor4f SkGradientShaderBase::getXformedColor(size_t i, SkColorSpace* dstCS) const {
    return dstCS ? to_colorspace(fOrigColors4f[i], fColorSpace.get(), dstCS)
                 : SkColor4f_from_SkColor(this->getLegacyColor(i), nullptr);
}

SK_DECLARE_STATIC_MUTEX(gGradientCacheMutex);
/*
 *  Because our caller might rebuild the same (logically the same) gradient
 *  over and over, we'd like to return exactly the same "bitmap" if possible,
 *  allowing the client to utilize a cache of our bitmap (e.g. with a GPU).
 *  To do that, we maintain a private cache of built-bitmaps, based on our
 *  colors and positions.
 */
void SkGradientShaderBase::getGradientTableBitmap(SkBitmap* bitmap,
                                                  GradientBitmapType bitmapType) const {
    // build our key: [numColors + colors[] + {positions[]} + flags + colorType ]
    static_assert(sizeof(SkColor4f) % sizeof(int32_t) == 0, "");
    const int colorsAsIntCount = fColorCount * sizeof(SkColor4f) / sizeof(int32_t);
    int count = 1 + colorsAsIntCount + 1 + 1;
    if (fColorCount > 2) {
        count += fColorCount - 1;
    }

    SkAutoSTMalloc<64, int32_t> storage(count);
    int32_t* buffer = storage.get();

    *buffer++ = fColorCount;
    memcpy(buffer, fOrigColors4f, fColorCount * sizeof(SkColor4f));
    buffer += colorsAsIntCount;
    if (fColorCount > 2) {
        for (int i = 1; i < fColorCount; i++) {
            *buffer++ = SkFloat2Bits(this->getPos(i));
        }
    }
    *buffer++ = fGradFlags;
    *buffer++ = static_cast<int32_t>(bitmapType);
    SkASSERT(buffer - storage.get() == count);

    ///////////////////////////////////

    static SkGradientBitmapCache* gCache;
    // each cache cost 1K or 2K of RAM, since each bitmap will be 1x256 at either 32bpp or 64bpp
    static const int MAX_NUM_CACHED_GRADIENT_BITMAPS = 32;
    SkAutoMutexAcquire ama(gGradientCacheMutex);

    if (nullptr == gCache) {
        gCache = new SkGradientBitmapCache(MAX_NUM_CACHED_GRADIENT_BITMAPS);
    }
    size_t size = count * sizeof(int32_t);

    if (!gCache->find(storage.get(), size, bitmap)) {
        // For these cases we use the bitmap cache, but not the GradientShaderCache. So just
        // allocate and populate the bitmap's data directly.

        SkImageInfo info;
        switch (bitmapType) {
        case GradientBitmapType::kLegacy:
            info = SkImageInfo::Make(kGradientTextureSize, 1, kRGBA_8888_SkColorType,
                                     kPremul_SkAlphaType);
            break;
        case GradientBitmapType::kSRGB:
            info = SkImageInfo::Make(kGradientTextureSize, 1, kRGBA_8888_SkColorType,
                                     kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
            break;
        case GradientBitmapType::kHalfFloat:
            info = SkImageInfo::Make(kGradientTextureSize, 1, kRGBA_F16_SkColorType,
                                     kPremul_SkAlphaType, SkColorSpace::MakeSRGBLinear());
            break;
        }

        bitmap->allocPixels(info);
        this->initLinearBitmap(bitmap, bitmapType);
        gCache->add(storage.get(), size, *bitmap);
    }
}

void SkGradientShaderBase::commonAsAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColorCount >= fColorCount) {
            if (info->fColors) {
                for (int i = 0; i < fColorCount; ++i) {
                    info->fColors[i] = this->getLegacyColor(i);
                }
            }
            if (info->fColorOffsets) {
                for (int i = 0; i < fColorCount; ++i) {
                    info->fColorOffsets[i] = this->getPos(i);
                }
            }
        }
        info->fColorCount = fColorCount;
        info->fTileMode = fTileMode;
        info->fGradientFlags = fGradFlags;
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkGradientShaderBase::toString(SkString* str) const {

    str->appendf("%d colors: ", fColorCount);

    for (int i = 0; i < fColorCount; ++i) {
        str->appendHex(this->getLegacyColor(i), 8);
        if (i < fColorCount-1) {
            str->append(", ");
        }
    }

    if (fColorCount > 2) {
        str->append(" points: (");
        for (int i = 0; i < fColorCount; ++i) {
            str->appendScalar(this->getPos(i));
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

    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Return true if these parameters are valid/legal/safe to construct a gradient
//
static bool valid_grad(const SkColor4f colors[], const SkScalar pos[], int count,
                       unsigned tileMode) {
    return nullptr != colors && count >= 1 && tileMode < (unsigned)SkShader::kTileModeCount;
}

static void desc_init(SkGradientShaderBase::Descriptor* desc,
                      const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                      const SkScalar pos[], int colorCount,
                      SkShader::TileMode mode, uint32_t flags, const SkMatrix* localMatrix) {
    SkASSERT(colorCount > 1);

    desc->fColors       = colors;
    desc->fColorSpace   = std::move(colorSpace);
    desc->fPos          = pos;
    desc->fCount        = colorCount;
    desc->fTileMode     = mode;
    desc->fGradFlags    = flags;
    desc->fLocalMatrix  = localMatrix;
}

// assumes colors is SkColor4f* and pos is SkScalar*
#define EXPAND_1_COLOR(count)                \
     SkColor4f tmp[2];                       \
     do {                                    \
         if (1 == count) {                   \
             tmp[0] = tmp[1] = colors[0];    \
             colors = tmp;                   \
             pos = nullptr;                  \
             count = 2;                      \
         }                                   \
     } while (0)

struct ColorStopOptimizer {
    ColorStopOptimizer(const SkColor4f* colors, const SkScalar* pos,
                       int count, SkShader::TileMode mode)
        : fColors(colors)
        , fPos(pos)
        , fCount(count) {

            if (!pos || count != 3) {
                return;
            }

            if (SkScalarNearlyEqual(pos[0], 0.0f) &&
                SkScalarNearlyEqual(pos[1], 0.0f) &&
                SkScalarNearlyEqual(pos[2], 1.0f)) {

                if (SkShader::kRepeat_TileMode == mode ||
                    SkShader::kMirror_TileMode == mode ||
                    colors[0] == colors[1]) {

                    // Ignore the leftmost color/pos.
                    fColors += 1;
                    fPos    += 1;
                    fCount   = 2;
                }
            } else if (SkScalarNearlyEqual(pos[0], 0.0f) &&
                       SkScalarNearlyEqual(pos[1], 1.0f) &&
                       SkScalarNearlyEqual(pos[2], 1.0f)) {

                if (SkShader::kRepeat_TileMode == mode ||
                    SkShader::kMirror_TileMode == mode ||
                    colors[1] == colors[2]) {

                    // Ignore the rightmost color/pos.
                    fCount  = 2;
                }
            }
    }

    const SkColor4f* fColors;
    const SkScalar*  fPos;
    int              fCount;
};

struct ColorConverter {
    ColorConverter(const SkColor* colors, int count) {
        for (int i = 0; i < count; ++i) {
            fColors4f.push_back(SkColor4f::FromColor(colors[i]));
        }
    }

    SkSTArray<2, SkColor4f, true> fColors4f;
};

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor colors[],
                                             const SkScalar pos[], int colorCount,
                                             SkShader::TileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeLinear(pts, converter.fColors4f.begin(), nullptr, pos, colorCount, mode, flags,
                      localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[], int colorCount,
                                             SkShader::TileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    if (!pts || !SkScalarIsFinite((pts[1] - pts[0]).length())) {
        return nullptr;
    }
    if (!valid_grad(colors, pos, colorCount, mode)) {
        return nullptr;
    }
    if (1 == colorCount) {
        return SkShader::MakeColorShader(colors[0], std::move(colorSpace));
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }

    ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc;
    desc_init(&desc, opt.fColors, std::move(colorSpace), opt.fPos, opt.fCount, mode, flags,
              localMatrix);
    return sk_make_sp<SkLinearGradient>(pts, desc);
}

sk_sp<SkShader> SkGradientShader::MakeRadial(const SkPoint& center, SkScalar radius,
                                             const SkColor colors[],
                                             const SkScalar pos[], int colorCount,
                                             SkShader::TileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeRadial(center, radius, converter.fColors4f.begin(), nullptr, pos, colorCount, mode,
                      flags, localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeRadial(const SkPoint& center, SkScalar radius,
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[], int colorCount,
                                             SkShader::TileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    if (radius <= 0) {
        return nullptr;
    }
    if (!valid_grad(colors, pos, colorCount, mode)) {
        return nullptr;
    }
    if (1 == colorCount) {
        return SkShader::MakeColorShader(colors[0], std::move(colorSpace));
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }

    ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc;
    desc_init(&desc, opt.fColors, std::move(colorSpace), opt.fPos, opt.fCount, mode, flags,
              localMatrix);
    return sk_make_sp<SkRadialGradient>(center, radius, desc);
}

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start,
                                                      SkScalar startRadius,
                                                      const SkPoint& end,
                                                      SkScalar endRadius,
                                                      const SkColor colors[],
                                                      const SkScalar pos[],
                                                      int colorCount,
                                                      SkShader::TileMode mode,
                                                      uint32_t flags,
                                                      const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeTwoPointConical(start, startRadius, end, endRadius, converter.fColors4f.begin(),
                               nullptr, pos, colorCount, mode, flags, localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start,
                                                      SkScalar startRadius,
                                                      const SkPoint& end,
                                                      SkScalar endRadius,
                                                      const SkColor4f colors[],
                                                      sk_sp<SkColorSpace> colorSpace,
                                                      const SkScalar pos[],
                                                      int colorCount,
                                                      SkShader::TileMode mode,
                                                      uint32_t flags,
                                                      const SkMatrix* localMatrix) {
    if (startRadius < 0 || endRadius < 0) {
        return nullptr;
    }
    if (SkScalarNearlyZero((start - end).length()) && SkScalarNearlyZero(startRadius)) {
        // We can treat this gradient as radial, which is faster.
        return MakeRadial(start, endRadius, colors, std::move(colorSpace), pos, colorCount,
                          mode, flags, localMatrix);
    }
    if (!valid_grad(colors, pos, colorCount, mode)) {
        return nullptr;
    }
    if (startRadius == endRadius) {
        if (start == end || startRadius == 0) {
            return SkShader::MakeEmptyShader();
        }
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }
    EXPAND_1_COLOR(colorCount);

    ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc;
    desc_init(&desc, opt.fColors, std::move(colorSpace), opt.fPos, opt.fCount, mode, flags,
              localMatrix);
    return SkTwoPointConicalGradient::Create(start, startRadius, end, endRadius, desc);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor colors[],
                                            const SkScalar pos[],
                                            int colorCount,
                                            SkShader::TileMode mode,
                                            SkScalar startAngle,
                                            SkScalar endAngle,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeSweep(cx, cy, converter.fColors4f.begin(), nullptr, pos, colorCount,
                     mode, startAngle, endAngle, flags, localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor4f colors[],
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkScalar pos[],
                                            int colorCount,
                                            SkShader::TileMode mode,
                                            SkScalar startAngle,
                                            SkScalar endAngle,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    if (!valid_grad(colors, pos, colorCount, mode)) {
        return nullptr;
    }
    if (1 == colorCount) {
        return SkShader::MakeColorShader(colors[0], std::move(colorSpace));
    }
    if (startAngle >= endAngle) {
        return nullptr;
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }

    if (startAngle <= 0 && endAngle >= 360) {
        // If the t-range includes [0,1], then we can always use clamping (presumably faster).
        mode = SkShader::kClamp_TileMode;
    }

    ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc;
    desc_init(&desc, opt.fColors, std::move(colorSpace), opt.fPos, opt.fCount, mode, flags,
              localMatrix);

    const SkScalar t0 = startAngle / 360,
                   t1 =   endAngle / 360;

    return sk_make_sp<SkSweepGradient>(SkPoint::Make(cx, cy), t0, t1, desc);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkGradientShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLinearGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkRadialGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSweepGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTwoPointConicalGradient)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrColorSpaceXform.h"
#include "GrContext.h"
#include "GrShaderCaps.h"
#include "GrTextureStripAtlas.h"
#include "gl/GrGLContext.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkGr.h"

static inline int color_type_to_color_count(GrGradientEffect::ColorType colorType) {
    switch (colorType) {
        case GrGradientEffect::kSingleHardStop_ColorType:
            return 4;
        case GrGradientEffect::kHardStopLeftEdged_ColorType:
        case GrGradientEffect::kHardStopRightEdged_ColorType:
            return 3;
        case GrGradientEffect::kTwo_ColorType:
            return 2;
        case GrGradientEffect::kThree_ColorType:
            return 3;
        case GrGradientEffect::kTexture_ColorType:
            return 0;
    }

    SkDEBUGFAIL("Unhandled ColorType in color_type_to_color_count()");
    return -1;
}

GrGradientEffect::ColorType GrGradientEffect::determineColorType(
        const SkGradientShaderBase& shader) {
    if (shader.fOrigPos) {
        if (4 == shader.fColorCount) {
            if (SkScalarNearlyEqual(shader.fOrigPos[0], 0.0f) &&
                SkScalarNearlyEqual(shader.fOrigPos[1], shader.fOrigPos[2]) &&
                SkScalarNearlyEqual(shader.fOrigPos[3], 1.0f)) {

                return kSingleHardStop_ColorType;
            }
        } else if (3 == shader.fColorCount) {
            if (SkScalarNearlyEqual(shader.fOrigPos[0], 0.0f) &&
                SkScalarNearlyEqual(shader.fOrigPos[1], 0.0f) &&
                SkScalarNearlyEqual(shader.fOrigPos[2], 1.0f)) {

                return kHardStopLeftEdged_ColorType;
            } else if (SkScalarNearlyEqual(shader.fOrigPos[0], 0.0f) &&
                       SkScalarNearlyEqual(shader.fOrigPos[1], 1.0f) &&
                       SkScalarNearlyEqual(shader.fOrigPos[2], 1.0f)) {

                return kHardStopRightEdged_ColorType;
            }
        }
    }

    if (2 == shader.fColorCount) {
        return kTwo_ColorType;
    } else if (3 == shader.fColorCount) {
        return kThree_ColorType;
    }

    return kTexture_ColorType;
}

void GrGradientEffect::GLSLProcessor::emitUniforms(GrGLSLUniformHandler* uniformHandler,
                                                   const GrGradientEffect& ge) {
    if (int colorCount = color_type_to_color_count(ge.getColorType())) {
        fColorsUni = uniformHandler->addUniformArray(kFragment_GrShaderFlag,
                                                     kHalf4_GrSLType,
                                                     "Colors",
                                                     colorCount);
        if (kSingleHardStop_ColorType == ge.fColorType || kThree_ColorType == ge.fColorType) {
            fExtraStopT = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat4_GrSLType,
                                                     kHigh_GrSLPrecision, "ExtraStopT");
        }
    } else {
        fFSYUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                             "GradientYCoordFS");
    }
}

void GrGradientEffect::GLSLProcessor::onSetData(const GrGLSLProgramDataManager& pdman,
                                                const GrFragmentProcessor& processor) {
    const GrGradientEffect& e = processor.cast<GrGradientEffect>();

    switch (e.getColorType()) {
        case GrGradientEffect::kSingleHardStop_ColorType:
        case GrGradientEffect::kThree_ColorType:
            // ( t, 1/t, 1/(1-t), t/(1-t) )
            // This lets us compute relative t on either side of the stop with at most a single FMA
            pdman.set4f(fExtraStopT, e.fPositions[1],
                                     1.0f / e.fPositions[1],
                                     1.0f / (1.0f - e.fPositions[1]),
                                     e.fPositions[1] / (1.0f - e.fPositions[1]));
            // fall through
        case GrGradientEffect::kHardStopLeftEdged_ColorType:
        case GrGradientEffect::kHardStopRightEdged_ColorType:
        case GrGradientEffect::kTwo_ColorType: {
            pdman.set4fv(fColorsUni, e.fColors4f.count(), (float*)&e.fColors4f[0]);
            break;
        }

        case GrGradientEffect::kTexture_ColorType: {
            SkScalar yCoord = e.getYCoord();
            if (yCoord != fCachedYCoord) {
                pdman.set1f(fFSYUni, yCoord);
                fCachedYCoord = yCoord;
            }
            break;
        }
    }
}

uint32_t GrGradientEffect::GLSLProcessor::GenBaseGradientKey(const GrProcessor& processor) {
    const GrGradientEffect& e = processor.cast<GrGradientEffect>();

    uint32_t key = 0;

    if (GrGradientEffect::kBeforeInterp_PremulType == e.getPremulType()) {
        key |= kPremulBeforeInterpKey;
    }

    if (GrGradientEffect::kTwo_ColorType == e.getColorType()) {
        key |= kTwoColorKey;
    } else if (GrGradientEffect::kThree_ColorType == e.getColorType()) {
        key |= kThreeColorKey;
    } else if (GrGradientEffect::kSingleHardStop_ColorType == e.getColorType()) {
        key |= kHardStopCenteredKey;
    } else if (GrGradientEffect::kHardStopLeftEdged_ColorType == e.getColorType()) {
        key |= kHardStopZeroZeroOneKey;
    } else if (GrGradientEffect::kHardStopRightEdged_ColorType == e.getColorType()) {
        key |= kHardStopZeroOneOneKey;
    }

    switch (e.fWrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            key |= kClampTileMode;
            break;
        case GrSamplerState::WrapMode::kRepeat:
            key |= kRepeatTileMode;
            break;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            key |= kMirrorTileMode;
            break;
    }

    return key;
}

void GrGradientEffect::GLSLProcessor::emitAnalyticalColor(GrGLSLFPFragmentBuilder* fragBuilder,
                                                          GrGLSLUniformHandler* uniformHandler,
                                                          const GrShaderCaps* shaderCaps,
                                                          const GrGradientEffect& ge,
                                                          const char* t,
                                                          const char* outputColor,
                                                          const char* inputColor) {
    // First, apply tiling rules.
    switch (ge.fWrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            fragBuilder->codeAppendf("half clamp_t = clamp(%s, 0.0, 1.0);", t);
            break;
        case GrSamplerState::WrapMode::kRepeat:
            fragBuilder->codeAppendf("half clamp_t = fract(%s);", t);
            break;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            fragBuilder->codeAppendf("half t_1 = %s - 1.0;", t);
            fragBuilder->codeAppendf("half clamp_t = abs(t_1 - 2.0 * floor(t_1 * 0.5) - 1.0);");
            break;
    }

    // Calculate the color.
    const char* colors = uniformHandler->getUniformCStr(fColorsUni);
    switch (ge.getColorType()) {
        case kSingleHardStop_ColorType: {
            // (t, 1/t, 1/(1-t), t/(1-t))
            const char* stopT = uniformHandler->getUniformCStr(fExtraStopT);

            fragBuilder->codeAppend ("half4 start, end;");
            fragBuilder->codeAppend ("half relative_t;");
            fragBuilder->codeAppendf("if (clamp_t < %s.x) {", stopT);
            fragBuilder->codeAppendf("    start = %s[0];", colors);
            fragBuilder->codeAppendf("    end   = %s[1];", colors);
            fragBuilder->codeAppendf("    relative_t = clamp_t * %s.y;", stopT);
            fragBuilder->codeAppend ("} else {");
            fragBuilder->codeAppendf("    start = %s[2];", colors);
            fragBuilder->codeAppendf("    end   = %s[3];", colors);
            // Want: (t-s)/(1-s), but arrange it as: t/(1-s) - s/(1-s), for FMA form
            fragBuilder->codeAppendf("    relative_t = (clamp_t * %s.z) - %s.w;", stopT, stopT);
            fragBuilder->codeAppend ("}");
            fragBuilder->codeAppend ("half4 colorTemp = mix(start, end, relative_t);");

            break;
        }

        case kHardStopLeftEdged_ColorType: {
            fragBuilder->codeAppendf("half4 colorTemp = mix(%s[1], %s[2], clamp_t);", colors,
                                     colors);
            if (GrSamplerState::WrapMode::kClamp == ge.fWrapMode) {
                fragBuilder->codeAppendf("if (%s < 0.0) {", t);
                fragBuilder->codeAppendf("    colorTemp = %s[0];", colors);
                fragBuilder->codeAppendf("}");
            }

            break;
        }

        case kHardStopRightEdged_ColorType: {
            fragBuilder->codeAppendf("half4 colorTemp = mix(%s[0], %s[1], clamp_t);", colors,
                                     colors);
            if (GrSamplerState::WrapMode::kClamp == ge.fWrapMode) {
                fragBuilder->codeAppendf("if (%s > 1.0) {", t);
                fragBuilder->codeAppendf("    colorTemp = %s[2];", colors);
                fragBuilder->codeAppendf("}");
            }

            break;
        }

        case kTwo_ColorType: {
            fragBuilder->codeAppendf("half4 colorTemp = mix(%s[0], %s[1], clamp_t);",
                                     colors, colors);

            break;
        }

        case kThree_ColorType: {
            // (t, 1/t, 1/(1-t), t/(1-t))
            const char* stopT = uniformHandler->getUniformCStr(fExtraStopT);

            fragBuilder->codeAppend("half4 start, end;");
            fragBuilder->codeAppend("half relative_t;");
            fragBuilder->codeAppendf("if (clamp_t < %s.x) {", stopT);
            fragBuilder->codeAppendf("    start = %s[0];", colors);
            fragBuilder->codeAppendf("    end   = %s[1];", colors);
            fragBuilder->codeAppendf("    relative_t = clamp_t * %s.y;", stopT);
            fragBuilder->codeAppend("} else {");
            fragBuilder->codeAppendf("    start = %s[1];", colors);
            fragBuilder->codeAppendf("    end   = %s[2];", colors);
            // Want: (t-s)/(1-s), but arrange it as: t/(1-s) - s/(1-s), for FMA form
            fragBuilder->codeAppendf("    relative_t = (clamp_t * %s.z) - %s.w;", stopT, stopT);
            fragBuilder->codeAppend("}");
            fragBuilder->codeAppend("half4 colorTemp = mix(start, end, relative_t);");

            break;
        }

        default:
            SkASSERT(false);
            break;
    }

    // We could skip this step if all colors are known to be opaque. Two considerations:
    // The gradient SkShader reporting opaque is more restrictive than necessary in the two
    // pt case. Make sure the key reflects this optimization (and note that it can use the
    // same shader as the kBeforeInterp case).
    if (GrGradientEffect::kAfterInterp_PremulType == ge.getPremulType()) {
        fragBuilder->codeAppend("colorTemp.rgb *= colorTemp.a;");
    }

    // If the input colors were floats, or there was a color space xform, we may end up out of
    // range. The simplest solution is to always clamp our (premul) value here. We only need to
    // clamp RGB, but that causes hangs on the Tegra3 Nexus7. Clamping RGBA avoids the problem.
    fragBuilder->codeAppend("colorTemp = clamp(colorTemp, 0, colorTemp.a);");

    fragBuilder->codeAppendf("%s = %s * colorTemp;", outputColor, inputColor);
}

void GrGradientEffect::GLSLProcessor::emitColor(GrGLSLFPFragmentBuilder* fragBuilder,
                                                GrGLSLUniformHandler* uniformHandler,
                                                const GrShaderCaps* shaderCaps,
                                                const GrGradientEffect& ge,
                                                const char* gradientTValue,
                                                const char* outputColor,
                                                const char* inputColor,
                                                const TextureSamplers& texSamplers) {
    if (ge.getColorType() != kTexture_ColorType) {
        this->emitAnalyticalColor(fragBuilder, uniformHandler, shaderCaps, ge, gradientTValue,
                                  outputColor, inputColor);
        return;
    }

    const char* fsyuni = uniformHandler->getUniformCStr(fFSYUni);

    fragBuilder->codeAppendf("half2 coord = half2(%s, %s);", gradientTValue, fsyuni);
    fragBuilder->codeAppendf("%s = ", outputColor);
    fragBuilder->appendTextureLookupAndModulate(inputColor, texSamplers[0], "coord",
                                                kFloat2_GrSLType);
    fragBuilder->codeAppend(";");
}

/////////////////////////////////////////////////////////////////////

inline GrFragmentProcessor::OptimizationFlags GrGradientEffect::OptFlags(bool isOpaque) {
    return isOpaque
                   ? kPreservesOpaqueInput_OptimizationFlag |
                             kCompatibleWithCoverageAsAlpha_OptimizationFlag
                   : kCompatibleWithCoverageAsAlpha_OptimizationFlag;
}

GrGradientEffect::GrGradientEffect(ClassID classID, const CreateArgs& args, bool isOpaque)
        : INHERITED(classID, OptFlags(isOpaque)) {
    const SkGradientShaderBase& shader(*args.fShader);

    fIsOpaque = shader.isOpaque();

    fColorType = this->determineColorType(shader);
    fWrapMode = args.fWrapMode;

    if (kTexture_ColorType == fColorType) {
        // Doesn't matter how this is set, just be consistent because it is part of the effect key.
        fPremulType = kBeforeInterp_PremulType;
    } else {
        if (SkGradientShader::kInterpolateColorsInPremul_Flag & shader.getGradFlags()) {
            fPremulType = kBeforeInterp_PremulType;
        } else {
            fPremulType = kAfterInterp_PremulType;
        }

        // Convert input colors to GrColor4f, possibly premul, and apply color space xform.
        // The xform is constructed assuming floats as input, but the color space can have a
        // transfer function on it, which will be applied below.
        auto colorSpaceXform = GrColorSpaceXform::Make(shader.fColorSpace.get(),
                                                       kRGBA_float_GrPixelConfig,
                                                       args.fDstColorSpace);
        SkASSERT(shader.fOrigColors4f);
        fColors4f.setCount(shader.fColorCount);
        for (int i = 0; i < shader.fColorCount; ++i) {
            if (args.fDstColorSpace) {
                fColors4f[i] = GrColor4f::FromSkColor4f(shader.fOrigColors4f[i]);
            } else {
                GrColor grColor = SkColorToUnpremulGrColor(shader.getLegacyColor(i));
                fColors4f[i] = GrColor4f::FromGrColor(grColor);
            }

            if (kBeforeInterp_PremulType == fPremulType) {
                fColors4f[i] = fColors4f[i].premul();
            }

            if (colorSpaceXform) {
                // We defer clamping to after interpolation (see emitAnalyticalColor)
                fColors4f[i] = colorSpaceXform->unclampedXform(fColors4f[i]);
            }
        }

        if (shader.fOrigPos) {
            fPositions = SkTDArray<SkScalar>(shader.fOrigPos, shader.fColorCount);
        } else if (kThree_ColorType == fColorType) {
            const SkScalar symmetricStops[] = { 0.0f, 0.5f, 1.0f };
            fPositions = SkTDArray<SkScalar>(symmetricStops, 3);
        }
    }

    switch (fColorType) {
        case kTwo_ColorType:
        case kThree_ColorType:
        case kHardStopLeftEdged_ColorType:
        case kHardStopRightEdged_ColorType:
        case kSingleHardStop_ColorType:
            fRow = -1;
            fCoordTransform.reset(*args.fMatrix);
            break;

        case kTexture_ColorType:
            SkGradientShaderBase::GradientBitmapType bitmapType =
                SkGradientShaderBase::GradientBitmapType::kLegacy;
            if (args.fDstColorSpace) {
                // Try to use F16 if we can
                if (args.fContext->caps()->isConfigTexturable(kRGBA_half_GrPixelConfig)) {
                    bitmapType = SkGradientShaderBase::GradientBitmapType::kHalfFloat;
                } else if (args.fContext->caps()->isConfigTexturable(kSRGBA_8888_GrPixelConfig)) {
                    bitmapType = SkGradientShaderBase::GradientBitmapType::kSRGB;
                } else {
                    // This can happen, but only if someone explicitly creates an unsupported
                    // (eg sRGB) surface. Just fall back to legacy behavior.
                }
            }

            SkBitmap bitmap;
            shader.getGradientTableBitmap(&bitmap, bitmapType);
            SkASSERT(1 == bitmap.height() && SkIsPow2(bitmap.width()));


            GrTextureStripAtlas::Desc desc;
            desc.fWidth  = bitmap.width();
            desc.fHeight = 32;
            desc.fRowHeight = bitmap.height();
            desc.fContext = args.fContext;
            desc.fConfig = SkImageInfo2GrPixelConfig(bitmap.info(), *args.fContext->caps());
            fAtlas = GrTextureStripAtlas::GetAtlas(desc);
            SkASSERT(fAtlas);

            // We always filter the gradient table. Each table is one row of a texture, always
            // y-clamp.
            GrSamplerState samplerState(args.fWrapMode, GrSamplerState::Filter::kBilerp);

            fRow = fAtlas->lockRow(bitmap);
            if (-1 != fRow) {
                fYCoord = fAtlas->getYOffset(fRow)+SK_ScalarHalf*fAtlas->getNormalizedTexelHeight();
                // This is 1/2 places where auto-normalization is disabled
                fCoordTransform.reset(*args.fMatrix, fAtlas->asTextureProxyRef().get(), false);
                fTextureSampler.reset(fAtlas->asTextureProxyRef(), samplerState);
            } else {
                // In this instance we know the samplerState state is:
                //   clampY, bilerp
                // and the proxy is:
                //   exact fit, power of two in both dimensions
                // Only the x-tileMode is unknown. However, given all the other knowns we know
                // that GrMakeCachedBitmapProxy is sufficient (i.e., it won't need to be
                // extracted to a subset or mipmapped).
                sk_sp<GrTextureProxy> proxy = GrMakeCachedBitmapProxy(
                                                                args.fContext->resourceProvider(),
                                                                bitmap);
                if (!proxy) {
                    SkDebugf("Gradient won't draw. Could not create texture.");
                    return;
                }
                // This is 2/2 places where auto-normalization is disabled
                fCoordTransform.reset(*args.fMatrix, proxy.get(), false);
                fTextureSampler.reset(std::move(proxy), samplerState);
                fYCoord = SK_ScalarHalf;
            }

            this->addTextureSampler(&fTextureSampler);

            break;
    }

    this->addCoordTransform(&fCoordTransform);
}

GrGradientEffect::GrGradientEffect(const GrGradientEffect& that)
        : INHERITED(that.classID(), OptFlags(that.fIsOpaque))
        , fColors4f(that.fColors4f)
        , fPositions(that.fPositions)
        , fWrapMode(that.fWrapMode)
        , fCoordTransform(that.fCoordTransform)
        , fTextureSampler(that.fTextureSampler)
        , fYCoord(that.fYCoord)
        , fAtlas(that.fAtlas)
        , fRow(that.fRow)
        , fIsOpaque(that.fIsOpaque)
        , fColorType(that.fColorType)
        , fPremulType(that.fPremulType) {
    this->addCoordTransform(&fCoordTransform);
    if (kTexture_ColorType == fColorType) {
        this->addTextureSampler(&fTextureSampler);
    }
    if (this->useAtlas()) {
        fAtlas->lockRow(fRow);
    }
}

GrGradientEffect::~GrGradientEffect() {
    if (this->useAtlas()) {
        fAtlas->unlockRow(fRow);
    }
}

bool GrGradientEffect::onIsEqual(const GrFragmentProcessor& processor) const {
    const GrGradientEffect& ge = processor.cast<GrGradientEffect>();

    if (fWrapMode != ge.fWrapMode || fColorType != ge.getColorType()) {
        return false;
    }
    SkASSERT(this->useAtlas() == ge.useAtlas());
    if (kTexture_ColorType == fColorType) {
        if (fYCoord != ge.getYCoord()) {
            return false;
        }
    } else {
        if (kSingleHardStop_ColorType == fColorType || kThree_ColorType == fColorType) {
            if (!SkScalarNearlyEqual(ge.fPositions[1], fPositions[1])) {
                return false;
            }
        }
        if (this->getPremulType() != ge.getPremulType() ||
            this->fColors4f.count() != ge.fColors4f.count()) {
            return false;
        }

        for (int i = 0; i < this->fColors4f.count(); i++) {
            if (*this->getColors4f(i) != *ge.getColors4f(i)) {
                return false;
            }
        }
    }
    return true;
}

#if GR_TEST_UTILS
GrGradientEffect::RandomGradientParams::RandomGradientParams(SkRandom* random) {
    // Set color count to min of 2 so that we don't trigger the const color optimization and make
    // a non-gradient processor.
    fColorCount = random->nextRangeU(2, kMaxRandomGradientColors);
    fUseColors4f = random->nextBool();

    // if one color, omit stops, otherwise randomly decide whether or not to
    if (fColorCount == 1 || (fColorCount >= 2 && random->nextBool())) {
        fStops = nullptr;
    } else {
        fStops = fStopStorage;
    }

    // if using SkColor4f, attach a random (possibly null) color space (with linear gamma)
    if (fUseColors4f) {
        fColorSpace = GrTest::TestColorSpace(random);
        if (fColorSpace) {
            SkASSERT(SkColorSpace_Base::Type::kXYZ == as_CSB(fColorSpace)->type());
            fColorSpace = static_cast<SkColorSpace_XYZ*>(fColorSpace.get())->makeLinearGamma();
        }
    }

    SkScalar stop = 0.f;
    for (int i = 0; i < fColorCount; ++i) {
        if (fUseColors4f) {
            fColors4f[i].fR = random->nextUScalar1();
            fColors4f[i].fG = random->nextUScalar1();
            fColors4f[i].fB = random->nextUScalar1();
            fColors4f[i].fA = random->nextUScalar1();
        } else {
            fColors[i] = random->nextU();
        }
        if (fStops) {
            fStops[i] = stop;
            stop = i < fColorCount - 1 ? stop + random->nextUScalar1() * (1.f - stop) : 1.f;
        }
    }
    fTileMode = static_cast<SkShader::TileMode>(random->nextULessThan(SkShader::kTileModeCount));
}
#endif

#endif
