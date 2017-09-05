/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkGeometry.h"
#include "SkMakeUnique.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkView.h"
#include "ccpr/GrCCPRCoverageProcessor.h"
#include "ccpr/GrCCPRGeometry.h"
#include "gl/GrGLGpu.cpp"
#include "ops/GrDrawOp.h"

using TriangleInstance = GrCCPRCoverageProcessor::TriangleInstance;
using CurveInstance = GrCCPRCoverageProcessor::CurveInstance;
using Mode = GrCCPRCoverageProcessor::Mode;

static int num_points(Mode mode)  {
    return mode >= Mode::kSerpentineInsets ? 4 : 3;
}

static int is_quadratic(Mode mode)  {
    return mode >= Mode::kQuadraticHulls && mode < Mode::kSerpentineInsets;
}

/**
 * This sample visualizes the AA bloat geometry generated by the ccpr geometry shaders. It
 * increases the AA bloat by 50x and outputs color instead of coverage (coverage=+1 -> green,
 * coverage=0 -> black, coverage=-1 -> red). Use the keys 1-7 to cycle through the different
 * geometry processors.
 */
class CCPRGeometryView : public SampleView {
public:
    CCPRGeometryView() { this->updateGpuData(); }
    void onDrawContent(SkCanvas*) override;

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned) override;
    bool onClick(SampleView::Click*) override;
    bool onQuery(SkEvent* evt) override;

private:
    class Click;
    class Op;

    void updateAndInval() {
        this->updateGpuData();
        this->inval(nullptr);
    }

    void updateGpuData();

    Mode fMode = Mode::kTriangleHulls;

    SkPoint fPoints[4] = {
        {100.05f, 100.05f},
        {100.05f, 300.95f},
        {400.75f, 300.95f},
        {400.75f, 100.05f}
    };

    SkTArray<SkPoint>   fGpuPoints;
    SkTArray<int32_t>   fInstanceData;
    int                 fInstanceCount;

    typedef SampleView INHERITED;
};

class CCPRGeometryView::Op : public GrDrawOp {
    DEFINE_OP_CLASS_ID

public:
    Op(CCPRGeometryView* view)
            : INHERITED(ClassID())
            , fView(view) {
        this->setBounds(SkRect::MakeLargest(), GrOp::HasAABloat::kNo, GrOp::IsZeroArea::kNo);
    }

    const char* name() const override { return "[Testing/Sample code] CCPRGeometryView::Op"; }

private:
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override {
        return RequiresDstTexture::kNo;
    }
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState*) override;

    CCPRGeometryView* fView;

    typedef GrDrawOp INHERITED;
};

void CCPRGeometryView::onDrawContent(SkCanvas* canvas) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->setMatrix(SkMatrix::I());

    SkPath outline;
    outline.moveTo(fPoints[0]);
    if (4 == num_points(fMode)) {
        outline.cubicTo(fPoints[1], fPoints[2], fPoints[3]);
    } else if (is_quadratic(fMode)) {
        outline.quadTo(fPoints[1], fPoints[3]);
    } else {
        outline.lineTo(fPoints[1]);
        outline.lineTo(fPoints[3]);
    }
    outline.close();

    SkPaint outlinePaint;
    outlinePaint.setColor(0x30000000);
    outlinePaint.setStyle(SkPaint::kStroke_Style);
    outlinePaint.setStrokeWidth(0);
    outlinePaint.setAntiAlias(true);

    canvas->drawPath(outline, outlinePaint);

    const char* caption = "Use GPU backend to visualize geometry.";

    if (GrRenderTargetContext* rtc =
        canvas->internal_private_accessTopLayerRenderTargetContext()) {
        rtc->priv().testingOnly_addDrawOp(skstd::make_unique<Op>(this));
        caption = GrCCPRCoverageProcessor::GetProcessorName(fMode);
    }

    SkPaint pointsPaint;
    pointsPaint.setColor(SK_ColorBLUE);
    pointsPaint.setStrokeWidth(8);
    pointsPaint.setAntiAlias(true);

    if (4 == num_points(fMode)) {
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, fPoints, pointsPaint);
    } else {
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, fPoints, pointsPaint);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 1, fPoints + 3, pointsPaint);
    }

    SkPaint captionPaint;
    captionPaint.setTextSize(20);
    captionPaint.setColor(SK_ColorBLACK);
    captionPaint.setAntiAlias(true);
    canvas->drawText(caption, strlen(caption), 10, 30, captionPaint);
}

