/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/Test.h"

#ifdef SK_SUPPORT_PDF

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkStream.h"
#include "include/docs/SkPDFDocument.h"

using PDFTag = SkPDF::StructureElementNode;

// Test building a tagged PDF containing a table.
// Add this to args.gn to output the PDF to a file:
//   extra_cflags = [ "-DSK_PDF_TEST_TAGS_OUTPUT_PATH=\"/tmp/table.pdf\"" ]
DEF_TEST(SkPDF_tagged_table, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_tagged, r);
#ifdef SK_PDF_TEST_TAGS_OUTPUT_PATH
    SkFILEWStream outputStream(SK_PDF_TEST_TAGS_OUTPUT_PATH);
#else
    SkDynamicMemoryWStream outputStream;
#endif

    SkSize pageSize = SkSize::Make(612, 792);  // U.S. Letter

    SkPDF::Metadata metadata;
    metadata.fTitle = "Example Tagged Table PDF";
    metadata.fCreator = "Skia";
    SkTime::DateTime now;
    SkTime::GetDateTime(&now);
    metadata.fCreation = now;
    metadata.fModified = now;

    constexpr int kRowCount = 5;
    constexpr int kColCount = 4;
    const char* cellData[kRowCount * kColCount] = {
        "Car",                  "Engine",   "City MPG", "Highway MPG",
        "Mitsubishi Mirage ES", "Gas",      "28",       "47",
        "Toyota Prius Three",   "Hybrid",   "43",       "59",
        "Nissan Leaf SL",       "Electric", "N/A",      nullptr,
        "Tesla Model 3",        nullptr,    "N/A",      nullptr
    };

    // The document tag.
    PDFTag root;
    root.fNodeId = 1;
    root.fType = SkPDF::DocumentStructureType::kDocument;
    root.fChildCount = 2;
    PDFTag rootChildren[2];

    // Heading.
    PDFTag& h1 = rootChildren[0];
    h1.fNodeId = 2;
    h1.fType = SkPDF::DocumentStructureType::kH1;
    h1.fChildCount = 0;

    // Table.
    PDFTag& table = rootChildren[1];
    table.fNodeId = 3;
    table.fType = SkPDF::DocumentStructureType::kTable;
    table.fChildCount = 5;
    table.fAttributes.appendFloatArray("Layout", "BBox", {72, 72, 360, 360});

    PDFTag rows[kRowCount];
    PDFTag all_cells[kRowCount * kColCount];
    for (int rowIndex = 0; rowIndex < kRowCount; rowIndex++) {
        PDFTag& row = rows[rowIndex];
        row.fNodeId = 4 + rowIndex;
        row.fType = SkPDF::DocumentStructureType::kTR;
        row.fChildCount = kColCount;
        PDFTag* cells = &all_cells[rowIndex * kColCount];

        for (int colIndex = 0; colIndex < kColCount; colIndex++) {
            int cellIndex = rowIndex * kColCount + colIndex;
            PDFTag& cell = cells[colIndex];
            cell.fNodeId = 10 + cellIndex;
            if (!cellData[cellIndex])
                cell.fType = SkPDF::DocumentStructureType::kNonStruct;
            else if (rowIndex == 0 || colIndex == 0)
                cell.fType = SkPDF::DocumentStructureType::kTH;
            else
                cell.fType = SkPDF::DocumentStructureType::kTD;
            cell.fChildCount = 0;

            if (cellIndex == 13) {
                cell.fAttributes.appendInt("Table", "RowSpan", 2);
            } else if (cellIndex == 14 || cellIndex == 18) {
                cell.fAttributes.appendInt("Table", "ColSpan", 2);
            } else if (cell.fType == SkPDF::DocumentStructureType::kTH) {
                cell.fAttributes.appendString(
                    "Table", "Scope", rowIndex == 0 ? "Column" : "Row");
            }
        }
        row.fChildren = cells;
    }
    table.fChildren = rows;
    root.fChildren = rootChildren;

    metadata.fStructureElementTreeRoot = &root;
    sk_sp<SkDocument> document = SkPDF::MakeDocument(
        &outputStream, metadata);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    SkCanvas* canvas =
            document->beginPage(pageSize.width(),
                                pageSize.height());
    SkPDF::SetNodeId(canvas, 2);
    SkFont font(nullptr, 36);
    canvas->drawString("Tagged PDF Table", 72, 72, font, paint);

    font.setSize(14);
    for (int rowIndex = 0; rowIndex < kRowCount; rowIndex++) {
        for (int colIndex = 0; colIndex < kColCount; colIndex++) {
            int cellIndex = rowIndex * kColCount + colIndex;
            const char* str = cellData[cellIndex];
            if (!str)
                continue;

            int x = 72 + colIndex * 108 + (colIndex > 0 ? 72 : 0);
            int y = 144 + rowIndex * 48;

            SkPDF::SetNodeId(canvas, 10 + cellIndex);
            canvas->drawString(str, x, y, font, paint);
        }
    }

    document->endPage();
    document->close();
    outputStream.flush();
}

#endif
