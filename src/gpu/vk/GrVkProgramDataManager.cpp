/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkProgramDataManager.h"

#include "GrVkGpu.h"
#include "GrVkUniformBuffer.h"

GrVkProgramDataManager::GrVkProgramDataManager(const UniformInfoArray& uniforms,
                                               uint32_t vertexUniformSize,
                                               uint32_t fragmentUniformSize)
    : fVertexUniformSize(vertexUniformSize)
    , fFragmentUniformSize(fragmentUniformSize) {
    fVertexUniformData.reset(vertexUniformSize);
    fFragmentUniformData.reset(fragmentUniformSize);
    int count = uniforms.count();
    fUniforms.push_back_n(count);
    // We must add uniforms in same order is the UniformInfoArray so that UniformHandles already
    // owned by other objects will still match up here.
    for (int i = 0; i < count; i++) {
        Uniform& uniform = fUniforms[i];
        const GrVkUniformHandler::UniformInfo uniformInfo = uniforms[i];
        SkASSERT(GrGLSLShaderVar::kNonArray == uniformInfo.fVariable.getArrayCount() ||
                 uniformInfo.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = uniformInfo.fVariable.getArrayCount();
            uniform.fType = uniformInfo.fVariable.getType();
        );
        uniform.fBinding = uniformInfo.fBinding;
        uniform.fOffset = uniformInfo.fUBOffset;
        SkDEBUGCODE(
            uniform.fSetNumber = uniformInfo.fSetNumber;
        );
    }
}

void GrVkProgramDataManager::set1f(UniformHandle u, float v0) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType);
    SkASSERT(GrGLSLShaderVar::kNonArray == uni.fArrayCount);
    SkASSERT(GrVkUniformHandler::kUniformBufferDescSet == uni.fSetNumber);
    void* buffer;
    if (GrVkUniformHandler::kVertexBinding == uni.fBinding) {
        buffer = fVertexUniformData.get();
    } else {
        SkASSERT(GrVkUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
    }
    buffer = static_cast<char*>(buffer) + uni.fOffset;
    SkASSERT(sizeof(float) == 4);
    memcpy(buffer, &v0, sizeof(float));
}

void GrVkProgramDataManager::set1fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrGLSLShaderVar::kNonArray == uni.fArrayCount));
    SkASSERT(GrVkUniformHandler::kUniformBufferDescSet == uni.fSetNumber);

    void* buffer;
    if (GrVkUniformHandler::kVertexBinding == uni.fBinding) {
        buffer = fVertexUniformData.get();
    } else {
        SkASSERT(GrVkUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
    }
    buffer = static_cast<char*>(buffer) + uni.fOffset;
    SkASSERT(sizeof(float) == 4);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[i];
        memcpy(buffer, curVec, sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrVkProgramDataManager::set2f(UniformHandle u, float v0, float v1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec2f_GrSLType);
    SkASSERT(GrGLSLShaderVar::kNonArray == uni.fArrayCount);
    SkASSERT(GrVkUniformHandler::kUniformBufferDescSet == uni.fSetNumber);
    void* buffer;
    if (GrVkUniformHandler::kVertexBinding == uni.fBinding) {
        buffer = fVertexUniformData.get();
    } else {
        SkASSERT(GrVkUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
    }
    buffer = static_cast<char*>(buffer) + uni.fOffset;
    SkASSERT(sizeof(float) == 4);
    float v[2] = { v0, v1 };
    memcpy(buffer, v, 2 * sizeof(float));
}

void GrVkProgramDataManager::set2fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec2f_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrGLSLShaderVar::kNonArray == uni.fArrayCount));
    SkASSERT(GrVkUniformHandler::kUniformBufferDescSet == uni.fSetNumber);

    void* buffer;
    if (GrVkUniformHandler::kVertexBinding == uni.fBinding) {
        buffer = fVertexUniformData.get();
    } else {
        SkASSERT(GrVkUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
    }
    buffer = static_cast<char*>(buffer) + uni.fOffset;
    SkASSERT(sizeof(float) == 4);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[2 * i];
        memcpy(buffer, curVec, 2 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrVkProgramDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec3f_GrSLType);
    SkASSERT(GrGLSLShaderVar::kNonArray == uni.fArrayCount);
    SkASSERT(GrVkUniformHandler::kUniformBufferDescSet == uni.fSetNumber);
    void* buffer;
    if (GrVkUniformHandler::kVertexBinding == uni.fBinding) {
        buffer = fVertexUniformData.get();
    } else {
        SkASSERT(GrVkUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
    }
    buffer = static_cast<char*>(buffer) + uni.fOffset;
    SkASSERT(sizeof(float) == 4);
    float v[3] = { v0, v1, v2 };
    memcpy(buffer, v, 3 * sizeof(float));
}

void GrVkProgramDataManager::set3fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec3f_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrGLSLShaderVar::kNonArray == uni.fArrayCount));
    SkASSERT(GrVkUniformHandler::kUniformBufferDescSet == uni.fSetNumber);

    void* buffer;
    if (GrVkUniformHandler::kVertexBinding == uni.fBinding) {
        buffer = fVertexUniformData.get();
    } else {
        SkASSERT(GrVkUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
    }
    buffer = static_cast<char*>(buffer) + uni.fOffset;
    SkASSERT(sizeof(float) == 4);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[3 * i];
        memcpy(buffer, curVec, 3 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrVkProgramDataManager::set4f(UniformHandle u, float v0, float v1, float v2, float v3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec4f_GrSLType);
    SkASSERT(GrGLSLShaderVar::kNonArray == uni.fArrayCount);
    SkASSERT(GrVkUniformHandler::kUniformBufferDescSet == uni.fSetNumber);
    void* buffer;
    if (GrVkUniformHandler::kVertexBinding == uni.fBinding) {
        buffer = fVertexUniformData.get();
    } else {
        SkASSERT(GrVkUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
    }
    buffer = static_cast<char*>(buffer) + uni.fOffset;
    SkASSERT(sizeof(float) == 4);
    float v[4] = { v0, v1, v2, v3 };
    memcpy(buffer, v, 4 * sizeof(float));
}

void GrVkProgramDataManager::set4fv(UniformHandle u,
                                    int arrayCount,
                                    const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kVec4f_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrGLSLShaderVar::kNonArray == uni.fArrayCount));
    SkASSERT(GrVkUniformHandler::kUniformBufferDescSet == uni.fSetNumber);

    void* buffer;
    if (GrVkUniformHandler::kVertexBinding == uni.fBinding) {
        buffer = fVertexUniformData.get();
    } else {
        SkASSERT(GrVkUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
    }
    buffer = static_cast<char*>(buffer) + uni.fOffset;
    SkASSERT(sizeof(float) == 4);
    memcpy(buffer, v, arrayCount * 4 * sizeof(float));
}

void GrVkProgramDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<2>(u, 1, matrix);
}

