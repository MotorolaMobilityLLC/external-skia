/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureDomainEffect_DEFINED
#define GrTextureDomainEffect_DEFINED

#include "GrSingleTextureEffect.h"
#include "gl/GrGLProcessor.h"

class GrGLProgramBuilder;
class GrGLShaderBuilder;
class GrInvariantOutput;
struct SkRect;

/**
 * Limits a texture's lookup coordinates to a domain. Samples outside the domain are either clamped
 * the edge of the domain or result in a vec4 of zeros (decal mode). The domain is clipped to
 * normalized texture coords ([0,1]x[0,1] square). Bilinear filtering can cause texels outside the
 * domain to affect the read value unless the caller considers this when calculating the domain.
 */
class GrTextureDomain {
public:
    enum Mode {
        // Ignore the texture domain rectangle.
        kIgnore_Mode,
        // Clamp texture coords to the domain rectangle.
        kClamp_Mode,
        // Treat the area outside the domain rectangle as fully transparent.
        kDecal_Mode,
        // Wrap texture coordinates.  NOTE: filtering may not work as expected because Bilerp will
        // read texels outside of the domain.  We could perform additional texture reads and filter
        // in the shader, but are not currently doing this for performance reasons
        kRepeat_Mode,

        kLastMode = kRepeat_Mode
    };
    static const int kModeCount = kLastMode + 1;

    static const GrTextureDomain& IgnoredDomain() {
        static const SkRect gDummyRect = {0, 0, 0, 0};
        static const GrTextureDomain gDomain(gDummyRect, kIgnore_Mode);
        return gDomain;
    }

    /**
     * @param index     Pass a value >= 0 if using multiple texture domains in the same effect.
     *                  It is used to keep inserted variables from causing name collisions.
     */
    GrTextureDomain(const SkRect& domain, Mode, int index = -1);

    const SkRect& domain() const { return fDomain; }
    Mode mode() const { return fMode; }

    /* Computes a domain that bounds all the texels in texelRect. Note that with bilerp enabled
       texels neighboring the domain may be read. */
    static const SkRect MakeTexelDomain(const GrTexture* texture, const SkIRect& texelRect) {
        SkScalar wInv = SK_Scalar1 / texture->width();
        SkScalar hInv = SK_Scalar1 / texture->height();
        SkRect result = {
            texelRect.fLeft * wInv,
            texelRect.fTop * hInv,
            texelRect.fRight * wInv,
            texelRect.fBottom * hInv
        };
        return result;
    }

    bool operator== (const GrTextureDomain& that) const {
        return fMode == that.fMode && (kIgnore_Mode == fMode || fDomain == that.fDomain);
    }

    /**
     * A GrGLProcessor subclass that corresponds to a GrProcessor subclass that uses GrTextureDomain
     * should include this helper. It generates the texture domain GLSL, produces the part of the
     * effect key that reflects the texture domain code, and performs the uniform uploads necessary
     * for texture domains.
     */
    class GLDomain {
    public:
        GLDomain() {
            fPrevDomain[0] = SK_FloatNaN;
            SkDEBUGCODE(fMode = (Mode) -1;)
        }

        /**
         * Call this from GrGLProcessor::emitCode() to sample the texture W.R.T. the domain and
         * mode.
         *
         * @param outcolor  name of vec4 variable to hold the sampled color.
         * @param inCoords  name of vec2 variable containing the coords to be used with the domain.
         *                  It is assumed that this is a variable and not an expression.
         * @param inModulateColor   if non-NULL the sampled color will be modulated with this
         *                          expression before being written to outColor.
         */
        void sampleTexture(GrGLShaderBuilder* builder,
                           const GrTextureDomain& textureDomain,
                           const char* outColor,
                           const SkString& inCoords,
                           const GrGLProcessor::TextureSampler sampler,
                           const char* inModulateColor = NULL);

        /**
         * Call this from GrGLProcessor::setData() to upload uniforms necessary for the texture
         * domain. The rectangle is automatically adjusted to account for the texture's origin.
         */
        void setData(const GrGLProgramDataManager& pdman, const GrTextureDomain& textureDomain,
                     GrSurfaceOrigin textureOrigin);

        enum {
            kDomainKeyBits = 2, // See DomainKey().
        };

        /**
         * GrGLProcessor::GenKey() must call this and include the returned value in it's computed
         * key. The returned will be limited to the lower kDomainKeyBits bits.
         */
        static uint32_t DomainKey(const GrTextureDomain& domain) {
            GR_STATIC_ASSERT(kModeCount <= 4);
            return domain.mode();
        }

    private:
        SkDEBUGCODE(Mode                      fMode;)
        GrGLProgramDataManager::UniformHandle fDomainUni;
        SkString                              fDomainName;
        GrGLfloat                             fPrevDomain[4];
    };

protected:
    Mode    fMode;
    SkRect  fDomain;
    int     fIndex;

    typedef GrSingleTextureEffect INHERITED;
};

class GrGLTextureDomainEffect;

/**
 * A basic texture effect that uses GrTextureDomain.
 */
class GrTextureDomainEffect : public GrSingleTextureEffect {

public:
    static GrFragmentProcessor* Create(GrTexture*,
                                       const SkMatrix&,
                                       const SkRect& domain,
                                       GrTextureDomain::Mode,
                                       GrTextureParams::FilterMode filterMode,
                                       GrCoordSet = kLocal_GrCoordSet);

    virtual ~GrTextureDomainEffect();

    static const char* Name() { return "TextureDomain"; }

    typedef GrGLTextureDomainEffect GLProcessor;

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE;

    const GrTextureDomain& textureDomain() const { return fTextureDomain; }

protected:
    GrTextureDomain fTextureDomain;

private:
    GrTextureDomainEffect(GrTexture*,
                          const SkMatrix&,
                          const SkRect& domain,
                          GrTextureDomain::Mode,
                          GrTextureParams::FilterMode,
                          GrCoordSet);

    virtual bool onIsEqual(const GrFragmentProcessor&) const SK_OVERRIDE;

    virtual void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
