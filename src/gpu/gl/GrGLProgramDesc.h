/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramDesc_DEFINED
#define GrGLProgramDesc_DEFINED

#include "GrGLEffect.h"
#include "GrDrawState.h"
#include "GrGpu.h"

class GrGpuGL;

#ifdef SK_DEBUG
  // Optionally compile the experimental GS code. Set to SK_DEBUG so that debug build bots will
  // execute the code.
  #define GR_GL_EXPERIMENTAL_GS 1
#else
  #define GR_GL_EXPERIMENTAL_GS 0
#endif


/** This class describes a program to generate. It also serves as a program cache key. Very little
    of this is GL-specific. There is the generation of GrGLEffect::EffectKeys and the dst-read part
    of the key set by GrGLShaderBuilder. If the interfaces that set those portions were abstracted
    to be API-neutral then so could this class. */
class GrGLProgramDesc {
public:
    GrGLProgramDesc() {}
    GrGLProgramDesc(const GrGLProgramDesc& desc) { *this = desc; }

    // Returns this as a uint32_t array to be used as a key in the program cache.
    const uint32_t* asKey() const {
        return reinterpret_cast<const uint32_t*>(fKey.begin());
    }

    // Gets the number of bytes in asKey(). It will be a 4-byte aligned value. When comparing two
    // keys the size of either key can be used with memcmp() since the lengths themselves begin the
    // keys and thus the memcmp will exit early if the keys are of different lengths.
    uint32_t keyLength() const { return *this->atOffset<uint32_t, kLengthOffset>(); }

    // Gets the a checksum of the key. Can be used as a hash value for a fast lookup in a cache.
    uint32_t getChecksum() const { return *this->atOffset<uint32_t, kChecksumOffset>(); }

    // For unit testing.
    bool setRandom(SkRandom*,
                   const GrGpuGL* gpu,
                   const GrRenderTarget* dummyDstRenderTarget,
                   const GrTexture* dummyDstCopyTexture,
                   const GrEffectStage* stages[],
                   int numColorStages,
                   int numCoverageStages,
                   int currAttribIndex);

    /**
     * Builds a program descriptor from a GrDrawState. Whether the primitive type is points, the
     * output of GrDrawState::getBlendOpts, and the caps of the GrGpuGL are also inputs. It also
     * outputs the color and coverage stages referenced by the generated descriptor. This may
     * not contain all stages from the draw state and coverage stages from the drawState may
     * be treated as color stages in the output.
     */
    static bool Build(const GrDrawState&,
                      GrGpu::DrawType drawType,
                      GrDrawState::BlendOptFlags,
                      GrBlendCoeff srcCoeff,
                      GrBlendCoeff dstCoeff,
                      const GrGpuGL* gpu,
                      const GrDeviceCoordTexture* dstCopy,
                      SkTArray<const GrEffectStage*, true>* outColorStages,
                      SkTArray<const GrEffectStage*, true>* outCoverageStages,
                      GrGLProgramDesc* outDesc);

    int numColorEffects() const {
        return this->getHeader().fColorEffectCnt;
    }

    int numCoverageEffects() const {
        return this->getHeader().fCoverageEffectCnt;
    }

    int numTotalEffects() const { return this->numColorEffects() + this->numCoverageEffects(); }

    GrGLProgramDesc& operator= (const GrGLProgramDesc& other);

    bool operator== (const GrGLProgramDesc& other) const {
        // The length is masked as a hint to the compiler that the address will be 4 byte aligned.
        return 0 == memcmp(this->asKey(), other.asKey(), this->keyLength() & ~0x3);
    }

    bool operator!= (const GrGLProgramDesc& other) const {
        return !(*this == other);
    }

    static bool Less(const GrGLProgramDesc& a, const GrGLProgramDesc& b) {
        return memcmp(a.asKey(), b.asKey(), a.keyLength() & ~0x3) < 0;
    }

private:
    // Specifies where the initial color comes from before the stages are applied.
    enum ColorInput {
        kSolidWhite_ColorInput,
        kTransBlack_ColorInput,
        kAttribute_ColorInput,
        kUniform_ColorInput,

        kColorInputCnt
    };

    enum CoverageOutput {
        // modulate color and coverage, write result as the color output.
        kModulate_CoverageOutput,
        // Writes color*coverage as the primary color output and also writes coverage as the
        // secondary output. Only set if dual source blending is supported.
        kSecondaryCoverage_CoverageOutput,
        // Writes color*coverage as the primary color output and also writes coverage * (1 - colorA)
        // as the secondary output. Only set if dual source blending is supported.
        kSecondaryCoverageISA_CoverageOutput,
        // Writes color*coverage as the primary color output and also writes coverage *
        // (1 - colorRGB) as the secondary output. Only set if dual source blending is supported.
        kSecondaryCoverageISC_CoverageOutput,
        // Combines the coverage, dst, and color as coverage * color + (1 - coverage) * dst. This
        // can only be set if fDstReadKey is non-zero.
        kCombineWithDst_CoverageOutput,

