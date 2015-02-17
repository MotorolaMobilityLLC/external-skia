
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceCache_DEFINED
#define GrResourceCache_DEFINED

#include "GrGpuResource.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrGpuResourcePriv.h"
#include "GrResourceKey.h"
#include "SkMessageBus.h"
#include "SkRefCnt.h"
#include "SkTArray.h"
#include "SkTDPQueue.h"
#include "SkTInternalLList.h"
#include "SkTMultiMap.h"

class SkString;

/**
 * Manages the lifetime of all GrGpuResource instances.
 *
 * Resources may have optionally have two types of keys:
 *      1) A scratch key. This is for resources whose allocations are cached but not their contents.
 *         Multiple resources can share the same scratch key. This is so a caller can have two
 *         resource instances with the same properties (e.g. multipass rendering that ping-pongs
 *         between two temporary surfaces. The scratch key is set at resource creation time and
 *         should never change. Resources need not have a scratch key.
 *      2) A content key. This key represents the contents of the resource rather than just its
 *         allocation properties. They may not collide. The content key can be set after resource
 *         creation. Currently it may only be set once and cannot be cleared. This restriction will
 *         be removed.
 * If a resource has neither key type then it will be deleted as soon as the last reference to it
 * is dropped. If a key has both keys the content key takes precedence.
 */
class GrResourceCache {
public:
    GrResourceCache();
    ~GrResourceCache();

    /** Used to access functionality needed by GrGpuResource for lifetime management. */
    class ResourceAccess;
    ResourceAccess resourceAccess();

    /**
     * Sets the cache limits in terms of number of resources and max gpu memory byte size.
     */
    void setLimits(int count, size_t bytes);

    /**
     * Returns the number of resources.
     */
    int getResourceCount() const {
        return fPurgeableQueue.count() + fNonpurgeableResources.count();
    }

    /**
     * Returns the number of resources that count against the budget.
     */
    int getBudgetedResourceCount() const { return fBudgetedCount; }

    /**
     * Returns the number of bytes consumed by resources.
     */
    size_t getResourceBytes() const { return fBytes; }

    /**
     * Returns the number of bytes consumed by budgeted resources.
     */
    size_t getBudgetedResourceBytes() const { return fBudgetedBytes; }

    /**
     * Returns the cached resources count budget.
     */
    int getMaxResourceCount() const { return fMaxCount; }

    /**
     * Returns the number of bytes consumed by cached resources.
     */
    size_t getMaxResourceBytes() const { return fMaxBytes; }

    /**
     * Abandons the backend API resources owned by all GrGpuResource objects and removes them from
     * the cache.
     */
    void abandonAll();

    /**
     * Releases the backend API resources owned by all GrGpuResource objects and removes them from
     * the cache.
     */
    void releaseAll();

    enum {
        /** Preferentially returns scratch resources with no pending IO. */
        kPreferNoPendingIO_ScratchFlag = 0x1,
        /** Will not return any resources that match but have pending IO. */
        kRequireNoPendingIO_ScratchFlag = 0x2,
    };

    /**
     * Find a resource that matches a scratch key.
     */
    GrGpuResource* findAndRefScratchResource(const GrScratchKey& scratchKey, uint32_t flags = 0);
    
#ifdef SK_DEBUG
    // This is not particularly fast and only used for validation, so debug only.
    int countScratchEntriesForKey(const GrScratchKey& scratchKey) const {
        return fScratchMap.countForKey(scratchKey);
    }
#endif

    /**
     * Find a resource that matches a content key.
     */
    GrGpuResource* findAndRefContentResource(const GrContentKey& contentKey) {
        GrGpuResource* resource = fContentHash.find(contentKey);
        if (resource) {
            this->refAndMakeResourceMRU(resource);
        }
        return resource;
    }

    /**
     * Query whether a content key exists in the cache.
     */
    bool hasContentKey(const GrContentKey& contentKey) const {
        return SkToBool(fContentHash.find(contentKey));
    }

