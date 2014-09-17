/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageDecoder.h"
#include "SkImage_Base.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"

class SkImage_Codec : public SkImage_Base {
public:
    static SkImage* NewEmpty();

    SkImage_Codec(SkData* encodedData, int width, int height);
    virtual ~SkImage_Codec();

    virtual void onDraw(SkCanvas*, SkScalar, SkScalar, const SkPaint*) const SK_OVERRIDE;
    virtual void onDrawRectToRect(SkCanvas*, const SkRect*, const SkRect&,
                                  const SkPaint*) const SK_OVERRIDE;

    virtual bool isOpaque() const SK_OVERRIDE;

private:
    bool ensureBitmapDecoded() const;

    SkData*     fEncodedData;
    SkBitmap    fBitmap;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Codec::SkImage_Codec(SkData* data, int width, int height) : INHERITED(width, height) {
    fEncodedData = data;
    fEncodedData->ref();
}

SkImage_Codec::~SkImage_Codec() {
    fEncodedData->unref();
}

bool SkImage_Codec::ensureBitmapDecoded() const {
    if (!fBitmap.pixelRef()) {
        // todo: this needs to be thread-safe
        SkBitmap* bitmap = const_cast<SkBitmap*>(&fBitmap);
        if (!SkImageDecoder::DecodeMemory(fEncodedData->bytes(), fEncodedData->size(), bitmap)) {
            return false;
        }
    }
    return true;
}

void SkImage_Codec::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const {
    if(!this->ensureBitmapDecoded()) {
        return;
    }

    canvas->drawBitmap(fBitmap, x, y, paint);
}

void SkImage_Codec::onDrawRectToRect(SkCanvas* canvas, const SkRect* src, const SkRect& dst,
                                     const SkPaint* paint) const {
    if(!this->ensureBitmapDecoded()) {
        return;
    }

    canvas->drawBitmapRectToRect(fBitmap, src, dst, paint);
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewEncodedData(SkData* data) {
    if (NULL == data) {
        return NULL;
    }

    SkBitmap bitmap;
    if (!SkImageDecoder::DecodeMemory(data->bytes(), data->size(), &bitmap, kUnknown_SkColorType,
                                      SkImageDecoder::kDecodeBounds_Mode)) {
        return NULL;
    }

    return SkNEW_ARGS(SkImage_Codec, (data, bitmap.width(), bitmap.height()));
}


bool SkImage_Codec::isOpaque() const {
    return fBitmap.isOpaque();
}
