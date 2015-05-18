/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOvalRenderer_DEFINED
#define GrOvalRenderer_DEFINED

#include "GrContext.h"
#include "GrPaint.h"

class GrContext;
class GrDrawTarget;
class GrPaint;
struct SkRect;
class SkStrokeRec;

/*
 * This class wraps helper functions that draw ovals and roundrects (filled & stroked)
 */
class GrOvalRenderer : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrOvalRenderer)

    bool drawOval(GrDrawTarget*,
                  GrPipelineBuilder*,
                  GrColor,
                  const SkMatrix& viewMatrix,
                  bool useAA,
                  const SkRect& oval,
                  const SkStrokeRec& stroke);
    bool drawRRect(GrDrawTarget*,
                   GrPipelineBuilder*,
                   GrColor,
                   const SkMatrix& viewMatrix,
                   bool useAA,
                   const SkRRect& rrect,
                   const SkStrokeRec& stroke);
    bool drawDRRect(GrDrawTarget* target,
                    GrPipelineBuilder*,
                    GrColor,
                    const SkMatrix& viewMatrix,
                    bool useAA,
                    const SkRRect& outer,
                    const SkRRect& inner);

private:
    bool drawEllipse(GrDrawTarget* target,
                     GrPipelineBuilder*,
                     GrColor,
                     const SkMatrix& viewMatrix,
                     bool useCoverageAA,
                     const SkRect& ellipse,
                     const SkStrokeRec& stroke);
    bool drawDIEllipse(GrDrawTarget* target,
                       GrPipelineBuilder*,
                       GrColor,
                       const SkMatrix& viewMatrix,
                       bool useCoverageAA,
                       const SkRect& ellipse,
                       const SkStrokeRec& stroke);
    void drawCircle(GrDrawTarget* target,
                    GrPipelineBuilder*,
                    GrColor,
                    const SkMatrix& viewMatrix,
                    bool useCoverageAA,
                    const SkRect& circle,
                    const SkStrokeRec& stroke);

    typedef SkRefCnt INHERITED;
};

#endif // GrOvalRenderer_DEFINED
