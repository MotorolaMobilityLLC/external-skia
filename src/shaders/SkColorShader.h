/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorShader_DEFINED
#define SkColorShader_DEFINED

#include "SkColorSpaceXformer.h"
#include "SkShaderBase.h"
#include "SkPM4f.h"

/** \class SkColorShader
    A Shader that represents a single color. In general, this effect can be
    accomplished by just using the color field on the paint, but if an
    actual shader object is needed, this provides that feature.
*/
class SkColorShader : public SkShaderBase {
public:
    /** Create a ColorShader that ignores the color in the paint, and uses the
        specified color. Note: like all shaders, at draw time the paint's alpha
        will be respected, and is applied to the specified color.
    */
    explicit SkColorShader(SkColor c);

    bool isOpaque() const override;
    bool isConstant() const override { return true; }

    class ColorShaderContext : public Context {
    public:
        ColorShaderContext(const SkColorShader& shader, const ContextRec&);

        uint32_t getFlags() const override;
        void shadeSpan(int x, int y, SkPMColor span[], int count) override;
        void shadeSpan4f(int x, int y, SkPMColor4f[], int count) override;

    private:
        SkPMColor4f fPMColor4f;
        SkPMColor   fPMColor;
        uint32_t    fFlags;

        typedef Context INHERITED;
    };

    GradientType asAGradient(GradientInfo* info) const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkColorShader)

    void flatten(SkWriteBuffer&) const override;
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc* storage) const override;
#endif

    bool onAsLuminanceColor(SkColor* lum) const override {
        *lum = fColor;
        return true;
    }

    bool onAppendStages(const StageRec&) const override;

    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override {
        return SkShader::MakeColorShader(xformer->apply(fColor));
    }

    SkColor fColor;
};

class SkColor4Shader : public SkShaderBase {
public:
    SkColor4Shader(const SkColor4f&, sk_sp<SkColorSpace>);

    bool isOpaque()   const override { return fColor.isOpaque(); }
    bool isConstant() const override { return true; }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkColor4Shader)

    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const StageRec&) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

    sk_sp<SkColorSpace> fColorSpace;
    const SkColor4f     fColor;
};

#endif