void CCPRGeometryView::updateGpuData() {
    int vertexCount = num_points(fMode);

    fGpuPoints.reset();
    fInstanceData.reset();
    fInstanceCount = 0;

    if (4 == vertexCount) {
        double t[2], s[2];
        SkCubicType type = SkClassifyCubic(fPoints, t, s);
        SkSTArray<2, float> chops;
        for (int i = 0; i < 2; ++i) {
            float chop = t[i] / s[i];
            if (chop > 0 && chop < 1) {
                chops.push_back(chop);
            }
        }

        int instanceCount = chops.count() + 1;
        SkPoint chopped[10];
        SkChopCubicAt(fPoints, chopped, chops.begin(), chops.count());

        fGpuPoints.push_back(chopped[0]);
        for (int i = 0; i < instanceCount; ++i) {
            fGpuPoints.push_back(chopped[3*i + 1]);
            fGpuPoints.push_back(chopped[3*i + 2]);
            if (3 == instanceCount && SkCubicType::kLoop == type) {
                fGpuPoints.push_back(chopped[3*i]); // Account for floating point error.
            } else {
                fGpuPoints.push_back(chopped[3*i + 3]);
            }
        }

        if (fMode < Mode::kLoopInsets && SkCubicType::kLoop == type) {
            fMode = (Mode) ((int) fMode + 2);
        }
        if (fMode >= Mode::kLoopInsets && SkCubicType::kLoop != type) {
            fMode = (Mode) ((int) fMode - 2);
        }

        for (int i = 0; i < instanceCount; ++i) {
            fInstanceData.push_back(3*i);
            fInstanceData.push_back(0); // Atlas offset.
            ++fInstanceCount;
        }
    } else if (is_quadratic(fMode)) {
        GrCCPRGeometry geometry;
        geometry.beginContour(fPoints[0]);
        geometry.quadraticTo(fPoints[1], fPoints[3]);
        geometry.endContour();
        fGpuPoints.push_back_n(geometry.points().count(), geometry.points().begin());
        for (GrCCPRGeometry::Verb verb : geometry.verbs()) {
            if (GrCCPRGeometry::Verb::kBeginContour == verb ||
                GrCCPRGeometry::Verb::kEndOpenContour == verb ||
                GrCCPRGeometry::Verb::kEndClosedContour == verb) {
                continue;
            }
            SkASSERT(GrCCPRGeometry::Verb::kMonotonicQuadraticTo == verb);
            fInstanceData.push_back(2 * fInstanceCount++); // Pts idx.
            fInstanceData.push_back(0); // Atlas offset.
        }
    } else {
        fGpuPoints.push_back(fPoints[0]);
        fGpuPoints.push_back(fPoints[1]);
        fGpuPoints.push_back(fPoints[3]);
        fInstanceData.push_back(0);
        fInstanceData.push_back(1);
        fInstanceData.push_back(2);
        fInstanceData.push_back(0); // Atlas offset.
        fInstanceCount = 1;
    }
}

