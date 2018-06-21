/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRun.h"

#include <algorithm>
#include <new>
#include <tuple>

#include "SkDevice.h"
#include "SkDraw.h"
#include "SkGlyphCache.h"
#include "SkMSAN.h"
#include "SkMakeUnique.h"
#include "SkPaint.h"
#include "SkPaintPriv.h"
#include "SkStrikeCache.h"
#include "SkTextBlob.h"
#include "SkTextBlobRunIterator.h"
#include "SkTo.h"
#include "SkUtils.h"

static SkTypeface::Encoding convert_encoding(SkPaint::TextEncoding encoding) {
    switch (encoding) {
        case  SkPaint::kUTF8_TextEncoding: return SkTypeface::kUTF8_Encoding;
        case SkPaint::kUTF16_TextEncoding: return SkTypeface::kUTF16_Encoding;
        case SkPaint::kUTF32_TextEncoding: return SkTypeface::kUTF32_Encoding;
        default: return SkTypeface::kUTF32_Encoding;
    }
}

// -- SkGlyphSet ----------------------------------------------------------------------------------
uint16_t SkGlyphSet::add(SkGlyphID glyphID) {
    static constexpr SkGlyphID  kUndefGlyph{0};

    if (glyphID >= fUniverseSize) {
        glyphID = kUndefGlyph;
    }

    if (glyphID >= fIndices.size()) {
        fIndices.resize(glyphID + 1);
    }

    auto index = fIndices[glyphID];
    if (index < fUniqueGlyphIDs->size() && (*fUniqueGlyphIDs)[index] == glyphID) {
        return index;
    }

    uint16_t newIndex = SkTo<uint16_t>(fUniqueGlyphIDs->size());
    fUniqueGlyphIDs->push_back(glyphID);
    fIndices[glyphID] = newIndex;
    return newIndex;
}

void SkGlyphSet::reuse(uint32_t glyphUniverseSize, std::vector<SkGlyphID>* uniqueGlyphIDs) {
    SkASSERT(glyphUniverseSize <= (1 << 16));
    fUniverseSize = glyphUniverseSize;
    fUniqueGlyphIDs = uniqueGlyphIDs;

    // If we're hanging onto these arrays for a long time, we don't want their size to drift
    // endlessly upwards. It's unusual to see a typeface with more than 4096 possible glyphs.
    if (glyphUniverseSize < 4096 && fIndices.size() > 4096) {
        fIndices.resize(4096);
        fIndices.shrink_to_fit();
    }

    // No need to clear fIndices here... SkGlyphSet's set insertion algorithm is designed to work
    // correctly even when the fIndexes buffer is uninitialized!
}

// -- SkGlyphRun -----------------------------------------------------------------------------------
SkGlyphRun::SkGlyphRun(SkSpan<uint16_t>   denseIndex,
                       SkSpan<SkPoint>    positions,
                       SkSpan<SkGlyphID>  scratchGlyphs,
                       SkSpan<SkGlyphID>  uniqueGlyphIDs,
                       SkSpan<const char> text,
                       SkSpan<uint32_t>   clusters)
        : fDenseIndex{denseIndex}, fPositions{positions}
        , fTemporaryShuntGlyphIDs{scratchGlyphs}
        , fUniqueGlyphIDs{uniqueGlyphIDs}
        , fText{text}
        , fClusters{clusters} {
    SkASSERT(denseIndex.size() == positions.size());
    SkASSERT(denseIndex.size() == scratchGlyphs.size());
}


void SkGlyphRun::temporaryShuntToDrawPosText(const SkPaint& paint, SkBaseDevice* device) {

    auto pos = (const SkScalar*) fPositions.data();

    device->drawPosText(
            fTemporaryShuntGlyphIDs.data(), fDenseIndex.size() * sizeof(SkGlyphID),
            pos, 2, SkPoint::Make(0, 0), paint);
}

void SkGlyphRun::temporaryShuntToCallback(TemporaryShuntCallback callback) {
    auto bytes = (const char *)fTemporaryShuntGlyphIDs.data();
    auto pos = (const SkScalar*)fPositions.data();
    callback(this->runSize(), bytes, pos);
}

