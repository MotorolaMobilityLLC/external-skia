/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramDesc_DEFINED
#define GrGLProgramDesc_DEFINED

#include "GrColor.h"
#include "GrProgramDesc.h"
#include "GrGpu.h"
#include "GrTypesPriv.h"

class GrGpuGL;

/**
 * This class can be used to build a GrProgramDesc.  It also provides helpers for accessing
 * GL specific info in the header.
 */
class GrGLProgramDescBuilder {
public:
    struct GLKeyHeader : public GrProgramDesc::KeyHeader {
        SkBool8 fUseNvpr;
    };

    // The key, stored in fKey, is composed of five parts(first 2 are defined in the key itself):
    // 1. uint32_t for total key length.
    // 2. uint32_t for a checksum.
    // 3. Header struct defined above.
    // 4. Backend-specific information including per-processor keys and their key lengths.
    //    Each processor's key is a variable length array of uint32_t.
    enum {
        // Part 3.
        kHeaderOffset = GrProgramDesc::kHeaderOffset,
        kHeaderSize = SkAlign4(sizeof(GLKeyHeader)),
        // Part 4.
        // This is the offset into the backenend specific part of the key, which includes
        // per-processor keys.
        kProcessorKeysOffset = kHeaderOffset + kHeaderSize,
    };

    /**
     * Builds a GL specific program descriptor
     *
     * @param GrOptDrawState  The optimized drawstate.  The descriptor will represent a program
     *                        which this optstate can use to draw with.  The optstate contains
     *                        general draw information, as well as the specific color, geometry,
     *                        and coverage stages which will be used to generate the GL Program for
     *                        this optstate.
     * @param DescInfo  A descriptor info struct, generated by the optstate, which contains a number
     *                  of important facts about the program the built descriptor will represent
     * @param DrawType
     * @param GrGpuGL  A GL Gpu, the caps and Gpu object are used to output processor specific
     *                 parts of the descriptor.
     * @param GrDeviceCoordTexture  A dstCopy texture, which may be null if frame buffer fetch is
     *                              supported
     * @param GrProgramDesc  The built and finalized descriptor
     **/
    static bool Build(const GrOptDrawState&,
                      const GrProgramDesc::DescInfo&,
                      GrGpu::DrawType,
                      GrGpuGL*,
                      GrProgramDesc*);

    static const GLKeyHeader& GetHeader(const GrProgramDesc& desc) {
        return *desc.atOffset<GLKeyHeader, kHeaderOffset>();
    }
};

#endif
