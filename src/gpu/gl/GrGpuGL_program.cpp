/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpuGL.h"

#include "builders/GrGLProgramBuilder.h"
#include "GrProcessor.h"
#include "GrGLProcessor.h"
#include "GrGLPathRendering.h"
#include "GrOptDrawState.h"
#include "SkRTConf.h"
#include "SkTSearch.h"

#ifdef PROGRAM_CACHE_STATS
SK_CONF_DECLARE(bool, c_DisplayCache, "gpu.displayCache", false,
                "Display program cache usage.");
#endif

typedef GrGLProgramDataManager::UniformHandle UniformHandle;

struct GrGpuGL::ProgramCache::Entry {
    SK_DECLARE_INST_COUNT_ROOT(Entry);
    Entry() : fProgram(NULL), fLRUStamp(0) {}

    SkAutoTUnref<GrGLProgram>   fProgram;
    unsigned int                fLRUStamp;
};

struct GrGpuGL::ProgramCache::ProgDescLess {
    bool operator() (const GrProgramDesc& desc, const Entry* entry) {
        SkASSERT(entry->fProgram.get());
        return GrProgramDesc::Less(desc, entry->fProgram->getDesc());
    }

    bool operator() (const Entry* entry, const GrProgramDesc& desc) {
        SkASSERT(entry->fProgram.get());
        return GrProgramDesc::Less(entry->fProgram->getDesc(), desc);
    }
};

GrGpuGL::ProgramCache::ProgramCache(GrGpuGL* gpu)
    : fCount(0)
    , fCurrLRUStamp(0)
    , fGpu(gpu)
#ifdef PROGRAM_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
    , fHashMisses(0)
#endif
{
    for (int i = 0; i < 1 << kHashBits; ++i) {
        fHashTable[i] = NULL;
    }
}

GrGpuGL::ProgramCache::~ProgramCache() {
    for (int i = 0; i < fCount; ++i){
        SkDELETE(fEntries[i]);
    }
    // dump stats
#ifdef PROGRAM_CACHE_STATS
    if (c_DisplayCache) {
        SkDebugf("--- Program Cache ---\n");
        SkDebugf("Total requests: %d\n", fTotalRequests);
        SkDebugf("Cache misses: %d\n", fCacheMisses);
        SkDebugf("Cache miss %%: %f\n", (fTotalRequests > 0) ?
                                            100.f * fCacheMisses / fTotalRequests :
                                            0.f);
        int cacheHits = fTotalRequests - fCacheMisses;
        SkDebugf("Hash miss %%: %f\n", (cacheHits > 0) ? 100.f * fHashMisses / cacheHits : 0.f);
        SkDebugf("---------------------\n");
    }
#endif
}

void GrGpuGL::ProgramCache::abandon() {
    for (int i = 0; i < fCount; ++i) {
        SkASSERT(fEntries[i]->fProgram.get());
        fEntries[i]->fProgram->abandon();
        SkDELETE(fEntries[i]);
    }
    fCount = 0;
}

int GrGpuGL::ProgramCache::search(const GrProgramDesc& desc) const {
    ProgDescLess less;
    return SkTSearch(fEntries, fCount, desc, sizeof(Entry*), less);
}

