/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU
#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrGLInterface.h"
#include "GrGLTypes.h"
#endif

#include "SkCanvas.h"
#include "SkDashPathEffect.h"
#include "SkCornerPathEffect.h"
#include "SkDiscretePathEffect.h"
#include "SkFontMgr.h"
#include "SkFontMgrPriv.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkScalar.h"
#include "SkSurface.h"
#include "SkSurfaceProps.h"
#include "SkTestFontMgr.h"
#if SK_INCLUDE_SKOTTIE
#include "Skottie.h"
#endif
#if SK_INCLUDE_NIMA
#include "nima/NimaActor.h"
#endif

#include <iostream>
#include <string>

#include <emscripten.h>
#include <emscripten/bind.h>
#if SK_SUPPORT_GPU
#include <GL/gl.h>
#include <emscripten/html5.h>
#endif

using namespace emscripten;

// Self-documenting types
using JSArray = emscripten::val;
using JSColor = int32_t;

void EMSCRIPTEN_KEEPALIVE initFonts() {
    gSkFontMgr_DefaultFactory = &sk_tool_utils::MakePortableFontMgr;
}

#if SK_SUPPORT_GPU
// Wraps the WebGL context in an SkSurface and returns it.
// This function based on the work of
// https://github.com/Zubnix/skia-wasm-port/, used under the terms of the MIT license.
sk_sp<SkSurface> getWebGLSurface(std::string id, int width, int height) {
    // Context configurations
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = true;
    attrs.premultipliedAlpha = true;
    attrs.majorVersion = 1;
    attrs.enableExtensionsByDefault = true;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(id.c_str(), &attrs);
    if (context < 0) {
        printf("failed to create webgl context %d\n", context);
        return nullptr;
    }
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
    if (r < 0) {
        printf("failed to make webgl current %d\n", r);
        return nullptr;
    }

    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // setup GrContext
    auto interface = GrGLMakeNativeInterface();

    // setup contexts
    sk_sp<GrContext> grContext(GrContext::MakeGL(interface));

    // Wrap the frame buffer object attached to the screen in a Skia render target so Skia can
    // render to it
    GrGLint buffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);
    GrGLFramebufferInfo info;
    info.fFBOID = (GrGLuint) buffer;
    SkColorType colorType;

    info.fFormat = GL_RGBA8;
    colorType = kRGBA_8888_SkColorType;

    GrBackendRenderTarget target(width, height, 0, 8, info);

    sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(grContext.get(), target,
                                                                    kBottomLeft_GrSurfaceOrigin,
                                                                    colorType, nullptr, nullptr));
    return surface;
}
#endif

#if SK_INCLUDE_SKOTTIE
sk_sp<skottie::Animation> MakeAnimation(std::string json) {
    return skottie::Animation::Make(json.c_str(), json.length());
}
#endif

//========================================================================================
// Path things
//========================================================================================

// All these Apply* methods are simple wrappers to avoid returning an object.
// The default WASM bindings produce code that will leak if a return value
// isn't assigned to a JS variable and has delete() called on it.
// These Apply methods, combined with the smarter binding code allow for chainable
// commands that don't leak if the return value is ignored (i.e. when used intuitively).

void ApplyAddPath(SkPath& orig, const SkPath& newPath,
                   SkScalar scaleX, SkScalar skewX,  SkScalar transX,
                   SkScalar skewY,  SkScalar scaleY, SkScalar transY,
                   SkScalar pers0, SkScalar pers1, SkScalar pers2) {
    SkMatrix m = SkMatrix::MakeAll(scaleX, skewX , transX,
                                   skewY , scaleY, transY,
                                   pers0 , pers1 , pers2);
    orig.addPath(newPath, m);
}

void ApplyArcTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                SkScalar radius) {
    p.arcTo(x1, y1, x2, y2, radius);
}

void ApplyClose(SkPath& p) {
    p.close();
}

void ApplyConicTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                  SkScalar w) {
    p.conicTo(x1, y1, x2, y2, w);
}

void ApplyCubicTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                  SkScalar x3, SkScalar y3) {
    p.cubicTo(x1, y1, x2, y2, x3, y3);
}

