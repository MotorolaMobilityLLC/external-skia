/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#include "SkSLIRGenerator.h"

#include "limits.h"

#include "ast/SkSLASTBoolLiteral.h"
#include "ast/SkSLASTFieldSuffix.h"
#include "ast/SkSLASTFloatLiteral.h"
#include "ast/SkSLASTIndexSuffix.h"
#include "ast/SkSLASTIntLiteral.h"
#include "ir/SkSLBinaryExpression.h"
#include "ir/SkSLBoolLiteral.h"
#include "ir/SkSLBreakStatement.h"
#include "ir/SkSLConstructor.h"
#include "ir/SkSLContinueStatement.h"
#include "ir/SkSLDiscardStatement.h"
#include "ir/SkSLDoStatement.h"
#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLField.h"
#include "ir/SkSLFieldAccess.h"
#include "ir/SkSLFloatLiteral.h"
#include "ir/SkSLForStatement.h"
#include "ir/SkSLFunctionCall.h"
#include "ir/SkSLFunctionDeclaration.h"
#include "ir/SkSLFunctionDefinition.h"
#include "ir/SkSLFunctionReference.h"
#include "ir/SkSLIfStatement.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLInterfaceBlock.h"
#include "ir/SkSLIntLiteral.h"
#include "ir/SkSLLayout.h"
#include "ir/SkSLPostfixExpression.h"
#include "ir/SkSLPrefixExpression.h"
#include "ir/SkSLReturnStatement.h"
#include "ir/SkSLSwizzle.h"
#include "ir/SkSLTernaryExpression.h"
#include "ir/SkSLUnresolvedFunction.h"
#include "ir/SkSLVariable.h"
#include "ir/SkSLVarDeclaration.h"
#include "ir/SkSLVarDeclarationStatement.h"
#include "ir/SkSLVariableReference.h"
#include "ir/SkSLWhileStatement.h"

namespace SkSL {

class AutoSymbolTable {
public:
    AutoSymbolTable(IRGenerator* ir) 
    : fIR(ir)
    , fPrevious(fIR->fSymbolTable) {
        fIR->pushSymbolTable();
    }

    ~AutoSymbolTable() {
        fIR->popSymbolTable();
        ASSERT(fPrevious == fIR->fSymbolTable);
    }

    IRGenerator* fIR;
    std::shared_ptr<SymbolTable> fPrevious;
};

IRGenerator::IRGenerator(std::shared_ptr<SymbolTable> symbolTable, 
                         ErrorReporter& errorReporter)
: fSymbolTable(std::move(symbolTable))
, fErrors(errorReporter) {
}

void IRGenerator::pushSymbolTable() {
    fSymbolTable.reset(new SymbolTable(std::move(fSymbolTable), fErrors));
}

void IRGenerator::popSymbolTable() {
    fSymbolTable = fSymbolTable->fParent;
}

std::unique_ptr<Extension> IRGenerator::convertExtension(const ASTExtension& extension) {
    return std::unique_ptr<Extension>(new Extension(extension.fPosition, extension.fName));
}

std::unique_ptr<Statement> IRGenerator::convertStatement(const ASTStatement& statement) {
    switch (statement.fKind) {
        case ASTStatement::kBlock_Kind:
            return this->convertBlock((ASTBlock&) statement);
        case ASTStatement::kVarDeclaration_Kind:
            return this->convertVarDeclarationStatement((ASTVarDeclarationStatement&) statement);
        case ASTStatement::kExpression_Kind:
            return this->convertExpressionStatement((ASTExpressionStatement&) statement);
        case ASTStatement::kIf_Kind:
            return this->convertIf((ASTIfStatement&) statement);
        case ASTStatement::kFor_Kind:
            return this->convertFor((ASTForStatement&) statement);
        case ASTStatement::kWhile_Kind:
            return this->convertWhile((ASTWhileStatement&) statement);
        case ASTStatement::kDo_Kind:
            return this->convertDo((ASTDoStatement&) statement);
        case ASTStatement::kReturn_Kind:
            return this->convertReturn((ASTReturnStatement&) statement);
        case ASTStatement::kBreak_Kind:
            return this->convertBreak((ASTBreakStatement&) statement);
        case ASTStatement::kContinue_Kind:
            return this->convertContinue((ASTContinueStatement&) statement);
        case ASTStatement::kDiscard_Kind:
            return this->convertDiscard((ASTDiscardStatement&) statement);
        default:
            ABORT("unsupported statement type: %d\n", statement.fKind);
    }
}

std::unique_ptr<Block> IRGenerator::convertBlock(const ASTBlock& block) {
    AutoSymbolTable table(this);
    std::vector<std::unique_ptr<Statement>> statements;
    for (size_t i = 0; i < block.fStatements.size(); i++) {
        std::unique_ptr<Statement> statement = this->convertStatement(*block.fStatements[i]);
        if (!statement) {
            return nullptr;
        }
        statements.push_back(std::move(statement));
    }
    return std::unique_ptr<Block>(new Block(block.fPosition, std::move(statements)));
}

std::unique_ptr<Statement> IRGenerator::convertVarDeclarationStatement(
                                                              const ASTVarDeclarationStatement& s) {
    auto decl = this->convertVarDeclaration(*s.fDeclaration, Variable::kLocal_Storage);
    if (!decl) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new VarDeclarationStatement(std::move(decl)));
}

Modifiers IRGenerator::convertModifiers(const ASTModifiers& modifiers) {
    return Modifiers(modifiers);
}

