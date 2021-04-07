/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphRun.h"

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTextBlob.h"
#include "include/private/SkTo.h"
#include "src/core/SkDevice.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkScalerCache.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkUtils.h"

// -- SkGlyphRun -----------------------------------------------------------------------------------
SkGlyphRun::SkGlyphRun(const SkFont& font,
                       SkSpan<const SkPoint> positions,
                       SkSpan<const SkGlyphID> glyphIDs,
                       SkSpan<const char> text,
                       SkSpan<const uint32_t> clusters)
        : fSource{SkMakeZip(glyphIDs, positions)}
        , fText{text}
        , fClusters{clusters}
        , fFont{font} {}

SkGlyphRun::SkGlyphRun(const SkGlyphRun& that, const SkFont& font)
    : fSource{that.fSource}
    , fText{that.fText}
    , fClusters{that.fClusters}
    , fFont{font} {}

// -- SkGlyphRunList -------------------------------------------------------------------------------
SkGlyphRunList::SkGlyphRunList() = default;
SkGlyphRunList::SkGlyphRunList(
        const SkTextBlob* blob,
        SkPoint origin,
        SkSpan<const SkGlyphRun> glyphRunList)
        : fGlyphRuns{glyphRunList}
        , fOriginalTextBlob{blob}
        , fOrigin{origin} { }

SkGlyphRunList::SkGlyphRunList(const SkGlyphRun& glyphRun)
        : fGlyphRuns{SkSpan<const SkGlyphRun>{&glyphRun, 1}}
        , fOriginalTextBlob{nullptr}
        , fOrigin{SkPoint::Make(0, 0)} {}

uint64_t SkGlyphRunList::uniqueID() const {
    return fOriginalTextBlob != nullptr ? fOriginalTextBlob->uniqueID()
                                        : SK_InvalidUniqueID;
}

bool SkGlyphRunList::anyRunsLCD() const {
    for (const auto& r : fGlyphRuns) {
        if (r.font().getEdging() == SkFont::Edging::kSubpixelAntiAlias) {
            return true;
        }
    }
    return false;
}

bool SkGlyphRunList::anyRunsSubpixelPositioned() const {
    for (const auto& r : fGlyphRuns) {
        if (r.font().isSubpixel()) {
            return true;
        }
    }
    return false;
}

void SkGlyphRunList::temporaryShuntBlobNotifyAddedToCache(uint32_t cacheID) const {
    SkASSERT(fOriginalTextBlob != nullptr);
    fOriginalTextBlob->notifyAddedToCache(cacheID);
}

// -- SkGlyphRunBuilder ----------------------------------------------------------------------------
void SkGlyphRunBuilder::drawTextUTF8(const SkFont& font, const void* bytes,
                                     size_t byteLength, SkPoint origin) {
    auto glyphIDs = textToGlyphIDs(font, bytes, byteLength, SkTextEncoding::kUTF8);
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawText(font, glyphIDs, origin, fPositions);
    }

    this->makeGlyphRunList(nullptr, SkPoint::Make(0, 0));
}

void SkGlyphRunBuilder::drawTextBlob(const SkPaint& paint, const SkTextBlob& blob, SkPoint origin,
                                     SkBaseDevice* device) {
    // Figure out all the storage needed to pre-size everything below.
    size_t totalGlyphs = 0;
    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        totalGlyphs += it.glyphCount();
    }

    // Pre-size all the buffers so they don't move during processing.
    this->initialize(totalGlyphs);

    SkPoint* positions = fPositions;

    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        if (!SkFontPriv::IsFinite(it.font())) {
            // If the font is not finite don't add the run.
            continue;
        }
        if (it.positioning() != SkTextBlobRunIterator::kRSXform_Positioning) {
            simplifyTextBlobIgnoringRSXForm(it, positions);
        } else {
            // Handle kRSXform_Positioning
            if (!this->empty()) {
                // Draw the things we have accumulated so far before drawing the RSX form.
                this->makeGlyphRunList(&blob, origin);
                device->drawGlyphRunList(this->useGlyphRunList(), paint);
                // re-init in case we keep looping and need the builder again
                this->initialize(totalGlyphs);
            }

            device->drawGlyphRunRSXform(it.font(), it.glyphs(), (const SkRSXform*)it.pos(),
                                        it.glyphCount(), origin, paint);

        }
        positions += it.glyphCount();
    }

    if (!this->empty()) {
        this->makeGlyphRunList(&blob, origin);
        device->drawGlyphRunList(this->useGlyphRunList(), paint);
    }
}

void SkGlyphRunBuilder::textBlobToGlyphRunListIgnoringRSXForm(
        const SkTextBlob& blob, SkPoint origin) {
    // Figure out all the storage needed to pre-size everything below.
    size_t totalGlyphs = 0;
    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        totalGlyphs += it.glyphCount();
    }

    // Pre-size all the buffers so they don't move during processing.
    this->initialize(totalGlyphs);

    SkPoint* positions = fPositions;

    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        simplifyTextBlobIgnoringRSXForm(it, positions);
        positions += it.glyphCount();
    }

    if (!this->empty()) {
        this->makeGlyphRunList(&blob, origin);
    }
}

