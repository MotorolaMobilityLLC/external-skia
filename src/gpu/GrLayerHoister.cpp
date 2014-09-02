/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrLayerCache.h"
#include "GrLayerHoister.h"
#include "SkPictureRangePlayback.h"
#include "SkCanvas.h"
#include "SkSurface.h"

// Return true if any layers are suitable for hoisting
bool GrLayerHoister::FindLayersToHoist(const GrAccelData *gpuData,
                                       const SkPicture::OperationList* ops,
                                       const SkRect& query,
                                       bool pullForward[]) {
    bool anyHoisted = false;
    for (int i = 0; i < gpuData->numSaveLayers(); ++i) {
        pullForward[i] = false;
    }

    // Layer hoisting pre-renders the entire layer since it will be cached and potentially
    // reused with different clips (e.g., in different tiles). Because of this the
    // clip will not be limiting the size of the pre-rendered layer. kSaveLayerMaxSize
    // is used to limit which clips are pre-rendered.
    static const int kSaveLayerMaxSize = 256;

    if (NULL != ops) {
        // In this case the picture has been generated with a BBH so we use
        // the BBH to limit the pre-rendering to just the layers needed to cover
        // the region being drawn
        for (int i = 0; i < ops->numOps(); ++i) {
            uint32_t offset = ops->offset(i);

            // For now we're saving all the layers in the GrAccelData so they
            // can be nested. Additionally, the nested layers appear before
            // their parent in the list.
            for (int j = 0; j < gpuData->numSaveLayers(); ++j) {
                const GrAccelData::SaveLayerInfo& info = gpuData->saveLayerInfo(j);

                if (pullForward[j]) {
                    continue;            // already pulling forward
                }

                if (offset < info.fSaveLayerOpID || offset > info.fRestoreOpID) {
                    continue;            // the op isn't in this range
                }

                // TODO: once this code is more stable unsuitable layers can
                // just be omitted during the optimization stage
                if (!info.fValid ||
                    kSaveLayerMaxSize < info.fSize.fWidth ||
                    kSaveLayerMaxSize < info.fSize.fHeight ||
                    info.fIsNested) {
                    continue;            // this layer is unsuitable
                }

                pullForward[j] = true;
                anyHoisted = true;
            }
        }
    } else {
        // In this case there is no BBH associated with the picture. Pre-render
        // all the layers that intersect the drawn region
        for (int j = 0; j < gpuData->numSaveLayers(); ++j) {
            const GrAccelData::SaveLayerInfo& info = gpuData->saveLayerInfo(j);

            SkRect layerRect = SkRect::MakeXYWH(SkIntToScalar(info.fOffset.fX),
                                                SkIntToScalar(info.fOffset.fY),
                                                SkIntToScalar(info.fSize.fWidth),
                                                SkIntToScalar(info.fSize.fHeight));

            if (!SkRect::Intersects(query, layerRect)) {
                continue;
            }

            // TODO: once this code is more stable unsuitable layers can
            // just be omitted during the optimization stage
            if (!info.fValid ||
                kSaveLayerMaxSize < info.fSize.fWidth ||
                kSaveLayerMaxSize < info.fSize.fHeight ||
                info.fIsNested) {
                continue;
            }

            pullForward[j] = true;
            anyHoisted = true;
        }
    }

    return anyHoisted;
}

void GrLayerHoister::DrawLayers(const SkPicture* picture,
                                const SkTDArray<GrCachedLayer*>& atlased,
                                const SkTDArray<GrCachedLayer*>& nonAtlased) {
    // Render the atlased layers that require it
    if (atlased.count() > 0) {
        // All the atlased layers are rendered into the same GrTexture
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(
                                                atlased[0]->texture()->asRenderTarget(),
                                                SkSurface::kStandard_TextRenderMode,
                                                SkSurface::kDontClear_RenderTargetFlag));

        SkCanvas* atlasCanvas = surface->getCanvas();

        SkPaint paint;
        paint.setColor(SK_ColorTRANSPARENT);
        paint.setXfermode(SkXfermode::Create(SkXfermode::kSrc_Mode))->unref();

        for (int i = 0; i < atlased.count(); ++i) {
            GrCachedLayer* layer = atlased[i];

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
            atlasCanvas->drawRect(bound, paint);

            // info.fCTM maps the layer's top/left to the origin.
            // Since this layer is atlased, the top/left corner needs
            // to be offset to the correct location in the backing texture.
            atlasCanvas->translate(bound.fLeft, bound.fTop);
            atlasCanvas->concat(layer->ctm());

            SkPictureRangePlayback rangePlayback(picture,
                                                 layer->start(),
                                                 layer->stop());
            rangePlayback.draw(atlasCanvas, NULL);

            atlasCanvas->restore();
        }

        atlasCanvas->flush();
    }

    // Render the non-atlased layers that require it
    for (int i = 0; i < nonAtlased.count(); ++i) {
        GrCachedLayer* layer = nonAtlased[i];

        // Each non-atlased layer has its own GrTexture
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(
            layer->texture()->asRenderTarget(),
            SkSurface::kStandard_TextRenderMode,
            SkSurface::kDontClear_RenderTargetFlag));

        SkCanvas* layerCanvas = surface->getCanvas();

        // Add a rect clip to make sure the rendering doesn't
        // extend beyond the boundaries of the atlased sub-rect
        SkRect bound = SkRect::MakeXYWH(SkIntToScalar(layer->rect().fLeft),
                                        SkIntToScalar(layer->rect().fTop),
                                        SkIntToScalar(layer->rect().width()),
                                        SkIntToScalar(layer->rect().height()));

        layerCanvas->clipRect(bound); // TODO: still useful?

        layerCanvas->clear(SK_ColorTRANSPARENT);

        layerCanvas->concat(layer->ctm());

        SkPictureRangePlayback rangePlayback(picture,
                                             layer->start(),
                                             layer->stop());
        rangePlayback.draw(layerCanvas, NULL);

        layerCanvas->flush();
    }
}

void GrLayerHoister::UnlockLayers(GrLayerCache* layerCache, const SkPicture* picture) {
    SkPicture::AccelData::Key key = GrAccelData::ComputeAccelDataKey();

    const SkPicture::AccelData* data = picture->EXPERIMENTAL_getAccelData(key);
    SkASSERT(NULL != data);

    const GrAccelData *gpuData = static_cast<const GrAccelData*>(data);
    SkASSERT(0 != gpuData->numSaveLayers());

    // unlock the layers
    for (int i = 0; i < gpuData->numSaveLayers(); ++i) {
        const GrAccelData::SaveLayerInfo& info = gpuData->saveLayerInfo(i);

        GrCachedLayer* layer = layerCache->findLayer(picture->uniqueID(),
                                                     info.fSaveLayerOpID,
                                                     info.fRestoreOpID,
                                                     info.fOriginXform);
        layerCache->unlock(layer);
    }

#if DISABLE_CACHING
    // This code completely clears out the atlas. It is required when
    // caching is disabled so the atlas doesn't fill up and force more
    // free floating layers
    layerCache->purge(picture->uniqueID());

    layerCache->purgeAll();
#endif
}

