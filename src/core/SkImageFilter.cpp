/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageFilter.h"

#include "SkBitmap.h"
#include "SkFlattenableBuffers.h"
#include "SkRect.h"
#include "stdarg.h"

SK_DEFINE_INST_COUNT(SkImageFilter)

SkImageFilter::SkImageFilter(int inputCount, SkImageFilter** inputs)
  : fInputCount(inputCount), fInputs(new SkImageFilter*[inputCount]) {
    for (int i = 0; i < inputCount; ++i) {
        fInputs[i] = inputs[i];
        SkSafeRef(fInputs[i]);
    }
}

SkImageFilter::SkImageFilter(int inputCount, ...)
  : fInputCount(inputCount), fInputs(new SkImageFilter*[inputCount]) {
    va_list ap;
    va_start(ap, inputCount);
    for (int i = 0; i < inputCount; ++i) {
        fInputs[i] = va_arg(ap, SkImageFilter*);
        SkSafeRef(fInputs[i]);
    }
    va_end(ap);
}

SkImageFilter::~SkImageFilter() {
    for (int i = 0; i < fInputCount; i++) {
        SkSafeUnref(fInputs[i]);
    }
    delete[] fInputs;
}

SkImageFilter::SkImageFilter(SkFlattenableReadBuffer& buffer)
    : fInputCount(buffer.readInt()), fInputs(new SkImageFilter*[fInputCount]) {
    for (int i = 0; i < fInputCount; i++) {
        if (buffer.readBool()) {
            fInputs[i] = static_cast<SkImageFilter*>(buffer.readFlattenable());
        } else {
            fInputs[i] = NULL;
        }
    }
}

void SkImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    buffer.writeInt(fInputCount);
    for (int i = 0; i < fInputCount; i++) {
        SkImageFilter* input = getInput(i);
        buffer.writeBool(input != NULL);
        if (input != NULL) {
            buffer.writeFlattenable(input);
        }
    }
}

SkBitmap SkImageFilter::getInputResult(int index, Proxy* proxy,
                                       const SkBitmap& src, const SkMatrix& ctm,
                                       SkIPoint* loc) {
    SkASSERT(index < fInputCount);
    SkImageFilter* input = getInput(index);
    SkBitmap result;
    if (input && input->filterImage(proxy, src, ctm, &result, loc)) {
        return result;
    } else {
        return src;
    }
}


bool SkImageFilter::filterImage(Proxy* proxy, const SkBitmap& src,
                                const SkMatrix& ctm,
                                SkBitmap* result, SkIPoint* loc) {
    SkASSERT(result);
    SkASSERT(loc);
    /*
     *  Give the proxy first shot at the filter. If it returns false, ask
     *  the filter to do it.
     */
    return (proxy && proxy->filterImage(this, src, ctm, result, loc)) ||
           this->onFilterImage(proxy, src, ctm, result, loc);
}

bool SkImageFilter::filterBounds(const SkIRect& src, const SkMatrix& ctm,
                                 SkIRect* dst) {
    SkASSERT(&src);
    SkASSERT(dst);
    return this->onFilterBounds(src, ctm, dst);
}

bool SkImageFilter::onFilterImage(Proxy*, const SkBitmap&, const SkMatrix&,
                                  SkBitmap*, SkIPoint*) {
    return false;
}

bool SkImageFilter::canFilterImageGPU() const {
    return false;
}

GrTexture* SkImageFilter::onFilterImageGPU(Proxy* proxy, GrTexture* texture, const SkRect& rect) {
    return NULL;
}

bool SkImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                   SkIRect* dst) {
    *dst = src;
    return true;
}

bool SkImageFilter::asNewCustomStage(GrCustomStage**, GrTexture*) const {
    return false;
}

SkColorFilter* SkImageFilter::asColorFilter() const {
    return NULL;
}
