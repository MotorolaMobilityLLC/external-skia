
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrResourceCache.h"
#include "GrGpuResourceCacheAccess.h"
#include "SkChecksum.h"
#include "SkGr.h"
#include "SkMessageBus.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrContentKeyInvalidatedMessage);

//////////////////////////////////////////////////////////////////////////////

GrScratchKey::ResourceType GrScratchKey::GenerateResourceType() {
    static int32_t gType = INHERITED::kInvalidDomain + 1;

    int32_t type = sk_atomic_inc(&gType);
    if (type > SK_MaxU16) {
        SkFAIL("Too many Resource Types");
    }

    return static_cast<ResourceType>(type);
}

GrContentKey::Domain GrContentKey::GenerateDomain() {
    static int32_t gDomain = INHERITED::kInvalidDomain + 1;

    int32_t domain = sk_atomic_inc(&gDomain);
    if (domain > SK_MaxU16) {
        SkFAIL("Too many Content Key Domains");
    }

    return static_cast<Domain>(domain);
}
uint32_t GrResourceKeyHash(const uint32_t* data, size_t size) {
    return SkChecksum::Compute(data, size);
}

//////////////////////////////////////////////////////////////////////////////

class GrResourceCache::AutoValidate : ::SkNoncopyable {
public:
    AutoValidate(GrResourceCache* cache) : fCache(cache) { cache->validate(); }
    ~AutoValidate() { fCache->validate(); }
private:
    GrResourceCache* fCache;
};

 //////////////////////////////////////////////////////////////////////////////

static const int kDefaultMaxCount = 2 * (1 << 10);
static const size_t kDefaultMaxSize = 96 * (1 << 20);

GrResourceCache::GrResourceCache()
    : fMaxCount(kDefaultMaxCount)
    , fMaxBytes(kDefaultMaxSize)
#if GR_CACHE_STATS
    , fHighWaterCount(0)
    , fHighWaterBytes(0)
    , fBudgetedHighWaterCount(0)
    , fBudgetedHighWaterBytes(0)
#endif
    , fCount(0)
    , fBytes(0)
    , fBudgetedCount(0)
    , fBudgetedBytes(0)
    , fPurging(false)
    , fNewlyPurgeableResourceWhilePurging(false)
    , fOverBudgetCB(NULL)
    , fOverBudgetData(NULL) {
}

GrResourceCache::~GrResourceCache() {
    this->releaseAll();
}

void GrResourceCache::setLimits(int count, size_t bytes) {
    fMaxCount = count;
    fMaxBytes = bytes;
    this->purgeAsNeeded();
}

void GrResourceCache::insertResource(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!this->isInCache(resource));
    SkASSERT(!fPurging);
    fResources.addToHead(resource);

    size_t size = resource->gpuMemorySize();
    ++fCount;
    fBytes += size;
#if GR_CACHE_STATS
    fHighWaterCount = SkTMax(fCount, fHighWaterCount);
    fHighWaterBytes = SkTMax(fBytes, fHighWaterBytes);
#endif
    if (resource->resourcePriv().isBudgeted()) {
        ++fBudgetedCount;
        fBudgetedBytes += size;
#if GR_CACHE_STATS
        fBudgetedHighWaterCount = SkTMax(fBudgetedCount, fBudgetedHighWaterCount);
        fBudgetedHighWaterBytes = SkTMax(fBudgetedBytes, fBudgetedHighWaterBytes);
#endif
    }
    if (resource->resourcePriv().getScratchKey().isValid()) {
        SkASSERT(!resource->cacheAccess().isWrapped());
        fScratchMap.insert(resource->resourcePriv().getScratchKey(), resource);
    }
    
    this->purgeAsNeeded();
}

void GrResourceCache::removeResource(GrGpuResource* resource) {
    SkASSERT(this->isInCache(resource));

    size_t size = resource->gpuMemorySize();
    --fCount;
    fBytes -= size;
    if (resource->resourcePriv().isBudgeted()) {
        --fBudgetedCount;
        fBudgetedBytes -= size;
    }

    fResources.remove(resource);
    if (resource->resourcePriv().getScratchKey().isValid()) {
        fScratchMap.remove(resource->resourcePriv().getScratchKey(), resource);
    }
    if (resource->getContentKey().isValid()) {
        fContentHash.remove(resource->getContentKey());
    }
    this->validate();
}