// -- SkGlyphRunList -------------------------------------------------------------------------------
SkGlyphRunList::SkGlyphRunList(SkSpan<SkGlyphRun> glyphRuns, uint64_t uniqueID)
        : fUniqueID{uniqueID}
        , fGlyphRuns{glyphRuns} { }

// -- SkGlyphRunBuilder ----------------------------------------------------------------------------
void SkGlyphRunBuilder::prepareDrawText(
        const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin) {
    this->initialize();
    SkSpan<const char> originalText((const char*)bytes, byteLength);
    if (paint.getTextEncoding() != SkPaint::kUTF8_TextEncoding) {
        originalText = SkSpan<const char>();
    }
    this->drawText(paint, bytes, byteLength, origin, originalText, SkSpan<uint32_t>());
}

void SkGlyphRunBuilder::prepareDrawPosTextH(const SkPaint& paint, const void* bytes,
                                            size_t byteLength, const SkScalar* xpos,
                                            SkScalar constY) {
    this->initialize();
    this->drawPosTextH(
            paint, bytes, byteLength, xpos, constY, SkSpan<const char>(), SkSpan<uint32_t>());
}

void SkGlyphRunBuilder::prepareDrawPosText(const SkPaint& paint, const void* bytes,
                                           size_t byteLength, const SkPoint* pos) {
    this->initialize();
    this->drawPosText(paint, bytes, byteLength, pos, SkSpan<const char>(), SkSpan<uint32_t>());
}

void SkGlyphRunBuilder::prepareTextBlob(
        const SkPaint& paint, const SkTextBlob& blob, SkPoint origin) {
    this->initialize();
    fUniqueID = blob.uniqueID();

    SkPaint runPaint = paint;

    SkTextBlobRunIterator it(&blob);
    for (;!it.done(); it.next()) {
        size_t glyphLen = it.glyphCount() * sizeof(uint16_t);
        const SkPoint& offset = it.offset();

        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);

        // These better be glyphs
        SkASSERT(runPaint.getTextEncoding() == SkPaint::kGlyphID_TextEncoding);

        auto text = SkSpan<const char>(it.text(), it.textSize());
        auto clusters = SkSpan<uint32_t>(it.clusters(), it.glyphCount());

        switch (it.positioning()) {
            case SkTextBlob::kDefault_Positioning: {
                auto dtOrigin = origin + offset;
                this->drawText(runPaint, it.glyphs(), glyphLen, dtOrigin, text, clusters);
            }
            break;
            case SkTextBlob::kHorizontal_Positioning: {
                auto constY = origin.y() + offset.y();
                this->drawPosTextH(
                        runPaint, it.glyphs(), glyphLen, it.pos(), constY, text, clusters);
            }
            break;
            case SkTextBlob::kFull_Positioning:
                this->drawPosText(
                        runPaint, it.glyphs(), glyphLen, (const SkPoint*)it.pos(), text, clusters);
            break;
            default:
                SK_ABORT("unhandled positioning mode");
        }
    }
}

SkGlyphRun* SkGlyphRunBuilder::useGlyphRun() {
    auto glyphRunList = this->useGlyphRunList();
    SkASSERT(glyphRunList->size() == 1);
    return &(*glyphRunList)[0];
}

SkGlyphRunList* SkGlyphRunBuilder::useGlyphRunList() {
    new ((void*)&fScratchGlyphRunList) SkGlyphRunList{SkSpan<SkGlyphRun>(fGlyphRuns), fUniqueID};
    return &fScratchGlyphRunList;
}

size_t SkGlyphRunBuilder::runSize() const { return fDenseIndex.size(); }

size_t SkGlyphRunBuilder::uniqueSize() const { return fUniqueGlyphs.size(); }

void SkGlyphRunBuilder::initialize() {
    fUniqueID = 0;
    fDenseIndex.clear();
    fPositions.clear();
    fUniqueGlyphs.clear();
    fGlyphRuns.clear();
    fLastDenseIndex = 0;
    fLastUniqueIndex = 0;
}

