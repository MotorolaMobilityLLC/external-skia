/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrLayerCache.h"
#include "GrLayerHoister.h"
#include "GrRecordReplaceDraw.h"

#include "SkCanvas.h"
#include "SkGrPixelRef.h"
#include "SkRecordDraw.h"
#include "SkSurface.h"

// Create the layer information for the hoisted layer and secure the
// required texture/render target resources.
static void prepare_for_hoisting(GrLayerCache* layerCache, 
                                 const SkPicture* topLevelPicture,
                                 const GrAccelData::SaveLayerInfo& info,
                                 const SkIRect& layerRect,
                                 SkTDArray<GrHoistedLayer>* atlased,
                                 SkTDArray<GrHoistedLayer>* nonAtlased,
                                 SkTDArray<GrHoistedLayer>* recycled) {
    const SkPicture* pict = info.fPicture ? info.fPicture : topLevelPicture;

    SkMatrix combined = SkMatrix::Concat(info.fPreMat, info.fLocalMat);

    GrCachedLayer* layer = layerCache->findLayerOrCreate(pict->uniqueID(),
                                                         info.fSaveLayerOpID,
                                                         info.fRestoreOpID,
                                                         layerRect,
                                                         combined,
                                                         info.fPaint);
    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = layerRect.width();
    desc.fHeight = layerRect.height();
    desc.fConfig = kSkia8888_GrPixelConfig;
    // TODO: need to deal with sample count


    bool disallowAtlasing = info.fHasNestedLayers || info.fIsNested ||
                            (layer->paint() && layer->paint()->getImageFilter());

    bool needsRendering = layerCache->lock(layer, desc, disallowAtlasing);
    if (NULL == layer->texture()) {
        // GPU resources could not be secured for the hoisting of this layer
        return;
    }

    GrHoistedLayer* hl;

    if (needsRendering) {
        if (layer->isAtlased()) {
            hl = atlased->append();
        } else {
            hl = nonAtlased->append();
        }
    } else {
        hl = recycled->append();
    }
    
    layerCache->addUse(layer);
    hl->fLayer = layer;
    hl->fPicture = pict;
    hl->fOffset = SkIPoint::Make(layerRect.fLeft, layerRect.fTop);
    hl->fLocalMat = info.fLocalMat;
    hl->fPreMat = info.fPreMat;
}

// Return true if any layers are suitable for hoisting
bool GrLayerHoister::FindLayersToHoist(GrContext* context,
                                       const SkPicture* topLevelPicture,
                                       const SkRect& query,
                                       SkTDArray<GrHoistedLayer>* atlased,
                                       SkTDArray<GrHoistedLayer>* nonAtlased,
                                       SkTDArray<GrHoistedLayer>* recycled) {
    GrLayerCache* layerCache = context->getLayerCache();

    layerCache->processDeletedPictures();

    SkPicture::AccelData::Key key = GrAccelData::ComputeAccelDataKey();

    const SkPicture::AccelData* topLevelData = topLevelPicture->EXPERIMENTAL_getAccelData(key);
    if (!topLevelData) {
        return false;
    }

    const GrAccelData *topLevelGPUData = static_cast<const GrAccelData*>(topLevelData);
    if (0 == topLevelGPUData->numSaveLayers()) {
        return false;
    }

    bool anyHoisted = false;

    // The layer hoisting code will pre-render and cache an entire layer if most
    // of it is being used (~70%) and it will fit in a texture. This is to allow
    // such layers to be re-used for different clips/tiles. 
    // Small layers will additionally be atlased.
    // The only limitation right now is that nested layers are currently not hoisted.
    // Parent layers are hoisted but are never atlased (so that we never swap
    // away from the atlas rendertarget when generating the hoisted layers).

    atlased->setReserve(atlased->count() + topLevelGPUData->numSaveLayers());

    // Find and prepare for hoisting all the layers that intersect the query rect
    for (int i = 0; i < topLevelGPUData->numSaveLayers(); ++i) {

        const GrAccelData::SaveLayerInfo& info = topLevelGPUData->saveLayerInfo(i);

        SkRect layerRect = SkRect::Make(info.fBounds);
        if (!layerRect.intersect(query)) {
            continue;
        }

        SkIRect ir;
        layerRect.roundOut(&ir);

        // TODO: ignore perspective projected layers here!
        // TODO: once this code is more stable unsuitable layers can
        // just be omitted during the optimization stage
        if (info.fIsNested) {
            continue;
        }

        prepare_for_hoisting(layerCache, topLevelPicture, info, ir, atlased, nonAtlased, recycled);
        anyHoisted = true;
    }

    return anyHoisted;
}

