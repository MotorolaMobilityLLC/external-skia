/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/src/reader/StreamReader.h"
#include "include/core/SkStream.h"

namespace skrive::internal {

std::unique_ptr<StreamReader> MakeBinaryStreamReader(std::unique_ptr<SkStreamAsset>) {
    // TODO
    return nullptr;
}

}
