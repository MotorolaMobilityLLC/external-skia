/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteGlyphCache_DEFINED
#define SkRemoteGlyphCache_DEFINED

#include <memory>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../private/SkTHash.h"
#include "SkData.h"
#include "SkDevice.h"
#include "SkDrawLooper.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkRefCnt.h"
#include "SkSerialProcs.h"
#include "SkTypeface.h"

class Serializer;
enum SkAxisAlignment : uint32_t;
class SkDescriptor;
class SkGlyphCache;
class SkGlyphRun;
class SkGlyphRunList;
struct SkPackedGlyphID;
enum SkScalerContextFlags : uint32_t;
class SkScalerContextRecDescriptor;
class SkStrikeCache;
class SkTypefaceProxy;
struct WireTypeface;

class SkStrikeServer;

struct SkDescriptorMapOperators {
    size_t operator()(const SkDescriptor* key) const;
    bool operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const;
};

template <typename T>
using SkDescriptorMap = std::unordered_map<const SkDescriptor*, T, SkDescriptorMapOperators,
                                           SkDescriptorMapOperators>;

using SkDescriptorSet =
        std::unordered_set<const SkDescriptor*, SkDescriptorMapOperators, SkDescriptorMapOperators>;

// A SkTextBlobCacheDiffCanvas is used to populate the SkStrikeServer with ops
// which will be serialized and renderered using the SkStrikeClient.
class SK_API SkTextBlobCacheDiffCanvas : public SkNoDrawCanvas {
public:
    struct SK_API Settings {
        Settings();
        ~Settings();

        bool fContextSupportsDistanceFieldText = true;
        SkScalar fMinDistanceFieldFontSize = -1.f;
        SkScalar fMaxDistanceFieldFontSize = -1.f;
        int fMaxTextureSize = 0;
        size_t fMaxTextureBytes = 0u;
    };
    SkTextBlobCacheDiffCanvas(int width, int height, const SkSurfaceProps& props,
                              SkStrikeServer* strikeserver, Settings settings = Settings());

    // TODO(khushalsagar): Remove once removed from chromium.
    SkTextBlobCacheDiffCanvas(int width, int height, const SkMatrix& deviceMatrix,
                              const SkSurfaceProps& props, SkStrikeServer* strikeserver,
                              Settings settings = Settings());
    ~SkTextBlobCacheDiffCanvas() override;

protected:
    SkCanvas::SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override;
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override;

private:
    class TrackLayerDevice : public SkNoPixelsDevice {
    public:
        TrackLayerDevice(const SkIRect& bounds, const SkSurfaceProps& props, SkStrikeServer* server,
                         const SkTextBlobCacheDiffCanvas::Settings& settings);

        SkBaseDevice* onCreateDevice(const CreateInfo& cinfo, const SkPaint*) override;

    protected:
        void drawGlyphRunList(const SkGlyphRunList& glyphRunList) override;

    private:
        void processGlyphRun(const SkPoint& origin, const SkGlyphRun& glyphRun);

        void processGlyphRunForMask(
                const SkGlyphRun& glyphRun, const SkMatrix& runMatrix, SkPoint origin);

        void processGlyphRunForPaths(const SkGlyphRun& glyphRun, const SkMatrix& runMatrix);

#if SK_SUPPORT_GPU
        bool maybeProcessGlyphRunForDFT(const SkGlyphRun& glyphRun, const SkMatrix& runMatrix);
#endif

        SkStrikeServer* const fStrikeServer;
        const SkTextBlobCacheDiffCanvas::Settings fSettings;
    };
};

using SkDiscardableHandleId = uint32_t;

// This class is not thread-safe.
class SK_API SkStrikeServer {
public:
    // An interface used by the server to create handles for pinning SkGlyphCache
    // entries on the remote client.
    class SK_API DiscardableHandleManager {
    public:
        virtual ~DiscardableHandleManager() {}

        // Creates a new *locked* handle and returns a unique ID that can be used to identify
        // it on the remote client.
        virtual SkDiscardableHandleId createHandle() = 0;

        // Returns true if the handle could be successfully locked. The server can
        // assume it will remain locked until the next set of serialized entries is
        // pulled from the SkStrikeServer.
        // If returns false, the cache entry mapped to the handle has been deleted
        // on the client. Any subsequent attempts to lock the same handle are not
        // allowed.
        virtual bool lockHandle(SkDiscardableHandleId) = 0;

        // TODO(khushalsagar): Add an API which checks whether a handle is still
        // valid without locking, so we can avoid tracking stale handles once they
        // have been purged on the remote side.
    };

    SkStrikeServer(DiscardableHandleManager* discardableHandleManager);
    ~SkStrikeServer();

    // Serializes the typeface to be remoted using this server.
    sk_sp<SkData> serializeTypeface(SkTypeface*);

    // Serializes the strike data captured using a SkTextBlobCacheDiffCanvas. Any
    // handles locked using the DiscardableHandleManager will be assumed to be
    // unlocked after this call.
    void writeStrikeData(std::vector<uint8_t>* memory);