std::unique_ptr<VarDeclaration> IRGenerator::convertVarDeclaration(const ASTVarDeclaration& decl,
                                                                   Variable::Storage storage) {
    std::vector<std::shared_ptr<Variable>> variables;
    std::vector<std::vector<std::unique_ptr<Expression>>> sizes;
    std::vector<std::unique_ptr<Expression>> values;
    std::shared_ptr<Type> baseType = this->convertType(*decl.fType);
    if (!baseType) {
        return nullptr;
    }
    for (size_t i = 0; i < decl.fNames.size(); i++) {
        Modifiers modifiers = this->convertModifiers(decl.fModifiers);
        std::shared_ptr<Type> type = baseType;
        ASSERT(type->kind() != Type::kArray_Kind);
        std::vector<std::unique_ptr<Expression>> currentVarSizes;
        for (size_t j = 0; j < decl.fSizes[i].size(); j++) {
            if (decl.fSizes[i][j]) {
                ASTExpression& rawSize = *decl.fSizes[i][j];
                auto size = this->coerce(this->convertExpression(rawSize), kInt_Type);
                if (!size) {
                    return nullptr;
                }
                std::string name = type->fName;
                uint64_t count;
                if (size->fKind == Expression::kIntLiteral_Kind) {
                    count = ((IntLiteral&) *size).fValue;
                    if (count <= 0) {
                        fErrors.error(size->fPosition, "array size must be positive");
                    }
                    name += "[" + to_string(count) + "]";
                } else {
                    count = -1;
                    name += "[]";
                }
                type = std::shared_ptr<Type>(new Type(name, Type::kArray_Kind, type, (int) count));
                currentVarSizes.push_back(std::move(size));
            } else {
                type = std::shared_ptr<Type>(new Type(type->fName + "[]", Type::kArray_Kind, type, 
                                                      -1));
                currentVarSizes.push_back(nullptr);
            }
        }
        sizes.push_back(std::move(currentVarSizes));
        auto var = std::make_shared<Variable>(decl.fPosition, modifiers, decl.fNames[i], type, 
                                              storage);
        variables.push_back(var);
        std::unique_ptr<Expression> value;
        if (decl.fValues[i]) {
            value = this->convertExpression(*decl.fValues[i]);
            if (!value) {
                return nullptr;
            }
            value = this->coerce(std::move(value), type);
        }
        fSymbolTable->add(var->fName, var);
        values.push_back(std::move(value));
    }
    return std::unique_ptr<VarDeclaration>(new VarDeclaration(decl.fPosition, std::move(variables), 
                                                              std::move(sizes), std::move(values)));
}

std::unique_ptr<Statement> IRGenerator::convertIf(const ASTIfStatement& s) {
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*s.fTest), kBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Statement> ifTrue = this->convertStatement(*s.fIfTrue);
    if (!ifTrue) {
        return nullptr;
    }
    std::unique_ptr<Statement> ifFalse;
    if (s.fIfFalse) {
        ifFalse = this->convertStatement(*s.fIfFalse);
        if (!ifFalse) {
            return nullptr;
        }
    }
    return std::unique_ptr<Statement>(new IfStatement(s.fPosition, std::move(test), 
                                                      std::move(ifTrue), std::move(ifFalse)));
}

std::unique_ptr<Statement> IRGenerator::convertFor(const ASTForStatement& f) {
    AutoSymbolTable table(this);
    std::unique_ptr<Statement> initializer = this->convertStatement(*f.fInitializer);
    if (!initializer) {
        return nullptr;
    }
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*f.fTest), kBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Expression> next = this->convertExpression(*f.fNext);
    if (!next) {
        return nullptr;
    }
    this->checkValid(*next);
    std::unique_ptr<Statement> statement = this->convertStatement(*f.fStatement);
    if (!statement) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new ForStatement(f.fPosition, std::move(initializer), 
                                                       std::move(test), std::move(next),
                                                       std::move(statement)));
}

std::unique_ptr<Statement> IRGenerator::convertWhile(const ASTWhileStatement& w) {
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*w.fTest), kBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Statement> statement = this->convertStatement(*w.fStatement);
    if (!statement) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new WhileStatement(w.fPosition, std::move(test),
                                                         std::move(statement)));
}

std::unique_ptr<Statement> IRGenerator::convertDo(const ASTDoStatement& d) {
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*d.fTest), kBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Statement> statement = this->convertStatement(*d.fStatement);
    if (!statement) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new DoStatement(d.fPosition, std::move(statement), 
                                                      std::move(test)));
}

std::unique_ptr<Statement> IRGenerator::convertExpressionStatement(
                                                                  const ASTExpressionStatement& s) {
    std::unique_ptr<Expression> e = this->convertExpression(*s.fExpression);
    if (!e) {
        return nullptr;
    }
    this->checkValid(*e);
    return std::unique_ptr<Statement>(new ExpressionStatement(std::move(e)));
}

std::unique_ptr<Statement> IRGenerator::convertReturn(const ASTReturnStatement& r) {
    ASSERT(fCurrentFunction);
    if (r.fExpression) {
        std::unique_ptr<Expression> result = this->convertExpression(*r.fExpression);
        if (!result) {
            return nullptr;
        }
        if (fCurrentFunction->fReturnType == kVoid_Type) {
            fErrors.error(result->fPosition, "may not return a value from a void function");
        } else {
            result = this->coerce(std::move(result), fCurrentFunction->fReturnType);
            if (!result) {
                return nullptr;
            }
        }
        return std::unique_ptr<Statement>(new ReturnStatement(std::move(result)));
    } else {
        if (fCurrentFunction->fReturnType != kVoid_Type) {
            fErrors.error(r.fPosition, "expected function to return '" +
                                       fCurrentFunction->fReturnType->description() + "'");
        }
        return std::unique_ptr<Statement>(new ReturnStatement(r.fPosition));
    }
}

std::unique_ptr<Statement> IRGenerator::convertBreak(const ASTBreakStatement& b) {
    return std::unique_ptr<Statement>(new BreakStatement(b.fPosition));
}

std::unique_ptr<Statement> IRGenerator::convertContinue(const ASTContinueStatement& c) {
    return std::unique_ptr<Statement>(new ContinueStatement(c.fPosition));
}

