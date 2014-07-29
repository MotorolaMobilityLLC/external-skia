/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlas.h"
#include "GrGpu.h"
#include "GrLayerCache.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrPictureDeletedMessage);

#ifdef SK_DEBUG
void GrCachedLayer::validate(const GrTexture* backingTexture) const {
    SkASSERT(SK_InvalidGenID != fKey.getPictureID());
    SkASSERT(-1 != fKey.getLayerID());


    if (NULL != fTexture) {
        // If the layer is in some texture then it must occupy some rectangle
        SkASSERT(!fRect.isEmpty());
        if (!this->isAtlased()) {
            // If it isn't atlased then the rectangle should start at the origin
            SkASSERT(0.0f == fRect.fLeft && 0.0f == fRect.fTop);
        }
    } else {
        SkASSERT(fRect.isEmpty());
        SkASSERT(NULL == fPlot);
        SkASSERT(!fLocked);     // layers without a texture cannot be locked
    }

    if (NULL != fPlot) {
        // If a layer has a plot (i.e., is atlased) then it must point to
        // the backing texture. Additionally, its rect should be non-empty.
        SkASSERT(NULL != fTexture && backingTexture == fTexture);
        SkASSERT(!fRect.isEmpty());
    }

    if (fLocked) {
        // If a layer is locked it must have a texture (though it need not be
        // the atlas-backing texture) and occupy some space.
        SkASSERT(NULL != fTexture);
        SkASSERT(!fRect.isEmpty());
    }
}

class GrAutoValidateLayer : ::SkNoncopyable {
public:
    GrAutoValidateLayer(GrTexture* backingTexture, const GrCachedLayer* layer) 
        : fBackingTexture(backingTexture)
        , fLayer(layer) {
        if (NULL != fLayer) {
            fLayer->validate(backingTexture);
        }
    }
    ~GrAutoValidateLayer() {
        if (NULL != fLayer) {
            fLayer->validate(fBackingTexture);
        }
    }
    void setBackingTexture(GrTexture* backingTexture) {
        SkASSERT(NULL == fBackingTexture || fBackingTexture == backingTexture);
        fBackingTexture = backingTexture;
    }

private:
    const GrTexture* fBackingTexture;
    const GrCachedLayer* fLayer;
};
#endif

GrLayerCache::GrLayerCache(GrContext* context)
    : fContext(context) {
    this->initAtlas();
    memset(fPlotLocks, 0, sizeof(fPlotLocks));
}

GrLayerCache::~GrLayerCache() {

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        GrCachedLayer* layer = &(*iter);
        this->unlock(layer);
        SkDELETE(layer);
    }

    // The atlas only lets go of its texture when the atlas is deleted. 
    fAtlas.free();    
}

void GrLayerCache::initAtlas() {
    SkASSERT(NULL == fAtlas.get());

    SkISize textureSize = SkISize::Make(kAtlasTextureWidth, kAtlasTextureHeight);
    fAtlas.reset(SkNEW_ARGS(GrAtlas, (fContext->getGpu(), kSkia8888_GrPixelConfig,
                                      kRenderTarget_GrTextureFlagBit,
                                      textureSize, kNumPlotsX, kNumPlotsY, false)));
}

void GrLayerCache::freeAll() {

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        GrCachedLayer* layer = &(*iter);
        this->unlock(layer);
        SkDELETE(layer);
    }
    fLayerHash.rewind();

    // The atlas only lets go of its texture when the atlas is deleted. 
    fAtlas.free();
    // GrLayerCache always assumes an atlas exists so recreate it. The atlas 
    // lazily allocates a replacement texture so reallocating a new 
    // atlas here won't disrupt a GrContext::contextDestroyed or freeGpuResources.
    // TODO: Make GrLayerCache lazily allocate the atlas manager?
    this->initAtlas();
}

GrCachedLayer* GrLayerCache::createLayer(const SkPicture* picture, int layerID) {
    SkASSERT(picture->uniqueID() != SK_InvalidGenID && layerID >= 0);

    GrCachedLayer* layer = SkNEW_ARGS(GrCachedLayer, (picture->uniqueID(), layerID));
    fLayerHash.add(layer);
    return layer;
}

GrCachedLayer* GrLayerCache::findLayer(const SkPicture* picture, int layerID) {
    SkASSERT(picture->uniqueID() != SK_InvalidGenID && layerID >= 0);
    return fLayerHash.find(GrCachedLayer::Key(picture->uniqueID(), layerID));
}

