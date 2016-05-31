/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Auto-generated by create_test_font.cpp

static SkTestFontData gTestFonts[] = {
    {    LiberationMonoNormalPoints, LiberationMonoNormalVerbs, LiberationMonoNormalCharCodes,
         LiberationMonoNormalCharCodesCount, LiberationMonoNormalWidths,
         LiberationMonoNormalMetrics, "Toy Liberation Mono", SkTypeface::kNormal, nullptr
    },
    {    LiberationMonoBoldPoints, LiberationMonoBoldVerbs, LiberationMonoBoldCharCodes,
         LiberationMonoBoldCharCodesCount, LiberationMonoBoldWidths,
         LiberationMonoBoldMetrics, "Toy Liberation Mono", SkTypeface::kBold, nullptr
    },
    {    LiberationMonoItalicPoints, LiberationMonoItalicVerbs, LiberationMonoItalicCharCodes,
         LiberationMonoItalicCharCodesCount, LiberationMonoItalicWidths,
         LiberationMonoItalicMetrics, "Toy Liberation Mono", SkTypeface::kItalic, nullptr
    },
    {    LiberationMonoBoldItalicPoints, LiberationMonoBoldItalicVerbs, LiberationMonoBoldItalicCharCodes,
         LiberationMonoBoldItalicCharCodesCount, LiberationMonoBoldItalicWidths,
         LiberationMonoBoldItalicMetrics, "Toy Liberation Mono", SkTypeface::kBoldItalic, nullptr
    },
    {    LiberationSansNormalPoints, LiberationSansNormalVerbs, LiberationSansNormalCharCodes,
         LiberationSansNormalCharCodesCount, LiberationSansNormalWidths,
         LiberationSansNormalMetrics, "Toy Liberation Sans", SkTypeface::kNormal, nullptr
    },
    {    LiberationSansBoldPoints, LiberationSansBoldVerbs, LiberationSansBoldCharCodes,
         LiberationSansBoldCharCodesCount, LiberationSansBoldWidths,
         LiberationSansBoldMetrics, "Toy Liberation Sans", SkTypeface::kBold, nullptr
    },
    {    LiberationSansItalicPoints, LiberationSansItalicVerbs, LiberationSansItalicCharCodes,
         LiberationSansItalicCharCodesCount, LiberationSansItalicWidths,
         LiberationSansItalicMetrics, "Toy Liberation Sans", SkTypeface::kItalic, nullptr
    },
    {    LiberationSansBoldItalicPoints, LiberationSansBoldItalicVerbs, LiberationSansBoldItalicCharCodes,
         LiberationSansBoldItalicCharCodesCount, LiberationSansBoldItalicWidths,
         LiberationSansBoldItalicMetrics, "Toy Liberation Sans", SkTypeface::kBoldItalic, nullptr
    },
    {    LiberationSerifNormalPoints, LiberationSerifNormalVerbs, LiberationSerifNormalCharCodes,
         LiberationSerifNormalCharCodesCount, LiberationSerifNormalWidths,
         LiberationSerifNormalMetrics, "Toy Liberation Serif", SkTypeface::kNormal, nullptr
    },
    {    LiberationSerifBoldPoints, LiberationSerifBoldVerbs, LiberationSerifBoldCharCodes,
         LiberationSerifBoldCharCodesCount, LiberationSerifBoldWidths,
         LiberationSerifBoldMetrics, "Toy Liberation Serif", SkTypeface::kBold, nullptr
    },
    {    LiberationSerifItalicPoints, LiberationSerifItalicVerbs, LiberationSerifItalicCharCodes,
         LiberationSerifItalicCharCodesCount, LiberationSerifItalicWidths,
         LiberationSerifItalicMetrics, "Toy Liberation Serif", SkTypeface::kItalic, nullptr
    },
    {    LiberationSerifBoldItalicPoints, LiberationSerifBoldItalicVerbs, LiberationSerifBoldItalicCharCodes,
         LiberationSerifBoldItalicCharCodesCount, LiberationSerifBoldItalicWidths,
         LiberationSerifBoldItalicMetrics, "Toy Liberation Serif", SkTypeface::kBoldItalic, nullptr
    },
};

