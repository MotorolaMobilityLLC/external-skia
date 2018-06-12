/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCDrawPathsOp.h"

#include "GrGpuCommandBuffer.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "ccpr/GrCCPerFlushResources.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"

GrCCDrawPathsOp* GrCCDrawPathsOp::Make(GrContext* context,
                                       GrPaint&& paint,
                                       const SkIRect& clipIBounds,
                                       const SkMatrix& m,
                                       const SkPath& path,
                                       const SkRect& devBounds) {
    return new GrCCDrawPathsOp(std::move(paint), clipIBounds, m, path, devBounds);
}

static bool has_coord_transforms(const GrPaint& paint) {
    GrFragmentProcessor::Iter iter(paint);
    while (const GrFragmentProcessor* fp = iter.next()) {
        if (!fp->coordTransforms().empty()) {
            return true;
        }
    }
    return false;
}

GrCCDrawPathsOp::GrCCDrawPathsOp(GrPaint&& paint, const SkIRect& clipIBounds, const SkMatrix& m,
                                 const SkPath& path, const SkRect& devBounds)
        : GrDrawOp(ClassID())
        , fSRGBFlags(GrPipeline::SRGBFlagsFromPaint(paint))
        , fViewMatrixIfUsingLocalCoords(has_coord_transforms(paint) ? m : SkMatrix::I())
        , fDraws({clipIBounds, m, path, paint.getColor(), nullptr})
        , fProcessors(std::move(paint)) {
    SkDEBUGCODE(fBaseInstance = -1);
    // FIXME: intersect with clip bounds to (hopefully) improve batching.
    // (This is nontrivial due to assumptions in generating the octagon cover geometry.)
    this->setBounds(devBounds, GrOp::HasAABloat::kYes, GrOp::IsZeroArea::kNo);
}

GrCCDrawPathsOp::~GrCCDrawPathsOp() {
    if (fOwningPerOpListPaths) {
        // Remove CCPR's dangling pointer to this Op before deleting it.
        fOwningPerOpListPaths->fDrawOps.remove(this);
    }
}

GrDrawOp::RequiresDstTexture GrCCDrawPathsOp::finalize(const GrCaps& caps,
                                                       const GrAppliedClip* clip,
                                                       GrPixelConfigIsClamped dstIsClamped) {
    // There should only be one single path draw in this Op right now.
    SkASSERT(1 == fNumDraws);
    GrProcessorSet::Analysis analysis =
            fProcessors.finalize(fDraws.head().fColor, GrProcessorAnalysisCoverage::kSingleChannel,
                                 clip, false, caps, dstIsClamped, &fDraws.head().fColor);
    return analysis.requiresDstTexture() ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
}

bool GrCCDrawPathsOp::onCombineIfPossible(GrOp* op, const GrCaps& caps) {
    GrCCDrawPathsOp* that = op->cast<GrCCDrawPathsOp>();
    SkASSERT(fOwningPerOpListPaths);
    SkASSERT(fNumDraws);
    SkASSERT(!that->fOwningPerOpListPaths || that->fOwningPerOpListPaths == fOwningPerOpListPaths);
    SkASSERT(that->fNumDraws);

    if (fSRGBFlags != that->fSRGBFlags || fProcessors != that->fProcessors ||
        fViewMatrixIfUsingLocalCoords != that->fViewMatrixIfUsingLocalCoords) {
        return false;
    }

    fDraws.append(std::move(that->fDraws), &fOwningPerOpListPaths->fAllocator);
    this->joinBounds(*that);

    SkDEBUGCODE(fNumDraws += that->fNumDraws);
    SkDEBUGCODE(that->fNumDraws = 0);
    return true;
}

void GrCCDrawPathsOp::wasRecorded(GrCCPerOpListPaths* owningPerOpListPaths) {
    SkASSERT(1 == fNumDraws);
    SkASSERT(!fOwningPerOpListPaths);
    owningPerOpListPaths->fDrawOps.addToTail(this);
    fOwningPerOpListPaths = owningPerOpListPaths;
}

