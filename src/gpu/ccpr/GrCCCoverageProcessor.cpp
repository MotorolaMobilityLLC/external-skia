/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCCoverageProcessor.h"

#include "GrGpuCommandBuffer.h"
#include "GrOpFlushState.h"
#include "SkMakeUnique.h"
#include "ccpr/GrCCCubicShader.h"
#include "ccpr/GrCCQuadraticShader.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

void GrCCCoverageProcessor::getGLSLProcessorKey(const GrShaderCaps&,
                                                GrProcessorKeyBuilder* b) const {
    int key = ((int)fRenderPass << 3);
    if (GSTriangleSubpass::kCorners == fGSTriangleSubpass) {
        key |= 4;
    }
    if (WindMethod::kInstanceData == fWindMethod) {
        key |= 2;
    }
    if (Impl::kVertexShader == fImpl) {
        key |= 1;
    }
#ifdef SK_DEBUG
    uint32_t bloatBits;
    memcpy(&bloatBits, &fDebugBloat, 4);
    b->add32(bloatBits);
#endif
    b->add32(key);
}

GrGLSLPrimitiveProcessor* GrCCCoverageProcessor::createGLSLInstance(const GrShaderCaps&) const {
    std::unique_ptr<Shader> shader;
    switch (fRenderPass) {
        case RenderPass::kTriangles:
            shader = skstd::make_unique<Shader>();
            break;
        case RenderPass::kQuadratics:
            shader = skstd::make_unique<GrCCQuadraticShader>();
            break;
        case RenderPass::kCubics:
            shader = skstd::make_unique<GrCCCubicShader>();
            break;
    }
    return Impl::kGeometryShader == fImpl ? this->createGSImpl(std::move(shader))
                                          : this->createVSImpl(std::move(shader));
}

void GrCCCoverageProcessor::draw(GrOpFlushState* flushState, const GrPipeline& pipeline,
                                 const GrMesh meshes[],
                                 const GrPipeline::DynamicState dynamicStates[], int meshCount,
                                 const SkRect& drawBounds) const {
    GrGpuRTCommandBuffer* cmdBuff = flushState->rtCommandBuffer();
    cmdBuff->draw(pipeline, *this, meshes, dynamicStates, meshCount, drawBounds);

    // Geometry shader backend draws triangles in two subpasses.
    if (RenderPass::kTriangles == fRenderPass && Impl::kGeometryShader == fImpl) {
        SkASSERT(GSTriangleSubpass::kHullsAndEdges == fGSTriangleSubpass);
        GrCCCoverageProcessor cornerProc(*this, GSTriangleSubpass::kCorners);
        cmdBuff->draw(pipeline, cornerProc, meshes, dynamicStates, meshCount, drawBounds);
    }
}

void GrCCCoverageProcessor::Shader::emitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                                 GrGLSLVarying::Scope scope, SkString* code,
                                                 const char* position, const char* coverage,
                                                 const char* wind) {
    SkASSERT(GrGLSLVarying::Scope::kVertToGeo != scope);
    code->appendf("half coverageTimesWind = %s * %s;", coverage, wind);
    CoverageHandling coverageHandling = this->onEmitVaryings(varyingHandler, scope, code, position,
                                                             "coverageTimesWind");
    if (CoverageHandling::kNotHandled == coverageHandling) {
        fCoverageTimesWind.reset(kHalf_GrSLType, scope);
        varyingHandler->addVarying("coverage_times_wind", &fCoverageTimesWind);
        code->appendf("%s = coverageTimesWind;", OutName(fCoverageTimesWind));
    }
}

