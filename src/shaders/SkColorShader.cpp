/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkUtils.h"
#include "src/shaders/SkColorShader.h"

SkColorShader::SkColorShader(SkColor c) : fColor(c) {}

bool SkColorShader::isOpaque() const {
    return SkColorGetA(fColor) == 255;
}

sk_sp<SkFlattenable> SkColorShader::CreateProc(SkReadBuffer& buffer) {
    return sk_make_sp<SkColorShader>(buffer.readColor());
}

void SkColorShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fColor);
}

SkShader::GradientType SkColorShader::asAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColors && info->fColorCount >= 1) {
            info->fColors[0] = fColor;
        }
        info->fColorCount = 1;
        info->fTileMode = SkTileMode::kRepeat;
    }
    return kColor_GradientType;
}

SkColor4Shader::SkColor4Shader(const SkColor4f& color, sk_sp<SkColorSpace> space)
    : fColorSpace(std::move(space))
    , fColor(color)
{}

sk_sp<SkFlattenable> SkColor4Shader::CreateProc(SkReadBuffer& buffer) {
    SkColor4f color;
    sk_sp<SkColorSpace> colorSpace;
    buffer.readColor4f(&color);
    if (buffer.readBool()) {
        sk_sp<SkData> data = buffer.readByteArrayAsData();
        colorSpace = data ? SkColorSpace::Deserialize(data->data(), data->size()) : nullptr;
    }
    return SkShaders::Color(color, std::move(colorSpace));
}

void SkColor4Shader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor4f(fColor);
    sk_sp<SkData> colorSpaceData = fColorSpace ? fColorSpace->serialize() : nullptr;
    if (colorSpaceData) {
        buffer.writeBool(true);
        buffer.writeDataAsByteArray(colorSpaceData.get());
    } else {
        buffer.writeBool(false);
    }
}


sk_sp<SkShader> SkShaders::Color(const SkColor4f& color, sk_sp<SkColorSpace> space) {
    if (!SkScalarsAreFinite(color.vec(), 4)) {
        return nullptr;
    }
    return sk_make_sp<SkColor4Shader>(color, std::move(space));
}

bool SkColorShader::onAppendStages(const SkStageRec& rec) const {
    SkColor4f color = SkColor4f::FromColor(fColor);
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                           rec.fDstCS,          kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->append_constant_color(rec.fAlloc, color.premul().vec());
    return true;
}

bool SkColor4Shader::onAppendStages(const SkStageRec& rec) const {
    SkColor4f color = fColor;
    SkColorSpaceXformSteps(fColorSpace.get(), kUnpremul_SkAlphaType,
                           rec.fDstCS,        kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->append_constant_color(rec.fAlloc, color.premul().vec());
    return true;
}

static bool common_program(SkColor4f color, SkColorSpace* cs,
                           skvm::Builder* p,
                           SkColorSpace* dstCS,
                           skvm::Arg uniforms, SkTDArray<uint32_t>* buf,
                           skvm::I32* r, skvm::I32* g, skvm::I32* b, skvm::I32* a) {
    SkColorSpaceXformSteps(   cs, kUnpremul_SkAlphaType,
                           dstCS, kUnpremul_SkAlphaType).apply(color.vec());

    if (color.fitsInBytes()) {
        skvm::I32 rgba = p->uniform32(uniforms, buf->bytes());
        buf->push_back(color.premul().toBytes_RGBA());

        *r = p->extract(rgba,  0, p->splat(0xff));
        *g = p->extract(rgba,  8, p->splat(0xff));
        *b = p->extract(rgba, 16, p->splat(0xff));
        *a = p->extract(rgba, 24, p->splat(0xff));
        return true;
    }
    return false;
}

bool SkColorShader::onProgram(skvm::Builder* p,
                              SkColorSpace* dstCS,
                              skvm::Arg uniforms, SkTDArray<uint32_t>* buf,
                              skvm::F32 /*x*/, skvm::F32 /*y*/,
                              skvm::I32* r, skvm::I32* g, skvm::I32* b, skvm::I32* a) const {
    return common_program(SkColor4f::FromColor(fColor), sk_srgb_singleton(),
                          p, dstCS, uniforms,buf, r,g,b,a);
}
bool SkColor4Shader::onProgram(skvm::Builder* p,
                               SkColorSpace* dstCS,
                               skvm::Arg uniforms, SkTDArray<uint32_t>* buf,
                               skvm::F32 /*x*/, skvm::F32 /*y*/,
                               skvm::I32* r, skvm::I32* g, skvm::I32* b, skvm::I32* a) const {
    return common_program(fColor, fColorSpace.get(),
                          p, dstCS, uniforms,buf, r,g,b,a);
}

#if SK_SUPPORT_GPU

#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"

std::unique_ptr<GrFragmentProcessor> SkColorShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    SkPMColor4f color = SkColorToPMColor4f(fColor, *args.fDstColorInfo);
    return GrConstColorProcessor::Make(color, GrConstColorProcessor::InputMode::kModulateA);
}

std::unique_ptr<GrFragmentProcessor> SkColor4Shader::asFragmentProcessor(
        const GrFPArgs& args) const {
    SkColorSpaceXformSteps steps{ fColorSpace.get(),                kUnpremul_SkAlphaType,
                                  args.fDstColorInfo->colorSpace(), kUnpremul_SkAlphaType };
    SkColor4f color = fColor;
    steps.apply(color.vec());
    return GrConstColorProcessor::Make(color.premul(),
                                       GrConstColorProcessor::InputMode::kModulateA);
}

#endif
