
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkData.h"
#include "SkDataSet.h"
#include "SkDataTable.h"
#include "SkStream.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"

template <typename T> class SkTUnref {
public:
    SkTUnref(T* ref) : fRef(ref) {}
    ~SkTUnref() { fRef->unref(); }

    operator T*() { return fRef; }
    operator const T*() { return fRef; }

private:
    T*  fRef;
};

static void test_is_equal(skiatest::Reporter* reporter,
                          const SkDataTable* a, const SkDataTable* b) {
    REPORTER_ASSERT(reporter, a->count() == b->count());
    for (int i = 0; i < a->count(); ++i) {
        size_t sizea, sizeb;
        const void* mema = a->at(i, &sizea);
        const void* memb = b->at(i, &sizeb);
        REPORTER_ASSERT(reporter, sizea == sizeb);
        REPORTER_ASSERT(reporter, !memcmp(mema, memb, sizea));
    }
}

static void test_datatable_flatten(skiatest::Reporter* reporter,
                                   SkDataTable* table) {
    SkOrderedWriteBuffer wb(1024);
    wb.writeFlattenable(table);

    size_t wsize = wb.size();
    SkAutoMalloc storage(wsize);
    wb.writeToMemory(storage.get());

    SkOrderedReadBuffer rb(storage.get(), wsize);
    SkAutoTUnref<SkDataTable> newTable((SkDataTable*)rb.readFlattenable());

    test_is_equal(reporter, table, newTable);
}

static void test_datatable_is_empty(skiatest::Reporter* reporter,
                                    SkDataTable* table) {
    REPORTER_ASSERT(reporter, table->isEmpty());
    REPORTER_ASSERT(reporter, 0 == table->count());
    test_datatable_flatten(reporter, table);
}

static void test_emptytable(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkDataTable> table0(SkDataTable::NewEmpty());
    SkAutoTUnref<SkDataTable> table1(SkDataTable::NewCopyArrays(NULL, NULL, 0));
    SkAutoTUnref<SkDataTable> table2(SkDataTable::NewCopyArray(NULL, 0, 0));
    SkAutoTUnref<SkDataTable> table3(SkDataTable::NewArrayProc(NULL, 0, 0,
                                                               NULL, NULL));

    test_datatable_is_empty(reporter, table0);
    test_datatable_is_empty(reporter, table1);
    test_datatable_is_empty(reporter, table2);
    test_datatable_is_empty(reporter, table3);

    test_is_equal(reporter, table0, table1);
    test_is_equal(reporter, table0, table2);
    test_is_equal(reporter, table0, table3);
}

static void test_simpletable(skiatest::Reporter* reporter) {
    const int idata[] = { 1, 4, 9, 16, 25, 63 };
    int icount = SK_ARRAY_COUNT(idata);
    SkAutoTUnref<SkDataTable> itable(SkDataTable::NewCopyArray(idata,
                                                               sizeof(idata[0]),
                                                               icount));
    REPORTER_ASSERT(reporter, itable->count() == icount);
    for (int i = 0; i < icount; ++i) {
        size_t size;
        REPORTER_ASSERT(reporter, sizeof(int) == itable->atSize(i));
        REPORTER_ASSERT(reporter, *itable->atT<int>(i, &size) == idata[i]);
        REPORTER_ASSERT(reporter, sizeof(int) == size);
    }
    test_datatable_flatten(reporter, itable);
}

static void test_vartable(skiatest::Reporter* reporter) {
    const char* str[] = {
        "", "a", "be", "see", "deigh", "ef", "ggggggggggggggggggggggggggg"
    };
    int count = SK_ARRAY_COUNT(str);
    size_t sizes[SK_ARRAY_COUNT(str)];
    for (int i = 0; i < count; ++i) {
        sizes[i] = strlen(str[i]) + 1;
    }

    SkAutoTUnref<SkDataTable> table(SkDataTable::NewCopyArrays(
                                        (const void*const*)str, sizes, count));

    REPORTER_ASSERT(reporter, table->count() == count);
    for (int i = 0; i < count; ++i) {
        size_t size;
        REPORTER_ASSERT(reporter, table->atSize(i) == sizes[i]);
        REPORTER_ASSERT(reporter, !strcmp(table->atT<const char>(i, &size),
                                          str[i]));
        REPORTER_ASSERT(reporter, size == sizes[i]);

        const char* s = table->atStr(i);
        REPORTER_ASSERT(reporter, strlen(s) == strlen(str[i]));
    }
    test_datatable_flatten(reporter, table);
}

static void test_tablebuilder(skiatest::Reporter* reporter) {
    const char* str[] = {
        "", "a", "be", "see", "deigh", "ef", "ggggggggggggggggggggggggggg"
    };
    int count = SK_ARRAY_COUNT(str);

    SkDataTableBuilder builder(16);

    for (int i = 0; i < count; ++i) {
        builder.append(str[i], strlen(str[i]) + 1);
    }
    SkAutoTUnref<SkDataTable> table(builder.detachDataTable());

    REPORTER_ASSERT(reporter, table->count() == count);
    for (int i = 0; i < count; ++i) {
        size_t size;
        REPORTER_ASSERT(reporter, table->atSize(i) == strlen(str[i]) + 1);
        REPORTER_ASSERT(reporter, !strcmp(table->atT<const char>(i, &size),
                                          str[i]));
        REPORTER_ASSERT(reporter, size == strlen(str[i]) + 1);

        const char* s = table->atStr(i);
        REPORTER_ASSERT(reporter, strlen(s) == strlen(str[i]));
    }
    test_datatable_flatten(reporter, table);
}

