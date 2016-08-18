/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNinePatch.h"

#include "GrBatchFlushState.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrResourceProvider.h"
#include "GrVertexBatch.h"
#include "SkBitmap.h"
#include "SkLatticeIter.h"
#include "SkRect.h"

static sk_sp<GrGeometryProcessor> create_gp(bool readsCoverage) {
    using namespace GrDefaultGeoProcFactory;
    Color color(Color::kAttribute_Type);
    Coverage coverage(readsCoverage ? Coverage::kSolid_Type : Coverage::kNone_Type);
    LocalCoords localCoords(LocalCoords::kHasExplicit_Type);
    return GrDefaultGeoProcFactory::Make(color, coverage, localCoords, SkMatrix::I());
}

class GrNonAANinePatchBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    static const int kVertsPerRect = 4;
    static const int kIndicesPerRect = 6;
    static const int kRectsPerInstance = 9; // We could skip empty rects

    GrNonAANinePatchBatch(GrColor color, const SkMatrix& viewMatrix, int imageWidth,
                          int imageHeight, const SkIRect& center, const SkRect &dst)
        : INHERITED(ClassID()) {
        Patch& patch = fPatches.push_back();
        patch.fViewMatrix = viewMatrix;
        patch.fColor = color;
        patch.fCenter = center;
        patch.fDst = dst;

        fImageWidth = imageWidth;
        fImageHeight = imageHeight;

        // setup bounds
        this->setTransformedBounds(patch.fDst, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "NonAANinePatchBatch"; }

    SkString dumpInfo() const override {
        SkString str;

        for (int i = 0; i < fPatches.count(); ++i) {
            str.appendf("%d: Color: 0x%08x Center [L: %d, T: %d, R: %d, B: %d], "
                        "Dst [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                        i,
                        fPatches[i].fColor,
                        fPatches[i].fCenter.fLeft, fPatches[i].fCenter.fTop,
                        fPatches[i].fCenter.fRight, fPatches[i].fCenter.fBottom,
                        fPatches[i].fDst.fLeft, fPatches[i].fDst.fTop,
                        fPatches[i].fDst.fRight, fPatches[i].fDst.fBottom);
        }

        str.append(INHERITED::dumpInfo());
        return str;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        color->setUnknownFourComponents();
        coverage->setKnownSingleComponent(0xff);
    }

private:
    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp(create_gp(fOverrides.readsCoverage()));
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();
        int patchCnt = fPatches.count();

        SkAutoTUnref<const GrBuffer> indexBuffer(
                target->resourceProvider()->refQuadIndexBuffer());
        InstancedHelper helper;
        void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer, kVertsPerRect,
                                     kIndicesPerRect, patchCnt * kRectsPerInstance);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < patchCnt; i++) {
            intptr_t verts = reinterpret_cast<intptr_t>(vertices) +
                             i * kRectsPerInstance * kVertsPerRect * vertexStride;

            const Patch& patch = fPatches[i];
            SkLatticeIter iter(fImageWidth, fImageHeight, patch.fCenter, patch.fDst);

            SkRect srcR, dstR;
            while (iter.next(&srcR, &dstR)) {
                SkPoint* positions = reinterpret_cast<SkPoint*>(verts);

                positions->setRectFan(dstR.fLeft, dstR.fTop,
                                      dstR.fRight, dstR.fBottom, vertexStride);

                SkASSERT(!patch.fViewMatrix.hasPerspective());
                patch.fViewMatrix.mapPointsWithStride(positions, vertexStride, kVertsPerRect);

                // Setup local coords
                static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);
                SkPoint* coords = reinterpret_cast<SkPoint*>(verts + kLocalOffset);
                coords->setRectFan(srcR.fLeft, srcR.fTop, srcR.fRight, srcR.fBottom, vertexStride);

                static const int kColorOffset = sizeof(SkPoint);
                GrColor* vertColor = reinterpret_cast<GrColor*>(verts + kColorOffset);
                for (int j = 0; j < 4; ++j) {
                    *vertColor = patch.fColor;
                    vertColor = (GrColor*) ((intptr_t) vertColor + vertexStride);
                }
                verts += kVertsPerRect * vertexStride;
            }
        }
        helper.recordDraw(target, gp.get());
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fPatches[0].fColor);
        fOverrides = overrides;
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        GrNonAANinePatchBatch* that = t->cast<GrNonAANinePatchBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        SkASSERT(this->fImageWidth == that->fImageWidth &&
                 this->fImageHeight == that->fImageHeight);

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (fOverrides.canTweakAlphaForCoverage() && !that->fOverrides.canTweakAlphaForCoverage()) {
            fOverrides = that->fOverrides;
        }

        fPatches.push_back_n(that->fPatches.count(), that->fPatches.begin());
        this->joinBounds(*that);
        return true;
    }

    struct Patch {
        SkMatrix fViewMatrix;
        SkIRect fCenter;
        SkRect fDst;
        GrColor fColor;
    };

    GrXPOverridesForBatch fOverrides;
    int fImageWidth;
    int fImageHeight;
    SkSTArray<1, Patch, true> fPatches;

    typedef GrVertexBatch INHERITED;
};

namespace GrNinePatch {
GrDrawBatch* CreateNonAA(GrColor color, const SkMatrix& viewMatrix, int imageWidth, int imageHeight,
                         const SkIRect& center, const SkRect& dst) {
    return new GrNonAANinePatchBatch(color, viewMatrix, imageWidth, imageHeight, center, dst);
}
};
