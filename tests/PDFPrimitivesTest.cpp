/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDocument.h"
#include "SkDeflate.h"
#include "SkImageEncoder.h"
#include "SkMatrix.h"
#include "SkPDFCanon.h"
#include "SkPDFDevice.h"
#include "SkPDFFont.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkReadBuffer.h"
#include "SkScalar.h"
#include "SkSpecialImage.h"
#include "SkStream.h"
#include "SkTypes.h"
#include "Test.h"
#include "sk_tool_utils.h"

#define DUMMY_TEXT "DCT compessed stream."

namespace {
struct Catalog {
    SkPDFSubstituteMap substitutes;
    SkPDFObjNumMap numbers;
};
}  // namespace

template <typename T>
static SkString emit_to_string(T& obj, Catalog* catPtr = nullptr) {
    Catalog catalog;
    SkDynamicMemoryWStream buffer;
    if (!catPtr) {
        catPtr = &catalog;
    }
    obj.emitObject(&buffer, catPtr->numbers, catPtr->substitutes);
    SkAutoTDelete<SkStreamAsset> asset(buffer.detachAsStream());
    SkString tmp(asset->getLength());
    asset->read(tmp.writable_str(), asset->getLength());
    return tmp;
}

static bool eq(const SkString& str, const char* strPtr, size_t len) {
    return len == str.size() && 0 == memcmp(str.c_str(), strPtr, len);
}

#define ASSERT_EQL(REPORTER, SKSTRING, STRING, LEN)                     \
    do {                                                                \
        const char* strptr = STRING;                                    \
        const SkString& sks = SKSTRING;                                 \
        if (!eq(sks, strptr, LEN)) {                                    \
            REPORT_FAILURE(                                             \
                    REPORTER,                                           \
                    "",                                                 \
                    SkStringPrintf("'%s' != '%s'", strptr, sks.c_str()));  \
        }                                                               \
    } while (false)

#define ASSERT_EQ(REPORTER, SKSTRING, STRING)             \
    do {                                                  \
        const char* str = STRING;                         \
        ASSERT_EQL(REPORTER, SKSTRING, str, strlen(str)); \
    } while (false)

#define ASSERT_EMIT_EQ(REPORTER, OBJECT, STRING)          \
    do {                                                  \
        SkString result = emit_to_string(OBJECT);         \
        ASSERT_EQ(REPORTER, result, STRING);              \
    } while (false)



static void TestPDFStream(skiatest::Reporter* reporter) {
    char streamBytes[] = "Test\nFoo\tBar";
    SkAutoTDelete<SkMemoryStream> streamData(new SkMemoryStream(
        streamBytes, strlen(streamBytes), true));
    sk_sp<SkPDFStream> stream(new SkPDFStream(streamData.get()));
    ASSERT_EMIT_EQ(reporter,
                   *stream,
                   "<</Length 12>> stream\nTest\nFoo\tBar\nendstream");
    stream->insertInt("Attribute", 42);
    ASSERT_EMIT_EQ(reporter,
                   *stream,
                   "<</Length 12\n/Attribute 42>> stream\n"
                   "Test\nFoo\tBar\nendstream");

    {
        char streamBytes2[] = "This is a longer string, so that compression "
                              "can do something with it. With shorter strings, "
                              "the short circuit logic cuts in and we end up "
                              "with an uncompressed string.";
        SkAutoDataUnref streamData2(SkData::NewWithCopy(streamBytes2,
                                                        strlen(streamBytes2)));
        sk_sp<SkPDFStream> stream(new SkPDFStream(streamData2.get()));

        SkDynamicMemoryWStream compressedByteStream;
        SkDeflateWStream deflateWStream(&compressedByteStream);
        deflateWStream.write(streamBytes2, strlen(streamBytes2));
        deflateWStream.finalize();

        SkDynamicMemoryWStream expected;
        expected.writeText("<</Filter /FlateDecode\n/Length 116>> stream\n");
        compressedByteStream.writeToStream(&expected);
        compressedByteStream.reset();
        expected.writeText("\nendstream");
        SkAutoDataUnref expectedResultData2(expected.copyToData());
        SkString result = emit_to_string(*stream);
        ASSERT_EQL(reporter,
                   result,
                   (const char*)expectedResultData2->data(),
                   expectedResultData2->size());
    }
}