int GrCCDrawPathsOp::countPaths(GrCCPathParser::PathStats* stats) const {
    int numPaths = 0;
    for (const GrCCDrawPathsOp::SingleDraw& draw : fDraws) {
        stats->statPath(draw.fPath);
        ++numPaths;
    }
    return numPaths;
}

void GrCCDrawPathsOp::setupResources(GrCCPerFlushResources* resources,
                                     GrOnFlushResourceProvider* onFlushRP) {
    const GrCCAtlas* currentAtlas = nullptr;
    SkASSERT(fNumDraws > 0);
    SkASSERT(-1 == fBaseInstance);
    fBaseInstance = resources->nextPathInstanceIdx();

    for (const SingleDraw& draw : fDraws) {
        // renderPathInAtlas gives us two tight bounding boxes: one in device space, as well as a
        // second one rotated an additional 45 degrees. The path vertex shader uses these two
        // bounding boxes to generate an octagon that circumscribes the path.
        SkRect devBounds, devBounds45;
        int16_t atlasOffsetX, atlasOffsetY;
        GrCCAtlas* atlas = resources->renderPathInAtlas(*onFlushRP->caps(), draw.fClipIBounds,
                                                        draw.fMatrix, draw.fPath, &devBounds,
                                                        &devBounds45, &atlasOffsetX, &atlasOffsetY);
        if (!atlas) {
            SkDEBUGCODE(++fNumSkippedInstances);
            continue;
        }
        if (currentAtlas != atlas) {
            if (currentAtlas) {
                this->addAtlasBatch(currentAtlas, resources->nextPathInstanceIdx());
            }
            currentAtlas = atlas;
        }

        resources->appendDrawPathInstance().set(draw.fPath.getFillType(), devBounds, devBounds45,
                                                atlasOffsetX, atlasOffsetY, draw.fColor);
    }

    SkASSERT(resources->nextPathInstanceIdx() == fBaseInstance + fNumDraws - fNumSkippedInstances);
    if (currentAtlas) {
        this->addAtlasBatch(currentAtlas, resources->nextPathInstanceIdx());
    }
}

void GrCCDrawPathsOp::onExecute(GrOpFlushState* flushState) {
    SkASSERT(fOwningPerOpListPaths);

    const GrCCPerFlushResources* resources = fOwningPerOpListPaths->fFlushResources.get();
    if (!resources) {
        return;  // Setup failed.
    }

    SkASSERT(fBaseInstance >= 0);  // Make sure setupResources has been called.

    GrPipeline::InitArgs initArgs;
    initArgs.fFlags = fSRGBFlags;
    initArgs.fProxy = flushState->drawOpArgs().fProxy;
    initArgs.fCaps = &flushState->caps();
    initArgs.fResourceProvider = flushState->resourceProvider();
    initArgs.fDstProxy = flushState->drawOpArgs().fDstProxy;
    GrPipeline pipeline(initArgs, std::move(fProcessors), flushState->detachAppliedClip());

    int baseInstance = fBaseInstance;

    for (int i = 0; i < fAtlasBatches.count(); baseInstance = fAtlasBatches[i++].fEndInstanceIdx) {
        const AtlasBatch& batch = fAtlasBatches[i];
        SkASSERT(batch.fEndInstanceIdx > baseInstance);

        if (!batch.fAtlas->textureProxy()) {
            continue;  // Atlas failed to allocate.
        }

        GrCCPathProcessor pathProc(flushState->resourceProvider(),
                                   sk_ref_sp(batch.fAtlas->textureProxy()),
                                   fViewMatrixIfUsingLocalCoords);
        pathProc.drawPaths(flushState, pipeline, resources->indexBuffer(),
                           resources->vertexBuffer(), resources->instanceBuffer(),
                           baseInstance, batch.fEndInstanceIdx, this->bounds());
    }

    SkASSERT(baseInstance == fBaseInstance + fNumDraws - fNumSkippedInstances);
}