std::unique_ptr<Statement> IRGenerator::convertDiscard(const ASTDiscardStatement& d) {
    return std::unique_ptr<Statement>(new DiscardStatement(d.fPosition));
}

static std::shared_ptr<Type> expand_generics(std::shared_ptr<Type> type, int i) {
    if (type->kind() == Type::kGeneric_Kind) {
        return type->coercibleTypes()[i];
    }
    return type;
}

static void expand_generics(FunctionDeclaration& decl, 
                            SymbolTable& symbolTable) {
    for (int i = 0; i < 4; i++) {
        std::shared_ptr<Type> returnType = expand_generics(decl.fReturnType, i);
        std::vector<std::shared_ptr<Variable>> arguments;
        for (const auto& p : decl.fParameters) {
            arguments.push_back(std::shared_ptr<Variable>(new Variable(
                                                                    p->fPosition, 
                                                                    Modifiers(p->fModifiers), 
                                                                    p->fName,
                                                                    expand_generics(p->fType, i),
                                                                    Variable::kParameter_Storage)));
        }
        std::shared_ptr<FunctionDeclaration> expanded(new FunctionDeclaration(
                                                                            decl.fPosition, 
                                                                            decl.fName, 
                                                                            std::move(arguments), 
                                                                            std::move(returnType)));
        symbolTable.add(expanded->fName, expanded);
    }
}

std::unique_ptr<FunctionDefinition> IRGenerator::convertFunction(const ASTFunction& f) {
    std::shared_ptr<SymbolTable> old = fSymbolTable;
    AutoSymbolTable table(this);
    bool isGeneric;
    std::shared_ptr<Type> returnType = this->convertType(*f.fReturnType);
    if (!returnType) {
        return nullptr;
    }
    isGeneric = returnType->kind() == Type::kGeneric_Kind;
    std::vector<std::shared_ptr<Variable>> parameters;
    for (const auto& param : f.fParameters) {
        std::shared_ptr<Type> type = this->convertType(*param->fType);
        if (!type) {
            return nullptr;
        }
        for (int j = (int) param->fSizes.size() - 1; j >= 0; j--) {
            int size = param->fSizes[j];
            std::string name = type->name() + "[" + to_string(size) + "]";
            type = std::shared_ptr<Type>(new Type(std::move(name), Type::kArray_Kind, 
                                                  std::move(type), size));
        }
        std::string name = param->fName;
        Modifiers modifiers = this->convertModifiers(param->fModifiers);
        Position pos = param->fPosition;
        std::shared_ptr<Variable> var = std::shared_ptr<Variable>(new Variable(
                                                                     pos, 
                                                                     modifiers, 
                                                                     std::move(name), 
                                                                     type,
                                                                     Variable::kParameter_Storage));
        parameters.push_back(std::move(var));
        isGeneric |= type->kind() == Type::kGeneric_Kind;
    }

    // find existing declaration
    std::shared_ptr<FunctionDeclaration> decl;
    auto entry = (*old)[f.fName];
    if (entry) {
        std::vector<std::shared_ptr<FunctionDeclaration>> functions;
        switch (entry->fKind) {
            case Symbol::kUnresolvedFunction_Kind:
                functions = std::static_pointer_cast<UnresolvedFunction>(entry)->fFunctions;
                break;
            case Symbol::kFunctionDeclaration_Kind:
                functions.push_back(std::static_pointer_cast<FunctionDeclaration>(entry));
                break;
            default:
                fErrors.error(f.fPosition, "symbol '" + f.fName + "' was already defined");
                return nullptr;
        }
        for (const auto& other : functions) {
            ASSERT(other->fName == f.fName);
            if (parameters.size() == other->fParameters.size()) {
                bool match = true;
                for (size_t i = 0; i < parameters.size(); i++) {
                    if (parameters[i]->fType != other->fParameters[i]->fType) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    if (returnType != other->fReturnType) {
                        FunctionDeclaration newDecl = FunctionDeclaration(f.fPosition, 
                                                                          f.fName, 
                                                                          parameters, 
                                                                          returnType);
                        fErrors.error(f.fPosition, "functions '" + newDecl.description() +
                                                   "' and '" + other->description() + 
                                                   "' differ only in return type");
                        return nullptr;
                    }
                    decl = other;
                    for (size_t i = 0; i < parameters.size(); i++) {
                        if (parameters[i]->fModifiers != other->fParameters[i]->fModifiers) {
                            fErrors.error(f.fPosition, "modifiers on parameter " + 
                                                       to_string(i + 1) + " differ between " +
                                                       "declaration and definition");
                            return nullptr;
                        }
                        fSymbolTable->add(parameters[i]->fName, decl->fParameters[i]);
                    }
                    if (other->fDefined) {
                        fErrors.error(f.fPosition, "duplicate definition of " + 
                                                   other->description());
                    }
                    break;
                }
            }
        }
    }
    if (!decl) {
        // couldn't find an existing declaration
        decl.reset(new FunctionDeclaration(f.fPosition, f.fName, parameters, returnType));
        for (auto var : parameters) {
            fSymbolTable->add(var->fName, var);
        }
    }
    if (isGeneric) {
        ASSERT(!f.fBody);
        expand_generics(*decl, *old);
    } else {
        old->add(decl->fName, decl);
        if (f.fBody) {
            ASSERT(!fCurrentFunction);
            fCurrentFunction = decl;
            decl->fDefined = true;
            std::unique_ptr<Block> body = this->convertBlock(*f.fBody);
            fCurrentFunction = nullptr;
            if (!body) {
                return nullptr;
            }
            return std::unique_ptr<FunctionDefinition>(new FunctionDefinition(f.fPosition, decl, 
                                                                              std::move(body)));
        }
    }
    return nullptr;
}

