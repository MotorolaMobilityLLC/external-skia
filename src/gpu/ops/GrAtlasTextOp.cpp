/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasTextOp.h"

#include "GrContext.h"
#include "GrOpFlushState.h"
#include "GrResourceProvider.h"

#include "SkGlyphCache.h"
#include "SkMathPriv.h"

#include "effects/GrBitmapTextGeoProc.h"
#include "effects/GrDistanceFieldGeoProc.h"
#include "text/GrAtlasGlyphCache.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

static const int kDistanceAdjustLumShift = 5;

SkString GrAtlasTextOp::dumpInfo() const {
    SkString str;

    for (int i = 0; i < fGeoCount; ++i) {
        str.appendf("%d: Color: 0x%08x Trans: %.2f,%.2f Runs: %d\n",
                    i,
                    fGeoData[i].fColor,
                    fGeoData[i].fX,
                    fGeoData[i].fY,
                    fGeoData[i].fBlob->runCount());
    }

    str += fProcessors.dumpProcessors();
    str += INHERITED::dumpInfo();
    return str;
}

GrDrawOp::FixedFunctionFlags GrAtlasTextOp::fixedFunctionFlags() const {
    return FixedFunctionFlags::kNone;
}

GrDrawOp::RequiresDstTexture GrAtlasTextOp::finalize(const GrCaps& caps,
                                                     const GrAppliedClip* clip,
                                                     GrPixelConfigIsClamped dstIsClamped) {
    GrProcessorAnalysisCoverage coverage;
    GrProcessorAnalysisColor color;
    if (kColorBitmapMask_MaskType == fMaskType) {
        color.setToUnknown();
    } else {
        color.setToConstant(fColor);
    }
    switch (fMaskType) {
        case kGrayscaleCoverageMask_MaskType:
        case kAliasedDistanceField_MaskType:
        case kGrayscaleDistanceField_MaskType:
            coverage = GrProcessorAnalysisCoverage::kSingleChannel;
            break;
        case kLCDCoverageMask_MaskType:
        case kLCDDistanceField_MaskType:
        case kLCDBGRDistanceField_MaskType:
            coverage = GrProcessorAnalysisCoverage::kLCD;
            break;
        case kColorBitmapMask_MaskType:
            coverage = GrProcessorAnalysisCoverage::kNone;
            break;
    }
    auto analysis = fProcessors.finalize(color, coverage, clip, false, caps, dstIsClamped, &fColor);
    fUsesLocalCoords = analysis.usesLocalCoords();
    fCanCombineOnTouchOrOverlap =
            !analysis.requiresDstTexture() &&
            !(fProcessors.xferProcessor() && fProcessors.xferProcessor()->xferBarrierType(caps));
    return analysis.requiresDstTexture() ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
}