static void TestObjectNumberMap(skiatest::Reporter* reporter) {
    SkPDFObjNumMap objNumMap;
    sk_sp<SkPDFArray> a1(new SkPDFArray);
    sk_sp<SkPDFArray> a2(new SkPDFArray);
    sk_sp<SkPDFArray> a3(new SkPDFArray);

    objNumMap.addObject(a1.get());
    objNumMap.addObject(a2.get());
    objNumMap.addObject(a3.get());

    // The objects should be numbered in the order they are added,
    // starting with 1.
    REPORTER_ASSERT(reporter, objNumMap.getObjectNumber(a1.get()) == 1);
    REPORTER_ASSERT(reporter, objNumMap.getObjectNumber(a2.get()) == 2);
    REPORTER_ASSERT(reporter, objNumMap.getObjectNumber(a3.get()) == 3);
    // Assert that repeated calls to get the object number return
    // consistent result.
    REPORTER_ASSERT(reporter, objNumMap.getObjectNumber(a1.get()) == 1);
}

static void TestObjectRef(skiatest::Reporter* reporter) {
    sk_sp<SkPDFArray> a1(new SkPDFArray);
    sk_sp<SkPDFArray> a2(new SkPDFArray);
    a2->appendObjRef(a1);

    Catalog catalog;
    catalog.numbers.addObject(a1.get());
    REPORTER_ASSERT(reporter, catalog.numbers.getObjectNumber(a1.get()) == 1);

    SkString result = emit_to_string(*a2, &catalog);
    // If appendObjRef misbehaves, then the result would
    // be [[]], not [1 0 R].
    ASSERT_EQ(reporter, result, "[1 0 R]");
}

static void TestSubstitute(skiatest::Reporter* reporter) {
    sk_sp<SkPDFDict> proxy(new SkPDFDict());
    sk_sp<SkPDFDict> stub(new SkPDFDict());

    proxy->insertInt("Value", 33);
    stub->insertInt("Value", 44);

    SkPDFSubstituteMap substituteMap;
    substituteMap.setSubstitute(proxy.get(), stub.get());
    SkPDFObjNumMap catalog;
    catalog.addObject(proxy.get());

    REPORTER_ASSERT(reporter, stub.get() == substituteMap.getSubstitute(proxy.get()));
    REPORTER_ASSERT(reporter, proxy.get() != substituteMap.getSubstitute(stub.get()));
}

// This test used to assert without the fix submitted for
// http://code.google.com/p/skia/issues/detail?id=1083.
// SKP files might have invalid glyph ids. This test ensures they are ignored,
// and there is no assert on input data in Debug mode.
static void test_issue1083() {
    SkDynamicMemoryWStream outStream;
    sk_sp<SkDocument> doc(SkDocument::MakePDF(&outStream));
    SkCanvas* canvas = doc->beginPage(100.0f, 100.0f);
    SkPaint paint;
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    uint16_t glyphID = 65000;
    canvas->drawText(&glyphID, 2, 0, 0, paint);

    doc->close();
}

