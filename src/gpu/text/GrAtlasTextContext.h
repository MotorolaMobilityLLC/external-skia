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
    static GrAtlasTextContext* Create(GrContext*, const SkSurfaceProps&);

private:
    GrAtlasTextContext(GrContext*, const SkSurfaceProps&);
    ~GrAtlasTextContext() override {}

    bool canDraw(const SkPaint&, const SkMatrix& viewMatrix) override;

    void onDrawText(GrDrawContext*, const GrClip&, const GrPaint&, const SkPaint&,
                    const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                    SkScalar x, SkScalar y, const SkIRect& regionClipBounds) override;
    void onDrawPosText(GrDrawContext*, const GrClip&, const GrPaint&,
                       const SkPaint&, const SkMatrix& viewMatrix,
                       const char text[], size_t byteLength,
                       const SkScalar pos[], int scalarsPerPosition,
                       const SkPoint& offset, const SkIRect& regionClipBounds) override;
    void drawTextBlob(GrDrawContext*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkTextBlob*, SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds) override;

    // sets up the descriptor on the blob and returns a detached cache.  Client must attach
    inline static GrColor ComputeCanonicalColor(const SkPaint&, bool lcd);
    void regenerateTextBlob(GrAtlasTextBlob* bmp, const SkPaint& skPaint, GrColor,
                            const SkMatrix& viewMatrix,
                            const SkTextBlob* blob, SkScalar x, SkScalar y,
                            SkDrawFilter* drawFilter);
    inline static bool HasLCD(const SkTextBlob*);

    // Test methods
    // TODO this is really ugly.  It'd be much nicer if positioning could be moved to batch
    inline GrAtlasTextBlob* createDrawTextBlob(const GrPaint&,
                                               const SkPaint&, const SkMatrix& viewMatrix,
                                               const char text[], size_t byteLength,
                                               SkScalar x, SkScalar y);
    inline GrAtlasTextBlob* createDrawPosTextBlob(const GrPaint&,
                                                  const SkPaint&, const SkMatrix& viewMatrix,
                                                  const char text[], size_t byteLength,
                                                  const SkScalar pos[], int scalarsPerPosition,
                                                  const SkPoint& offset);
    const GrDistanceFieldAdjustTable* dfAdjustTable() const { return fDistanceAdjustTable; }

    GrBatchTextStrike* fCurrStrike;
    GrTextBlobCache* fCache;
    SkAutoTUnref<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;

#ifdef GR_TEST_UTILS
    DRAW_BATCH_TEST_FRIEND(TextBlobBatch);
#endif

    typedef GrTextContext INHERITED;
};

#endif
