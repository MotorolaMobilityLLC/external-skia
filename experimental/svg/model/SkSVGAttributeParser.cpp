/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkParse.h"
#include "SkSVGAttributeParser.h"
#include "SkSVGTypes.h"

namespace {

// TODO: these should be shared with SkParse.cpp

inline bool is_between(char c, char min, char max) {
    SkASSERT(min <= max);
    return (unsigned)(c - min) <= (unsigned)(max - min);
}

inline bool is_eos(char c) {
    return !c;
}

inline bool is_ws(char c) {
    return is_between(c, 1, 32);
}

inline bool is_sep(char c) {
    return is_ws(c) || c == ',' || c == ';';
}

} // anonymous ns

SkSVGAttributeParser::SkSVGAttributeParser(const char attributeString[])
    : fCurPos(attributeString) {}

template <typename F>
inline bool SkSVGAttributeParser::advanceWhile(F f) {
    auto initial = fCurPos;
    while (f(*fCurPos)) {
        fCurPos++;
    }
    return fCurPos != initial;
}

inline bool SkSVGAttributeParser::parseEOSToken() {
    return is_eos(*fCurPos);
}

inline bool SkSVGAttributeParser::parseSepToken() {
    return this->advanceWhile(is_sep);
}

inline bool SkSVGAttributeParser::parseWSToken() {
    return this->advanceWhile(is_ws);
}

inline bool SkSVGAttributeParser::parseExpectedStringToken(const char* expected) {
    const char* c = fCurPos;

    while (*c && *expected && *c == *expected) {
        c++;
        expected++;
    }

    if (*expected) {
        return false;
    }

    fCurPos = c;
    return true;
}

bool SkSVGAttributeParser::parseScalarToken(SkScalar* res) {
    if (const char* next = SkParse::FindScalar(fCurPos, res)) {
        fCurPos = next;
        return true;
    }
    return false;
}

bool SkSVGAttributeParser::parseHexToken(uint32_t* res) {
     if (const char* next = SkParse::FindHex(fCurPos, res)) {
         fCurPos = next;
         return true;
     }
     return false;
}

bool SkSVGAttributeParser::parseLengthUnitToken(SkSVGLength::Unit* unit) {
    static const struct {
        const char*       fUnitName;
        SkSVGLength::Unit fUnit;
    } gUnitInfo[] = {
        { "%" , SkSVGLength::Unit::kPercentage },
        { "em", SkSVGLength::Unit::kEMS        },
        { "ex", SkSVGLength::Unit::kEXS        },
        { "px", SkSVGLength::Unit::kPX         },
        { "cm", SkSVGLength::Unit::kCM         },
        { "mm", SkSVGLength::Unit::kMM         },
        { "in", SkSVGLength::Unit::kIN         },
        { "pt", SkSVGLength::Unit::kPT         },
        { "pc", SkSVGLength::Unit::kPC         },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gUnitInfo); ++i) {
        if (this->parseExpectedStringToken(gUnitInfo[i].fUnitName)) {
            *unit = gUnitInfo[i].fUnit;
            return true;
        }
    }
    return false;
}

bool SkSVGAttributeParser::parseNamedColorToken(SkColor* c) {
    if (const char* next = SkParse::FindNamedColor(fCurPos, strlen(fCurPos), c)) {
        fCurPos = next;
        return true;
    }
    return false;
}

bool SkSVGAttributeParser::parseHexColorToken(SkColor* c) {
    uint32_t v;
    const char* initial = fCurPos;

    if (!this->parseExpectedStringToken("#") || !this->parseHexToken(&v)) {
        return false;
    }

    switch (fCurPos - initial) {
    case 7:
        // matched #xxxxxxx
        break;
    case 4:
        // matched '#xxx;
        v = ((v << 12) & 0x00f00000) |
            ((v <<  8) & 0x000ff000) |
            ((v <<  4) & 0x00000ff0) |
            ((v <<  0) & 0x0000000f);
        break;
    default:
        return false;
    }

    *c = v | 0xff000000;
    return true;
}

