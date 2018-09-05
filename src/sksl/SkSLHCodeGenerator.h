/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_HCODEGENERATOR
#define SKSL_HCODEGENERATOR

#include "SkSLCodeGenerator.h"
#include "SkSLSectionAndParameterHelper.h"
#include "ir/SkSLType.h"
#include "ir/SkSLVariable.h"

#include <cctype>

constexpr const char* kFragmentProcessorHeader =
R"(
/**************************************************************************************************
 *** This file was autogenerated from %s.fp; do not modify.
 **************************************************************************************************/
)";

namespace SkSL {

class HCodeGenerator : public CodeGenerator {
public:
    HCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                   String name, OutputStream* out);

    bool generateCode() override;

    static String ParameterType(const Context& context, const Type& type, const Layout& layout);

    static Layout::CType ParameterCType(const Context& context, const Type& type,
                                        const Layout& layout);

    static String FieldType(const Context& context, const Type& type, const Layout& layout);

    // Either the field type, or a const reference of the field type if the field type is complex.
    static String AccessType(const Context& context, const Type& type, const Layout& layout);

    static String FieldName(const char* varName) {
        return String::printf("f%c%s", toupper(varName[0]), varName + 1);
    }

    static String CoordTransformName(const String& arg, int index) {
        if (arg.size()) {
            return HCodeGenerator::FieldName(arg.c_str()) + "CoordTransform";
        }
        return "fCoordTransform" + to_string(index);
    }

    static String GetHeader(const Program& program, ErrorReporter& errors);

private:
    void writef(const char* s, va_list va) SKSL_PRINTF_LIKE(2, 0);

    void writef(const char* s, ...) SKSL_PRINTF_LIKE(2, 3);

    bool writeSection(const char* name, const char* prefix = "");

    // given a @constructorParams section of e.g. 'int x, float y', writes out "<separator>x, y".
    // Writes nothing (not even the separator) if there is no @constructorParams section.
    void writeExtraConstructorParams(const char* separator);

    void writeMake();

    void writeConstructor();

    void writeFields();

    void failOnSection(const char* section, const char* msg);

    const Context& fContext;
    String fName;
    String fFullName;
    SectionAndParameterHelper fSectionAndParameterHelper;

    typedef CodeGenerator INHERITED;
};

} // namespace SkSL

#endif
