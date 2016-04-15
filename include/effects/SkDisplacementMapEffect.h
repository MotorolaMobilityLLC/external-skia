/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDisplacementMapEffect_DEFINED
#define SkDisplacementMapEffect_DEFINED

#include "SkImageFilter.h"

class SK_API SkDisplacementMapEffect : public SkImageFilter {
public:
    enum ChannelSelectorType {
        kUnknown_ChannelSelectorType,
        kR_ChannelSelectorType,
        kG_ChannelSelectorType,
        kB_ChannelSelectorType,
        kA_ChannelSelectorType
    };

    ~SkDisplacementMapEffect();

    static SkImageFilter* Create(ChannelSelectorType xChannelSelector,
                                 ChannelSelectorType yChannelSelector,
                                 SkScalar scale, SkImageFilter* displacement,
                                 SkImageFilter* color = NULL,
                                 const CropRect* cropRect = NULL);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDisplacementMapEffect)

    SkRect computeFastBounds(const SkRect& src) const override;

    virtual SkIRect onFilterBounds(const SkIRect& src, const SkMatrix&,
                                   MapDirection) const override;
    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix&, MapDirection) const override;

    SK_TO_STRING_OVERRIDE()

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

    SkDisplacementMapEffect(ChannelSelectorType xChannelSelector,
                            ChannelSelectorType yChannelSelector,
                            SkScalar scale, SkImageFilter* inputs[2],
                            const CropRect* cropRect);
    void flatten(SkWriteBuffer&) const override;

private:
    ChannelSelectorType fXChannelSelector;
    ChannelSelectorType fYChannelSelector;
    SkScalar fScale;
    typedef SkImageFilter INHERITED;
    const SkImageFilter* getDisplacementInput() const { return getInput(0); }
    const SkImageFilter* getColorInput() const { return getInput(1); }
};

#endif
