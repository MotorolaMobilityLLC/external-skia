/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLHCodeGenerator.h"

#include "include/private/SkSLSampleUsage.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLSection.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <set>

#if defined(SKSL_STANDALONE) || defined(GR_TEST_UTILS)

namespace SkSL {

HCodeGenerator::HCodeGenerator(const Context* context, const Program* program,
                               ErrorReporter* errors, String name, OutputStream* out)
: INHERITED(program, errors, out)
, fContext(*context)
, fName(std::move(name))
, fFullName(String::printf("Gr%s", fName.c_str()))
, fSectionAndParameterHelper(program, *errors) {}

String HCodeGenerator::ParameterType(const Context& context, const Type& type,
                                     const Layout& layout) {
    Layout::CType ctype = ParameterCType(context, type, layout);
    if (ctype != Layout::CType::kDefault) {
        return Layout::CTypeToStr(ctype);
    }
    return type.name();
}

Layout::CType HCodeGenerator::ParameterCType(const Context& context, const Type& type,
                                     const Layout& layout) {
    if (layout.fCType != Layout::CType::kDefault) {
        return layout.fCType;
    }
    if (type.kind() == Type::kNullable_Kind) {
        return ParameterCType(context, type.componentType(), layout);
    } else if (type == *context.fFloat_Type || type == *context.fHalf_Type) {
        return Layout::CType::kFloat;
    } else if (type == *context.fInt_Type ||
               type == *context.fShort_Type ||
               type == *context.fByte_Type) {
        return Layout::CType::kInt32;
    } else if (type == *context.fFloat2_Type || type == *context.fHalf2_Type) {
        return Layout::CType::kSkPoint;
    } else if (type == *context.fInt2_Type ||
               type == *context.fShort2_Type ||
               type == *context.fByte2_Type) {
        return Layout::CType::kSkIPoint;
    } else if (type == *context.fInt4_Type ||
               type == *context.fShort4_Type ||
               type == *context.fByte4_Type) {
        return Layout::CType::kSkIRect;
    } else if (type == *context.fFloat4_Type || type == *context.fHalf4_Type) {
        return Layout::CType::kSkRect;
    } else if (type == *context.fFloat3x3_Type || type == *context.fHalf3x3_Type) {
        return Layout::CType::kSkMatrix;
    } else if (type == *context.fFloat4x4_Type || type == *context.fHalf4x4_Type) {
        return Layout::CType::kSkM44;
    } else if (type.kind() == Type::kSampler_Kind) {
        return Layout::CType::kGrSurfaceProxyView;
    } else if (type == *context.fFragmentProcessor_Type) {
        return Layout::CType::kGrFragmentProcessor;
    }
    return Layout::CType::kDefault;
}

String HCodeGenerator::FieldType(const Context& context, const Type& type,
                                 const Layout& layout) {
    if (type.kind() == Type::kSampler_Kind) {
        return "TextureSampler";
    } else if (type == *context.fFragmentProcessor_Type) {
        // we don't store fragment processors in fields, they get registered via
        // registerChildProcessor instead
        SkASSERT(false);
        return "<error>";
    }
    return ParameterType(context, type, layout);
}

String HCodeGenerator::AccessType(const Context& context, const Type& type,
                                  const Layout& layout) {
    static const std::set<String> primitiveTypes = { "int32_t", "float", "bool", "SkPMColor" };

    String fieldType = FieldType(context, type, layout);
    bool isPrimitive = primitiveTypes.find(fieldType) != primitiveTypes.end();
    if (isPrimitive) {
        return fieldType;
    } else {
        return String::printf("const %s&", fieldType.c_str());
    }
}

void HCodeGenerator::writef(const char* s, va_list va) {
    static constexpr int BUFFER_SIZE = 1024;
    va_list copy;
    va_copy(copy, va);
    char buffer[BUFFER_SIZE];
    int length = vsnprintf(buffer, BUFFER_SIZE, s, va);
    if (length < BUFFER_SIZE) {
        fOut->write(buffer, length);
    } else {
        std::unique_ptr<char[]> heap(new char[length + 1]);
        vsprintf(heap.get(), s, copy);
        fOut->write(heap.get(), length);
    }
    va_end(copy);
}

void HCodeGenerator::writef(const char* s, ...) {
    va_list va;
    va_start(va, s);
    this->writef(s, va);
    va_end(va);
}

bool HCodeGenerator::writeSection(const char* name, const char* prefix) {
    const Section* s = fSectionAndParameterHelper.getSection(name);
    if (s) {
        this->writef("%s%s", prefix, s->fText.c_str());
        return true;
    }
    return false;
}

void HCodeGenerator::writeExtraConstructorParams(const char* separator) {
    // super-simple parse, just assume the last token before a comma is the name of a parameter
    // (which is true as long as there are no multi-parameter template types involved). Will replace
    // this with something more robust if the need arises.
    const Section* section = fSectionAndParameterHelper.getSection(kConstructorParamsSection);
    if (section) {
        const char* s = section->fText.c_str();
        #define BUFFER_SIZE 64
        char lastIdentifier[BUFFER_SIZE];
        int lastIdentifierLength = 0;
        bool foundBreak = false;
        while (*s) {
            char c = *s;
            ++s;
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                c == '_') {
                if (foundBreak) {
                    lastIdentifierLength = 0;
                    foundBreak = false;
                }
                SkASSERT(lastIdentifierLength < BUFFER_SIZE);
                lastIdentifier[lastIdentifierLength] = c;
                ++lastIdentifierLength;
            } else {
                foundBreak = true;
                if (c == ',') {
                    SkASSERT(lastIdentifierLength < BUFFER_SIZE);
                    lastIdentifier[lastIdentifierLength] = 0;
                    this->writef("%s%s", separator, lastIdentifier);
                    separator = ", ";
                } else if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                    lastIdentifierLength = 0;
                }
            }
        }
        if (lastIdentifierLength) {
            SkASSERT(lastIdentifierLength < BUFFER_SIZE);
            lastIdentifier[lastIdentifierLength] = 0;
            this->writef("%s%s", separator, lastIdentifier);
        }
    }
}

