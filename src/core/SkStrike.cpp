/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrike.h"

#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkEnumerate.h"
#include <cctype>

static SkFontMetrics use_or_generate_metrics(
        const SkFontMetrics* metrics, SkScalerContext* context) {
    SkFontMetrics answer;
    if (metrics) {
        answer = *metrics;
    } else {
        context->getFontMetrics(&answer);
    }
    return answer;
}

SkStrike::SkStrike(
    const SkDescriptor& desc,
    std::unique_ptr<SkScalerContext> scaler,
    const SkFontMetrics* fontMetrics)
        : fDesc{desc}
        , fScalerContext{std::move(scaler)}
        , fFontMetrics{use_or_generate_metrics(fontMetrics, fScalerContext.get())}
        , fRoundingSpec{fScalerContext->isSubpixel(),
                        fScalerContext->computeAxisAlignmentForHText()} {
    SkASSERT(fScalerContext != nullptr);
}

#ifdef SK_DEBUG
#define VALIDATE()  AutoValidate av(this)
#else
#define VALIDATE()
#endif

// -- glyph creation -------------------------------------------------------------------------------
SkGlyph* SkStrike::makeGlyph(SkPackedGlyphID packedGlyphID) {
    fMemoryUsed += sizeof(SkGlyph);
    SkGlyph* glyph = fAlloc.make<SkGlyph>(packedGlyphID);
    fGlyphMap.set(glyph);
    return glyph;
}

SkGlyph* SkStrike::glyph(SkPackedGlyphID packedGlyphID) {
    VALIDATE();
    SkGlyph* glyph = fGlyphMap.findOrNull(packedGlyphID);
    if (glyph == nullptr) {
        glyph = this->makeGlyph(packedGlyphID);
        fScalerContext->getMetrics(glyph);
    }
    return glyph;
}

SkGlyph* SkStrike::glyphFromPrototype(const SkGlyphPrototype& p, void* image) {
    SkAutoMutexExclusive lock{fMu};
    SkGlyph* glyph = fGlyphMap.findOrNull(p.id);
    if (glyph == nullptr) {
        fMemoryUsed += sizeof(SkGlyph);
        glyph = fAlloc.make<SkGlyph>(p);
        fGlyphMap.set(glyph);
    }
    if (glyph->setImage(&fAlloc, image)) {
        fMemoryUsed += glyph->imageSize();
    }
    return glyph;
}

SkGlyph* SkStrike::glyphOrNull(SkPackedGlyphID id) const {
    SkAutoMutexExclusive lock{fMu};
    return this->internalGlyphOrNull(id);
}

const SkPath* SkStrike::preparePath(SkGlyph* glyph) {
    if (glyph->setPath(&fAlloc, fScalerContext.get())) {
        fMemoryUsed += glyph->path()->approximateBytesUsed();
    }
    return glyph->path();
}

const SkPath* SkStrike::preparePath(SkGlyph* glyph, const SkPath* path) {
    SkAutoMutexExclusive lock{fMu};
    if (glyph->setPath(&fAlloc, path)) {
        fMemoryUsed += glyph->path()->approximateBytesUsed();
    }
    return glyph->path();
}

const SkDescriptor& SkStrike::getDescriptor() const {
    return *fDesc.getDesc();
}

int SkStrike::countCachedGlyphs() const {
    SkAutoMutexExclusive lock(fMu);
    return fGlyphMap.count();
}

SkGlyph* SkStrike::internalGlyphOrNull(SkPackedGlyphID id) const {
    return fGlyphMap.findOrNull(id);
}

SkSpan<const SkGlyph*> SkStrike::internalPrepare(
        SkSpan<const SkGlyphID> glyphIDs, PathDetail pathDetail, const SkGlyph** results) {
    const SkGlyph** cursor = results;
    for (auto glyphID : glyphIDs) {
        SkGlyph* glyphPtr = this->glyph(SkPackedGlyphID{glyphID});
        if (pathDetail == kMetricsAndPath) {
            this->preparePath(glyphPtr);
        }
        *cursor++ = glyphPtr;
    }

    return {results, glyphIDs.size()};
}

const void* SkStrike::prepareImage(SkGlyph* glyph) {
    if (glyph->setImage(&fAlloc, fScalerContext.get())) {
        fMemoryUsed += glyph->imageSize();
    }
    return glyph->image();
}

SkGlyph* SkStrike::mergeGlyphAndImage(SkPackedGlyphID toID, const SkGlyph& from) {
    SkAutoMutexExclusive lock{fMu};
    SkGlyph* glyph = fGlyphMap.findOrNull(toID);
    if (glyph == nullptr) {
        glyph = this->makeGlyph(toID);
    }
    if (glyph->setMetricsAndImage(&fAlloc, from)) {
        fMemoryUsed += glyph->imageSize();
    }
    return glyph;
}

SkSpan<const SkGlyph*> SkStrike::metrics(SkSpan<const SkGlyphID> glyphIDs,
                                         const SkGlyph* results[]) {
    SkAutoMutexExclusive lock{fMu};
    return this->internalPrepare(glyphIDs, kMetricsOnly, results);
}

SkSpan<const SkGlyph*> SkStrike::preparePaths(SkSpan<const SkGlyphID> glyphIDs,
                                              const SkGlyph* results[]) {
    SkAutoMutexExclusive lock{fMu};
    return this->internalPrepare(glyphIDs, kMetricsAndPath, results);
}