GrCachedLayer* GrLayerCache::findLayerOrCreate(const SkPicture* picture, int layerID) {
    SkASSERT(picture->uniqueID() != SK_InvalidGenID && layerID >= 0);
    GrCachedLayer* layer = fLayerHash.find(GrCachedLayer::Key(picture->uniqueID(), layerID));
    if (NULL == layer) {
        layer = this->createLayer(picture, layerID);
    }

    return layer;
}

bool GrLayerCache::lock(GrCachedLayer* layer, const GrTextureDesc& desc) {
    SkDEBUGCODE(GrAutoValidateLayer avl(fAtlas->getTexture(), layer);)

    if (layer->locked()) {
        // This layer is already locked
#ifdef SK_DEBUG
        if (layer->isAtlased()) {
            // It claims to be atlased
            SkASSERT(layer->rect().width() == desc.fWidth);
            SkASSERT(layer->rect().height() == desc.fHeight);
        }
#endif
        return true;
    }

    if (layer->isAtlased()) {
        // Hooray it is still in the atlas - make sure it stays there
        layer->setLocked(true);
        fPlotLocks[layer->plot()->id()]++;
        return true;
    } else if (PlausiblyAtlasable(desc.fWidth, desc.fHeight)) {
        // Not in the atlas - will it fit?
        GrPictureInfo* pictInfo = fPictureHash.find(layer->pictureID());
        if (NULL == pictInfo) {
            pictInfo = SkNEW_ARGS(GrPictureInfo, (layer->pictureID()));
            fPictureHash.add(pictInfo);
        }

        SkIPoint16 loc;
        for (int i = 0; i < 2; ++i) { // extra pass in case we fail to add but are able to purge
            GrPlot* plot = fAtlas->addToAtlas(&pictInfo->fPlotUsage,
                                              desc.fWidth, desc.fHeight,
                                              NULL, &loc);
            // addToAtlas can allocate the backing texture
            SkDEBUGCODE(avl.setBackingTexture(fAtlas->getTexture()));
            if (NULL != plot) {
                // The layer was successfully added to the atlas
                GrIRect16 bounds = GrIRect16::MakeXYWH(loc.fX, loc.fY,
                                                       SkToS16(desc.fWidth), 
                                                       SkToS16(desc.fHeight));
                layer->setTexture(fAtlas->getTexture(), bounds);
                layer->setPlot(plot);
                layer->setLocked(true);
                fPlotLocks[layer->plot()->id()]++;
                return false;
            }

            // The layer was rejected by the atlas (even though we know it is 
            // plausibly atlas-able). See if a plot can be purged and try again.
            if (!this->purgePlot()) {
                break;  // We weren't able to purge any plots
            }
        }
    }

    // The texture wouldn't fit in the cache - give it it's own texture.
    // This path always uses a new scratch texture and (thus) doesn't cache anything.
    // This can yield a lot of re-rendering
    layer->setTexture(fContext->lockAndRefScratchTexture(desc, GrContext::kApprox_ScratchTexMatch),
                      GrIRect16::MakeWH(SkToS16(desc.fWidth), SkToS16(desc.fHeight)));
    layer->setLocked(true);
    return false;
}

void GrLayerCache::unlock(GrCachedLayer* layer) {
    SkDEBUGCODE(GrAutoValidateLayer avl(fAtlas->getTexture(), layer);)

    if (NULL == layer || !layer->locked()) {
        // invalid or not locked
        return;
    }

    if (layer->isAtlased()) {
        const int plotID = layer->plot()->id();

        SkASSERT(fPlotLocks[plotID] > 0);
        fPlotLocks[plotID]--;
        // At this point we could aggressively clear out un-locked plots but
        // by delaying we may be able to reuse some of the atlased layers later.
    } else {
        fContext->unlockScratchTexture(layer->texture());
        layer->setTexture(NULL, GrIRect16::MakeEmpty());
    }

    layer->setLocked(false);
}

