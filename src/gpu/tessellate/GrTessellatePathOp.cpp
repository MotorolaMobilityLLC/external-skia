/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellatePathOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrTriangulator.h"
#include "src/gpu/tessellate/GrFillPathShader.h"
#include "src/gpu/tessellate/GrInnerPolygonContourParser.h"
#include "src/gpu/tessellate/GrMidpointContourParser.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"

GrTessellatePathOp::FixedFunctionFlags GrTessellatePathOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

void GrTessellatePathOp::onPrePrepare(GrRecordingContext*,
                                      const GrSurfaceProxyView* writeView,
                                      GrAppliedClip*,
                                      const GrXferProcessor::DstProxyView&) {
}

void GrTessellatePathOp::onPrepare(GrOpFlushState* state) {
    // First check if the path is large and/or simple enough that we can actually tessellate the
    // inner polygon(s) on the CPU. This is our fastest approach. It allows us to stencil only the
    // curves, and then draw the internal polygons directly to the final render target, thus filling
    // in the majority of pixels in a single render pass.
    SkScalar scales[2];
    SkAssertResult(fViewMatrix.getMinMaxScales(scales));  // Will fail if perspective.
    const SkRect& bounds = fPath.getBounds();
    int numVerbs = fPath.countVerbs();
    if (numVerbs <= 0) {
        return;
    }
    float gpuFragmentWork = bounds.height() * scales[0] * bounds.width() * scales[1];
    float cpuTessellationWork = (float)numVerbs * SkNextLog2(numVerbs);  // N log N.
    if (cpuTessellationWork * 500 + (256 * 256) < gpuFragmentWork) {  // Don't try below 256x256.
        GrEagerDynamicVertexAllocator pathVertexAllocator(state, &fPathVertexBuffer,
                                                          &fBasePathVertex);
        int numCountedCurves;
        // PathToTriangles(..kSimpleInnerPolygon..) will fail if the inner polygon is not simple.
        if ((fPathVertexCount = GrTriangulator::PathToTriangles(
                fPath, 0, SkRect::MakeEmpty(), &pathVertexAllocator,
                GrTriangulator::Mode::kSimpleInnerPolygons, &numCountedCurves))) {
            if (((Flags::kStencilOnly | Flags::kWireframe) & fFlags) ||
                GrAAType::kCoverage == fAAType ||
                (state->appliedClip() && state->appliedClip()->hasStencilClip())) {
                // If we have certain flags, mixed samples, or a stencil clip then we unfortunately
                // can't fill the inner polygon directly. Create a stencil shader here to ensure we
                // still stencil the entire path.
                fStencilPathShader = state->allocator()->make<GrStencilTriangleShader>(fViewMatrix);
            }
            if (!(Flags::kStencilOnly & fFlags)) {
                fFillPathShader = state->allocator()->make<GrFillTriangleShader>(
                        fViewMatrix, fColor);
            }
            this->prepareOuterCubics(state, numCountedCurves);
            return;
        }
    }

    // Next see if we can split up inner polygon triangles and curves, and triangulate the inner
    // polygon(s) more efficiently. This causes greater CPU overhead due to the extra shaders and
    // draw calls, but the better triangulation can reduce the rasterizer load by a great deal on
    // complex paths.
    // NOTE: Raster-edge work is 1-dimensional, so we sum height and width instead of multiplying.
    float rasterEdgeWork = (bounds.height() + bounds.width()) * scales[1] * fPath.countVerbs();
    if (rasterEdgeWork > 1000 * 1000) {
        int numCountedCurves;
        if (this->prepareInnerTriangles(state, &numCountedCurves)) {
            fStencilPathShader = state->allocator()->make<GrStencilTriangleShader>(fViewMatrix);
        }
        this->prepareOuterCubics(state, numCountedCurves);
        return;
    }

    // Fastest CPU approach: emit one cubic wedge per verb, fanning out from the center.
    if (this->prepareCubicWedges(state)) {
        fStencilPathShader = state->allocator()->make<GrStencilWedgeShader>(fViewMatrix);
    }
}

