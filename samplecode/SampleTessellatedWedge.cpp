/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "samplecode/Sample.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "tools/ToolUtils.h"

#if SK_SUPPORT_GPU

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/tessellate/GrPathTessellateOp.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

static float kConicWeight = .5;

// This sample enables wireframe and visualizes the triangulation generated by
// GrTessellateWedgeShader.
class TessellatedWedge : public Sample {
public:
    TessellatedWedge() {
#if 0
        fPath.moveTo(1, 0);
        int numSides = 32 * 3;
        for (int i = 1; i < numSides; ++i) {
            float theta = 2*3.1415926535897932384626433832785 * i / numSides;
            fPath.lineTo(std::cos(theta), std::sin(theta));
        }
        fPath.transform(SkMatrix::Scale(200, 200));
        fPath.transform(SkMatrix::Translate(300, 300));
#else
        fPath.moveTo(100, 300);
        fPath.conicTo(300, 100, 500, 300, kConicWeight);
        fPath.cubicTo(433, 366, 366, 433, 300, 500);
#endif
    }

private:
    void onDrawContent(SkCanvas*) override;
    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override;
    bool onClick(Sample::Click*) override;
    bool onChar(SkUnichar) override;

    SkString name() override { return SkString("TessellatedWedge"); }

    SkMatrix fLastViewMatrix = SkMatrix::I();
    SkPath fPath;
    GrTessellationPathRenderer::OpFlags fOpFlags = GrTessellationPathRenderer::OpFlags::kWireframe;

    class Click;
};

void TessellatedWedge::onDrawContent(SkCanvas* canvas) {
    canvas->clear(SK_ColorBLACK);

    auto ctx = canvas->recordingContext();
    GrSurfaceDrawContext* sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);

    SkString error;
    if (!sdc || !ctx) {
        error = "GPU Only.";
    } else if (!ctx->priv().caps()->drawInstancedSupport()) {
        error = "Instanced rendering not supported.";
    } else if (sdc->numSamples() == 1 && !ctx->priv().caps()->mixedSamplesSupport()) {
        error = "MSAA/mixed samples only.";
    }
    if (!error.isEmpty()) {
        SkFont font(nullptr, 20);
        SkPaint captionPaint;
        captionPaint.setColor(SK_ColorWHITE);
        canvas->drawString(error.c_str(), 10, 30, font, captionPaint);
        return;
    }

    GrPaint paint;
    paint.setColor4f({1,0,1,1});

    GrAAType aa;
    if (sdc->numSamples() > 1) {
        aa = GrAAType::kMSAA;
    } else if (sdc->asRenderTargetProxy()->canUseMixedSamples(*ctx->priv().caps())) {
        aa = GrAAType::kCoverage;
    } else {
        aa = GrAAType::kNone;
    }

    sdc->addDrawOp(GrOp::Make<GrPathTessellateOp>(ctx, canvas->getTotalMatrix(), fPath,
                                                  std::move(paint), aa, fOpFlags));

    // Draw the path points.
    SkPaint pointsPaint;
    pointsPaint.setColor(SK_ColorBLUE);
    pointsPaint.setStrokeWidth(8);
    SkPath devPath = fPath;
    devPath.transform(canvas->getTotalMatrix());
    {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->setMatrix(SkMatrix::I());
        canvas->drawPoints(SkCanvas::kPoints_PointMode, devPath.countPoints(),
                           SkPathPriv::PointData(devPath), pointsPaint);
    }

    fLastViewMatrix = canvas->getTotalMatrix();


    SkString caption;
    caption.printf("w=%f  (=/- and +/_ to change)", kConicWeight);
    SkFont font(nullptr, 20);
    SkPaint captionPaint;
    captionPaint.setColor(SK_ColorWHITE);
    canvas->drawString(caption, 10, 30, font, captionPaint);
}

class TessellatedWedge::Click : public Sample::Click {
public:
    Click(int ptIdx) : fPtIdx(ptIdx) {}

