/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapCache.h"
#include "SkImage.h"
#include "SkResourceCache.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkRect.h"

/**
 *  Use this for bitmapcache and mipmapcache entries.
 */
uint64_t SkMakeResourceCacheSharedIDForBitmap(uint32_t bitmapGenID) {
    uint64_t sharedID = SkSetFourByteTag('b', 'm', 'a', 'p');
    return (sharedID << 32) | bitmapGenID;
}

void SkNotifyBitmapGenIDIsStale(uint32_t bitmapGenID) {
    SkResourceCache::PostPurgeSharedID(SkMakeResourceCacheSharedIDForBitmap(bitmapGenID));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBitmap::Allocator* SkBitmapCache::GetAllocator() {
    return SkResourceCache::GetAllocator();
}

/**
 This function finds the bounds of the bitmap *within its pixelRef*.
 If the bitmap lacks a pixelRef, it will return an empty rect, since
 that doesn't make sense.  This may be a useful enough function that
 it should be somewhere else (in SkBitmap?).
 */
static SkIRect get_bounds_from_bitmap(const SkBitmap& bm) {
    if (!(bm.pixelRef())) {
        return SkIRect::MakeEmpty();
    }
    SkIPoint origin = bm.pixelRefOrigin();
    return SkIRect::MakeXYWH(origin.fX, origin.fY, bm.width(), bm.height());
}

/**
 *  This function finds the bounds of the image. Today this is just the entire bounds,
 *  but in the future we may support subsets within an image, in which case this should
 *  return that subset (see get_bounds_from_bitmap).
 */
static SkIRect get_bounds_from_image(const SkImage* image) {
    return SkIRect::MakeWH(image->width(), image->height());
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkBitmap& bm, int width, int height) {
    return { bm.getGenerationID(), width, height, get_bounds_from_bitmap(bm) };
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkBitmap& bm) {
    return Make(bm, bm.width(), bm.height());
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkImage* image, int width, int height) {
    return { image->uniqueID(), width, height, get_bounds_from_image(image) };
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkImage* image) {
    return Make(image, image->width(), image->height());
}

namespace {
static unsigned gBitmapKeyNamespaceLabel;

struct BitmapKey : public SkResourceCache::Key {
public:
    BitmapKey(uint32_t genID, int width, int height, const SkIRect& bounds)
        : fDesc({ genID, width, height, bounds })
    {
        this->init(&gBitmapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(fDesc.fImageID),
                   sizeof(fDesc));
    }

    BitmapKey(const SkBitmapCacheDesc& desc) : fDesc(desc) {
        this->init(&gBitmapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(fDesc.fImageID),
                   sizeof(fDesc));
    }

    void dump() const {
        SkDebugf("-- add [%d %d] %d [%d %d %d %d]\n", fDesc.fWidth, fDesc.fHeight, fDesc.fImageID,
            fDesc.fBounds.x(), fDesc.fBounds.y(), fDesc.fBounds.width(), fDesc.fBounds.height());
    }

    const SkBitmapCacheDesc fDesc;
};

struct BitmapRec : public SkResourceCache::Rec {
    BitmapRec(uint32_t genID, int width, int height, const SkIRect& bounds, const SkBitmap& result)
        : fKey(genID, width, height, bounds)
        , fBitmap(result)
    {
#ifdef TRACE_NEW_BITMAP_CACHE_RECS
        fKey.dump();
#endif
    }

    BitmapRec(const SkBitmapCacheDesc& desc, const SkBitmap& result)
        : fKey(desc)
        , fBitmap(result)
    {
#ifdef TRACE_NEW_BITMAP_CACHE_RECS
        fKey.dump();
#endif
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(fKey) + fBitmap.getSize(); }

    const char* getCategory() const override { return "bitmap"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fBitmap.pixelRef()->diagnostic_only_getDiscardable();
    }

    static bool Finder(const SkResourceCache::Rec& baseRec, void* contextBitmap) {
        const BitmapRec& rec = static_cast<const BitmapRec&>(baseRec);
        SkBitmap* result = (SkBitmap*)contextBitmap;

        *result = rec.fBitmap;
        result->lockPixels();
        return SkToBool(result->getPixels());
    }

private:
    BitmapKey   fKey;
    SkBitmap    fBitmap;
};
} // namespace

#define CHECK_LOCAL(localCache, localName, globalName, ...) \
    ((localCache) ? localCache->localName(__VA_ARGS__) : SkResourceCache::globalName(__VA_ARGS__))

bool SkBitmapCache::FindWH(const SkBitmapCacheDesc& desc, SkBitmap* result,
                           SkResourceCache* localCache) {
    if (0 == desc.fWidth || 0 == desc.fHeight) {
        // degenerate
        return false;
    }
    return CHECK_LOCAL(localCache, find, Find, BitmapKey(desc), BitmapRec::Finder, result);
}

bool SkBitmapCache::AddWH(const SkBitmapCacheDesc& desc, const SkBitmap& result,
                          SkResourceCache* localCache) {
    if (0 == desc.fWidth || 0 == desc.fHeight) {
        // degenerate, and the key we use for mipmaps
        return false;
    }
    SkASSERT(result.isImmutable());
    BitmapRec* rec = new BitmapRec(desc, result);
    CHECK_LOCAL(localCache, add, Add, rec);
    return true;
}

bool SkBitmapCache::Find(uint32_t genID, SkBitmap* result, SkResourceCache* localCache) {
    BitmapKey key(genID, SK_Scalar1, SK_Scalar1, SkIRect::MakeEmpty());

    return CHECK_LOCAL(localCache, find, Find, key, BitmapRec::Finder, result);
}

void SkBitmapCache::Add(uint32_t genID, const SkBitmap& result, SkResourceCache* localCache) {
    SkASSERT(result.isImmutable());

    BitmapRec* rec = new BitmapRec(genID, 1, 1, SkIRect::MakeEmpty(), result);

    CHECK_LOCAL(localCache, add, Add, rec);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

namespace {
static unsigned gMipMapKeyNamespaceLabel;

struct MipMapKey : public SkResourceCache::Key {
public:
    MipMapKey(uint32_t genID, SkDestinationSurfaceColorMode colorMode, const SkIRect& bounds)
        : fGenID(genID), fColorMode(static_cast<uint32_t>(colorMode)), fBounds(bounds)
    {
        this->init(&gMipMapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(genID),
                   sizeof(fGenID) + sizeof(fColorMode) + sizeof(fBounds));
    }

    uint32_t    fGenID;
    uint32_t    fColorMode;
    SkIRect     fBounds;
};

struct MipMapRec : public SkResourceCache::Rec {
    MipMapRec(const SkBitmap& src, SkDestinationSurfaceColorMode colorMode, const SkMipMap* result)
        : fKey(src.getGenerationID(), colorMode, get_bounds_from_bitmap(src))
        , fMipMap(result)
    {
        fMipMap->attachToCacheAndRef();
    }

    ~MipMapRec() override {
        fMipMap->detachFromCacheAndUnref();
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(fKey) + fMipMap->size(); }
    const char* getCategory() const override { return "mipmap"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fMipMap->diagnostic_only_getDiscardable();
    }

    static bool Finder(const SkResourceCache::Rec& baseRec, void* contextMip) {
        const MipMapRec& rec = static_cast<const MipMapRec&>(baseRec);
        const SkMipMap* mm = SkRef(rec.fMipMap);
        // the call to ref() above triggers a "lock" in the case of discardable memory,
        // which means we can now check for null (in case the lock failed).
        if (nullptr == mm->data()) {
            mm->unref();    // balance our call to ref()
            return false;
        }
        // the call must call unref() when they are done.
        *(const SkMipMap**)contextMip = mm;
        return true;
    }

private:
    MipMapKey       fKey;
    const SkMipMap* fMipMap;
};
}

const SkMipMap* SkMipMapCache::FindAndRef(const SkBitmapCacheDesc& desc,
                                          SkDestinationSurfaceColorMode colorMode,
                                          SkResourceCache* localCache) {
    // Note: we ignore width/height from desc, just need id and bounds
    MipMapKey key(desc.fImageID, colorMode, desc.fBounds);
    const SkMipMap* result;

    if (!CHECK_LOCAL(localCache, find, Find, key, MipMapRec::Finder, &result)) {
        result = nullptr;
    }
    return result;
}

static SkResourceCache::DiscardableFactory get_fact(SkResourceCache* localCache) {
    return localCache ? localCache->GetDiscardableFactory()
                      : SkResourceCache::GetDiscardableFactory();
}

const SkMipMap* SkMipMapCache::AddAndRef(const SkBitmap& src,
                                         SkDestinationSurfaceColorMode colorMode,
                                         SkResourceCache* localCache) {
    SkMipMap* mipmap = SkMipMap::Build(src, colorMode, get_fact(localCache));
    if (mipmap) {
        MipMapRec* rec = new MipMapRec(src, colorMode, mipmap);
        CHECK_LOCAL(localCache, add, Add, rec);
        src.pixelRef()->notifyAddedToCache();
    }
    return mipmap;
}