static void test_globaltable(skiatest::Reporter* reporter) {
    static const int gData[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    int count = SK_ARRAY_COUNT(gData);

    SkAutoTUnref<SkDataTable> table(SkDataTable::NewArrayProc(gData,
                                          sizeof(gData[0]), count, NULL, NULL));

    REPORTER_ASSERT(reporter, table->count() == count);
    for (int i = 0; i < count; ++i) {
        size_t size;
        REPORTER_ASSERT(reporter, table->atSize(i) == sizeof(int));
        REPORTER_ASSERT(reporter, *table->atT<const char>(i, &size) == i);
        REPORTER_ASSERT(reporter, sizeof(int) == size);
    }
    test_datatable_flatten(reporter, table);
}

static void TestDataTable(skiatest::Reporter* reporter) {
    test_emptytable(reporter);
    test_simpletable(reporter);
    test_vartable(reporter);
    test_tablebuilder(reporter);
    test_globaltable(reporter);
}

static void unrefAll(const SkDataSet::Pair pairs[], int count) {
    for (int i = 0; i < count; ++i) {
        pairs[i].fValue->unref();
    }
}

// asserts that inner is a subset of outer
static void test_dataset_subset(skiatest::Reporter* reporter,
                                const SkDataSet& outer, const SkDataSet& inner) {
    SkDataSet::Iter iter(inner);
    for (; !iter.done(); iter.next()) {
        SkData* outerData = outer.find(iter.key());
        REPORTER_ASSERT(reporter, outerData);
        REPORTER_ASSERT(reporter, outerData->equals(iter.value()));
    }
}

static void test_datasets_equal(skiatest::Reporter* reporter,
                                const SkDataSet& ds0, const SkDataSet& ds1) {
    REPORTER_ASSERT(reporter, ds0.count() == ds1.count());

    test_dataset_subset(reporter, ds0, ds1);
    test_dataset_subset(reporter, ds1, ds0);
}

static void test_dataset(skiatest::Reporter* reporter, const SkDataSet& ds,
                         int count) {
    REPORTER_ASSERT(reporter, ds.count() == count);

    SkDataSet::Iter iter(ds);
    int index = 0;
    for (; !iter.done(); iter.next()) {
//        const char* name = iter.key();
//        SkData* data = iter.value();
//        SkDebugf("[%d] %s:%s\n", index, name, (const char*)data->bytes());
        index += 1;
    }
    REPORTER_ASSERT(reporter, index == count);

    SkDynamicMemoryWStream ostream;
    ds.writeToStream(&ostream);
    SkMemoryStream istream;
    istream.setData(ostream.copyToData())->unref();
    SkDataSet copy(&istream);

    test_datasets_equal(reporter, ds, copy);
}

static void test_dataset(skiatest::Reporter* reporter) {
    SkDataSet set0(NULL, 0);
    SkDataSet set1("hello", SkTUnref<SkData>(SkData::NewWithCString("world")));

    const SkDataSet::Pair pairs[] = {
        { "one", SkData::NewWithCString("1") },
        { "two", SkData::NewWithCString("2") },
        { "three", SkData::NewWithCString("3") },
    };
    SkDataSet set3(pairs, 3);
    unrefAll(pairs, 3);

    test_dataset(reporter, set0, 0);
    test_dataset(reporter, set1, 1);
    test_dataset(reporter, set3, 3);
}

static void* gGlobal;

static void delete_int_proc(const void* ptr, size_t len, void* context) {
    int* data = (int*)ptr;
    SkASSERT(context == gGlobal);
    delete[] data;
}

static void assert_len(skiatest::Reporter* reporter, SkData* ref, size_t len) {
    REPORTER_ASSERT(reporter, ref->size() == len);
}

static void assert_data(skiatest::Reporter* reporter, SkData* ref,
                        const void* data, size_t len) {
    REPORTER_ASSERT(reporter, ref->size() == len);
    REPORTER_ASSERT(reporter, !memcmp(ref->data(), data, len));
}

static void test_cstring(skiatest::Reporter* reporter) {
    const char str[] = "Hello world";
    size_t     len = strlen(str);

    SkAutoTUnref<SkData> r0(SkData::NewWithCopy(str, len + 1));
    SkAutoTUnref<SkData> r1(SkData::NewWithCString(str));

    REPORTER_ASSERT(reporter, r0->equals(r1));

    SkAutoTUnref<SkData> r2(SkData::NewWithCString(NULL));
    REPORTER_ASSERT(reporter, 1 == r2->size());
    REPORTER_ASSERT(reporter, 0 == *r2->bytes());
}

static void TestData(skiatest::Reporter* reporter) {
    const char* str = "We the people, in order to form a more perfect union.";
    const int N = 10;

    SkAutoTUnref<SkData> r0(SkData::NewEmpty());
    SkAutoTUnref<SkData> r1(SkData::NewWithCopy(str, strlen(str)));
    SkAutoTUnref<SkData> r2(SkData::NewWithProc(new int[N], N*sizeof(int),
                                           delete_int_proc, gGlobal));
    SkAutoTUnref<SkData> r3(SkData::NewSubset(r1, 7, 6));

    assert_len(reporter, r0, 0);
    assert_len(reporter, r1, strlen(str));
    assert_len(reporter, r2, N * sizeof(int));
    assert_len(reporter, r3, 6);

    assert_data(reporter, r1, str, strlen(str));
    assert_data(reporter, r3, "people", 6);

    SkData* tmp = SkData::NewSubset(r1, strlen(str), 10);
    assert_len(reporter, tmp, 0);
    tmp->unref();
    tmp = SkData::NewSubset(r1, 0, 0);
    assert_len(reporter, tmp, 0);
    tmp->unref();

    test_cstring(reporter);
    test_dataset(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Data", DataTestClass, TestData)
DEFINE_TESTCLASS("DataTable", DataTableTestClass, TestDataTable)