std::unique_ptr<InterfaceBlock> IRGenerator::convertInterfaceBlock(const ASTInterfaceBlock& intf) {
    std::shared_ptr<SymbolTable> old = fSymbolTable;
    AutoSymbolTable table(this);
    Modifiers mods = this->convertModifiers(intf.fModifiers);
    std::vector<Type::Field> fields;
    for (size_t i = 0; i < intf.fDeclarations.size(); i++) {
        std::unique_ptr<VarDeclaration> decl = this->convertVarDeclaration(
                                                                         *intf.fDeclarations[i], 
                                                                         Variable::kGlobal_Storage);
        for (size_t j = 0; j < decl->fVars.size(); j++) {
            fields.push_back(Type::Field(decl->fVars[j]->fModifiers, decl->fVars[j]->fName, 
                                         decl->fVars[j]->fType));
            if (decl->fValues[j]) {
                fErrors.error(decl->fPosition, 
                              "initializers are not permitted on interface block fields");
            }
            if (decl->fVars[j]->fModifiers.fFlags & (Modifiers::kIn_Flag |
                                                     Modifiers::kOut_Flag |
                                                     Modifiers::kUniform_Flag |
                                                     Modifiers::kConst_Flag)) {
                fErrors.error(decl->fPosition, 
                              "interface block fields may not have storage qualifiers");
            }
        }        
    }
    std::shared_ptr<Type> type = std::shared_ptr<Type>(new Type(intf.fInterfaceName, fields));
    std::string name = intf.fValueName.length() > 0 ? intf.fValueName : intf.fInterfaceName;
    std::shared_ptr<Variable> var = std::shared_ptr<Variable>(new Variable(intf.fPosition, mods, 
                                                                          name, type,
                                                                Variable::kGlobal_Storage));
    if (intf.fValueName.length()) {
        old->add(intf.fValueName, var);

    } else {
        for (size_t i = 0; i < fields.size(); i++) {
            std::shared_ptr<Field> field = std::shared_ptr<Field>(new Field(intf.fPosition, var, 
                                                                            (int) i));
            old->add(fields[i].fName, field);
        }
    }
    return std::unique_ptr<InterfaceBlock>(new InterfaceBlock(intf.fPosition, var));
}

