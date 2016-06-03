/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Viewer.h"

#include "GMSlide.h"
#include "SKPSlide.h"

#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkOSFile.h"
#include "SkRandom.h"
#include "SkStream.h"

using namespace sk_app;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new Viewer(argc, argv, platformData);
}

static void on_paint_handler(SkCanvas* canvas, void* userData) {
    Viewer* vv = reinterpret_cast<Viewer*>(userData);

    return vv->onPaint(canvas);
}

static bool on_touch_handler(int owner, Window::InputState state, float x, float y, void* userData)
{
    Viewer* viewer = reinterpret_cast<Viewer*>(userData);

    return viewer->onTouch(owner, state, x, y);
}

static void on_ui_state_changed_handler(const SkString& stateName, const SkString& stateValue, void* userData) {
    Viewer* viewer = reinterpret_cast<Viewer*>(userData);

    return viewer->onUIStateChanged(stateName, stateValue);
}

DEFINE_bool2(fullscreen, f, true, "Run fullscreen.");
DEFINE_string(key, "", "Space-separated key/value pairs to add to JSON identifying this builder.");
DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of bench name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching bench to always be skipped\n"
               "^ requires the start of the bench to match\n"
               "$ requires the end of the bench to match\n"
               "^ and $ requires an exact match\n"
               "If a bench does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");
DEFINE_string(skps, "skps", "Directory to read skps from.");
DEFINE_bool(vulkan, true, "Run with Vulkan.");

const char *kBackendTypeStrings[sk_app::Window::kBackendTypeCount] = {
    " [OpenGL]",
    " [Vulkan]"
};

const char* kName = "name";
const char* kValue = "value";
const char* kOptions = "options";
const char* kSlideStateName = "Slide";
const char* kBackendStateName = "Backend";
const char* kSoftkeyStateName = "Softkey";
const char* kSoftkeyHint = "Please select a softkey";


