/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLiteDL_DEFINED
#define SkLiteDL_DEFINED

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkDrawable.h"
#include "SkRect.h"
#include "SkTDArray.h"

class GrContext;

class SkLiteDL final : public SkDrawable {
public:
    static sk_sp<SkLiteDL> New(SkRect);

    void optimizeFor(GrContext*);

    void save();
    void saveLayer(const SkRect*, const SkPaint*, const SkImageFilter*, uint32_t);
    void restore();

    void    concat (const SkMatrix&);
    void setMatrix (const SkMatrix&);
    void translateZ(SkScalar) {/*TODO*/}

    void clipPath  (const   SkPath&, SkRegion::Op, bool aa);
    void clipRect  (const   SkRect&, SkRegion::Op, bool aa);
    void clipRRect (const  SkRRect&, SkRegion::Op, bool aa);
    void clipRegion(const SkRegion&, SkRegion::Op);


    void drawPaint (const SkPaint&);
    void drawPath  (const SkPath&, const SkPaint&);
    void drawRect  (const SkRect&, const SkPaint&);
    void drawOval  (const SkRect&, const SkPaint&);
    void drawRRect (const SkRRect&, const SkPaint&);
    void drawDRRect(const SkRRect&, const SkRRect&, const SkPaint&);

    void drawAnnotation     (const SkRect&, const char*, SkData*) {/*TODO*/}
    void drawDrawable       (SkDrawable*, const SkMatrix*) {/*TODO*/}
    void drawPicture        (const SkPicture*, const SkMatrix*, const SkPaint*) {/*TODO*/}
    void drawShadowedPicture(const SkPicture*, const SkMatrix*, const SkPaint*) {/*TODO*/}

    void drawText       (const void*, size_t, SkScalar, SkScalar, const SkPaint&);
    void drawPosText    (const void*, size_t, const SkPoint[], const SkPaint&);
    void drawPosTextH   (const void*, size_t, const SkScalar[], SkScalar, const SkPaint&);
    void drawTextOnPath (const void*, size_t, const SkPath&, const SkMatrix*, const SkPaint&) {/*TODO*/}
    void drawTextRSXForm(const void*, size_t, const SkRSXform[], const SkRect*, const SkPaint&) {/*TODO*/}
    void drawTextBlob   (const SkTextBlob*, SkScalar,SkScalar, const SkPaint&);

    void drawBitmap    (const SkBitmap&, SkScalar, SkScalar,            const SkPaint*);
    void drawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect&, const SkPaint*);
    void drawBitmapRect(const SkBitmap&, const SkRect*,  const SkRect&, const SkPaint*,
                        SkCanvas::SrcRectConstraint);

    void drawImage    (const SkImage*, SkScalar,SkScalar,             const SkPaint*);
    void drawImageNine(const SkImage*, const SkIRect&, const SkRect&, const SkPaint*);
    void drawImageRect(const SkImage*, const SkRect*, const SkRect&,  const SkPaint*,
                       SkCanvas::SrcRectConstraint);
    void drawImageLattice(const SkImage*, const SkCanvas::Lattice&, const SkRect&, const SkPaint*)
        {/*TODO*/}

    void drawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4],
                   SkXfermode*, const SkPaint&) {/*TODO*/}
    void drawPoints(SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&) {/*TODO*/}
    void drawVertices(SkCanvas::VertexMode, int, const SkPoint[], const SkPoint[], const SkColor[],
                      SkXfermode*, const uint16_t[], int, const SkPaint&) {/*TODO*/}
    void drawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                   SkXfermode::Mode, const SkRect*, const SkPaint*) {/*TODO*/}

private:
    SkLiteDL();
    ~SkLiteDL();

    void internal_dispose() const override;

    SkRect   onGetBounds() override;
    void onDraw(SkCanvas*) override;

    SkLiteDL*          fNext;
    SkRect             fBounds;
    SkTDArray<uint8_t> fBytes;
};

#endif//SkLiteDL_DEFINED