static void clip_quads(const SkIRect& clipRect,
                       unsigned char* currVertex, unsigned char* blobVertices,
                       size_t vertexStride, int glyphCount) {
    for (int i = 0; i < glyphCount; ++i) {
        SkPoint* blobPositionLT = reinterpret_cast<SkPoint*>(blobVertices);
        SkPoint* blobPositionRB = reinterpret_cast<SkPoint*>(blobVertices + 3*vertexStride);

        SkRect positionRect = SkRect::MakeLTRB(blobPositionLT->fX,
                                               blobPositionLT->fY,
                                               blobPositionRB->fX,
                                               blobPositionRB->fY);
        if (clipRect.contains(positionRect)) {
            memcpy(currVertex, blobVertices, 4 * vertexStride);
            currVertex += 4 * vertexStride;
        } else {
            // Pull out some more data that we'll need.
            // In the LCD case the color will be garbage, but we'll overwrite it with the texcoords
            // and it avoids a lot of conditionals.
            SkColor color = *reinterpret_cast<SkColor*>(blobVertices + sizeof(SkPoint));
            size_t coordOffset = vertexStride - sizeof(SkIPoint16);
            SkIPoint16* blobCoordsLT = reinterpret_cast<SkIPoint16*>(blobVertices + coordOffset);
            SkIPoint16* blobCoordsRB = reinterpret_cast<SkIPoint16*>(blobVertices +
                                                                     3*vertexStride +
                                                                     coordOffset);
            SkIRect coordsRect = SkIRect::MakeLTRB(blobCoordsLT->fX / 2,
                                                   blobCoordsLT->fY / 2,
                                                   blobCoordsRB->fX / 2,
                                                   blobCoordsRB->fY / 2);
            int pageIndexX = blobCoordsLT->fX & 0x1;
            int pageIndexY = blobCoordsLT->fY & 0x1;

            SkASSERT(positionRect.width() == coordsRect.width());

            // Clip position and texCoords to the clipRect
            if (positionRect.fLeft < clipRect.fLeft) {
                coordsRect.fLeft += clipRect.fLeft - positionRect.fLeft;
                positionRect.fLeft = clipRect.fLeft;
            }
            if (positionRect.fTop < clipRect.fTop) {
                coordsRect.fTop += clipRect.fTop - positionRect.fTop;
                positionRect.fTop = clipRect.fTop;
            }
            if (positionRect.fRight > clipRect.fRight) {
                coordsRect.fRight += clipRect.fRight - positionRect.fRight;
                positionRect.fRight = clipRect.fRight;
            }
            if (positionRect.fBottom > clipRect.fBottom) {
                coordsRect.fBottom += clipRect.fBottom - positionRect.fBottom;
                positionRect.fBottom = clipRect.fBottom;
            }
            if (positionRect.fLeft > positionRect.fRight) {
                positionRect.fLeft = positionRect.fRight;
            }
            if (positionRect.fTop > positionRect.fBottom) {
                positionRect.fTop = positionRect.fBottom;
            }
            coordsRect.fLeft = 2 * coordsRect.fLeft | pageIndexX;
            coordsRect.fTop = 2 * coordsRect.fTop | pageIndexY;
            coordsRect.fRight = 2 * coordsRect.fRight | pageIndexX;
            coordsRect.fBottom = 2 * coordsRect.fBottom | pageIndexY;

            SkPoint* currPosition = reinterpret_cast<SkPoint*>(currVertex);
            currPosition->fX = positionRect.fLeft;
            currPosition->fY = positionRect.fTop;
            *(reinterpret_cast<SkColor*>(currVertex + sizeof(SkPoint))) = color;
            SkIPoint16* currCoords = reinterpret_cast<SkIPoint16*>(currVertex + coordOffset);
            currCoords->fX = coordsRect.fLeft;
            currCoords->fY = coordsRect.fTop;
            currVertex += vertexStride;

            currPosition = reinterpret_cast<SkPoint*>(currVertex);
            currPosition->fX = positionRect.fLeft;
            currPosition->fY = positionRect.fBottom;
            *(reinterpret_cast<SkColor*>(currVertex + sizeof(SkPoint))) = color;
            currCoords = reinterpret_cast<SkIPoint16*>(currVertex + coordOffset);
            currCoords->fX = coordsRect.fLeft;
            currCoords->fY = coordsRect.fBottom;
            currVertex += vertexStride;

            currPosition = reinterpret_cast<SkPoint*>(currVertex);
            currPosition->fX = positionRect.fRight;
            currPosition->fY = positionRect.fTop;
            *(reinterpret_cast<SkColor*>(currVertex + sizeof(SkPoint))) = color;
            currCoords = reinterpret_cast<SkIPoint16*>(currVertex + coordOffset);
            currCoords->fX = coordsRect.fRight;
            currCoords->fY = coordsRect.fTop;
            currVertex += vertexStride;

            currPosition = reinterpret_cast<SkPoint*>(currVertex);
            currPosition->fX = positionRect.fRight;
            currPosition->fY = positionRect.fBottom;
            *(reinterpret_cast<SkColor*>(currVertex + sizeof(SkPoint))) = color;
            currCoords = reinterpret_cast<SkIPoint16*>(currVertex + coordOffset);
            currCoords->fX = coordsRect.fRight;
            currCoords->fY = coordsRect.fBottom;
            currVertex += vertexStride;
        }

        blobVertices += 4 * vertexStride;
    }
}