void ApplyLineTo(SkPath& p, SkScalar x, SkScalar y) {
    p.lineTo(x, y);
}

void ApplyMoveTo(SkPath& p, SkScalar x, SkScalar y) {
    p.moveTo(x, y);
}

void ApplyQuadTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
    p.quadTo(x1, y1, x2, y2);
}

void ApplyTransform(SkPath& orig,
                    SkScalar scaleX, SkScalar skewX,  SkScalar transX,
                    SkScalar skewY,  SkScalar scaleY, SkScalar transY,
                    SkScalar pers0, SkScalar pers1, SkScalar pers2) {
    SkMatrix m = SkMatrix::MakeAll(scaleX, skewX , transX,
                                   skewY , scaleY, transY,
                                   pers0 , pers1 , pers2);
    orig.transform(m);
}

SkPath EMSCRIPTEN_KEEPALIVE CopyPath(const SkPath& a) {
    SkPath copy(a);
    return copy;
}

bool EMSCRIPTEN_KEEPALIVE Equals(const SkPath& a, const SkPath& b) {
    return a == b;
}

// to map from raw memory to a uint8array
val getSkDataBytes(const SkData *data) {
    return val(typed_memory_view(data->size(), data->bytes()));
}

// Hack to avoid embind creating a binding for SkData destructor
namespace emscripten {
    namespace internal {
        template<typename ClassType>
        void raw_destructor(ClassType *);

        template<>
        void raw_destructor<SkData>(SkData *ptr) {
        }
    }
}

