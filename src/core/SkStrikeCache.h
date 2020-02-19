/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrikeCache_DEFINED
#define SkStrikeCache_DEFINED

#include <unordered_map>
#include <unordered_set>

#include "include/private/SkSpinlock.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkScalerCache.h"

class SkTraceMemoryDump;

#ifndef SK_DEFAULT_FONT_CACHE_COUNT_LIMIT
    #define SK_DEFAULT_FONT_CACHE_COUNT_LIMIT   2048
#endif

#ifndef SK_DEFAULT_FONT_CACHE_LIMIT
    #define SK_DEFAULT_FONT_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

#ifndef SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT
    #define SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT  256
#endif

///////////////////////////////////////////////////////////////////////////////

class SkStrikePinner {
public:
    virtual ~SkStrikePinner() = default;
    virtual bool canDelete() = 0;
};

class SkStrikeCache final : public SkStrikeForGPUCacheInterface {
public:
    SkStrikeCache() = default;
    ~SkStrikeCache() override;

    class Strike final : public SkRefCnt, public SkStrikeForGPU {
    public:
        Strike(SkStrikeCache* strikeCache,
               const SkDescriptor& desc,
               std::unique_ptr<SkScalerContext> scaler,
               const SkFontMetrics* metrics,
               std::unique_ptr<SkStrikePinner> pinner)
                : fStrikeCache{strikeCache}
                , fScalerCache{desc, std::move(scaler), metrics}
                , fPinner{std::move(pinner)} {}

        SkGlyph* mergeGlyphAndImage(SkPackedGlyphID toID, const SkGlyph& from) {
            auto [glyph, increase] = fScalerCache.mergeGlyphAndImage(toID, from);
            this->updateDelta(increase);
            return glyph;
        }

        const SkPath* mergePath(SkGlyph* glyph, const SkPath* path) {
            auto [glyphPath, increase] = fScalerCache.mergePath(glyph, path);
            this->updateDelta(increase);
            return glyphPath;
        }

        SkScalerContext* getScalerContext() const {
            return fScalerCache.getScalerContext();
        }

        void findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                            SkGlyph* glyph, SkScalar* array, int* count) {
            fScalerCache.findIntercepts(bounds, scale, xPos, glyph, array, count);
        }

        const SkFontMetrics& getFontMetrics() const {
            return fScalerCache.getFontMetrics();
        }

        SkSpan<const SkGlyph*> metrics(SkSpan<const SkGlyphID> glyphIDs,
                                       const SkGlyph* results[]) {
            auto [glyphs, increase] = fScalerCache.metrics(glyphIDs, results);
            this->updateDelta(increase);
            return glyphs;
        }

        SkSpan<const SkGlyph*> preparePaths(SkSpan<const SkGlyphID> glyphIDs,
                                            const SkGlyph* results[]) {
            auto [glyphs, increase] = fScalerCache.preparePaths(glyphIDs, results);
            this->updateDelta(increase);
            return glyphs;
        }

        SkSpan<const SkGlyph*> prepareImages(SkSpan<const SkPackedGlyphID> glyphIDs,
                                             const SkGlyph* results[]) {
            auto [glyphs, increase] = fScalerCache.prepareImages(glyphIDs, results);
            this->updateDelta(increase);
            return glyphs;
        }

        void prepareForDrawingMasksCPU(SkDrawableGlyphBuffer* drawables) {
            size_t increase = fScalerCache.prepareForDrawingMasksCPU(drawables);
            this->updateDelta(increase);
        }

        const SkGlyphPositionRoundingSpec& roundingSpec() const override {
            return fScalerCache.roundingSpec();
        }

        const SkDescriptor& getDescriptor() const override {
            return fScalerCache.getDescriptor();
        }

        void prepareForMaskDrawing(
                SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) override {
            size_t increase = fScalerCache.prepareForMaskDrawing(drawbles, rejects);
            this->updateDelta(increase);
        }

        void prepareForSDFTDrawing(
                SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) override {
            size_t increase = fScalerCache.prepareForSDFTDrawing(drawbles, rejects);
            this->updateDelta(increase);
        }

        void prepareForPathDrawing(
                SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) override {
            size_t increase = fScalerCache.prepareForPathDrawing(drawbles, rejects);
            this->updateDelta(increase);
        }

        void onAboutToExitScope() override {
            this->unref();
        }

        void updateDelta(size_t increase) {
            if (increase != 0) {
                SkAutoSpinlock lock{fStrikeCache->fLock};
                fMemoryUsed += increase;
                if (fStrikeCache != nullptr) {
                    fStrikeCache->fTotalMemoryUsed += increase;
                }
            }
        }

