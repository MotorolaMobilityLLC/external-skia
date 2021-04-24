/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellateShader_DEFINED
#define GrStrokeTessellateShader_DEFINED

#include "src/gpu/tessellate/GrPathShader.h"

#include "include/core/SkStrokeRec.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include <array>

class GrGLSLUniformHandler;

// Tessellates a batch of stroke patches directly to the canvas. Tessellated stroking works by
// creating stroke-width, orthogonal edges at set locations along the curve and then connecting them
// with a quad strip. These orthogonal edges come from two different sets: "parametric edges" and
// "radial edges". Parametric edges are spaced evenly in the parametric sense, and radial edges
// divide the curve's _rotation_ into even steps. The tessellation shader evaluates both sets of
// edges and sorts them into a single quad strip. With this combined set of edges we can stroke any
// curve, regardless of curvature.
class GrStrokeTessellateShader : public GrPathShader {
public:
    // Are we using hardware tessellation or indirect draws?
    enum class Mode {
        kHardwareTessellation,
        kLog2Indirect,
        kFixedCount
    };

    enum class ShaderFlags {
        kNone          = 0,
        kHasConics     = 1 << 0,
        kWideColor     = 1 << 1,
        kDynamicStroke = 1 << 2,  // Each patch or instance has its own stroke width and join type.
        kDynamicColor  = 1 << 3,  // Each patch or instance has its own color.
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(ShaderFlags);

    // Returns the fixed number of edges that are always emitted with the given join type. If the
    // join is round, the caller needs to account for the additional radial edges on their own.
    // Specifically, each join always emits:
    //
    //   * Two colocated edges at the beginning (a full-width edge to seam with the preceding stroke
    //     and a half-width edge to begin the join).
    //
    //   * An extra edge in the middle for miter joins, or else a variable number of radial edges
    //     for round joins (the caller is responsible for counting radial edges from round joins).
    //
    //   * A half-width edge at the end of the join that will be colocated with the first
    //     (full-width) edge of the stroke.
    //
    constexpr static int NumFixedEdgesInJoin(SkPaint::Join joinType) {
        switch (joinType) {
            case SkPaint::kMiter_Join:
                return 4;
            case SkPaint::kRound_Join:
                // The caller is responsible for counting the variable number of middle, radial
                // segments on round joins.
                [[fallthrough]];
            case SkPaint::kBevel_Join:
                return 3;
        }
        SkUNREACHABLE;
    }

    // We encode all of a join's information in a single float value:
    //
    //     Negative => Round Join
    //     Zero     => Bevel Join
    //     Positive => Miter join, and the value is also the miter limit
    //
    static float GetJoinType(const SkStrokeRec& stroke) {
        switch (stroke.getJoin()) {
            case SkPaint::kRound_Join: return -1;
            case SkPaint::kBevel_Join: return 0;
            case SkPaint::kMiter_Join: SkASSERT(stroke.getMiter() >= 0); return stroke.getMiter();
        }
        SkUNREACHABLE;
    }

    // This struct gets written out to each patch or instance if kDynamicStroke is enabled.
    struct DynamicStroke {
        static bool StrokesHaveEqualDynamicState(const SkStrokeRec& a, const SkStrokeRec& b) {
            return a.getWidth() == b.getWidth() && a.getJoin() == b.getJoin() &&
                   (a.getJoin() != SkPaint::kMiter_Join || a.getMiter() == b.getMiter());
        }
        void set(const SkStrokeRec& stroke) {
            fRadius = stroke.getWidth() * .5f;
            fJoinType = GetJoinType(stroke);
        }
        float fRadius;
        float fJoinType;  // See GetJoinType().
    };

