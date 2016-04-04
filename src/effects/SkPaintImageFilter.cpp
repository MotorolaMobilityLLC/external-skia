/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaintImageFilter.h"
#include "SkCanvas.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"

SkPaintImageFilter::SkPaintImageFilter(const SkPaint& paint, const CropRect* cropRect)
    : INHERITED(nullptr, 0, cropRect)
    , fPaint(paint) {
}

SkFlattenable* SkPaintImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    SkPaint paint;
    buffer.readPaint(&paint);
    return SkPaintImageFilter::Make(paint, &common.cropRect()).release();
}

void SkPaintImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePaint(fPaint);
}

sk_sp<SkSpecialImage> SkPaintImageFilter::onFilterImage(SkSpecialImage* source,
                                                        const Context& ctx,
                                                        SkIPoint* offset) const {
    SkIRect bounds;
    const SkIRect srcBounds = SkIRect::MakeWH(source->width(), source->height());
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::MakeN32(bounds.width(), bounds.height(),
                                            kPremul_SkAlphaType);

    sk_sp<SkSpecialSurface> surf(source->makeSurface(info));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    SkRect rect = SkRect::MakeIWH(bounds.width(), bounds.height());
    SkMatrix inverse;
    if (matrix.invert(&inverse)) {
        inverse.mapRect(&rect);
    }
    canvas->setMatrix(matrix);
    canvas->drawRect(rect, fPaint);

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

bool SkPaintImageFilter::canComputeFastBounds() const {
    // http:skbug.com/4627: "make computeFastBounds and onFilterBounds() CropRect-aware"
    // computeFastBounds() doesn't currently take the crop rect into account,
    // so we can't compute it. If a full crop rect is set, we should return true here.
    return false;
}

#ifndef SK_IGNORE_TO_STRING
void SkPaintImageFilter::toString(SkString* str) const {
    str->appendf("SkPaintImageFilter: (");
    fPaint.toString(str);
    str->append(")");
}
#endif