void SkGlyphRunBuilder::simplifyTextBlobIgnoringRSXForm(const SkTextBlobRunIterator& it,
                                                        SkPoint* positions) {
    size_t runSize = it.glyphCount();

    auto text = SkSpan<const char>(it.text(), it.textSize());
    auto clusters = SkSpan<const uint32_t>(it.clusters(), runSize);
    const SkPoint& offset = it.offset();
    auto glyphIDs = SkSpan<const SkGlyphID>{it.glyphs(), runSize};

    switch (it.positioning()) {
        case SkTextBlobRunIterator::kDefault_Positioning: {
            this->simplifyDrawText(
                    it.font(), glyphIDs, offset, positions, text, clusters);
            break;
        }
        case SkTextBlobRunIterator::kHorizontal_Positioning: {
            auto constY = offset.y();
            this->simplifyDrawPosTextH(
                    it.font(), glyphIDs, it.pos(), constY, positions, text, clusters);
            break;
        }
        case SkTextBlobRunIterator::kFull_Positioning: {
            this->simplifyDrawPosText(
                    it.font(), glyphIDs, (const SkPoint*) it.pos(), text, clusters);
            break;
        }
        case SkTextBlobRunIterator::kRSXform_Positioning: break;
    }
}

void SkGlyphRunBuilder::drawGlyphsWithPositions(const SkFont& font,
                                            SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos) {
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawPosText(font, glyphIDs, pos);
        this->makeGlyphRunList(nullptr, SkPoint::Make(0, 0));
    }
}

const SkGlyphRunList& SkGlyphRunBuilder::useGlyphRunList() {
    return fGlyphRunList;
}

void SkGlyphRunBuilder::initialize(size_t totalRunSize) {

    if (totalRunSize > fMaxTotalRunSize) {
        fMaxTotalRunSize = totalRunSize;
        fPositions.reset(fMaxTotalRunSize);
    }

    fGlyphRunListStorage.clear();
}

SkSpan<const SkGlyphID> SkGlyphRunBuilder::textToGlyphIDs(
        const SkFont& font, const void* bytes, size_t byteLength, SkTextEncoding encoding) {
    if (encoding != SkTextEncoding::kGlyphID) {
        int count = font.countText(bytes, byteLength, encoding);
        if (count > 0) {
            fScratchGlyphIDs.resize(count);
            font.textToGlyphs(bytes, byteLength, encoding, fScratchGlyphIDs.data(), count);
            return SkSpan(fScratchGlyphIDs);
        } else {
            return SkSpan<const SkGlyphID>();
        }
    } else {
        return SkSpan<const SkGlyphID>((const SkGlyphID*)bytes, byteLength / 2);
    }
}

void SkGlyphRunBuilder::makeGlyphRun(
        const SkFont& font,
        SkSpan<const SkGlyphID> glyphIDs,
        SkSpan<const SkPoint> positions,
        SkSpan<const char> text,
        SkSpan<const uint32_t> clusters) {

    // Ignore empty runs.
    if (!glyphIDs.empty()) {
        fGlyphRunListStorage.emplace_back(
                font,
                positions,
                glyphIDs,
                text,
                clusters);
    }
}

void SkGlyphRunBuilder::makeGlyphRunList(const SkTextBlob* blob, SkPoint origin) {

    fGlyphRunList.~SkGlyphRunList();
    new (&fGlyphRunList) SkGlyphRunList{blob, origin, SkSpan(fGlyphRunListStorage)};
}

void SkGlyphRunBuilder::simplifyDrawText(
        const SkFont& font, SkSpan<const SkGlyphID> glyphIDs,
        SkPoint origin, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    SkASSERT(!glyphIDs.empty());

    auto runSize = glyphIDs.size();

    if (!glyphIDs.empty()) {
        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeWithNoDevice(font);
        SkBulkGlyphMetrics storage{strikeSpec};
        auto glyphs = storage.glyphs(glyphIDs);

        SkPoint endOfLastGlyph = origin;
        SkPoint* cursor = positions;
        for (auto glyph : glyphs) {
            *cursor++ = endOfLastGlyph;
            endOfLastGlyph += glyph->advanceVector();
        }

        this->makeGlyphRun(
                font,
                glyphIDs,
                SkSpan<const SkPoint>{positions, runSize},
                text,
                clusters);
    }
}

void SkGlyphRunBuilder::simplifyDrawPosTextH(
        const SkFont& font, SkSpan<const SkGlyphID> glyphIDs,
        const SkScalar* xpos, SkScalar constY, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    auto posCursor = positions;
    for (auto x : SkSpan<const SkScalar>{xpos, glyphIDs.size()}) {
        *posCursor++ = SkPoint::Make(x, constY);
    }

    simplifyDrawPosText(font, glyphIDs, positions, text, clusters);
}

void SkGlyphRunBuilder::simplifyDrawPosText(
        const SkFont& font, SkSpan<const SkGlyphID> glyphIDs,
        const SkPoint* pos,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    auto runSize = glyphIDs.size();

    this->makeGlyphRun(
            font,
            glyphIDs,
            SkSpan<const SkPoint>{pos, runSize},
            text,
            clusters);
}