GrGLProgram* GrGpuGL::ProgramCache::getProgram(const GrOptDrawState& optState) {
#ifdef PROGRAM_CACHE_STATS
    ++fTotalRequests;
#endif

    Entry* entry = NULL;

    uint32_t hashIdx = optState.programDesc().getChecksum();
    hashIdx ^= hashIdx >> 16;
    if (kHashBits <= 8) {
        hashIdx ^= hashIdx >> 8;
    }
    hashIdx &=((1 << kHashBits) - 1);
    Entry* hashedEntry = fHashTable[hashIdx];
    if (hashedEntry && hashedEntry->fProgram->getDesc() == optState.programDesc()) {
        SkASSERT(hashedEntry->fProgram);
        entry = hashedEntry;
    }

    int entryIdx;
    if (NULL == entry) {
        entryIdx = this->search(optState.programDesc());
        if (entryIdx >= 0) {
            entry = fEntries[entryIdx];
#ifdef PROGRAM_CACHE_STATS
            ++fHashMisses;
#endif
        }
    }

    if (NULL == entry) {
        // We have a cache miss
#ifdef PROGRAM_CACHE_STATS
        ++fCacheMisses;
#endif
        GrGLProgram* program = GrGLProgramBuilder::CreateProgram(optState, fGpu);
        if (NULL == program) {
            return NULL;
        }
        int purgeIdx = 0;
        if (fCount < kMaxEntries) {
            entry = SkNEW(Entry);
            purgeIdx = fCount++;
            fEntries[purgeIdx] = entry;
        } else {
            SkASSERT(fCount == kMaxEntries);
            purgeIdx = 0;
            for (int i = 1; i < kMaxEntries; ++i) {
                if (fEntries[i]->fLRUStamp < fEntries[purgeIdx]->fLRUStamp) {
                    purgeIdx = i;
                }
            }
            entry = fEntries[purgeIdx];
            int purgedHashIdx = entry->fProgram->getDesc().getChecksum() & ((1 << kHashBits) - 1);
            if (fHashTable[purgedHashIdx] == entry) {
                fHashTable[purgedHashIdx] = NULL;
            }
        }
        SkASSERT(fEntries[purgeIdx] == entry);
        entry->fProgram.reset(program);
        // We need to shift fEntries around so that the entry currently at purgeIdx is placed
        // just before the entry at ~entryIdx (in order to keep fEntries sorted by descriptor).
        entryIdx = ~entryIdx;
        if (entryIdx < purgeIdx) {
            //  Let E and P be the entries at index entryIdx and purgeIdx, respectively.
            //  If the entries array looks like this:
            //       aaaaEbbbbbPccccc
            //  we rearrange it to look like this:
            //       aaaaPEbbbbbccccc
            size_t copySize = (purgeIdx - entryIdx) * sizeof(Entry*);
            memmove(fEntries + entryIdx + 1, fEntries + entryIdx, copySize);
            fEntries[entryIdx] = entry;
        } else if (purgeIdx < entryIdx) {
            //  If the entries array looks like this:
            //       aaaaPbbbbbEccccc
            //  we rearrange it to look like this:
            //       aaaabbbbbPEccccc
            size_t copySize = (entryIdx - purgeIdx - 1) * sizeof(Entry*);
            memmove(fEntries + purgeIdx, fEntries + purgeIdx + 1, copySize);
            fEntries[entryIdx - 1] = entry;
        }
#ifdef SK_DEBUG
        SkASSERT(fEntries[0]->fProgram.get());
        for (int i = 0; i < fCount - 1; ++i) {
            SkASSERT(fEntries[i + 1]->fProgram.get());
            const GrProgramDesc& a = fEntries[i]->fProgram->getDesc();
            const GrProgramDesc& b = fEntries[i + 1]->fProgram->getDesc();
            SkASSERT(GrProgramDesc::Less(a, b));
            SkASSERT(!GrProgramDesc::Less(b, a));
        }
#endif
    }

    fHashTable[hashIdx] = entry;
    entry->fLRUStamp = fCurrLRUStamp;

    if (SK_MaxU32 == fCurrLRUStamp) {
        // wrap around! just trash our LRU, one time hit.
        for (int i = 0; i < fCount; ++i) {
            fEntries[i]->fLRUStamp = 0;
        }
    }
    ++fCurrLRUStamp;
    return entry->fProgram;
}

////////////////////////////////////////////////////////////////////////////////

#define GL_CALL(X) GR_GL_CALL(this->glInterface(), X)