void GrAtlasTextOp::onPrepareDraws(Target* target) {
    // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix.
    // TODO actually only invert if we don't have RGBA
    SkMatrix localMatrix;
    if (this->usesLocalCoords() && !this->viewMatrix().invert(&localMatrix)) {
        SkDebugf("Cannot invert viewmatrix\n");
        return;
    }

    GrMaskFormat maskFormat = this->maskFormat();

    uint32_t atlasPageCount = fFontCache->getAtlasPageCount(maskFormat);
    const sk_sp<GrTextureProxy>* proxies = fFontCache->getProxies(maskFormat);
    if (!atlasPageCount || !proxies[0]) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return;
    }

    FlushInfo flushInfo;
    flushInfo.fPipeline =
            target->makePipeline(fSRGBFlags, std::move(fProcessors), target->detachAppliedClip());
    if (this->usesDistanceFields()) {
        flushInfo.fGeometryProcessor = this->setupDfProcessor();
    } else {
        flushInfo.fGeometryProcessor = GrBitmapTextGeoProc::Make(
            this->color(), proxies, GrSamplerState::ClampNearest(), maskFormat,
            localMatrix, this->usesLocalCoords());
    }

    flushInfo.fGlyphsToFlush = 0;
    size_t vertexStride = flushInfo.fGeometryProcessor->getVertexStride();
    SkASSERT(vertexStride == GrAtlasTextBlob::GetVertexStride(maskFormat));

    int glyphCount = this->numGlyphs();
    const GrBuffer* vertexBuffer;

    void* vertices = target->makeVertexSpace(
            vertexStride, glyphCount * kVerticesPerGlyph, &vertexBuffer, &flushInfo.fVertexOffset);
    flushInfo.fVertexBuffer.reset(SkRef(vertexBuffer));
    flushInfo.fIndexBuffer = target->resourceProvider()->refQuadIndexBuffer();
    if (!vertices || !flushInfo.fVertexBuffer) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    unsigned char* currVertex = reinterpret_cast<unsigned char*>(vertices);

    GrBlobRegenHelper helper(this, target, &flushInfo);
    SkAutoGlyphCache glyphCache;
    // each of these is a SubRun
    for (int i = 0; i < fGeoCount; i++) {
        const Geometry& args = fGeoData[i];
        Blob* blob = args.fBlob;
        size_t byteCount;
        void* blobVertices;
        int subRunGlyphCount;
        blob->regenInOp(target, fFontCache, &helper, args.fRun, args.fSubRun, &glyphCache,
                        vertexStride, args.fViewMatrix, args.fX, args.fY, args.fColor,
                        &blobVertices, &byteCount, &subRunGlyphCount);

        // now copy all vertices
        if (args.fClipRect.isEmpty()) {
            memcpy(currVertex, blobVertices, byteCount);
        } else {
            clip_quads(args.fClipRect, currVertex, reinterpret_cast<unsigned char*>(blobVertices),
                       vertexStride, subRunGlyphCount);
        }

        currVertex += byteCount;
    }

    this->flush(target, &flushInfo);
}

void GrAtlasTextOp::flush(GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const {
    GrGeometryProcessor* gp = flushInfo->fGeometryProcessor.get();
    GrMaskFormat maskFormat = this->maskFormat();
    if (gp->numTextureSamplers() != (int)fFontCache->getAtlasPageCount(maskFormat)) {
        // During preparation the number of atlas pages has increased.
        // Update the proxies used in the GP to match.
        if (this->usesDistanceFields()) {
            if (this->isLCD()) {
                reinterpret_cast<GrDistanceFieldLCDTextGeoProc*>(gp)->addNewProxies(
                    fFontCache->getProxies(maskFormat), GrSamplerState::ClampBilerp());
            } else {
                reinterpret_cast<GrDistanceFieldA8TextGeoProc*>(gp)->addNewProxies(
                    fFontCache->getProxies(maskFormat), GrSamplerState::ClampBilerp());
            }
        } else {
            reinterpret_cast<GrBitmapTextGeoProc*>(gp)->addNewProxies(
                fFontCache->getProxies(maskFormat), GrSamplerState::ClampNearest());
        }
    }

    GrMesh mesh(GrPrimitiveType::kTriangles);
    int maxGlyphsPerDraw =
            static_cast<int>(flushInfo->fIndexBuffer->gpuMemorySize() / sizeof(uint16_t) / 6);
    mesh.setIndexedPatterned(flushInfo->fIndexBuffer.get(), kIndicesPerGlyph, kVerticesPerGlyph,
                             flushInfo->fGlyphsToFlush, maxGlyphsPerDraw);
    mesh.setVertexData(flushInfo->fVertexBuffer.get(), flushInfo->fVertexOffset);
    target->draw(flushInfo->fGeometryProcessor.get(), flushInfo->fPipeline, mesh);
    flushInfo->fVertexOffset += kVerticesPerGlyph * flushInfo->fGlyphsToFlush;
    flushInfo->fGlyphsToFlush = 0;
}

bool GrAtlasTextOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    GrAtlasTextOp* that = t->cast<GrAtlasTextOp>();
    if (fProcessors != that->fProcessors) {
        return false;
    }

    if (!fCanCombineOnTouchOrOverlap && GrRectsTouchOrOverlap(this->bounds(), that->bounds())) {
        return false;
    }

    if (fMaskType != that->fMaskType) {
        return false;
    }

    if (!this->usesDistanceFields()) {
        if (kColorBitmapMask_MaskType == fMaskType && this->color() != that->color()) {
            return false;
        }
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }
    } else {
        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        if (fLuminanceColor != that->fLuminanceColor) {
            return false;
        }
    }

    fNumGlyphs += that->numGlyphs();

    // Reallocate space for geo data if necessary and then import that's geo data.
    int newGeoCount = that->fGeoCount + fGeoCount;
    // We assume (and here enforce) that the allocation size is the smallest power of two that
    // is greater than or equal to the number of geometries (and at least
    // kMinGeometryAllocated).
    int newAllocSize = GrNextPow2(newGeoCount);
    int currAllocSize = SkTMax<int>(kMinGeometryAllocated, GrNextPow2(fGeoCount));

    if (newGeoCount > currAllocSize) {
        fGeoData.realloc(newAllocSize);
    }

    // We steal the ref on the blobs from the other AtlasTextOp and set its count to 0 so that
    // it doesn't try to unref them.
    memcpy(&fGeoData[fGeoCount], that->fGeoData.get(), that->fGeoCount * sizeof(Geometry));
