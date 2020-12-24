/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/gpu/GrDirectContext.h"
#include "samplecode/Sample.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/ccpr/GrCCCoverageProcessor.h"
#include "src/gpu/ccpr/GrCCFillGeometry.h"
#include "src/gpu/ccpr/GrGSCoverageProcessor.h"
#include "src/gpu/ccpr/GrVSCoverageProcessor.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ops/GrDrawOp.h"

#ifdef SK_GL
#include "src/gpu/gl/GrGLGpu.h"
#endif

using TriPointInstance = GrCCCoverageProcessor::TriPointInstance;
using QuadPointInstance = GrCCCoverageProcessor::QuadPointInstance;
using PrimitiveType = GrCCCoverageProcessor::PrimitiveType;

static constexpr float kDebugBloat = 40;

/**
 * This sample visualizes the AA bloat geometry generated by the ccpr geometry shaders. It
 * increases the AA bloat by 50x and outputs color instead of coverage (coverage=+1 -> green,
 * coverage=0 -> black, coverage=-1 -> red). Use the keys 1-7 to cycle through the different
 * geometry processors.
 */
class CCPRGeometryView : public Sample {
    void onOnceBeforeDraw() override { this->updateGpuData(); }
    void onDrawContent(SkCanvas*) override;

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override;
    bool onClick(Sample::Click*) override;
    bool onChar(SkUnichar) override;
    SkString name() override { return SkString("CCPRGeometry"); }

    class Click;
    class DrawCoverageCountOp;
    class VisualizeCoverageCountFP;

    void updateAndInval() { this->updateGpuData(); }

    void updateGpuData();

    PrimitiveType fPrimitiveType = PrimitiveType::kCubics;

    SkPoint fPoints[4] = {
            {100.05f, 100.05f}, {400.75f, 100.05f}, {400.75f, 300.95f}, {100.05f, 300.95f}};

    float fConicWeight = .5;
    float fStrokeWidth = 40;
    SkPaint::Join fStrokeJoin = SkPaint::kMiter_Join;
    SkPaint::Cap fStrokeCap = SkPaint::kButt_Cap;
    bool fDoStroke = true;

    SkTArray<TriPointInstance> fTriPointInstances;
    SkTArray<QuadPointInstance> fQuadPointInstances;
    SkPath fPath;
};

class CCPRGeometryView::DrawCoverageCountOp : public GrDrawOp {
    DEFINE_OP_CLASS_ID

public:
    DrawCoverageCountOp(CCPRGeometryView* view) : INHERITED(ClassID()), fView(view) {
        this->setBounds(SkRect::MakeIWH(fView->width(), fView->height()), GrOp::HasAABloat::kNo,
                        GrOp::IsHairline::kNo);
    }

    const char* name() const override {
        return "[Testing/Sample code] CCPRGeometryView::DrawCoverageCountOp";
    }

private:
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override {}
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    CCPRGeometryView* fView;

    using INHERITED = GrDrawOp;
};

class CCPRGeometryView::VisualizeCoverageCountFP : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP) {
        return std::unique_ptr<GrFragmentProcessor>(
                new VisualizeCoverageCountFP(std::move(inputFP)));
    }

private:
    const char* name() const override { return "VisualizeCoverageCountFP"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new VisualizeCoverageCountFP(*this));
    }
    VisualizeCoverageCountFP(std::unique_ptr<GrFragmentProcessor> inputFP)
            : GrFragmentProcessor(kTestFP_ClassID, kNone_OptimizationFlags) {
        this->registerChild(std::move(inputFP));
    }
    VisualizeCoverageCountFP(const VisualizeCoverageCountFP& that)
            : GrFragmentProcessor(kTestFP_ClassID, kNone_OptimizationFlags) {
        this->cloneAndRegisterAllChildProcessors(that);
    }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    class Impl : public GrGLSLFragmentProcessor {
        void emitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* f = args.fFragBuilder;
            static constexpr int kInputFPIndex = 0;
            SkString inputColor = this->invokeChild(kInputFPIndex, args);
            f->codeAppendf("half count = %s.a;", inputColor.c_str());
            f->codeAppendf("return half4(saturate(-count), saturate(+count), 0, abs(count));");
        }
    };

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new Impl; }
};