    void doClick(SkPath* path) {
        if (fPtIdx >= 0) {
            SkPoint pt = path->getPoint(fPtIdx);
            SkPathPriv::UpdatePathPoint(path, fPtIdx, pt + fCurr - fPrev);
        } else {
            path->transform(
                    SkMatrix::Translate(fCurr.x() - fPrev.x(), fCurr.y() - fPrev.y()), path);
        }
    }

private:
    int fPtIdx;
};

Sample::Click* TessellatedWedge::onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) {
    const SkPoint* pts = SkPathPriv::PointData(fPath);
    float fuzz = 20 / fLastViewMatrix.getMaxScale();
    for (int i = 0; i < fPath.countPoints(); ++i) {
        SkPoint screenPoint = pts[i];
        if (fabs(x - screenPoint.x()) < fuzz && fabsf(y - screenPoint.y()) < fuzz) {
            return new Click(i);
        }
    }
    return new Click(-1);
}

static float find_conic_max_error(const SkConic& conic, int numChops) {
    if (numChops > 1) {
        int leftChops = numChops / 2;
        SkConic halves[2];
        if (conic.chopAt((float)leftChops/numChops, halves)) {
            return std::max(find_conic_max_error(halves[0], leftChops),
                            find_conic_max_error(halves[1], numChops - leftChops));
        }
    }

    const SkPoint* p = conic.fPts;
    float w = conic.fW;
    SkVector n = {p[2].fY - p[0].fY, p[0].fX - p[2].fX};
    float h1 = (p[1] - p[0]).dot(n) / n.length();
    float h = h1*w / (1 + w);
    return h;
}

static void dump_conic_max_errors(const SkPath& path) {
    SkPath path_;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        if (verb == SkPathVerb::kConic) {
            int n = GrWangsFormula::quadratic(4, pts);
            float err = find_conic_max_error(SkConic(pts, *w), n);
            SkDebugf("CONIC MAX ERROR:  %f\n", err);
        }
    }
}

bool TessellatedWedge::onClick(Sample::Click* click) {
    Click* myClick = (Click*)click;
    myClick->doClick(&fPath);
    dump_conic_max_errors(fPath);
    return true;
}

static SkPath update_weight(const SkPath& path) {
    SkPath path_;
    for (auto [verb, pts, _] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                path_.moveTo(pts[0]);
                break;
            case SkPathVerb::kLine:
                path_.lineTo(pts[1]);
                break;
            case SkPathVerb::kQuad:
                path_.quadTo(pts[1], pts[2]);
                break;
            case SkPathVerb::kCubic:
                path_.cubicTo(pts[1], pts[2], pts[3]);
                break;
            case SkPathVerb::kConic:
                path_.conicTo(pts[1], pts[2], (kConicWeight != 1) ? kConicWeight : .99f);
                break;
            default:
                SkUNREACHABLE;
        }
    }
    dump_conic_max_errors(path);
    return path_;
}

bool TessellatedWedge::onChar(SkUnichar unichar) {
    switch (unichar) {
        case 'w':
            fOpFlags = (GrTessellationPathRenderer::OpFlags)(
                    (int)fOpFlags ^ (int)GrTessellationPathRenderer::OpFlags::kWireframe);
            return true;
        case 'D': {
            fPath.dump();
            return true;
        }
        case '+':
            kConicWeight *= 2;
            fPath = update_weight(fPath);
            return true;
        case '=':
            kConicWeight *= 5/4.f;
            fPath = update_weight(fPath);
            return true;
        case '_':
            kConicWeight *= .5f;
            fPath = update_weight(fPath);
            return true;
        case '-':
            kConicWeight *= 4/5.f;
            fPath = update_weight(fPath);
            return true;
    }
    return false;
}

Sample* MakeTessellatedWedgeSample() { return new TessellatedWedge; }
static SampleRegistry gTessellatedWedgeSample(MakeTessellatedWedgeSample);

#endif  // SK_SUPPORT_GPU
