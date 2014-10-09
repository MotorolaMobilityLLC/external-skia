/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramBuilder_DEFINED
#define GrGLProgramBuilder_DEFINED

#include "GrGLFragmentShaderBuilder.h"
#include "GrGLGeometryShaderBuilder.h"
#include "GrGLVertexShaderBuilder.h"
#include "../GrGLProgramDataManager.h"
#include "../GrGLUniformHandle.h"

class GrGLInstalledProcessors;

/*
 * This is the base class for a series of interfaces.  This base class *MUST* remain abstract with
 * NO data members because it is used in multiple interface inheritance.
 * Heirarchy:
 *                      GrGLUniformBuilder
 *                     /                  \
 *                GrGLFPBuilder       GrGLGPBuilder
 *                     \                  /
 *                     GrGLProgramBuilder(internal use only)
 */
class GrGLUniformBuilder {
public:
    enum ShaderVisibility {
        kVertex_Visibility   = 0x1,
        kGeometry_Visibility = 0x2,
        kFragment_Visibility = 0x4,
    };

    virtual ~GrGLUniformBuilder() {}

    typedef GrGLProgramDataManager::UniformHandle UniformHandle;

    /** Add a uniform variable to the current program, that has visibility in one or more shaders.
        visibility is a bitfield of ShaderVisibility values indicating from which shaders the
        uniform should be accessible. At least one bit must be set. Geometry shader uniforms are not
        supported at this time. The actual uniform name will be mangled. If outName is not NULL then
        it will refer to the final uniform name after return. Use the addUniformArray variant to add
        an array of uniforms. */
    virtual UniformHandle addUniform(uint32_t visibility,
                                     GrSLType type,
                                     const char* name,
                                     const char** outName = NULL) = 0;
    virtual UniformHandle addUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          const char* name,
                                          int arrayCount,
                                          const char** outName = NULL) = 0;

    virtual const GrGLShaderVar& getUniformVariable(UniformHandle u) const = 0;

    /**
     * Shortcut for getUniformVariable(u).c_str()
     */
    virtual const char* getUniformCStr(UniformHandle u) const = 0;

    virtual const GrGLContextInfo& ctxInfo() const = 0;

    virtual GrGpuGL* gpu() const = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

/* a specialization of the above for GPs.  Lets the user add uniforms, varyings, and VS / FS code */
class GrGLGPBuilder : public virtual GrGLUniformBuilder {
public:
    virtual void addVarying(GrSLType type,
                            const char* name,
                            const char** vsOutName = NULL,
                            const char** fsInName = NULL,
                            GrGLShaderVar::Precision fsPrecision=GrGLShaderVar::kDefault_Precision) = 0;

    // TODO rename getFragmentBuilder
    virtual GrGLGPFragmentBuilder* getFragmentShaderBuilder() = 0;
    virtual GrGLVertexBuilder* getVertexShaderBuilder() = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

/* a specializations for FPs. Lets the user add uniforms and FS code */
class GrGLFPBuilder : public virtual GrGLUniformBuilder {
public:
    virtual GrGLFPFragmentBuilder* getFragmentShaderBuilder() = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

/*
 * Please note - no diamond problems because of virtual inheritance.  Also, both base classes
 * are pure virtual with no data members.  This is the base class for program building.
 * Subclasses are nearly identical but each has their own way of emitting transforms.  State for
 * each of the elements of the shader pipeline, ie vertex, fragment, geometry, etc, lives in those
 * respective builders
*/
class GrGLProgramBuilder : public GrGLGPBuilder,
                           public GrGLFPBuilder {
public:
    /** Generates a shader program.
     *
     * The program implements what is specified in the stages given as input.
     * After successful generation, the builder result objects are available
     * to be used.
     * @return true if generation was successful.
     */
    static GrGLProgram* CreateProgram(const GrOptDrawState&,
                                      const GrGLProgramDesc&,
                                      GrGpu::DrawType,
                                      const GrGeometryStage* inGeometryProcessor,
                                      const GrFragmentStage* inColorStages[],
                                      const GrFragmentStage* inCoverageStages[],
                                      GrGpuGL* gpu);

    virtual UniformHandle addUniform(uint32_t visibility,
                                     GrSLType type,
                                     const char* name,
                                     const char** outName = NULL) SK_OVERRIDE {
        return this->addUniformArray(visibility, type, name, GrGLShaderVar::kNonArray, outName);
    }
    virtual UniformHandle addUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          const char* name,
                                          int arrayCount,
                                          const char** outName = NULL) SK_OVERRIDE;