void CCPRGeometryView::onDrawContent(SkCanvas* canvas) {
    canvas->clear(SK_ColorBLACK);

    SkPaint outlinePaint;
    outlinePaint.setColor(0xff808080);
    outlinePaint.setStyle(SkPaint::kStroke_Style);
    if (fDoStroke) {
        outlinePaint.setStrokeWidth(fStrokeWidth);
    } else {
        outlinePaint.setStrokeWidth(0);
    }
    outlinePaint.setStrokeJoin(fStrokeJoin);
    outlinePaint.setStrokeCap(fStrokeCap);
    outlinePaint.setAntiAlias(true);
    canvas->drawPath(fPath, outlinePaint);

#if 0
    SkPaint gridPaint;
    gridPaint.setColor(0x10000000);
    gridPaint.setStyle(SkPaint::kStroke_Style);
    gridPaint.setStrokeWidth(0);
    gridPaint.setAntiAlias(true);
    for (int y = 0; y < this->height(); y += kDebugBloat) {
        canvas->drawLine(0, y, this->width(), y, gridPaint);
    }
    for (int x = 0; x < this->width(); x += kDebugBloat) {
        canvas->drawLine(x, 0, x, this->height(), outlinePaint);
    }
#endif

    SkString caption;
    caption.appendf("PrimitiveType_%s",
                    GrCCCoverageProcessor::PrimitiveTypeName(fPrimitiveType));
    if (PrimitiveType::kCubics == fPrimitiveType) {
        caption.appendf(" (%s)", SkCubicTypeName(SkClassifyCubic(fPoints)));
    } else if (PrimitiveType::kConics == fPrimitiveType) {
        caption.appendf(" (w=%f)", fConicWeight);
    }

    if (fDoStroke) {
        caption.appendf(" (stroke_width=%f)", fStrokeWidth);
    } else if (GrSurfaceDrawContext* sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas)) {
        // Render coverage count.
        auto ctx = canvas->recordingContext();
        SkASSERT(ctx);

        int width = this->width();
        int height = this->height();
        auto ccbuff = GrSurfaceDrawContext::Make(
                ctx, GrColorType::kAlpha_F16, nullptr, SkBackingFit::kApprox, {width, height});
        SkASSERT(ccbuff);
        ccbuff->clear(SK_PMColor4fTRANSPARENT);
        ccbuff->addDrawOp(GrOp::Make<DrawCoverageCountOp>(ctx, this));

        // Visualize coverage count in main canvas.
        GrPaint paint;
        paint.setColorFragmentProcessor(VisualizeCoverageCountFP::Make(
                GrTextureEffect::Make(ccbuff->readSurfaceView(), ccbuff->colorInfo().alphaType())));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrcOver);
        sdc->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                      SkRect::MakeIWH(this->width(), this->height()));
    } else {
        caption = "Use GPU backend to visualize geometry.";
    }

    SkPaint pointsPaint;
    pointsPaint.setColor(SK_ColorBLUE);
    pointsPaint.setStrokeWidth(8);
    pointsPaint.setAntiAlias(true);

    if (PrimitiveType::kCubics == fPrimitiveType) {
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, fPoints, pointsPaint);
    } else {
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, fPoints, pointsPaint);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 1, fPoints + 3, pointsPaint);
    }

    SkFont font(nullptr, 20);
    SkPaint captionPaint;
    captionPaint.setColor(SK_ColorWHITE);
    canvas->drawString(caption, 10, 30, font, captionPaint);
}

