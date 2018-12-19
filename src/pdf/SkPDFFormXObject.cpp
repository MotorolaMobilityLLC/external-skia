/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFFormXObject.h"
#include "SkPDFUtils.h"

SkPDFIndirectReference SkPDFMakeFormXObject(SkPDFDocument* doc,
                                            std::unique_ptr<SkStreamAsset> content,
                                            sk_sp<SkPDFArray> mediaBox,
                                            sk_sp<SkPDFDict> resourceDict,
                                            const SkMatrix& inverseTransform,
                                            const char* colorSpace) {
    sk_sp<SkPDFDict> dict = sk_make_sp<SkPDFDict>();
    dict->insertName("Type", "XObject");
    dict->insertName("Subtype", "Form");
    if (!inverseTransform.isIdentity()) {
        dict->insertObject("Matrix", SkPDFUtils::MatrixToArray(inverseTransform));
    }
    dict->insertObject("Resources", std::move(resourceDict));
    dict->insertObject("BBox", std::move(mediaBox));

    // Right now FormXObject is only used for saveLayer, which implies
    // isolated blending.  Do this conditionally if that changes.
    // TODO(halcanary): Is this comment obsolete, since we use it for
    // alpha masks?
    auto group = sk_make_sp<SkPDFDict>("Group");
    group->insertName("S", "Transparency");
    if (colorSpace != nullptr) {
        group->insertName("CS", colorSpace);
    }
    group->insertBool("I", true);  // Isolated.
    dict->insertObject("Group", std::move(group));
    return SkPDFStreamOut(std::move(dict), std::move(content), doc);
}
