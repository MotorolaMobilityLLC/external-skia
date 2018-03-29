/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../Fuzz.h"

void fuzz_NullGLCanvas(Fuzz* f);

extern "C" {

    // Set default LSAN options.
    const char *__lsan_default_options() {
        // Don't print the list of LSAN suppressions on every execution.
        return "print_suppressions=0";
    }

    int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        auto fuzz = Fuzz(SkData::MakeWithoutCopy(data, size));
        fuzz_NullGLCanvas(&fuzz);
        return 0;
    }
}  // extern "C"