void CCPRGeometryView::updateGpuData() {
    using Verb = GrCCFillGeometry::Verb;
    fTriPointInstances.reset();
    fQuadPointInstances.reset();

    fPath.reset();
    fPath.moveTo(fPoints[0]);

    if (PrimitiveType::kCubics == fPrimitiveType) {
        GrCCFillGeometry geometry;
        geometry.beginContour(fPoints[0]);
        geometry.cubicTo(fPoints, kDebugBloat / 2, kDebugBloat / 2);
        geometry.endContour();
        int ptsIdx = 0;
        for (Verb verb : geometry.verbs()) {
            switch (verb) {
                case Verb::kLineTo:
                    ++ptsIdx;
                    continue;
                case Verb::kMonotonicQuadraticTo:
                    ptsIdx += 2;
                    continue;
                case Verb::kMonotonicCubicTo:
                    fQuadPointInstances.push_back().set(&geometry.points()[ptsIdx], 0, 0);
                    ptsIdx += 3;
                    continue;
                default:
                    continue;
            }
        }
        fPath.cubicTo(fPoints[1], fPoints[2], fPoints[3]);
    } else if (PrimitiveType::kTriangles != fPrimitiveType) {
        SkPoint P3[3] = {fPoints[0], fPoints[1], fPoints[3]};
        GrCCFillGeometry geometry;
        geometry.beginContour(P3[0]);
        if (PrimitiveType::kQuadratics == fPrimitiveType) {
            geometry.quadraticTo(P3);
            fPath.quadTo(fPoints[1], fPoints[3]);
        } else {
            SkASSERT(PrimitiveType::kConics == fPrimitiveType);
            geometry.conicTo(P3, fConicWeight);
            fPath.conicTo(fPoints[1], fPoints[3], fConicWeight);
        }
        geometry.endContour();
        int ptsIdx = 0, conicWeightIdx = 0;
        for (Verb verb : geometry.verbs()) {
            if (Verb::kBeginContour == verb ||
                Verb::kEndOpenContour == verb ||
                Verb::kEndClosedContour == verb) {
                continue;
            }
            if (Verb::kLineTo == verb) {
                ++ptsIdx;
                continue;
            }
            SkASSERT(Verb::kMonotonicQuadraticTo == verb || Verb::kMonotonicConicTo == verb);
            if (PrimitiveType::kQuadratics == fPrimitiveType &&
                Verb::kMonotonicQuadraticTo == verb) {
                fTriPointInstances.push_back().set(
                        &geometry.points()[ptsIdx], Sk2f(0, 0),
                        TriPointInstance::Ordering::kXYTransposed);
            } else if (PrimitiveType::kConics == fPrimitiveType &&
                       Verb::kMonotonicConicTo == verb) {
                fQuadPointInstances.push_back().setW(&geometry.points()[ptsIdx], Sk2f(0, 0),
                                                     geometry.getConicWeight(conicWeightIdx++));
            }
            ptsIdx += 2;
        }
    } else {
        fTriPointInstances.push_back().set(
                fPoints[0], fPoints[1], fPoints[3], Sk2f(0, 0),
                TriPointInstance::Ordering::kXYTransposed);
        fPath.lineTo(fPoints[1]);
        fPath.lineTo(fPoints[3]);
        fPath.close();
    }
}

void CCPRGeometryView::DrawCoverageCountOp::onExecute(GrOpFlushState* state,
                                                      const SkRect& chainBounds) {
    GrResourceProvider* rp = state->resourceProvider();
#ifdef SK_GL
    auto direct = state->gpu()->getContext();
    GrGLGpu* glGpu = GrBackendApi::kOpenGL == direct->backend()
                             ? static_cast<GrGLGpu*>(state->gpu())
                             : nullptr;
    if (glGpu) {
        glGpu->handleDirtyContext();
        // GR_GL_CALL(glGpu->glInterface(), PolygonMode(GR_GL_FRONT_AND_BACK, GR_GL_LINE));
        GR_GL_CALL(glGpu->glInterface(), Enable(GR_GL_LINE_SMOOTH));
    }
#endif

    GrPipeline pipeline(GrScissorTest::kDisabled, SkBlendMode::kPlus,
                        state->drawOpArgs().writeView().swizzle());

    std::unique_ptr<GrCCCoverageProcessor> proc;
    if (state->caps().shaderCaps()->geometryShaderSupport()) {
        proc = std::make_unique<GrGSCoverageProcessor>();
    } else {
        proc = std::make_unique<GrVSCoverageProcessor>();
    }
    SkDEBUGCODE(proc->enableDebugBloat(kDebugBloat));

    GrOpsRenderPass* renderPass = state->opsRenderPass();

    for (int i = 0; i < proc->numSubpasses(); ++i) {
        proc->reset(fView->fPrimitiveType, i, rp);
        proc->bindPipeline(state, pipeline, this->bounds());

        if (PrimitiveType::kCubics == fView->fPrimitiveType ||
            PrimitiveType::kConics == fView->fPrimitiveType) {
            sk_sp<GrGpuBuffer> instBuff(rp->createBuffer(
                    fView->fQuadPointInstances.count() * sizeof(QuadPointInstance),
                    GrGpuBufferType::kVertex, kDynamic_GrAccessPattern,
                    fView->fQuadPointInstances.begin()));
            if (!fView->fQuadPointInstances.empty() && instBuff) {
                proc->bindBuffers(renderPass, std::move(instBuff));
                proc->drawInstances(renderPass, fView->fQuadPointInstances.count(), 0);
            }
        } else {
            sk_sp<GrGpuBuffer> instBuff(rp->createBuffer(
                    fView->fTriPointInstances.count() * sizeof(TriPointInstance),
                    GrGpuBufferType::kVertex, kDynamic_GrAccessPattern,
                    fView->fTriPointInstances.begin()));
            if (!fView->fTriPointInstances.empty() && instBuff) {
                proc->bindBuffers(renderPass, std::move(instBuff));
                proc->drawInstances(renderPass, fView->fTriPointInstances.count(), 0);
            }
        }
    }

#ifdef SK_GL
    if (glGpu) {
        direct->resetContext(kMisc_GrGLBackendState);
    }
#endif
}

