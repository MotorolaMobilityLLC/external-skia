/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterShader_DEFINED
#define SkColorFilterShader_DEFINED

#include "SkColorFilter.h"
#include "SkShaderBase.h"

class SkArenaAlloc;

class SkColorFilterShader : public SkShaderBase {
public:
    SkColorFilterShader(sk_sp<SkShader> shader, sk_sp<SkColorFilter> filter);

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    class FilterShaderContext : public Context {
    public:
        // Takes ownership of shaderContext and calls its destructor.
        FilterShaderContext(const SkColorFilterShader&, SkShaderBase::Context*, const ContextRec&);

        uint32_t getFlags() const override;

        void shadeSpan(int x, int y, SkPMColor[], int count) override;
        void shadeSpan4f(int x, int y, SkPM4f[], int count) override;

        void set3DMask(const SkMask* mask) override {
            // forward to our proxy
            fShaderContext->set3DMask(mask);
        }

    private:
        SkShaderBase::Context* fShaderContext;

        typedef Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorFilterShader)

protected:
    void flatten(SkWriteBuffer&) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc* alloc) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;
    bool onAppendStages(SkRasterPipeline*, SkColorSpace* dstCS, SkArenaAlloc*,
                        const SkMatrix&, const SkPaint&, const SkMatrix* localM) const override;
    bool isRasterPipelineOnly() const override {
#ifndef SK_SUPPORT_LEGACY_COLORFILTER_FILTERSPAN
        return true;
#endif
        return as_SB(fShader)->isRasterPipelineOnly();
    }

private:
    sk_sp<SkShader>      fShader;
    sk_sp<SkColorFilter> fFilter;

    typedef SkShaderBase INHERITED;
};

#endif
