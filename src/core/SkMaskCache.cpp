/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMaskCache.h"

#define CHECK_LOCAL(localCache, localName, globalName, ...) \
    ((localCache) ? localCache->localName(__VA_ARGS__) : SkResourceCache::globalName(__VA_ARGS__))

struct MaskValue {
    SkMask          fMask;
    SkCachedData*   fData;
};

namespace {
static unsigned gRRectBlurKeyNamespaceLabel;

struct RRectBlurKey : public SkResourceCache::Key {
public:
    RRectBlurKey(SkScalar sigma, const SkRRect& rrect, SkBlurStyle style, SkBlurQuality quality)
        : fSigma(sigma)
        , fRRect(rrect)
        , fStyle(style)
        , fQuality(quality) {
        this->init(&gRRectBlurKeyNamespaceLabel,
                   sizeof(fSigma) + sizeof(fRRect) + sizeof(fStyle) + sizeof(fQuality));
    }

    SkScalar   fSigma;
    SkRRect    fRRect;
    int32_t    fStyle;
    int32_t    fQuality;
};

struct RRectBlurRec : public SkResourceCache::Rec {
    RRectBlurRec(RRectBlurKey key, const SkMask& mask, SkCachedData* data)
        : fKey(key)
    {
        fValue.fMask = mask;
        fValue.fData = data;
        fValue.fData->attachToCacheAndRef();
    }
    ~RRectBlurRec() {
        fValue.fData->detachFromCacheAndUnref();
    }

    RRectBlurKey   fKey;
    MaskValue      fValue;

    virtual const Key& getKey() const SK_OVERRIDE { return fKey; }
    virtual size_t bytesUsed() const SK_OVERRIDE { return sizeof(*this) + fValue.fData->size(); }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* contextData) {
        const RRectBlurRec& rec = static_cast<const RRectBlurRec&>(baseRec);
        MaskValue* result = (MaskValue*)contextData;

        SkCachedData* tmpData = rec.fValue.fData;
        tmpData->ref();
        if (NULL == tmpData->data()) {
            tmpData->unref();
            return false;
        }
        *result = rec.fValue;
        return true;
    }
};
} // namespace

SkCachedData* SkMaskCache::FindAndRef(SkScalar sigma, const SkRRect& rrect, SkBlurStyle style,
                                      SkBlurQuality quality, SkMask* mask,
                                      SkResourceCache* localCache) {
    MaskValue result;
    RRectBlurKey key(sigma, rrect, style, quality);
    if (!CHECK_LOCAL(localCache, find, Find, key, RRectBlurRec::Visitor, &result)) {
        return NULL;
    }

    *mask = result.fMask;
    mask->fImage = (uint8_t*)(result.fData->data());
    return result.fData;
}

void SkMaskCache::Add(SkScalar sigma, const SkRRect& rrect, SkBlurStyle style,
                      SkBlurQuality quality, const SkMask& mask, SkCachedData* data,
                      SkResourceCache* localCache) {
    RRectBlurKey key(sigma, rrect, style, quality);
    return CHECK_LOCAL(localCache, add, Add, SkNEW_ARGS(RRectBlurRec, (key, mask, data)));
}

//////////////////////////////////////////////////////////////////////////////////////////

namespace {
static unsigned gRectsBlurKeyNamespaceLabel;

struct RectsBlurKey : public SkResourceCache::Key {
public:
    RectsBlurKey(SkScalar sigma, int count, const SkRect rects[], SkBlurStyle style)
        : fSigma(sigma)
        , fRecCount(count)
        , fStyle(style){
        SkASSERT(1 == count || 2 == count);
        fRects[0] = SkRect::MakeEmpty();
        fRects[1] = SkRect::MakeEmpty();
        for (int i = 0; i < count; i++) {
            fRects[i] = rects[i];
        }
        this->init(&gRectsBlurKeyNamespaceLabel,
                   sizeof(fSigma) + sizeof(fRecCount) + sizeof(fRects) + sizeof(fStyle));
    }

    SkScalar    fSigma;
    int         fRecCount;
    SkRect      fRects[2];
    int32_t     fStyle;
};

struct RectsBlurRec : public SkResourceCache::Rec {
    RectsBlurRec(RectsBlurKey key, const SkMask& mask, SkCachedData* data)
        : fKey(key)
    {
        fValue.fMask = mask;
        fValue.fData = data;
        fValue.fData->attachToCacheAndRef();
    }
    ~RectsBlurRec() {
        fValue.fData->detachFromCacheAndUnref();
    }

    RectsBlurKey   fKey;
    MaskValue      fValue;

    virtual const Key& getKey() const SK_OVERRIDE { return fKey; }
    virtual size_t bytesUsed() const SK_OVERRIDE { return sizeof(*this) + fValue.fData->size(); }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* contextData) {
        const RectsBlurRec& rec = static_cast<const RectsBlurRec&>(baseRec);
        MaskValue* result = (MaskValue*)contextData;

        SkCachedData* tmpData = rec.fValue.fData;
        tmpData->ref();
        if (NULL == tmpData->data()) {
            tmpData->unref();
            return false;
        }
        *result = rec.fValue;
        return true;
    }
};
} // namespace

SkCachedData* SkMaskCache::FindAndRef(SkScalar sigma, const SkRect rects[], int count,
                                      SkBlurStyle style, SkMask* mask,
                                      SkResourceCache* localCache) {
    MaskValue result;
    RectsBlurKey key(sigma, count, rects, style);
    if (!CHECK_LOCAL(localCache, find, Find, key, RectsBlurRec::Visitor, &result)) {
        return NULL;
    }

    *mask = result.fMask;
    mask->fImage = (uint8_t*)(result.fData->data());
    return result.fData;
}

void SkMaskCache::Add(SkScalar sigma, const SkRect rects[], int count, SkBlurStyle style,
                      const SkMask& mask, SkCachedData* data,
                      SkResourceCache* localCache) {
    RectsBlurKey key(sigma, count, rects, style);
    return CHECK_LOCAL(localCache, add, Add, SkNEW_ARGS(RectsBlurRec, (key, mask, data)));
}