Viewer::Viewer(int argc, char** argv, void* platformData)
    : fCurrentMeasurement(0)
    , fDisplayStats(false)
    , fBackendType(sk_app::Window::kVulkan_BackendType)
    , fZoomCenterX(0.0f)
    , fZoomCenterY(0.0f)
    , fZoomLevel(0.0f)
    , fZoomScale(SK_Scalar1)
{
    memset(fMeasurements, 0, sizeof(fMeasurements));

    SkDebugf("Command line arguments: ");
    for (int i = 1; i < argc; ++i) {
        SkDebugf("%s ", argv[i]);
    }
    SkDebugf("\n");

    SkCommandLineFlags::Parse(argc, argv);

    fBackendType = FLAGS_vulkan ? sk_app::Window::kVulkan_BackendType
                                : sk_app::Window::kNativeGL_BackendType;

    fWindow = Window::CreateNativeWindow(platformData);
    fWindow->attach(fBackendType, DisplayParams());

    // register callbacks
    fCommands.attach(fWindow);
    fWindow->registerPaintFunc(on_paint_handler, this);
    fWindow->registerTouchFunc(on_touch_handler, this);
    fWindow->registerUIStateChangedFunc(on_ui_state_changed_handler, this);

    // add key-bindings
    fCommands.addCommand('s', "Overlays", "Toggle stats display", [this]() {
        this->fDisplayStats = !this->fDisplayStats;
        fWindow->inval();
    });
    fCommands.addCommand('c', "Modes", "Toggle sRGB color mode", [this]() {
        DisplayParams params = fWindow->getDisplayParams();
        params.fProfileType = (kLinear_SkColorProfileType == params.fProfileType)
            ? kSRGB_SkColorProfileType : kLinear_SkColorProfileType;
        fWindow->setDisplayParams(params);
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand(Window::Key::kRight, "Right", "Navigation", "Next slide", [this]() {
        int previousSlide = fCurrentSlide;
        fCurrentSlide++;
        if (fCurrentSlide >= fSlides.count()) {
            fCurrentSlide = 0;
        }
        this->setupCurrentSlide(previousSlide);
    });
    fCommands.addCommand(Window::Key::kLeft, "Left", "Navigation", "Previous slide", [this]() {
        int previousSlide = fCurrentSlide;
        fCurrentSlide--;
        if (fCurrentSlide < 0) {
            fCurrentSlide = fSlides.count() - 1;
        }
        this->setupCurrentSlide(previousSlide);
    });
    fCommands.addCommand(Window::Key::kUp, "Up", "Transform", "Zoom in", [this]() {
        this->changeZoomLevel(1.f / 32.f);
        fWindow->inval();
    });
    fCommands.addCommand(Window::Key::kDown, "Down", "Transform", "Zoom out", [this]() {
        this->changeZoomLevel(-1.f / 32.f);
        fWindow->inval();
    });
#if 0  // this doesn't seem to work on any platform right now
#ifndef SK_BUILD_FOR_ANDROID
    fCommands.addCommand('d', "Modes", "Change rendering backend", [this]() {
        fWindow->detach();

        if (sk_app::Window::kVulkan_BackendType == fBackendType) {
            fBackendType = sk_app::Window::kNativeGL_BackendType;
        } 
        // TODO: get Vulkan -> OpenGL working on Windows without swapchain creation failure
        //else if (sk_app::Window::kNativeGL_BackendType == fBackendType) {
        //    fBackendType = sk_app::Window::kVulkan_BackendType;
        //}

        fWindow->attach(fBackendType, DisplayParams());
        this->updateTitle();
        fWindow->inval();
    });
#endif
#endif

    // set up slides
    this->initSlides();

    fAnimTimer.run();

    // set up first frame
    fCurrentSlide = 0;
    setupCurrentSlide(-1);

    fWindow->show();
}

void Viewer::initSlides() {
    const skiagm::GMRegistry* gms(skiagm::GMRegistry::Head());
    while (gms) {
        SkAutoTDelete<skiagm::GM> gm(gms->factory()(nullptr));

        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, gm->getName())) {
            sk_sp<Slide> slide(new GMSlide(gm.release()));
            fSlides.push_back(slide);
        }

        gms = gms->next();
    }

    // reverse array
    for (int i = 0; i < fSlides.count()/2; ++i) {
        sk_sp<Slide> temp = fSlides[i];
        fSlides[i] = fSlides[fSlides.count() - i - 1];
        fSlides[fSlides.count() - i - 1] = temp;
    }

    // SKPs
    for (int i = 0; i < FLAGS_skps.count(); i++) {
        if (SkStrEndsWith(FLAGS_skps[i], ".skp")) {
            if (SkCommandLineFlags::ShouldSkip(FLAGS_match, FLAGS_skps[i])) {
                continue;
            }

            SkString path(FLAGS_skps[i]);
            sk_sp<SKPSlide> slide(new SKPSlide(SkOSPath::Basename(path.c_str()), path));
            if (slide) {
                fSlides.push_back(slide);
            }
        } else {
            SkOSFile::Iter it(FLAGS_skps[i], ".skp");
            SkString skpName;
            while (it.next(&skpName)) {
                if (SkCommandLineFlags::ShouldSkip(FLAGS_match, skpName.c_str())) {
                    continue;
                }

                SkString path = SkOSPath::Join(FLAGS_skps[i], skpName.c_str());
                sk_sp<SKPSlide> slide(new SKPSlide(skpName, path));
                if (slide) {
                    fSlides.push_back(slide);
                }
            }
        }
    }
}


Viewer::~Viewer() {
    fWindow->detach();
    delete fWindow;
}

void Viewer::updateTitle() {
    SkString title("Viewer: ");
    title.append(fSlides[fCurrentSlide]->getName());
    if (kSRGB_SkColorProfileType == fWindow->getDisplayParams().fProfileType) {
        title.append(" sRGB");
    }
    title.append(kBackendTypeStrings[fBackendType]);
    fWindow->setTitle(title.c_str());
}

void Viewer::setupCurrentSlide(int previousSlide) {
    if (fCurrentSlide == previousSlide) {
        return; // no change; do nothing
    }

    fGesture.reset();
    fDefaultMatrix.reset();
    fDefaultMatrixInv.reset();

    if (fWindow->supportsContentRect() && fWindow->scaleContentToFit()) {
        const SkRect contentRect = fWindow->getContentRect();
        const SkISize slideSize = fSlides[fCurrentSlide]->getDimensions();
        const SkRect slideBounds = SkRect::MakeIWH(slideSize.width(), slideSize.height());
        if (contentRect.width() > 0 && contentRect.height() > 0) {
            fDefaultMatrix.setRectToRect(slideBounds, contentRect, SkMatrix::kStart_ScaleToFit);
            SkAssertResult(fDefaultMatrix.invert(&fDefaultMatrixInv));
        }
    }

    if (fWindow->supportsContentRect()) {
        const SkISize slideSize = fSlides[fCurrentSlide]->getDimensions();
        SkRect windowRect = fWindow->getContentRect();
        fDefaultMatrixInv.mapRect(&windowRect);
        fGesture.setTransLimit(SkRect::MakeWH(SkIntToScalar(slideSize.width()), 
                                              SkIntToScalar(slideSize.height())),
                               windowRect);
    }

    this->updateTitle();
    this->updateUIState();
    fSlides[fCurrentSlide]->load();
    if (previousSlide >= 0) {
        fSlides[previousSlide]->unload();
    }
    fWindow->inval();
}

