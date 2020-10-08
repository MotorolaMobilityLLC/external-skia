/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DISCARDSTATEMENT
#define SKSL_DISCARDSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'discard' statement.
 */
class DiscardStatement : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kDiscard;

    DiscardStatement(int offset)
    : INHERITED(offset, kStatementKind) {}

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new DiscardStatement(fOffset));
    }

    String description() const override {
        return String("discard;");
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
