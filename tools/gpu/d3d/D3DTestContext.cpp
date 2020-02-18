/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/d3d/D3DTestContext.h"

#ifdef SK_DIRECT3D

#include "include/gpu/GrContext.h"

namespace {

// TODO: Implement D3DFenceSync

class D3DTestContextImpl : public sk_gpu_test::D3DTestContext {
public:
    static D3DTestContext* Create(D3DTestContext* sharedContext) {
        GrD3DBackendContext backendContext;
        bool ownsContext;
        if (sharedContext) {
            // take from the given context
            ownsContext = false;
        } else {
            // create our own
            ownsContext = true;
        }
        return new D3DTestContextImpl(backendContext, ownsContext);
    }

    ~D3DTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    // There is really nothing to here since we don't own any unqueued command buffers here.
    void submit() override {}

    void finish() override {}

    sk_sp<GrContext> makeGrContext(const GrContextOptions& options) override {
        return GrContext::MakeDirect3D(fD3D, options);
    }

protected:
    void teardown() override {
        INHERITED::teardown();
        if (fOwnsContext) {
            // delete all the D3D objects in the backend context
        }
    }

private:
    D3DTestContextImpl(const GrD3DBackendContext& backendContext, bool ownsContext)
            : D3DTestContext(backendContext, ownsContext) {
// TODO       fFenceSync.reset(new D3DFenceSync(backendContext));
    }

    void onPlatformMakeNotCurrent() const override {}
    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override  { return nullptr; }
    void onPlatformSwapBuffers() const override {}

    typedef sk_gpu_test::D3DTestContext INHERITED;
};
}  // anonymous namespace

namespace sk_gpu_test {
D3DTestContext* CreatePlatformD3DTestContext(D3DTestContext* sharedContext) {
    return D3DTestContextImpl::Create(sharedContext);
}
}  // namespace sk_gpu_test

#endif
