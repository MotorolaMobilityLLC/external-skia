/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrEffect.h"
#include "GrBackendEffectFactory.h"
#include "GrContext.h"
#include "GrMemoryPool.h"
#include "SkTLS.h"

SK_DEFINE_INST_COUNT(GrEffect)

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
SkTArray<GrEffectTestFactory*, true>* GrEffectTestFactory::GetFactories() {
    static SkTArray<GrEffectTestFactory*, true> gFactories;
    return &gFactories;
}
#endif

namespace GrEffectUnitTest {
const SkMatrix& TestMatrix(SkRandom* random) {
    static SkMatrix gMatrices[5];
    static bool gOnce;
    if (!gOnce) {
        gMatrices[0].reset();
        gMatrices[1].setTranslate(SkIntToScalar(-100), SkIntToScalar(100));
        gMatrices[2].setRotate(SkIntToScalar(17));
        gMatrices[3].setRotate(SkIntToScalar(185));
        gMatrices[3].postTranslate(SkIntToScalar(66), SkIntToScalar(-33));
        gMatrices[3].postScale(SkIntToScalar(2), SK_ScalarHalf);
        gMatrices[4].setRotate(SkIntToScalar(215));
        gMatrices[4].set(SkMatrix::kMPersp0, SkFloatToScalar(0.00013f));
        gMatrices[4].set(SkMatrix::kMPersp1, SkFloatToScalar(-0.000039f));
        gOnce = true;
    }
    return gMatrices[random->nextULessThan(static_cast<uint32_t>(SK_ARRAY_COUNT(gMatrices)))];
}
}

class GrEffect_Globals {
public:
    static GrMemoryPool* GetTLS() {
        return (GrMemoryPool*)SkTLS::Get(CreateTLS, DeleteTLS);
    }

private:
    static void* CreateTLS() {
        return SkNEW_ARGS(GrMemoryPool, (4096, 4096));
    }

    static void DeleteTLS(void* pool) {
        SkDELETE(reinterpret_cast<GrMemoryPool*>(pool));
    }
};

int32_t GrBackendEffectFactory::fCurrEffectClassID = GrBackendEffectFactory::kIllegalEffectClassID;

GrEffect::GrEffect(int numTextures)
    : fNumTextures(numTextures) {
}

GrEffect::~GrEffect() {

}

bool GrEffect::isOpaque(bool inputTextureIsOpaque) const {
    return false;
}

const char* GrEffect::name() const {
    return this->getFactory().name();
}


bool GrEffect::isEqual(const GrEffect& s) const {
    if (this->numTextures() != s.numTextures()) {
        return false;
    }
    for (int i = 0; i < this->numTextures(); ++i) {
        if (this->textureAccess(i) != s.textureAccess(i)) {
            return false;
        }
    }
    return true;
}

const GrTextureAccess& GrEffect::textureAccess(int index) const {
    GrCrash("We shouldn't be calling this function on the base class.");
    static GrTextureAccess kDummy;
    return kDummy;
}

void * GrEffect::operator new(size_t size) {
    return GrEffect_Globals::GetTLS()->allocate(size);
}

void GrEffect::operator delete(void* target) {
    GrEffect_Globals::GetTLS()->release(target);
}
