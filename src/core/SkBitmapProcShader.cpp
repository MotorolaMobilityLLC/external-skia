/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcShader.h"
#include "SkBitmapProcState.h"
#include "SkBitmapProvider.h"
#include "SkColorPriv.h"
#include "SkErrorInternals.h"
#include "SkPixelRef.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "SkGrPriv.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrSimpleTextureEffect.h"
#endif

static bool only_scale_and_translate(const SkMatrix& matrix) {
    unsigned mask = SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask;
    return (matrix.getType() & ~mask) == 0;
}

class BitmapProcInfoContext : public SkShader::Context {
public:
    // The context takes ownership of the info. It will call its destructor
    // but will NOT free the memory.
    BitmapProcInfoContext(const SkShader& shader, const SkShader::ContextRec& rec,
                            SkBitmapProcInfo* info)
        : INHERITED(shader, rec)
        , fInfo(info)
    {
        fFlags = 0;
        if (fInfo->fPixmap.isOpaque() && (255 == this->getPaintAlpha())) {
            fFlags |= SkShader::kOpaqueAlpha_Flag;
        }

        if (1 == fInfo->fPixmap.height() && only_scale_and_translate(this->getTotalInverse())) {
            fFlags |= SkShader::kConstInY32_Flag;
        }
    }

    ~BitmapProcInfoContext() override {
        // The bitmap proc state has been created outside of the context on memory that will be freed
        // elsewhere. Only call the destructor but leave the freeing of the memory to the caller.
        fInfo->~SkBitmapProcInfo();
    }
    
    uint32_t getFlags() const override { return fFlags; }

private:
    SkBitmapProcInfo*   fInfo;
    uint32_t            fFlags;

    typedef SkShader::Context INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BitmapProcShaderContext : public BitmapProcInfoContext {
public:
    // The context takes ownership of the state. It will call its destructor
    // but will NOT free the memory.
    BitmapProcShaderContext(const SkShader& shader, const SkShader::ContextRec& rec,
                            SkBitmapProcState* state)
        : INHERITED(shader, rec, state)
        , fState(state)
    {}

    void shadeSpan(int x, int y, SkPMColor dstC[], int count) override {
        const SkBitmapProcState& state = *fState;
        if (state.getShaderProc32()) {
            state.getShaderProc32()(&state, x, y, dstC, count);
            return;
        }

        const int BUF_MAX = 128;
        uint32_t buffer[BUF_MAX];
        SkBitmapProcState::MatrixProc   mproc = state.getMatrixProc();
        SkBitmapProcState::SampleProc32 sproc = state.getSampleProc32();
        const int max = state.maxCountForBufferSize(sizeof(buffer[0]) * BUF_MAX);

        SkASSERT(state.fPixmap.addr());

        for (;;) {
            int n = SkTMin(count, max);
            SkASSERT(n > 0 && n < BUF_MAX*2);
            mproc(state, buffer, n, x, y);
            sproc(state, buffer, n, dstC);

            if ((count -= n) == 0) {
                break;
            }
            SkASSERT(count > 0);
            x += n;
            dstC += n;
        }
    }

    ShadeProc asAShadeProc(void** ctx) override {
        if (fState->getShaderProc32()) {
            *ctx = fState;
            return (ShadeProc)fState->getShaderProc32();
        }
        return nullptr;
    }

private:
    SkBitmapProcState*  fState;

    typedef BitmapProcInfoContext INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t SkBitmapProcShader::ContextSize(const ContextRec& rec) {
    // The SkBitmapProcState is stored outside of the context object, with the context holding
    // a pointer to it.
    return SkAlign16(sizeof(BitmapProcShaderContext) + sizeof(SkBitmapProcState));
}

SkShader::Context* SkBitmapProcShader::MakeContext(const SkShader& shader,
                                                   TileMode tmx, TileMode tmy,
                                                   const SkBitmapProvider& provider,
                                                   const ContextRec& rec, void* storage) {
    SkMatrix totalInverse;
    // Do this first, so we know the matrix can be inverted.
    if (!shader.computeTotalInverse(rec, &totalInverse)) {
        return nullptr;
    }

    void* stateStorage = (char*)storage + sizeof(BitmapProcShaderContext);
    SkBitmapProcState* state = new (stateStorage) SkBitmapProcState(provider, tmx, tmy);

    SkASSERT(state);
    if (!state->setup(totalInverse, *rec.fPaint)) {
        state->~SkBitmapProcState();
        return nullptr;
    }

    return new (storage) BitmapProcShaderContext(shader, rec, state);
}

SkShader::Context* SkBitmapProcShader::onCreateContext(const ContextRec& rec, void* storage) const {
    return MakeContext(*this, (TileMode)fTileModeX, (TileMode)fTileModeY,
                       SkBitmapProvider(fRawBitmap), rec, storage);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBitmapProcShader::SkBitmapProcShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                       const SkMatrix* localMatrix)
        : INHERITED(localMatrix) {
    fRawBitmap = src;
    fTileModeX = (uint8_t)tmx;
    fTileModeY = (uint8_t)tmy;
}

bool SkBitmapProcShader::onIsABitmap(SkBitmap* texture, SkMatrix* texM, TileMode xy[]) const {
    if (texture) {
        *texture = fRawBitmap;
    }
    if (texM) {
        texM->reset();
    }
    if (xy) {
        xy[0] = (TileMode)fTileModeX;
        xy[1] = (TileMode)fTileModeY;
    }
    return true;
}

SkFlattenable* SkBitmapProcShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    SkBitmap bm;
    if (!buffer.readBitmap(&bm)) {
        return nullptr;
    }
    bm.setImmutable();
    TileMode mx = (TileMode)buffer.readUInt();
    TileMode my = (TileMode)buffer.readUInt();
    return SkShader::CreateBitmapShader(bm, mx, my, &lm);
}

void SkBitmapProcShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.writeBitmap(fRawBitmap);
    buffer.writeUInt(fTileModeX);
    buffer.writeUInt(fTileModeY);
}

