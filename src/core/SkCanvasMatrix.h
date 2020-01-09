/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCanvasMatrix_DEFINED
#define SkCanvasMatrix_DEFINED

#include "include/core/SkMatrix.h"
#include "include/private/SkM44.h"

class SkCanvasMatrix {
    SkM44 fM;
public:
    SkCanvasMatrix& operator=(const SkMatrix& other) { fM = other; return *this; }

    void reset() { fM.setIdentity(); }
    void preTranslate(SkScalar x, SkScalar y) { fM.preTranslate(x, y); }
    void preConcat(const SkMatrix& m) { fM.preConcat(m); }
    void preConcat44(const SkScalar m[]) { fM.setConcat(fM, m); }

    operator SkMatrix() const { return fM.asM33(); }
    // the legacy check was just for the 3x3 portion, so we only check those
    bool isScaleTranslate() const {
        return fM.atColMajor(1) == 0 && fM.atColMajor(3) == 0 &&
               fM.atColMajor(4) == 0 && fM.atColMajor(7) == 0 && fM.atColMajor(15) == 1;
    }
    bool rectStaysRect() const { return fM.asM33().rectStaysRect(); }

    float getScaleX() const { return fM.atColMajor(0); }
    float getScaleY() const { return fM.atColMajor(5); }
    float getTranslateX() const { return fM.atColMajor(12); }
    float getTranslateY() const { return fM.atColMajor(13); }

    bool invert(SkMatrix* inv) const { return fM.asM33().invert(inv); }

    bool mapRect(SkRect* dst, const SkRect& src) { return fM.asM33().mapRect(dst, src); }
};

#endif
