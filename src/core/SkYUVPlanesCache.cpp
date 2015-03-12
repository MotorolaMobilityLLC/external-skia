/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapCache.h"
#include "SkResourceCache.h"
#include "SkYUVPlanesCache.h"

#define CHECK_LOCAL(localCache, localName, globalName, ...) \
    ((localCache) ? localCache->localName(__VA_ARGS__) : SkResourceCache::globalName(__VA_ARGS__))

namespace {
static unsigned gYUVPlanesKeyNamespaceLabel;

struct YUVValue {
    SkYUVPlanesCache::Info fInfo;
    SkCachedData*          fData;
};

struct YUVPlanesKey : public SkResourceCache::Key {
    YUVPlanesKey(uint32_t genID)
        : fGenID(genID)
    {
        this->init(&gYUVPlanesKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(genID),
                   sizeof(genID));
    }

    uint32_t fGenID;
};

struct YUVPlanesRec : public SkResourceCache::Rec {
    YUVPlanesRec(YUVPlanesKey key, SkCachedData* data, SkYUVPlanesCache::Info* info)
        : fKey(key)
    {
        fValue.fData = data;
        fValue.fInfo = *info;
        fValue.fData->attachToCacheAndRef();
    }
    ~YUVPlanesRec() {
        fValue.fData->detachFromCacheAndUnref();
    }

    YUVPlanesKey  fKey;
    YUVValue      fValue;

    const Key& getKey() const SK_OVERRIDE { return fKey; }
    size_t bytesUsed() const SK_OVERRIDE { return sizeof(*this) + fValue.fData->size(); }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* contextData) {
        const YUVPlanesRec& rec = static_cast<const YUVPlanesRec&>(baseRec);
        YUVValue* result = static_cast<YUVValue*>(contextData);

        SkCachedData* tmpData = rec.fValue.fData;
        tmpData->ref();
        if (NULL == tmpData->data()) {
            tmpData->unref();
            return false;
        }
        result->fData = tmpData;
        result->fInfo = rec.fValue.fInfo;
        return true;
    }
};
} // namespace

SkCachedData* SkYUVPlanesCache::FindAndRef(uint32_t genID, Info* info,
                                           SkResourceCache* localCache) {
    YUVValue result;
    YUVPlanesKey key(genID);
    if (!CHECK_LOCAL(localCache, find, Find, key, YUVPlanesRec::Visitor, &result)) {
        return NULL;
    }
    
    *info = result.fInfo;
    return result.fData;
}

void SkYUVPlanesCache::Add(uint32_t genID, SkCachedData* data, Info* info,
                           SkResourceCache* localCache) {
    YUVPlanesKey key(genID);
    return CHECK_LOCAL(localCache, add, Add, SkNEW_ARGS(YUVPlanesRec, (key, data, info)));
}
