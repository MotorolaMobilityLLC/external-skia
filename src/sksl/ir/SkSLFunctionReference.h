/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_FUNCTIONREFERENCE
#define SKSL_FUNCTIONREFERENCE

#include "SkSLExpression.h"

namespace SkSL {

/**
 * An identifier referring to a function name. This is an intermediate value: FunctionReferences are 
 * always eventually replaced by FunctionCalls in valid programs.
 */
struct FunctionReference : public Expression {
    FunctionReference(Position position, std::vector<std::shared_ptr<FunctionDeclaration>> function)
    : INHERITED(position, kFunctionReference_Kind, kInvalid_Type)
    , fFunctions(function) {}

    virtual std::string description() const override {
    	ASSERT(false);
    	return "<function>";
    }

    const std::vector<std::shared_ptr<FunctionDeclaration>> fFunctions;

    typedef Expression INHERITED;
};

} // namespace

#endif