std::shared_ptr<Type> IRGenerator::convertType(const ASTType& type) {
    std::shared_ptr<Symbol> result = (*fSymbolTable)[type.fName];
    if (result && result->fKind == Symbol::kType_Kind) {
        return std::static_pointer_cast<Type>(result);
    }
    fErrors.error(type.fPosition, "unknown type '" + type.fName + "'");
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertExpression(const ASTExpression& expr) {
    switch (expr.fKind) {
        case ASTExpression::kIdentifier_Kind:
            return this->convertIdentifier((ASTIdentifier&) expr);
        case ASTExpression::kBool_Kind:
            return std::unique_ptr<Expression>(new BoolLiteral(expr.fPosition,
                                                               ((ASTBoolLiteral&) expr).fValue));
        case ASTExpression::kInt_Kind:
            return std::unique_ptr<Expression>(new IntLiteral(expr.fPosition,
                                                              ((ASTIntLiteral&) expr).fValue));
        case ASTExpression::kFloat_Kind:
            return std::unique_ptr<Expression>(new FloatLiteral(expr.fPosition,
                                                                ((ASTFloatLiteral&) expr).fValue));
        case ASTExpression::kBinary_Kind:
            return this->convertBinaryExpression((ASTBinaryExpression&) expr);
        case ASTExpression::kPrefix_Kind:
            return this->convertPrefixExpression((ASTPrefixExpression&) expr);
        case ASTExpression::kSuffix_Kind:
            return this->convertSuffixExpression((ASTSuffixExpression&) expr);
        case ASTExpression::kTernary_Kind:
            return this->convertTernaryExpression((ASTTernaryExpression&) expr);
        default:
            ABORT("unsupported expression type: %d\n", expr.fKind);
    }
}

std::unique_ptr<Expression> IRGenerator::convertIdentifier(const ASTIdentifier& identifier) {
    std::shared_ptr<Symbol> result = (*fSymbolTable)[identifier.fText];
    if (!result) {
        fErrors.error(identifier.fPosition, "unknown identifier '" + identifier.fText + "'");
        return nullptr;
    }
    switch (result->fKind) {
        case Symbol::kFunctionDeclaration_Kind: {
            std::vector<std::shared_ptr<FunctionDeclaration>> f = {
                std::static_pointer_cast<FunctionDeclaration>(result)
            };
            return std::unique_ptr<FunctionReference>(new FunctionReference(identifier.fPosition,
                                                                            std::move(f)));
        }
        case Symbol::kUnresolvedFunction_Kind: {
            auto f = std::static_pointer_cast<UnresolvedFunction>(result);
            return std::unique_ptr<FunctionReference>(new FunctionReference(identifier.fPosition,
                                                                            f->fFunctions));
        }
        case Symbol::kVariable_Kind: {
            std::shared_ptr<Variable> var = std::static_pointer_cast<Variable>(result);
            this->markReadFrom(var);
            return std::unique_ptr<VariableReference>(new VariableReference(identifier.fPosition,
                                                                            std::move(var)));
        }
        case Symbol::kField_Kind: {
            std::shared_ptr<Field> field = std::static_pointer_cast<Field>(result);
            VariableReference* base = new VariableReference(identifier.fPosition, field->fOwner);
            return std::unique_ptr<Expression>(new FieldAccess(std::unique_ptr<Expression>(base),
                                                               field->fFieldIndex));
        }
        case Symbol::kType_Kind: {
            auto t = std::static_pointer_cast<Type>(result);
            return std::unique_ptr<TypeReference>(new TypeReference(identifier.fPosition, 
                                                                    std::move(t)));
        }
        default:
            ABORT("unsupported symbol type %d\n", result->fKind);
    }

}

std::unique_ptr<Expression> IRGenerator::coerce(std::unique_ptr<Expression> expr, 
                                                std::shared_ptr<Type> type) {
    if (!expr) {
        return nullptr;
    }
    if (*expr->fType == *type) {
        return expr;
    }
    this->checkValid(*expr);
    if (*expr->fType == *kInvalid_Type) {
        return nullptr;
    }
    if (!expr->fType->canCoerceTo(type)) {
        fErrors.error(expr->fPosition, "expected '" + type->description() + "', but found '" + 
                                        expr->fType->description() + "'");
        return nullptr;
    }
    if (type->kind() == Type::kScalar_Kind) {
        std::vector<std::unique_ptr<Expression>> args;
        args.push_back(std::move(expr));
        ASTIdentifier id(Position(), type->description());
        std::unique_ptr<Expression> ctor = this->convertIdentifier(id);
        ASSERT(ctor);
        return this->call(Position(), std::move(ctor), std::move(args));
    }
    ABORT("cannot coerce %s to %s", expr->fType->description().c_str(), 
          type->description().c_str());
}

/**
 * Determines the operand and result types of a binary expression. Returns true if the expression is
 * legal, false otherwise. If false, the values of the out parameters are undefined.
 */
static bool determine_binary_type(Token::Kind op, std::shared_ptr<Type> left, 
                                  std::shared_ptr<Type> right, 
                                  std::shared_ptr<Type>* outLeftType,
                                  std::shared_ptr<Type>* outRightType,
                                  std::shared_ptr<Type>* outResultType,
                                  bool tryFlipped) {
    bool isLogical;
    switch (op) {
        case Token::EQEQ: // fall through
        case Token::NEQ:  // fall through
        case Token::LT:   // fall through
        case Token::GT:   // fall through
        case Token::LTEQ: // fall through
        case Token::GTEQ:
            isLogical = true;
            break;
        case Token::LOGICALOR: // fall through
        case Token::LOGICALAND: // fall through
        case Token::LOGICALXOR: // fall through
        case Token::LOGICALOREQ: // fall through
        case Token::LOGICALANDEQ: // fall through
        case Token::LOGICALXOREQ:
            *outLeftType = kBool_Type;
            *outRightType = kBool_Type;
            *outResultType = kBool_Type;
            return left->canCoerceTo(kBool_Type) && right->canCoerceTo(kBool_Type);
        case Token::STAR: // fall through
        case Token::STAREQ: 
            // FIXME need to handle non-square matrices
            if (left->kind() == Type::kMatrix_Kind && right->kind() == Type::kVector_Kind) {
                *outLeftType = left;
                *outRightType = right;
                *outResultType = right;
                return left->rows() == right->columns();
            }  
            if (left->kind() == Type::kVector_Kind && right->kind() == Type::kMatrix_Kind) {
                *outLeftType = left;
                *outRightType = right;
                *outResultType = left;
                return left->columns() == right->columns();
            }
            // fall through
        default:
            isLogical = false;
    }
    // FIXME: need to disallow illegal operations like vec3 > vec3. Also do not currently have
    // full support for numbers other than float.
    if (left == right) {
        *outLeftType = left;
        *outRightType = left;
        if (isLogical) {
            *outResultType = kBool_Type;
        } else {
            *outResultType = left;
        }
        return true;
    }
    // FIXME: incorrect for shift operations
    if (left->canCoerceTo(right)) {
        *outLeftType = right;
        *outRightType = right;
        if (isLogical) {
            *outResultType = kBool_Type;
        } else {
            *outResultType = right;
        }
        return true;
    }
    if ((left->kind() == Type::kVector_Kind || left->kind() == Type::kMatrix_Kind) && 
        (right->kind() == Type::kScalar_Kind)) {
        if (determine_binary_type(op, left->componentType(), right, outLeftType, outRightType,
                                  outResultType, false)) {
            *outLeftType = (*outLeftType)->toCompound(left->columns(), left->rows());
            if (!isLogical) {
                *outResultType = (*outResultType)->toCompound(left->columns(), left->rows());
            }
            return true;
        }
        return false;
    }
    if (tryFlipped) {
        return determine_binary_type(op, right, left, outRightType, outLeftType, outResultType, 
                                     false);
    }
    return false;
}

std::unique_ptr<Expression> IRGenerator::convertBinaryExpression(
                                                            const ASTBinaryExpression& expression) {
    std::unique_ptr<Expression> left = this->convertExpression(*expression.fLeft);
    if (!left) {
        return nullptr;
    }
    std::unique_ptr<Expression> right = this->convertExpression(*expression.fRight);
    if (!right) {
        return nullptr;
    }
    std::shared_ptr<Type> leftType;
    std::shared_ptr<Type> rightType;
    std::shared_ptr<Type> resultType;
    if (!determine_binary_type(expression.fOperator, left->fType, right->fType, &leftType,
                               &rightType, &resultType, true)) {
        fErrors.error(expression.fPosition, "type mismatch: '" + 
                                            Token::OperatorName(expression.fOperator) + 
                                            "' cannot operate on '" + left->fType->fName + 
                                            "', '" + right->fType->fName + "'");
        return nullptr;
    }
    switch (expression.fOperator) {
        case Token::EQ:           // fall through
        case Token::PLUSEQ:       // fall through
        case Token::MINUSEQ:      // fall through
        case Token::STAREQ:       // fall through
        case Token::SLASHEQ:      // fall through
        case Token::PERCENTEQ:    // fall through
        case Token::SHLEQ:        // fall through
        case Token::SHREQ:        // fall through
        case Token::BITWISEOREQ:  // fall through
        case Token::BITWISEXOREQ: // fall through
        case Token::BITWISEANDEQ: // fall through
        case Token::LOGICALOREQ:  // fall through
        case Token::LOGICALXOREQ: // fall through
        case Token::LOGICALANDEQ: 
            this->markWrittenTo(*left);
        default:
            break;
    }
    return std::unique_ptr<Expression>(new BinaryExpression(expression.fPosition, 
                                                            this->coerce(std::move(left), leftType), 
                                                            expression.fOperator, 
                                                            this->coerce(std::move(right), 
                                                                         rightType), 
                                                            resultType));
}

std::unique_ptr<Expression> IRGenerator::convertTernaryExpression(  
                                                           const ASTTernaryExpression& expression) {
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*expression.fTest), 
                                                    kBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Expression> ifTrue = this->convertExpression(*expression.fIfTrue);
    if (!ifTrue) {
        return nullptr;
    }
    std::unique_ptr<Expression> ifFalse = this->convertExpression(*expression.fIfFalse);
    if (!ifFalse) {
        return nullptr;
    }
    std::shared_ptr<Type> trueType;
    std::shared_ptr<Type> falseType;
    std::shared_ptr<Type> resultType;
    if (!determine_binary_type(Token::EQEQ, ifTrue->fType, ifFalse->fType, &trueType,
                               &falseType, &resultType, true)) {
        fErrors.error(expression.fPosition, "ternary operator result mismatch: '" + 
                                            ifTrue->fType->fName + "', '" + 
                                            ifFalse->fType->fName + "'");
        return nullptr;
    }
    ASSERT(trueType == falseType);
    ifTrue = this->coerce(std::move(ifTrue), trueType);
    ifFalse = this->coerce(std::move(ifFalse), falseType);
    return std::unique_ptr<Expression>(new TernaryExpression(expression.fPosition, 
                                                             std::move(test),
                                                             std::move(ifTrue), 
                                                             std::move(ifFalse)));
}