void GrResourceCache::abandonAll() {
    AutoValidate av(this);

    SkASSERT(!fPurging);
    while (GrGpuResource* head = fResources.head()) {
        SkASSERT(!head->wasDestroyed());
        head->cacheAccess().abandon();
        // abandon should have already removed this from the list.
        SkASSERT(head != fResources.head());
    }
    SkASSERT(!fScratchMap.count());
    SkASSERT(!fContentHash.count());
    SkASSERT(!fCount);
    SkASSERT(!fBytes);
    SkASSERT(!fBudgetedCount);
    SkASSERT(!fBudgetedBytes);
}

void GrResourceCache::releaseAll() {
    AutoValidate av(this);

    SkASSERT(!fPurging);
    while (GrGpuResource* head = fResources.head()) {
        SkASSERT(!head->wasDestroyed());
        head->cacheAccess().release();
        // release should have already removed this from the list.
        SkASSERT(head != fResources.head());
    }
    SkASSERT(!fScratchMap.count());
    SkASSERT(!fCount);
    SkASSERT(!fBytes);
    SkASSERT(!fBudgetedCount);
    SkASSERT(!fBudgetedBytes);
}

class GrResourceCache::AvailableForScratchUse {
public:
    AvailableForScratchUse(bool rejectPendingIO) : fRejectPendingIO(rejectPendingIO) { }

    bool operator()(const GrGpuResource* resource) const {
        if (resource->internalHasRef() || !resource->cacheAccess().isScratch()) {
            return false;
        }
        return !fRejectPendingIO || !resource->internalHasPendingIO();
    }

private:
    bool fRejectPendingIO;
};

GrGpuResource* GrResourceCache::findAndRefScratchResource(const GrScratchKey& scratchKey,
                                                           uint32_t flags) {
    SkASSERT(!fPurging);
    SkASSERT(scratchKey.isValid());

    GrGpuResource* resource;
    if (flags & (kPreferNoPendingIO_ScratchFlag | kRequireNoPendingIO_ScratchFlag)) {
        resource = fScratchMap.find(scratchKey, AvailableForScratchUse(true));
        if (resource) {
            resource->ref();
            this->makeResourceMRU(resource);
            this->validate();
            return resource;
        } else if (flags & kRequireNoPendingIO_ScratchFlag) {
            return NULL;
        }
        // TODO: fail here when kPrefer is specified, we didn't find a resource without pending io,
        // but there is still space in our budget for the resource.
    }
    resource = fScratchMap.find(scratchKey, AvailableForScratchUse(false));
    if (resource) {
        resource->ref();
        this->makeResourceMRU(resource);
        this->validate();
    }
    return resource;
}

void GrResourceCache::willRemoveScratchKey(const GrGpuResource* resource) {
    SkASSERT(resource->resourcePriv().getScratchKey().isValid());
    fScratchMap.remove(resource->resourcePriv().getScratchKey(), resource);
}

void GrResourceCache::willRemoveContentKey(const GrGpuResource* resource) {
    // Someone has a ref to this resource in order to invalidate it. When the ref count reaches
    // zero we will get a notifyPurgable() and figure out what to do with it.
    SkASSERT(resource->getContentKey().isValid());
    fContentHash.remove(resource->getContentKey());
}

bool GrResourceCache::didSetContentKey(GrGpuResource* resource) {
    SkASSERT(!fPurging);
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));
    SkASSERT(resource->getContentKey().isValid());

    GrGpuResource* res = fContentHash.find(resource->getContentKey());
    if (NULL != res) {
        return false;
    }

    fContentHash.add(resource);
    this->validate();
    return true;
}

void GrResourceCache::makeResourceMRU(GrGpuResource* resource) {
    SkASSERT(!fPurging);
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));
    fResources.remove(resource);    
    fResources.addToHead(resource);
}