void HCodeGenerator::writeMake() {
    const char* separator;
    if (!this->writeSection(kMakeSection)) {
        this->writef("    static std::unique_ptr<GrFragmentProcessor> Make(");
        separator = "";
        for (const auto& param : fSectionAndParameterHelper.getParameters()) {
            this->writef("%s%s %s", separator, ParameterType(fContext, param->fType,
                                                             param->fModifiers.fLayout).c_str(),
                         String(param->fName).c_str());
            separator = ", ";
        }
        this->writeSection(kConstructorParamsSection, separator);
        this->writef(") {\n"
                     "        return std::unique_ptr<GrFragmentProcessor>(new %s(",
                     fFullName.c_str());
        separator = "";
        for (const auto& param : fSectionAndParameterHelper.getParameters()) {
            if (param->fType.nonnullable() == *fContext.fFragmentProcessor_Type ||
                param->fType.nonnullable().kind() == Type::kSampler_Kind) {
                this->writef("%sstd::move(%s)", separator, String(param->fName).c_str());
            } else {
                this->writef("%s%s", separator, String(param->fName).c_str());
            }
            separator = ", ";
        }
        this->writeExtraConstructorParams(separator);
        this->writef("));\n"
                     "    }\n");
    }
}

void HCodeGenerator::failOnSection(const char* section, const char* msg) {
    std::vector<const Section*> s = fSectionAndParameterHelper.getSections(section);
    if (s.size()) {
        fErrors.error(s[0]->fOffset, String("@") + section + " " + msg);
    }
}

void HCodeGenerator::writeConstructor() {
    if (this->writeSection(kConstructorSection)) {
        const char* msg = "may not be present when constructor is overridden";
        this->failOnSection(kConstructorCodeSection, msg);
        this->failOnSection(kConstructorParamsSection, msg);
        this->failOnSection(kInitializersSection, msg);
        this->failOnSection(kOptimizationFlagsSection, msg);
        return;
    }
    this->writef("    %s(", fFullName.c_str());
    const char* separator = "";
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        this->writef("%s%s %s", separator, ParameterType(fContext, param->fType,
                                                         param->fModifiers.fLayout).c_str(),
                     String(param->fName).c_str());
        separator = ", ";
    }
    this->writeSection(kConstructorParamsSection, separator);
    this->writef(")\n"
                 "    : INHERITED(k%s_ClassID", fFullName.c_str());
    if (!this->writeSection(kOptimizationFlagsSection, ", (OptimizationFlags) ")) {
        this->writef(", kNone_OptimizationFlags");
    }
    this->writef(")");
    this->writeSection(kInitializersSection, "\n    , ");
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        String nameString(param->fName);
        const char* name = nameString.c_str();
        const Type& type = param->fType.nonnullable();
        if (type.kind() == Type::kSampler_Kind) {
            this->writef("\n    , %s(std::move(%s)", FieldName(name).c_str(), name);
            for (const Section* s : fSectionAndParameterHelper.getSections(
                                                                          kSamplerParamsSection)) {
                if (s->fArgument == name) {
                    this->writef(", %s", s->fText.c_str());
                }
            }
            this->writef(")");
        } else if (type == *fContext.fFragmentProcessor_Type) {
            // do nothing
        } else {
            this->writef("\n    , %s(%s)", FieldName(name).c_str(), name);
        }
    }
    this->writef(" {\n");
    this->writeSection(kConstructorCodeSection);

    if (Analysis::ReferencesSampleCoords(fProgram)) {
        this->writef("        this->setUsesSampleCoordsDirectly();\n");
    }

    int samplerCount = 0;
    for (const Variable* param : fSectionAndParameterHelper.getParameters()) {
        if (param->fType.kind() == Type::kSampler_Kind) {
            ++samplerCount;
        } else if (param->fType.nonnullable() == *fContext.fFragmentProcessor_Type) {
            if (param->fType.kind() != Type::kNullable_Kind) {
                this->writef("        SkASSERT(%s);", String(param->fName).c_str());
            }

            SampleUsage usage = Analysis::GetSampleUsage(fProgram, *param);

            std::string perspExpression;
            if (usage.hasUniformMatrix()) {
                for (const Variable* p : fSectionAndParameterHelper.getParameters()) {
                    if ((p->fModifiers.fFlags & Modifiers::kIn_Flag) &&
                        usage.fExpression == String(p->fName)) {
                        perspExpression = usage.fExpression + ".hasPerspective()";
                        break;
                    }
                }
            }
            std::string usageArg = usage.constructor(std::move(perspExpression));

            this->writef("        this->registerChild(std::move(%s), %s);",
                         String(param->fName).c_str(),
                         usageArg.c_str());
        }
    }
    if (samplerCount) {
        this->writef("        this->setTextureSamplerCnt(%d);", samplerCount);
    }
    this->writef("    }\n");
}