#define MAX_ZOOM_LEVEL  8
#define MIN_ZOOM_LEVEL  -8

void Viewer::changeZoomLevel(float delta) {
    fZoomLevel += delta;
    if (fZoomLevel > 0) {
        fZoomLevel = SkMinScalar(fZoomLevel, MAX_ZOOM_LEVEL);
        fZoomScale = fZoomLevel + SK_Scalar1;
    } else if (fZoomLevel < 0) {
        fZoomLevel = SkMaxScalar(fZoomLevel, MIN_ZOOM_LEVEL);
        fZoomScale = SK_Scalar1 / (SK_Scalar1 - fZoomLevel);
    } else {
        fZoomScale = SK_Scalar1;
    }
}

SkMatrix Viewer::computeMatrix() {
    SkMatrix m;
    m.reset();

    if (fZoomLevel) {
        SkPoint center;
        //m = this->getLocalMatrix();//.invert(&m);
        m.mapXY(fZoomCenterX, fZoomCenterY, &center);
        SkScalar cx = center.fX;
        SkScalar cy = center.fY;

        m.setTranslate(-cx, -cy);
        m.postScale(fZoomScale, fZoomScale);
        m.postTranslate(cx, cy);
    }

    m.preConcat(fGesture.localM());
    m.preConcat(fGesture.globalM());

    return m;
}

void Viewer::onPaint(SkCanvas* canvas) {
    int count = canvas->save();

    if (fWindow->supportsContentRect()) {
        SkRect contentRect = fWindow->getContentRect();
        canvas->clipRect(contentRect);
        canvas->translate(contentRect.fLeft, contentRect.fTop);
    }

    canvas->clear(SK_ColorWHITE);
    canvas->concat(fDefaultMatrix);
    canvas->concat(computeMatrix());

    fSlides[fCurrentSlide]->draw(canvas);
    canvas->restoreToCount(count);

    if (fDisplayStats) {
        drawStats(canvas);
    }
    fCommands.drawHelp(canvas);
}

bool Viewer::onTouch(int owner, Window::InputState state, float x, float y) {
    void* castedOwner = reinterpret_cast<void*>(owner);
    SkPoint touchPoint = fDefaultMatrixInv.mapXY(x, y);
    switch (state) {
        case Window::kUp_InputState: {
            fGesture.touchEnd(castedOwner);
            break;
        }
        case Window::kDown_InputState: {
            fGesture.touchBegin(castedOwner, touchPoint.fX, touchPoint.fY);
            break;
        }
        case Window::kMove_InputState: {
            fGesture.touchMoved(castedOwner, touchPoint.fX, touchPoint.fY);
            break;
        }
    }
    fWindow->inval();
    return true;
}

void Viewer::drawStats(SkCanvas* canvas) {
    static const float kPixelPerMS = 2.0f;
    static const int kDisplayWidth = 130;
    static const int kDisplayHeight = 100;
    static const int kDisplayPadding = 10;
    static const int kGraphPadding = 3;
    static const SkScalar kBaseMS = 1000.f / 60.f;  // ms/frame to hit 60 fps

    SkISize canvasSize = canvas->getDeviceSize();
    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(canvasSize.fWidth-kDisplayWidth-kDisplayPadding),
                                   SkIntToScalar(kDisplayPadding),
                                   SkIntToScalar(kDisplayWidth), SkIntToScalar(kDisplayHeight));
    SkPaint paint;
    canvas->save();

    if (fWindow->supportsContentRect()) {
        SkRect contentRect = fWindow->getContentRect();
        canvas->clipRect(contentRect);
        canvas->translate(contentRect.fLeft, contentRect.fTop);
    }

    canvas->clipRect(rect);
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(rect, paint);
    // draw the 16ms line
    paint.setColor(SK_ColorLTGRAY);
    canvas->drawLine(rect.fLeft, rect.fBottom - kBaseMS*kPixelPerMS,
                     rect.fRight, rect.fBottom - kBaseMS*kPixelPerMS, paint);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(rect, paint);

    int x = SkScalarTruncToInt(rect.fLeft) + kGraphPadding;
    const int xStep = 2;
    const int startY = SkScalarTruncToInt(rect.fBottom);
    int i = fCurrentMeasurement;
    do {
        int endY = startY - (int)(fMeasurements[i] * kPixelPerMS + 0.5);  // round to nearest value
        canvas->drawLine(SkIntToScalar(x), SkIntToScalar(startY),
                         SkIntToScalar(x), SkIntToScalar(endY), paint);
        i++;
        i &= (kMeasurementCount - 1);  // fast mod
        x += xStep;
    } while (i != fCurrentMeasurement);

    canvas->restore();
}

