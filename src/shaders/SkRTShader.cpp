/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkRTShader.h"

#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLInterpreter.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrFPArgs.h"
#include "src/gpu/effects/GrSkSLFP.h"
#endif

SkRTShader::SkRTShader(sk_sp<SkRuntimeEffect> effect, sk_sp<SkData> inputs,
                       const SkMatrix* localMatrix, sk_sp<SkShader>* children, size_t childCount,
                       bool isOpaque)
        : SkShaderBase(localMatrix)
        , fEffect(std::move(effect))
        , fIsOpaque(isOpaque)
        , fInputs(std::move(inputs))
        , fChildren(children, children + childCount) {
}

SkRTShader::~SkRTShader() = default;

bool SkRTShader::onAppendStages(const SkStageRec& rec) const {
    SkMatrix inverse;
    if (!this->computeTotalInverse(rec.fCTM, rec.fLocalM, &inverse)) {
        return false;
    }

    auto ctx = rec.fAlloc->make<SkRasterPipeline_InterpreterCtx>();
    ctx->paintColor = rec.fPaint.getColor4f();
    ctx->inputs = fInputs->data();
    ctx->ninputs = fEffect->uniformSize() / 4;
    ctx->shaderConvention = true;

    SkAutoMutexExclusive ama(fInterpreterMutex);
    if (!fInterpreter) {
        auto [byteCode, errorText] = fEffect->toByteCode(fInputs->data());
        if (!byteCode) {
            SkDebugf("%s\n", errorText.c_str());
            return false;
        }
        fMain = byteCode->getFunction("main");
        fInterpreter.reset(new SkSL::Interpreter<SkRasterPipeline_InterpreterCtx::VECTOR_WIDTH>(
                                                                      std::move(byteCode)));
    }
    ctx->fn = fMain;
    ctx->interpreter = fInterpreter.get();

    rec.fPipeline->append(SkRasterPipeline::seed_shader);
    rec.fPipeline->append_matrix(rec.fAlloc, inverse);
    rec.fPipeline->append(SkRasterPipeline::interpreter, ctx);
    return true;
}

enum Flags {
    kIsOpaque_Flag          = 1 << 0,
    kHasLocalMatrix_Flag    = 1 << 1,
};

void SkRTShader::flatten(SkWriteBuffer& buffer) const {
    uint32_t flags = 0;
    if (fIsOpaque) {
        flags |= kIsOpaque_Flag;
    }
    if (!this->getLocalMatrix().isIdentity()) {
        flags |= kHasLocalMatrix_Flag;
    }

    buffer.writeString(fEffect->source().c_str());
    if (fInputs) {
        buffer.writeDataAsByteArray(fInputs.get());
    } else {
        buffer.writeByteArray(nullptr, 0);
    }
    buffer.write32(flags);
    if (flags & kHasLocalMatrix_Flag) {
        buffer.writeMatrix(this->getLocalMatrix());
    }
    buffer.write32(fChildren.size());
    for (const auto& child : fChildren) {
        buffer.writeFlattenable(child.get());
    }
}

sk_sp<SkFlattenable> SkRTShader::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> inputs = buffer.readByteArrayAsData();
    uint32_t flags = buffer.read32();

    bool isOpaque = SkToBool(flags & kIsOpaque_Flag);
    SkMatrix localM, *localMPtr = nullptr;
    if (flags & kHasLocalMatrix_Flag) {
        buffer.readMatrix(&localM);
        localMPtr = &localM;
    }

    auto effect = std::get<0>(SkRuntimeEffect::Make(std::move(sksl)));
    if (!effect) {
        buffer.validate(false);
        return nullptr;
    }

    size_t childCount = buffer.read32();
    if (childCount != effect->children().count()) {
        buffer.validate(false);
        return nullptr;
    }

    std::vector<sk_sp<SkShader>> children;
    children.resize(childCount);
    for (size_t i = 0; i < children.size(); ++i) {
        children[i] = buffer.readShader();
    }

    return effect->makeShader(std::move(inputs), children.data(), children.size(), localMPtr,
                              isOpaque);
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkRTShader::asFragmentProcessor(const GrFPArgs& args) const {
    SkMatrix matrix;
    if (!this->totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    auto fp = GrSkSLFP::Make(args.fContext, fEffect, "runtime-shader", fInputs, &matrix);
    for (const auto& child : fChildren) {
        auto childFP = child ? as_SB(child)->asFragmentProcessor(args) : nullptr;
        if (!childFP) {
            // TODO: This is the case that should eventually mean "the original input color"
            return nullptr;
        }
        fp->addChild(std::move(childFP));
    }
    if (GrColorTypeClampType(args.fDstColorInfo->colorType()) != GrClampType::kNone) {
        return GrFragmentProcessor::ClampPremulOutput(std::move(fp));
    } else {
        return std::move(fp);
    }
}
#endif