    // Methods used internally in skia ------------------------------------------
    class SkGlyphCacheState {
    public:
        SkGlyphCacheState(std::unique_ptr<SkDescriptor> deviceDescriptor,
                          std::unique_ptr<SkDescriptor> keyDescriptor,
                          SkDiscardableHandleId discardableHandleId,
                          std::unique_ptr<SkScalerContext> scalerContext);
        ~SkGlyphCacheState();

        void addGlyph(SkPackedGlyphID, bool pathOnly);
        void writePendingGlyphs(Serializer* serializer);
        SkDiscardableHandleId discardableHandleId() const { return fDiscardableHandleId; }
        const SkDescriptor& getDeviceDescriptor() {
            return *fDeviceDescriptor;
        }
        bool isSubpixel() const { return fIsSubpixel; }
        SkAxisAlignment axisAlignmentForHText() const { return fAxisAlignmentForHText; }

        const SkDescriptor& getKeyDescriptor() {
            return *fKeyDescriptor;
        }
        const SkGlyph& findGlyph(SkPackedGlyphID);

    private:
        bool hasPendingGlyphs() const {
            return !fPendingGlyphImages.empty() || !fPendingGlyphPaths.empty();
        }
        void writeGlyphPath(const SkPackedGlyphID& glyphID, Serializer* serializer) const;

        // The set of glyphs cached on the remote client.
        SkTHashSet<SkPackedGlyphID> fCachedGlyphImages;
        SkTHashSet<SkPackedGlyphID> fCachedGlyphPaths;

        // The set of glyphs which has not yet been serialized and sent to the
        // remote client.
        std::vector<SkPackedGlyphID> fPendingGlyphImages;
        std::vector<SkPackedGlyphID> fPendingGlyphPaths;

        // The device descriptor is used to create the scaler context. The glyphs to have the
        // correct device rendering. The key descriptor is used for communication. The GPU side will
        // create descriptors with out the device filtering, thus matching the key descriptor.
        std::unique_ptr<SkDescriptor> fDeviceDescriptor;
        std::unique_ptr<SkDescriptor> fKeyDescriptor;
        const SkDiscardableHandleId fDiscardableHandleId;
        // The context built using fDeviceDescriptor
        const std::unique_ptr<SkScalerContext> fContext;
        const bool fIsSubpixel;
        const SkAxisAlignment fAxisAlignmentForHText;

        // FallbackTextHelper cases require glyph metrics when analyzing a glyph run, in which case
        // we cache them here.
        SkTHashMap<SkPackedGlyphID, SkGlyph> fGlyphMap;
    };

    SkGlyphCacheState* getOrCreateCache(const SkPaint&, const SkSurfaceProps*, const SkMatrix*,
                                        SkScalerContextFlags flags,
                                        SkScalerContextEffects* effects);

private:
    SkDescriptorMap<std::unique_ptr<SkGlyphCacheState>> fRemoteGlyphStateMap;
    DiscardableHandleManager* const fDiscardableHandleManager;
    SkTHashSet<SkFontID> fCachedTypefaces;

    // State cached until the next serialization.
    SkDescriptorSet fLockedDescs;
    std::vector<WireTypeface> fTypefacesToSend;
};

class SK_API SkStrikeClient {
public:
    // This enum is used in histogram reporting in chromium. Please don't re-order the list of
    // entries, and consider it to be append-only.
    enum CacheMissType : uint32_t {
        // Hard failures where no fallback could be found.
        kFontMetrics = 0,
        kGlyphMetrics = 1,
        kGlyphImage = 2,
        kGlyphPath = 3,

        // The original glyph could not be found and a fallback was used.
        kGlyphMetricsFallback = 4,
        kGlyphPathFallback = 5,

        kLast = kGlyphPathFallback
    };

    // An interface to delete handles that may be pinned by the remote server.
    class DiscardableHandleManager : public SkRefCnt {
    public:
        virtual ~DiscardableHandleManager() {}

        // Returns true if the handle was unlocked and can be safely deleted. Once
        // successful, subsequent attempts to delete the same handle are invalid.
        virtual bool deleteHandle(SkDiscardableHandleId) = 0;

        virtual void notifyCacheMiss(CacheMissType) {}
    };

    SkStrikeClient(sk_sp<DiscardableHandleManager>,
                   bool isLogging = true,
                   SkStrikeCache* strikeCache = nullptr);
    ~SkStrikeClient();

    // Deserializes the typeface previously serialized using the SkStrikeServer. Returns null if the
    // data is invalid.
    sk_sp<SkTypeface> deserializeTypeface(const void* data, size_t length);

    // Deserializes the strike data from a SkStrikeServer. All messages generated
    // from a server when serializing the ops must be deserialized before the op
    // is rasterized.
    // Returns false if the data is invalid.
    bool readStrikeData(const volatile void* memory, size_t memorySize);

private:
    class DiscardableStrikePinner;

    sk_sp<SkTypeface> addTypeface(const WireTypeface& wire);

    SkTHashMap<SkFontID, sk_sp<SkTypeface>> fRemoteFontIdToTypeface;
    sk_sp<DiscardableHandleManager> fDiscardableHandleManager;
    SkStrikeCache* const fStrikeCache;
    const bool fIsLogging;
};

#endif  // SkRemoteGlyphCache_DEFINED