#ifdef SK_DEBUG
    for (int i = 0; i < that->fGeoCount; ++i) {
        that->fGeoData.get()[i].fBlob = (Blob*)0x1;
    }
#endif
    that->fGeoCount = 0;
    fGeoCount = newGeoCount;

    this->joinBounds(*that);
    return true;
}

// TODO trying to figure out why lcd is so whack
// (see comments in GrAtlasTextContext::ComputeCanonicalColor)
sk_sp<GrGeometryProcessor> GrAtlasTextOp::setupDfProcessor() const {
    const SkMatrix& viewMatrix = this->viewMatrix();
    const sk_sp<GrTextureProxy>* p = fFontCache->getProxies(this->maskFormat());
    bool isLCD = this->isLCD();
    // set up any flags
    uint32_t flags = viewMatrix.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
    flags |= viewMatrix.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    flags |= fUseGammaCorrectDistanceTable ? kGammaCorrect_DistanceFieldEffectFlag : 0;
    flags |= (kAliasedDistanceField_MaskType == fMaskType) ? kAliased_DistanceFieldEffectFlag : 0;

    // see if we need to create a new effect
    if (isLCD) {
        flags |= kUseLCD_DistanceFieldEffectFlag;
        flags |= (kLCDBGRDistanceField_MaskType == fMaskType) ? kBGR_DistanceFieldEffectFlag : 0;

        float redCorrection = fDistanceAdjustTable->getAdjustment(
                SkColorGetR(fLuminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        float greenCorrection = fDistanceAdjustTable->getAdjustment(
                SkColorGetG(fLuminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        float blueCorrection = fDistanceAdjustTable->getAdjustment(
                SkColorGetB(fLuminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        GrDistanceFieldLCDTextGeoProc::DistanceAdjust widthAdjust =
                GrDistanceFieldLCDTextGeoProc::DistanceAdjust::Make(
                        redCorrection, greenCorrection, blueCorrection);

        return GrDistanceFieldLCDTextGeoProc::Make(this->color(), viewMatrix, p,
                                                   GrSamplerState::ClampBilerp(), widthAdjust,
                                                   flags, this->usesLocalCoords());
    } else {
#ifdef SK_GAMMA_APPLY_TO_A8
        float correction = 0;
        if (kAliasedDistanceField_MaskType != fMaskType) {
            U8CPU lum = SkColorSpaceLuminance::computeLuminance(SK_GAMMA_EXPONENT,
                                                                fLuminanceColor);
            correction = fDistanceAdjustTable->getAdjustment(lum >> kDistanceAdjustLumShift,
                                                             fUseGammaCorrectDistanceTable);
        }
        return GrDistanceFieldA8TextGeoProc::Make(this->color(), viewMatrix, p,
                                                  GrSamplerState::ClampBilerp(), correction, flags,
                                                  this->usesLocalCoords());
#else
        return GrDistanceFieldA8TextGeoProc::Make(this->color(), viewMatrix, p,
                                                  GrSamplerState::ClampBilerp(), flags,
                                                  this->usesLocalCoords());
#endif
    }
}

void GrBlobRegenHelper::flush() { fOp->flush(fTarget, fFlushInfo); }