// https://www.w3.org/TR/SVG/types.html#DataTypeColor
bool SkSVGAttributeParser::parseColor(SkSVGColorType* color) {
    SkColor c;

    // consume preceding whitespace
    this->parseWSToken();

    // TODO: rgb(...)
    bool parsedValue = false;
    if (this->parseHexColorToken(&c) || this->parseNamedColorToken(&c)) {
        *color = SkSVGColorType(c);
        parsedValue = true;

        // consume trailing whitespace
        this->parseWSToken();
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG/types.html#DataTypeNumber
bool SkSVGAttributeParser::parseNumber(SkSVGNumberType* number) {
    // consume WS
    this->parseWSToken();

    SkScalar s;
    if (this->parseScalarToken(&s)) {
        *number = SkSVGNumberType(s);
        // consume trailing separators
        this->parseSepToken();
        return true;
    }

    return false;
}

// https://www.w3.org/TR/SVG/types.html#DataTypeLength
bool SkSVGAttributeParser::parseLength(SkSVGLength* length) {
    SkScalar s;
    SkSVGLength::Unit u = SkSVGLength::Unit::kNumber;

    if (this->parseScalarToken(&s) &&
        (this->parseLengthUnitToken(&u) || this->parseSepToken() || this->parseEOSToken())) {
        *length = SkSVGLength(s, u);
        // consume trailing separators
        this->parseSepToken();
        return true;
    }

    return false;
}

// https://www.w3.org/TR/SVG/coords.html#ViewBoxAttribute
bool SkSVGAttributeParser::parseViewBox(SkSVGViewBoxType* vb) {
    SkScalar x, y, w, h;
    this->parseWSToken();

    bool parsedValue = false;
    if (this->parseScalarToken(&x) && this->parseSepToken() &&
        this->parseScalarToken(&y) && this->parseSepToken() &&
        this->parseScalarToken(&w) && this->parseSepToken() &&
        this->parseScalarToken(&h)) {

        *vb = SkSVGViewBoxType(SkRect::MakeXYWH(x, y, w, h));
        parsedValue = true;
        // consume trailing whitespace
        this->parseWSToken();
    }
    return parsedValue && this->parseEOSToken();
}

template <typename Func, typename T>
bool SkSVGAttributeParser::parseParenthesized(const char* prefix, Func f, T* result) {
    this->parseWSToken();
    if (prefix && !this->parseExpectedStringToken(prefix)) {
        return false;
    }
    this->parseWSToken();
    if (!this->parseExpectedStringToken("(")) {
        return false;
    }
    this->parseWSToken();

    if (!f(result)) {
        return false;
    }
    this->parseWSToken();

    return this->parseExpectedStringToken(")");
}

bool SkSVGAttributeParser::parseMatrixToken(SkMatrix* matrix) {
    return this->parseParenthesized("matrix", [this](SkMatrix* m) -> bool {
        SkScalar scalars[6];
        for (int i = 0; i < 6; ++i) {
            if (!(this->parseScalarToken(scalars + i) &&
                  (i > 4 || this->parseSepToken()))) {
                return false;
            }
        }

        m->setAll(scalars[0], scalars[2], scalars[4], scalars[1], scalars[3], scalars[5], 0, 0, 1);
        return true;
    }, matrix);
}

bool SkSVGAttributeParser::parseTranslateToken(SkMatrix* matrix) {
    return this->parseParenthesized("translate", [this](SkMatrix* m) -> bool {
        SkScalar tx, ty;
        this->parseWSToken();
        if (!this->parseScalarToken(&tx)) {
            return false;
        }

        if (!(this->parseSepToken() && this->parseScalarToken(&ty))) {
            ty = tx;
        }

        m->setTranslate(tx, ty);
        return true;
    }, matrix);
}

bool SkSVGAttributeParser::parseScaleToken(SkMatrix* matrix) {
    return this->parseParenthesized("scale", [this](SkMatrix* m) -> bool {
        SkScalar sx, sy;
        if (!this->parseScalarToken(&sx)) {
            return false;
        }

        if (!(this->parseSepToken() && this->parseScalarToken(&sy))) {
            sy = sx;
        }

        m->setScale(sx, sy);
        return true;
    }, matrix);
}

bool SkSVGAttributeParser::parseRotateToken(SkMatrix* matrix) {
    return this->parseParenthesized("rotate", [this](SkMatrix* m) -> bool {
        SkScalar angle;
        if (!this->parseScalarToken(&angle)) {
            return false;
        }

        SkScalar cx = 0;
        SkScalar cy = 0;
        // optional [<cx> <cy>]
        if (this->parseSepToken() && this->parseScalarToken(&cx)) {
            if (!(this->parseSepToken() && this->parseScalarToken(&cy))) {
                return false;
            }
        }

        m->setRotate(angle, cx, cy);
        return true;
    }, matrix);
}

bool SkSVGAttributeParser::parseSkewXToken(SkMatrix* matrix) {
    return this->parseParenthesized("skewX", [this](SkMatrix* m) -> bool {
        SkScalar angle;
        if (!this->parseScalarToken(&angle)) {
            return false;
        }
        m->setSkewX(angle);
        return true;
    }, matrix);
}

bool SkSVGAttributeParser::parseSkewYToken(SkMatrix* matrix) {
    return this->parseParenthesized("skewY", [this](SkMatrix* m) -> bool {
        SkScalar angle;
        if (!this->parseScalarToken(&angle)) {
            return false;
        }
        m->setSkewY(angle);
        return true;
    }, matrix);
}

// https://www.w3.org/TR/SVG/coords.html#TransformAttribute
bool SkSVGAttributeParser::parseTransform(SkSVGTransformType* t) {
    SkMatrix matrix = SkMatrix::I();

    bool parsed = false;
    while (true) {
        SkMatrix m;

        if (!( this->parseMatrixToken(&m)
            || this->parseTranslateToken(&m)
            || this->parseScaleToken(&m)
            || this->parseRotateToken(&m)
            || this->parseSkewXToken(&m)
            || this->parseSkewYToken(&m))) {
            break;
        }

        matrix.preConcat(m);
        parsed = true;
    }

    this->parseWSToken();
    if (!parsed || !this->parseEOSToken()) {
        return false;
    }

    *t = SkSVGTransformType(matrix);
    return true;
}
