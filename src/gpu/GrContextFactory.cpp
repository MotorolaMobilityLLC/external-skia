
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContextFactory.h"

#if SK_ANGLE
    #include "gl/angle/SkANGLEGLContext.h"
#endif
#if SK_COMMAND_BUFFER
    #include "gl/command_buffer/SkCommandBufferGLContext.h"
#endif
#include "gl/debug/SkDebugGLContext.h"
#if SK_MESA
    #include "gl/mesa/SkMesaGLContext.h"
#endif
#include "gl/SkGLContext.h"
#include "gl/SkNullGLContext.h"
#include "gl/GrGLGpu.h"
#include "GrCaps.h"

GrContextFactory::ContextInfo* GrContextFactory::getContextInfo(GLContextType type,
                                                                GrGLStandard forcedGpuAPI) {
    for (int i = 0; i < fContexts.count(); ++i) {
        if (forcedGpuAPI != kNone_GrGLStandard &&
            forcedGpuAPI != fContexts[i]->fGLContext->gl()->fStandard)
            continue;

        if (fContexts[i]->fType == type) {
            fContexts[i]->fGLContext->makeCurrent();
            return fContexts[i];
        }
    }
    SkAutoTUnref<SkGLContext> glCtx;
    SkAutoTUnref<GrContext> grCtx;
    switch (type) {
        case kNVPR_GLContextType: // fallthru
        case kNative_GLContextType:
            glCtx.reset(SkCreatePlatformGLContext(forcedGpuAPI));
            break;
#ifdef SK_ANGLE
        case kANGLE_GLContextType:
            glCtx.reset(SkANGLEGLContext::Create(forcedGpuAPI, false));
            break;
        case kANGLE_GL_GLContextType:
            glCtx.reset(SkANGLEGLContext::Create(forcedGpuAPI, true));
            break;
#endif
#ifdef SK_COMMAND_BUFFER
        case kCommandBuffer_GLContextType:
            glCtx.reset(SkCommandBufferGLContext::Create(forcedGpuAPI));
            break;
#endif
#ifdef SK_MESA
        case kMESA_GLContextType:
            glCtx.reset(SkMesaGLContext::Create(forcedGpuAPI));
            break;
#endif
        case kNull_GLContextType:
            glCtx.reset(SkNullGLContext::Create(forcedGpuAPI));
            break;
        case kDebug_GLContextType:
            glCtx.reset(SkDebugGLContext::Create(forcedGpuAPI));
            break;
    }
    if (nullptr == glCtx.get()) {
        return nullptr;
    }

    SkASSERT(glCtx->isValid());

    // Block NVPR from non-NVPR types.
    SkAutoTUnref<const GrGLInterface> glInterface(SkRef(glCtx->gl()));
    if (kNVPR_GLContextType != type) {
        glInterface.reset(GrGLInterfaceRemoveNVPR(glInterface));
        if (!glInterface) {
            return nullptr;
        }
    }

    glCtx->makeCurrent();
    GrBackendContext p3dctx = reinterpret_cast<GrBackendContext>(glInterface.get());
#ifdef SK_VULKAN
    grCtx.reset(GrContext::Create(kVulkan_GrBackend, p3dctx, fGlobalOptions));
#else
    grCtx.reset(GrContext::Create(kOpenGL_GrBackend, p3dctx, fGlobalOptions));
#endif
    if (!grCtx.get()) {
        return nullptr;
    }
    if (kNVPR_GLContextType == type) {
        if (!grCtx->caps()->shaderCaps()->pathRenderingSupport()) {
            return nullptr;
        }
    }

    ContextInfo* ctx = fContexts.emplace_back(new ContextInfo);
    ctx->fGLContext = SkRef(glCtx.get());
    ctx->fGrContext = SkRef(grCtx.get());
    ctx->fType = type;
    return ctx;
}
