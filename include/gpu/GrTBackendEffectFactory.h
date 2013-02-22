/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTBackendEffectFactory_DEFINED
#define GrTBackendEffectFactory_DEFINED

#include "GrBackendEffectFactory.h"
#include "GrEffectStage.h"

/**
 * Implements GrBackendEffectFactory for a GrEffect subclass as a singleton.
 */
template <typename EffectClass>
class GrTBackendEffectFactory : public GrBackendEffectFactory {

public:
    typedef typename EffectClass::GLEffect GLEffect;

    /** Returns a human-readable name that is accessible via GrEffect or
        GrGLEffect and is consistent between the two of them.
     */
    virtual const char* name() const SK_OVERRIDE { return EffectClass::Name(); }

    /** Returns a value that identifies the GLSL shader code generated by
        a GrEffect. This enables caching of generated shaders. Part of the
        id identifies the GrEffect subclass. The remainder is based
        on the aspects of the GrEffect object's configuration that affect
        GLSL code generation. */
    virtual EffectKey glEffectKey(const GrEffectStage& stage,
                                  const GrGLCaps& caps) const SK_OVERRIDE {
        GrAssert(kIllegalEffectClassID != fEffectClassID);
        EffectKey effectKey = GLEffect::GenKey(stage, caps);
        EffectKey textureKey = GLEffect::GenTextureKey(stage.getEffect(), caps);
#if GR_DEBUG
        static const EffectKey kIllegalIDMask = (uint16_t) (~((1U << kEffectKeyBits) - 1));
        GrAssert(!(kIllegalIDMask & effectKey));

        static const EffectKey kIllegalTextureKeyMask = (uint16_t) (~((1U << kTextureKeyBits) - 1));
        GrAssert(!(kIllegalTextureKeyMask & textureKey));
#endif
        return fEffectClassID | (textureKey << kEffectKeyBits) | effectKey;
    }

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrEffect; caller is responsible for deleting
        the object. */
    virtual GLEffect* createGLInstance(const GrEffectRef& effect) const SK_OVERRIDE {
        return SkNEW_ARGS(GLEffect, (*this, effect));
    }

    /** This class is a singleton. This function returns the single instance.
     */
    static const GrBackendEffectFactory& getInstance() {
        static SkAlignedSTStorage<1, GrTBackendEffectFactory> gInstanceMem;
        static const GrTBackendEffectFactory* gInstance;
        if (!gInstance) {
            gInstance = SkNEW_PLACEMENT(gInstanceMem.get(),
                                        GrTBackendEffectFactory);
        }
        return *gInstance;
    }

protected:
    GrTBackendEffectFactory() {
        fEffectClassID = GenID() << (kEffectKeyBits + kTextureKeyBits) ;
    }
};

#endif