class CCPRGeometryView::Click : public Sample::Click {
public:
    Click(int ptIdx) : fPtIdx(ptIdx) {}

    void doClick(SkPoint points[]) {
        if (fPtIdx >= 0) {
            points[fPtIdx] += fCurr - fPrev;
        } else {
            for (int i = 0; i < 4; ++i) {
                points[i] += fCurr - fPrev;
            }
        }
    }

private:
    int fPtIdx;
};

Sample::Click* CCPRGeometryView::onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) {
    for (int i = 0; i < 4; ++i) {
        if (PrimitiveType::kCubics != fPrimitiveType && 2 == i) {
            continue;
        }
        if (fabs(x - fPoints[i].x()) < 20 && fabsf(y - fPoints[i].y()) < 20) {
            return new Click(i);
        }
    }
    return new Click(-1);
}

bool CCPRGeometryView::onClick(Sample::Click* click) {
    Click* myClick = (Click*)click;
    myClick->doClick(fPoints);
    this->updateAndInval();
    return true;
}

bool CCPRGeometryView::onChar(SkUnichar unichar) {
        if (unichar >= '1' && unichar <= '4') {
            fPrimitiveType = PrimitiveType(unichar - '1');
            if (fPrimitiveType >= PrimitiveType::kWeightedTriangles) {
                fPrimitiveType = (PrimitiveType) ((int)fPrimitiveType + 1);
            }
            this->updateAndInval();
            return true;
        }
        float* valueToScale = nullptr;
        if (PrimitiveType::kConics == fPrimitiveType) {
            valueToScale = &fConicWeight;
        } else if (fDoStroke) {
            valueToScale = &fStrokeWidth;
        }
        if (valueToScale) {
            if (unichar == '+') {
                *valueToScale *= 2;
                this->updateAndInval();
                return true;
            }
            if (unichar == '+' || unichar == '=') {
                *valueToScale *= 5/4.f;
                this->updateAndInval();
                return true;
            }
            if (unichar == '-') {
                *valueToScale *= 4/5.f;
                this->updateAndInval();
                return true;
            }
            if (unichar == '_') {
                *valueToScale *= .5f;
                this->updateAndInval();
                return true;
            }
        }
        if (unichar == 'D') {
            SkDebugf("    SkPoint fPoints[4] = {\n");
            SkDebugf("        {%ff, %ff},\n", fPoints[0].x(), fPoints[0].y());
            SkDebugf("        {%ff, %ff},\n", fPoints[1].x(), fPoints[1].y());
            SkDebugf("        {%ff, %ff},\n", fPoints[2].x(), fPoints[2].y());
            SkDebugf("        {%ff, %ff}\n", fPoints[3].x(), fPoints[3].y());
            SkDebugf("    };\n");
            return true;
        }
        if (unichar == 'S') {
            fDoStroke = !fDoStroke;
            this->updateAndInval();
            return true;
        }
        if (unichar == 'J') {
            fStrokeJoin = (SkPaint::Join)((fStrokeJoin + 1) % 3);
            this->updateAndInval();
            return true;
        }
        if (unichar == 'C') {
            fStrokeCap = (SkPaint::Cap)((fStrokeCap + 1) % 3);
            this->updateAndInval();
            return true;
        }
        return false;
}

DEF_SAMPLE(return new CCPRGeometryView;)

#endif  // SK_SUPPORT_GPU
