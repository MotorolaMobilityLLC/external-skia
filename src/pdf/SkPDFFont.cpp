/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ctype.h>

#include "SkData.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPDFCanon.h"
#include "SkPDFDevice.h"
#include "SkPDFFont.h"
#include "SkPDFFontImpl.h"
#include "SkPDFUtils.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTypefacePriv.h"
#include "SkTypes.h"
#include "SkUtils.h"

#if defined (SK_SFNTLY_SUBSETTER)
    #if defined (GOOGLE3)
        // #including #defines doesn't work with this build system.
        #include "typography/font/sfntly/src/sample/chromium/font_subsetter.h"
    #else
        #include SK_SFNTLY_SUBSETTER
    #endif
#endif

// PDF's notion of symbolic vs non-symbolic is related to the character set, not
// symbols vs. characters.  Rarely is a font the right character set to call it
// non-symbolic, so always call it symbolic.  (PDF 1.4 spec, section 5.7.1)
static const int kPdfSymbolic = 4;

struct AdvanceMetric {
    enum MetricType {
        kDefault,  // Default advance: fAdvance.count = 1
        kRange,    // Advances for a range: fAdvance.count = fEndID-fStartID
        kRun       // fStartID-fEndID have same advance: fAdvance.count = 1
    };
    MetricType fType;
    uint16_t fStartId;
    uint16_t fEndId;
    SkTDArray<int16_t> fAdvance;
    AdvanceMetric(uint16_t startId) : fStartId(startId) {}
    AdvanceMetric(AdvanceMetric&&) = default;
    AdvanceMetric& operator=(AdvanceMetric&& other) = default;
    AdvanceMetric(const AdvanceMetric&) = delete;
    AdvanceMetric& operator=(const AdvanceMetric&) = delete;
};