void Viewer::onIdle(double ms) {
    // Record measurements
    fMeasurements[fCurrentMeasurement++] = ms;
    fCurrentMeasurement &= (kMeasurementCount - 1);  // fast mod
    SkASSERT(fCurrentMeasurement < kMeasurementCount);

    fAnimTimer.updateTime();
    if (fSlides[fCurrentSlide]->animate(fAnimTimer) || fDisplayStats) {
        fWindow->inval();
    }
}

void Viewer::updateUIState() {
    // Slide state
    Json::Value slideState(Json::objectValue);
    slideState[kName] = kSlideStateName;
    slideState[kValue] = fSlides[fCurrentSlide]->getName().c_str();
    Json::Value allSlideNames(Json::arrayValue);
    for(auto slide : fSlides) {
        allSlideNames.append(Json::Value(slide->getName().c_str()));
    }
    slideState[kOptions] = allSlideNames;

    // Backend state
    Json::Value backendState(Json::objectValue);
    backendState[kName] = kBackendStateName;
    backendState[kValue] = kBackendTypeStrings[fBackendType];
    backendState[kOptions] = Json::Value(Json::arrayValue);
    for (auto str : kBackendTypeStrings) {
        backendState[kOptions].append(Json::Value(str));
    }

    // Softkey state
    Json::Value softkeyState(Json::objectValue);
    softkeyState[kName] = kSoftkeyStateName;
    softkeyState[kValue] = kSoftkeyHint;
    softkeyState[kOptions] = Json::Value(Json::arrayValue);
    softkeyState[kOptions].append(kSoftkeyHint);
    for (const auto& softkey : fCommands.getCommandsAsSoftkeys()) {
        softkeyState[kOptions].append(Json::Value(softkey.c_str()));
    }

    Json::Value state(Json::arrayValue);
    state.append(slideState);
    state.append(backendState);
    state.append(softkeyState);

    fWindow->setUIState(state);
}

void Viewer::onUIStateChanged(const SkString& stateName, const SkString& stateValue) {
    // For those who will add more features to handle the state change in this function:
    // After the change, please call updateUIState no notify the frontend (e.g., Android app).
    // For example, after slide change, updateUIState is called inside setupCurrentSlide;
    // after backend change, updateUIState is called in this function.
    if (stateName.equals(kSlideStateName)) {
        int previousSlide = fCurrentSlide;
        fCurrentSlide = 0;
        for(auto slide : fSlides) {
            if (slide->getName().equals(stateValue)) {
                setupCurrentSlide(previousSlide);
                break;
            }
            fCurrentSlide++;
        }
        if (fCurrentSlide >= fSlides.count()) {
            fCurrentSlide = previousSlide;
            SkDebugf("Slide not found: %s", stateValue.c_str());
        }
    } else if (stateName.equals(kBackendStateName)) {
        for (int i = 0; i < sk_app::Window::kBackendTypeCount; i++) {
            if (stateValue.equals(kBackendTypeStrings[i])) {
                if (fBackendType != i) {
                    fBackendType = (sk_app::Window::BackendType)i;
                    fWindow->detach();
                    fWindow->attach(fBackendType, DisplayParams());
                    fWindow->inval();
                    updateTitle();
                    updateUIState();
                }
                break;
            }
        }
    } else if (stateName.equals(kSoftkeyStateName)) {
        if (!stateValue.equals(kSoftkeyHint)) {
            fCommands.onSoftkey(stateValue);
            updateUIState(); // This is still needed to reset the value to kSoftkeyHint
        }
    } else {
        SkDebugf("Unknown stateName: %s", stateName.c_str());
    }
}
