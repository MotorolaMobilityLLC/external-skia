/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Running create_test_font generates ./tools/fonts/test_font_index.inc
// and ./tools/fonts/test_font_<generic name>.inc which are read by
// ./tools/fonts/SkTestFontMgr.cpp

#include "SkFontStyle.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkSpan.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTSort.h"
#include "SkTypeface.h"
#include "SkUTF.h"

#include <stdio.h>

namespace {

struct NamedFontStyle {
    char const * const fName;
    char const * const fIdentifierName;
    SkFontStyle const fStyle;
};

struct FontDesc {
    NamedFontStyle const fNamedStyle;
    char const * const fFile;
};

struct FontFamilyDesc {
    char const * const fGenericName;
    char const * const fFamilyName;
    char const * const fIdentifierName;
    SkSpan<const FontDesc> const fFonts;
};

} // namespace

static FILE* font_header(const char* family) {
    SkString outPath(SkOSPath::Join(".", "tools"));
    outPath = SkOSPath::Join(outPath.c_str(), "fonts");
    outPath = SkOSPath::Join(outPath.c_str(), "test_font_");
    SkString fam(family);
    do {
        int dashIndex = fam.find("-");
        if (dashIndex < 0) {
            break;
        }
        fam.writable_str()[dashIndex] = '_';
    } while (true);
    outPath.append(fam);
    outPath.append(".inc");
    FILE* out = fopen(outPath.c_str(), "w");

    static const char kHeader[] =
        "/*\n"
        " * Copyright 2015 Google Inc.\n"
        " *\n"
        " * Use of this source code is governed by a BSD-style license that can be\n"
        " * found in the LICENSE file.\n"
        " */\n"
        "\n"
        "// Auto-generated by ";
    fprintf(out, "%s%s\n\n", kHeader, SkOSPath::Basename(__FILE__).c_str());
    return out;
}

enum {
    kMaxLineLength = 80,
};

static ptrdiff_t last_line_length(const SkString& str) {
    const char* first = str.c_str();
    const char* last = first + str.size();
    const char* ptr = last;
    while (ptr > first && *--ptr != '\n')
        ;
    return last - ptr - 1;
}

static void output_fixed(SkScalar num, int emSize, SkString* out) {
    int hex = (int) (num * 65536 / emSize);
    out->appendf("0x%08x,", hex);
    *out += (int) last_line_length(*out) >= kMaxLineLength ? '\n' : ' ';
}

static void output_scalar(SkScalar num, int emSize, SkString* out) {
    num /= emSize;
    if (num == (int) num) {
       out->appendS32((int) num);
    } else {
        SkString str;
        str.printf("%1.6g", num);
        int width = (int) str.size();
        const char* cStr = str.c_str();
        while (cStr[width - 1] == '0') {
            --width;
        }
        str.remove(width, str.size() - width);
        out->appendf("%sf", str.c_str());
    }
    *out += ',';
    *out += (int) last_line_length(*out) >= kMaxLineLength ? '\n' : ' ';
}

static int output_points(const SkPoint* pts, int emSize, int count, SkString* ptsOut) {
    for (int index = 0; index < count; ++index) {
        output_scalar(pts[index].fX, emSize, ptsOut);
        output_scalar(pts[index].fY, emSize, ptsOut);
    }
    return count;
}