void GrResourceCache::notifyPurgeable(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));
    SkASSERT(resource->isPurgeable());

    // We can't purge if in the middle of purging because purge is iterating. Instead record
    // that additional resources became purgeable.
    if (fPurging) {
        fNewlyPurgeableResourceWhilePurging = true;
        return;
    }

    bool release = false;

    if (resource->cacheAccess().isWrapped()) {
        release = true;
    } else if (!resource->resourcePriv().isBudgeted()) {
        // Check whether this resource could still be used as a scratch resource.
        if (resource->resourcePriv().getScratchKey().isValid()) {
            // We won't purge an existing resource to make room for this one.
            bool underBudget = fBudgetedCount < fMaxCount &&
                               fBudgetedBytes + resource->gpuMemorySize() <= fMaxBytes;
            if (underBudget) {
                resource->resourcePriv().makeBudgeted();
            } else {
                release = true;
            }
        } else {
            release = true;
        }
    } else {
        // Purge the resource if we're over budget
        bool overBudget = fBudgetedCount > fMaxCount || fBudgetedBytes > fMaxBytes;

        // Also purge if the resource has neither a valid scratch key nor a content key.
        bool noKey = !resource->resourcePriv().getScratchKey().isValid() &&
                     !resource->getContentKey().isValid();
        if (overBudget || noKey) {
            release = true;
        }
    }

    if (release) {
        SkDEBUGCODE(int beforeCount = fCount;)
        resource->cacheAccess().release();
        // We should at least free this resource, perhaps dependent resources as well.
        SkASSERT(fCount < beforeCount);
    }
    this->validate();
}

void GrResourceCache::didChangeGpuMemorySize(const GrGpuResource* resource, size_t oldSize) {
    // SkASSERT(!fPurging); GrPathRange increases size during flush. :(
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    ptrdiff_t delta = resource->gpuMemorySize() - oldSize;

    fBytes += delta;
#if GR_CACHE_STATS
    fHighWaterBytes = SkTMax(fBytes, fHighWaterBytes);
#endif
    if (resource->resourcePriv().isBudgeted()) {
        fBudgetedBytes += delta;
#if GR_CACHE_STATS
        fBudgetedHighWaterBytes = SkTMax(fBudgetedBytes, fBudgetedHighWaterBytes);
#endif
    }

    this->purgeAsNeeded();
    this->validate();
}

void GrResourceCache::didChangeBudgetStatus(GrGpuResource* resource) {
    SkASSERT(!fPurging);
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    size_t size = resource->gpuMemorySize();

    if (resource->resourcePriv().isBudgeted()) {
        ++fBudgetedCount;
        fBudgetedBytes += size;
#if GR_CACHE_STATS
        fBudgetedHighWaterBytes = SkTMax(fBudgetedBytes, fBudgetedHighWaterBytes);
        fBudgetedHighWaterCount = SkTMax(fBudgetedCount, fBudgetedHighWaterCount);
#endif
        this->purgeAsNeeded();
    } else {
        --fBudgetedCount;
        fBudgetedBytes -= size;
    }

    this->validate();
}

void GrResourceCache::internalPurgeAsNeeded() {
    SkASSERT(!fPurging);
    SkASSERT(!fNewlyPurgeableResourceWhilePurging);
    SkASSERT(fBudgetedCount > fMaxCount || fBudgetedBytes > fMaxBytes);

    fPurging = true;

    bool overBudget = true;
    do {
        fNewlyPurgeableResourceWhilePurging = false;
        ResourceList::Iter resourceIter;
        GrGpuResource* resource = resourceIter.init(fResources,
                                                    ResourceList::Iter::kTail_IterStart);

        while (resource) {
            GrGpuResource* prev = resourceIter.prev();
            if (resource->isPurgeable()) {
                resource->cacheAccess().release();
            }
            resource = prev;
            if (fBudgetedCount <= fMaxCount && fBudgetedBytes <= fMaxBytes) {
                overBudget = false;
                resource = NULL;
            }
        }

        if (!fNewlyPurgeableResourceWhilePurging && overBudget && fOverBudgetCB) {
            // Despite the purge we're still over budget. Call our over budget callback.
            (*fOverBudgetCB)(fOverBudgetData);
        }
    } while (overBudget && fNewlyPurgeableResourceWhilePurging);

    fNewlyPurgeableResourceWhilePurging = false;
    fPurging = false;
    this->validate();
}

