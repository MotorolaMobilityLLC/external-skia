/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "SkReader32.h"
#include "Test.h"

static void assert_eof(skiatest::Reporter* reporter, const SkReader32& reader) {
    REPORTER_ASSERT(reporter, reader.eof());
    REPORTER_ASSERT(reporter, reader.size() == reader.offset());
    REPORTER_ASSERT(reporter, (const char*)reader.peek() ==
                    (const char*)reader.base() + reader.size());
}

static void assert_start(skiatest::Reporter* reporter, const SkReader32& reader) {
    REPORTER_ASSERT(reporter, 0 == reader.offset());
    REPORTER_ASSERT(reporter, reader.size() == reader.available());
    REPORTER_ASSERT(reporter, reader.isAvailable(reader.size()));
    REPORTER_ASSERT(reporter, !reader.isAvailable(reader.size() + 1));
    REPORTER_ASSERT(reporter, reader.peek() == reader.base());
}

static void assert_empty(skiatest::Reporter* reporter, const SkReader32& reader) {
    REPORTER_ASSERT(reporter, 0 == reader.size());
    REPORTER_ASSERT(reporter, 0 == reader.offset());
    REPORTER_ASSERT(reporter, 0 == reader.available());
    REPORTER_ASSERT(reporter, !reader.isAvailable(1));
    assert_eof(reporter, reader);
    assert_start(reporter, reader);
}

static void Tests(skiatest::Reporter* reporter) {
    SkReader32 reader;
    assert_empty(reporter, reader);
    REPORTER_ASSERT(reporter, NULL == reader.base());
    REPORTER_ASSERT(reporter, NULL == reader.peek());

    size_t i;

    const int32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    const SkScalar data2[] = { 0, SK_Scalar1, -SK_Scalar1, SK_Scalar1/2 };
    char buffer[SkMax32(sizeof(data), sizeof(data2))];

    reader.setMemory(data, sizeof(data));
    for (i = 0; i < SK_ARRAY_COUNT(data); ++i) {
        REPORTER_ASSERT(reporter, sizeof(data) == reader.size());
        REPORTER_ASSERT(reporter, i*4 == reader.offset());
        REPORTER_ASSERT(reporter, (const void*)data == reader.base());
        REPORTER_ASSERT(reporter, (const void*)&data[i] == reader.peek());
        REPORTER_ASSERT(reporter, data[i] == reader.readInt());
    }
    assert_eof(reporter, reader);
    reader.rewind();
    assert_start(reporter, reader);
    reader.read(buffer, sizeof(data));
    REPORTER_ASSERT(reporter, !memcmp(data, buffer, sizeof(data)));

    reader.setMemory(data2, sizeof(data2));
    for (i = 0; i < SK_ARRAY_COUNT(data2); ++i) {
        REPORTER_ASSERT(reporter, sizeof(data2) == reader.size());
        REPORTER_ASSERT(reporter, i*4 == reader.offset());
        REPORTER_ASSERT(reporter, (const void*)data2 == reader.base());
        REPORTER_ASSERT(reporter, (const void*)&data2[i] == reader.peek());
        REPORTER_ASSERT(reporter, data2[i] == reader.readScalar());
    }
    assert_eof(reporter, reader);
    reader.rewind();
    assert_start(reporter, reader);
    reader.read(buffer, sizeof(data2));
    REPORTER_ASSERT(reporter, !memcmp(data2, buffer, sizeof(data2)));

    reader.setMemory(NULL, 0);
    assert_empty(reporter, reader);
    REPORTER_ASSERT(reporter, NULL == reader.base());
    REPORTER_ASSERT(reporter, NULL == reader.peek());
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Reader32", Reader32Class, Tests)

