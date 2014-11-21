/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Base_DEFINED
#define SkImage_Base_DEFINED

#include "SkImage.h"
#include "SkSurface.h"

static SkSurfaceProps copy_or_safe_defaults(const SkSurfaceProps* props) {
    return props ? *props : SkSurfaceProps(0, kUnknown_SkPixelGeometry);
}

class SkImage_Base : public SkImage {
public:
    SkImage_Base(int width, int height, const SkSurfaceProps* props)
        : INHERITED(width, height)
        , fProps(copy_or_safe_defaults(props))
    {}

    /**
     *  If the props weren't know at constructor time, call this but only before the image is
     *  ever released into the wild (since the props field must appear to be immutable).
     */
    void initWithProps(const SkSurfaceProps& props) {
        SkASSERT(this->unique());   // only viewed by one thread
        SkSurfaceProps* mutableProps = const_cast<SkSurfaceProps*>(&fProps);
        SkASSERT(mutableProps != &props);   // check for self-assignment
        mutableProps->~SkSurfaceProps();
        new (mutableProps) SkSurfaceProps(props);
    }

    const SkSurfaceProps& props() const { return fProps; }

    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) const = 0;
    virtual void onDrawRect(SkCanvas*, const SkRect* src,
                                  const SkRect& dst, const SkPaint*) const = 0;
    virtual SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) const = 0;

    // Default impl calls onDraw
    virtual bool onReadPixels(SkBitmap*, const SkIRect& subset) const;

    virtual const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const {
        return NULL;
    }

    virtual GrTexture* onGetTexture() const { return NULL; }

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(SkBitmap*) const { return false; }

    virtual SkShader* onNewShader(SkShader::TileMode,
                                  SkShader::TileMode,
                                  const SkMatrix* localMatrix) const { return NULL; };

private:
    const SkSurfaceProps fProps;

    typedef SkImage INHERITED;
};

static inline SkImage_Base* as_IB(SkImage* image) {
    return static_cast<SkImage_Base*>(image);
}

static inline const SkImage_Base* as_IB(const SkImage* image) {
    return static_cast<const SkImage_Base*>(image);
}

#endif
