/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMorphologyImageFilter_DEFINED
#define SkMorphologyImageFilter_DEFINED

#include "SkColor.h"
#include "SkImageFilter.h"
#include "SkSize.h"

class SK_API SkMorphologyImageFilter : public SkImageFilter {
public:
    SkMorphologyImageFilter(int radiusX, int radiusY, SkImageFilter* input, const CropRect* cropRect);
    virtual void computeFastBounds(const SkRect& src, SkRect* dst) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix& ctm, SkIRect* dst) const SK_OVERRIDE;

    /**
     * All morphology procs have the same signature: src is the source buffer, dst the
     * destination buffer, radius is the morphology radius, width and height are the bounds
     * of the destination buffer (in pixels), and srcStride and dstStride are the
     * number of pixels per row in each buffer. All buffers are 8888.
     */

    typedef void (*Proc)(const SkPMColor* src, SkPMColor* dst, int radius,
                         int width, int height, int srcStride, int dstStride);

protected:
    bool filterImageGeneric(Proc procX, Proc procY,
                            Proxy*, const SkBitmap& src, const SkMatrix&,
                            SkBitmap* result, SkIPoint* offset);
    SkMorphologyImageFilter(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool canFilterImageGPU() const SK_OVERRIDE { return true; }
    bool filterImageGPUGeneric(bool dilate, Proxy* proxy, const SkBitmap& src,
                               const SkMatrix& ctm, SkBitmap* result,
                               SkIPoint* offset);
#endif

    SkISize    radius() const { return fRadius; }

private:
    SkISize    fRadius;
    typedef SkImageFilter INHERITED;
};

class SK_API SkDilateImageFilter : public SkMorphologyImageFilter {
public:
    SkDilateImageFilter(int radiusX, int radiusY,
                        SkImageFilter* input = NULL,
                        const CropRect* cropRect = NULL)
    : INHERITED(radiusX, radiusY, input, cropRect) {}

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDilateImageFilter)

protected:
    SkDilateImageFilter(SkReadBuffer& buffer) : INHERITED(buffer) {}

private:
    typedef SkMorphologyImageFilter INHERITED;
};

class SK_API SkErodeImageFilter : public SkMorphologyImageFilter {
public:
    SkErodeImageFilter(int radiusX, int radiusY,
                       SkImageFilter* input = NULL,
                       const CropRect* cropRect = NULL)
    : INHERITED(radiusX, radiusY, input, cropRect) {}

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkErodeImageFilter)

protected:
    SkErodeImageFilter(SkReadBuffer& buffer) : INHERITED(buffer) {}

private:
    typedef SkMorphologyImageFilter INHERITED;
};

#endif
