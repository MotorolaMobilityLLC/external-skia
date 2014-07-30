/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSWMaskHelper.h"

#include "GrDrawState.h"
#include "GrDrawTargetCaps.h"
#include "GrGpu.h"

#include "SkData.h"
#include "SkStrokeRec.h"

// TODO: try to remove this #include
#include "GrContext.h"

namespace {

/*
 * Convert a boolean operation into a transfer mode code
 */
SkXfermode::Mode op_to_mode(SkRegion::Op op) {

    static const SkXfermode::Mode modeMap[] = {
        SkXfermode::kDstOut_Mode,   // kDifference_Op
        SkXfermode::kModulate_Mode, // kIntersect_Op
        SkXfermode::kSrcOver_Mode,  // kUnion_Op
        SkXfermode::kXor_Mode,      // kXOR_Op
        SkXfermode::kClear_Mode,    // kReverseDifference_Op
        SkXfermode::kSrc_Mode,      // kReplace_Op
    };

    return modeMap[op];
}

static inline GrPixelConfig fmt_to_config(SkTextureCompressor::Format fmt) {
    static const GrPixelConfig configMap[] = {
        kLATC_GrPixelConfig,       // kLATC_Format,
        kR11_EAC_GrPixelConfig,    // kR11_EAC_Format,
        kASTC_12x12_GrPixelConfig  // kASTC_12x12_Format,
    };
    GR_STATIC_ASSERT(0 == SkTextureCompressor::kLATC_Format);
    GR_STATIC_ASSERT(1 == SkTextureCompressor::kR11_EAC_Format);
    GR_STATIC_ASSERT(2 == SkTextureCompressor::kASTC_12x12_Format);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(configMap) == SkTextureCompressor::kFormatCnt);

    return configMap[fmt];
}

#if GR_COMPRESS_ALPHA_MASK
static bool choose_compressed_fmt(const GrDrawTargetCaps* caps,
                                  SkTextureCompressor::Format *fmt) {
    if (NULL == fmt) {
        return false;
    }

    // We can't use scratch textures without the ability to update
    // compressed textures...
    if (!(caps->compressedTexSubImageSupport())) {
        return false;
    }

    // Figure out what our preferred texture type is. If ASTC is available, that always
    // gives the biggest win. Otherwise, in terms of compression speed and accuracy,
    // LATC has a slight edge over R11 EAC.
    if (caps->isConfigTexturable(kASTC_12x12_GrPixelConfig)) {
        *fmt = SkTextureCompressor::kASTC_12x12_Format;
        return true;
    } else if (caps->isConfigTexturable(kLATC_GrPixelConfig)) {
        *fmt = SkTextureCompressor::kLATC_Format;
        return true;
    } else if (caps->isConfigTexturable(kR11_EAC_GrPixelConfig)) {
        *fmt = SkTextureCompressor::kR11_EAC_Format;
        return true;
    }

    return false;
}
#endif

}

/**
 * Draw a single rect element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::draw(const SkRect& rect, SkRegion::Op op,
                          bool antiAlias, uint8_t alpha) {
    SkPaint paint;

    SkXfermode* mode = SkXfermode::Create(op_to_mode(op));

    paint.setXfermode(mode);
    paint.setAntiAlias(antiAlias);
    paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));

    fDraw.drawRect(rect, paint);

    SkSafeUnref(mode);
}

/**
 * Draw a single path element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::draw(const SkPath& path, const SkStrokeRec& stroke, SkRegion::Op op,
                          bool antiAlias, uint8_t alpha) {

    SkPaint paint;
    if (stroke.isHairlineStyle()) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SK_Scalar1);
    } else {
        if (stroke.isFillStyle()) {
            paint.setStyle(SkPaint::kFill_Style);
        } else {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeJoin(stroke.getJoin());
            paint.setStrokeCap(stroke.getCap());
            paint.setStrokeWidth(stroke.getWidth());
        }
    }
    paint.setAntiAlias(antiAlias);

    if (SkRegion::kReplace_Op == op && 0xFF == alpha) {
        SkASSERT(0xFF == paint.getAlpha());
        fDraw.drawPathCoverage(path, paint);
    } else {
        paint.setXfermodeMode(op_to_mode(op));
        paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));
        fDraw.drawPath(path, paint);
    }
}

bool GrSWMaskHelper::init(const SkIRect& resultBounds,
                          const SkMatrix* matrix) {
    if (NULL != matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.setIdentity();
    }

    // Now translate so the bound's UL corner is at the origin
    fMatrix.postTranslate(-resultBounds.fLeft * SK_Scalar1,
                          -resultBounds.fTop * SK_Scalar1);
    SkIRect bounds = SkIRect::MakeWH(resultBounds.width(),
                                     resultBounds.height());

#if GR_COMPRESS_ALPHA_MASK
    fCompressMask = choose_compressed_fmt(fContext->getGpu()->caps(), &fCompressedFormat);
#else
    fCompressMask = false;
#endif

    // Make sure that the width is a multiple of 16 so that we can use
    // specialized SIMD instructions that compress 4 blocks at a time.
    int cmpWidth, cmpHeight;
    if (fCompressMask) {
        int dimX, dimY;
        SkTextureCompressor::GetBlockDimensions(fCompressedFormat, &dimX, &dimY);
        cmpWidth = dimX * ((bounds.fRight + (dimX - 1)) / dimX);
        cmpHeight = dimY * ((bounds.fBottom + (dimY - 1)) / dimY);
    } else {
        cmpWidth = bounds.fRight;
        cmpHeight = bounds.fBottom;
    }

    if (!fBM.allocPixels(SkImageInfo::MakeA8(cmpWidth, cmpHeight))) {
        return false;
    }

    sk_bzero(fBM.getPixels(), fBM.getSafeSize());

    sk_bzero(&fDraw, sizeof(fDraw));
    fRasterClip.setRect(bounds);
    fDraw.fRC    = &fRasterClip;
    fDraw.fClip  = &fRasterClip.bwRgn();
    fDraw.fMatrix = &fMatrix;
    fDraw.fBitmap = &fBM;
    return true;
}

/**
 * Get a texture (from the texture cache) of the correct size & format.
 * Return true on success; false on failure.
 */