        SkStrikeCache*                  fStrikeCache{nullptr};
        Strike*                         fNext{nullptr};
        Strike*                         fPrev{nullptr};
        SkScalerCache                   fScalerCache;
        std::unique_ptr<SkStrikePinner> fPinner;
        size_t                          fMemoryUsed{sizeof(SkScalerCache)};
    };  // Strike

    class ExclusiveStrikePtr {
    public:
        explicit ExclusiveStrikePtr(sk_sp<Strike> strike);
        ExclusiveStrikePtr();
        ExclusiveStrikePtr(const ExclusiveStrikePtr&) = delete;
        ExclusiveStrikePtr& operator = (const ExclusiveStrikePtr&) = delete;
        ExclusiveStrikePtr(ExclusiveStrikePtr&&);
        ExclusiveStrikePtr& operator = (ExclusiveStrikePtr&&);

        Strike* get() const;
        Strike* operator -> () const;
        Strike& operator *  () const;
        explicit operator bool () const;
        friend bool operator == (const ExclusiveStrikePtr&, const ExclusiveStrikePtr&);
        friend bool operator == (const ExclusiveStrikePtr&, decltype(nullptr));
        friend bool operator == (decltype(nullptr), const ExclusiveStrikePtr&);

    private:
        sk_sp<Strike> fStrike;
    };

    static SkStrikeCache* GlobalStrikeCache();

    ExclusiveStrikePtr findStrikeExclusive(const SkDescriptor&);

    ExclusiveStrikePtr createStrikeExclusive(
            const SkDescriptor& desc,
            std::unique_ptr<SkScalerContext> scaler,
            SkFontMetrics* maybeMetrics = nullptr,
            std::unique_ptr<SkStrikePinner> = nullptr);

    ExclusiveStrikePtr findOrCreateStrikeExclusive(
            const SkDescriptor& desc,
            const SkScalerContextEffects& effects,
            const SkTypeface& typeface);

    SkScopedStrikeForGPU findOrCreateScopedStrike(const SkDescriptor& desc,
                                                  const SkScalerContextEffects& effects,
                                                  const SkTypeface& typeface) override;

    static void PurgeAll();
    static void Dump();

    // Dump memory usage statistics of all the attaches caches in the process using the
    // SkTraceMemoryDump interface.
    static void DumpMemoryStatistics(SkTraceMemoryDump* dump);

    void purgeAll(); // does not change budget

    int getCacheCountLimit() const;
    int setCacheCountLimit(int limit);
    int getCacheCountUsed() const;

    size_t getCacheSizeLimit() const;
    size_t setCacheSizeLimit(size_t limit);
    size_t getTotalMemoryUsed() const;

    int  getCachePointSizeLimit() const;
    int  setCachePointSizeLimit(int limit);

private:
#ifdef SK_DEBUG
    // A simple accounting of what each glyph cache reports and the strike cache total.
    void validate() const SK_REQUIRES(fLock);
#else
    void validate() const {}
#endif

    sk_sp<Strike> findStrikeOrNull(const SkDescriptor& desc) SK_EXCLUDES(fLock);
    sk_sp<Strike> createStrike(
            const SkDescriptor& desc,
            std::unique_ptr<SkScalerContext> scaler,
            SkFontMetrics* maybeMetrics = nullptr,
            std::unique_ptr<SkStrikePinner> = nullptr) SK_EXCLUDES(fLock);
    sk_sp<Strike> findOrCreateStrike(
            const SkDescriptor& desc,
            const SkScalerContextEffects& effects,
            const SkTypeface& typeface) SK_EXCLUDES(fLock);

    // The following methods can only be called when mutex is already held.
    void internalRemoveStrike(Strike* strike) SK_REQUIRES(fLock);
    void internalAttachToHead(sk_sp<Strike> strike) SK_REQUIRES(fLock);

    // Checkout budgets, modulated by the specified min-bytes-needed-to-purge,
    // and attempt to purge caches to match.
    // Returns number of bytes freed.
    size_t internalPurge(size_t minBytesNeeded = 0) SK_REQUIRES(fLock);

    void forEachStrike(std::function<void(const Strike&)> visitor) const;

    mutable SkSpinlock fLock;
    Strike* fHead SK_GUARDED_BY(fLock) {nullptr};
    Strike* fTail SK_GUARDED_BY(fLock) {nullptr};
    size_t  fCacheSizeLimit{SK_DEFAULT_FONT_CACHE_LIMIT};
    size_t  fTotalMemoryUsed SK_GUARDED_BY(fLock) {0};
    int32_t fCacheCountLimit{SK_DEFAULT_FONT_CACHE_COUNT_LIMIT};
    int32_t fCacheCount SK_GUARDED_BY(fLock) {0};
    int32_t fPointSizeLimit{SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT};
};

using SkExclusiveStrikePtr = SkStrikeCache::ExclusiveStrikePtr;
using SkStrike = SkStrikeCache::Strike;

#endif  // SkStrikeCache_DEFINED
