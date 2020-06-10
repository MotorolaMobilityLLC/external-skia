// Copyright 2019 Google LLC.
#ifndef ParagraphImpl_DEFINED
#define ParagraphImpl_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "include/private/SkTemplates.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextShadow.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Run.h"
#include "src/core/SkSpan.h"

#include <unicode/ubrk.h>
#include <memory>
#include <string>
#include <vector>

class SkCanvas;

namespace skia {
namespace textlayout {

enum CodeUnitFlags {
    kNoCodeUnitFlag = 0x0,
    kPartOfWhiteSpace = 0x1,
    kGraphemeBreakBefore = 0x2,
    kSoftLineBreakBefore = 0x4,
    kHardLineBreakBefore = 0x8,
};
}
}

namespace sknonstd {
template <> struct is_bitmask_enum<skia::textlayout::CodeUnitFlags> : std::true_type {};
}

namespace skia {
namespace textlayout {

class LineMetrics;
class TextLine;

template <typename T> bool operator==(const SkSpan<T>& a, const SkSpan<T>& b) {
    return a.size() == b.size() && a.begin() == b.begin();
}

template <typename T> bool operator<=(const SkSpan<T>& a, const SkSpan<T>& b) {
    return a.begin() >= b.begin() && a.end() <= b.end();
}

template <typename TStyle>
struct StyleBlock {
    StyleBlock() : fRange(EMPTY_RANGE), fStyle() { }
    StyleBlock(size_t start, size_t end, const TStyle& style) : fRange(start, end), fStyle(style) {}
    StyleBlock(TextRange textRange, const TStyle& style) : fRange(textRange), fStyle(style) {}
    void add(TextRange tail) {
        SkASSERT(fRange.end == tail.start);
        fRange = TextRange(fRange.start, fRange.start + fRange.width() + tail.width());
    }
    TextRange fRange;
    TStyle fStyle;
};

struct ResolvedFontDescriptor {

    ResolvedFontDescriptor(TextIndex index, SkFont font)
        : fFont(font), fTextStart(index) { }
    SkFont fFont;
    TextIndex fTextStart;
};

struct BidiRegion {
    BidiRegion(size_t start, size_t end, uint8_t dir)
        : text(start, end), direction(dir) { }
    TextRange text;
    uint8_t direction;
};

class ParagraphImpl final : public Paragraph {

public:

    ParagraphImpl(const SkString& text,
                  ParagraphStyle style,
                  SkTArray<Block, true> blocks,
                  SkTArray<Placeholder, true> placeholders,
                  sk_sp<FontCollection> fonts);

    ParagraphImpl(const std::u16string& utf16text,
                  ParagraphStyle style,
                  SkTArray<Block, true> blocks,
                  SkTArray<Placeholder, true> placeholders,
                  sk_sp<FontCollection> fonts);
    ~ParagraphImpl() override;

    void layout(SkScalar width) override;
    void paint(SkCanvas* canvas, SkScalar x, SkScalar y) override;
    std::vector<TextBox> getRectsForRange(unsigned start,
                                          unsigned end,
                                          RectHeightStyle rectHeightStyle,
                                          RectWidthStyle rectWidthStyle) override;
    std::vector<TextBox> getRectsForPlaceholders() override;
    void getLineMetrics(std::vector<LineMetrics>&) override;
    PositionWithAffinity getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy) override;
    SkRange<size_t> getWordBoundary(unsigned offset) override;

    size_t lineNumber() override { return fLines.size(); }

    TextLine& addLine(SkVector offset, SkVector advance, TextRange text, TextRange textWithSpaces,
                      ClusterRange clusters, ClusterRange clustersWithGhosts, SkScalar widthWithSpaces,
                      InternalLineMetrics sizes);

    SkSpan<const char> text() const { return SkSpan<const char>(fText.c_str(), fText.size()); }
    InternalState state() const { return fState; }
    SkSpan<Run> runs() { return SkSpan<Run>(fRuns.data(), fRuns.size()); }
    SkSpan<Block> styles() {
        return SkSpan<Block>(fTextStyles.data(), fTextStyles.size());
    }
    SkSpan<Placeholder> placeholders() {
        return SkSpan<Placeholder>(fPlaceholders.data(), fPlaceholders.size());
    }
    SkSpan<TextLine> lines() { return SkSpan<TextLine>(fLines.data(), fLines.size()); }
    const ParagraphStyle& paragraphStyle() const { return fParagraphStyle; }
    SkSpan<Cluster> clusters() { return SkSpan<Cluster>(fClusters.begin(), fClusters.size()); }
    sk_sp<FontCollection> fontCollection() const { return fFontCollection; }
    SkSpan<CodepointRepresentation> codepoints(){ return SkSpan<CodepointRepresentation>(fCodepoints.begin(), fCodepoints.size()); }
    void formatLines(SkScalar maxWidth);

    bool strutEnabled() const { return paragraphStyle().getStrutStyle().getStrutEnabled(); }
    bool strutForceHeight() const {
        return paragraphStyle().getStrutStyle().getForceStrutHeight();
    }
    bool strutHeightOverride() const {
        return paragraphStyle().getStrutStyle().getHeightOverride();
    }
    InternalLineMetrics strutMetrics() const { return fStrutMetrics; }

