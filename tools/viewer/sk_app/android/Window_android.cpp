/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Window_android.h"
#include "../GLWindowContext.h"
#include "../VulkanWindowContext.h"

namespace sk_app {

Window* Window::CreateNativeWindow(void* platformData) {
    Window_android* window = new Window_android();
    if (!window->init((SkiaAndroidApp*)platformData)) {
        delete window;
        return nullptr;
    }
    return window;
}

bool Window_android::init(SkiaAndroidApp* skiaAndroidApp) {
    SkASSERT(skiaAndroidApp);
    fSkiaAndroidApp = skiaAndroidApp;
    fSkiaAndroidApp->fWindow = this;
    return true;
}

const DisplayParams& Window_android::getDisplayParams() {
    if (fWindowContext) {
        return fWindowContext->getDisplayParams();
    } else {
        // fWindowContext doesn't exist because we haven't
        // initDisplay yet.
        return fDisplayParams;
    }
}

void Window_android::setTitle(const char* title) {
    fSkiaAndroidApp->setTitle(title);
}

void Window_android::setUIState(const Json::Value& state) {
    fSkiaAndroidApp->setUIState(state);
}

bool Window_android::attach(BackendType attachType, const DisplayParams& params) {
    fBackendType = attachType;
    fDisplayParams = params;

    if (fNativeWindow) {
        this->initDisplay(fNativeWindow);
    }
    // If fNativeWindow is not set,
    // we delay the creation of fWindowContext until Android informs us that
    // the native window is ready to use.
    // The creation will be done in initDisplay, which is initiated by kSurfaceCreated event.
    return true;
}

void Window_android::initDisplay(ANativeWindow* window) {
    SkASSERT(window);
    fNativeWindow = window;
    ContextPlatformData_android platformData;
    platformData.fNativeWindow = window;
    switch (fBackendType) {
        case kNativeGL_BackendType:
            fWindowContext = GLWindowContext::Create((void*)&platformData, fDisplayParams);
            break;

        case kVulkan_BackendType:
        default:
            fWindowContext = VulkanWindowContext::Create((void*)&platformData, fDisplayParams);
            break;
    }
}

void Window_android::onDisplayDestroyed() {
    detach();
    fNativeWindow = nullptr;
}

void Window_android::onInval() {
    fSkiaAndroidApp->postMessage(Message(kContentInvalidated));
}

void Window_android::paintIfNeeded() {
    if (fWindowContext) { // Check if initDisplay has already been called
        onPaint();
    } else {
        markInvalProcessed();
    }
}

}   // namespace sk_app