std::unique_ptr<Expression> IRGenerator::call(
                                         Position position, 
                                         std::shared_ptr<FunctionDeclaration> function, 
                                         std::vector<std::unique_ptr<Expression>> arguments) {
    if (function->fParameters.size() != arguments.size()) {
        std::string msg = "call to '" + function->fName + "' expected " + 
                                 to_string(function->fParameters.size()) + 
                                 " argument";
        if (function->fParameters.size() != 1) {
            msg += "s";
        }
        msg += ", but found " + to_string(arguments.size());
        fErrors.error(position, msg);
        return nullptr;
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        arguments[i] = this->coerce(std::move(arguments[i]), function->fParameters[i]->fType);
        if (arguments[i] && (function->fParameters[i]->fModifiers.fFlags & Modifiers::kOut_Flag)) {
            this->markWrittenTo(*arguments[i]);
        }
    }
    return std::unique_ptr<FunctionCall>(new FunctionCall(position, std::move(function),
                                                          std::move(arguments)));
}

/**
 * Determines the cost of coercing the arguments of a function to the required types. Returns true 
 * if the cost could be computed, false if the call is not valid. Cost has no particular meaning 
 * other than "lower costs are preferred".
 */
bool IRGenerator::determineCallCost(std::shared_ptr<FunctionDeclaration> function, 
                                    const std::vector<std::unique_ptr<Expression>>& arguments,
                                    int* outCost) {
    if (function->fParameters.size() != arguments.size()) {
        return false;
    }
    int total = 0;
    for (size_t i = 0; i < arguments.size(); i++) {
        int cost;
        if (arguments[i]->fType->determineCoercionCost(function->fParameters[i]->fType, &cost)) {
            total += cost;
        } else {
            return false;
        }
    }
    *outCost = total;
    return true;
}

std::unique_ptr<Expression> IRGenerator::call(Position position, 
                                              std::unique_ptr<Expression> functionValue, 
                                              std::vector<std::unique_ptr<Expression>> arguments) {
    if (functionValue->fKind == Expression::kTypeReference_Kind) {
        return this->convertConstructor(position, 
                                        ((TypeReference&) *functionValue).fValue, 
                                        std::move(arguments));
    }
    if (functionValue->fKind != Expression::kFunctionReference_Kind) {
        fErrors.error(position, "'" + functionValue->description() + "' is not a function");
        return nullptr;
    }
    FunctionReference* ref = (FunctionReference*) functionValue.get();
    int bestCost = INT_MAX;
    std::shared_ptr<FunctionDeclaration> best;
    if (ref->fFunctions.size() > 1) {
        for (const auto& f : ref->fFunctions) {
            int cost;
            if (this->determineCallCost(f, arguments, &cost) && cost < bestCost) {
                bestCost = cost;
                best = f;
            }
        }
        if (best) {
            return this->call(position, std::move(best), std::move(arguments));
        }
        std::string msg = "no match for " + ref->fFunctions[0]->fName + "(";
        std::string separator = "";
        for (size_t i = 0; i < arguments.size(); i++) {
            msg += separator;
            separator = ", ";
            msg += arguments[i]->fType->description();
        }
        msg += ")";
        fErrors.error(position, msg);
        return nullptr;
    }
    return this->call(position, ref->fFunctions[0], std::move(arguments));
}

