/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBitmapTextContext_DEFINED
#define GrBitmapTextContext_DEFINED

#include "GrTextContext.h"

class GrGeometryProcessor;
class GrTextStrike;

/*
 * This class implements GrTextContext using standard bitmap fonts
 */
class GrBitmapTextContext : public GrTextContext {
public:
    GrBitmapTextContext(GrContext*, const SkDeviceProperties&);
    virtual ~GrBitmapTextContext();

    virtual bool canDraw(const SkPaint& paint) SK_OVERRIDE;

    virtual void drawText(const GrPaint&, const SkPaint&, const char text[], size_t byteLength,
                          SkScalar x, SkScalar y) SK_OVERRIDE;
    virtual void drawPosText(const GrPaint&, const SkPaint&,
                             const char text[], size_t byteLength,
                             const SkScalar pos[], int scalarsPerPosition,
                             const SkPoint& offset) SK_OVERRIDE;

private:
    enum {
        kMinRequestedGlyphs      = 1,
        kDefaultRequestedGlyphs  = 64,
        kMinRequestedVerts       = kMinRequestedGlyphs * 4,
        kDefaultRequestedVerts   = kDefaultRequestedGlyphs * 4,
    };

    GrTextStrike*                     fStrike;
    void*                             fVertices;
    int                               fCurrVertex;
    int                               fMaxVertices;
    SkRect                            fVertexBounds;
    GrTexture*                        fCurrTexture;
    GrMaskFormat                      fCurrMaskFormat;
    SkAutoTUnref<GrGeometryProcessor> fCachedGeometryProcessor;
    // Used to check whether fCachedEffect is still valid.
    uint32_t                          fEffectTextureUniqueID;

    void init(const GrPaint&, const SkPaint&);
    void appendGlyph(GrGlyph::PackedID, SkFixed left, SkFixed top, GrFontScaler*);
    void flush();                 // automatically called by destructor
    void finish();

};

#endif
