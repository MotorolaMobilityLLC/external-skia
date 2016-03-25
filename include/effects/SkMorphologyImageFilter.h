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

///////////////////////////////////////////////////////////////////////////////
class SK_API SkMorphologyImageFilter : public SkImageFilter {
public:
    SkRect computeFastBounds(const SkRect& src) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix&, MapDirection) const override;

    /**
     * All morphology procs have the same signature: src is the source buffer, dst the
     * destination buffer, radius is the morphology radius, width and height are the bounds
     * of the destination buffer (in pixels), and srcStride and dstStride are the
     * number of pixels per row in each buffer. All buffers are 8888.
     */

    typedef void (*Proc)(const SkPMColor* src, SkPMColor* dst, int radius,
                         int width, int height, int srcStride, int dstStride);

protected:
    SkMorphologyImageFilter(int radiusX, int radiusY, SkImageFilter* input,
                            const CropRect* cropRect);
    sk_sp<SkSpecialImage> filterImageGeneric(bool dilate, 
                                             SkSpecialImage* source,
                                             const Context&,
                                             SkIPoint* offset) const;
    void flatten(SkWriteBuffer&) const override;

    SkISize radius() const { return fRadius; }

private:
    SkISize fRadius;
    typedef SkImageFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
class SK_API SkDilateImageFilter : public SkMorphologyImageFilter {
public:
    static SkImageFilter* Create(int radiusX, int radiusY,
                                 SkImageFilter* input = nullptr,
                                 const CropRect* cropRect = nullptr) {
        if (radiusX < 0 || radiusY < 0) {
            return nullptr;
        }
        return new SkDilateImageFilter(radiusX, radiusY, input, cropRect);
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDilateImageFilter)

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

private:
    SkDilateImageFilter(int radiusX, int radiusY, SkImageFilter* input, const CropRect* cropRect)
        : INHERITED(radiusX, radiusY, input, cropRect) {}

    typedef SkMorphologyImageFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
class SK_API SkErodeImageFilter : public SkMorphologyImageFilter {
public:
    static SkImageFilter* Create(int radiusX, int radiusY,
                                 SkImageFilter* input = nullptr,
                                 const CropRect* cropRect = nullptr) {
        if (radiusX < 0 || radiusY < 0) {
            return nullptr;
        }
        return new SkErodeImageFilter(radiusX, radiusY, input, cropRect);
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkErodeImageFilter)

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

private:
    SkErodeImageFilter(int radiusX, int radiusY, SkImageFilter* input, const CropRect* cropRect)
        : INHERITED(radiusX, radiusY, input, cropRect) {}

    typedef SkMorphologyImageFilter INHERITED;
};

#endif
