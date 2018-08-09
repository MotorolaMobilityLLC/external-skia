/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextUtils_DEFINED
#define GrTextUtils_DEFINED

#include "GrColor.h"
#include "GrColorSpaceInfo.h"
#include "SkColorFilter.h"
#include "SkPaint.h"
#include "SkScalar.h"
#include "SkTextToPathIter.h"
#include "SkTLazy.h"

class GrTextBlob;
class GrAtlasTextOp;
class GrTextStrike;
class GrClip;
class GrColorSpaceXform;
class GrContext;
class GrGlyphCache;
class GrPaint;
class GrShaderCaps;
class SkColorSpace;
class SkGlyph;
class SkGlyphRunListDrawer;
class SkMatrix;
struct SkIRect;
struct SkPoint;
class SkGlyphCache;
class SkTextBlobRunIterator;
class SkSurfaceProps;

/**
 * A class to house a bunch of common text utilities.  This class should *ONLY* have static
 * functions.  It is not a namespace only because we wish to friend SkPaint
 */
class GrTextUtils {
public:
    class Target {
    public:
        virtual ~Target() = default;

        int width() const { return fWidth; }

        int height() const { return fHeight; }

        const GrColorSpaceInfo& colorSpaceInfo() const { return fColorSpaceInfo; }

        virtual void addDrawOp(const GrClip&, std::unique_ptr<GrAtlasTextOp> op) = 0;

        virtual void drawPath(const GrClip&, const SkPath&, const SkPaint&,
                              const SkMatrix& viewMatrix, const SkMatrix* pathMatrix,
                              const SkIRect& clipBounds) = 0;

        virtual void makeGrPaint(GrMaskFormat, const SkPaint&, const SkMatrix& viewMatrix,
                                 GrPaint*) = 0;

        virtual GrContext* getContext() = 0;

        virtual SkGlyphRunListDrawer* glyphDrawer() = 0;

    protected:
        Target(int width, int height, const GrColorSpaceInfo& colorSpaceInfo)
                : fWidth(width), fHeight(height), fColorSpaceInfo(colorSpaceInfo) {}

    private:
        int fWidth;
        int fHeight;
        const GrColorSpaceInfo& fColorSpaceInfo;
    };

    /**
     *  This is used to wrap a SkPaint and its post-color filter color. It is also used by RunPaint
     *  (below). This keeps a pointer to the SkPaint it is initialized with and expects it to remain
     *  const. It is also used to transform to GrPaint.
     */
    class Paint {
    public:
        explicit Paint(const SkPaint* paint, const GrColorSpaceInfo* dstColorSpaceInfo)
                : fPaint(paint), fDstColorSpaceInfo(dstColorSpaceInfo) {
            this->initFilteredColor();
        }

        // These expose the paint's color run through its color filter (if any). This is only valid
        // when drawing grayscale/lcd glyph masks and not when drawing color glyphs.
        GrColor filteredPremulColor() const { return fFilteredPremulColor; }

        SkColor luminanceColor() const { return fPaint->computeLuminanceColor(); }

        const SkPaint& skPaint() const { return *fPaint; }

        operator const SkPaint&() const { return this->skPaint(); }


        void initFilteredColor();

        Paint() = default;

        const SkPaint* fPaint;
        const GrColorSpaceInfo* fDstColorSpaceInfo;
        // This is the paint's color run through its color filter, if present. This color should
        // be used except when rendering bitmap text, in which case the bitmap must be filtered in
        // the fragment shader.
        GrColor fFilteredPremulColor;
    };
};
#endif
