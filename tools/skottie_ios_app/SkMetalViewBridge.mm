// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkMetalViewBridge.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/mtl/GrMtlTypes.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

sk_sp<SkSurface> SkMtkViewToSurface(MTKView* mtkView, GrContext* grContext) {
    if (!grContext ||
        MTLPixelFormatDepth32Float_Stencil8 != [mtkView depthStencilPixelFormat] ||
        MTLPixelFormatBGRA8Unorm != [mtkView colorPixelFormat]) {
        return nullptr;
    }

    const SkColorType colorType = kBGRA_8888_SkColorType;  // MTLPixelFormatBGRA8Unorm
    sk_sp<SkColorSpace> colorSpace = nullptr;  // MTLPixelFormatBGRA8Unorm
    const GrSurfaceOrigin origin = kTopLeft_GrSurfaceOrigin;
    const SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);
    int sampleCount = (int)[mtkView sampleCount];

    return SkSurface::MakeFromMTKView(grContext, (__bridge GrMTLHandle)mtkView, origin, sampleCount,
                                      colorType, colorSpace, &surfaceProps);
}

void GrContextRelease::operator()(GrContext* ptr) { SkSafeUnref(ptr); }

GrContextHolder SkMetalDeviceToGrContext(id<MTLDevice> device, id<MTLCommandQueue> queue) {
    GrContextOptions grContextOptions;  // set different options here.
    return GrContextHolder(GrContext::MakeMetal((__bridge void*)device,
                                                (__bridge void*)queue,
                                                grContextOptions).release());
}

void SkMtkViewConfigForSkia(MTKView* mtkView) {
    [mtkView setDepthStencilPixelFormat:MTLPixelFormatDepth32Float_Stencil8];
    [mtkView setColorPixelFormat:MTLPixelFormatBGRA8Unorm];
    [mtkView setSampleCount:1];
}