bool SkBitmapProcShader::isOpaque() const {
    return fRawBitmap.isOpaque();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkUnPreMultiply.h"
#include "SkColorShader.h"
#include "SkEmptyShader.h"

// returns true and set color if the bitmap can be drawn as a single color
// (for efficiency)
static bool can_use_color_shader(const SkBitmap& bm, SkColor* color) {
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // HWUI does not support color shaders (see b/22390304)
    return false;
#endif

    if (1 != bm.width() || 1 != bm.height()) {
        return false;
    }

    SkAutoLockPixels alp(bm);
    if (!bm.readyToDraw()) {
        return false;
    }

    switch (bm.colorType()) {
        case kN32_SkColorType:
            *color = SkUnPreMultiply::PMColorToColor(*bm.getAddr32(0, 0));
            return true;
        case kRGB_565_SkColorType:
            *color = SkPixel16ToColor(*bm.getAddr16(0, 0));
            return true;
        case kIndex_8_SkColorType:
            *color = SkUnPreMultiply::PMColorToColor(bm.getIndex8Color(0, 0));
            return true;
        default: // just skip the other configs for now
            break;
    }
    return false;
}

static bool bitmap_is_too_big(const SkBitmap& bm) {
    // SkBitmapProcShader stores bitmap coordinates in a 16bit buffer, as it
    // communicates between its matrix-proc and its sampler-proc. Until we can
    // widen that, we have to reject bitmaps that are larger.
    //
    static const int kMaxSize = 65535;

    return bm.width() > kMaxSize || bm.height() > kMaxSize;
}

SkShader* SkCreateBitmapShader(const SkBitmap& src, SkShader::TileMode tmx,
                               SkShader::TileMode tmy, const SkMatrix* localMatrix,
                               SkTBlitterAllocator* allocator) {
    SkShader* shader;
    SkColor color;
    if (src.isNull() || bitmap_is_too_big(src)) {
        if (nullptr == allocator) {
            shader = new SkEmptyShader;
        } else {
            shader = allocator->createT<SkEmptyShader>();
        }
    } else if (can_use_color_shader(src, &color)) {
        if (nullptr == allocator) {
            shader = new SkColorShader(color);
        } else {
            shader = allocator->createT<SkColorShader>(color);
        }
    } else {
        if (nullptr == allocator) {
            shader = new SkBitmapProcShader(src, tmx, tmy, localMatrix);
        } else {
            shader = allocator->createT<SkBitmapProcShader>(src, tmx, tmy, localMatrix);
        }
    }
    return shader;
}

///////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
void SkBitmapProcShader::toString(SkString* str) const {
    static const char* gTileModeName[SkShader::kTileModeCount] = {
        "clamp", "repeat", "mirror"
    };

    str->append("BitmapShader: (");

    str->appendf("(%s, %s)",
                 gTileModeName[fTileModeX],
                 gTileModeName[fTileModeY]);

    str->append(" ");
    fRawBitmap.toString(str);

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrTextureAccess.h"
#include "SkGr.h"
#include "effects/GrSimpleTextureEffect.h"

const GrFragmentProcessor* SkBitmapProcShader::asFragmentProcessor(GrContext* context,
                                             const SkMatrix& viewM, const SkMatrix* localMatrix,
                                             SkFilterQuality filterQuality) const {
    SkMatrix matrix;
    matrix.setIDiv(fRawBitmap.width(), fRawBitmap.height());

    SkMatrix lmInverse;
    if (!this->getLocalMatrix().invert(&lmInverse)) {
        return nullptr;
    }
    if (localMatrix) {
        SkMatrix inv;
        if (!localMatrix->invert(&inv)) {
            return nullptr;
        }
        lmInverse.postConcat(inv);
    }
    matrix.preConcat(lmInverse);

    SkShader::TileMode tm[] = {
        (TileMode)fTileModeX,
        (TileMode)fTileModeY,
    };

    // Must set wrap and filter on the sampler before requesting a texture. In two places below
    // we check the matrix scale factors to determine how to interpret the filter quality setting.
    // This completely ignores the complexity of the drawVertices case where explicit local coords
    // are provided by the caller.
    bool doBicubic;
    GrTextureParams::FilterMode textureFilterMode =
            GrSkFilterQualityToGrFilterMode(filterQuality, viewM, this->getLocalMatrix(),
                                            &doBicubic);
    GrTextureParams params(tm, textureFilterMode);
    SkAutoTUnref<GrTexture> texture(GrRefCachedBitmapTexture(context, fRawBitmap, params));

    if (!texture) {
        SkErrorInternals::SetError( kInternalError_SkError,
                                    "Couldn't convert bitmap to texture.");
        return nullptr;
    }

    SkAutoTUnref<const GrFragmentProcessor> inner;
    if (doBicubic) {
        inner.reset(GrBicubicEffect::Create(texture, matrix, tm));
    } else {
        inner.reset(GrSimpleTextureEffect::Create(texture, matrix, params));
    }

    if (kAlpha_8_SkColorType == fRawBitmap.colorType()) {
        return GrFragmentProcessor::MulOutputByInputUnpremulColor(inner);
    }
    return GrFragmentProcessor::MulOutputByInputAlpha(inner);
}

#endif