void GrCCCoverageProcessor::Shader::emitFragmentCode(const GrCCCoverageProcessor& proc,
                                                     GrGLSLFPFragmentBuilder* f,
                                                     const char* skOutputColor,
                                                     const char* skOutputCoverage) const {
    f->codeAppendf("half coverage = +1;");
    this->onEmitFragmentCode(proc, f, "coverage");
    if (fCoverageTimesWind.fsIn()) {
        f->codeAppendf("coverage *= %s;", fCoverageTimesWind.fsIn());
    }
    f->codeAppendf("%s.a = coverage;", skOutputColor);
    f->codeAppendf("%s = half4(1);", skOutputCoverage);
#ifdef SK_DEBUG
    if (proc.debugVisualizationsEnabled()) {
        f->codeAppendf("%s = half4(-%s.a, %s.a, 0, abs(%s.a));",
                       skOutputColor, skOutputColor, skOutputColor, skOutputColor);
    }
#endif
}

void GrCCCoverageProcessor::Shader::CalcEdgeCoverageAtBloatVertex(GrGLSLVertexGeoBuilder* s,
                                                                  const char* leftPt,
                                                                  const char* rightPt,
                                                                  const char* rasterVertexDir,
                                                                  const char* outputCoverage) {
    // Here we find an edge's coverage at one corner of a conservative raster bloat box whose center
    // falls on the edge in question. (A bloat box is axis-aligned and the size of one pixel.) We
    // always set up coverage so it is -1 at the outermost corner, 0 at the innermost, and -.5 at
    // the center. Interpolated, these coverage values convert jagged conservative raster edges into
    // smooth antialiased edges.
    //
    // d1 == (P + sign(n) * bloat) dot n                   (Distance at the bloat box vertex whose
    //    == P dot n + (abs(n.x) + abs(n.y)) * bloatSize    coverage=-1, where the bloat box is
    //                                                      centered on P.)
    //
    // d0 == (P - sign(n) * bloat) dot n                   (Distance at the bloat box vertex whose
    //    == P dot n - (abs(n.x) + abs(n.y)) * bloatSize    coverage=0, where the bloat box is
    //                                                      centered on P.)
    //
    // d == (P + rasterVertexDir * bloatSize) dot n        (Distance at the bloat box vertex whose
    //   == P dot n + (rasterVertexDir dot n) * bloatSize   coverage we wish to calculate.)
    //
    // coverage == -(d - d0) / (d1 - d0)                   (coverage=-1 at d=d1; coverage=0 at d=d0)
    //
    //          == (rasterVertexDir dot n) / (abs(n.x) + abs(n.y)) * -.5 - .5
    //
    s->codeAppendf("float2 n = float2(%s.y - %s.y, %s.x - %s.x);",
                   rightPt, leftPt, leftPt, rightPt);
    s->codeAppend ("float nwidth = abs(n.x) + abs(n.y);");
    s->codeAppendf("float t = dot(%s, n);", rasterVertexDir);
    // The below conditional guarantees we get exactly 1 on the divide when nwidth=t (in case the
    // GPU divides by multiplying by the reciprocal?) It also guards against NaN when nwidth=0.
    s->codeAppendf("%s = (abs(t) != nwidth ? t / nwidth : sign(t)) * -.5 - .5;", outputCoverage);
}

void GrCCCoverageProcessor::Shader::CalcEdgeCoveragesAtBloatVertices(GrGLSLVertexGeoBuilder* s,
                                                                     const char* leftPt,
                                                                     const char* rightPt,
                                                                     const char* bloatDir1,
                                                                     const char* bloatDir2,
                                                                     const char* outputCoverages) {
    // See comments in CalcEdgeCoverageAtBloatVertex.
    s->codeAppendf("float2 n = float2(%s.y - %s.y, %s.x - %s.x);",
                   rightPt, leftPt, leftPt, rightPt);
    s->codeAppend ("float nwidth = abs(n.x) + abs(n.y);");
    s->codeAppendf("float2 t = n * float2x2(%s, %s);", bloatDir1, bloatDir2);
    s->codeAppendf("for (int i = 0; i < 2; ++i) {");
    s->codeAppendf(    "%s[i] = (abs(t[i]) != nwidth ? t[i] / nwidth : sign(t[i])) * -.5 - .5;",
                       outputCoverages);
    s->codeAppendf("}");
}