    virtual const GrGLShaderVar& getUniformVariable(UniformHandle u) const SK_OVERRIDE {
        return fUniforms[u.toShaderBuilderIndex()].fVariable;
    }

    virtual const char* getUniformCStr(UniformHandle u) const SK_OVERRIDE {
        return this->getUniformVariable(u).c_str();
    }

    virtual const GrGLContextInfo& ctxInfo() const SK_OVERRIDE;

    virtual GrGpuGL* gpu() const SK_OVERRIDE { return fGpu; }

    virtual GrGLFPFragmentBuilder* getFragmentShaderBuilder() SK_OVERRIDE { return &fFS; }
    virtual GrGLVertexBuilder* getVertexShaderBuilder() SK_OVERRIDE { return &fVS; }

    virtual void addVarying(GrSLType type,
                            const char* name,
                            const char** vsOutName = NULL,
                            const char** fsInName = NULL,
                            GrGLShaderVar::Precision fsPrecision=GrGLShaderVar::kDefault_Precision);

    // Handles for program uniforms (other than per-effect uniforms)
    struct BuiltinUniformHandles {
        UniformHandle       fViewMatrixUni;
        UniformHandle       fRTAdjustmentUni;
        UniformHandle       fColorUni;
        UniformHandle       fCoverageUni;

        // We use the render target height to provide a y-down frag coord when specifying
        // origin_upper_left is not supported.
        UniformHandle       fRTHeightUni;

        // Uniforms for computing texture coords to do the dst-copy lookup
        UniformHandle       fDstCopyTopLeftUni;
        UniformHandle       fDstCopyScaleUni;
        UniformHandle       fDstCopySamplerUni;
    };

protected:
    static GrGLProgramBuilder* CreateProgramBuilder(const GrGLProgramDesc&,
                                                    const GrOptDrawState&,
                                                    GrGpu::DrawType,
                                                    bool hasGeometryProcessor,
                                                    GrGpuGL*);

    GrGLProgramBuilder(GrGpuGL*, const GrOptDrawState&, const GrGLProgramDesc&);

    const GrOptDrawState& optState() const { return fOptState; }
    const GrGLProgramDesc& desc() const { return fDesc; }
    const GrGLProgramDesc::KeyHeader& header() const { return fDesc.getHeader(); }

    // Generates a name for a variable. The generated string will be name prefixed by the prefix
    // char (unless the prefix is '\0'). It also mangles the name to be stage-specific if we're
    // generating stage code.
    void nameVariable(SkString* out, char prefix, const char* name);
    void setupUniformColorAndCoverageIfNeeded(GrGLSLExpr4* inputColor, GrGLSLExpr4* inputCoverage);
    void createAndEmitProcessors(const GrGeometryStage* geometryProcessor,
                                 const GrFragmentStage* colorStages[],
                                 const GrFragmentStage* coverageStages[],
                                 GrGLSLExpr4* inputColor,
                                 GrGLSLExpr4* inputCoverage);
    template <class ProcessorStage>
    void createAndEmitProcessors(const ProcessorStage*[],
                                 int effectCnt,
                                 const GrGLProgramDesc::EffectKeyProvider&,
                                 GrGLSLExpr4* fsInOutColor,
                                 GrGLInstalledProcessors*);
    void verify(const GrGeometryProcessor&);
    void verify(const GrFragmentProcessor&);
    void emitSamplers(const GrProcessor&,
                      GrGLProcessor::TextureSamplerArray* outSamplers,
                      GrGLInstalledProcessors*);

    // each specific program builder has a distinct transform and must override this function
    virtual void emitTransforms(const GrProcessorStage&,
                                GrGLProcessor::TransformedCoordsArray* outCoords,
                                GrGLInstalledProcessors*);
    GrGLProgram* finalize();
    void bindUniformLocations(GrGLuint programID);
    bool checkLinkStatus(GrGLuint programID);
    void resolveUniformLocations(GrGLuint programID);

    void cleanupProgram(GrGLuint programID, const SkTDArray<GrGLuint>& shaderIDs);
    void cleanupShaders(const SkTDArray<GrGLuint>& shaderIDs);

    // Subclasses create different programs
    virtual GrGLProgram* createProgram(GrGLuint programID);

    void appendUniformDecls(ShaderVisibility, SkString*) const;

    // reset is called by program creator between each processor's emit code.  It increments the
    // stage offset for variable name mangling, and also ensures verfication variables in the
    // fragment shader are cleared.
    void reset() {
        this->enterStage();
        this->addStage();
        fFS.reset();
    }
    void addStage() { fStageIndex++; }