static void TestPDFUnion(skiatest::Reporter* reporter) {
    SkPDFUnion boolTrue = SkPDFUnion::Bool(true);
    ASSERT_EMIT_EQ(reporter, boolTrue, "true");

    SkPDFUnion boolFalse = SkPDFUnion::Bool(false);
    ASSERT_EMIT_EQ(reporter, boolFalse, "false");

    SkPDFUnion int42 = SkPDFUnion::Int(42);
    ASSERT_EMIT_EQ(reporter, int42, "42");

    SkPDFUnion realHalf = SkPDFUnion::Scalar(SK_ScalarHalf);
    ASSERT_EMIT_EQ(reporter, realHalf, ".5");

    SkPDFUnion bigScalar = SkPDFUnion::Scalar(110999.75f);
    ASSERT_EMIT_EQ(reporter, bigScalar, "110999.75");

    SkPDFUnion biggerScalar = SkPDFUnion::Scalar(50000000.1f);
    ASSERT_EMIT_EQ(reporter, biggerScalar, "50000000");

    SkPDFUnion smallestScalar = SkPDFUnion::Scalar(1.0f / 65536);
    ASSERT_EMIT_EQ(reporter, smallestScalar, ".0000152587890");

    SkPDFUnion stringSimple = SkPDFUnion::String("test ) string ( foo");
    ASSERT_EMIT_EQ(reporter, stringSimple, "(test \\) string \\( foo)");

    SkString stringComplexInput("\ttest ) string ( foo");
    SkPDFUnion stringComplex = SkPDFUnion::String(stringComplexInput);
    ASSERT_EMIT_EQ(reporter, stringComplex, "(\\011test \\) string \\( foo)");

    SkString binaryStringInput("\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20");
    SkPDFUnion binaryString = SkPDFUnion::String(binaryStringInput);
    ASSERT_EMIT_EQ(reporter, binaryString, "<0102030405060708090A0B0C0D0E0F10>");

    SkString nameInput("Test name\twith#tab");
    SkPDFUnion name = SkPDFUnion::Name(nameInput);
    ASSERT_EMIT_EQ(reporter, name, "/Test#20name#09with#23tab");

    SkString nameInput2("A#/%()<>[]{}B");
    SkPDFUnion name2 = SkPDFUnion::Name(nameInput2);
    ASSERT_EMIT_EQ(reporter, name2, "/A#23#2F#25#28#29#3C#3E#5B#5D#7B#7DB");

    SkPDFUnion name3 = SkPDFUnion::Name("SimpleNameWithOnlyPrintableASCII");
    ASSERT_EMIT_EQ(reporter, name3, "/SimpleNameWithOnlyPrintableASCII");

    // Test that we correctly handle characters with the high-bit set.
    SkString highBitString("\xDE\xAD" "be\xEF");
    SkPDFUnion highBitName = SkPDFUnion::Name(highBitString);
    ASSERT_EMIT_EQ(reporter, highBitName, "/#DE#ADbe#EF");
}

static void TestPDFArray(skiatest::Reporter* reporter) {
    sk_sp<SkPDFArray> array(new SkPDFArray);
    ASSERT_EMIT_EQ(reporter, *array, "[]");

    array->appendInt(42);
    ASSERT_EMIT_EQ(reporter, *array, "[42]");

    array->appendScalar(SK_ScalarHalf);
    ASSERT_EMIT_EQ(reporter, *array, "[42 .5]");

    array->appendInt(0);
    ASSERT_EMIT_EQ(reporter, *array, "[42 .5 0]");

    array->appendBool(true);
    ASSERT_EMIT_EQ(reporter, *array, "[42 .5 0 true]");

    array->appendName("ThisName");
    ASSERT_EMIT_EQ(reporter, *array, "[42 .5 0 true /ThisName]");

    array->appendName(SkString("AnotherName"));
    ASSERT_EMIT_EQ(reporter, *array, "[42 .5 0 true /ThisName /AnotherName]");

    array->appendString("This String");
    ASSERT_EMIT_EQ(reporter, *array,
                   "[42 .5 0 true /ThisName /AnotherName (This String)]");

    array->appendString(SkString("Another String"));
    ASSERT_EMIT_EQ(reporter, *array,
                   "[42 .5 0 true /ThisName /AnotherName (This String) "
                   "(Another String)]");

    sk_sp<SkPDFArray> innerArray(new SkPDFArray);
    innerArray->appendInt(-1);
    array->appendObject(std::move(innerArray));
    ASSERT_EMIT_EQ(reporter, *array,
                   "[42 .5 0 true /ThisName /AnotherName (This String) "
                   "(Another String) [-1]]");

    sk_sp<SkPDFArray> referencedArray(new SkPDFArray);
    Catalog catalog;
    catalog.numbers.addObject(referencedArray.get());
    REPORTER_ASSERT(reporter, catalog.numbers.getObjectNumber(
                            referencedArray.get()) == 1);
    array->appendObjRef(std::move(referencedArray));

    SkString result = emit_to_string(*array, &catalog);
    ASSERT_EQ(reporter, result,
              "[42 .5 0 true /ThisName /AnotherName (This String) "
              "(Another String) [-1] 1 0 R]");
}