bool GrGpuGL::flushGraphicsState(const GrOptDrawState& optState) {
    // GrGpu::setupClipAndFlushState should have already checked this and bailed if not true.
    SkASSERT(optState.getRenderTarget());

    if (kStencilPath_DrawType == optState.drawType()) {
        const GrRenderTarget* rt = optState.getRenderTarget();
        SkISize size;
        size.set(rt->width(), rt->height());
        this->glPathRendering()->setProjectionMatrix(optState.getViewMatrix(), size, rt->origin());
    } else {
        this->flushMiscFixedFunctionState(optState);

        GrBlendCoeff srcCoeff = optState.getSrcBlendCoeff();
        GrBlendCoeff dstCoeff = optState.getDstBlendCoeff();

        fCurrentProgram.reset(fProgramCache->getProgram(optState));
        if (NULL == fCurrentProgram.get()) {
            SkDEBUGFAIL("Failed to create program!");
            return false;
        }

        fCurrentProgram.get()->ref();

        GrGLuint programID = fCurrentProgram->programID();
        if (fHWProgramID != programID) {
            GL_CALL(UseProgram(programID));
            fHWProgramID = programID;
        }

        this->flushBlend(optState, kDrawLines_DrawType == optState.drawType(), srcCoeff, dstCoeff);

        fCurrentProgram->setData(optState);
    }

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(optState.getRenderTarget());
    this->flushStencil(optState.getStencil(), optState.drawType());
    this->flushScissor(optState.getScissorState(), glRT->getViewport(), glRT->origin());
    this->flushAAState(optState);

    // This must come after textures are flushed because a texture may need
    // to be msaa-resolved (which will modify bound FBO state).
    this->flushRenderTarget(glRT, NULL);

    return true;
}

void GrGpuGL::setupGeometry(const GrOptDrawState& optState,
                            const GrDrawTarget::DrawInfo& info,
                            size_t* indexOffsetInBytes) {
    GrGLVertexBuffer* vbuf;
    vbuf = (GrGLVertexBuffer*) info.vertexBuffer();

    SkASSERT(vbuf);
    SkASSERT(!vbuf->isMapped());

    GrGLIndexBuffer* ibuf = NULL;
    if (info.isIndexed()) {
        SkASSERT(indexOffsetInBytes);

        *indexOffsetInBytes = 0;
        ibuf = (GrGLIndexBuffer*)info.indexBuffer();

        SkASSERT(ibuf);
        SkASSERT(!ibuf->isMapped());
        *indexOffsetInBytes += ibuf->baseOffset();
    }
    GrGLAttribArrayState* attribState =
        fHWGeometryState.bindArrayAndBuffersToDraw(this, vbuf, ibuf);

    if (fCurrentProgram->hasVertexShader()) {
        const GrGeometryProcessor* gp = optState.getGeometryProcessor();

        GrGLsizei stride = static_cast<GrGLsizei>(gp->getVertexStride());

        size_t vertexOffsetInBytes = stride * info.startVertex();

        vertexOffsetInBytes += vbuf->baseOffset();

        const SkTArray<GrGeometryProcessor::GrAttribute, true>& attribs = gp->getAttribs();
        int vaCount = attribs.count();
        uint32_t usedAttribArraysMask = 0;
        size_t offset = 0;

        for (int attribIndex = 0; attribIndex < vaCount; attribIndex++) {
            usedAttribArraysMask |= (1 << attribIndex);
            GrVertexAttribType attribType = attribs[attribIndex].fType;
            attribState->set(this,
                             attribIndex,
                             vbuf,
                             GrGLAttribTypeToLayout(attribType).fCount,
                             GrGLAttribTypeToLayout(attribType).fType,
                             GrGLAttribTypeToLayout(attribType).fNormalized,
                             stride,
                             reinterpret_cast<GrGLvoid*>(vertexOffsetInBytes + offset));
            offset += attribs[attribIndex].fOffset;
        }
        attribState->disableUnusedArrays(this, usedAttribArraysMask);
    }
}

void GrGpuGL::buildProgramDesc(const GrOptDrawState& optState,
                               const GrProgramDesc::DescInfo& descInfo,
                               GrGpu::DrawType drawType,
                               GrProgramDesc* desc) {
    if (!GrGLProgramDescBuilder::Build(optState, descInfo, drawType, this, desc)) {
        SkDEBUGFAIL("Failed to generate GL program descriptor");
    }
}
