// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

const ASSEMBLE_INTERFACE_GL_ES = `/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * THIS FILE IS AUTOGENERATED
 * Make edits to tools/gpu/gl/interface/templates.go or they will
 * be overwritten.
 */

#include "include/gpu/gl/GrGLAssembleHelpers.h"
#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "src/gpu/gl/GrGLUtil.h"

#define GET_PROC(F) functions->f##F = (GrGL##F##Fn*)get(ctx, "gl" #F)
#define GET_PROC_SUFFIX(F, S) functions->f##F = (GrGL##F##Fn*)get(ctx, "gl" #F #S)
#define GET_PROC_LOCAL(F) GrGL##F##Fn* F = (GrGL##F##Fn*)get(ctx, "gl" #F)

#define GET_EGL_PROC_SUFFIX(F, S) functions->fEGL##F = (GrEGL##F##Fn*)get(ctx, "egl" #F #S)

#if SK_DISABLE_GL_ES_INTERFACE
sk_sp<const GrGLInterface> GrGLMakeAssembledGLESInterface(void *ctx, GrGLGetProc get) {
    return nullptr;
}
#else
sk_sp<const GrGLInterface> GrGLMakeAssembledGLESInterface(void *ctx, GrGLGetProc get) {
    GET_PROC_LOCAL(GetString);
    if (nullptr == GetString) {
        return nullptr;
    }

    const char* verStr = reinterpret_cast<const char*>(GetString(GR_GL_VERSION));
    GrGLVersion glVer = GrGLGetVersionFromString(verStr);

    if (glVer < GR_GL_VER(2,0)) {
        return nullptr;
    }

    GET_PROC_LOCAL(GetIntegerv);
    GET_PROC_LOCAL(GetStringi);
    GrEGLQueryStringFn* queryString;
    GrEGLDisplay display;
    GrGetEGLQueryAndDisplay(&queryString, &display, ctx, get);
    GrGLExtensions extensions;
    if (!extensions.init(kGLES_GrGLStandard, GetString, GetStringi, GetIntegerv, queryString,
                         display)) {
        return nullptr;
    }

    sk_sp<GrGLInterface> interface(new GrGLInterface);
    GrGLInterface::Functions* functions = &interface->fFunctions;

    // Autogenerated content follows
[[content]]
    // End autogenerated content
    // TODO(kjlubick): Do we want a feature that removes the extension if it doesn't have
    // the function? This is common on some low-end GPUs.

    if (extensions.has("GL_KHR_debug")) {
        // In general we have a policy against removing extension strings when the driver does
        // not provide function pointers for an advertised extension. However, because there is a
        // known device that advertises GL_KHR_debug but fails to provide the functions and this is
        // a debugging- only extension we've made an exception. This also can happen when using
        // APITRACE.
        if (!interface->fFunctions.fDebugMessageControl) {
            extensions.remove("GL_KHR_debug");
        }
    }
    interface->fStandard = kGLES_GrGLStandard;
    interface->fExtensions.swap(&extensions);

    return std::move(interface);
}
#endif
`

const ASSEMBLE_INTERFACE_GL = `/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * THIS FILE IS AUTOGENERATED
 * Make edits to tools/gpu/gl/interface/templates.go or they will
 * be overwritten.
 */

#include "include/gpu/gl/GrGLAssembleHelpers.h"
#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "src/gpu/gl/GrGLUtil.h"

#define GET_PROC(F) functions->f##F = (GrGL##F##Fn*)get(ctx, "gl" #F)
#define GET_PROC_SUFFIX(F, S) functions->f##F = (GrGL##F##Fn*)get(ctx, "gl" #F #S)
#define GET_PROC_LOCAL(F) GrGL##F##Fn* F = (GrGL##F##Fn*)get(ctx, "gl" #F)

#define GET_EGL_PROC_SUFFIX(F, S) functions->fEGL##F = (GrEGL##F##Fn*)get(ctx, "egl" #F #S)

#if SK_DISABLE_GL_INTERFACE
sk_sp<const GrGLInterface> GrGLMakeAssembledGLInterface(void *ctx, GrGLGetProc get) {
    return nullptr;
}
#else
sk_sp<const GrGLInterface> GrGLMakeAssembledGLInterface(void *ctx, GrGLGetProc get) {
    GET_PROC_LOCAL(GetString);
    GET_PROC_LOCAL(GetStringi);
    GET_PROC_LOCAL(GetIntegerv);

    // GetStringi may be nullptr depending on the GL version.
    if (nullptr == GetString || nullptr == GetIntegerv) {
        return nullptr;
    }

    const char* versionString = (const char*) GetString(GR_GL_VERSION);
    GrGLVersion glVer = GrGLGetVersionFromString(versionString);

    if (glVer < GR_GL_VER(2,0) || GR_GL_INVALID_VER == glVer) {
        // This is our minimum for non-ES GL.
        return nullptr;
    }

    GrEGLQueryStringFn* queryString;
    GrEGLDisplay display;
    GrGetEGLQueryAndDisplay(&queryString, &display, ctx, get);
    GrGLExtensions extensions;
    if (!extensions.init(kGL_GrGLStandard, GetString, GetStringi, GetIntegerv, queryString,
                         display)) {
        return nullptr;
    }

    sk_sp<GrGLInterface> interface(new GrGLInterface());
    GrGLInterface::Functions* functions = &interface->fFunctions;

    // Autogenerated content follows
[[content]]
    // End autogenerated content
    interface->fStandard = kGL_GrGLStandard;
    interface->fExtensions.swap(&extensions);

    return std::move(interface);
}
#endif
`