const int gTestFontsCount = (int) SK_ARRAY_COUNT(gTestFonts);

struct SubFont {
    const char* fName;
    SkFontStyle fStyle;
    SkTestFontData& fFont;
    const char* fFile;
};

const SubFont gSubFonts[] = {
    { "monospace", SkFontStyle(), gTestFonts[0], "LiberationMono-Regular.ttf" },
    { "monospace", SkFontStyle::FromOldStyle(SkTypeface::kBold), gTestFonts[1], "LiberationMono-Bold.ttf" },
    { "monospace", SkFontStyle::FromOldStyle(SkTypeface::kItalic), gTestFonts[2], "LiberationMono-Italic.ttf" },
    { "monospace", SkFontStyle::FromOldStyle(SkTypeface::kBoldItalic), gTestFonts[3], "LiberationMono-BoldItalic.ttf" },
    { "sans-serif", SkFontStyle(), gTestFonts[4], "LiberationSans-Regular.ttf" },
    { "sans-serif", SkFontStyle::FromOldStyle(SkTypeface::kBold), gTestFonts[5], "LiberationSans-Bold.ttf" },
    { "sans-serif", SkFontStyle::FromOldStyle(SkTypeface::kItalic), gTestFonts[6], "LiberationSans-Italic.ttf" },
    { "sans-serif", SkFontStyle::FromOldStyle(SkTypeface::kBoldItalic), gTestFonts[7], "LiberationSans-BoldItalic.ttf" },
    { "serif", SkFontStyle(), gTestFonts[8], "LiberationSerif-Regular.ttf" },
    { "serif", SkFontStyle::FromOldStyle(SkTypeface::kBold), gTestFonts[9], "LiberationSerif-Bold.ttf" },
    { "serif", SkFontStyle::FromOldStyle(SkTypeface::kItalic), gTestFonts[10], "LiberationSerif-Italic.ttf" },
    { "serif", SkFontStyle::FromOldStyle(SkTypeface::kBoldItalic), gTestFonts[11], "LiberationSerif-BoldItalic.ttf" },
    { "Toy Liberation Mono", SkFontStyle(), gTestFonts[0], "LiberationMono-Regular.ttf" },
    { "Toy Liberation Mono", SkFontStyle::FromOldStyle(SkTypeface::kBold), gTestFonts[1], "LiberationMono-Bold.ttf" },
    { "Toy Liberation Mono", SkFontStyle::FromOldStyle(SkTypeface::kItalic), gTestFonts[2], "LiberationMono-Italic.ttf" },
    { "Toy Liberation Mono", SkFontStyle::FromOldStyle(SkTypeface::kBoldItalic), gTestFonts[3], "LiberationMono-BoldItalic.ttf" },
    { "Toy Liberation Sans", SkFontStyle(), gTestFonts[4], "LiberationSans-Regular.ttf" },
    { "Toy Liberation Sans", SkFontStyle::FromOldStyle(SkTypeface::kBold), gTestFonts[5], "LiberationSans-Bold.ttf" },
    { "Toy Liberation Sans", SkFontStyle::FromOldStyle(SkTypeface::kItalic), gTestFonts[6], "LiberationSans-Italic.ttf" },
    { "Toy Liberation Sans", SkFontStyle::FromOldStyle(SkTypeface::kBoldItalic), gTestFonts[7], "LiberationSans-BoldItalic.ttf" },
    { "Toy Liberation Serif", SkFontStyle(), gTestFonts[8], "LiberationSerif-Regular.ttf" },
    { "Toy Liberation Serif", SkFontStyle::FromOldStyle(SkTypeface::kBold), gTestFonts[9], "LiberationSerif-Bold.ttf" },
    { "Toy Liberation Serif", SkFontStyle::FromOldStyle(SkTypeface::kItalic), gTestFonts[10], "LiberationSerif-Italic.ttf" },
    { "Toy Liberation Serif", SkFontStyle::FromOldStyle(SkTypeface::kBoldItalic), gTestFonts[11], "LiberationSerif-BoldItalic.ttf" },
};

const int gSubFontsCount = (int) SK_ARRAY_COUNT(gSubFonts);

const int gDefaultFontIndex = 4;