void CCPRGeometryView::Op::onExecute(GrOpFlushState* state) {
    GrResourceProvider* rp = state->resourceProvider();
    GrContext* context = state->gpu()->getContext();
    GrGLGpu* glGpu = kOpenGL_GrBackend == context->contextPriv().getBackend() ?
                     static_cast<GrGLGpu*>(state->gpu()) : nullptr;
    int vertexCount = num_points(fView->fMode);

    sk_sp<GrBuffer> pointsBuffer(rp->createBuffer(fView->fGpuPoints.count() * sizeof(SkPoint),
                                                  kTexel_GrBufferType, kDynamic_GrAccessPattern,
                                                  GrResourceProvider::kNoPendingIO_Flag |
                                                  GrResourceProvider::kRequireGpuMemory_Flag,
                                                  fView->fGpuPoints.begin()));
    if (!pointsBuffer) {
        return;
    }

    sk_sp<GrBuffer> instanceBuffer(rp->createBuffer(fView->fInstanceData.count() * sizeof(int),
                                                    kVertex_GrBufferType, kDynamic_GrAccessPattern,
                                                    GrResourceProvider::kNoPendingIO_Flag |
                                                    GrResourceProvider::kRequireGpuMemory_Flag,
                                                    fView->fInstanceData.begin()));
    if (!instanceBuffer) {
        return;
    }

    GrPipeline pipeline(state->drawOpArgs().fProxy, GrPipeline::ScissorState::kDisabled,
                        SkBlendMode::kSrcOver);

    GrCCPRCoverageProcessor ccprProc(fView->fMode, pointsBuffer.get());
    SkDEBUGCODE(ccprProc.enableDebugVisualizations();)

    GrMesh mesh(4 == vertexCount ?  GrPrimitiveType::kLinesAdjacency : GrPrimitiveType::kTriangles);
    mesh.setInstanced(instanceBuffer.get(), fView->fInstanceCount, 0, vertexCount);

    if (glGpu) {
        glGpu->handleDirtyContext();
        GR_GL_CALL(glGpu->glInterface(), PolygonMode(GR_GL_FRONT_AND_BACK, GR_GL_LINE));
        GR_GL_CALL(glGpu->glInterface(), Enable(GR_GL_LINE_SMOOTH));
    }

    state->rtCommandBuffer()->draw(pipeline, ccprProc, &mesh, nullptr, 1, this->bounds());

    if (glGpu) {
        context->resetContext(kMisc_GrGLBackendState);
    }
}

class CCPRGeometryView::Click : public SampleView::Click {
public:
    Click(SkView* target, int ptIdx) : SampleView::Click(target), fPtIdx(ptIdx) {}

    void doClick(SkPoint points[]) {
        if (fPtIdx >= 0) {
            this->dragPoint(points, fPtIdx);
        } else {
            for (int i = 0; i < 4; ++i) {
                this->dragPoint(points, i);
            }
        }
    }

private:
    void dragPoint(SkPoint points[], int idx)  {
        SkIPoint delta = fICurr - fIPrev;
        points[idx] += SkPoint::Make(delta.x(), delta.y());
    }

    int fPtIdx;
};

SkView::Click* CCPRGeometryView::onFindClickHandler(SkScalar x, SkScalar y, unsigned) {
    for (int i = 0; i < 4; ++i) {
        if (4 != num_points(fMode) && 2 == i) {
            continue;
        }
        if (fabs(x - fPoints[i].x()) < 20 && fabsf(y - fPoints[i].y()) < 20) {
            return new Click(this, i);
        }
    }
    return new Click(this, -1);
}

bool CCPRGeometryView::onClick(SampleView::Click* click) {
    Click* myClick = (Click*) click;
    myClick->doClick(fPoints);
    this->updateAndInval();
    return true;
}

bool CCPRGeometryView::onQuery(SkEvent* evt) {
    if (SampleCode::TitleQ(*evt)) {
        SampleCode::TitleR(evt, "CCPRGeometry");
        return true;
    }
    SkUnichar unichar;
    if (SampleCode::CharQ(*evt, &unichar)) {
        if (unichar >= '1' && unichar <= '7') {
            fMode = Mode(unichar - '1');
            if (fMode >= Mode::kCombinedTriangleHullsAndEdges) {
                fMode = Mode(int(fMode) + 1);
            }
            this->updateAndInval();
            return true;
        }
        if (unichar == 'D') {
            SkDebugf("    SkPoint fPoints[4] = {\n");
            SkDebugf("        {%f, %f},\n", fPoints[0].x(), fPoints[0].y());
            SkDebugf("        {%f, %f},\n", fPoints[1].x(), fPoints[1].y());
            SkDebugf("        {%f, %f},\n", fPoints[2].x(), fPoints[2].y());
            SkDebugf("        {%f, %f}\n", fPoints[3].x(), fPoints[3].y());
            SkDebugf("    };\n");
            return true;
        }
    }
    return this->INHERITED::onQuery(evt);
}

DEF_SAMPLE( return new CCPRGeometryView; )

#endif // SK_SUPPORT_GPU
