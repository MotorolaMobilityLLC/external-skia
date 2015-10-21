/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramDataManager_DEFINED
#define GrGLProgramDataManager_DEFINED

#include "GrAllocator.h"
#include "gl/GrGLShaderVar.h"
#include "gl/GrGLTypes.h"

#include "SkTArray.h"

class GrGLGpu;
class SkMatrix;
class GrGLProgram;
class GrGLProgramBuilder;

/** Manages the resources used by a shader program.
 * The resources are objects the program uses to communicate with the
 * application code.
 */
class GrGLProgramDataManager : SkNoncopyable {
public:
    // Opaque handle to a resource
    class ShaderResourceHandle {
    public:
        ShaderResourceHandle(int value)
            : fValue(value) {
            SkASSERT(this->isValid());
        }

        ShaderResourceHandle()
            : fValue(kInvalid_ShaderResourceHandle) {
        }

        bool operator==(const ShaderResourceHandle& other) const { return other.fValue == fValue; }
        bool isValid() const { return kInvalid_ShaderResourceHandle != fValue; }
        int toIndex() const { SkASSERT(this->isValid()); return fValue; }

    private:
        static const int kInvalid_ShaderResourceHandle = -1;
        int fValue;
    };

    typedef ShaderResourceHandle UniformHandle;

    struct UniformInfo {
        GrGLShaderVar fVariable;
        uint32_t      fVisibility;
        GrGLint       fLocation;
    };

    struct SeparableVaryingInfo {
        GrGLShaderVar fVariable;
        GrGLint       fLocation;
    };

    // This uses an allocator rather than array so that the GrGLShaderVars don't move in memory
    // after they are inserted. Users of GrGLShaderBuilder get refs to the vars and ptrs to their
    // name strings. Otherwise, we'd have to hand out copies.
    typedef GrTAllocator<UniformInfo> UniformInfoArray;
    typedef GrTAllocator<SeparableVaryingInfo> SeparableVaryingInfoArray;

    GrGLProgramDataManager(GrGLGpu*, GrGLuint programID, const UniformInfoArray&,
                           const SeparableVaryingInfoArray&);

    /** Functions for uploading uniform values. The varities ending in v can be used to upload to an
     *  array of uniforms. arrayCount must be <= the array count of the uniform.
     */
    void setSampler(UniformHandle, GrGLint texUnit) const;
    void set1f(UniformHandle, GrGLfloat v0) const;
    void set1fv(UniformHandle, int arrayCount, const GrGLfloat v[]) const;
    void set2f(UniformHandle, GrGLfloat, GrGLfloat) const;
    void set2fv(UniformHandle, int arrayCount, const GrGLfloat v[]) const;
    void set3f(UniformHandle, GrGLfloat, GrGLfloat, GrGLfloat) const;
    void set3fv(UniformHandle, int arrayCount, const GrGLfloat v[]) const;
    void set4f(UniformHandle, GrGLfloat, GrGLfloat, GrGLfloat, GrGLfloat) const;
    void set4fv(UniformHandle, int arrayCount, const GrGLfloat v[]) const;
    // matrices are column-major, the first three upload a single matrix, the latter three upload
    // arrayCount matrices into a uniform array.
    void setMatrix3f(UniformHandle, const GrGLfloat matrix[]) const;
    void setMatrix4f(UniformHandle, const GrGLfloat matrix[]) const;
    void setMatrix3fv(UniformHandle, int arrayCount, const GrGLfloat matrices[]) const;
    void setMatrix4fv(UniformHandle, int arrayCount, const GrGLfloat matrices[]) const;

    // convenience method for uploading a SkMatrix to a 3x3 matrix uniform
    void setSkMatrix(UniformHandle, const SkMatrix&) const;

    // for nvpr only
    typedef GrGLProgramDataManager::ShaderResourceHandle SeparableVaryingHandle;
    void setPathFragmentInputTransform(SeparableVaryingHandle u, int components,
                                       const SkMatrix& matrix) const;

private:
    enum {
        kUnusedUniform = -1,
    };

    struct Uniform {
        GrGLint     fVSLocation;
        GrGLint     fFSLocation;
        SkDEBUGCODE(
            GrSLType    fType;
            int         fArrayCount;
        );
    };

    enum {
        kUnusedSeparableVarying = -1,
    };
    struct SeparableVarying {
        GrGLint     fLocation;
        SkDEBUGCODE(
            GrSLType    fType;
            int         fArrayCount;
        );
    };

    SkDEBUGCODE(void printUnused(const Uniform&) const;)

    SkTArray<Uniform, true> fUniforms;
    SkTArray<SeparableVarying, true> fSeparableVaryings;
    GrGLGpu* fGpu;
    GrGLuint fProgramID;

    typedef SkNoncopyable INHERITED;
};

#endif
