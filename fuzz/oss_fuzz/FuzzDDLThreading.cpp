/*
 * Copyright 2021 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"

void fuzz_DDLThreadingGL(Fuzz* f);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4000) {
        return 0;
    }
    auto fuzz = Fuzz(SkData::MakeWithoutCopy(data, size));
    fuzz_DDLThreadingGL(&fuzz);
    return 0;
}