void GrVkProgramDataManager::setMatrix2fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<2>(u, arrayCount, m);
}

void GrVkProgramDataManager::setMatrix3f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<3>(u, 1, matrix);
}

void GrVkProgramDataManager::setMatrix3fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<3>(u, arrayCount, m);
}

void GrVkProgramDataManager::setMatrix4f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<4>(u, 1, matrix);
}

void GrVkProgramDataManager::setMatrix4fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<4>(u, arrayCount, m);
}

template<int N> struct set_uniform_matrix;

template<int N> inline void GrVkProgramDataManager::setMatrices(UniformHandle u,
                                                                   int arrayCount,
                                                                   const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kMat22f_GrSLType + (N - 2));
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrGLSLShaderVar::kNonArray == uni.fArrayCount));
    SkASSERT(GrVkUniformHandler::kUniformBufferDescSet == uni.fSetNumber);

    void* buffer;
    if (GrVkUniformHandler::kVertexBinding == uni.fBinding) {
        buffer = fVertexUniformData.get();
    } else {
        SkASSERT(GrVkUniformHandler::kFragBinding == uni.fBinding);
        buffer = fFragmentUniformData.get();
    }

    set_uniform_matrix<N>::set(buffer, uni.fOffset, arrayCount, matrices);
}

template<int N> struct set_uniform_matrix {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        GR_STATIC_ASSERT(sizeof(float) == 4);
        buffer = static_cast<char*>(buffer) + uniformOffset;
        for (int i = 0; i < count; ++i) {
            const float* matrix = &matrices[N * N * i];
            memcpy(buffer, &matrix[0], N * sizeof(float));
            buffer = static_cast<char*>(buffer) + 4*sizeof(float);
            memcpy(buffer, &matrix[3], N * sizeof(float));
            buffer = static_cast<char*>(buffer) + 4*sizeof(float);
            memcpy(buffer, &matrix[6], N * sizeof(float));
            buffer = static_cast<char*>(buffer) + 4*sizeof(float);
        }
    }
};

template<> struct set_uniform_matrix<4> {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        GR_STATIC_ASSERT(sizeof(float) == 4);
        buffer = static_cast<char*>(buffer) + uniformOffset;
        memcpy(buffer, matrices, count * 16 * sizeof(float));
    }
};

void GrVkProgramDataManager::uploadUniformBuffers(const GrVkGpu* gpu,
                                                  GrVkUniformBuffer* vertexBuffer,
                                                  GrVkUniformBuffer* fragmentBuffer) const {
    if (vertexBuffer) {
        vertexBuffer->addMemoryBarrier(gpu,
                                       VK_ACCESS_UNIFORM_READ_BIT,
                                       VK_ACCESS_HOST_WRITE_BIT,
                                       VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                                       VK_PIPELINE_STAGE_HOST_BIT,
                                       false);
        SkAssertResult(vertexBuffer->updateData(gpu, fVertexUniformData.get(), fVertexUniformSize));
    }

    if (fragmentBuffer) {
        fragmentBuffer->addMemoryBarrier(gpu,
                                         VK_ACCESS_UNIFORM_READ_BIT,
                                         VK_ACCESS_HOST_WRITE_BIT,
                                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                         VK_PIPELINE_STAGE_HOST_BIT,
                                         false);
        SkAssertResult(fragmentBuffer->updateData(gpu, fFragmentUniformData.get(),
                                                  fFragmentUniformSize));
    }
}