namespace {

///////////////////////////////////////////////////////////////////////////////
// File-Local Functions
///////////////////////////////////////////////////////////////////////////////

const int16_t kInvalidAdvance = SK_MinS16;
const int16_t kDontCareAdvance = SK_MinS16 + 1;

static void stripUninterestingTrailingAdvancesFromRange(
        AdvanceMetric* range) {
    SkASSERT(range);

    int expectedAdvanceCount = range->fEndId - range->fStartId + 1;
    if (range->fAdvance.count() < expectedAdvanceCount) {
        return;
    }

    for (int i = expectedAdvanceCount - 1; i >= 0; --i) {
        if (range->fAdvance[i] != kDontCareAdvance &&
            range->fAdvance[i] != kInvalidAdvance &&
            range->fAdvance[i] != 0) {
            range->fEndId = range->fStartId + i;
            break;
        }
    }
}

static void zeroWildcardsInRange(AdvanceMetric* range) {
    SkASSERT(range);
    if (range->fType != AdvanceMetric::kRange) {
        return;
    }
    SkASSERT(range->fAdvance.count() == range->fEndId - range->fStartId + 1);

    // Zero out wildcards.
    for (int i = 0; i < range->fAdvance.count(); ++i) {
        if (range->fAdvance[i] == kDontCareAdvance) {
            range->fAdvance[i] = 0;
        }
    }
}

static void FinishRange(
        AdvanceMetric* range,
        int endId,
        AdvanceMetric::MetricType type) {
    range->fEndId = endId;
    range->fType = type;
    stripUninterestingTrailingAdvancesFromRange(range);
    int newLength;
    if (type == AdvanceMetric::kRange) {
        newLength = range->fEndId - range->fStartId + 1;
    } else {
        if (range->fEndId == range->fStartId) {
            range->fType = AdvanceMetric::kRange;
        }
        newLength = 1;
    }
    SkASSERT(range->fAdvance.count() >= newLength);
    range->fAdvance.setCount(newLength);
    zeroWildcardsInRange(range);
}


/** Retrieve advance data for glyphs. Used by the PDF backend.
    @param num_glyphs    Total number of glyphs in the given font.
    @param glyphIDs      For per-glyph info, specify subset of the
                         font by giving glyph ids.  Each integer
                         represents a glyph id.  Passing nullptr
                         means all glyphs in the font.
    @param glyphIDsCount Number of elements in subsetGlyphIds.
                         Ignored if glyphIDs is nullptr.
*/
// TODO(halcanary): this function is complex enough to need its logic
// tested with unit tests.  On the other hand, I want to do another
// round of re-factoring before figuring out how to mock this.
// TODO(halcanary): this function should be combined with
// composeAdvanceData() so that we don't need to produce a linked list
// of intermediate values.  Or we could make the intermediate value
// something other than a linked list.
static void get_glyph_widths(SkSinglyLinkedList<AdvanceMetric>* glyphWidths,
                             int num_glyphs,
                             const uint32_t* subsetGlyphIDs,
                             uint32_t subsetGlyphIDsLength,
                             SkGlyphCache* glyphCache) {
    // Assuming that on average, the ASCII representation of an advance plus
    // a space is 8 characters and the ASCII representation of a glyph id is 3
    // characters, then the following cut offs for using different range types
    // apply:
    // The cost of stopping and starting the range is 7 characers
    //  a. Removing 4 0's or don't care's is a win
    // The cost of stopping and starting the range plus a run is 22
    // characters
    //  b. Removing 3 repeating advances is a win
    //  c. Removing 2 repeating advances and 3 don't cares is a win
    // When not currently in a range the cost of a run over a range is 16
    // characaters, so:
    //  d. Removing a leading 0/don't cares is a win because it is omitted
    //  e. Removing 2 repeating advances is a win

    AdvanceMetric* prevRange = nullptr;
    int16_t lastAdvance = kInvalidAdvance;
    int repeatedAdvances = 0;
    int wildCardsInRun = 0;
    int trailingWildCards = 0;
    uint32_t subsetIndex = 0;

    // Limit the loop count to glyph id ranges provided.
    int firstIndex = 0;
    int lastIndex = num_glyphs;
    if (subsetGlyphIDs) {
        firstIndex = static_cast<int>(subsetGlyphIDs[0]);
        lastIndex =
                static_cast<int>(subsetGlyphIDs[subsetGlyphIDsLength - 1]) + 1;
    }
    AdvanceMetric curRange(firstIndex);

    for (int gId = firstIndex; gId <= lastIndex; gId++) {
        int16_t advance = kInvalidAdvance;
        if (gId < lastIndex) {
            // Get glyph id only when subset is nullptr, or the id is in subset.
            SkASSERT(!subsetGlyphIDs || (subsetIndex < subsetGlyphIDsLength &&
                    static_cast<uint32_t>(gId) <= subsetGlyphIDs[subsetIndex]));
            if (!subsetGlyphIDs ||
                (subsetIndex < subsetGlyphIDsLength &&
                 static_cast<uint32_t>(gId) == subsetGlyphIDs[subsetIndex])) {
                advance = (int16_t)glyphCache->getGlyphIDAdvance(gId).fAdvanceX;
                ++subsetIndex;
            } else {
                advance = kDontCareAdvance;
            }
        }
        if (advance == lastAdvance) {
            repeatedAdvances++;
            trailingWildCards = 0;
        } else if (advance == kDontCareAdvance) {
            wildCardsInRun++;
            trailingWildCards++;
        } else if (curRange.fAdvance.count() ==
                   repeatedAdvances + 1 + wildCardsInRun) {  // All in run.
            if (lastAdvance == 0) {
                curRange.fStartId = gId;  // reset
                curRange.fAdvance.setCount(0);
                trailingWildCards = 0;
            } else if (repeatedAdvances + 1 >= 2 || trailingWildCards >= 4) {
                FinishRange(&curRange, gId - 1, AdvanceMetric::kRun);
                prevRange = glyphWidths->emplace_back(std::move(curRange));
                curRange = AdvanceMetric(gId);
                trailingWildCards = 0;
            }
            repeatedAdvances = 0;
            wildCardsInRun = trailingWildCards;
            trailingWildCards = 0;
        } else {
            if (lastAdvance == 0 &&
                    repeatedAdvances + 1 + wildCardsInRun >= 4) {
                FinishRange(&curRange,
                            gId - repeatedAdvances - wildCardsInRun - 2,
                            AdvanceMetric::kRange);
                prevRange = glyphWidths->emplace_back(std::move(curRange));
                curRange = AdvanceMetric(gId);
                trailingWildCards = 0;
            } else if (trailingWildCards >= 4 && repeatedAdvances + 1 < 2) {
                FinishRange(&curRange, gId - trailingWildCards - 1,
                            AdvanceMetric::kRange);
                prevRange = glyphWidths->emplace_back(std::move(curRange));
                curRange = AdvanceMetric(gId);
                trailingWildCards = 0;
            } else if (lastAdvance != 0 &&
                       (repeatedAdvances + 1 >= 3 ||
                        (repeatedAdvances + 1 >= 2 && wildCardsInRun >= 3))) {
                FinishRange(&curRange,
                            gId - repeatedAdvances - wildCardsInRun - 2,
                            AdvanceMetric::kRange);
                (void)glyphWidths->emplace_back(std::move(curRange));
                curRange =
                        AdvanceMetric(gId - repeatedAdvances - wildCardsInRun - 1);
                curRange.fAdvance.append(1, &lastAdvance);
                FinishRange(&curRange, gId - 1, AdvanceMetric::kRun);
                prevRange = glyphWidths->emplace_back(std::move(curRange));
                curRange = AdvanceMetric(gId);
                trailingWildCards = 0;
            }
            repeatedAdvances = 0;
            wildCardsInRun = trailingWildCards;
            trailingWildCards = 0;
        }
        curRange.fAdvance.append(1, &advance);
        if (advance != kDontCareAdvance) {
            lastAdvance = advance;
        }
    }
    if (curRange.fStartId == lastIndex) {
        if (!prevRange) {
            glyphWidths->reset();
            return;  // https://crbug.com/567031
        }
    } else {
        FinishRange(&curRange, lastIndex - 1, AdvanceMetric::kRange);
        glyphWidths->emplace_back(std::move(curRange));
    }
}

////////////////////////////////////////////////////////////////////////////////


bool parsePFBSection(const uint8_t** src, size_t* len, int sectionType,
                     size_t* size) {
    // PFB sections have a two or six bytes header. 0x80 and a one byte
    // section type followed by a four byte section length.  Type one is
    // an ASCII section (includes a length), type two is a binary section
    // (includes a length) and type three is an EOF marker with no length.
    const uint8_t* buf = *src;
    if (*len < 2 || buf[0] != 0x80 || buf[1] != sectionType) {
        return false;
    } else if (buf[1] == 3) {
        return true;
    } else if (*len < 6) {
        return false;
    }

    *size = (size_t)buf[2] | ((size_t)buf[3] << 8) | ((size_t)buf[4] << 16) |
            ((size_t)buf[5] << 24);
    size_t consumed = *size + 6;
    if (consumed > *len) {
        return false;
    }
    *src = *src + consumed;
    *len = *len - consumed;
    return true;
}

bool parsePFB(const uint8_t* src, size_t size, size_t* headerLen,
              size_t* dataLen, size_t* trailerLen) {
    const uint8_t* srcPtr = src;
    size_t remaining = size;

    return parsePFBSection(&srcPtr, &remaining, 1, headerLen) &&
           parsePFBSection(&srcPtr, &remaining, 2, dataLen) &&
           parsePFBSection(&srcPtr, &remaining, 1, trailerLen) &&
           parsePFBSection(&srcPtr, &remaining, 3, nullptr);
}

/* The sections of a PFA file are implicitly defined.  The body starts
 * after the line containing "eexec," and the trailer starts with 512
 * literal 0's followed by "cleartomark" (plus arbitrary white space).
 *
 * This function assumes that src is NUL terminated, but the NUL
 * termination is not included in size.
 *
 */
bool parsePFA(const char* src, size_t size, size_t* headerLen,
              size_t* hexDataLen, size_t* dataLen, size_t* trailerLen) {
    const char* end = src + size;

    const char* dataPos = strstr(src, "eexec");
    if (!dataPos) {
        return false;
    }
    dataPos += strlen("eexec");
    while ((*dataPos == '\n' || *dataPos == '\r' || *dataPos == ' ') &&
            dataPos < end) {
        dataPos++;
    }
    *headerLen = dataPos - src;

    const char* trailerPos = strstr(dataPos, "cleartomark");
    if (!trailerPos) {
        return false;
    }
    int zeroCount = 0;
    for (trailerPos--; trailerPos > dataPos && zeroCount < 512; trailerPos--) {
        if (*trailerPos == '\n' || *trailerPos == '\r' || *trailerPos == ' ') {
            continue;
        } else if (*trailerPos == '0') {
            zeroCount++;
        } else {
            return false;
        }
    }
    if (zeroCount != 512) {
        return false;
    }

    *hexDataLen = trailerPos - src - *headerLen;
    *trailerLen = size - *headerLen - *hexDataLen;

    // Verify that the data section is hex encoded and count the bytes.
    int nibbles = 0;
    for (; dataPos < trailerPos; dataPos++) {
        if (isspace(*dataPos)) {
            continue;
        }
        if (!isxdigit(*dataPos)) {
            return false;
        }
        nibbles++;
    }
    *dataLen = (nibbles + 1) / 2;

    return true;
}

int8_t hexToBin(uint8_t c) {
    if (!isxdigit(c)) {
        return -1;
    } else if (c <= '9') {
        return c - '0';
    } else if (c <= 'F') {
        return c - 'A' + 10;
    } else if (c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

static sk_sp<SkData> handle_type1_stream(SkStream* srcStream, size_t* headerLen,
                                         size_t* dataLen, size_t* trailerLen) {
    // srcStream may be backed by a file or a unseekable fd, so we may not be
    // able to use skip(), rewind(), or getMemoryBase().  read()ing through
    // the input only once is doable, but very ugly. Furthermore, it'd be nice
    // if the data was NUL terminated so that we can use strstr() to search it.
    // Make as few copies as possible given these constraints.
    SkDynamicMemoryWStream dynamicStream;
    std::unique_ptr<SkMemoryStream> staticStream;
    sk_sp<SkData> data;
    const uint8_t* src;
    size_t srcLen;
    if ((srcLen = srcStream->getLength()) > 0) {
        staticStream.reset(new SkMemoryStream(srcLen + 1));
        src = (const uint8_t*)staticStream->getMemoryBase();
        if (srcStream->getMemoryBase() != nullptr) {
            memcpy((void *)src, srcStream->getMemoryBase(), srcLen);
        } else {
            size_t read = 0;
            while (read < srcLen) {
                size_t got = srcStream->read((void *)staticStream->getAtPos(),
                                             srcLen - read);
                if (got == 0) {
                    return nullptr;
                }
                read += got;
                staticStream->seek(read);
            }
        }
        ((uint8_t *)src)[srcLen] = 0;
    } else {
        static const size_t kBufSize = 4096;
        uint8_t buf[kBufSize];
        size_t amount;
        while ((amount = srcStream->read(buf, kBufSize)) > 0) {
            dynamicStream.write(buf, amount);
        }
        amount = 0;
        dynamicStream.write(&amount, 1);  // nullptr terminator.
        data.reset(dynamicStream.copyToData());
        src = data->bytes();
        srcLen = data->size() - 1;
    }

    if (parsePFB(src, srcLen, headerLen, dataLen, trailerLen)) {
        static const int kPFBSectionHeaderLength = 6;
        const size_t length = *headerLen + *dataLen + *trailerLen;
        SkASSERT(length > 0);
        SkASSERT(length + (2 * kPFBSectionHeaderLength) <= srcLen);

        sk_sp<SkData> data(SkData::MakeUninitialized(length));

        const uint8_t* const srcHeader = src + kPFBSectionHeaderLength;
        // There is a six-byte section header before header and data
        // (but not trailer) that we're not going to copy.
        const uint8_t* const srcData = srcHeader + *headerLen + kPFBSectionHeaderLength;
        const uint8_t* const srcTrailer = srcData + *headerLen;

        uint8_t* const resultHeader = (uint8_t*)data->writable_data();
        uint8_t* const resultData = resultHeader + *headerLen;
        uint8_t* const resultTrailer = resultData + *dataLen;

        SkASSERT(resultTrailer + *trailerLen == resultHeader + length);

        memcpy(resultHeader,  srcHeader,  *headerLen);
        memcpy(resultData,    srcData,    *dataLen);
        memcpy(resultTrailer, srcTrailer, *trailerLen);

        return data;
    }

    // A PFA has to be converted for PDF.
    size_t hexDataLen;
    if (parsePFA((const char*)src, srcLen, headerLen, &hexDataLen, dataLen,
                 trailerLen)) {
        const size_t length = *headerLen + *dataLen + *trailerLen;
        SkASSERT(length > 0);
        SkAutoTMalloc<uint8_t> buffer(length);

        memcpy(buffer.get(), src, *headerLen);
        uint8_t* const resultData = &(buffer[SkToInt(*headerLen)]);

        const uint8_t* hexData = src + *headerLen;
        const uint8_t* trailer = hexData + hexDataLen;
        size_t outputOffset = 0;
        uint8_t dataByte = 0;  // To hush compiler.
        bool highNibble = true;
        for (; hexData < trailer; hexData++) {
            int8_t curNibble = hexToBin(*hexData);
            if (curNibble < 0) {
                continue;
            }
            if (highNibble) {
                dataByte = curNibble << 4;
                highNibble = false;
            } else {
                dataByte |= curNibble;
                highNibble = true;
                resultData[outputOffset++] = dataByte;
            }
        }
        if (!highNibble) {
            resultData[outputOffset++] = dataByte;
        }
        SkASSERT(outputOffset == *dataLen);

        uint8_t* const resultTrailer = &(buffer[SkToInt(*headerLen + outputOffset)]);
        memcpy(resultTrailer, src + *headerLen + hexDataLen, *trailerLen);

        return SkData::MakeFromMalloc(buffer.release(), length);
    }
    return nullptr;
}

// scale from em-units to base-1000, returning as a SkScalar
SkScalar scaleFromFontUnits(int16_t val, uint16_t emSize) {
    SkScalar scaled = SkIntToScalar(val);
    if (emSize == 1000) {
        return scaled;
    } else {
        return SkScalarMulDiv(scaled, 1000, emSize);
    }
}

void setGlyphWidthAndBoundingBox(SkScalar width, SkIRect box,
                                 SkWStream* content) {
    // Specify width and bounding box for the glyph.
    SkPDFUtils::AppendScalar(width, content);
    content->writeText(" 0 ");
    content->writeDecAsText(box.fLeft);
    content->writeText(" ");
    content->writeDecAsText(box.fTop);
    content->writeText(" ");
    content->writeDecAsText(box.fRight);
    content->writeText(" ");
    content->writeDecAsText(box.fBottom);
    content->writeText(" d1\n");
}

static sk_sp<SkPDFArray> makeFontBBox(SkIRect glyphBBox, uint16_t emSize) {
    auto bbox = sk_make_sp<SkPDFArray>();
    bbox->reserve(4);
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fLeft, emSize));
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fBottom, emSize));
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fRight, emSize));
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fTop, emSize));
    return bbox;
}