std::unique_ptr<Expression> IRGenerator::convertConstructor(
                                                    Position position, 
                                                    std::shared_ptr<Type> type, 
                                                    std::vector<std::unique_ptr<Expression>> args) {
    // FIXME: add support for structs and arrays
    Type::Kind kind = type->kind();
    if (!type->isNumber() && kind != Type::kVector_Kind && kind != Type::kMatrix_Kind) {
        fErrors.error(position, "cannot construct '" + type->description() + "'");
        return nullptr;
    }
    if (type == kFloat_Type && args.size() == 1 && 
        args[0]->fKind == Expression::kIntLiteral_Kind) {
        int64_t value = ((IntLiteral&) *args[0]).fValue;
        return std::unique_ptr<Expression>(new FloatLiteral(position, (double) value));
    }
    if (args.size() == 1 && args[0]->fType == type) {
        // argument is already the right type, just return it
        return std::move(args[0]);
    }
    if (type->isNumber()) {
        if (args.size() != 1) {
            fErrors.error(position, "invalid arguments to '" + type->description() + 
                                    "' constructor, (expected exactly 1 argument, but found " +
                                    to_string(args.size()) + ")");
        }
        if (args[0]->fType == kBool_Type) {
            std::unique_ptr<IntLiteral> zero(new IntLiteral(position, 0));
            std::unique_ptr<IntLiteral> one(new IntLiteral(position, 1));
            return std::unique_ptr<Expression>(
                                         new TernaryExpression(position, std::move(args[0]),
                                                               this->coerce(std::move(one), type),
                                                               this->coerce(std::move(zero), 
                                                                            type)));
        } else if (!args[0]->fType->isNumber()) {
            fErrors.error(position, "invalid argument to '" + type->description() + 
                                    "' constructor (expected a number or bool, but found '" +
                                    args[0]->fType->description() + "')");
        }
    } else {
        ASSERT(kind == Type::kVector_Kind || kind == Type::kMatrix_Kind);
        int actual = 0;
        for (size_t i = 0; i < args.size(); i++) {
            if (args[i]->fType->kind() == Type::kVector_Kind || 
                args[i]->fType->kind() == Type::kMatrix_Kind) {
                int columns = args[i]->fType->columns();
                int rows = args[i]->fType->rows();
                args[i] = this->coerce(std::move(args[i]), 
                                       type->componentType()->toCompound(columns, rows));
                actual += args[i]->fType->rows() * args[i]->fType->columns();
            } else if (args[i]->fType->kind() == Type::kScalar_Kind) {
                actual += 1;
                if (type->kind() != Type::kScalar_Kind) {
                    args[i] = this->coerce(std::move(args[i]), type->componentType());
                }
            } else {
                fErrors.error(position, "'" + args[i]->fType->description() + "' is not a valid "
                                        "parameter to '" + type->description() + "' constructor");
                return nullptr;
            }
        }
        int min = type->rows() * type->columns();
        int max = type->columns() > 1 ? INT_MAX : min;
        if ((actual < min || actual > max) &&
            !((kind == Type::kVector_Kind || kind == Type::kMatrix_Kind) && (actual == 1))) {
            fErrors.error(position, "invalid arguments to '" + type->description() + 
                                    "' constructor (expected " + to_string(min) + " scalar" + 
                                    (min == 1 ? "" : "s") + ", but found " + to_string(actual) + 
                                    ")");
            return nullptr;
        }
    }
    return std::unique_ptr<Expression>(new Constructor(position, std::move(type), std::move(args)));
}

std::unique_ptr<Expression> IRGenerator::convertPrefixExpression(
                                                            const ASTPrefixExpression& expression) {
    std::unique_ptr<Expression> base = this->convertExpression(*expression.fOperand);
    if (!base) {
        return nullptr;
    }
    switch (expression.fOperator) {
        case Token::PLUS:
            if (!base->fType->isNumber() && base->fType->kind() != Type::kVector_Kind) {
                fErrors.error(expression.fPosition, 
                              "'+' cannot operate on '" + base->fType->description() + "'");
                return nullptr;
            }
            return base;
        case Token::MINUS:
            if (!base->fType->isNumber() && base->fType->kind() != Type::kVector_Kind) {
                fErrors.error(expression.fPosition, 
                              "'-' cannot operate on '" + base->fType->description() + "'");
                return nullptr;
            }
            if (base->fKind == Expression::kIntLiteral_Kind) {
                return std::unique_ptr<Expression>(new IntLiteral(base->fPosition,
                                                                  -((IntLiteral&) *base).fValue));
            }
            if (base->fKind == Expression::kFloatLiteral_Kind) {
                double value = -((FloatLiteral&) *base).fValue;
                return std::unique_ptr<Expression>(new FloatLiteral(base->fPosition, value));
            }
            return std::unique_ptr<Expression>(new PrefixExpression(Token::MINUS, std::move(base)));
        case Token::PLUSPLUS:
            if (!base->fType->isNumber()) {
                fErrors.error(expression.fPosition, 
                              "'" + Token::OperatorName(expression.fOperator) + 
                              "' cannot operate on '" + base->fType->description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base);
            break;
        case Token::MINUSMINUS: 
            if (!base->fType->isNumber()) {
                fErrors.error(expression.fPosition, 
                              "'" + Token::OperatorName(expression.fOperator) + 
                              "' cannot operate on '" + base->fType->description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base);
            break;
        case Token::NOT:
            if (base->fType != kBool_Type) {
                fErrors.error(expression.fPosition, 
                              "'" + Token::OperatorName(expression.fOperator) + 
                              "' cannot operate on '" + base->fType->description() + "'");
                return nullptr;
            }
            break;
        default: 
            ABORT("unsupported prefix operator\n");
    }
    return std::unique_ptr<Expression>(new PrefixExpression(expression.fOperator, 
                                                            std::move(base)));
}

std::unique_ptr<Expression> IRGenerator::convertIndex(std::unique_ptr<Expression> base,
                                                      const ASTExpression& index) {
    if (base->fType->kind() != Type::kArray_Kind && base->fType->kind() != Type::kMatrix_Kind) {
        fErrors.error(base->fPosition, "expected array, but found '" + base->fType->description() + 
                                       "'");
        return nullptr;
    }
    std::unique_ptr<Expression> converted = this->convertExpression(index);
    if (!converted) {
        return nullptr;
    }
    converted = this->coerce(std::move(converted), kInt_Type);
    if (!converted) {
        return nullptr;
    }
    return std::unique_ptr<Expression>(new IndexExpression(std::move(base), std::move(converted)));
}

std::unique_ptr<Expression> IRGenerator::convertField(std::unique_ptr<Expression> base,
                                                      const std::string& field) {
    auto fields = base->fType->fields();
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].fName == field) {
            return std::unique_ptr<Expression>(new FieldAccess(std::move(base), (int) i));
        }
    }
    fErrors.error(base->fPosition, "type '" + base->fType->description() + "' does not have a "
                                   "field named '" + field + "");
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertSwizzle(std::unique_ptr<Expression> base,
                                                        const std::string& fields) {
    if (base->fType->kind() != Type::kVector_Kind) {
        fErrors.error(base->fPosition, "cannot swizzle type '" + base->fType->description() + "'");
        return nullptr;
    }
    std::vector<int> swizzleComponents;
    for (char c : fields) {
        switch (c) {
            case 'x': // fall through
            case 'r': // fall through
            case 's': 
                swizzleComponents.push_back(0);
                break;
            case 'y': // fall through
            case 'g': // fall through
            case 't':
                if (base->fType->columns() >= 2) {
                    swizzleComponents.push_back(1);
                    break;
                }
                // fall through
            case 'z': // fall through
            case 'b': // fall through
            case 'p': 
                if (base->fType->columns() >= 3) {
                    swizzleComponents.push_back(2);
                    break;
                }
                // fall through
            case 'w': // fall through
            case 'a': // fall through
            case 'q':
                if (base->fType->columns() >= 4) {
                    swizzleComponents.push_back(3);
                    break;
                }
                // fall through
            default:
                fErrors.error(base->fPosition, "invalid swizzle component '" + std::string(1, c) +
                                               "'");
                return nullptr;
        }
    }
    ASSERT(swizzleComponents.size() > 0);
    if (swizzleComponents.size() > 4) {
        fErrors.error(base->fPosition, "too many components in swizzle mask '" + fields + "'");
        return nullptr;
    }
    return std::unique_ptr<Expression>(new Swizzle(std::move(base), swizzleComponents));
}