#ifdef SK_DEBUG
void GrLayerCache::validate() const {
    int plotLocks[kNumPlotsX * kNumPlotsY];
    memset(plotLocks, 0, sizeof(plotLocks));

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::ConstIter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        const GrCachedLayer* layer = &(*iter);

        layer->validate(fAtlas->getTexture());

        const GrPictureInfo* pictInfo = fPictureHash.find(layer->pictureID());
        if (NULL != pictInfo) {
            // In aggressive cleanup mode a picture info should only exist if
            // it has some atlased layers
            SkASSERT(!pictInfo->fPlotUsage.isEmpty());
        } else {
            // If there is no picture info for this layer then all of its
            // layers should be non-atlased.
            SkASSERT(!layer->isAtlased());
        }

        if (NULL != layer->plot()) {
            SkASSERT(NULL != pictInfo);
            SkASSERT(pictInfo->fPictureID == layer->pictureID());

            SkASSERT(pictInfo->fPlotUsage.contains(layer->plot()));

            if (layer->locked()) {
                plotLocks[layer->plot()->id()]++;
            }
        } 
    }

    for (int i = 0; i < kNumPlotsX*kNumPlotsY; ++i) {
        SkASSERT(plotLocks[i] == fPlotLocks[i]);
    }
}

class GrAutoValidateCache : ::SkNoncopyable {
public:
    explicit GrAutoValidateCache(GrLayerCache* cache)
        : fCache(cache) {
        fCache->validate();
    }
    ~GrAutoValidateCache() {
        fCache->validate();
    }
private:
    GrLayerCache* fCache;
};
#endif

void GrLayerCache::purge(uint32_t pictureID) {

    SkDEBUGCODE(GrAutoValidateCache avc(this);)

    // We need to find all the layers associated with 'picture' and remove them.
    SkTDArray<GrCachedLayer*> toBeRemoved;

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        if (pictureID == (*iter).pictureID()) {
            *toBeRemoved.append() = &(*iter);
        }
    }

    for (int i = 0; i < toBeRemoved.count(); ++i) {
        this->unlock(toBeRemoved[i]);
        fLayerHash.remove(GrCachedLayer::GetKey(*toBeRemoved[i]));
        SkDELETE(toBeRemoved[i]);
    }

    GrPictureInfo* pictInfo = fPictureHash.find(pictureID);
    if (NULL != pictInfo) {
        fPictureHash.remove(pictureID);
        SkDELETE(pictInfo);
    }
}

bool GrLayerCache::purgePlot() {
    SkDEBUGCODE(GrAutoValidateCache avc(this);)

    GrAtlas::PlotIter iter;
    GrPlot* plot;
    for (plot = fAtlas->iterInit(&iter, GrAtlas::kLRUFirst_IterOrder);
         NULL != plot;
         plot = iter.prev()) {
        if (fPlotLocks[plot->id()] > 0) {
            continue;
        }

        // We need to find all the layers in 'plot' and remove them.
        SkTDArray<GrCachedLayer*> toBeRemoved;

        SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
        for (; !iter.done(); ++iter) {
            if (plot == (*iter).plot()) {
                *toBeRemoved.append() = &(*iter);
            }
        }

        for (int i = 0; i < toBeRemoved.count(); ++i) {
            SkASSERT(!toBeRemoved[i]->locked());

            GrPictureInfo* pictInfo = fPictureHash.find(toBeRemoved[i]->pictureID());
            SkASSERT(NULL != pictInfo);

            GrAtlas::RemovePlot(&pictInfo->fPlotUsage, plot);

            // Aggressively remove layers and, if now totally uncached, picture info
            fLayerHash.remove(GrCachedLayer::GetKey(*toBeRemoved[i]));
            SkDELETE(toBeRemoved[i]);

            if (pictInfo->fPlotUsage.isEmpty()) {
                fPictureHash.remove(pictInfo->fPictureID);
                SkDELETE(pictInfo);
            }
        }

        plot->resetRects();
        return true;
    }

    return false;
}

class GrPictureDeletionListener : public SkPicture::DeletionListener {
    virtual void onDeletion(uint32_t pictureID) SK_OVERRIDE{
        const GrPictureDeletedMessage message = { pictureID };
        SkMessageBus<GrPictureDeletedMessage>::Post(message);
    }
};

void GrLayerCache::trackPicture(const SkPicture* picture) {
    if (NULL == fDeletionListener) {
        fDeletionListener.reset(SkNEW(GrPictureDeletionListener));
    }

    picture->addDeletionListener(fDeletionListener);
}

void GrLayerCache::processDeletedPictures() {
    SkTDArray<GrPictureDeletedMessage> deletedPictures;
    fPictDeletionInbox.poll(&deletedPictures);

    for (int i = 0; i < deletedPictures.count(); i++) {
        this->purge(deletedPictures[i].pictureID);
    }
}