bool GrTessellatePathOp::prepareInnerTriangles(GrOpFlushState* flushState, int* numCountedCurves) {
    // No initial moveTo, plus an implicit close at the end; n-2 trianles fill an n-gon.
    // Each triangle has 3 vertices.
    int maxVertices = (fPath.countVerbs() - 1) * 3;

    GrEagerDynamicVertexAllocator vertexAlloc(flushState, &fPathVertexBuffer, &fBasePathVertex);
    auto* vertexData = vertexAlloc.lock<SkPoint>(maxVertices);
    if (!vertexData) {
        return false;
    }
    fPathVertexCount = 0;

    GrInnerPolygonContourParser parser(fPath, maxVertices);
    while (parser.parseNextContour()) {
        fPathVertexCount += parser.emitInnerPolygon(vertexData + fPathVertexCount);
    }
    *numCountedCurves = parser.numCountedCurves();

    vertexAlloc.unlock(fPathVertexCount);
    return SkToBool(fPathVertexCount);
}

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
}

static SkPoint write_line_as_cubic(SkPoint* data, const SkPoint& p0, const SkPoint& p1) {
    data[0] = p0;
    data[1] = lerp(p0, p1, 1/3.f);
    data[2] = lerp(p0, p1, 2/3.f);
    data[3] = p1;
    return data[3];
}

static SkPoint write_quadratic_as_cubic(SkPoint* data, const SkPoint& p0, const SkPoint& p1,
                                        const SkPoint& p2) {
    data[0] = p0;
    data[1] = lerp(p0, p1, 2/3.f);
    data[2] = lerp(p1, p2, 1/3.f);
    data[3] = p2;
    return data[3];
}

static SkPoint write_cubic(SkPoint* data, const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                           const SkPoint& p3) {
    data[0] = p0;
    data[1] = p1;
    data[2] = p2;
    data[3] = p3;
    return data[3];
}

void GrTessellatePathOp::prepareOuterCubics(GrOpFlushState* flushState, int numCountedCurves) {
    SkASSERT(!fCubicInstanceBuffer);

    if (numCountedCurves == 0) {
        return;
    }

    auto* instanceData = static_cast<std::array<SkPoint, 4>*>(flushState->makeVertexSpace(
            sizeof(SkPoint) * 4, numCountedCurves, &fCubicInstanceBuffer, &fBaseCubicInstance));
    if (!instanceData) {
        return;
    }
    fCubicInstanceCount = 0;

    SkPath::Iter iter(fPath, false);
    SkPath::Verb verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (SkPath::kQuad_Verb == verb) {
            SkASSERT(fCubicInstanceCount < numCountedCurves);
            write_quadratic_as_cubic(
                    instanceData[fCubicInstanceCount++].data(), pts[0], pts[1], pts[2]);
            continue;
        }
        if (SkPath::kCubic_Verb == verb) {
            SkASSERT(fCubicInstanceCount < numCountedCurves);
            memcpy(instanceData[fCubicInstanceCount++].data(), pts, sizeof(SkPoint) * 4);
            continue;
        }
    }
    SkASSERT(fCubicInstanceCount == numCountedCurves);
}

bool GrTessellatePathOp::prepareCubicWedges(GrOpFlushState* flushState) {
    // No initial moveTo, one wedge per verb, plus an implicit close at the end.
    // Each wedge has 5 vertices.
    int maxVertices = (fPath.countVerbs() + 1) * 5;

    GrEagerDynamicVertexAllocator vertexAlloc(flushState, &fPathVertexBuffer, &fBasePathVertex);
    auto* vertexData = vertexAlloc.lock<SkPoint>(maxVertices);
    if (!vertexData) {
        return false;
    }
    fPathVertexCount = 0;

    GrMidpointContourParser parser(fPath);
    while (parser.parseNextContour()) {
        int ptsIdx = 0;
        SkPoint lastPoint = parser.startPoint();
        for (int i = 0; i < parser.countVerbs(); ++i) {
            switch (parser.atVerb(i)) {
                case SkPathVerb::kClose:
                case SkPathVerb::kDone:
                    if (parser.startPoint() != lastPoint) {
                        lastPoint = write_line_as_cubic(
                                vertexData + fPathVertexCount, lastPoint, parser.startPoint());
                        break;
                    }  // fallthru
                default:
                    continue;

                case SkPathVerb::kLine:
                    lastPoint = write_line_as_cubic(vertexData + fPathVertexCount, lastPoint,
                                                    parser.atPoint(ptsIdx));
                    ++ptsIdx;
                    break;
                case SkPathVerb::kQuad:
                    lastPoint = write_quadratic_as_cubic(vertexData + fPathVertexCount, lastPoint,
                                                         parser.atPoint(ptsIdx),
                                                         parser.atPoint(ptsIdx + 1));
                    ptsIdx += 2;
                    break;
                case SkPathVerb::kCubic:
                    lastPoint = write_cubic(vertexData + fPathVertexCount, lastPoint,
                                            parser.atPoint(ptsIdx), parser.atPoint(ptsIdx + 1),
                                            parser.atPoint(ptsIdx + 2));
                    ptsIdx += 3;
                    break;
                case SkPathVerb::kConic:
                    SkUNREACHABLE;
            }
            vertexData[fPathVertexCount + 4] = parser.midpoint();
            fPathVertexCount += 5;
        }
    }

    vertexAlloc.unlock(fPathVertexCount);
    return SkToBool(fPathVertexCount);
}

void GrTessellatePathOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    this->drawStencilPass(state);
    if (!(Flags::kStencilOnly & fFlags)) {
        this->drawCoverPass(state);
    }
}

void GrTessellatePathOp::drawStencilPass(GrOpFlushState* state) {
    // Increments clockwise triangles and decrements counterclockwise. Used for "winding" fill.
    constexpr static GrUserStencilSettings kIncrDecrStencil(
        GrUserStencilSettings::StaticInitSeparate<
            0x0000,                                0x0000,
            GrUserStencilTest::kAlwaysIfInClip,    GrUserStencilTest::kAlwaysIfInClip,
            0xffff,                                0xffff,
            GrUserStencilOp::kIncWrap,             GrUserStencilOp::kDecWrap,
            GrUserStencilOp::kKeep,                GrUserStencilOp::kKeep,
            0xffff,                                0xffff>());

    // Inverts the bottom stencil bit. Used for "even/odd" fill.
    constexpr static GrUserStencilSettings kInvertStencil(
        GrUserStencilSettings::StaticInit<
            0x0000,
            GrUserStencilTest::kAlwaysIfInClip,
            0xffff,
            GrUserStencilOp::kInvert,
            GrUserStencilOp::kKeep,
            0x0001>());

    GrPipeline::InitArgs initArgs;
    if (GrAAType::kNone != fAAType) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
    if (state->caps().wireframeSupport() && (Flags::kWireframe & fFlags)) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kWireframe;
    }
    SkASSERT(SkPathFillType::kWinding == fPath.getFillType() ||
             SkPathFillType::kEvenOdd == fPath.getFillType());
    initArgs.fUserStencil = (SkPathFillType::kWinding == fPath.getFillType()) ?
            &kIncrDecrStencil : &kInvertStencil;
    initArgs.fCaps = &state->caps();
    GrPipeline pipeline(initArgs, GrDisableColorXPFactory::MakeXferProcessor(),
                        state->appliedHardClip());

    if (fStencilPathShader) {
        SkASSERT(fPathVertexBuffer);
        GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline, fStencilPathShader);
        state->bindPipelineAndScissorClip(programInfo, this->bounds());
        state->bindBuffers(nullptr, nullptr, fPathVertexBuffer.get());
        state->draw(fPathVertexCount, fBasePathVertex);
    }

    if (fCubicInstanceBuffer) {
        // Here we treat the cubic instance buffer as tessellation patches to stencil the curves.
        GrStencilCubicShader shader(fViewMatrix);
        GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline, &shader);
        state->bindPipelineAndScissorClip(programInfo, this->bounds());
        // Bind instancedBuff as vertex.
        state->bindBuffers(nullptr, nullptr, fCubicInstanceBuffer.get());
        state->draw(fCubicInstanceCount * 4, fBaseCubicInstance * 4);
    }

    // http://skbug.com/9739
    if (state->caps().requiresManualFBBarrierAfterTessellatedStencilDraw()) {
        state->gpu()->insertManualFramebufferBarrier();
    }
}