std::unique_ptr<Expression> IRGenerator::convertSuffixExpression(
                                                            const ASTSuffixExpression& expression) {
    std::unique_ptr<Expression> base = this->convertExpression(*expression.fBase);
    if (!base) {
        return nullptr;
    }
    switch (expression.fSuffix->fKind) {
        case ASTSuffix::kIndex_Kind:
            return this->convertIndex(std::move(base), 
                                      *((ASTIndexSuffix&) *expression.fSuffix).fExpression);
        case ASTSuffix::kCall_Kind: {
            auto rawArguments = &((ASTCallSuffix&) *expression.fSuffix).fArguments;
            std::vector<std::unique_ptr<Expression>> arguments;
            for (size_t i = 0; i < rawArguments->size(); i++) {
                std::unique_ptr<Expression> converted = 
                        this->convertExpression(*(*rawArguments)[i]);
                if (!converted) {
                    return nullptr;
                }
                arguments.push_back(std::move(converted));
            }
            return this->call(expression.fPosition, std::move(base), std::move(arguments));
        }
        case ASTSuffix::kField_Kind: {
            switch (base->fType->kind()) {
                case Type::kVector_Kind:
                    return this->convertSwizzle(std::move(base), 
                                                ((ASTFieldSuffix&) *expression.fSuffix).fField);
                case Type::kStruct_Kind:
                    return this->convertField(std::move(base),
                                              ((ASTFieldSuffix&) *expression.fSuffix).fField);
                default:
                    fErrors.error(base->fPosition, "cannot swizzle value of type '" + 
                                                   base->fType->description() + "'");
                    return nullptr;
            }
        }
        case ASTSuffix::kPostIncrement_Kind:
            if (!base->fType->isNumber()) {
                fErrors.error(expression.fPosition, 
                              "'++' cannot operate on '" + base->fType->description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base);
            return std::unique_ptr<Expression>(new PostfixExpression(std::move(base), 
                                                                     Token::PLUSPLUS));
        case ASTSuffix::kPostDecrement_Kind:
            if (!base->fType->isNumber()) {
                fErrors.error(expression.fPosition, 
                              "'--' cannot operate on '" + base->fType->description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base);
            return std::unique_ptr<Expression>(new PostfixExpression(std::move(base), 
                                                                     Token::MINUSMINUS));
        default:
            ABORT("unsupported suffix operator");
    }
}

void IRGenerator::checkValid(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kFunctionReference_Kind:
            fErrors.error(expr.fPosition, "expected '(' to begin function call");
            break;
        case Expression::kTypeReference_Kind:
            fErrors.error(expr.fPosition, "expected '(' to begin constructor invocation");
            break;
        default:
            ASSERT(expr.fType != kInvalid_Type);
            break;
    }
}

void IRGenerator::markReadFrom(std::shared_ptr<Variable> var) {
    var->fIsReadFrom = true;
}

static bool has_duplicates(const Swizzle& swizzle) {
    int bits = 0;
    for (int idx : swizzle.fComponents) {
        ASSERT(idx >= 0 && idx <= 3);
        int bit = 1 << idx;
        if (bits & bit) {
            return true;
        }
        bits |= bit;
    }
    return false;
}

void IRGenerator::markWrittenTo(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = *((VariableReference&) expr).fVariable;
            if (var.fModifiers.fFlags & (Modifiers::kConst_Flag | Modifiers::kUniform_Flag)) {
                fErrors.error(expr.fPosition, 
                              "cannot modify immutable variable '" + var.fName + "'");
            }
            var.fIsWrittenTo = true;
            break;
        }
        case Expression::kFieldAccess_Kind:
            this->markWrittenTo(*((FieldAccess&) expr).fBase);
            break;
        case Expression::kSwizzle_Kind:
            if (has_duplicates((Swizzle&) expr)) {
                fErrors.error(expr.fPosition, 
                              "cannot write to the same swizzle field more than once");
            }
            this->markWrittenTo(*((Swizzle&) expr).fBase);
            break;
        case Expression::kIndex_Kind:
            this->markWrittenTo(*((IndexExpression&) expr).fBase);
            break;
        default:
            fErrors.error(expr.fPosition, "cannot assign to '" + expr.description() + "'");
            break;
    }
}

}