static void TestPDFDict(skiatest::Reporter* reporter) {
    sk_sp<SkPDFDict> dict(new SkPDFDict);
    ASSERT_EMIT_EQ(reporter, *dict, "<<>>");

    dict->insertInt("n1", SkToSizeT(42));
    ASSERT_EMIT_EQ(reporter, *dict, "<</n1 42>>");

    dict.reset(new SkPDFDict);
    ASSERT_EMIT_EQ(reporter, *dict, "<<>>");

    dict->insertInt("n1", 42);
    ASSERT_EMIT_EQ(reporter, *dict, "<</n1 42>>");

    dict->insertScalar("n2", SK_ScalarHalf);

    SkString n3("n3");
    sk_sp<SkPDFArray> innerArray(new SkPDFArray);
    innerArray->appendInt(-100);
    dict->insertObject(n3, std::move(innerArray));
    ASSERT_EMIT_EQ(reporter, *dict, "<</n1 42\n/n2 .5\n/n3 [-100]>>");

    dict.reset(new SkPDFDict);
    ASSERT_EMIT_EQ(reporter, *dict, "<<>>");

    dict->insertInt("n1", 24);
    ASSERT_EMIT_EQ(reporter, *dict, "<</n1 24>>");

    dict->insertInt("n2", SkToSizeT(99));
    ASSERT_EMIT_EQ(reporter, *dict, "<</n1 24\n/n2 99>>");

    dict->insertScalar("n3", SK_ScalarHalf);
    ASSERT_EMIT_EQ(reporter, *dict, "<</n1 24\n/n2 99\n/n3 .5>>");

    dict->insertName("n4", "AName");
    ASSERT_EMIT_EQ(reporter, *dict, "<</n1 24\n/n2 99\n/n3 .5\n/n4 /AName>>");

    dict->insertName("n5", SkString("AnotherName"));
    ASSERT_EMIT_EQ(reporter, *dict, "<</n1 24\n/n2 99\n/n3 .5\n/n4 /AName\n"
                   "/n5 /AnotherName>>");

    dict->insertString("n6", "A String");
    ASSERT_EMIT_EQ(reporter, *dict, "<</n1 24\n/n2 99\n/n3 .5\n/n4 /AName\n"
                   "/n5 /AnotherName\n/n6 (A String)>>");

    dict->insertString("n7", SkString("Another String"));
    ASSERT_EMIT_EQ(reporter, *dict, "<</n1 24\n/n2 99\n/n3 .5\n/n4 /AName\n"
                   "/n5 /AnotherName\n/n6 (A String)\n/n7 (Another String)>>");

    dict.reset(new SkPDFDict("DType"));
    ASSERT_EMIT_EQ(reporter, *dict, "<</Type /DType>>");

    sk_sp<SkPDFArray> referencedArray(new SkPDFArray);
    Catalog catalog;
    catalog.numbers.addObject(referencedArray.get());
    REPORTER_ASSERT(reporter, catalog.numbers.getObjectNumber(
                            referencedArray.get()) == 1);
    dict->insertObjRef("n1", std::move(referencedArray));
    SkString result = emit_to_string(*dict, &catalog);
    ASSERT_EQ(reporter, result, "<</Type /DType\n/n1 1 0 R>>");
}

DEF_TEST(PDFPrimitives, reporter) {
    TestPDFUnion(reporter);
    TestPDFArray(reporter);
    TestPDFDict(reporter);
    TestPDFStream(reporter);
    TestObjectNumberMap(reporter);
    TestObjectRef(reporter);
    TestSubstitute(reporter);
    test_issue1083();
}

namespace {

class DummyImageFilter : public SkImageFilter {
public:
    static sk_sp<DummyImageFilter> Make(bool visited = false) {
        return sk_sp<DummyImageFilter>(new DummyImageFilter(visited));
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(DummyImageFilter)
    bool visited() const { return fVisited; }

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override {
        fVisited = true;
        offset->fX = offset->fY = 0;
        return sk_ref_sp<SkSpecialImage>(source);
    }

private:
    DummyImageFilter(bool visited) : INHERITED(nullptr, 0, nullptr), fVisited(visited) {}

    mutable bool fVisited;

    typedef SkImageFilter INHERITED;
};

sk_sp<SkFlattenable> DummyImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    bool visited = buffer.readBool();
    return DummyImageFilter::Make(visited);
}

#ifndef SK_IGNORE_TO_STRING
void DummyImageFilter::toString(SkString* str) const {
    str->appendf("DummyImageFilter: (");
    str->append(")");
}
#endif

};