void GrTessellatePathOp::drawCoverPass(GrOpFlushState* state) {
    // Allows non-zero stencil values to pass and write a color, and resets the stencil value back
    // to zero; discards immediately on stencil values of zero.
    // NOTE: It's ok to not check the clip here because the previous stencil pass only wrote to
    // samples already inside the clip.
    constexpr static GrUserStencilSettings kTestAndResetStencil(
        GrUserStencilSettings::StaticInit<
            0x0000,
            GrUserStencilTest::kNotEqual,
            0xffff,
            GrUserStencilOp::kZero,
            GrUserStencilOp::kKeep,
            0xffff>());

    GrPipeline::InitArgs initArgs;
    if (GrAAType::kNone != fAAType) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
        if (1 == state->proxy()->numSamples()) {
            SkASSERT(GrAAType::kCoverage == fAAType);
            // We are mixed sampled. Use conservative raster to make the sample coverage mask 100%
            // at every fragment. This way we will still get a double hit on shared edges, but
            // whichever side comes first will cover every sample and will clear the stencil. The
            // other side will then be discarded and not cause a double blend.
            initArgs.fInputFlags |= GrPipeline::InputFlags::kConservativeRaster;
        }
    }
    initArgs.fCaps = &state->caps();
    initArgs.fDstProxyView = state->drawOpArgs().dstProxyView();
    initArgs.fWriteSwizzle = state->drawOpArgs().writeSwizzle();
    GrPipeline pipeline(initArgs, std::move(fProcessors), state->detachAppliedClip());

    if (fFillPathShader) {
        SkASSERT(fPathVertexBuffer);

        // These are a twist on the standard red book stencil settings that allow us to draw the
        // inner polygon directly to the final render target. At this point, the curves are already
        // stencilled in. So if the stencil value is zero, then it means the path at our sample is
        // not affected by any curves and we fill the path in directly. If the stencil value is
        // nonzero, then we don't fill and instead continue the standard red book stencil process.
        //
        // NOTE: These settings are currently incompatible with a stencil clip.
        constexpr static GrUserStencilSettings kFillOrIncrDecrStencil(
            GrUserStencilSettings::StaticInitSeparate<
                0x0000,                        0x0000,
                GrUserStencilTest::kEqual,     GrUserStencilTest::kEqual,
                0xffff,                        0xffff,
                GrUserStencilOp::kKeep,        GrUserStencilOp::kKeep,
                GrUserStencilOp::kIncWrap,     GrUserStencilOp::kDecWrap,
                0xffff,                        0xffff>());

        constexpr static GrUserStencilSettings kFillOrInvertStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kEqual,
                0xffff,
                GrUserStencilOp::kKeep,
                GrUserStencilOp::kZero,
                0xffff>());

        if (fStencilPathShader) {
            // The path was already stencilled. Here we just need to do a cover pass.
            pipeline.setUserStencil(&kTestAndResetStencil);
        } else if (!fCubicInstanceBuffer) {
            // There are no curves, so we can just ignore stencil and fill the path directly.
            pipeline.setUserStencil(&GrUserStencilSettings::kUnused);
        } else if (SkPathFillType::kWinding == fPath.getFillType()) {
            // Fill in the path pixels not touched by curves, incr/decr stencil otherwise.
            SkASSERT(!pipeline.hasStencilClip());
            pipeline.setUserStencil(&kFillOrIncrDecrStencil);
        } else {
            // Fill in the path pixels not touched by curves, invert stencil otherwise.
            SkASSERT(!pipeline.hasStencilClip());
            pipeline.setUserStencil(&kFillOrInvertStencil);
        }
        GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline, fFillPathShader);
        state->bindPipelineAndScissorClip(programInfo, this->bounds());
        state->bindTextures(*fFillPathShader, nullptr, pipeline);
        state->bindBuffers(nullptr, nullptr, fPathVertexBuffer.get());
        state->draw(fPathVertexCount, fBasePathVertex);

        if (fCubicInstanceBuffer) {
            // At this point, every pixel is filled in except the ones touched by curves. Issue a
            // final cover pass over the curves by drawing their convex hulls. This will fill in any
            // remaining samples and reset the stencil buffer.
            pipeline.setUserStencil(&kTestAndResetStencil);
            GrFillCubicHullShader shader(fViewMatrix, fColor);
            GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline, &shader);
            state->bindPipelineAndScissorClip(programInfo, this->bounds());
            state->bindTextures(shader, nullptr, pipeline);
            state->bindBuffers(nullptr, fCubicInstanceBuffer.get(), nullptr);
            state->drawInstanced(fCubicInstanceCount, fBaseCubicInstance, 4, 0);
        }
        return;
    }

    // There is not a fill shader for the path. Just draw a bounding box.
    pipeline.setUserStencil(&kTestAndResetStencil);
    GrFillBoundingBoxShader shader(fViewMatrix, fColor, fPath.getBounds());
    GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline, &shader);
    state->bindPipelineAndScissorClip(programInfo, this->bounds());
    state->bindTextures(shader, nullptr, pipeline);
    state->bindBuffers(nullptr, nullptr, nullptr);
    state->draw(4, 0);
}