SkSpan<const SkGlyph*>
SkStrike::prepareImages(SkSpan<const SkPackedGlyphID> glyphIDs, const SkGlyph* results[]) {
    const SkGlyph** cursor = results;
    SkAutoMutexExclusive lock{fMu};
    for (auto glyphID : glyphIDs) {
        SkGlyph* glyphPtr = this->glyph(glyphID);
        (void)this->prepareImage(glyphPtr);
        *cursor++ = glyphPtr;
    }

    return {results, glyphIDs.size()};
}

template <typename Fn>
void SkStrike::commonFilterLoop(SkDrawableGlyphBuffer* drawables, Fn&& fn) {
    for (auto [i, packedID, pos] : SkMakeEnumerate(drawables->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            SkGlyph* glyph = this->glyph(packedID);
            if (!glyph->isEmpty()) {
                fn(i, glyph, pos);
            }
        }
    }
}

void SkStrike::prepareForDrawingMasksCPU(SkDrawableGlyphBuffer* drawables) {
    SkAutoMutexExclusive lock{fMu};
    this->commonFilterLoop(drawables,
        [&](size_t i, SkGlyph* glyph, SkPoint pos) SK_REQUIRES(fMu) {
            // If the glyph is too large, then no image is created.
            if (this->prepareImage(glyph) != nullptr) {
                drawables->push_back(glyph, i);
            }
        });
}

void SkStrike::prepareForDrawingPathsCPU(SkDrawableGlyphBuffer* drawables) {
    SkAutoMutexExclusive lock{fMu};
    this->commonFilterLoop(drawables,
        [&](size_t i, SkGlyph* glyph, SkPoint pos) SK_REQUIRES(fMu) {
            const SkPath* path = this->preparePath(glyph);
            // The glyph my not have a path.
            if (path != nullptr) {
                drawables->push_back(path, i);
            }
        });
}

// Note: this does not actually fill out the image. That happens at atlas building time.
void SkStrike::prepareForMaskDrawing(
        SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) {
    SkAutoMutexExclusive lock{fMu};
    this->commonFilterLoop(drawables,
        [&](size_t i, SkGlyph* glyph, SkPoint pos) {
            if (CanDrawAsMask(*glyph)) {
                drawables->push_back(glyph, i);
            } else {
                rejects->reject(i);
            }
        });
}

void SkStrike::prepareForSDFTDrawing(
        SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) {
    SkAutoMutexExclusive lock{fMu};
    this->commonFilterLoop(drawables,
        [&](size_t i, SkGlyph* glyph, SkPoint pos) {
            if (CanDrawAsSDFT(*glyph)) {
                drawables->push_back(glyph, i);
            } else {
                rejects->reject(i);
            }
        });
}

void SkStrike::prepareForPathDrawing(
        SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) {
    SkAutoMutexExclusive lock{fMu};
    this->commonFilterLoop(drawables,
        [&](size_t i, SkGlyph* glyph, SkPoint pos) SK_REQUIRES(fMu) {
            if (!glyph->isColor()) {
                const SkPath* path = this->preparePath(glyph);
                if (path != nullptr) {
                    // Save off the path to draw later.
                    drawables->push_back(path, i);
                } else {
                    // Glyph does not have a path. It is probably bitmap only.
                    rejects->reject(i, glyph->maxDimension());
                }
            } else {
                // Glyph is color.
                rejects->reject(i, glyph->maxDimension());
            }
        });
}

void SkStrike::findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
        SkGlyph* glyph, SkScalar* array, int* count) {
    glyph->ensureIntercepts(bounds, scale, xPos, array, count, &fAlloc);
}

void SkStrike::dump() const {
    SkAutoMutexExclusive lock{fMu};
    const SkTypeface* face = fScalerContext->getTypeface();
    const SkScalerContextRec& rec = fScalerContext->getRec();
    SkMatrix matrix;
    rec.getSingleMatrix(&matrix);
    matrix.preScale(SkScalarInvert(rec.fTextSize), SkScalarInvert(rec.fTextSize));
    SkString name;
    face->getFamilyName(&name);

    SkString msg;
    SkFontStyle style = face->fontStyle();
    msg.printf("cache typeface:%x %25s:(%d,%d,%d)\n %s glyphs:%3d",
               face->uniqueID(), name.c_str(), style.weight(), style.width(), style.slant(),
               rec.dump().c_str(), fGlyphMap.count());
    SkDebugf("%s\n", msg.c_str());
}

void SkStrike::onAboutToExitScope() { }

#ifdef SK_DEBUG
void SkStrike::forceValidate() const {
    SkAutoMutexExclusive lock{fMu};
    size_t memoryUsed = sizeof(*this);
    fGlyphMap.foreach ([&memoryUsed](const SkGlyph* glyphPtr) {
        memoryUsed += sizeof(SkGlyph);
        if (glyphPtr->setImageHasBeenCalled()) {
            memoryUsed += glyphPtr->imageSize();
        }
        if (glyphPtr->setPathHasBeenCalled() && glyphPtr->path() != nullptr) {
            memoryUsed += glyphPtr->path()->approximateBytesUsed();
        }
    });
    SkASSERT(fMemoryUsed == memoryUsed);
}

void SkStrike::validate() const {
#ifdef SK_DEBUG_GLYPH_CACHE
    forceValidate();
#endif
}

#endif  // SK_DEBUG


