/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOffsetImageFilter.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkFlattenableBuffers.h"
#include "SkMatrix.h"
#include "SkPaint.h"

bool SkOffsetImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& source,
                                        const SkMatrix& matrix,
                                        SkBitmap* result,
                                        SkIPoint* offset) {
    SkImageFilter* input = getInput(0);
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
#ifdef SK_DISABLE_OFFSETIMAGEFILTER_OPTIMIZATION
    if (false) {
#else
    if (!cropRectIsSet()) {
#endif
        if (input && !input->filterImage(proxy, source, matrix, &src, &srcOffset)) {
            return false;
        }

        SkVector vec;
        matrix.mapVectors(&vec, &fOffset, 1);

        offset->fX = srcOffset.fX + SkScalarRoundToInt(vec.fX);
        offset->fY = srcOffset.fY + SkScalarRoundToInt(vec.fY);
        *result = src;
    } else {
        if (input && !input->filterImage(proxy, source, matrix, &src, &srcOffset)) {
            return false;
        }

        SkIRect bounds;
        src.getBounds(&bounds);
        bounds.offset(srcOffset);

        if (!applyCropRect(&bounds, matrix)) {
            return false;
        }

        SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
        if (NULL == device.get()) {
            return false;
        }
        SkCanvas canvas(device);
        SkPaint paint;
        paint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas.translate(SkIntToScalar(srcOffset.fX - bounds.fLeft),
                         SkIntToScalar(srcOffset.fY - bounds.fTop));
        canvas.drawBitmap(src, fOffset.x(), fOffset.y(), &paint);
        *result = device->accessBitmap(false);
        offset->fX = bounds.fLeft;
        offset->fY = bounds.fTop;
    }
    return true;
}

bool SkOffsetImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                         SkIRect* dst) {
    SkVector vec;
    ctm.mapVectors(&vec, &fOffset, 1);

    *dst = src;
    dst->offset(SkScalarRoundToInt(vec.fX), SkScalarRoundToInt(vec.fY));
    return true;
}

void SkOffsetImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fOffset);
}

SkOffsetImageFilter::SkOffsetImageFilter(SkScalar dx, SkScalar dy, SkImageFilter* input,
                                         const CropRect* cropRect) : INHERITED(input, cropRect) {
    fOffset.set(dx, dy);
}

SkOffsetImageFilter::SkOffsetImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(1, buffer) {
    buffer.readPoint(&fOffset);
    buffer.validate(SkScalarIsFinite(fOffset.fX) &&
                    SkScalarIsFinite(fOffset.fY));
}
