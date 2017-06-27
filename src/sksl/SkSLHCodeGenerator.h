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

namespace SkSL {

class HCodeGenerator : public CodeGenerator {
public:
    HCodeGenerator(const Program* program, ErrorReporter* errors, String name, OutputStream* out);

    bool generateCode() override;

    static String ParameterType(const Type& type);

    static String FieldType(const Type& type);

    static String FieldName(const char* varName) {
        return String::printf("f%c%s", toupper(varName[0]), varName + 1);
    }

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

    String fName;
    String fFullName;
    SectionAndParameterHelper fSectionAndParameterHelper;

    typedef CodeGenerator INHERITED;
};

} // namespace SkSL

#endif
