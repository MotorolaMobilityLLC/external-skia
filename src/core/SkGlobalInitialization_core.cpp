/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlattenable.h"
#include "SkOnce.h"

void SkFlattenable::RegisterFlattenablesIfNeeded() {
    static SkOnce once;
    once([]{
        SkFlattenable::PrivateInitializer::InitEffects();
        SkFlattenable::PrivateInitializer::InitImageFilters();
        SkFlattenable::Finalize();
    });
}