    /** Purges resources to become under budget and processes resources with invalidated content
        keys. */
    void purgeAsNeeded() {
        SkTArray<GrContentKeyInvalidatedMessage> invalidKeyMsgs;
        fInvalidContentKeyInbox.poll(&invalidKeyMsgs);
        if (invalidKeyMsgs.count()) {
            this->processInvalidContentKeys(invalidKeyMsgs);
        }
        if (fBudgetedCount <= fMaxCount && fBudgetedBytes <= fMaxBytes) {
            return;
        }
        this->internalPurgeAsNeeded();
    }

    /** Purges all resources that don't have external owners. */
    void purgeAllUnlocked();

    /**
     * The callback function used by the cache when it is still over budget after a purge. The
     * passed in 'data' is the same 'data' handed to setOverbudgetCallback.
     */
    typedef void (*PFOverBudgetCB)(void* data);

    /**
     * Set the callback the cache should use when it is still over budget after a purge. The 'data'
     * provided here will be passed back to the callback. Note that the cache will attempt to purge
     * any resources newly freed by the callback.
     */
    void setOverBudgetCallback(PFOverBudgetCB overBudgetCB, void* data) {
        fOverBudgetCB = overBudgetCB;
        fOverBudgetData = data;
    }

#if GR_GPU_STATS
    void dumpStats(SkString*) const;
#endif

private:
    ///////////////////////////////////////////////////////////////////////////
    /// @name Methods accessible via ResourceAccess
    ////
    void insertResource(GrGpuResource*);
    void removeResource(GrGpuResource*);
    void notifyPurgeable(GrGpuResource*);
    void didChangeGpuMemorySize(const GrGpuResource*, size_t oldSize);
    bool didSetContentKey(GrGpuResource*);
    void willRemoveScratchKey(const GrGpuResource*);
    void willRemoveContentKey(const GrGpuResource*);
    void didChangeBudgetStatus(GrGpuResource*);
    void refAndMakeResourceMRU(GrGpuResource*);
    /// @}

    void internalPurgeAsNeeded();
    void processInvalidContentKeys(const SkTArray<GrContentKeyInvalidatedMessage>&);
    void addToNonpurgeableArray(GrGpuResource*);
    void removeFromNonpurgeableArray(GrGpuResource*);
    bool overBudget() const { return fBudgetedBytes > fMaxBytes || fBudgetedCount > fMaxCount; }

#ifdef SK_DEBUG
    bool isInCache(const GrGpuResource* r) const;
    void validate() const;
#else
    void validate() const {}
#endif

    class AutoValidate;

    class AvailableForScratchUse;

    struct ScratchMapTraits {
        static const GrScratchKey& GetKey(const GrGpuResource& r) {
            return r.resourcePriv().getScratchKey();
        }

        static uint32_t Hash(const GrScratchKey& key) { return key.hash(); }
    };
    typedef SkTMultiMap<GrGpuResource, GrScratchKey, ScratchMapTraits> ScratchMap;

    struct ContentHashTraits {
        static const GrContentKey& GetKey(const GrGpuResource& r) {
            return r.getContentKey();
        }

        static uint32_t Hash(const GrContentKey& key) { return key.hash(); }
    };
    typedef SkTDynamicHash<GrGpuResource, GrContentKey, ContentHashTraits> ContentHash;

    static bool CompareTimestamp(GrGpuResource* const& a, GrGpuResource* const& b) {
        return a->cacheAccess().timestamp() < b->cacheAccess().timestamp();
    }

    static int* AccessResourceIndex(GrGpuResource* const& res) {
        return res->cacheAccess().accessCacheIndex();
    }