sk_sp<SkPDFArray> composeAdvanceData(
        const SkSinglyLinkedList<AdvanceMetric>& advanceInfo,
        uint16_t emSize,
        int16_t* defaultAdvance) {
    auto result = sk_make_sp<SkPDFArray>();
    for (const AdvanceMetric& range : advanceInfo) {
        switch (range.fType) {
            case AdvanceMetric::kDefault: {
                SkASSERT(range.fAdvance.count() == 1);
                *defaultAdvance = range.fAdvance[0];
                break;
            }
            case AdvanceMetric::kRange: {
                auto advanceArray = sk_make_sp<SkPDFArray>();
                for (int j = 0; j < range.fAdvance.count(); j++)
                    advanceArray->appendScalar(
                            scaleFromFontUnits(range.fAdvance[j], emSize));
                result->appendInt(range.fStartId);
                result->appendObject(std::move(advanceArray));
                break;
            }
            case AdvanceMetric::kRun: {
                SkASSERT(range.fAdvance.count() == 1);
                result->appendInt(range.fStartId);
                result->appendInt(range.fEndId);
                result->appendScalar(
                        scaleFromFontUnits(range.fAdvance[0], emSize));
                break;
            }
        }
    }
    return result;
}

}  // namespace

static void append_tounicode_header(SkDynamicMemoryWStream* cmap,
                                    uint16_t firstGlyphID,
                                    uint16_t lastGlyphID) {
    // 12 dict begin: 12 is an Adobe-suggested value. Shall not change.
    // It's there to prevent old version Adobe Readers from malfunctioning.
    const char* kHeader =
        "/CIDInit /ProcSet findresource begin\n"
        "12 dict begin\n"
        "begincmap\n";
    cmap->writeText(kHeader);

    // The /CIDSystemInfo must be consistent to the one in
    // SkPDFFont::populateCIDFont().
    // We can not pass over the system info object here because the format is
    // different. This is not a reference object.
    const char* kSysInfo =
        "/CIDSystemInfo\n"
        "<<  /Registry (Adobe)\n"
        "/Ordering (UCS)\n"
        "/Supplement 0\n"
        ">> def\n";
    cmap->writeText(kSysInfo);

    // The CMapName must be consistent to /CIDSystemInfo above.
    // /CMapType 2 means ToUnicode.
    // Codespace range just tells the PDF processor the valid range.
    const char* kTypeInfoHeader =
        "/CMapName /Adobe-Identity-UCS def\n"
        "/CMapType 2 def\n"
        "1 begincodespacerange\n";
    cmap->writeText(kTypeInfoHeader);

    // e.g.     "<0000> <FFFF>\n"
    SkString range;
    range.appendf("<%04X> <%04X>\n", firstGlyphID, lastGlyphID);
    cmap->writeText(range.c_str());

    const char* kTypeInfoFooter = "endcodespacerange\n";
    cmap->writeText(kTypeInfoFooter);
}

static void append_cmap_footer(SkDynamicMemoryWStream* cmap) {
    const char* kFooter =
        "endcmap\n"
        "CMapName currentdict /CMap defineresource pop\n"
        "end\n"
        "end";
    cmap->writeText(kFooter);
}

struct BFChar {
    uint16_t fGlyphId;
    SkUnichar fUnicode;
};

struct BFRange {
    uint16_t fStart;
    uint16_t fEnd;
    SkUnichar fUnicode;
};

static void write_utf16be(SkDynamicMemoryWStream* wStream, SkUnichar utf32) {
    uint16_t utf16[2] = {0, 0};
    size_t len = SkUTF16_FromUnichar(utf32, utf16);
    SkASSERT(len == 1 || len == 2);
    SkPDFUtils::WriteUInt16BE(wStream, utf16[0]);
    if (len == 2) {
        SkPDFUtils::WriteUInt16BE(wStream, utf16[1]);
    }
}

