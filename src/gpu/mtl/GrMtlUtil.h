/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlUtil_DEFINED
#define GrMtlUtil_DEFINED

#import <Metal/Metal.h>

#include "include/gpu/GrBackendSurface.h"
#include "include/private/GrMtlTypesPriv.h"
#include "include/private/GrTypesPriv.h"
#include "src/sksl/ir/SkSLProgram.h"

class GrMtlGpu;
class GrSurface;

GR_NORETAIN_BEGIN

/**
 * Returns a id<MTLTexture> to the MTLTexture pointed at by the const void*.
 *
 * TODO: Remove this and the other bridging functions? It's better to cast on the calling
 * side so ARC has more context, and they don't add much value.
 */
SK_ALWAYS_INLINE id<MTLTexture> GrGetMTLTexture(const void* mtlTexture) {
#if __has_feature(objc_arc)
    return (__bridge id<MTLTexture>)mtlTexture;
#else
    return (id<MTLTexture>)mtlTexture;
#endif
}

/**
 * Returns a const void* to whatever the id object is pointing to.
 */
SK_ALWAYS_INLINE const void* GrGetPtrFromId(id idObject) {
#if __has_feature(objc_arc)
    return (__bridge const void*)idObject;
#else
    return (const void*)idObject;
#endif
}

/**
 * Returns a const void* to whatever the id object is pointing to.
 * Will call CFRetain on the object.
 */
SK_ALWAYS_INLINE CF_RETURNS_RETAINED const void* GrRetainPtrFromId(id idObject) {
    return CFBridgingRetain(idObject);
}

enum class GrMtlErrorCode {
    kTimeout = 1,
};

NSError* GrCreateMtlError(NSString* description, GrMtlErrorCode errorCode);

/**
 * Returns a MTLTextureDescriptor which describes the MTLTexture. Useful when creating a duplicate
 * MTLTexture without the same storage allocation.
 */
MTLTextureDescriptor* GrGetMTLTextureDescriptor(id<MTLTexture> mtlTexture);

/**
 * Produces MSL code generated by SkSLC
 */
bool GrSkSLToMSL(const GrMtlGpu* gpu,
                 const SkSL::String& sksl,
                 SkSL::ProgramKind kind,
                 const SkSL::Program::Settings& settings,
                 SkSL::String* msl,
                 SkSL::Program::Inputs* outInputs,
                 GrContextOptions::ShaderErrorHandler* errorHandler);

/**
 * Returns a compiled MTLLibrary created from MSL code
 */
id<MTLLibrary> GrCompileMtlShaderLibrary(const GrMtlGpu* gpu,
                                         const SkSL::String& msl,
                                         GrContextOptions::ShaderErrorHandler* errorHandler);

/**
 * Attempts to compile an MSL shader asynchronously. We are not concerned about the result, which
 * will be cached in the Apple shader cache.
 */
void GrPrecompileMtlShaderLibrary(const GrMtlGpu* gpu,
                                  const SkSL::String& msl);

/**
 * Replacement for newLibraryWithSource:options:error that has a timeout.
 */
id<MTLLibrary> GrMtlNewLibraryWithSource(id<MTLDevice>, NSString* mslCode,
                                         MTLCompileOptions*, NSError**);

/**
 * Replacement for newRenderPipelineStateWithDescriptor:error that has a timeout.
 */
id<MTLRenderPipelineState> GrMtlNewRenderPipelineStateWithDescriptor(
        id<MTLDevice>, MTLRenderPipelineDescriptor*, NSError**);

/**
 * Returns a MTLTexture corresponding to the GrSurface.
 */
id<MTLTexture> GrGetMTLTextureFromSurface(GrSurface* surface);

static inline MTLPixelFormat GrBackendFormatAsMTLPixelFormat(const GrBackendFormat& format) {
    return static_cast<MTLPixelFormat>(format.asMtlFormat());
}

/**
 * Returns true if the format is compressed.
 */
bool GrMtlFormatIsCompressed(MTLPixelFormat mtlFormat);

/**
 * Maps a MTLPixelFormat into the CompressionType enum if applicable.
 */
SkImage::CompressionType GrMtlFormatToCompressionType(MTLPixelFormat mtlFormat);

size_t GrMtlFormatBytesPerBlock(MTLPixelFormat);

int GrMtlFormatStencilBits(MTLPixelFormat);

#ifdef SK_BUILD_FOR_IOS
bool GrMtlIsAppInBackground();
#endif

GR_NORETAIN_END

#endif