    // This simple class exits the stage and then restores the stage when it goes out of scope
    class AutoStageRestore {
    public:
        AutoStageRestore(GrGLProgramBuilder* pb)
            : fPB(pb), fOutOfStage(pb->fOutOfStage) { pb->exitStage(); }
        ~AutoStageRestore() { fPB->fOutOfStage = fOutOfStage; }
    private:
        GrGLProgramBuilder* fPB;
        bool fOutOfStage;
    };
    class AutoStageAdvance {
    public:
        AutoStageAdvance(GrGLProgramBuilder* pb) : fPB(pb) { fPB->reset(); }
        ~AutoStageAdvance() { fPB->exitStage(); }
    private:
        GrGLProgramBuilder* fPB;
    };
    void exitStage() { fOutOfStage = true; }
    void enterStage() { fOutOfStage = false; }
    int stageIndex() const { return fStageIndex; }

    typedef GrGLProgramDesc::EffectKeyProvider EffectKeyProvider;
    typedef GrGLProgramDataManager::UniformInfo UniformInfo;
    typedef GrGLProgramDataManager::UniformInfoArray UniformInfoArray;

    // number of each input/output type in a single allocation block, used by many builders
    static const int kVarsPerBlock;

    BuiltinUniformHandles fUniformHandles;
    GrGLVertexBuilder fVS;
    GrGLGeometryBuilder fGS;
    GrGLFragmentShaderBuilder fFS;
    bool fOutOfStage;
    int fStageIndex;

    SkAutoTUnref<GrGLInstalledProcessors> fGeometryProcessor;
    SkAutoTUnref<GrGLInstalledProcessors> fColorEffects;
    SkAutoTUnref<GrGLInstalledProcessors> fCoverageEffects;

    const GrOptDrawState& fOptState;
    const GrGLProgramDesc& fDesc;
    GrGpuGL* fGpu;
    UniformInfoArray fUniforms;

    friend class GrGLShaderBuilder;
    friend class GrGLVertexBuilder;
    friend class GrGLFragmentShaderBuilder;
    friend class GrGLGeometryBuilder;
};

/**
 * This class encapsulates an array of GrGLProcessors and their supporting data (coord transforms
 * and textures). It is built by GrGLProgramBuilder, then used to manage the necessary GL
 * state and shader uniforms in GLPrograms.  Its just Plain old data, and as such is entirely public
 *
 * TODO We really don't need this class to have an array of processors.  It makes sense for it
 * to just have one, also break out the transforms
 */
class GrGLInstalledProcessors : public SkRefCnt {
public:
    GrGLInstalledProcessors(int reserveCount, bool hasExplicitLocalCoords = false)
        : fGLProcessors(reserveCount)
        , fSamplers(reserveCount)
        , fTransforms(reserveCount)
        , fHasExplicitLocalCoords(hasExplicitLocalCoords) {
    }

    virtual ~GrGLInstalledProcessors();

    typedef GrGLProgramDataManager::UniformHandle UniformHandle;

    struct Sampler {
        SkDEBUGCODE(Sampler() : fTextureUnit(-1) {})
        UniformHandle  fUniform;
        int            fTextureUnit;
    };

    class ShaderVarHandle {
    public:
        bool isValid() const { return fHandle > -1; }
        ShaderVarHandle() : fHandle(-1) {}
        ShaderVarHandle(int value) : fHandle(value) { SkASSERT(this->isValid()); }
        int handle() const { SkASSERT(this->isValid()); return fHandle; }
        UniformHandle convertToUniformHandle() {
            SkASSERT(this->isValid());
            return GrGLProgramDataManager::UniformHandle::CreateFromUniformIndex(fHandle);
        }

    private:
        int fHandle;
    };

    struct Transform {
        Transform() : fType(kVoid_GrSLType) { fCurrentValue = SkMatrix::InvalidMatrix(); }
        ShaderVarHandle fHandle;
        SkMatrix       fCurrentValue;
        GrSLType       fType;
    };

    void addEffect(GrGLProcessor* effect) { fGLProcessors.push_back(effect); }
    SkTArray<Sampler, true>& addSamplers() { return fSamplers.push_back(); }
    SkTArray<Transform, true>& addTransforms() { return fTransforms.push_back(); }

    SkTArray<GrGLProcessor*>                 fGLProcessors;
    SkTArray<SkSTArray<4, Sampler, true> >   fSamplers;
    SkTArray<SkSTArray<2, Transform, true> > fTransforms;
    bool                                     fHasExplicitLocalCoords;

    friend class GrGLShaderBuilder;
    friend class GrGLVertexShaderBuilder;
    friend class GrGLFragmentShaderBuilder;
    friend class GrGLGeometryShaderBuilder;
};

#endif