// Check that PDF rendering of image filters successfully falls back to
// CPU rasterization.
DEF_TEST(PDFImageFilter, reporter) {
    SkDynamicMemoryWStream stream;
    sk_sp<SkDocument> doc(SkDocument::MakePDF(&stream));
    SkCanvas* canvas = doc->beginPage(100.0f, 100.0f);

    sk_sp<DummyImageFilter> filter(DummyImageFilter::Make());

    // Filter just created; should be unvisited.
    REPORTER_ASSERT(reporter, !filter->visited());
    SkPaint paint;
    paint.setImageFilter(filter.get());
    canvas->drawRect(SkRect::MakeWH(100, 100), paint);
    doc->close();

    // Filter was used in rendering; should be visited.
    REPORTER_ASSERT(reporter, filter->visited());
}

// Check that PDF rendering of image filters successfully falls back to
// CPU rasterization.
DEF_TEST(PDFFontCanEmbedTypeface, reporter) {
    SkPDFCanon canon;

    const char resource[] = "fonts/Roboto2-Regular_NoEmbed.ttf";
    sk_sp<SkTypeface> noEmbedTypeface(MakeResourceAsTypeface(resource));
    if (noEmbedTypeface) {
        REPORTER_ASSERT(reporter,
                        !SkPDFFont::CanEmbedTypeface(noEmbedTypeface.get(), &canon));
    }
    sk_sp<SkTypeface> portableTypeface(
            sk_tool_utils::create_portable_typeface(NULL, SkFontStyle()));
    REPORTER_ASSERT(reporter,
                    SkPDFFont::CanEmbedTypeface(portableTypeface.get(), &canon));
}


// test to see that all finite scalars round trip via scanf().
static void check_pdf_scalar_serialization(
        skiatest::Reporter* reporter, float inputFloat) {
    char floatString[SkPDFUtils::kMaximumFloatDecimalLength];
    size_t len = SkPDFUtils::FloatToDecimal(inputFloat, floatString);
    if (len >= sizeof(floatString)) {
        ERRORF(reporter, "string too long: %u", (unsigned)len);
        return;
    }
    if (floatString[len] != '\0' || strlen(floatString) != len) {
        ERRORF(reporter, "terminator misplaced.");
        return;  // The terminator is needed for sscanf().
    }
    if (reporter->verbose()) {
        SkDebugf("%15.9g = \"%s\"\n", inputFloat, floatString);
    }
    float roundTripFloat;
    if (1 != sscanf(floatString, "%f", &roundTripFloat)) {
        ERRORF(reporter, "unscannable result: %s", floatString);
        return;
    }
    if (isfinite(inputFloat) && roundTripFloat != inputFloat) {
        ERRORF(reporter, "roundTripFloat (%.9g) != inputFloat (%.9g)",
               roundTripFloat, inputFloat);
    }
}

// Test SkPDFUtils::AppendScalar for accuracy.
DEF_TEST(PDFPrimitives_Scalar, reporter) {
    SkRandom random(0x5EED);
    int iterationCount = 512;
    while (iterationCount-- > 0) {
        union { uint32_t u; float f; };
        u = random.nextU();
        static_assert(sizeof(float) == sizeof(uint32_t), "");
        check_pdf_scalar_serialization(reporter, f);
    }
    float alwaysCheck[] = {
        0.0f, -0.0f, 1.0f, -1.0f, SK_ScalarPI, 0.1f, FLT_MIN, FLT_MAX,
        -FLT_MIN, -FLT_MAX, FLT_MIN / 16.0f, -FLT_MIN / 16.0f,
        SK_FloatNaN, SK_FloatInfinity, SK_FloatNegativeInfinity,
        -FLT_MIN / 8388608.0
    };
    for (float inputFloat: alwaysCheck) {
        check_pdf_scalar_serialization(reporter, inputFloat);
    }
}
