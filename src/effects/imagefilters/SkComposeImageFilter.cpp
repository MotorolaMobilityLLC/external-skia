/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkComposeImageFilter.h"

#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkComposeImageFilterImpl final : public SkImageFilter_Base {
public:
    explicit SkComposeImageFilterImpl(sk_sp<SkImageFilter> inputs[2])
            : INHERITED(inputs, 2, nullptr) {
        SkASSERT(inputs[0].get());
        SkASSERT(inputs[1].get());
    }

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;
    bool onCanHandleComplexCTM() const override { return true; }

private:
    friend void SkComposeImageFilter::RegisterFlattenables();
    SK_FLATTENABLE_HOOKS(SkComposeImageFilterImpl)

    typedef SkImageFilter_Base INHERITED;
};

} // end namespace

sk_sp<SkImageFilter> SkComposeImageFilter::Make(sk_sp<SkImageFilter> outer,
                                                sk_sp<SkImageFilter> inner) {
    if (!outer) {
        return inner;
    }
    if (!inner) {
        return outer;
    }
    sk_sp<SkImageFilter> inputs[2] = { std::move(outer), std::move(inner) };
    return sk_sp<SkImageFilter>(new SkComposeImageFilterImpl(inputs));
}

void SkComposeImageFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkComposeImageFilterImpl);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkComposeImageFilter", SkComposeImageFilterImpl::CreateProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkComposeImageFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    return SkComposeImageFilter::Make(common.getInput(0), common.getInput(1));
}

SkRect SkComposeImageFilterImpl::computeFastBounds(const SkRect& src) const {
    const SkImageFilter* outer = this->getInput(0);
    const SkImageFilter* inner = this->getInput(1);

    return outer->computeFastBounds(inner->computeFastBounds(src));
}

sk_sp<SkSpecialImage> SkComposeImageFilterImpl::onFilterImage(const Context& ctx,
                                                              SkIPoint* offset) const {
    // The bounds passed to the inner filter must be filtered by the outer
    // filter, so that the inner filter produces the pixels that the outer
    // filter requires as input. This matters if the outer filter moves pixels.
    SkIRect innerClipBounds;
    innerClipBounds = this->getInput(0)->filterBounds(ctx.clipBounds(), ctx.ctm(),
                                                      kReverse_MapDirection, &ctx.clipBounds());
    Context innerContext = ctx.withNewClipBounds(innerClipBounds);
    SkIPoint innerOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> inner(this->filterInput(1, innerContext, &innerOffset));
    if (!inner) {
        return nullptr;
    }

    // TODO (michaelludwig) - Once all filters are updated to process coordinate spaces more
    // robustly, we can allow source images to have non-(0,0) origins, which will mean that the
    // CTM/clipBounds modifications for the outerContext can go away.
    SkMatrix outerMatrix(ctx.ctm());
    outerMatrix.postTranslate(SkIntToScalar(-innerOffset.x()), SkIntToScalar(-innerOffset.y()));
    SkIRect clipBounds = ctx.clipBounds();
    clipBounds.offset(-innerOffset.x(), -innerOffset.y());
    // NOTE: This is the only spot in image filtering where the source image of the context
    // is not constant for the entire DAG evaluation. Given that the inner and outer DAG branches
    // were already created, there's no alternative way for the leaf nodes of the outer DAG to
    // get the results of the inner DAG. Overriding the source image of the context has the correct
    // effect, but means that the source image is not fixed for the entire filter process.
    Context outerContext(outerMatrix, clipBounds, ctx.cache(), ctx.colorType(), ctx.colorSpace(),
                         inner.get());

    SkIPoint outerOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> outer(this->filterInput(0, outerContext, &outerOffset));
    if (!outer) {
        return nullptr;
    }

    *offset = innerOffset + outerOffset;
    return outer;
}

SkIRect SkComposeImageFilterImpl::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                                 MapDirection dir, const SkIRect* inputRect) const {
    const SkImageFilter* outer = this->getInput(0);
    const SkImageFilter* inner = this->getInput(1);

    const SkIRect innerRect = inner->filterBounds(src, ctm, dir, inputRect);
    return outer->filterBounds(innerRect, ctm, dir,
                               kReverse_MapDirection == dir ? &innerRect : nullptr);
}