static void output_path_data(const SkPaint& paint,
        int emSize, SkString* ptsOut, SkTDArray<SkPath::Verb>* verbs,
        SkTDArray<unsigned>* charCodes, SkTDArray<SkScalar>* widths) {
    for (SkUnichar index = 0x00; index < 0x7f; ++index) {
        uint16_t utf16[2];
        size_t utf16Bytes = sizeof(uint16_t) * SkUTF::ToUTF16(index, utf16);
        SkPath path;
        SkASSERT(paint.getTextEncoding() == SkPaint::kUTF16_TextEncoding);
        paint.getTextPath(utf16, utf16Bytes, 0, 0, &path);
        SkPath::RawIter iter(path);
        SkPath::Verb verb;
        SkPoint pts[4];
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            *verbs->append() = verb;
            switch (verb) {
                case SkPath::kMove_Verb:
                    output_points(&pts[0], emSize, 1, ptsOut);
                    break;
                case SkPath::kLine_Verb:
                    output_points(&pts[1], emSize, 1, ptsOut);
                    break;
                case SkPath::kQuad_Verb:
                    output_points(&pts[1], emSize, 2, ptsOut);
                    break;
                case SkPath::kCubic_Verb:
                    output_points(&pts[1], emSize, 3, ptsOut);
                    break;
                case SkPath::kClose_Verb:
                    break;
                default:
                    SkDEBUGFAIL("bad verb");
                    SkASSERT(0);
            }
        }
        *verbs->append() = SkPath::kDone_Verb;
        *charCodes->append() = index;
        SkScalar width;
        SkDEBUGCODE(int charCount =) paint.getTextWidths(utf16, utf16Bytes, &width);
        SkASSERT(charCount == 1);
     // SkASSERT(floor(width) == width);  // not true for Hiragino Maru Gothic Pro
        *widths->append() = width;
        if (0 == index) {
            index = 0x1f;  // skip the rest of the control codes
        }
    }
}

static int offset_str_len(unsigned num) {
    if (num == (unsigned) -1) {
        return 10;
    }
    unsigned result = 1;
    unsigned ref = 10;
    while (ref <= num) {
        ++result;
        ref *= 10;
    }
    return result;
}

static SkString strip_final(const SkString& str) {
    SkString result(str);
    if (result.endsWith("\n")) {
        result.remove(result.size() - 1, 1);
    }
    if (result.endsWith(" ")) {
        result.remove(result.size() - 1, 1);
    }
    if (result.endsWith(",")) {
        result.remove(result.size() - 1, 1);
    }
    return result;
}

static void output_font(sk_sp<SkTypeface> face, const char* identifier, FILE* out) {
    int emSize = face->getUnitsPerEm() * 2;
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    paint.setTextSize(emSize);
    paint.setTypeface(std::move(face));
    SkTDArray<SkPath::Verb> verbs;
    SkTDArray<unsigned> charCodes;
    SkTDArray<SkScalar> widths;
    SkString ptsOut;
    output_path_data(paint, emSize, &ptsOut, &verbs, &charCodes, &widths);
    fprintf(out, "const SkScalar %sPoints[] = {\n", identifier);
    ptsOut = strip_final(ptsOut);
    fprintf(out, "%s", ptsOut.c_str());
    fprintf(out, "\n};\n\n");
    fprintf(out, "const unsigned char %sVerbs[] = {\n", identifier);
    int verbCount = verbs.count();
    int outChCount = 0;
    for (int index = 0; index < verbCount;) {
        SkPath::Verb verb = verbs[index];
        SkASSERT(verb >= SkPath::kMove_Verb && verb <= SkPath::kDone_Verb);
        SkASSERT(SkTFitsIn<uint8_t>(verb));
        fprintf(out, "%u", verb);
        if (++index < verbCount) {
            outChCount += 3;
            fprintf(out, "%c", ',');
            if (outChCount >= kMaxLineLength) {
                outChCount = 0;
                fprintf(out, "%c", '\n');
            } else {
                fprintf(out, "%c", ' ');
            }
        }
    }
    fprintf(out, "\n};\n\n");

    // all fonts are now 0x00, 0x20 - 0xFE
    // don't need to generate or output character codes?
    fprintf(out, "const SkUnichar %sCharCodes[] = {\n", identifier);
    int offsetCount = charCodes.count();
    for (int index = 0; index < offsetCount;) {
        unsigned offset = charCodes[index];
        fprintf(out, "%u", offset);
        if (++index < offsetCount) {
            outChCount += offset_str_len(offset) + 2;
            fprintf(out, "%c", ',');
            if (outChCount >= kMaxLineLength) {
                outChCount = 0;
                fprintf(out, "%c", '\n');
            } else {
                fprintf(out, "%c", ' ');
            }
        }
    }
    fprintf(out, "\n};\n\n");

    SkString widthsStr;
    fprintf(out, "const SkFixed %sWidths[] = {\n", identifier);
    for (int index = 0; index < offsetCount; ++index) {
        output_fixed(widths[index], emSize, &widthsStr);
    }
    widthsStr = strip_final(widthsStr);
    fprintf(out, "%s\n};\n\n", widthsStr.c_str());

    fprintf(out, "const size_t %sCharCodesCount = SK_ARRAY_COUNT(%sCharCodes);\n\n",
            identifier, identifier);

    SkFontMetrics metrics;
    paint.getFontMetrics(&metrics);
    fprintf(out, "const SkFontMetrics %sMetrics = {\n", identifier);
    SkString metricsStr;
    metricsStr.printf("0x%08x, ", metrics.fFlags);
    output_scalar(metrics.fTop, emSize, &metricsStr);
    output_scalar(metrics.fAscent, emSize, &metricsStr);
    output_scalar(metrics.fDescent, emSize, &metricsStr);
    output_scalar(metrics.fBottom, emSize, &metricsStr);
    output_scalar(metrics.fLeading, emSize, &metricsStr);
    output_scalar(metrics.fAvgCharWidth, emSize, &metricsStr);
    output_scalar(metrics.fMaxCharWidth, emSize, &metricsStr);
    output_scalar(metrics.fXMin, emSize, &metricsStr);
    output_scalar(metrics.fXMax, emSize, &metricsStr);
    output_scalar(metrics.fXHeight, emSize, &metricsStr);
    output_scalar(metrics.fCapHeight, emSize, &metricsStr);
    output_scalar(metrics.fUnderlineThickness, emSize, &metricsStr);
    output_scalar(metrics.fUnderlinePosition, emSize, &metricsStr);
    output_scalar(metrics.fStrikeoutThickness, emSize, &metricsStr);
    output_scalar(metrics.fStrikeoutPosition, emSize, &metricsStr);
    metricsStr = strip_final(metricsStr);
    fprintf(out, "%s\n};\n\n", metricsStr.c_str());
}