static void append_bfchar_section(const SkTDArray<BFChar>& bfchar,
                                  SkDynamicMemoryWStream* cmap) {
    // PDF spec defines that every bf* list can have at most 100 entries.
    for (int i = 0; i < bfchar.count(); i += 100) {
        int count = bfchar.count() - i;
        count = SkMin32(count, 100);
        cmap->writeDecAsText(count);
        cmap->writeText(" beginbfchar\n");
        for (int j = 0; j < count; ++j) {
            cmap->writeText("<");
            SkPDFUtils::WriteUInt16BE(cmap, bfchar[i + j].fGlyphId);
            cmap->writeText("> <");
            write_utf16be(cmap, bfchar[i + j].fUnicode);
            cmap->writeText(">\n");
        }
        cmap->writeText("endbfchar\n");
    }
}

static void append_bfrange_section(const SkTDArray<BFRange>& bfrange,
                                   SkDynamicMemoryWStream* cmap) {
    // PDF spec defines that every bf* list can have at most 100 entries.
    for (int i = 0; i < bfrange.count(); i += 100) {
        int count = bfrange.count() - i;
        count = SkMin32(count, 100);
        cmap->writeDecAsText(count);
        cmap->writeText(" beginbfrange\n");
        for (int j = 0; j < count; ++j) {
            cmap->writeText("<");
            SkPDFUtils::WriteUInt16BE(cmap, bfrange[i + j].fStart);
            cmap->writeText("> <");
            SkPDFUtils::WriteUInt16BE(cmap, bfrange[i + j].fEnd);
            cmap->writeText("> <");
            write_utf16be(cmap, bfrange[i + j].fUnicode);
            cmap->writeText(">\n");
        }
        cmap->writeText("endbfrange\n");
    }
}

// Generate <bfchar> and <bfrange> table according to PDF spec 1.4 and Adobe
// Technote 5014.
// The function is not static so we can test it in unit tests.
//
// Current implementation guarantees bfchar and bfrange entries do not overlap.
//
// Current implementation does not attempt aggresive optimizations against
// following case because the specification is not clear.
//
// 4 beginbfchar          1 beginbfchar
// <0003> <0013>          <0020> <0014>
// <0005> <0015>    to    endbfchar
// <0007> <0017>          1 beginbfrange
// <0020> <0014>          <0003> <0007> <0013>
// endbfchar              endbfrange
//
// Adobe Technote 5014 said: "Code mappings (unlike codespace ranges) may
// overlap, but succeeding maps supersede preceding maps."
//
// In case of searching text in PDF, bfrange will have higher precedence so
// typing char id 0x0014 in search box will get glyph id 0x0004 first.  However,
// the spec does not mention how will this kind of conflict being resolved.
//
// For the worst case (having 65536 continuous unicode and we use every other
// one of them), the possible savings by aggressive optimization is 416KB
// pre-compressed and does not provide enough motivation for implementation.

// TODO(halcanary): this should be in a header so that it is separately testable
// ( see caller in tests/ToUnicode.cpp )
void append_cmap_sections(const SkTDArray<SkUnichar>& glyphToUnicode,
                          const SkPDFGlyphSet* subset,
                          SkDynamicMemoryWStream* cmap,
                          bool multiByteGlyphs,
                          uint16_t firstGlyphID,
                          uint16_t lastGlyphID);

void append_cmap_sections(const SkTDArray<SkUnichar>& glyphToUnicode,
                          const SkPDFGlyphSet* subset,
                          SkDynamicMemoryWStream* cmap,
                          bool multiByteGlyphs,
                          uint16_t firstGlyphID,
                          uint16_t lastGlyphID) {
    if (glyphToUnicode.isEmpty()) {
        return;
    }
    int glyphOffset = 0;
    if (!multiByteGlyphs) {
        glyphOffset = firstGlyphID - 1;
    }

    SkTDArray<BFChar> bfcharEntries;
    SkTDArray<BFRange> bfrangeEntries;

    BFRange currentRangeEntry = {0, 0, 0};
    bool rangeEmpty = true;
    const int limit =
            SkMin32(lastGlyphID + 1, glyphToUnicode.count()) - glyphOffset;

    for (int i = firstGlyphID - glyphOffset; i < limit + 1; ++i) {
        bool inSubset = i < limit &&
                        (subset == nullptr || subset->has(i + glyphOffset));
        if (!rangeEmpty) {
            // PDF spec requires bfrange not changing the higher byte,
            // e.g. <1035> <10FF> <2222> is ok, but
            //      <1035> <1100> <2222> is no good
            bool inRange =
                i == currentRangeEntry.fEnd + 1 &&
                i >> 8 == currentRangeEntry.fStart >> 8 &&
                i < limit &&
                glyphToUnicode[i + glyphOffset] ==
                    currentRangeEntry.fUnicode + i - currentRangeEntry.fStart;
            if (!inSubset || !inRange) {
                if (currentRangeEntry.fEnd > currentRangeEntry.fStart) {
                    bfrangeEntries.push(currentRangeEntry);
                } else {
                    BFChar* entry = bfcharEntries.append();
                    entry->fGlyphId = currentRangeEntry.fStart;
                    entry->fUnicode = currentRangeEntry.fUnicode;
                }
                rangeEmpty = true;
            }
        }
        if (inSubset) {
            currentRangeEntry.fEnd = i;
            if (rangeEmpty) {
              currentRangeEntry.fStart = i;
              currentRangeEntry.fUnicode = glyphToUnicode[i + glyphOffset];
              rangeEmpty = false;
            }
        }
    }

    // The spec requires all bfchar entries for a font must come before bfrange
    // entries.
    append_bfchar_section(bfcharEntries, cmap);
    append_bfrange_section(bfrangeEntries, cmap);
}

