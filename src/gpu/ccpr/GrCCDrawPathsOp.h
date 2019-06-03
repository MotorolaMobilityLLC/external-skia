/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCDrawPathsOp_DEFINED
#define GrCCDrawPathsOp_DEFINED

#include "src/core/SkTInternalLList.h"
#include "src/gpu/ccpr/GrCCPathCache.h"
#include "src/gpu/ccpr/GrCCSTLList.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrDrawOp.h"

class GrCCAtlas;
class GrCCPerFlushResources;
struct GrCCPerFlushResourceSpecs;
struct GrCCPerOpListPaths;
class GrOnFlushResourceProvider;
class GrRecordingContext;

/**
 * This is the Op that draws paths to the actual canvas, using atlases generated by CCPR.
 */
class GrCCDrawPathsOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrCCDrawPathsOp);

    static std::unique_ptr<GrCCDrawPathsOp> Make(GrRecordingContext*, const SkIRect& clipIBounds,
                                                 const SkMatrix&, const GrShape&, GrPaint&&);
    ~GrCCDrawPathsOp() override;

    const char* name() const override { return "GrCCDrawPathsOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrFSAAType,
                                      GrClampType) override;
    CombineResult onCombineIfPossible(GrOp*, const GrCaps&) override;
    void visitProxies(const VisitProxyFunc& fn) const override {
        for (const auto& range : fInstanceRanges) {
            fn(range.fAtlasProxy, GrMipMapped::kNo);
        }
        fProcessors.visitProxies(fn);
    }
    void onPrepare(GrOpFlushState*) override {}

    void addToOwningPerOpListPaths(sk_sp<GrCCPerOpListPaths> owningPerOpListPaths);

    // Makes decisions about how to draw each path (cached, copied, rendered, etc.), and
    // increments/fills out the corresponding GrCCPerFlushResourceSpecs.
    void accountForOwnPaths(GrCCPathCache*, GrOnFlushResourceProvider*, GrCCPerFlushResourceSpecs*);

    // Allows the caller to decide whether to actually do the suggested copies from cached 16-bit
    // coverage count atlases, and into 8-bit literal coverage atlases. Purely to save space.
    enum class DoCopiesToA8Coverage : bool {
        kNo = false,
        kYes = true
    };

    // Allocates the GPU resources indicated by accountForOwnPaths(), in preparation for drawing. If
    // DoCopiesToA8Coverage is kNo, the paths slated for copy will instead be left in their 16-bit
    // coverage count atlases.
    //
    // NOTE: If using DoCopiesToA8Coverage::kNo, it is the caller's responsibility to have called
    // cancelCopies() on the GrCCPerFlushResourceSpecs, prior to making this call.
    void setupResources(GrCCPathCache*, GrOnFlushResourceProvider*, GrCCPerFlushResources*,
                        DoCopiesToA8Coverage);

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

private:
    friend class GrOpMemoryPool;

    static std::unique_ptr<GrCCDrawPathsOp> InternalMake(GrRecordingContext*,
                                                         const SkIRect& clipIBounds,
                                                         const SkMatrix&, const GrShape&,
                                                         float strokeDevWidth,
                                                         const SkRect& conservativeDevBounds,
                                                         GrPaint&&);

    GrCCDrawPathsOp(const SkMatrix&, const GrShape&, float strokeDevWidth,
                    const SkIRect& shapeConservativeIBounds, const SkIRect& maskDevIBounds,
                    const SkRect& conservativeDevBounds, GrPaint&&);

    void recordInstance(GrTextureProxy* atlasProxy, int instanceIdx);

    const SkMatrix fViewMatrixIfUsingLocalCoords;

    class SingleDraw {
    public:
        SingleDraw(const SkMatrix&, const GrShape&, float strokeDevWidth,
                   const SkIRect& shapeConservativeIBounds, const SkIRect& maskDevIBounds,
                   const SkPMColor4f&);

        // See the corresponding methods in GrCCDrawPathsOp.
        GrProcessorSet::Analysis finalize(
                const GrCaps&, const GrAppliedClip*, GrFSAAType, GrClampType, GrProcessorSet*);
        void accountForOwnPath(GrCCPathCache*, GrOnFlushResourceProvider*,
                               GrCCPerFlushResourceSpecs*);
        void setupResources(GrCCPathCache*, GrOnFlushResourceProvider*, GrCCPerFlushResources*,
                            DoCopiesToA8Coverage, GrCCDrawPathsOp*);

    private:
        bool shouldCachePathMask(int maxRenderTargetSize) const;

        SkMatrix fMatrix;
        GrShape fShape;
        float fStrokeDevWidth;
        const SkIRect fShapeConservativeIBounds;
        SkIRect fMaskDevIBounds;
        SkPMColor4f fColor;

        GrCCPathCache::OnFlushEntryRef fCacheEntry;
        SkIVector fCachedMaskShift;
        bool fDoCopyToA8Coverage = false;
        bool fDoCachePathMask = false;

        SingleDraw* fNext = nullptr;

        friend class GrCCSTLList<SingleDraw>;  // To access fNext.
    };

    // Declare fOwningPerOpListPaths first, before fDraws. The draws use memory allocated by
    // fOwningPerOpListPaths, so it must not be unreffed until after fDraws is destroyed.
    sk_sp<GrCCPerOpListPaths> fOwningPerOpListPaths;

    GrCCSTLList<SingleDraw> fDraws;
    SkDEBUGCODE(int fNumDraws = 1);

    GrProcessorSet fProcessors;

    struct InstanceRange {
        GrTextureProxy* fAtlasProxy;
        int fEndInstanceIdx;
    };

    SkSTArray<2, InstanceRange, true> fInstanceRanges;
    int fBaseInstance SkDEBUGCODE(= -1);
};

#endif