    SkSpan<const char> text(TextRange textRange);
    SkSpan<Cluster> clusters(ClusterRange clusterRange);
    Cluster& cluster(ClusterIndex clusterIndex);
    ClusterIndex clusterIndex(TextIndex textIndex) {
        auto clusterIndex = this->fClustersIndexFromCodeUnit[textIndex];
        SkASSERT(clusterIndex != EMPTY_INDEX);
        return clusterIndex;
    }
    Run& run(RunIndex runIndex) {
        SkASSERT(runIndex < fRuns.size());
        return fRuns[runIndex];
    }

    Run& runByCluster(ClusterIndex clusterIndex);
    SkSpan<Block> blocks(BlockRange blockRange);
    Block& block(BlockIndex blockIndex);
    SkTArray<ResolvedFontDescriptor> resolvedFonts() const { return fFontSwitches; }

    void markDirty() override { fState = kUnknown; }

    int32_t unresolvedGlyphs() override;

    void setState(InternalState state);
    sk_sp<SkPicture> getPicture() { return fPicture; }
    SkRect getBoundaries() const { return fOrigin; }

    SkScalar widthWithTrailingSpaces() { return fMaxWidthWithTrailingSpaces; }

    void resetContext();
    void resolveStrut();

    bool computeCodeUnitProperties();
    bool computeWords();
    bool getBidiRegions();

    void buildClusterTable();
    void spaceGlyphs();
    bool shapeTextIntoEndlessLine();
    void breakShapedTextIntoLines(SkScalar maxWidth);
    void paintLinesIntoPicture();

    void updateTextAlign(TextAlign textAlign) override;
    void updateText(size_t from, SkString text) override;
    void updateFontSize(size_t from, size_t to, SkScalar fontSize) override;
    void updateForegroundPaint(size_t from, size_t to, SkPaint paint) override;
    void updateBackgroundPaint(size_t from, size_t to, SkPaint paint) override;

    InternalLineMetrics getEmptyMetrics() const { return fEmptyMetrics; }
    InternalLineMetrics getStrutMetrics() const { return fStrutMetrics; }

    BlockRange findAllBlocks(TextRange textRange);

    void resetShifts() {
        for (auto& run : fRuns) {
            run.resetJustificationShifts();
            run.resetShifts();
        }
    }

    using CodeUnitRangeVisitor = std::function<bool(TextRange textRange)>;
    void forEachCodeUnitPropertyRange(CodeUnitFlags property, CodeUnitRangeVisitor visitor);
    size_t getWhitespacesLength(TextRange textRange);

    bool codeUnitHasProperty(size_t index, CodeUnitFlags property) const { return (fCodeUnitProperties[index] & property) == property; }

private:
    friend class ParagraphBuilder;
    friend class ParagraphCacheKey;
    friend class ParagraphCacheValue;
    friend class ParagraphCache;

    friend class TextWrapper;
    friend class OneLineShaper;

    void calculateBoundaries();

    void markGraphemes16();

    void computeEmptyMetrics();

    // Input
    SkTArray<StyleBlock<SkScalar>> fLetterSpaceStyles;
    SkTArray<StyleBlock<SkScalar>> fWordSpaceStyles;
    SkTArray<StyleBlock<SkPaint>> fBackgroundStyles;
    SkTArray<StyleBlock<SkPaint>> fForegroundStyles;
    SkTArray<StyleBlock<std::vector<TextShadow>>> fShadowStyles;
    SkTArray<StyleBlock<Decoration>> fDecorationStyles;
    SkTArray<Block, true> fTextStyles; // TODO: take out only the font stuff
    SkTArray<Placeholder, true> fPlaceholders;
    SkString fText;

    // Internal structures
    InternalState fState;
    SkTArray<Run, false> fRuns;         // kShaped
    SkTArray<Cluster, true> fClusters;  // kClusterized (cached: text, word spacing, letter spacing, resolved fonts)
    SkTArray<CodeUnitFlags> fCodeUnitProperties;
    SkTArray<size_t> fClustersIndexFromCodeUnit;
    std::vector<size_t> fWords;
    SkTArray<BidiRegion> fBidiRegions;
    SkTArray<Grapheme, true> fGraphemes16;
    SkTArray<CodepointRepresentation, true> fCodepoints;
    size_t fUnresolvedGlyphs;

    SkTArray<TextLine, false> fLines;   // kFormatted   (cached: width, max lines, ellipsis, text align)
    sk_sp<SkPicture> fPicture;          // kRecorded    (cached: text styles)

    SkTArray<ResolvedFontDescriptor> fFontSwitches;

    InternalLineMetrics fEmptyMetrics;
    InternalLineMetrics fStrutMetrics;

    SkScalar fOldWidth;
    SkScalar fOldHeight;
    SkScalar fMaxWidthWithTrailingSpaces;
    SkRect fOrigin;
};
}  // namespace textlayout
}  // namespace skia


#endif  // ParagraphImpl_DEFINED
