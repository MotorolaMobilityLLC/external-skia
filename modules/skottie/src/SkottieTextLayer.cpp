/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottiePriv.h"

#include "SkFontMgr.h"
#include "SkMakeUnique.h"
#include "SkottieJson.h"
#include "SkottieValue.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGGroup.h"
#include "SkSGText.h"
#include "SkTypes.h"

#include <string.h>

namespace skottie {
namespace internal {

namespace {

bool ParseGlyph(const skjson::ObjectValue* jglyph, FontInfo* finfo) {
    // TODO: add glyphs support.

    return true;
}

SkFontStyle FontStyle(const char* style) {
    static constexpr struct {
        const char*               fName;
        const SkFontStyle::Weight fWeight;
    } gWeightMap[] = {
        { "ExtraLight", SkFontStyle::kExtraLight_Weight },
        { "Light"     , SkFontStyle::kLight_Weight      },
        { "Regular"   , SkFontStyle::kNormal_Weight     },
        { "Medium"    , SkFontStyle::kMedium_Weight     },
        { "SemiBold"  , SkFontStyle::kSemiBold_Weight   },
        { "Bold"      , SkFontStyle::kBold_Weight       },
        { "ExtraBold" , SkFontStyle::kExtraBold_Weight  },
    };

    SkFontStyle::Weight weight = SkFontStyle::kNormal_Weight;
    for (const auto& w : gWeightMap) {
        const auto name_len = strlen(w.fName);
        if (!strncmp(style, w.fName, name_len)) {
            weight = w.fWeight;
            style += name_len;
            break;
        }
    }

    static constexpr struct {
        const char*              fName;
        const SkFontStyle::Slant fSlant;
    } gSlantMap[] = {
        { "Italic" , SkFontStyle::kItalic_Slant  },
        { "Oblique", SkFontStyle::kOblique_Slant },
    };

    SkFontStyle::Slant slant = SkFontStyle::kUpright_Slant;
    if (*style != '\0') {
        for (const auto& s : gSlantMap) {
            if (!strcmp(style, s.fName)) {
                slant = s.fSlant;
                style += strlen(s.fName);
                break;
            }
        }
    }

    if (*style != '\0') {
        LOG("?? Unknown font style: %s\n", style);
    }

    return SkFontStyle(weight, SkFontStyle::kNormal_Width, slant);
}

} // namespace

bool FontInfo::matches(const char family[], const char style[]) const {
    return 0 == strcmp(fFamily.c_str(), family)
        && 0 == strcmp(fStyle.c_str(), style);
}

FontMap ParseFonts(const skjson::ObjectValue* jfonts, const skjson::ArrayValue* jchars,
                   const SkFontMgr* fontmgr) {
    FontMap fonts;

    // Optional array of font entries, referenced (by name) from text layer document nodes. E.g.
    // "fonts": {
    //        "list": [
    //            {
    //                "ascent": 75,
    //                "fClass": "",
    //                "fFamily": "Roboto",
    //                "fName": "Roboto-Regular",
    //                "fPath": "",
    //                "fStyle": "Regular",
    //                "fWeight": "",
    //                "origin": 1
    //            }
    //        ]
    //    },
    if (jfonts) {
        if (const skjson::ArrayValue* jlist = (*jfonts)["list"]) {
            for (const skjson::ObjectValue* jfont : *jlist) {
                if (!jfont) {
                    continue;
                }

                const skjson::StringValue* jname   = (*jfont)["fName"];
                const skjson::StringValue* jfamily = (*jfont)["fFamily"];
                const skjson::StringValue* jstyle  = (*jfont)["fStyle"];

                if (!jname   || !jname->size() ||
                    !jfamily || !jfamily->size() ||
                    !jstyle  || !jstyle->size()) {
                    LogJSON(*jfont, "!! Ignoring invalid font");
                    continue;
                }

                sk_sp<SkTypeface> tf(fontmgr->matchFamilyStyle(jfamily->begin(),
                                                               FontStyle(jstyle->begin())));
                if (!tf) {
                    LOG("!! Could not create typeface for %s|%s\n",
                        jfamily->begin(), jstyle->begin());
                    // Last resort.
                    tf.reset(fontmgr->matchFamilyStyle("Arial", SkFontStyle::Normal()));
                    if (!tf) {
                        continue;
                    }
                }

                fonts.set(SkString(jname->begin(), jname->size()),
                          {
                              SkString(jfamily->begin(), jfamily->size()),
                              SkString(jstyle->begin(), jstyle->size()),
                              ParseDefault((*jfont)["ascent"] , 0.0f),
                              std::move(tf)
                          });
            }
        }
    }

    // Optional array of glyphs, to be associated with one of the declared fonts. E.g.
    // "chars": [
    //     {
    //         "ch": "t",
    //         "data": {
    //             "shapes": [...]
    //         },
    //         "fFamily": "Roboto",
    //         "size": 50,
    //         "style": "Regular",
    //         "w": 32.67
    //    }
    // ]
    if (jchars) {
        FontInfo* current_font = nullptr;

        for (const skjson::ObjectValue* jchar : *jchars) {
            if (!jchar) {
                continue;
            }

            const skjson::StringValue* jch = (*jchar)["ch"];
            if (!jch) {
                continue;
            }

            const skjson::StringValue* jfamily = (*jchar)["fFamily"];
            const skjson::StringValue* jstyle  = (*jchar)["style"]; // "style", not "fStyle"...

            const auto* ch_ptr = jch->begin();
            const auto  ch_len = jch->size();

            if (!jfamily || !jstyle || (SkUTF::CountUTF8(ch_ptr, ch_len) != 1)) {
                LogJSON(*jchar, "!! Invalid glyph");
                continue;
            }

            const auto uni = SkUTF::NextUTF8(&ch_ptr, ch_ptr + ch_len);
            SkASSERT(uni != -1);

            const auto* family = jfamily->begin();
            const auto* style  = jstyle->begin();

            // Locate (and cache) the font info. Unlike text nodes, glyphs reference the font by
            // (family, style) -- not by name :(  For now this performs a linear search over *all*
            // fonts: generally there are few of them, and glyph definitions are font-clustered.
            // If problematic, we can refactor as a two-level hashmap.
            if (!current_font || !current_font->matches(family, style)) {
                current_font = nullptr;
                fonts.foreach([&](const SkString& name, FontInfo* finfo) {
                    if (finfo->matches(family, style)) {
                        current_font = finfo;
                        // TODO: would be nice to break early here...
                    }
                });
                if (!current_font) {
                    LOG("!! Font not found for codepoint (%d, %s, %s)\n", uni, family, style);
                    continue;
                }
            }

            if (!ParseGlyph(*jchar, current_font)) {
                LogJSON(*jchar, "!! Invalid glyph");
            }
        }
    }

    return fonts;
}

sk_sp<sksg::RenderNode> AttachTextLayer(const skjson::ObjectValue& layer, AttachContext* ctx) {
    // General text node format:
    // "t": {
    //    "a": [], // animators (TODO)
    //    "d": {
    //        "k": [
    //            {
    //                "s": {
    //                    "f": "Roboto-Regular",
    //                    "fc": [
    //                        0.42,
    //                        0.15,
    //                        0.15
    //                    ],
    //                    "j": 1,
    //                    "lh": 60,
    //                    "ls": 0,
    //                    "s": 50,
    //                    "t": "text align right",
    //                    "tr": 0
    //                },
    //                "t": 0
    //            }
    //        ]
    //    },
    //    "m": {}, // "more options" (TODO)
    //    "p": {}  // "path options" (TODO)
    // },
    const skjson::ObjectValue* jt = layer["t"];
    if (!jt) {
        LogJSON(layer, "!! Missing text layer \"t\" property");
        return nullptr;
    }

    const skjson::ArrayValue* animated_props = (*jt)["a"];
    if (animated_props && animated_props->size() > 0) {
        LOG("?? Unsupported animated text properties.\n");
    }

    // TODO: The "d" node is keyframed, not static. Add a new animated value type and parse as such.
    const skjson::ObjectValue* jd  = (*jt)["d"];
    const skjson::ArrayValue*  jk  = jd
            ? (*jd)["k"].operator const skjson::ArrayValue*() : nullptr;
    const skjson::ObjectValue* jv0 = jk && jk->size() == 1
            ? (*jk)[0].operator const skjson::ObjectValue*() : nullptr;
    const skjson::ObjectValue* jprops = jv0
            ? (*jv0)["s"].operator const skjson::ObjectValue*() : nullptr;

    if (!jprops) {
        LogJSON(*jt, "!! Unexpected text property");
        return nullptr;
    }

    const skjson::StringValue* font_name = (*jprops)["f"];
    const skjson::StringValue* text      = (*jprops)["t"];
    const skjson::NumberValue* text_size = (*jprops)["s"];
    if (!font_name || !text || !text_size) {
        LogJSON(*jprops, "!! Invalid text properties");
        return nullptr;
    }

    const auto* font = ctx->fFonts.find(SkString(font_name->begin(), font_name->size()));
    if (!font) {
        LOG("!! Unknown font: \"%s\"\n", font_name->begin());
        return nullptr;
    }

    static constexpr SkPaint::Align gAlignMap[] = {
        SkPaint::kLeft_Align,  // 'j': 0
        SkPaint::kRight_Align, // 'j': 1
        SkPaint::kCenter_Align // 'j': 2
    };
    const auto align = gAlignMap[SkTMin<size_t>(ParseDefault<size_t>((*jprops)["j"], 0),
                                                SK_ARRAY_COUNT(gAlignMap))];

    // Emit a SG fragment with the following general format:
    //
    // [Group]
    //   [Draw]
    //     [FillPaint]
    //     [Text]
    //   [Draw]
    //     [StrokePaint]
    //     [Text]
    //
    auto text_node = sksg::Text::Make(font->fTypeface, SkString(text->begin(), text->size()));
    text_node->setSize(**text_size);
    text_node->setAlign(align);

    const auto parse_color = [](const skjson::ArrayValue* jcolor) -> sk_sp<sksg::Color> {
        VectorValue color_vec;
        if (!jcolor || !Parse(*jcolor, &color_vec)) {
            return nullptr;
        }

        auto paint = sksg::Color::Make(ValueTraits<VectorValue>::As<SkColor>(color_vec));
        paint->setAntiAlias(true);

        return paint;
    };

    auto fill_paint = parse_color((*jprops)["fc"]),
       stroke_paint = parse_color((*jprops)["sc"]);
    auto  fill_node = sksg::Draw::Make(text_node, fill_paint),
        stroke_node = sksg::Draw::Make(text_node, stroke_paint);

    if (!stroke_node) {
        return std::move(fill_node);
    }

    stroke_paint->setStyle(SkPaint::kStroke_Style);
    stroke_paint->setStrokeWidth(ParseDefault((*jprops)["sw"], 0.0f));

    if (!fill_node) {
        return std::move(stroke_node);
    }

    // Fill & stroke
    auto group_node = sksg::Group::Make();
    group_node->addChild(std::move(fill_node));
    group_node->addChild(std::move(stroke_node));

    return std::move(group_node);
}

} // namespace internal
} // namespace skottie
