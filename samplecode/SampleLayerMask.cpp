/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sample.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"

///////////////////////////////////////////////////////////////////////////////

class LayerMaskView : public Sample {
public:
    LayerMaskView() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    virtual bool onQuery(Sample::Event* evt) {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "LayerMask");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawMask(SkCanvas* canvas, const SkRect& r) {
        SkPaint paint;
        paint.setAntiAlias(true);

        if (true) {
            SkBitmap mask;
            int w = SkScalarRoundToInt(r.width());
            int h = SkScalarRoundToInt(r.height());
            mask.allocN32Pixels(w, h);
            mask.eraseColor(SK_ColorTRANSPARENT);
            SkCanvas c(mask);
            SkRect bounds = r;
            bounds.offset(-bounds.fLeft, -bounds.fTop);
            c.drawOval(bounds, paint);

            paint.setBlendMode(SkBlendMode::kDstIn);
            canvas->drawBitmap(mask, r.fLeft, r.fTop, &paint);
        } else {
            SkPath p;
            p.addOval(r);
            p.setFillType(SkPath::kInverseWinding_FillType);
            paint.setBlendMode(SkBlendMode::kDstOut);
            canvas->drawPath(p, paint);
        }
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkRect  r;
        r.set(SkIntToScalar(20), SkIntToScalar(20), SkIntToScalar(120), SkIntToScalar(120));
        canvas->saveLayer(&r, nullptr);
        canvas->drawColor(SK_ColorRED);
        drawMask(canvas, r);
        canvas->restore();
    }

private:
    typedef Sample INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new LayerMaskView(); )
