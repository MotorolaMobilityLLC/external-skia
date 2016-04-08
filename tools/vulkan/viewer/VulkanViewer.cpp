/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "VulkanViewer.h"

#include "GMSlide.h"
#include "SKPSlide.h"

#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkOSFile.h"
#include "SkRandom.h"
#include "SkStream.h"

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new VulkanViewer(argc, argv, platformData);
}

static bool on_key_handler(Window::Key key, Window::InputState state, uint32_t modifiers,
                           void* userData) {
    VulkanViewer* vv = reinterpret_cast<VulkanViewer*>(userData);

    return vv->onKey(key, state, modifiers);
}

static bool on_char_handler(SkUnichar c, uint32_t modifiers, void* userData) {
    VulkanViewer* vv = reinterpret_cast<VulkanViewer*>(userData);

    return vv->onChar(c, modifiers);
}

static void on_paint_handler(SkCanvas* canvas, void* userData) {
    VulkanViewer* vv = reinterpret_cast<VulkanViewer*>(userData);

    return vv->onPaint(canvas);
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

VulkanViewer::VulkanViewer(int argc, char** argv, void* platformData)
    : fCurrentMeasurement(0)
    , fDisplayStats(false)
{
    memset(fMeasurements, 0, sizeof(fMeasurements));

    SkDebugf("Command line arguments: ");
    for (int i = 1; i < argc; ++i) {
        SkDebugf("%s ", argv[i]);
    }
    SkDebugf("\n");

    SkCommandLineFlags::Parse(argc, argv);

    fWindow = Window::CreateNativeWindow(platformData);
    fWindow->attach(Window::kVulkan_BackendType, 0, nullptr);

    // register callbacks
    fWindow->registerKeyFunc(on_key_handler, this);
    fWindow->registerCharFunc(on_char_handler, this);
    fWindow->registerPaintFunc(on_paint_handler, this);

    // set up slides
    this->initSlides();

    // set up first frame
    fCurrentSlide = 0;
    setupCurrentSlide(-1);
    fLocalMatrix.reset();

    fWindow->show();
}

void VulkanViewer::initSlides() {
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


VulkanViewer::~VulkanViewer() {
    fWindow->detach();
    delete fWindow;
}

void VulkanViewer::setupCurrentSlide(int previousSlide) {
    SkString title("VulkanViewer: ");
    title.append(fSlides[fCurrentSlide]->getName());
    fSlides[fCurrentSlide]->load();
    if (previousSlide >= 0) {
        fSlides[previousSlide]->unload();
    }
    fWindow->setTitle(title.c_str());
    fWindow->inval();
}

#define MAX_ZOOM_LEVEL  8
#define MIN_ZOOM_LEVEL  -8

void VulkanViewer::changeZoomLevel(float delta) {
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
    this->updateMatrix();
}

void VulkanViewer::updateMatrix(){
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

    // TODO: add gesture support
    // Apply any gesture matrix
    //m.preConcat(fGesture.localM());
    //m.preConcat(fGesture.globalM());

    fLocalMatrix = m;
}

bool VulkanViewer::onKey(Window::Key key, Window::InputState state, uint32_t modifiers) {
    if (Window::kDown_InputState == state) {
        switch (key) {
            case Window::kRight_Key: {
                int previousSlide = fCurrentSlide;
                fCurrentSlide++;
                if (fCurrentSlide >= fSlides.count()) {
                    fCurrentSlide = 0;
                }
                setupCurrentSlide(previousSlide);
                return true;
            }

            case Window::kLeft_Key: {
                int previousSlide = fCurrentSlide;
                fCurrentSlide--;
                if (fCurrentSlide < 0) {
                    fCurrentSlide = fSlides.count() - 1;
                }
                SkString title("VulkanViewer: ");
                title.append(fSlides[fCurrentSlide]->getName());
                fWindow->setTitle(title.c_str());
                setupCurrentSlide(previousSlide);
                return true;
            }

            case Window::kUp_Key: {
                this->changeZoomLevel(1.f / 32.f);
                return true;
            }

            case Window::kDown_Key: {
                this->changeZoomLevel(-1.f / 32.f);
                return true;
            }

            default:
                break;
        }
    }

    return false;
}

bool VulkanViewer::onChar(SkUnichar c, uint32_t modifiers) {
    if ('s' == c) {
        fDisplayStats = !fDisplayStats;
        return true;
    }

    return false;
}

void VulkanViewer::onPaint(SkCanvas* canvas) {

    canvas->clear(SK_ColorWHITE);

    int count = canvas->save();
    canvas->setMatrix(fLocalMatrix);

    fSlides[fCurrentSlide]->draw(canvas);
    canvas->restoreToCount(count);

    if (fDisplayStats) {
        drawStats(canvas);
    }
}

void VulkanViewer::drawStats(SkCanvas* canvas) {
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

void VulkanViewer::onIdle(double ms) {
    // Record measurements
    fMeasurements[fCurrentMeasurement++] = ms;
    fCurrentMeasurement &= (kMeasurementCount - 1);  // fast mod
    SkASSERT(fCurrentMeasurement < kMeasurementCount);

    fAnimTimer.updateTime();
    if (fDisplayStats || fSlides[fCurrentSlide]->animate(fAnimTimer)) {
        fWindow->inval();
    }
}