        kCoverageOutputCnt
    };

    static bool CoverageOutputUsesSecondaryOutput(CoverageOutput co) {
        switch (co) {
            case kSecondaryCoverage_CoverageOutput: //  fallthru
            case kSecondaryCoverageISA_CoverageOutput:
            case kSecondaryCoverageISC_CoverageOutput:
                return true;
            default:
                return false;
        }
    }

    struct KeyHeader {
        uint8_t                     fDstReadKey;        // set by GrGLShaderBuilder if there
                                                        // are effects that must read the dst.
                                                        // Otherwise, 0.
        uint8_t                     fFragPosKey;        // set by GrGLShaderBuilder if there are
                                                        // effects that read the fragment position.
                                                        // Otherwise, 0.
        ColorInput                  fColorInput : 8;
        ColorInput                  fCoverageInput : 8;
        CoverageOutput              fCoverageOutput : 8;

        SkBool8                     fHasVertexCode;
        SkBool8                     fEmitsPointSize;

        // To enable experimental geometry shader code (not for use in
        // production)
#if GR_GL_EXPERIMENTAL_GS
        SkBool8                     fExperimentalGS;
#endif

        int8_t                      fPositionAttributeIndex;
        int8_t                      fLocalCoordAttributeIndex;
        int8_t                      fColorAttributeIndex;
        int8_t                      fCoverageAttributeIndex;

        int8_t                      fColorEffectCnt;
        int8_t                      fCoverageEffectCnt;
    };

    // The key, stored in fKey, is composed of five parts:
    // 1. uint32_t for total key length.
    // 2. uint32_t for a checksum.
    // 3. Header struct defined above.
    // 4. uint32_t offsets to beginning of every effects' key (see 5).
    // 5. per-effect keys. Each effect's key is a variable length array of uint32_t.
    enum {
        kLengthOffset = 0,
        kChecksumOffset = kLengthOffset + sizeof(uint32_t),
        kHeaderOffset = kChecksumOffset + sizeof(uint32_t),
        kHeaderSize = SkAlign4(sizeof(KeyHeader)),
        kEffectKeyLengthsOffset = kHeaderOffset + kHeaderSize,
    };

    template<typename T, size_t OFFSET> T* atOffset() {
        return reinterpret_cast<T*>(reinterpret_cast<intptr_t>(fKey.begin()) + OFFSET);
    }

    template<typename T, size_t OFFSET> const T* atOffset() const {
        return reinterpret_cast<const T*>(reinterpret_cast<intptr_t>(fKey.begin()) + OFFSET);
    }

    typedef GrBackendEffectFactory::EffectKey EffectKey;

    KeyHeader* header() { return this->atOffset<KeyHeader, kHeaderOffset>(); }

    void finalize();

    const KeyHeader& getHeader() const { return *this->atOffset<KeyHeader, kHeaderOffset>(); }

    /** Used to provide effects' keys to their emitCode() function. */
    class EffectKeyProvider {
    public:
        enum EffectType {
            kColor_EffectType,
            kCoverage_EffectType,
        };

        EffectKeyProvider(const GrGLProgramDesc* desc, EffectType type) : fDesc(desc) {
            // Coverage effect key offsets begin immediately after those of the color effects.
            fBaseIndex = kColor_EffectType == type ? 0 : desc->numColorEffects();
        }

        EffectKey get(int index) const {
            const uint32_t* offsets = reinterpret_cast<const uint32_t*>(fDesc->fKey.begin() +
                                                                        kEffectKeyLengthsOffset);
            uint32_t offset = offsets[fBaseIndex + index];
            return *reinterpret_cast<const EffectKey*>(fDesc->fKey.begin() + offset);
        }
    private:
        const GrGLProgramDesc*  fDesc;
        int                     fBaseIndex;
    };

    enum {
        kMaxPreallocEffects = 8,
        kIntsPerEffect      = 4,    // This is an overestimate of the average effect key size.
        kPreAllocSize = kEffectKeyLengthsOffset +
                        kMaxPreallocEffects * sizeof(uint32_t) * kIntsPerEffect,
    };

    SkSTArray<kPreAllocSize, uint8_t, true> fKey;

    // GrGLProgram and GrGLShaderBuilder read the private fields to generate code. TODO: Split out
    // part of GrGLShaderBuilder that is used by effects so that this header doesn't need to be
    // visible to GrGLEffects. Then make public accessors as necessary and remove friends.
    friend class GrGLProgram;
    friend class GrGLShaderBuilder;
    friend class GrGLFullShaderBuilder;
    friend class GrGLFragmentOnlyShaderBuilder;
};

#endif