bool GrSWMaskHelper::getTexture(GrAutoScratchTexture* texture) {
    GrTextureDesc desc;
    desc.fWidth = fBM.width();
    desc.fHeight = fBM.height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    if (fCompressMask) {

#ifdef SK_DEBUG
        int dimX, dimY;
        SkTextureCompressor::GetBlockDimensions(fCompressedFormat, &dimX, &dimY);
        SkASSERT((desc.fWidth % dimX) == 0);
        SkASSERT((desc.fHeight % dimY) == 0);
#endif

        desc.fConfig = fmt_to_config(fCompressedFormat);

        // If this config isn't supported then we should fall back to A8
        if (!(fContext->getGpu()->caps()->isConfigTexturable(desc.fConfig))) {
            SkDEBUGFAIL("Determining compression should be set from choose_compressed_fmt");
            desc.fConfig = kAlpha_8_GrPixelConfig;
        }
    }

    texture->set(fContext, desc);
    return NULL != texture->texture();
}

void GrSWMaskHelper::sendTextureData(GrTexture *texture, const GrTextureDesc& desc,
                                     const void *data, int rowbytes) {
    // If we aren't reusing scratch textures we don't need to flush before
    // writing since no one else will be using 'texture'
    bool reuseScratch = fContext->getGpu()->caps()->reuseScratchTextures();

    // Since we're uploading to it, and it's compressed, 'texture' shouldn't
    // have a render target.
    SkASSERT(NULL == texture->asRenderTarget());

    texture->writePixels(0, 0, desc.fWidth, desc.fHeight,
                         desc.fConfig, data, rowbytes,
                         reuseScratch ? 0 : GrContext::kDontFlush_PixelOpsFlag);
}

void GrSWMaskHelper::compressTextureData(GrTexture *texture, const GrTextureDesc& desc) {

    SkASSERT(GrPixelConfigIsCompressed(desc.fConfig));
    SkASSERT(fmt_to_config(fCompressedFormat) == desc.fConfig);

    SkAutoDataUnref cmpData(SkTextureCompressor::CompressBitmapToFormat(fBM, fCompressedFormat));
    SkASSERT(NULL != cmpData);

    this->sendTextureData(texture, desc, cmpData->data(), 0);
}

/**
 * Move the result of the software mask generation back to the gpu
 */
void GrSWMaskHelper::toTexture(GrTexture *texture) {
    SkAutoLockPixels alp(fBM);

    GrTextureDesc desc;
    desc.fWidth = fBM.width();
    desc.fHeight = fBM.height();
    desc.fConfig = texture->config();
        
    // First see if we should compress this texture before uploading.
    if (fCompressMask) {
        this->compressTextureData(texture, desc);
    } else {
        // Looks like we have to send a full A8 texture. 
        this->sendTextureData(texture, desc, fBM.getPixels(), fBM.rowBytes());
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Software rasterizes path to A8 mask (possibly using the context's matrix)
 * and uploads the result to a scratch texture. Returns the resulting
 * texture on success; NULL on failure.
 */
GrTexture* GrSWMaskHelper::DrawPathMaskToTexture(GrContext* context,
                                                 const SkPath& path,
                                                 const SkStrokeRec& stroke,
                                                 const SkIRect& resultBounds,
                                                 bool antiAlias,
                                                 SkMatrix* matrix) {
    GrSWMaskHelper helper(context);

    if (!helper.init(resultBounds, matrix)) {
        return NULL;
    }

    helper.draw(path, stroke, SkRegion::kReplace_Op, antiAlias, 0xFF);

    GrAutoScratchTexture ast;
    if (!helper.getTexture(&ast)) {
        return NULL;
    }

    helper.toTexture(ast.texture());

    return ast.detach();
}

void GrSWMaskHelper::DrawToTargetWithPathMask(GrTexture* texture,
                                              GrDrawTarget* target,
                                              const SkIRect& rect) {
    GrDrawState* drawState = target->drawState();

    GrDrawState::AutoViewMatrixRestore avmr;
    if (!avmr.setIdentity(drawState)) {
        return;
    }
    GrDrawState::AutoRestoreEffects are(drawState);

    SkRect dstRect = SkRect::MakeLTRB(SK_Scalar1 * rect.fLeft,
                                      SK_Scalar1 * rect.fTop,
                                      SK_Scalar1 * rect.fRight,
                                      SK_Scalar1 * rect.fBottom);

    // We want to use device coords to compute the texture coordinates. We set our matrix to be
    // equal to the view matrix followed by a translation so that the top-left of the device bounds
    // maps to 0,0, and then a scaling matrix to normalized coords. We apply this matrix to the
    // vertex positions rather than local coords.
    SkMatrix maskMatrix;
    maskMatrix.setIDiv(texture->width(), texture->height());
    maskMatrix.preTranslate(SkIntToScalar(-rect.fLeft), SkIntToScalar(-rect.fTop));
    maskMatrix.preConcat(drawState->getViewMatrix());

    drawState->addCoverageEffect(
                         GrSimpleTextureEffect::Create(texture,
                                                       maskMatrix,
                                                       GrTextureParams::kNone_FilterMode,
                                                       kPosition_GrCoordSet))->unref();

    target->drawSimpleRect(dstRect);
}