const ASSEMBLE_INTERFACE_WEBGL = `/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * THIS FILE IS AUTOGENERATED
 * Make edits to tools/gpu/gl/interface/templates.go or they will
 * be overwritten.
 */

#include "include/gpu/gl/GrGLAssembleHelpers.h"
#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "src/gpu/gl/GrGLUtil.h"

#define GET_PROC(F) functions->f##F = (GrGL##F##Fn*)get(ctx, "gl" #F)
#define GET_PROC_SUFFIX(F, S) functions->f##F = (GrGL##F##Fn*)get(ctx, "gl" #F #S)
#define GET_PROC_LOCAL(F) GrGL##F##Fn* F = (GrGL##F##Fn*)get(ctx, "gl" #F)

#define GET_EGL_PROC_SUFFIX(F, S) functions->fEGL##F = (GrEGL##F##Fn*)get(ctx, "egl" #F #S)

#if SK_DISABLE_WEBGL_INTERFACE
sk_sp<const GrGLInterface> GrGLMakeAssembledWebGLInterface(void *ctx, GrGLGetProc get) {
    return nullptr;
}
#else
sk_sp<const GrGLInterface> GrGLMakeAssembledWebGLInterface(void *ctx, GrGLGetProc get) {
    GET_PROC_LOCAL(GetString);
    if (nullptr == GetString) {
        return nullptr;
    }

    const char* verStr = reinterpret_cast<const char*>(GetString(GR_GL_VERSION));
    GrGLVersion glVer = GrGLGetVersionFromString(verStr);

    if (glVer < GR_GL_VER(1,0)) {
        return nullptr;
    }

    GET_PROC_LOCAL(GetIntegerv);
    GET_PROC_LOCAL(GetStringi);
    GrEGLQueryStringFn* queryString;
    GrEGLDisplay display;
    GrGetEGLQueryAndDisplay(&queryString, &display, ctx, get);
    GrGLExtensions extensions;
    if (!extensions.init(kWebGL_GrGLStandard, GetString, GetStringi, GetIntegerv, queryString,
                         display)) {
        return nullptr;
    }

    sk_sp<GrGLInterface> interface(new GrGLInterface);
    GrGLInterface::Functions* functions = &interface->fFunctions;

    // Autogenerated content follows
[[content]]
    // End autogenerated content

    interface->fStandard = kWebGL_GrGLStandard;
    interface->fExtensions.swap(&extensions);

    return std::move(interface);
}
#endif
`

const VALIDATE_INTERFACE = `/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * THIS FILE IS AUTOGENERATED
 * Make edits to tools/gpu/gl/interface/templates.go or they will
 * be overwritten.
 */

#include "include/gpu/gl/GrGLExtensions.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/gl/GrGLUtil.h"

#include <stdio.h>

GrGLInterface::GrGLInterface() {
    fStandard = kNone_GrGLStandard;
}

#define RETURN_FALSE_INTERFACE                                                 \
    SkDEBUGF("%s:%d GrGLInterface::validate() failed.\n", __FILE__, __LINE__); \
    return false

bool GrGLInterface::validate() const {

    if (kNone_GrGLStandard == fStandard) {
        RETURN_FALSE_INTERFACE;
    }

    if (!fExtensions.isInitialized()) {
        RETURN_FALSE_INTERFACE;
    }

    GrGLVersion glVer = GrGLGetVersion(this);
    if (GR_GL_INVALID_VER == glVer) {
        RETURN_FALSE_INTERFACE;
    }
    // Autogenerated content follows
[[content]]
    // End autogenerated content
    return true;
}

#if GR_TEST_UTILS

void GrGLInterface::abandon() const {
    const_cast<GrGLInterface*>(this)->fFunctions = GrGLInterface::Functions();
}

#endif // GR_TEST_UTILS
`