static SkString identifier(const FontFamilyDesc& family, const FontDesc& font) {
    SkString id(family.fIdentifierName);
    id.append(font.fNamedStyle.fIdentifierName);
    return id;
}

static void generate_fonts(const char* basepath, const SkSpan<const FontFamilyDesc>& families) {
    FILE* out = nullptr;
    for (const FontFamilyDesc& family : families) {
        out = font_header(family.fGenericName);
        for (const FontDesc& font : family.fFonts) {
            SkString filepath(SkOSPath::Join(basepath, font.fFile));
            SkASSERTF(sk_exists(filepath.c_str()), "The file %s does not exist.", filepath.c_str());
            sk_sp<SkTypeface> resourceTypeface = SkTypeface::MakeFromFile(filepath.c_str());
            SkASSERTF(resourceTypeface, "The file %s is not a font.", filepath.c_str());
            output_font(std::move(resourceTypeface), identifier(family, font).c_str(), out);
        }
        fclose(out);
    }
}

static const char* slant_to_string(SkFontStyle::Slant slant) {
    switch (slant) {
        case SkFontStyle::kUpright_Slant: return "SkFontStyle::kUpright_Slant";
        case SkFontStyle::kItalic_Slant : return "SkFontStyle::kItalic_Slant" ;
        case SkFontStyle::kOblique_Slant: return "SkFontStyle::kOblique_Slant";
        default: SK_ABORT("Unknown slant"); return "";
    }
}

