/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPerlinNoiseShader.h"

class PerlinNoiseBench : public SkBenchmark {
    SkISize fSize;

public:
    PerlinNoiseBench(void* param) : INHERITED(param) {
        fSize = SkISize::Make(80, 80);
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "perlinnoise";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        this->test(canvas, 0, 0, SkPerlinNoiseShader::kFractalNoise_Type,
             0.1f, 0.1f, 3, 0, false);
    }

private:
    void drawClippedRect(SkCanvas* canvas, int x, int y, const SkPaint& paint) {
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                         SkIntToScalar(fSize.width()), SkIntToScalar(fSize.height())));
        SkRect r = SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                                    SkIntToScalar(fSize.width()),
                                    SkIntToScalar(fSize.height()));
        canvas->drawRect(r, paint);
        canvas->restore();
    }

    void test(SkCanvas* canvas, int x, int y, SkPerlinNoiseShader::Type type,
              float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed,
              bool stitchTiles) {
        SkShader* shader = (type == SkPerlinNoiseShader::kFractalNoise_Type) ?
            SkPerlinNoiseShader::CreateFractalNoise(baseFrequencyX, baseFrequencyY, numOctaves,
                                                    seed, stitchTiles ? &fSize : NULL) :
            SkPerlinNoiseShader::CreateTubulence(baseFrequencyX, baseFrequencyY, numOctaves,
                                                 seed, stitchTiles ? &fSize : NULL);
        SkPaint paint;
        paint.setShader(shader)->unref();
        this->drawClippedRect(canvas, x, y, paint);
    }

    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new PerlinNoiseBench(p); )
