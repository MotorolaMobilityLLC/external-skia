/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/*****************************************************************************************
 ******************** This file was generated by sksllex. Do not edit. *******************
 *****************************************************************************************/
#include "SkSLLayoutLexer.h"

namespace SkSL {

static int8_t mappings[127] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                               1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                               1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                               1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                               1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                               1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static int16_t transitions[2][1] = {
        {
                0,
        },
        {
                0,
        },
};

static int8_t accepts[1] = {
        -1,
};

LayoutToken LayoutLexer::next() {
    // note that we cheat here: normally a lexer needs to worry about the case
    // where a token has a prefix which is not itself a valid token - for instance,
    // maybe we have a valid token 'while', but 'w', 'wh', etc. are not valid
    // tokens. Our grammar doesn't have this property, so we can simplify the logic
    // a bit.
    int32_t startOffset = fOffset;
    if (startOffset == fLength) {
        return LayoutToken(LayoutToken::END_OF_FILE, startOffset, 0);
    }
    int16_t state = 1;
    while (fOffset < fLength) {
        if ((uint8_t)fText[fOffset] >= 127) {
            ++fOffset;
            break;
        }
        int16_t newState = transitions[mappings[(int)fText[fOffset]]][state];
        if (!newState) {
            break;
        }
        state = newState;
        ++fOffset;
    }
    Token::Kind kind = (LayoutToken::Kind)accepts[state];
    return LayoutToken(kind, startOffset, fOffset - startOffset);
}

}  // namespace SkSL
