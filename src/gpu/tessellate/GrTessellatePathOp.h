/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellatePathOp_DEFINED
#define GrTessellatePathOp_DEFINED

#include "src/gpu/ops/GrDrawOp.h"

class GrAppliedHardClip;
class GrFillPathShader;
class GrStencilPathShader;

// Renders paths using a hybrid Red Book "stencil, then cover" method. Curves get linearized by
// GPU tessellation shaders. This Op doesn't apply analytic AA, so it requires a render target that
// supports either MSAA or mixed samples if AA is desired.
class GrTessellatePathOp : public GrDrawOp {
public:
    enum class Flags {
        kNone = 0,
        kStencilOnly = (1 << 0),
        kWireframe = (1 << 1)
    };

private:
    DEFINE_OP_CLASS_ID

    GrTessellatePathOp(const SkMatrix& viewMatrix, const SkPath& path, GrPaint&& paint,
                       GrAAType aaType, Flags flags = Flags::kNone)
            : GrDrawOp(ClassID())
            , fFlags(flags)
            , fViewMatrix(viewMatrix)
            , fPath(path)
            , fAAType(aaType)
            , fColor(paint.getColor4f())
            , fProcessors(std::move(paint)) {
        SkRect devBounds;
        fViewMatrix.mapRect(&devBounds, path.getBounds());
        this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
    }

    const char* name() const override { return "GrTessellatePathOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override { fProcessors.visitProxies(fn); }
    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      bool hasMixedSampledCoverage,
                                      GrClampType clampType) override {
        return fProcessors.finalize(
                fColor, GrProcessorAnalysisCoverage::kNone, clip, &GrUserStencilSettings::kUnused,
                hasMixedSampledCoverage, caps, clampType, &fColor);
    }

    FixedFunctionFlags fixedFunctionFlags() const override;
    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView*, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&) override;
    void onPrepare(GrOpFlushState* state) override;

    // Triangulates and writes the SkPath's inner polygon(s). The inner polygons connect the
    // endpoints of each verb. (i.e., they are the path that would result from collapsing all curves
    // to single lines.) Stencilled together with the outer cubics, these define the complete path.
    //
    // This method works by recursively subdividing the path rather than emitting a linear triangle
    // fan or strip. This can reduce the load on the rasterizer by a great deal on complex paths.
    bool prepareInnerTriangles(GrOpFlushState*, int* numCountedCurves);

    // Writes an array of "outer" cubics from each bezier in the SkPath, converting any quadratics
    // to cubics. An outer cubic is an independent, 4-point closed contour consisting of a single
    // cubic curve. Stencilled together with the inner triangles, these define the complete path.
    void prepareOuterCubics(GrOpFlushState* flushState, int numCountedCurves);

    // Writes an array of cubic "wedges" from the SkPath, converting any lines or quadratics to
    // cubics. A wedge is an independent, 5-point closed contour consisting of 4 cubic control
    // points plus an anchor point fanning from the center of the curve's resident contour. Once
    // stencilled, these wedges alone define the complete path.
    //
    // TODO: Eventually we want to use rational cubic wedges in order to support conics.
    bool prepareCubicWedges(GrOpFlushState*);

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;
    void drawStencilPass(GrOpFlushState*);
    void drawCoverPass(GrOpFlushState*);

    const Flags fFlags;
    const SkMatrix fViewMatrix;
    const SkPath fPath;
    const GrAAType fAAType;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

    // These path shaders get created during onPrepare for drawing the below path vertex data.
    //
    // If fFillPathShader is null, then we just stencil the full path using fStencilPathShader and
    // fCubicInstanceBuffer, and then fill it using a simple bounding box.
    //
    // If fFillPathShader is not null, then we fill the path using it plus cubic hulls from
    // fCubicInstanceBuffer instead of a bounding box.
    //
    // If fFillPathShader is not null and fStencilPathShader *is* null, then the vertex data
    // contains non-overlapping path geometry that can be drawn directly to the final render target.
    // We only need to stencil curves from fCubicInstanceBuffer, and then draw the rest of the path
    // directly.
    GrStencilPathShader* fStencilPathShader = nullptr;
    GrFillPathShader* fFillPathShader = nullptr;

    // The "path vertex data" is made up of cubic wedges or inner polygon triangles (either red book
    // style or fully tessellated). The geometry is generated by
    // GrPathParser::EmitCenterWedgePatches, GrPathParser::EmitInnerPolygonTriangles,
    // or GrTriangulator::PathToTriangles.
    sk_sp<const GrBuffer> fPathVertexBuffer;
    int fBasePathVertex;
    int fPathVertexCount;

    // The cubic instance buffer defines standalone cubics to tessellate into the stencil buffer, in
    // addition to the above path geometry.
    sk_sp<const GrBuffer> fCubicInstanceBuffer;
    int fBaseCubicInstance;
    int fCubicInstanceCount;

    friend class GrOpMemoryPool;  // For ctor.
};

GR_MAKE_BITFIELD_CLASS_OPS(GrTessellatePathOp::Flags);

#endif