    // 'viewMatrix' is applied to the geometry post tessellation. It cannot have perspective.
    GrStrokeTessellateShader(Mode mode, ShaderFlags shaderFlags, const SkMatrix& viewMatrix,
                             const SkStrokeRec& stroke, SkPMColor4f color)
            : GrPathShader(kTessellate_GrStrokeTessellateShader_ClassID, viewMatrix,
                           (mode == Mode::kHardwareTessellation) ?
                                   GrPrimitiveType::kPatches : GrPrimitiveType::kTriangleStrip,
                           (mode == Mode::kHardwareTessellation) ? 1 : 0)
            , fMode(mode)
            , fShaderFlags(shaderFlags)
            , fStroke(stroke)
            , fColor(color) {
        if (fMode == Mode::kHardwareTessellation) {
            // A join calculates its starting angle using prevCtrlPtAttr.
            fAttribs.emplace_back("prevCtrlPtAttr", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
            // pts 0..3 define the stroke as a cubic bezier. If p3.y is infinity, then it's a conic
            // with w=p3.x.
            //
            // If p0 == prevCtrlPtAttr, then no join is emitted.
            //
            // pts=[p0, p3, p3, p3] is a reserved pattern that means this patch is a join only,
            // whose start and end tangents are (p0 - inputPrevCtrlPt) and (p3 - p0).
            //
            // pts=[p0, p0, p0, p3] is a reserved pattern that means this patch is a "bowtie", or
            // double-sided round join, anchored on p0 and rotating from (p0 - prevCtrlPtAttr) to
            // (p3 - p0).
            fAttribs.emplace_back("pts01Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
            fAttribs.emplace_back("pts23Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        } else {
            // pts 0..3 define the stroke as a cubic bezier. If p3.y is infinity, then it's a conic
            // with w=p3.x.
            //
            // An empty stroke (p0==p1==p2==p3) is a special case that denotes a circle, or
            // 180-degree point stroke.
            fAttribs.emplace_back("pts01Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
            fAttribs.emplace_back("pts23Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
            if (fMode == Mode::kLog2Indirect) {
                // argsAttr.xy contains the lastControlPoint for setting up the join.
                //
                // "argsAttr.z=numTotalEdges" tells the shader the literal number of edges in the
                // triangle strip being rendered (i.e., it should be vertexCount/2). If
                // numTotalEdges is negative and the join type is "kRound", it also instructs the
                // shader to only allocate one segment the preceding round join.
                fAttribs.emplace_back("argsAttr", kFloat3_GrVertexAttribType, kFloat3_GrSLType);
            } else {
                SkASSERT(fMode == Mode::kFixedCount);
                // argsAttr contains the lastControlPoint for setting up the join.
                fAttribs.emplace_back("argsAttr", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
            }
        }
        if (fShaderFlags & ShaderFlags::kDynamicStroke) {
            fAttribs.emplace_back("dynamicStrokeAttr", kFloat2_GrVertexAttribType,
                                  kFloat2_GrSLType);
        }
        if (fShaderFlags & ShaderFlags::kDynamicColor) {
            fAttribs.emplace_back("dynamicColorAttr",
                                  (fShaderFlags & ShaderFlags::kWideColor)
                                          ? kFloat4_GrVertexAttribType
                                          : kUByte4_norm_GrVertexAttribType,
                                  kHalf4_GrSLType);
        }
        if (fMode == Mode::kHardwareTessellation) {
            this->setVertexAttributes(fAttribs.data(), fAttribs.count());
        } else {
            this->setInstanceAttributes(fAttribs.data(), fAttribs.count());
        }
        SkASSERT(fAttribs.count() <= kMaxAttribCount);
    }

    bool hasConics() const { return fShaderFlags & ShaderFlags::kHasConics; }
    bool hasDynamicStroke() const { return fShaderFlags & ShaderFlags::kDynamicStroke; }
    bool hasDynamicColor() const { return fShaderFlags & ShaderFlags::kDynamicColor; }

    // Used by GrFixedCountTessellator to configure the uniform value that tells the shader how many
    // total edges are in the triangle strip.
    void setFixedCountNumTotalEdges(int value) {
        SkASSERT(fMode == Mode::kFixedCount);
        fFixedCountNumTotalEdges = value;
    }

private:
    const char* name() const override { return "GrStrokeTessellateShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override;
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;

    SkString getTessControlShaderGLSL(const GrGLSLGeometryProcessor*,
                                      const char* versionAndExtensionDecls,
                                      const GrGLSLUniformHandler&,
                                      const GrShaderCaps&) const override;
    SkString getTessEvaluationShaderGLSL(const GrGLSLGeometryProcessor*,
                                         const char* versionAndExtensionDecls,
                                         const GrGLSLUniformHandler&,
                                         const GrShaderCaps&) const override;

    const Mode fMode;
    const ShaderFlags fShaderFlags;
    const SkStrokeRec fStroke;
    const SkPMColor4f fColor;

    constexpr static int kMaxAttribCount = 5;
    SkSTArray<kMaxAttribCount, Attribute> fAttribs;

    // This is a uniform value used when fMode is kFixedCount that tells the shader how many total
    // edges are in the triangle strip.
    float fFixedCountNumTotalEdges = 0;

    class TessellationImpl;
    class InstancedImpl;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrStrokeTessellateShader::ShaderFlags);

#endif