static sk_sp<SkPDFStream> generate_tounicode_cmap(
        const SkTDArray<SkUnichar>& glyphToUnicode,
        const SkPDFGlyphSet* subset,
        bool multiByteGlyphs,
        uint16_t firstGlyphID,
        uint16_t lastGlyphID) {
    SkDynamicMemoryWStream cmap;
    if (multiByteGlyphs) {
        append_tounicode_header(&cmap, firstGlyphID, lastGlyphID);
    } else {
        append_tounicode_header(&cmap, 1, lastGlyphID - firstGlyphID + 1);
    }
    append_cmap_sections(glyphToUnicode, subset, &cmap, multiByteGlyphs,
                         firstGlyphID, lastGlyphID);
    append_cmap_footer(&cmap);
    return sk_make_sp<SkPDFStream>(
            std::unique_ptr<SkStreamAsset>(cmap.detachAsStream()));
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFGlyphSet
///////////////////////////////////////////////////////////////////////////////

SkPDFGlyphSet::SkPDFGlyphSet() : fBitSet(SK_MaxU16 + 1) {
}

void SkPDFGlyphSet::set(const uint16_t* glyphIDs, int numGlyphs) {
    for (int i = 0; i < numGlyphs; ++i) {
        fBitSet.setBit(glyphIDs[i], true);
    }
}

bool SkPDFGlyphSet::has(uint16_t glyphID) const {
    return fBitSet.isBitSet(glyphID);
}

void SkPDFGlyphSet::exportTo(SkTDArray<unsigned int>* glyphIDs) const {
    fBitSet.exportTo(glyphIDs);
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFGlyphSetMap
///////////////////////////////////////////////////////////////////////////////

SkPDFGlyphSetMap::SkPDFGlyphSetMap() {}

SkPDFGlyphSetMap::~SkPDFGlyphSetMap() {
    fMap.reset();
}

void SkPDFGlyphSetMap::noteGlyphUsage(SkPDFFont* font, const uint16_t* glyphIDs,
                                      int numGlyphs) {
    SkPDFGlyphSet* subset = getGlyphSetForFont(font);
    if (subset) {
        subset->set(glyphIDs, numGlyphs);
    }
}

SkPDFGlyphSet* SkPDFGlyphSetMap::getGlyphSetForFont(SkPDFFont* font) {
    int index = fMap.count();
    for (int i = 0; i < index; ++i) {
        if (fMap[i].fFont == font) {
            return &fMap[i].fGlyphSet;
        }
    }
    FontGlyphSetPair& pair = fMap.push_back();
    pair.fFont = font;
    return &pair.fGlyphSet;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFFont
///////////////////////////////////////////////////////////////////////////////

/* Font subset design: It would be nice to be able to subset fonts
 * (particularly type 3 fonts), but it's a lot of work and not a priority.
 *
 * Resources are canonicalized and uniqueified by pointer so there has to be
 * some additional state indicating which subset of the font is used.  It
 * must be maintained at the page granularity and then combined at the document
 * granularity. a) change SkPDFFont to fill in its state on demand, kind of
 * like SkPDFGraphicState.  b) maintain a per font glyph usage class in each
 * page/pdf device. c) in the document, retrieve the per font glyph usage
 * from each page and combine it and ask for a resource with that subset.
 */

SkPDFFont::~SkPDFFont() {}

SkTypeface* SkPDFFont::typeface() {
    return fTypeface.get();
}

SkAdvancedTypefaceMetrics::FontType SkPDFFont::getType() {
    return fFontType;
}

bool SkPDFFont::canEmbed() const {
    if (!fFontInfo.get()) {
        SkASSERT(fFontType == SkAdvancedTypefaceMetrics::kOther_Font);
        return true;
    }
    return (fFontInfo->fFlags &
            SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag) == 0;
}

bool SkPDFFont::canSubset() const {
    if (!fFontInfo.get()) {
        SkASSERT(fFontType == SkAdvancedTypefaceMetrics::kOther_Font);
        return true;
    }
    return (fFontInfo->fFlags &
            SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag) == 0;
}

bool SkPDFFont::hasGlyph(uint16_t id) {
    return (id >= fFirstGlyphID && id <= fLastGlyphID) || id == 0;
}

int SkPDFFont::glyphsToPDFFontEncoding(uint16_t* glyphIDs, int numGlyphs) {
    // A font with multibyte glyphs will support all glyph IDs in a single font.
    if (this->multiByteGlyphs()) {
        return numGlyphs;
    }

    for (int i = 0; i < numGlyphs; i++) {
        if (glyphIDs[i] == 0) {
            continue;
        }
        if (glyphIDs[i] < fFirstGlyphID || glyphIDs[i] > fLastGlyphID) {
            return i;
        }
        glyphIDs[i] -= (fFirstGlyphID - 1);
    }

    return numGlyphs;
}

// static
SkPDFFont* SkPDFFont::GetFontResource(SkPDFCanon* canon,
                                      SkTypeface* typeface,
                                      uint16_t glyphID) {
    SkASSERT(canon);
    SkAutoResolveDefaultTypeface autoResolve(typeface);
    typeface = autoResolve.get();
    const uint32_t fontID = typeface->uniqueID();

    SkPDFFont* relatedFont;
    if (SkPDFFont* pdfFont = canon->findFont(fontID, glyphID, &relatedFont)) {
        return SkRef(pdfFont);
    }

    sk_sp<const SkAdvancedTypefaceMetrics> fontMetrics;
    SkPDFDict* relatedFontDescriptor = nullptr;
    if (relatedFont) {
        fontMetrics.reset(SkSafeRef(relatedFont->fontInfo()));
        relatedFontDescriptor = relatedFont->getFontDescriptor();

        // This only is to catch callers who pass invalid glyph ids.
        // If glyph id is invalid, then we will create duplicate entries
        // for TrueType fonts.
        SkAdvancedTypefaceMetrics::FontType fontType =
            fontMetrics.get() ? fontMetrics.get()->fType :
                                SkAdvancedTypefaceMetrics::kOther_Font;

        if (fontType == SkAdvancedTypefaceMetrics::kType1CID_Font ||
            fontType == SkAdvancedTypefaceMetrics::kTrueType_Font) {
            return SkRef(relatedFont);
        }
    } else {
        SkTypeface::PerGlyphInfo info =
            SkTBitOr(SkTypeface::kGlyphNames_PerGlyphInfo,
                     SkTypeface::kToUnicode_PerGlyphInfo);
        fontMetrics.reset(
            typeface->getAdvancedTypefaceMetrics(info, nullptr, 0));
    }

    SkPDFFont* font = SkPDFFont::Create(canon, fontMetrics.get(), typeface,
                                        glyphID, relatedFontDescriptor);
    canon->addFont(font, fontID, font->fFirstGlyphID);
    return font;
}

SkPDFFont* SkPDFFont::getFontSubset(const SkPDFGlyphSet*) {
    return nullptr;  // Default: no support.
}

SkPDFFont::SkPDFFont(const SkAdvancedTypefaceMetrics* info,
                     SkTypeface* typeface,
                     SkPDFDict* relatedFontDescriptor)
    : SkPDFDict("Font")
    , fTypeface(ref_or_default(typeface))
    , fFirstGlyphID(1)
    , fLastGlyphID(info ? info->fLastGlyphID : 0)
    , fFontInfo(SkSafeRef(info))
    , fDescriptor(SkSafeRef(relatedFontDescriptor)) {
    if (info == nullptr ||
            info->fFlags & SkAdvancedTypefaceMetrics::kMultiMaster_FontFlag) {
        fFontType = SkAdvancedTypefaceMetrics::kOther_Font;
    } else {
        fFontType = info->fType;
    }
}

// static
SkPDFFont* SkPDFFont::Create(SkPDFCanon* canon,
                             const SkAdvancedTypefaceMetrics* info,
                             SkTypeface* typeface,
                             uint16_t glyphID,
                             SkPDFDict* relatedFontDescriptor) {
    SkAdvancedTypefaceMetrics::FontType type =
        info ? info->fType : SkAdvancedTypefaceMetrics::kOther_Font;

    if (info && (info->fFlags & SkAdvancedTypefaceMetrics::kMultiMaster_FontFlag)) {
        return new SkPDFType3Font(info, typeface, glyphID);
    }
    if (type == SkAdvancedTypefaceMetrics::kType1CID_Font ||
        type == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        SkASSERT(relatedFontDescriptor == nullptr);
        return new SkPDFType0Font(info, typeface);
    }
    if (type == SkAdvancedTypefaceMetrics::kType1_Font) {
        return new SkPDFType1Font(info, typeface, glyphID, relatedFontDescriptor);
    }

    SkASSERT(type == SkAdvancedTypefaceMetrics::kCFF_Font ||
             type == SkAdvancedTypefaceMetrics::kOther_Font);

    return new SkPDFType3Font(info, typeface, glyphID);
}

const SkAdvancedTypefaceMetrics* SkPDFFont::fontInfo() {
    return fFontInfo.get();
}

void SkPDFFont::setFontInfo(const SkAdvancedTypefaceMetrics* info) {
    if (info == nullptr || info == fFontInfo.get()) {
        return;
    }
    fFontInfo.reset(info);
    SkSafeRef(info);
}

uint16_t SkPDFFont::firstGlyphID() const {
    return fFirstGlyphID;
}

uint16_t SkPDFFont::lastGlyphID() const {
    return fLastGlyphID;
}

void SkPDFFont::setLastGlyphID(uint16_t glyphID) {
    fLastGlyphID = glyphID;
}

SkPDFDict* SkPDFFont::getFontDescriptor() {
    return fDescriptor.get();
}

void SkPDFFont::setFontDescriptor(SkPDFDict* descriptor) {
    fDescriptor.reset(descriptor);
    SkSafeRef(descriptor);
}

bool SkPDFFont::addCommonFontDescriptorEntries(int16_t defaultWidth) {
    if (fDescriptor.get() == nullptr) {
        return false;
    }

    const uint16_t emSize = fFontInfo->fEmSize;

    fDescriptor->insertName("FontName", fFontInfo->fFontName);
    fDescriptor->insertInt("Flags", fFontInfo->fStyle | kPdfSymbolic);
    fDescriptor->insertScalar("Ascent",
            scaleFromFontUnits(fFontInfo->fAscent, emSize));
    fDescriptor->insertScalar("Descent",
            scaleFromFontUnits(fFontInfo->fDescent, emSize));
    fDescriptor->insertScalar("StemV",
            scaleFromFontUnits(fFontInfo->fStemV, emSize));

    fDescriptor->insertScalar("CapHeight",
            scaleFromFontUnits(fFontInfo->fCapHeight, emSize));
    fDescriptor->insertInt("ItalicAngle", fFontInfo->fItalicAngle);
    fDescriptor->insertObject(
            "FontBBox", makeFontBBox(fFontInfo->fBBox, fFontInfo->fEmSize));

    if (defaultWidth > 0) {
        fDescriptor->insertScalar("MissingWidth",
                scaleFromFontUnits(defaultWidth, emSize));
    }
    return true;
}

void SkPDFFont::adjustGlyphRangeForSingleByteEncoding(uint16_t glyphID) {
    // Single byte glyph encoding supports a max of 255 glyphs.
    fFirstGlyphID = glyphID - (glyphID - 1) % 255;
    if (fLastGlyphID > fFirstGlyphID + 255 - 1) {
        fLastGlyphID = fFirstGlyphID + 255 - 1;
    }
}

void SkPDFFont::populateToUnicodeTable(const SkPDFGlyphSet* subset) {
    if (fFontInfo == nullptr || fFontInfo->fGlyphToUnicode.begin() == nullptr) {
        return;
    }
    this->insertObjRef("ToUnicode",
                       generate_tounicode_cmap(fFontInfo->fGlyphToUnicode,
                                               subset,
                                               multiByteGlyphs(),
                                               firstGlyphID(),
                                               lastGlyphID()));
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType0Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType0Font::SkPDFType0Font(const SkAdvancedTypefaceMetrics* info, SkTypeface* typeface)
    : SkPDFFont(info, typeface, nullptr) {
    SkDEBUGCODE(fPopulated = false);
    if (!canSubset()) {
        this->populate(nullptr);
    }
}

SkPDFType0Font::~SkPDFType0Font() {}

SkPDFFont* SkPDFType0Font::getFontSubset(const SkPDFGlyphSet* subset) {
    if (!canSubset()) {
        return nullptr;
    }
    SkPDFType0Font* newSubset = new SkPDFType0Font(fontInfo(), typeface());
    newSubset->populate(subset);
    return newSubset;
}

#ifdef SK_DEBUG
void SkPDFType0Font::emitObject(SkWStream* stream,
                                const SkPDFObjNumMap& objNumMap,
                                const SkPDFSubstituteMap& substitutes) const {
    SkASSERT(fPopulated);
    return INHERITED::emitObject(stream, objNumMap, substitutes);
}
#endif

bool SkPDFType0Font::populate(const SkPDFGlyphSet* subset) {
    insertName("Subtype", "Type0");
    insertName("BaseFont", fontInfo()->fFontName);
    insertName("Encoding", "Identity-H");

    sk_sp<SkPDFCIDFont> newCIDFont(
            new SkPDFCIDFont(fontInfo(), typeface(), subset));
    auto descendantFonts = sk_make_sp<SkPDFArray>();
    descendantFonts->appendObjRef(std::move(newCIDFont));
    this->insertObject("DescendantFonts", std::move(descendantFonts));

    this->populateToUnicodeTable(subset);

    SkDEBUGCODE(fPopulated = true);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFCIDFont
///////////////////////////////////////////////////////////////////////////////

SkPDFCIDFont::SkPDFCIDFont(const SkAdvancedTypefaceMetrics* info,
                           SkTypeface* typeface,
                           const SkPDFGlyphSet* subset)
    : SkPDFFont(info, typeface, nullptr) {
    this->populate(subset);
}

SkPDFCIDFont::~SkPDFCIDFont() {}

#ifdef SK_SFNTLY_SUBSETTER
// if possible, make no copy.
static sk_sp<SkData> stream_to_data(std::unique_ptr<SkStreamAsset> stream) {
    SkASSERT(stream);
    (void)stream->rewind();
    SkASSERT(stream->hasLength());
    size_t size = stream->getLength();
    if (const void* base = stream->getMemoryBase()) {
        SkData::ReleaseProc proc =
            [](const void*, void* ctx) { delete (SkStream*)ctx; };
        return SkData::MakeWithProc(base, size, proc, stream.release());
    }
    return SkData::MakeFromStream(stream.get(), size);
}

static sk_sp<SkPDFObject> get_subset_font_stream(
        std::unique_ptr<SkStreamAsset> fontAsset,
        const SkTDArray<uint32_t>& subset,
        const char* fontName) {
    // sfntly requires unsigned int* to be passed in,
    // as far as we know, unsigned int is equivalent
    // to uint32_t on all platforms.
    static_assert(sizeof(unsigned) == sizeof(uint32_t), "");

    // TODO(halcanary): Use ttcIndex, not fontName.

    unsigned char* subsetFont{nullptr};
    int subsetFontSize{0};
    {
        sk_sp<SkData> fontData(stream_to_data(std::move(fontAsset)));
        subsetFontSize =
            SfntlyWrapper::SubsetFont(fontName,
                                      fontData->bytes(),
                                      fontData->size(),
                                      subset.begin(),
                                      subset.count(),
                                      &subsetFont);
    }
    SkASSERT(subsetFontSize > 0 || subsetFont == nullptr);
    if (subsetFontSize < 1) {
        return nullptr;
    }
    SkASSERT(subsetFont != nullptr);
    auto subsetStream = sk_make_sp<SkPDFStream>(
            SkData::MakeWithProc(
                    subsetFont, subsetFontSize,
                    [](const void* p, void*) { delete[] (unsigned char*)p; },
                    nullptr));
    subsetStream->dict()->insertInt("Length1", subsetFontSize);
    return subsetStream;
}
#endif  // SK_SFNTLY_SUBSETTER

bool SkPDFCIDFont::addFontDescriptor(int16_t defaultWidth,
                                     const SkTDArray<uint32_t>* subset) {
    auto descriptor = sk_make_sp<SkPDFDict>("FontDescriptor");
    setFontDescriptor(descriptor.get());
    if (!addCommonFontDescriptorEntries(defaultWidth)) {
        this->insertObjRef("FontDescriptor", std::move(descriptor));
        return false;
    }
    SkASSERT(this->canEmbed());

    switch (getType()) {
        case SkAdvancedTypefaceMetrics::kTrueType_Font: {
            int ttcIndex;
            std::unique_ptr<SkStreamAsset> fontAsset(
                    this->typeface()->openStream(&ttcIndex));
            SkASSERT(fontAsset);
            if (!fontAsset) {
                return false;
            }
            size_t fontSize = fontAsset->getLength();
            SkASSERT(fontSize > 0);
            if (fontSize == 0) {
                return false;
            }

            #ifdef SK_SFNTLY_SUBSETTER
            if (this->canSubset() && subset) {
                sk_sp<SkPDFObject> subsetStream = get_subset_font_stream(
                        std::move(fontAsset), *subset, fontInfo()->fFontName.c_str());
                if (subsetStream) {
                    descriptor->insertObjRef("FontFile2", std::move(subsetStream));
                    break;
                }
                // If subsetting fails, fall back to original font data.
                fontAsset.reset(this->typeface()->openStream(&ttcIndex));
            }
            #endif  // SK_SFNTLY_SUBSETTER
            auto fontStream = sk_make_sp<SkPDFSharedStream>(std::move(fontAsset));
            fontStream->dict()->insertInt("Length1", fontSize);
            descriptor->insertObjRef("FontFile2", std::move(fontStream));
            break;
        }
        case SkAdvancedTypefaceMetrics::kCFF_Font:
        case SkAdvancedTypefaceMetrics::kType1CID_Font: {
            std::unique_ptr<SkStreamAsset> fontData(
                    this->typeface()->openStream(nullptr));
            SkASSERT(fontData);
            SkASSERT(fontData->getLength() > 0);
            if (!fontData || 0 == fontData->getLength()) {
                return false;
            }
            auto fontStream = sk_make_sp<SkPDFSharedStream>(std::move(fontData));
            if (getType() == SkAdvancedTypefaceMetrics::kCFF_Font) {
                fontStream->dict()->insertName("Subtype", "Type1C");
            } else {
                fontStream->dict()->insertName("Subtype", "CIDFontType0c");
            }
            descriptor->insertObjRef("FontFile3", std::move(fontStream));
            break;
        }
        default:
            SkASSERT(false);
    }
    this->insertObjRef("FontDescriptor", std::move(descriptor));
    return true;
}

void set_glyph_widths(SkTypeface* tf,
                      const SkTDArray<uint32_t>* glyphIDs,
                      SkSinglyLinkedList<AdvanceMetric>* dst) {
    SkPaint tmpPaint;
    tmpPaint.setHinting(SkPaint::kNo_Hinting);
    tmpPaint.setTypeface(sk_ref_sp(tf));
    tmpPaint.setTextSize((SkScalar)tf->getUnitsPerEm());
    SkAutoGlyphCache autoGlyphCache(tmpPaint, nullptr, nullptr);
    if (!glyphIDs || glyphIDs->isEmpty()) {
        get_glyph_widths(dst, tf->countGlyphs(), nullptr, 0, autoGlyphCache.get());
    } else {
        get_glyph_widths(dst, tf->countGlyphs(), glyphIDs->begin(),
                         glyphIDs->count(), autoGlyphCache.get());
    }
}

bool SkPDFCIDFont::populate(const SkPDFGlyphSet* subset) {
    // Generate new font metrics with advance info for true type fonts.
    // Generate glyph id array.
    SkTDArray<uint32_t> glyphIDs;
    if (subset) {
        if (!subset->has(0)) {
            glyphIDs.push(0);  // Always include glyph 0.
        }
        subset->exportTo(&glyphIDs);
    }
    if (fontInfo()->fType == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        SkTypeface::PerGlyphInfo info = SkTypeface::kGlyphNames_PerGlyphInfo;
        uint32_t* glyphs = (glyphIDs.count() == 0) ? nullptr : glyphIDs.begin();
        uint32_t glyphsCount = glyphs ? glyphIDs.count() : 0;
        sk_sp<const SkAdvancedTypefaceMetrics> fontMetrics(
            typeface()->getAdvancedTypefaceMetrics(info, glyphs, glyphsCount));
        setFontInfo(fontMetrics.get());
        addFontDescriptor(0, &glyphIDs);
    } else {
        // Other CID fonts
        addFontDescriptor(0, nullptr);
    }

    insertName("BaseFont", fontInfo()->fFontName);

    if (getType() == SkAdvancedTypefaceMetrics::kType1CID_Font) {
        insertName("Subtype", "CIDFontType0");
    } else if (getType() == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        insertName("Subtype", "CIDFontType2");
        insertName("CIDToGIDMap", "Identity");
    } else {
        SkASSERT(false);
    }

    auto sysInfo = sk_make_sp<SkPDFDict>();
    sysInfo->insertString("Registry", "Adobe");
    sysInfo->insertString("Ordering", "Identity");
    sysInfo->insertInt("Supplement", 0);
    this->insertObject("CIDSystemInfo", std::move(sysInfo));

    SkSinglyLinkedList<AdvanceMetric> tmpMetrics;
    set_glyph_widths(this->typeface(), &glyphIDs, &tmpMetrics);
    int16_t defaultWidth = 0;
    uint16_t emSize = (uint16_t)this->fontInfo()->fEmSize;
    sk_sp<SkPDFArray> widths = composeAdvanceData(tmpMetrics, emSize, &defaultWidth);
    if (widths->size()) {
        this->insertObject("W", std::move(widths));
    }

    this->insertScalar(
            "DW", scaleFromFontUnits(defaultWidth, emSize));
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType1Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType1Font::SkPDFType1Font(const SkAdvancedTypefaceMetrics* info,
                               SkTypeface* typeface,
                               uint16_t glyphID,
                               SkPDFDict* relatedFontDescriptor)
    : SkPDFFont(info, typeface, relatedFontDescriptor) {
    this->populate(glyphID);
}

SkPDFType1Font::~SkPDFType1Font() {}

bool SkPDFType1Font::addFontDescriptor(int16_t defaultWidth) {
    if (SkPDFDict* descriptor = getFontDescriptor()) {
        this->insertObjRef("FontDescriptor",
                           sk_ref_sp(descriptor));
        return true;
    }

    auto descriptor = sk_make_sp<SkPDFDict>("FontDescriptor");
    setFontDescriptor(descriptor.get());

    int ttcIndex;
    size_t header SK_INIT_TO_AVOID_WARNING;
    size_t data SK_INIT_TO_AVOID_WARNING;
    size_t trailer SK_INIT_TO_AVOID_WARNING;
    std::unique_ptr<SkStreamAsset> rawFontData(typeface()->openStream(&ttcIndex));
    SkASSERT(rawFontData);
    SkASSERT(rawFontData->getLength() > 0);
    if (!rawFontData || 0 == rawFontData->getLength()) {
        return false;
    }
    sk_sp<SkData> fontData(handle_type1_stream(rawFontData.get(), &header, &data, &trailer));
    if (fontData.get() == nullptr) {
        return false;
    }
    SkASSERT(this->canEmbed());
    auto fontStream = sk_make_sp<SkPDFStream>(std::move(fontData));
    fontStream->dict()->insertInt("Length1", header);
    fontStream->dict()->insertInt("Length2", data);
    fontStream->dict()->insertInt("Length3", trailer);
    descriptor->insertObjRef("FontFile", std::move(fontStream));

    this->insertObjRef("FontDescriptor", std::move(descriptor));

    return addCommonFontDescriptorEntries(defaultWidth);
}

bool SkPDFType1Font::populate(int16_t glyphID) {
    adjustGlyphRangeForSingleByteEncoding(glyphID);

    int16_t defaultWidth = 0;
    const AdvanceMetric* widthRangeEntry = nullptr;
    {
        SkSinglyLinkedList<AdvanceMetric> tmpMetrics;
        set_glyph_widths(this->typeface(), nullptr, &tmpMetrics);
        for (const auto& widthEntry : tmpMetrics) {
            switch (widthEntry.fType) {
                case AdvanceMetric::kDefault:
                    defaultWidth = widthEntry.fAdvance[0];
                    break;
                case AdvanceMetric::kRun:
                    SkASSERT(false);
                    break;
                case AdvanceMetric::kRange:
                    SkASSERT(widthRangeEntry == nullptr);
                    widthRangeEntry = &widthEntry;
                    break;
            }
        }
    }

    if (!addFontDescriptor(defaultWidth)) {
        return false;
    }

    insertName("Subtype", "Type1");
    insertName("BaseFont", fontInfo()->fFontName);

    addWidthInfoFromRange(defaultWidth, widthRangeEntry);
    auto encDiffs = sk_make_sp<SkPDFArray>();
    encDiffs->reserve(lastGlyphID() - firstGlyphID() + 2);
    encDiffs->appendInt(1);
    SkASSERT(this->fontInfo()->fGlyphNames.count() >= this->lastGlyphID());
    for (int gID = firstGlyphID(); gID <= lastGlyphID(); gID++) {
        encDiffs->appendName(fontInfo()->fGlyphNames[gID].c_str());
    }

    auto encoding = sk_make_sp<SkPDFDict>("Encoding");
    encoding->insertObject("Differences", std::move(encDiffs));
    this->insertObject("Encoding", std::move(encoding));
    return true;
}

void SkPDFType1Font::addWidthInfoFromRange(
        int16_t defaultWidth,
        const AdvanceMetric* widthRangeEntry) {
    auto widthArray = sk_make_sp<SkPDFArray>();
    int firstChar = 0;
    if (widthRangeEntry) {
        const uint16_t emSize = fontInfo()->fEmSize;
        int startIndex = firstGlyphID() - widthRangeEntry->fStartId;
        int endIndex = startIndex + lastGlyphID() - firstGlyphID() + 1;
        if (startIndex < 0)
            startIndex = 0;
        if (endIndex > widthRangeEntry->fAdvance.count())
            endIndex = widthRangeEntry->fAdvance.count();
        if (widthRangeEntry->fStartId == 0) {
            widthArray->appendScalar(
                    scaleFromFontUnits(widthRangeEntry->fAdvance[0], emSize));
        } else {
            firstChar = startIndex + widthRangeEntry->fStartId;
        }
        for (int i = startIndex; i < endIndex; i++) {
            widthArray->appendScalar(
                    scaleFromFontUnits(widthRangeEntry->fAdvance[i], emSize));
        }
    } else {
        widthArray->appendScalar(
                scaleFromFontUnits(defaultWidth, 1000));
    }
    this->insertInt("FirstChar", firstChar);
    this->insertInt("LastChar", firstChar + widthArray->size() - 1);
    this->insertObject("Widths", std::move(widthArray));
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType3Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType3Font::SkPDFType3Font(const SkAdvancedTypefaceMetrics* info,
                               SkTypeface* typeface,
                               uint16_t glyphID)
    : SkPDFFont(info, typeface, nullptr) {
    this->populate(glyphID);
}

SkPDFType3Font::~SkPDFType3Font() {}

bool SkPDFType3Font::populate(uint16_t glyphID) {
    SkPaint paint;
    paint.setTypeface(sk_ref_sp(this->typeface()));
    paint.setTextSize(1000);
    const SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    SkAutoGlyphCache autoCache(paint, &props, nullptr);
    SkGlyphCache* cache = autoCache.getCache();
    // If fLastGlyphID isn't set (because there is not fFontInfo), look it up.
    if (lastGlyphID() == 0) {
        setLastGlyphID(cache->getGlyphCount() - 1);
    }

    adjustGlyphRangeForSingleByteEncoding(glyphID);

    insertName("Subtype", "Type3");
    // Flip about the x-axis and scale by 1/1000.
    SkMatrix fontMatrix;
    fontMatrix.setScale(SkScalarInvert(1000), -SkScalarInvert(1000));
    this->insertObject("FontMatrix", SkPDFUtils::MatrixToArray(fontMatrix));

    auto charProcs = sk_make_sp<SkPDFDict>();
    auto encoding = sk_make_sp<SkPDFDict>("Encoding");

    auto encDiffs = sk_make_sp<SkPDFArray>();
    encDiffs->reserve(lastGlyphID() - firstGlyphID() + 2);
    encDiffs->appendInt(1);

    auto widthArray = sk_make_sp<SkPDFArray>();

    SkIRect bbox = SkIRect::MakeEmpty();
    for (int gID = firstGlyphID(); gID <= lastGlyphID(); gID++) {
        SkString characterName;
        characterName.printf("gid%d", gID);
        encDiffs->appendName(characterName.c_str());

        const SkGlyph& glyph = cache->getGlyphIDMetrics(gID);
        widthArray->appendScalar(SkFloatToScalar(glyph.fAdvanceX));
        SkIRect glyphBBox = SkIRect::MakeXYWH(glyph.fLeft, glyph.fTop,
                                              glyph.fWidth, glyph.fHeight);
        bbox.join(glyphBBox);

        SkDynamicMemoryWStream content;
        setGlyphWidthAndBoundingBox(SkFloatToScalar(glyph.fAdvanceX), glyphBBox,
                                    &content);
        const SkPath* path = cache->findPath(glyph);
        if (path) {
            SkPDFUtils::EmitPath(*path, paint.getStyle(), &content);
            SkPDFUtils::PaintPath(paint.getStyle(), path->getFillType(),
                                  &content);
        }
        charProcs->insertObjRef(
                characterName, sk_make_sp<SkPDFStream>(
                        std::unique_ptr<SkStreamAsset>(content.detachAsStream())));
    }

    encoding->insertObject("Differences", std::move(encDiffs));

    this->insertObject("CharProcs", std::move(charProcs));
    this->insertObject("Encoding", std::move(encoding));

    this->insertObject("FontBBox", makeFontBBox(bbox, 1000));
    this->insertInt("FirstChar", 1);
    this->insertInt("LastChar", lastGlyphID() - firstGlyphID() + 1);
    this->insertObject("Widths", std::move(widthArray));
    this->insertName("CIDToGIDMap", "Identity");

    this->populateToUnicodeTable(nullptr);
    return true;
}

SkPDFFont::Match SkPDFFont::IsMatch(SkPDFFont* existingFont,
                                    uint32_t existingFontID,
                                    uint16_t existingGlyphID,
                                    uint32_t searchFontID,
                                    uint16_t searchGlyphID) {
    if (existingFontID != searchFontID) {
        return SkPDFFont::kNot_Match;
    }
    if (existingGlyphID == 0 || searchGlyphID == 0) {
        return SkPDFFont::kExact_Match;
    }
    if (existingFont != nullptr) {
        return (existingFont->fFirstGlyphID <= searchGlyphID &&
                searchGlyphID <= existingFont->fLastGlyphID)
                       ? SkPDFFont::kExact_Match
                       : SkPDFFont::kRelated_Match;
    }
    return (existingGlyphID == searchGlyphID) ? SkPDFFont::kExact_Match
                                              : SkPDFFont::kRelated_Match;
}

//  Since getAdvancedTypefaceMetrics is expensive, cache the result.
bool SkPDFFont::CanEmbedTypeface(SkTypeface* typeface, SkPDFCanon* canon) {
    SkAutoResolveDefaultTypeface face(typeface);
    uint32_t id = face->uniqueID();
    if (bool* value = canon->fCanEmbedTypeface.find(id)) {
        return *value;
    }
    bool canEmbed = true;
    sk_sp<const SkAdvancedTypefaceMetrics> fontMetrics(
            face->getAdvancedTypefaceMetrics(
                    SkTypeface::kNo_PerGlyphInfo, nullptr, 0));
    if (fontMetrics) {
        canEmbed = !SkToBool(
                fontMetrics->fFlags &
                SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag);
    }
    return *canon->fCanEmbedTypeface.set(id, canEmbed);
}

void SkPDFFont::drop() {
    fTypeface = nullptr;
    fFontInfo = nullptr;
    fDescriptor = nullptr;
    this->SkPDFDict::drop();
}
