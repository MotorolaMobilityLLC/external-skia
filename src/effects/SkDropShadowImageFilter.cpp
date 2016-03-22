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
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkDropShadowImageFilter::SkDropShadowImageFilter(SkScalar dx, SkScalar dy,
                                                 SkScalar sigmaX, SkScalar sigmaY, SkColor color,
                                                 ShadowMode shadowMode, SkImageFilter* input,
                                                 const CropRect* cropRect)
    : INHERITED(1, &input, cropRect)
    , fDx(dx)
    , fDy(dy)
    , fSigmaX(sigmaX)
    , fSigmaY(sigmaY)
    , fColor(color)
    , fShadowMode(shadowMode)
{
}

SkFlattenable* SkDropShadowImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar dx = buffer.readScalar();
    SkScalar dy = buffer.readScalar();
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    SkColor color = buffer.readColor();
    ShadowMode shadowMode = buffer.isVersionLT(SkReadBuffer::kDropShadowMode_Version) ?
                            kDrawShadowAndForeground_ShadowMode :
                            static_cast<ShadowMode>(buffer.readInt());
    return Create(dx, dy, sigmaX, sigmaY, color, shadowMode, common.getInput(0),
                  &common.cropRect());
}

void SkDropShadowImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeScalar(fSigmaX);
    buffer.writeScalar(fSigmaY);
    buffer.writeColor(fColor);
    buffer.writeInt(static_cast<int>(fShadowMode));
}

bool SkDropShadowImageFilter::onFilterImageDeprecated(Proxy* proxy, const SkBitmap& source,
                                                      const Context& ctx,
                                                      SkBitmap* result, SkIPoint* offset) const {
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputDeprecated(0, proxy, source, ctx, &src, &srcOffset))
        return false;

    SkIRect srcBounds = src.bounds();
    srcBounds.offset(srcOffset);
    SkIRect bounds;
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (nullptr == device.get()) {
        return false;
    }
    SkCanvas canvas(device.get());

    SkVector sigma = SkVector::Make(fSigmaX, fSigmaY);
    ctx.ctm().mapVectors(&sigma, 1);
    sigma.fX = SkMaxScalar(0, sigma.fX);
    sigma.fY = SkMaxScalar(0, sigma.fY);
    SkAutoTUnref<SkImageFilter> blurFilter(SkBlurImageFilter::Create(sigma.fX, sigma.fY));
    SkPaint paint;
    paint.setImageFilter(blurFilter.get());
    paint.setColorFilter(SkColorFilter::MakeModeFilter(fColor, SkXfermode::kSrcIn_Mode));
    paint.setXfermodeMode(SkXfermode::kSrcOver_Mode);
    SkVector offsetVec = SkVector::Make(fDx, fDy);
    ctx.ctm().mapVectors(&offsetVec, 1);
    canvas.translate(SkIntToScalar(srcOffset.fX - bounds.fLeft),
                     SkIntToScalar(srcOffset.fY - bounds.fTop));
    canvas.drawBitmap(src, offsetVec.fX, offsetVec.fY, &paint);
    if (fShadowMode == kDrawShadowAndForeground_ShadowMode) {
        canvas.drawBitmap(src, 0, 0);
    }
    *result = device->accessBitmap(false);
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return true;
}

SkRect SkDropShadowImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    SkRect shadowBounds = bounds;
    shadowBounds.offset(fDx, fDy);
    shadowBounds.outset(SkScalarMul(fSigmaX, SkIntToScalar(3)),
                        SkScalarMul(fSigmaY, SkIntToScalar(3)));
    if (fShadowMode == kDrawShadowAndForeground_ShadowMode) {
        bounds.join(shadowBounds);
    } else {
        bounds = shadowBounds;
    }
    return bounds;
}

SkIRect SkDropShadowImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                    MapDirection direction) const {
    SkVector offsetVec = SkVector::Make(fDx, fDy);
    if (kReverse_MapDirection == direction) {
        offsetVec.negate();
    }
    ctm.mapVectors(&offsetVec, 1);
    SkIRect dst = src.makeOffset(SkScalarCeilToInt(offsetVec.x()),
                                 SkScalarCeilToInt(offsetVec.y()));
    SkVector sigma = SkVector::Make(fSigmaX, fSigmaY);
    ctm.mapVectors(&sigma, 1);
    dst.outset(SkScalarCeilToInt(SkScalarMul(sigma.x(), SkIntToScalar(3))),
                SkScalarCeilToInt(SkScalarMul(sigma.y(), SkIntToScalar(3))));
    if (fShadowMode == kDrawShadowAndForeground_ShadowMode) {
        dst.join(src);
    }
    return dst;
}

#ifndef SK_IGNORE_TO_STRING
void SkDropShadowImageFilter::toString(SkString* str) const {
    str->appendf("SkDropShadowImageFilter: (");

    str->appendf("dX: %f ", fDx);
    str->appendf("dY: %f ", fDy);
    str->appendf("sigmaX: %f ", fSigmaX);
    str->appendf("sigmaY: %f ", fSigmaY);

    str->append("Color: ");
    str->appendHex(fColor);

    static const char* gModeStrings[] = {
        "kDrawShadowAndForeground", "kDrawShadowOnly"
    };

    static_assert(kShadowModeCount == SK_ARRAY_COUNT(gModeStrings), "enum_mismatch");

    str->appendf(" mode: %s", gModeStrings[fShadowMode]);

    str->append(")");
}
#endif