void HCodeGenerator::writeFields() {
    this->writeSection(kFieldsSection);
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        String name = FieldName(String(param->fName).c_str());
        if (param->fType.nonnullable() == *fContext.fFragmentProcessor_Type) {
            // Don't need to write any fields, FPs are held as children
        } else {
            this->writef("    %s %s;\n", FieldType(fContext, param->fType,
                                                   param->fModifiers.fLayout).c_str(),
                                         name.c_str());
        }
    }
}

String HCodeGenerator::GetHeader(const Program& program, ErrorReporter& errors) {
    SymbolTable types(&errors);
    Parser parser(program.fSource->c_str(), program.fSource->length(), types, errors);
    for (;;) {
        Token header = parser.nextRawToken();
        switch (header.fKind) {
            case Token::Kind::TK_WHITESPACE:
                break;
            case Token::Kind::TK_BLOCK_COMMENT:
                return String(program.fSource->c_str() + header.fOffset, header.fLength);
            default:
                return "";
        }
    }
}

bool HCodeGenerator::generateCode() {
    this->writef("%s\n", GetHeader(fProgram, fErrors).c_str());
    this->writef(kFragmentProcessorHeader, fFullName.c_str());
    this->writef("#ifndef %s_DEFINED\n"
                 "#define %s_DEFINED\n"
                 "\n"
                 "#include \"include/core/SkM44.h\"\n"
                 "#include \"include/core/SkTypes.h\"\n"
                 "\n",
                 fFullName.c_str(),
                 fFullName.c_str());
    this->writeSection(kHeaderSection);
    this->writef("\n"
                 "#include \"src/gpu/GrFragmentProcessor.h\"\n"
                 "\n"
                 "class %s : public GrFragmentProcessor {\n"
                 "public:\n",
                 fFullName.c_str());
    for (const auto& p : fProgram) {
        if (ProgramElement::kEnum_Kind == p.fKind && !((Enum&) p).fBuiltin) {
            this->writef("%s\n", ((Enum&) p).code().c_str());
        }
    }
    this->writeSection(kClassSection);
    this->writeMake();
    this->writef("    %s(const %s& src);\n"
                 "    std::unique_ptr<GrFragmentProcessor> clone() const override;\n"
                 "    const char* name() const override { return \"%s\"; }\n",
                 fFullName.c_str(), fFullName.c_str(), fName.c_str());
    this->writeFields();
    this->writef("private:\n");
    this->writeConstructor();
    this->writef("    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;\n"
                 "    void onGetGLSLProcessorKey(const GrShaderCaps&, "
                                                "GrProcessorKeyBuilder*) const override;\n"
                 "    bool onIsEqual(const GrFragmentProcessor&) const override;\n");
    for (const auto& param : fSectionAndParameterHelper.getParameters()) {
        if (param->fType.kind() == Type::kSampler_Kind) {
            this->writef("    const TextureSampler& onTextureSampler(int) const override;");
            break;
        }
    }
    this->writef("#if GR_TEST_UTILS\n"
                 "    SkString onDumpInfo() const override;\n"
                 "#endif\n"
                 "    GR_DECLARE_FRAGMENT_PROCESSOR_TEST\n"
                 "    typedef GrFragmentProcessor INHERITED;\n"
                 "};\n");
    this->writeSection(kHeaderEndSection);
    this->writef("#endif\n");
    return 0 == fErrors.errorCount();
}

}  // namespace SkSL

#endif // defined(SKSL_STANDALONE) || defined(GR_TEST_UTILS)
