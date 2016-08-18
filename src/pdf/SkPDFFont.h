/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFFont_DEFINED
#define SkPDFFont_DEFINED

#include "SkAdvancedTypefaceMetrics.h"
#include "SkBitSet.h"
#include "SkPDFTypes.h"
#include "SkTDArray.h"
#include "SkTypeface.h"

class SkPDFCanon;
class SkPDFFont;

class SkPDFGlyphSet : SkNoncopyable {
public:
    SkPDFGlyphSet();
    SkPDFGlyphSet(SkPDFGlyphSet&& o) : fBitSet(std::move(o.fBitSet)) {}

    void set(const uint16_t* glyphIDs, int numGlyphs);
    bool has(uint16_t glyphID) const;
    void exportTo(SkTDArray<uint32_t>* glyphIDs) const;
    const SkBitSet& bitSet() const { return fBitSet; }

private:
    SkBitSet fBitSet;
};

class SkPDFGlyphSetMap : SkNoncopyable {
public:
    struct FontGlyphSetPair : SkNoncopyable {
        FontGlyphSetPair() : fFont(nullptr) {}
        FontGlyphSetPair(FontGlyphSetPair&& o)
            : fFont(o.fFont)
            , fGlyphSet(std::move(o.fGlyphSet)) {
            o.fFont = nullptr;
        }
        SkPDFFont* fFont;
        SkPDFGlyphSet fGlyphSet;
    };

    SkPDFGlyphSetMap();
    ~SkPDFGlyphSetMap();

    const FontGlyphSetPair* begin() const { return fMap.begin(); }
    const FontGlyphSetPair* end() const { return fMap.end(); }

    void noteGlyphUsage(SkPDFFont* font, const uint16_t* glyphIDs,
                        int numGlyphs);

private:
    SkPDFGlyphSet* getGlyphSetForFont(SkPDFFont* font);

    SkTArray<FontGlyphSetPair> fMap;
};

/** \class SkPDFFont
    A PDF Object class representing a font.  The font may have resources
    attached to it in order to embed the font.  SkPDFFonts are canonicalized
    so that resource deduplication will only include one copy of a font.
    This class uses the same pattern as SkPDFGraphicState, a static weak
    reference to each instantiated class.
*/
class SkPDFFont : public SkPDFDict {

public:
    virtual ~SkPDFFont();

    /** Returns the typeface represented by this class. Returns nullptr for the
     *  default typeface.
     */
    SkTypeface* typeface() const { return fTypeface.get(); }

    /** Returns the font type represented in this font.  For Type0 fonts,
     *  returns the type of the decendant font.
     */
    SkAdvancedTypefaceMetrics::FontType getType() const { return fFontType; }

    static bool IsMultiByte(SkAdvancedTypefaceMetrics::FontType type) {
        return type == SkAdvancedTypefaceMetrics::kType1CID_Font ||
               type == SkAdvancedTypefaceMetrics::kTrueType_Font;
    }

    /** Returns true if this font encoding supports glyph IDs above 255.
     */
    bool multiByteGlyphs() const { return SkPDFFont::IsMultiByte(this->getType()); }

    /** Return true if this font has an encoding for the passed glyph id.
     */
    bool hasGlyph(SkGlyphID gid) {
        return (gid >= fFirstGlyphID && gid <= fLastGlyphID) || gid == 0;
    }

    /** Convert (in place) the input glyph IDs into the font encoding.  If the
     *  font has more glyphs than can be encoded (like a type 1 font with more
     *  than 255 glyphs) this method only converts up to the first out of range
     *  glyph ID.
     *  @param glyphIDs       The input text as glyph IDs.
     *  @param numGlyphs      The number of input glyphs.
     *  @return               Returns the number of glyphs consumed.
     */
    int glyphsToPDFFontEncoding(SkGlyphID* glyphIDs, int numGlyphs) const;
    /**
     * Like above, but does not modify glyphIDs array.
     */
    int glyphsToPDFFontEncodingCount(const SkGlyphID* glyphIDs,
                                     int numGlyphs) const;

    /** Get the font resource for the passed typeface and glyphID. The
     *  reference count of the object is incremented and it is the caller's
     *  responsibility to unreference it when done.  This is needed to
     *  accommodate the weak reference pattern used when the returned object
     *  is new and has no other references.
     *  @param typeface  The typeface to find.
     *  @param glyphID   Specify which section of a large font is of interest.
     */
    static SkPDFFont* GetFontResource(SkPDFCanon* canon,
                                      SkTypeface* typeface,
                                      SkGlyphID glyphID);

    // Uses (kGlyphNames_PerGlyphInfo | kToUnicode_PerGlyphInfo).
    static const SkAdvancedTypefaceMetrics* GetMetrics(SkTypeface* typeface,
                                                       SkPDFCanon* canon);

    /** Subset the font based on usage set. Returns a SkPDFFont instance with
     *  subset.
     *  @param usage  Glyph subset requested.
     *  @return       nullptr if font does not support subsetting, a new instance
     *                of SkPDFFont otherwise.
     */
    virtual sk_sp<SkPDFObject> getFontSubset(SkPDFCanon* canon,
                                             const SkPDFGlyphSet* usage);

    /**
     *  Return false iff the typeface has its NotEmbeddable flag set.
     *  If typeface is NULL, the default typeface is checked.
     */
    static bool CanEmbedTypeface(SkTypeface*, SkPDFCanon*);

protected:
    // Common constructor to handle common members.
    SkPDFFont(sk_sp<SkTypeface> typeface,
              SkAdvancedTypefaceMetrics::FontType fontType);

    SkGlyphID firstGlyphID() const { return fFirstGlyphID; }
    SkGlyphID lastGlyphID() const { return fLastGlyphID; }

    sk_sp<SkTypeface> refTypeface() const { return fTypeface; }

    /** Set fFirstGlyphID and fLastGlyphID to span at most 255 glyphs,
     *  including the passed glyphID.
     */
    void adjustGlyphRangeForSingleByteEncoding(SkGlyphID glyphID);

    void drop() override;

private:
    sk_sp<SkTypeface> fTypeface;

    // The glyph IDs accessible with this font.  For Type1 (non CID) fonts,
    // this will be a subset if the font has more than 255 glyphs.
    SkGlyphID fFirstGlyphID;
    SkGlyphID fLastGlyphID;
    SkAdvancedTypefaceMetrics::FontType fFontType;

    typedef SkPDFDict INHERITED;
};

#endif
