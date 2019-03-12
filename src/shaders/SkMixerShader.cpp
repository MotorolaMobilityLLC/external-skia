/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkMixerShader.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

sk_sp<SkShader> SkShader::MakeMixer(sk_sp<SkShader> s0, sk_sp<SkShader> s1, sk_sp<SkMixer> mixer) {
    if (!mixer) {
        return nullptr;
    }
    if (!s0) {
        return s1;
    }
    if (!s1) {
        return s0;
    }
    return sk_sp<SkShader>(new SkShader_Mixer(std::move(s0), std::move(s1), std::move(mixer)));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkShader_Mixer::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> s0(buffer.readShader());
    sk_sp<SkShader> s1(buffer.readShader());
    sk_sp<SkMixer>  mx(buffer.readMixer());

    return MakeMixer(std::move(s0), std::move(s1), std::move(mx));
}

void SkShader_Mixer::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader0.get());
    buffer.writeFlattenable(fShader1.get());
    buffer.writeFlattenable(fMixer.get());
}

bool SkShader_Mixer::onAppendStages(const StageRec& rec) const {
    struct Storage {
        float   fRGBA[4 * SkRasterPipeline_kMaxStride];
    };
    auto storage = rec.fAlloc->make<Storage>();

    if (!as_SB(fShader1)->appendStages(rec)) {
        return false;
    }
    rec.fPipeline->append(SkRasterPipeline::store_src, storage->fRGBA);

    if (!as_SB(fShader0)->appendStages(rec)) {
        return false;
    }
    // r,g,b,a are good, as output by fShader0
    // need to restore our previously computed dr,dg,db,da
    rec.fPipeline->append(SkRasterPipeline::load_dst, storage->fRGBA);

    // 1st color in  r, g, b, a
    // 2nd color in dr,dg,db,da
    // The mixer's output will be in r,g,b,a
    return as_MB(fMixer)->appendStages(rec.fPipeline, rec.fDstCS, rec.fAlloc);
}

#if SK_SUPPORT_GPU

#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

/////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor>
SkShader_Mixer::asFragmentProcessor(const GrFPArgs& args) const {
    std::unique_ptr<GrFragmentProcessor> fpA(as_SB(fShader0)->asFragmentProcessor(args));
    if (!fpA) {
        return nullptr;
    }
    std::unique_ptr<GrFragmentProcessor> fpB(as_SB(fShader1)->asFragmentProcessor(args));
    if (!fpB) {
        return nullptr;
    }

    // TODO: need to make a mixer-processor...
    return nullptr;
    //return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB), std::move(fpA), fMode);
}
#endif