    typedef SkMessageBus<GrContentKeyInvalidatedMessage>::Inbox InvalidContentKeyInbox;
    typedef SkTDPQueue<GrGpuResource*, CompareTimestamp, AccessResourceIndex> PurgeableQueue;
    typedef SkTDArray<GrGpuResource*> ResourceArray;

    // Whenever a resource is added to the cache or the result of a cache lookup, fTimestamp is
    // assigned as the resource's timestamp and then incremented. fPurgeableQueue orders the
    // purgeable resources by this value, and thus is used to purge resources in LRU order.
    uint32_t                            fTimestamp;
    PurgeableQueue                      fPurgeableQueue;
    ResourceArray                       fNonpurgeableResources;

    // This map holds all resources that can be used as scratch resources.
    ScratchMap                          fScratchMap;
    // This holds all resources that have content keys.
    ContentHash                         fContentHash;

    // our budget, used in purgeAsNeeded()
    int                                 fMaxCount;
    size_t                              fMaxBytes;

#if GR_CACHE_STATS
    int                                 fHighWaterCount;
    size_t                              fHighWaterBytes;
    int                                 fBudgetedHighWaterCount;
    size_t                              fBudgetedHighWaterBytes;
#endif

    // our current stats for all resources
    SkDEBUGCODE(int                     fCount;)
    size_t                              fBytes;

    // our current stats for resources that count against the budget
    int                                 fBudgetedCount;
    size_t                              fBudgetedBytes;

    PFOverBudgetCB                      fOverBudgetCB;
    void*                               fOverBudgetData;

    InvalidContentKeyInbox              fInvalidContentKeyInbox;
};

class GrResourceCache::ResourceAccess {
private:
    ResourceAccess(GrResourceCache* cache) : fCache(cache) { }
    ResourceAccess(const ResourceAccess& that) : fCache(that.fCache) { }
    ResourceAccess& operator=(const ResourceAccess&); // unimpl

    /**
     * Insert a resource into the cache.
     */
    void insertResource(GrGpuResource* resource) { fCache->insertResource(resource); }

    /**
     * Removes a resource from the cache.
     */
    void removeResource(GrGpuResource* resource) { fCache->removeResource(resource); }

    /**
     * Called by GrGpuResources when they detects that they are newly purgeable.
     */
    void notifyPurgeable(GrGpuResource* resource) { fCache->notifyPurgeable(resource); }

    /**
     * Called by GrGpuResources when their sizes change.
     */
    void didChangeGpuMemorySize(const GrGpuResource* resource, size_t oldSize) {
        fCache->didChangeGpuMemorySize(resource, oldSize);
    }

    /**
     * Called by GrGpuResources when their content keys change.
     *
     * This currently returns a bool and fails when an existing resource has a key that collides
     * with the new content key. In the future it will null out the content key for the existing
     * resource. The failure is a temporary measure taken because duties are split between two
     * cache objects currently.
     */
    bool didSetContentKey(GrGpuResource* resource) { return fCache->didSetContentKey(resource); }

    /**
     * Called by a GrGpuResource when it removes its content key.
     */
    void willRemoveContentKey(GrGpuResource* resource) {
        return fCache->willRemoveContentKey(resource);
    }

    /**
     * Called by a GrGpuResource when it removes its scratch key.
     */
    void willRemoveScratchKey(const GrGpuResource* resource) {
        fCache->willRemoveScratchKey(resource);
    }

    /**
     * Called by GrGpuResources when they change from budgeted to unbudgeted or vice versa.
     */
    void didChangeBudgetStatus(GrGpuResource* resource) { fCache->didChangeBudgetStatus(resource); }

    // No taking addresses of this type.
    const ResourceAccess* operator&() const;
    ResourceAccess* operator&();

    GrResourceCache* fCache;

    friend class GrGpuResource; // To access all the proxy inline methods.
    friend class GrResourceCache; // To create this type.
};

inline GrResourceCache::ResourceAccess GrResourceCache::resourceAccess() {
    return ResourceAccess(this);
}

#endif