SkGlyphID* SkGlyphRunBuilder::addDenseAndUnique(
        const SkPaint& paint, const void* bytes, size_t byteLength) {

    size_t runSize = 0;
    SkGlyphID* glyphIDs = nullptr;
    auto encoding = paint.getTextEncoding();
    auto typeface = SkPaintPriv::GetTypefaceOrDefault(paint);
    if (encoding != SkPaint::kGlyphID_TextEncoding) {
        auto tfEncoding = convert_encoding(encoding);
        int utfSize = SkUTFN_CountUnichars(tfEncoding, bytes, byteLength);
        if (utfSize > 0) {
            runSize = SkTo<size_t>(utfSize);
            fScratchGlyphIDs.resize(runSize);
            typeface->charsToGlyphs(bytes, tfEncoding, fScratchGlyphIDs.data(), runSize);
            glyphIDs = fScratchGlyphIDs.data();
        }
    } else {
        runSize = byteLength / 2;
        glyphIDs = (SkGlyphID*)bytes;
    }

    SkASSERT(glyphIDs != nullptr);

    if (runSize > 0) {
        fGlyphSet.reuse(typeface->countGlyphs(), &fUniqueGlyphs);
        for (size_t i = 0; i < runSize; i++) {
            fDenseIndex.push_back(fGlyphSet.add(glyphIDs[i]));
        }
    }

    return glyphIDs;
}

void SkGlyphRunBuilder::addGlyphRunToList(
        SkGlyphID* temporaryShuntGlyphIDs, SkSpan<const char> text, SkSpan<uint32_t> clusters) {

    // Ignore empty runs.
    if (fDenseIndex.size() != fLastDenseIndex) {
        auto runSize = fDenseIndex.size() - fLastDenseIndex;
        auto uniqueSize = fUniqueGlyphs.size() - fLastUniqueIndex;

        fGlyphRuns.emplace_back(
                SkSpan<uint16_t>(&fDenseIndex[fLastDenseIndex], runSize),
                SkSpan<SkPoint>(&fPositions[fLastDenseIndex], runSize),
                SkSpan<SkGlyphID>(temporaryShuntGlyphIDs, runSize),
                SkSpan<SkGlyphID>(&fUniqueGlyphs[fLastDenseIndex], uniqueSize),
                text,
                clusters);

        fLastDenseIndex = fDenseIndex.size();
        fLastUniqueIndex = fUniqueGlyphs.size();
    }
}

void SkGlyphRunBuilder::drawText(
        const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin,
        SkSpan<const char> text, SkSpan<uint32_t> clusters) {

    SkGlyphID* temporaryShuntGlyphIDs = this->addDenseAndUnique(paint, bytes, byteLength);

    fScratchAdvances.resize(this->uniqueSize());
    {
        auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);
        cache->getAdvances(SkSpan<SkGlyphID>{fUniqueGlyphs}, fScratchAdvances.data());
    }

    SkPoint endOfLastGlyph = origin;

    for (size_t i = 0; i < this->runSize(); i++) {
        fPositions.push_back(endOfLastGlyph);
        endOfLastGlyph += fScratchAdvances[fDenseIndex[i]];
    }

    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector len = endOfLastGlyph - origin;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            len.scale(SK_ScalarHalf);
        }
        for (size_t i = fLastDenseIndex; i < this->runSize(); i++) {
            fPositions[i] -= len;
        }
    }

    this->addGlyphRunToList(temporaryShuntGlyphIDs, text, clusters);
}

void SkGlyphRunBuilder::drawPosTextH(const SkPaint& paint, const void* bytes,
                                     size_t byteLength, const SkScalar* xpos,
                                     SkScalar constY,
                                     SkSpan<const char> text, SkSpan<uint32_t> clusters) {

    SkGlyphID* temporaryShuntGlyphIDs = this->addDenseAndUnique(paint, bytes, byteLength);

    for (size_t i = 0; i < runSize(); i++) {
        fPositions.push_back(SkPoint::Make(xpos[i], constY));
    }

    this->addGlyphRunToList(temporaryShuntGlyphIDs, text, clusters);
}

void SkGlyphRunBuilder::drawPosText(const SkPaint& paint, const void* bytes,
                                    size_t byteLength, const SkPoint* pos,
                                    SkSpan<const char> text, SkSpan<uint32_t> clusters) {
    SkGlyphID* temporaryShuntGlyphIDs = this->addDenseAndUnique(paint, bytes, byteLength);

    for (size_t i = 0; i < runSize(); i++) {
        fPositions.push_back(pos[i]);
    }

    this->addGlyphRunToList(temporaryShuntGlyphIDs, text, clusters);
}