static void generate_index(const SkSpan<const FontFamilyDesc>& families,
                           const FontDesc* defaultFont)
{
    FILE* out = font_header("index");
    fprintf(out, "static SkTestFontData gTestFonts[] = {\n");
    for (const FontFamilyDesc& family : families) {
        for (const FontDesc& font : family.fFonts) {
            SkString identifierStr = identifier(family, font);
            const char* identifier = identifierStr.c_str();
            const SkFontStyle& style = font.fNamedStyle.fStyle;
            fprintf(out,
                    "    {    %sPoints, %sVerbs,\n"
                    "         %sCharCodes, %sCharCodesCount, %sWidths,\n"
                    "         %sMetrics, \"Toy %s\", SkFontStyle(%d,%d,%s)\n"
                    "    },\n",
                    identifier, identifier,
                    identifier, identifier, identifier,
                    identifier, family.fFamilyName,
                    style.weight(), style.width(), slant_to_string(style.slant()));
        }
    }
    fprintf(out, "};\n\n");
    fprintf(out,
            "struct SubFont {\n"
            "    const char* fFamilyName;\n"
            "    const char* fStyleName;\n"
            "    SkFontStyle fStyle;\n"
            "    SkTestFontData& fFont;\n"
            "    const char* fFile;\n"
            "};\n\n"
            "const SubFont gSubFonts[] = {\n");
    int defaultIndex = -1;
    int testFontsIndex = 0;
    for (const FontFamilyDesc& family : families) {
        for (const FontDesc& font : family.fFonts) {
            if (&font == defaultFont) {
                defaultIndex = testFontsIndex;
            }
            const SkFontStyle& style = font.fNamedStyle.fStyle;
            fprintf(out,
                    "    { \"%s\", \"%s\", SkFontStyle(%d,%d,%s), gTestFonts[%d], \"%s\" },\n",
                    family.fGenericName, font.fNamedStyle.fName,
                    style.weight(), style.width(), slant_to_string(style.slant()),
                    testFontsIndex, font.fFile);
            testFontsIndex++;
        }
    }
    testFontsIndex = 0;
    for (const FontFamilyDesc& family : families) {
        for (const FontDesc& font : family.fFonts) {
            fprintf(out,
                    "    { \"Toy %s\", \"%s\", SkFontStyle(%d,%d,%s), gTestFonts[%d], \"%s\" },\n",
                    family.fFamilyName, font.fNamedStyle.fName,
                    font.fNamedStyle.fStyle.weight(), font.fNamedStyle.fStyle.width(),
                    slant_to_string(font.fNamedStyle.fStyle.slant()), testFontsIndex, font.fFile);
            testFontsIndex++;
        }
    }
    fprintf(out, "};\n\n");
    SkASSERT(defaultIndex >= 0);
    fprintf(out, "const size_t gDefaultFontIndex = %d;\n", defaultIndex);
    fclose(out);
}

int main(int , char * const []) {
    constexpr NamedFontStyle normal     = {"Normal",      "Normal",     SkFontStyle::Normal()    };
    constexpr NamedFontStyle bold       = {"Bold",        "Bold",       SkFontStyle::Bold()      };
    constexpr NamedFontStyle italic     = {"Italic",      "Italic",     SkFontStyle::Italic()    };
    constexpr NamedFontStyle bolditalic = {"Bold Italic", "BoldItalic", SkFontStyle::BoldItalic()};

    static constexpr FontDesc kMonoFonts[] = {
        {normal,     "LiberationMono-Regular.ttf"},
        {bold,       "LiberationMono-Bold.ttf"},
        {italic,     "LiberationMono-Italic.ttf"},
        {bolditalic, "LiberationMono-BoldItalic.ttf"},
    };

    static constexpr FontDesc kSansFonts[] = {
        {normal,     "LiberationSans-Regular.ttf"},
        {bold,       "LiberationSans-Bold.ttf"},
        {italic,     "LiberationSans-Italic.ttf"},
        {bolditalic, "LiberationSans-BoldItalic.ttf"},
    };

    static constexpr FontDesc kSerifFonts[] = {
        {normal,     "LiberationSerif-Regular.ttf"},
        {bold,       "LiberationSerif-Bold.ttf"},
        {italic,     "LiberationSerif-Italic.ttf"},
        {bolditalic, "LiberationSerif-BoldItalic.ttf"},
    };

    static constexpr FontFamilyDesc kFamiliesData[] = {
        {"monospace",  "Liberation Mono",  "LiberationMono",  { kMonoFonts  }},
        {"sans-serif", "Liberation Sans",  "LiberationSans",  { kSansFonts  }},
        {"serif",      "Liberation Serif", "LiberationSerif", { kSerifFonts }},
    };

    static constexpr SkSpan<const FontFamilyDesc> kFamilies(kFamiliesData);

#ifdef SK_BUILD_FOR_UNIX
    generate_fonts("/usr/share/fonts/truetype/liberation/", kFamilies);
#else
    generate_fonts("/Library/Fonts/", kFamilies);
#endif
    generate_index(kFamilies, &kFamilies[1].fFonts[0]);
    return 0;
}
