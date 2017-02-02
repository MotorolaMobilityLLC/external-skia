/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_BOOLLITERAL
#define SKSL_BOOLLITERAL

#include "SkSLContext.h"
#include "SkSLExpression.h"

namespace SkSL {

/**
 * Represents 'true' or 'false'.
 */
struct BoolLiteral : public Expression {
    BoolLiteral(const Context& context, Position position, bool value)
    : INHERITED(position, kBoolLiteral_Kind, *context.fBool_Type)
    , fValue(value) {}

    SkString description() const override {
        return SkString(fValue ? "true" : "false");
    }

    bool hasSideEffects() const override {
        return false;
    }

    bool isConstant() const override {
        return true;
    }

    const bool fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