static void wrap_texture(GrTexture* texture, int width, int height, SkBitmap* result) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    result->setInfo(info);
    result->setPixelRef(SkNEW_ARGS(SkGrPixelRef, (info, texture)))->unref();
}

static void convert_layers_to_replacements(const SkTDArray<GrHoistedLayer>& layers,
                                           GrReplacements* replacements) {
    // TODO: just replace GrReplacements::ReplacementInfo with GrCachedLayer?
    for (int i = 0; i < layers.count(); ++i) {
        GrCachedLayer* layer = layers[i].fLayer;
        const SkPicture* picture = layers[i].fPicture;

        SkMatrix combined = SkMatrix::Concat(layers[i].fPreMat, layers[i].fLocalMat);

        GrReplacements::ReplacementInfo* layerInfo =
                    replacements->newReplacement(picture->uniqueID(),
                                                 layer->start(),
                                                 combined);
        layerInfo->fStop = layer->stop();
        layerInfo->fPos = layers[i].fOffset;

        SkBitmap bm;
        wrap_texture(layers[i].fLayer->texture(),
                     !layers[i].fLayer->isAtlased() ? layers[i].fLayer->rect().width()
                                                    : layers[i].fLayer->texture()->width(),
                     !layers[i].fLayer->isAtlased() ? layers[i].fLayer->rect().height()
                                                    : layers[i].fLayer->texture()->height(),
                     &bm);
        layerInfo->fImage = SkImage::NewTexture(bm);

        layerInfo->fPaint = layers[i].fLayer->paint()
                                ? SkNEW_ARGS(SkPaint, (*layers[i].fLayer->paint()))
                                : NULL;

        layerInfo->fSrcRect = SkIRect::MakeXYWH(layers[i].fLayer->rect().fLeft,
                                                layers[i].fLayer->rect().fTop,
                                                layers[i].fLayer->rect().width(),
                                                layers[i].fLayer->rect().height());
    }
}

