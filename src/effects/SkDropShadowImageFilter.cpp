/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDropShadowImageFilter.h"

#include "SkBitmap.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorMatrixFilter.h"
#include "SkDevice.h"
#include "SkFlattenableBuffers.h"

SkDropShadowImageFilter::SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigma, SkColor color, SkImageFilter* input)
    : INHERITED(input)
    , fDx(dx)
    , fDy(dy)
    , fSigmaX(sigma)
    , fSigmaY(sigma)
    , fColor(color)
{
}

SkDropShadowImageFilter::SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color, SkImageFilter* input, const CropRect* cropRect)
    : INHERITED(input, cropRect)
    , fDx(dx)
    , fDy(dy)
    , fSigmaX(sigmaX)
    , fSigmaY(sigmaY)
    , fColor(color)
{
}

SkDropShadowImageFilter::SkDropShadowImageFilter(SkFlattenableReadBuffer& buffer)
 : INHERITED(1, buffer) {
    fDx = buffer.readScalar();
    fDy = buffer.readScalar();
    fSigmaX = buffer.readScalar();
    fSigmaY = buffer.readScalar();
    fColor = buffer.readColor();
    buffer.validate(SkScalarIsFinite(fDx) &&
                    SkScalarIsFinite(fDy) &&
                    SkScalarIsFinite(fSigmaX) &&
                    SkScalarIsFinite(fSigmaY));
}

void SkDropShadowImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const
{
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeScalar(fSigmaX);
    buffer.writeScalar(fSigmaY);
    buffer.writeColor(fColor);
}

bool SkDropShadowImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& source, const SkMatrix& matrix, SkBitmap* result, SkIPoint* offset)
{
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, matrix, &src, &srcOffset))
        return false;

    SkIRect bounds;
    src.getBounds(&bounds);
    bounds.offset(srcOffset);
    if (!this->applyCropRect(&bounds, matrix)) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (NULL == device.get()) {
        return false;
    }
    SkCanvas canvas(device.get());

    SkVector sigma, localSigma = SkVector::Make(fSigmaX, fSigmaY);
    matrix.mapVectors(&sigma, &localSigma, 1);
    SkAutoTUnref<SkImageFilter> blurFilter(new SkBlurImageFilter(sigma.fX, sigma.fY));
    SkAutoTUnref<SkColorFilter> colorFilter(SkColorFilter::CreateModeFilter(fColor, SkXfermode::kSrcIn_Mode));
    SkPaint paint;
    paint.setImageFilter(blurFilter.get());
    paint.setColorFilter(colorFilter.get());
    paint.setXfermodeMode(SkXfermode::kSrcOver_Mode);
    SkVector offsetVec, localOffsetVec = SkVector::Make(fDx, fDy);
    matrix.mapVectors(&offsetVec, &localOffsetVec, 1);
    canvas.translate(-SkIntToScalar(bounds.fLeft), -SkIntToScalar(bounds.fTop));
    canvas.drawBitmap(src, offsetVec.fX, offsetVec.fY, &paint);
    canvas.drawBitmap(src, 0, 0);
    *result = device->accessBitmap(false);
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return true;
}