// Some timesignatures below have uintptr_t instead of a pointer to a primative
// type (e.g. SkScalar). This is necessary because we can't use "bind" (EMSCRIPTEN_BINDINGS)
// and pointers to primitive types (Only bound types like SkPoint). We could if we used
// cwrap (see https://becominghuman.ai/passing-and-returning-webassembly-array-parameters-a0f572c65d97)
// but that requires us to stick to C code and, AFAIK, doesn't allow us to return nice things like
// SkPath or SkCanvas.
//
// So, basically, if we are using C++ and EMSCRIPTEN_BINDINGS, we can't have primative pointers
// in our function type signatures. (this gives an error message like "Cannot call foo due to unbound
// types Pi, Pf").  But, we can just pretend they are numbers and cast them to be pointers and
// the compiler is happy.
EMSCRIPTEN_BINDINGS(Skia) {
    function("initFonts", &initFonts);
#if SK_SUPPORT_GPU
    function("_getWebGLSurface", &getWebGLSurface, allow_raw_pointers());
    function("currentContext", &emscripten_webgl_get_current_context);
    function("setCurrentContext", &emscripten_webgl_make_context_current);
    constant("gpu", true);
#else
    function("_getRasterN32PremulSurface", optional_override([](int width, int height)->sk_sp<SkSurface> {
        return SkSurface::MakeRasterN32Premul(width, height, nullptr);
    }), allow_raw_pointers());
#endif
    function("MakeSkCornerPathEffect", &SkCornerPathEffect::Make, allow_raw_pointers());
    function("MakeSkDiscretePathEffect", &SkDiscretePathEffect::Make, allow_raw_pointers());
    // Won't be called directly, there's a JS helper to deal with typed arrays.
    function("_MakeSkDashPathEffect", optional_override([](uintptr_t /* float* */ cptr, int count, SkScalar phase)->sk_sp<SkPathEffect> {
        // See comment above for uintptr_t explanation
        const float* intervals = reinterpret_cast<const float*>(cptr);
        return SkDashPathEffect::Make(intervals, count, phase);
    }), allow_raw_pointers());
    function("getSkDataBytes", &getSkDataBytes, allow_raw_pointers());

    class_<SkCanvas>("SkCanvas")
        .constructor<>()
        .function("clear", optional_override([](SkCanvas& self, JSColor color)->void {
            // JS side gives us a signed int instead of an unsigned int for color
            // Add a lambda to change it out.
            self.clear(SkColor(color));
        }))
        .function("drawPaint", &SkCanvas::drawPaint)
        .function("drawPath", &SkCanvas::drawPath)
        .function("drawRect", &SkCanvas::drawRect)
        .function("drawText", optional_override([](SkCanvas& self, std::string text, SkScalar x, SkScalar y, const SkPaint& p) {
            // TODO(kjlubick): This does not work well for non-ascii
            // Need to maybe add a helper in interface.js that supports UTF-8
            // Otherwise, go with std::wstring and set UTF-32 encoding.
            self.drawText(text.c_str(), text.length(), x, y, p);
        }))
        .function("flush", &SkCanvas::flush)
        .function("rotate", select_overload<void (SkScalar degrees, SkScalar px, SkScalar py)>(&SkCanvas::rotate))
        .function("save", &SkCanvas::save)
        .function("scale", &SkCanvas::scale)
        .function("setMatrix", &SkCanvas::setMatrix)
        .function("skew", &SkCanvas::skew)
        .function("translate", &SkCanvas::translate);

    class_<SkData>("SkData")
        .smart_ptr<sk_sp<SkData>>("sk_sp<SkData>>")
        .function("size", &SkData::size);

    class_<SkImage>("SkImage")
        .smart_ptr<sk_sp<SkImage>>("sk_sp<SkImage>")
        .function("encodeToData", select_overload<sk_sp<SkData>()const>(&SkImage::encodeToData));

    class_<SkPaint>("SkPaint")
        .constructor<>()
        .function("copy", optional_override([](const SkPaint& self)->SkPaint {
            SkPaint p(self);
            return p;
        }))
        .function("measureText", optional_override([](SkPaint& self, std::string text) {
            // TODO(kjlubick): This does not work well for non-ascii
            // Need to maybe add a helper in interface.js that supports UTF-8
            // Otherwise, go with std::wstring and set UTF-32 encoding.
            return self.measureText(text.c_str(), text.length());
        }))
        .function("setAntiAlias", &SkPaint::setAntiAlias)
        .function("setColor", optional_override([](SkPaint& self, JSColor color)->void {
            // JS side gives us a signed int instead of an unsigned int for color
            // Add a lambda to change it out.
            self.setColor(SkColor(color));
        }))
        .function("setPathEffect", &SkPaint::setPathEffect)
        .function("setShader", &SkPaint::setShader)
        .function("setStrokeWidth", &SkPaint::setStrokeWidth)
        .function("setStyle", &SkPaint::setStyle)
        .function("setTextSize", &SkPaint::setTextSize);

    class_<SkPathEffect>("SkPathEffect")
        .smart_ptr<sk_sp<SkPathEffect>>("sk_sp<SkPathEffect>");

    class_<SkPath>("SkPath")
        .constructor<>()
        .constructor<const SkPath&>()
        // interface.js has 3 overloads of addPath
        .function("_addPath", &ApplyAddPath)
        .function("_arcTo", &ApplyArcTo)
        .function("_close", &ApplyClose)
        .function("_conicTo", &ApplyConicTo)
        .function("_cubicTo", &ApplyCubicTo)
        .function("_lineTo", &ApplyLineTo)
        .function("_moveTo", &ApplyMoveTo)
        .function("_quadTo", &ApplyQuadTo)
        .function("_transform", select_overload<void(SkPath& orig, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar)>(&ApplyTransform))

        .function("setFillType", &SkPath::setFillType)
        .function("getFillType", &SkPath::getFillType)
        .function("getBounds", &SkPath::getBounds)
        .function("computeTightBounds", &SkPath::computeTightBounds)
        .function("equals", &Equals)
        .function("copy", &CopyPath);

    class_<SkSurface>("SkSurface")
        .smart_ptr<sk_sp<SkSurface>>("sk_sp<SkSurface>")
        .function("width", &SkSurface::width)
        .function("height", &SkSurface::height)
        .function("_flush", &SkSurface::flush)
        .function("makeImageSnapshot", &SkSurface::makeImageSnapshot)
        .function("_readPixels", optional_override([](SkSurface& self, int width, int height, uintptr_t /* uint8_t* */ cptr)->bool {
            auto* dst = reinterpret_cast<uint8_t*>(cptr);
            auto dstInfo = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
            return self.readPixels(dstInfo, dst, width*4, 0, 0);
        }))
        .function("getCanvas", &SkSurface::getCanvas, allow_raw_pointers());


    enum_<SkPaint::Style>("PaintStyle")
        .value("FILL",              SkPaint::Style::kFill_Style)
        .value("STROKE",            SkPaint::Style::kStroke_Style)
        .value("STROKE_AND_FILL",   SkPaint::Style::kStrokeAndFill_Style);

    enum_<SkPath::FillType>("FillType")
        .value("WINDING",            SkPath::FillType::kWinding_FillType)
        .value("EVENODD",            SkPath::FillType::kEvenOdd_FillType)
        .value("INVERSE_WINDING",    SkPath::FillType::kInverseWinding_FillType)
        .value("INVERSE_EVENODD",    SkPath::FillType::kInverseEvenOdd_FillType);

    // A value object is much simpler than a class - it is returned as a JS
    // object and does not require delete().
    // https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/embind.html#value-types
    value_object<SkRect>("SkRect")
        .field("fLeft",   &SkRect::fLeft)
        .field("fTop",    &SkRect::fTop)
        .field("fRight",  &SkRect::fRight)
        .field("fBottom", &SkRect::fBottom);

    // SkPoints can be represented by [x, y]
    value_array<SkPoint>("SkPoint")
        .element(&SkPoint::fX)
        .element(&SkPoint::fY);

    // {"w": Number, "h", Number}
    value_object<SkSize>("SkSize")
        .field("w",   &SkSize::fWidth)
        .field("h",   &SkSize::fHeight);

    value_object<SkISize>("SkISize")
        .field("w",   &SkISize::fWidth)
        .field("h",   &SkISize::fHeight);

#if SK_INCLUDE_SKOTTIE
    // Animation things (may eventually go in own library)
    class_<skottie::Animation>("Animation")
        .smart_ptr<sk_sp<skottie::Animation>>("sk_sp<Animation>")
        .function("version", optional_override([](skottie::Animation& self)->std::string {
            return std::string(self.version().c_str());
        }))
        .function("size", &skottie::Animation::size)
        .function("duration", &skottie::Animation::duration)
        .function("seek", &skottie::Animation::seek)
        .function("render", optional_override([](skottie::Animation& self, SkCanvas* canvas)->void {
            self.render(canvas, nullptr);
        }), allow_raw_pointers())
        .function("render", optional_override([](skottie::Animation& self, SkCanvas* canvas, const SkRect r)->void {
            self.render(canvas, &r);
        }), allow_raw_pointers());

    function("MakeAnimation", &MakeAnimation);
    constant("skottie", true);
#endif

#if SK_INCLUDE_NIMA
    class_<NimaActor>("NimaActor")
        .function("duration", &NimaActor::duration)
        .function("getAnimationNames",  optional_override([](NimaActor& self)->JSArray {
            JSArray names = emscripten::val::array();
            auto vNames = self.getAnimationNames();
            for (size_t i = 0; i < vNames.size(); i++) {
                names.call<void>("push", vNames[i]);
            }
            return names;
        }), allow_raw_pointers())
        .function("render",  optional_override([](NimaActor& self, SkCanvas* canvas)->void {
            self.render(canvas, 0);
        }), allow_raw_pointers())
        .function("seek", &NimaActor::seek)
        .function("setAnimationByIndex", select_overload<void(uint8_t    )>(&NimaActor::setAnimation))
        .function("setAnimationByName" , select_overload<void(std::string)>(&NimaActor::setAnimation));

    function("_MakeNimaActor", optional_override([](uintptr_t /* uint8_t* */ nptr, int nlen,
                                                    uintptr_t /* uint8_t* */ tptr, int tlen)->NimaActor* {
        // See comment above for uintptr_t explanation
        const uint8_t* nimaBytes = reinterpret_cast<const uint8_t*>(nptr);
        const uint8_t* textureBytes = reinterpret_cast<const uint8_t*>(tptr);

        auto nima = SkData::MakeWithoutCopy(nimaBytes, nlen);
        auto texture = SkData::MakeWithoutCopy(textureBytes, tlen);
        return new NimaActor(nima, texture);
    }), allow_raw_pointers());
    constant("nima", true);
#endif
}