void GrResourceCache::purgeAllUnlocked() {
    SkASSERT(!fPurging);
    SkASSERT(!fNewlyPurgeableResourceWhilePurging);

    fPurging = true;

    do {
        fNewlyPurgeableResourceWhilePurging = false;
        ResourceList::Iter resourceIter;
        GrGpuResource* resource =
            resourceIter.init(fResources, ResourceList::Iter::kTail_IterStart);

        while (resource) {
            GrGpuResource* prev = resourceIter.prev();
            if (resource->isPurgeable()) {
                resource->cacheAccess().release();
            }
            resource = prev;
        }

        if (!fNewlyPurgeableResourceWhilePurging && fCount && fOverBudgetCB) {
            (*fOverBudgetCB)(fOverBudgetData);
        }
    } while (fNewlyPurgeableResourceWhilePurging);
    fPurging = false;
    this->validate();
}

void GrResourceCache::processInvalidContentKeys(
    const SkTArray<GrContentKeyInvalidatedMessage>& msgs) {
    for (int i = 0; i < msgs.count(); ++i) {
        GrGpuResource* resource = this->findAndRefContentResource(msgs[i].key());
        if (resource) {
            resource->resourcePriv().removeContentKey();
            resource->unref(); // will call notifyPurgeable, if it is indeed now purgeable.
        }
    }
}

#ifdef SK_DEBUG
void GrResourceCache::validate() const {
    // Reduce the frequency of validations for large resource counts.
    static SkRandom gRandom;
    int mask = (SkNextPow2(fCount + 1) >> 5) - 1;
    if (~mask && (gRandom.nextU() & mask)) {
        return;
    }

    size_t bytes = 0;
    int count = 0;
    int budgetedCount = 0;
    size_t budgetedBytes = 0;
    int locked = 0;
    int scratch = 0;
    int couldBeScratch = 0;
    int content = 0;

    ResourceList::Iter iter;
    GrGpuResource* resource = iter.init(fResources, ResourceList::Iter::kHead_IterStart);
    for ( ; resource; resource = iter.next()) {
        bytes += resource->gpuMemorySize();
        ++count;

        if (!resource->isPurgeable()) {
            ++locked;
        }

        if (resource->cacheAccess().isScratch()) {
            SkASSERT(!resource->getContentKey().isValid());
            ++scratch;
            SkASSERT(fScratchMap.countForKey(resource->resourcePriv().getScratchKey()));
            SkASSERT(!resource->cacheAccess().isWrapped());
        } else if (resource->resourcePriv().getScratchKey().isValid()) {
            SkASSERT(!resource->resourcePriv().isBudgeted() ||
                     resource->getContentKey().isValid());
            ++couldBeScratch;
            SkASSERT(fScratchMap.countForKey(resource->resourcePriv().getScratchKey()));
            SkASSERT(!resource->cacheAccess().isWrapped());
        }
        const GrContentKey& contentKey = resource->getContentKey();
        if (contentKey.isValid()) {
            ++content;
            SkASSERT(fContentHash.find(contentKey) == resource);
            SkASSERT(!resource->cacheAccess().isWrapped());
            SkASSERT(resource->resourcePriv().isBudgeted());
        }

        if (resource->resourcePriv().isBudgeted()) {
            ++budgetedCount;
            budgetedBytes += resource->gpuMemorySize();
        }
    }

    SkASSERT(fBudgetedCount <= fCount);
    SkASSERT(fBudgetedBytes <= fBudgetedBytes);
    SkASSERT(bytes == fBytes);
    SkASSERT(count == fCount);
    SkASSERT(budgetedBytes == fBudgetedBytes);
    SkASSERT(budgetedCount == fBudgetedCount);
#if GR_CACHE_STATS
    SkASSERT(fBudgetedHighWaterCount <= fHighWaterCount);
    SkASSERT(fBudgetedHighWaterBytes <= fHighWaterBytes);
    SkASSERT(bytes <= fHighWaterBytes);
    SkASSERT(count <= fHighWaterCount);
    SkASSERT(budgetedBytes <= fBudgetedHighWaterBytes);
    SkASSERT(budgetedCount <= fBudgetedHighWaterCount);
#endif
    SkASSERT(content == fContentHash.count());
    SkASSERT(scratch + couldBeScratch == fScratchMap.count());

    // This assertion is not currently valid because we can be in recursive notifyIsPurgeable()
    // calls. This will be fixed when subresource registration is explicit.
    // bool overBudget = budgetedBytes > fMaxBytes || budgetedCount > fMaxCount;
    // SkASSERT(!overBudget || locked == count || fPurging);
}
#endif