void GrLayerHoister::DrawLayers(GrContext* context,
                                const SkTDArray<GrHoistedLayer>& atlased,
                                const SkTDArray<GrHoistedLayer>& nonAtlased,
                                const SkTDArray<GrHoistedLayer>& recycled,
                                GrReplacements* replacements) {
    // Render the atlased layers that require it
    if (atlased.count() > 0) {
        // All the atlased layers are rendered into the same GrTexture
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(
                                        atlased[0].fLayer->texture()->asRenderTarget(), NULL));

        SkCanvas* atlasCanvas = surface->getCanvas();

        SkPaint clearPaint;
        clearPaint.setColor(SK_ColorTRANSPARENT);
        clearPaint.setXfermode(SkXfermode::Create(SkXfermode::kSrc_Mode))->unref();

        for (int i = 0; i < atlased.count(); ++i) {
            const GrCachedLayer* layer = atlased[i].fLayer;
            const SkPicture* pict = atlased[i].fPicture;
            const SkIPoint offset = atlased[i].fOffset;
            SkDEBUGCODE(const SkPaint* layerPaint = layer->paint();)

            SkASSERT(!layerPaint || !layerPaint->getImageFilter());

            atlasCanvas->save();

            // Add a rect clip to make sure the rendering doesn't
            // extend beyond the boundaries of the atlased sub-rect
            SkRect bound = SkRect::MakeXYWH(SkIntToScalar(layer->rect().fLeft),
                                            SkIntToScalar(layer->rect().fTop),
                                            SkIntToScalar(layer->rect().width()),
                                            SkIntToScalar(layer->rect().height()));
            atlasCanvas->clipRect(bound);

            // Since 'clear' doesn't respect the clip we need to draw a rect
            // TODO: ensure none of the atlased layers contain a clear call!
            atlasCanvas->drawRect(bound, clearPaint);

            // info.fCTM maps the layer's top/left to the origin.
            // Since this layer is atlased, the top/left corner needs
            // to be offset to the correct location in the backing texture.
            SkMatrix initialCTM;
            initialCTM.setTranslate(SkIntToScalar(-offset.fX), 
                                    SkIntToScalar(-offset.fY));
            initialCTM.postTranslate(bound.fLeft, bound.fTop);
            initialCTM.postConcat(atlased[i].fPreMat);

            atlasCanvas->translate(SkIntToScalar(-offset.fX), 
                                   SkIntToScalar(-offset.fY));
            atlasCanvas->translate(bound.fLeft, bound.fTop);
            atlasCanvas->concat(atlased[i].fPreMat);
            atlasCanvas->concat(atlased[i].fLocalMat);

            SkRecordPartialDraw(*pict->fRecord.get(), atlasCanvas, bound,
                                layer->start()+1, layer->stop(), initialCTM);

            atlasCanvas->restore();
        }

        atlasCanvas->flush();
    }

    // Render the non-atlased layers that require it
    for (int i = 0; i < nonAtlased.count(); ++i) {
        GrCachedLayer* layer = nonAtlased[i].fLayer;
        const SkPicture* pict = nonAtlased[i].fPicture;
        const SkIPoint& offset = nonAtlased[i].fOffset;

        // Each non-atlased layer has its own GrTexture
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(
                                        layer->texture()->asRenderTarget(), NULL));

        SkCanvas* layerCanvas = surface->getCanvas();

        SkASSERT(0 == layer->rect().fLeft && 0 == layer->rect().fTop);

        // Add a rect clip to make sure the rendering doesn't
        // extend beyond the boundaries of the atlased sub-rect
        SkRect bound = SkRect::MakeXYWH(SkIntToScalar(layer->rect().fLeft),
                                        SkIntToScalar(layer->rect().fTop),
                                        SkIntToScalar(layer->rect().width()),
                                        SkIntToScalar(layer->rect().height()));

        layerCanvas->clipRect(bound); // TODO: still useful?

        layerCanvas->clear(SK_ColorTRANSPARENT);

        SkMatrix initialCTM;
        initialCTM.setTranslate(SkIntToScalar(-offset.fX), SkIntToScalar(-offset.fY));
        initialCTM.postConcat(nonAtlased[i].fPreMat);

        layerCanvas->translate(SkIntToScalar(-offset.fX), SkIntToScalar(-offset.fY));
        layerCanvas->concat(nonAtlased[i].fPreMat);
        layerCanvas->concat(nonAtlased[i].fLocalMat);

        SkRecordPartialDraw(*pict->fRecord.get(), layerCanvas, bound,
                            layer->start()+1, layer->stop(), initialCTM);

        layerCanvas->flush();
    }

    convert_layers_to_replacements(atlased, replacements);
    convert_layers_to_replacements(nonAtlased, replacements);
    convert_layers_to_replacements(recycled, replacements);
}

void GrLayerHoister::UnlockLayers(GrContext* context,
                                  const SkTDArray<GrHoistedLayer>& atlased,
                                  const SkTDArray<GrHoistedLayer>& nonAtlased,
                                  const SkTDArray<GrHoistedLayer>& recycled) {
    GrLayerCache* layerCache = context->getLayerCache();

    for (int i = 0; i < atlased.count(); ++i) {
        layerCache->removeUse(atlased[i].fLayer);
    }

    for (int i = 0; i < nonAtlased.count(); ++i) {
        layerCache->removeUse(nonAtlased[i].fLayer);
    }

    for (int i = 0; i < recycled.count(); ++i) {
        layerCache->removeUse(recycled[i].fLayer);
    }

#if DISABLE_CACHING
    // This code completely clears out the atlas. It is required when
    // caching is disabled so the atlas doesn't fill up and force more
    // free floating layers
    layerCache->purgeAll();
#endif

    SkDEBUGCODE(layerCache->validate();)
}

