/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextContext_DEFINED
#define GrAtlasTextContext_DEFINED

#include "GrTextContext.h"

#include "GrAtlasTextBlob.h"
#include "GrDistanceFieldAdjustTable.h"
#include "GrGeometryProcessor.h"
#include "SkTextBlobRunIterator.h"

#ifdef GR_TEST_UTILS
#include "GrBatchTest.h"
#endif

class GrDrawBatch;
class GrDrawContext;
class GrDrawTarget;
class GrPipelineBuilder;
class GrTextBlobCache;
class SkGlyph;

/*
 * This class implements GrTextContext using standard bitmap fonts, and can also process textblobs.
 */
class GrAtlasTextContext : public GrTextContext {
public:
    static GrAtlasTextContext* Create();

    bool canDraw(const SkPaint&, const SkMatrix& viewMatrix, const SkSurfaceProps&,
                 const GrShaderCaps&);
    void drawText(GrContext*, GrDrawContext*, const GrClip&, const GrPaint&, const SkPaint&,
                  const SkMatrix& viewMatrix, const SkSurfaceProps&, const char text[],
                  size_t byteLength, SkScalar x, SkScalar y,
                  const SkIRect& regionClipBounds) override;
    void drawPosText(GrContext*, GrDrawContext*, const GrClip&, const GrPaint&,
                     const SkPaint&, const SkMatrix& viewMatrix, const SkSurfaceProps&,
                     const char text[], size_t byteLength,
                     const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkIRect& regionClipBounds) override;
    void drawTextBlob(GrContext*, GrDrawContext*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkSurfaceProps&, const SkTextBlob*,
                      SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds) override;

private:
    GrAtlasTextContext();

    // sets up the descriptor on the blob and returns a detached cache.  Client must attach
    inline static GrColor ComputeCanonicalColor(const SkPaint&, bool lcd);
    static void RegenerateTextBlob(GrAtlasTextBlob* bmp,
                                   GrBatchFontCache*,
                                   const GrShaderCaps&,
                                   const SkPaint& skPaint, GrColor,
                                   const SkMatrix& viewMatrix,
                                   const SkSurfaceProps&,
                                   const SkTextBlob* blob, SkScalar x, SkScalar y,
                                   SkDrawFilter* drawFilter);
    inline static bool HasLCD(const SkTextBlob*);

    static inline GrAtlasTextBlob* CreateDrawTextBlob(GrTextBlobCache*,
                                                      GrBatchFontCache*, const GrShaderCaps&,
                                                      const GrPaint&,
                                                      const SkPaint&, const SkMatrix& viewMatrix,
                                                      const SkSurfaceProps&,
                                                      const char text[], size_t byteLength,
                                                      SkScalar x, SkScalar y);
    static inline GrAtlasTextBlob* CreateDrawPosTextBlob(GrTextBlobCache*, GrBatchFontCache*,
                                                         const GrShaderCaps&,
                                                         const GrPaint&,
                                                         const SkPaint&, const SkMatrix& viewMatrix,
                                                         const SkSurfaceProps&,
                                                         const char text[], size_t byteLength,
                                                         const SkScalar pos[],
                                                         int scalarsPerPosition,
                                                         const SkPoint& offset);
    const GrDistanceFieldAdjustTable* dfAdjustTable() const { return fDistanceAdjustTable; }

    SkAutoTUnref<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;

#ifdef GR_TEST_UTILS
    DRAW_BATCH_TEST_FRIEND(TextBlobBatch);
#endif

    typedef GrTextContext INHERITED;
};

#endif
