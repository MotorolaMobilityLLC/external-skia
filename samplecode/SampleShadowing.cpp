/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkPictureRecorder.h"
#include "SkShadowPaintFilterCanvas.h"
#include "SkShadowShader.h"
#include "SkSurface.h"

#ifdef SK_EXPERIMENTAL_SHADOWING

class ShadowingView : public SampleView {
public:
    ShadowingView() {

        this->setBGColor(0xFFCCCCCC);
        SkLights::Builder builder;
        builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(0.2f, 0.3f, 0.4f),
                                                     SkVector3::Make(0.2f, 0.05f, 1.0f)));
        builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(0.4f, 0.3f, 0.2f),
                                                     SkVector3::Make(0.05f, 0.2f, 1.0f)));
        builder.add(SkLights::Light::MakeAmbient(SkColor3f::Make(0.4f, 0.4f, 0.4f)));
        fLights = builder.finish();

        fTestRects[0].fColor = 0xFFEE8888;
        fTestRects[0].fDepth = 80;
        fTestRects[0].fGeometry = SkRect::MakeLTRB(200,150,350,300);

        fTestRects[1].fColor = 0xFF88EE88;
        fTestRects[1].fDepth = 160;
        fTestRects[1].fGeometry = SkRect::MakeLTRB(150,200,300,350);

        fTestRects[2].fColor = 0xFF8888EE;
        fTestRects[2].fDepth = 240;
        fTestRects[2].fGeometry = SkRect::MakeLTRB(100,100,250,250);

        fSliders[0].fGeometry = SkRect::MakeLTRB(20, 400, 30, 420);
        fSliders[0].fOffset = 0.0f;
        fSliders[0].fScale = 0.1f;

        fSliders[1].fGeometry = SkRect::MakeLTRB(100, 420, 110, 440);
        fSliders[1].fOffset = 0.0f;
        fSliders[1].fScale = 10.0f;

        fSliders[2].fGeometry = SkRect::MakeLTRB(0, 440, 10, 460);
        fSliders[2].fOffset = 0.0f;
        fSliders[2].fScale = 0.0025f;

        fSceneChanged = true;
        fLightsChanged = true;

        fSelectedRect = -1;
        fSelectedSlider = -1;
        fMoveLight = false;

        fClearShadowMaps = false;

        fShadowParams.fShadowRadius = 2.0f;
        fShadowParams.fBiasingConstant = 0.3f;
        fShadowParams.fMinVariance = 1024;
        fShadowParams.fType = SkShadowParams::kVariance_ShadowType;
    }

protected:
    bool onQuery(SkEvent *evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "shadowing");
            return true;
        }

        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case 'L':
                    fMoveLight = !fMoveLight;
                    break;
                case 'd':
                    // Raster generated shadow maps have their origin in the UL corner
                    // GPU shadow maps can have an arbitrary origin.
                    // We override the 'd' keypress so that when the device is cycled,
                    // the shadow maps will be re-generated according to the new backend.
                    fClearShadowMaps = true;
                    break;
                case 'B':
                    if (SkShadowParams::kVariance_ShadowType == fShadowParams.fType) {
                        fShadowParams.fType = SkShadowParams::kNoBlur_ShadowType;
                    } else if (SkShadowParams::kNoBlur_ShadowType ==
                               fShadowParams.fType) {
                        fShadowParams.fType = SkShadowParams::kVariance_ShadowType;
                    }
                    fLightsChanged = true;
                    break;
                default:
                    break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    sk_sp<SkPicture> makeTestPicture(int width, int height) {
        SkPictureRecorder recorder;

        // LONG RANGE TODO: eventually add SkBBHFactory (bounding box factory)
        SkCanvas* canvas = recorder.beginRecording(SkRect::MakeIWH(width, height));

        SkASSERT(canvas->getTotalMatrix().isIdentity());
        SkPaint paint;
        paint.setColor(SK_ColorGRAY);

        // LONG RANGE TODO: tag occluders
        // LONG RANGE TODO: track number of IDs we need (hopefully less than 256)
        //                  and determinate the mapping from z to id

        // universal receiver, "ground"
        canvas->drawRect(SkRect::MakeIWH(width, height), paint);

        for (int i = 0; i < kNumTestRects; i++) {
            paint.setColor(fTestRects[i].fColor);
            if (i == 0) {
                canvas->translateZ(fTestRects[0].fDepth);
            } else {
                canvas->translateZ(fTestRects[i].fDepth - fTestRects[i-1].fDepth);
            }
            canvas->drawRect(fTestRects[i].fGeometry, paint);
        }

        return recorder.finishRecordingAsPicture();
    }

    void onDrawContent(SkCanvas *canvas) override {
        if (fSceneChanged) {
            fPicture = this->makeTestPicture(kWidth, kHeight);
        }

        if (fSceneChanged || fLightsChanged || fClearShadowMaps) {
            for (int i = 0; i < fLights->numLights(); i++) {
                fLights->light(i).setShadowMap(nullptr);
            }

            fSceneChanged = false;
            fLightsChanged = false;
            fClearShadowMaps = false;
        }

        canvas->setLights(fLights);
        canvas->drawShadowedPicture(fPicture, nullptr, nullptr, fShadowParams);

        for (int i = 0; i < kNumSliders; i++) {
            SkPaint paint;
            paint.setColor(SK_ColorBLACK);
            canvas->drawRect(fSliders[i].fGeometry, paint);
        }
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        return new SkView::Click(this);
    }

    void updateFromSelectedSlider() {
        SkScalar newValue = fSliders[fSelectedSlider].fGeometry.fLeft *
                            fSliders[fSelectedSlider].fScale +
                            fSliders[fSelectedSlider].fOffset;

        switch (fSelectedSlider) {
            case 0:
                fShadowParams.fShadowRadius = newValue;
                break;
            case 1:
                fShadowParams.fMinVariance = newValue;
                break;
            case 2:
                fShadowParams.fBiasingConstant = newValue;
                break;
            default:
                break;
        }
    }

    bool onClick(Click *click) override {
        SkScalar x = click->fCurr.fX;
        SkScalar y = click->fCurr.fY;

        SkScalar dx = x - click->fPrev.fX;
        SkScalar dy = y - click->fPrev.fY;

        if (fMoveLight) {
            if (dx != 0 || dy != 0) {
                float recipX = 1.0f / kWidth;
                float recipY = 1.0f / kHeight;

                SkLights::Builder builder;
                builder.add(SkLights::Light::MakeDirectional(
                        SkColor3f::Make(0.2f, 0.3f, 0.4f),
                        SkVector3::Make(0.2f + (200.0f - x) * recipX,
                                        0.05f + (200.0f - y) * recipY,
                                        1.0f)));
                builder.add(SkLights::Light::MakeDirectional(
                        SkColor3f::Make(0.4f, 0.3f, 0.2f),
                        SkVector3::Make(0.05f + (200.0f - x) * recipX,
                                        0.2f + (200.0f - y) * recipY,
                                        1.0f)));
                builder.add(SkLights::Light::MakeAmbient(
                        SkColor3f::Make(0.4f, 0.4f, 0.4f)));
                fLights = builder.finish();

                fLightsChanged = true;
                this->inval(nullptr);
            }
            return true;
        }

        if (click->fState == Click::State::kUp_State) {
            fSelectedRect = -1;
            fSelectedSlider = -1;
            return true;
        }

        if (fSelectedRect > -1) {
            fTestRects[fSelectedRect].fGeometry.offset(dx, dy);

            fSceneChanged = true;
            this->inval(nullptr);
            return true;
        }

        if (fSelectedSlider > -1) {
            fSliders[fSelectedSlider].fGeometry.offset(dx, 0);

            this->updateFromSelectedSlider();

            fLightsChanged = true;
            this->inval(nullptr);
            return true;
        }

        // assume last elements are highest
        for (int i = kNumTestRects - 1; i >= 0; i--) {
            if (fTestRects[i].fGeometry.contains(SkRect::MakeXYWH(x, y, 1, 1))) {
                fSelectedRect = i;
                fTestRects[i].fGeometry.offset(dx, dy);

                fSceneChanged = true;
                this->inval(nullptr);
                break;
            }
        }

        for (int i = 0; i <= kNumSliders; i++) {
            if (fSliders[i].fGeometry.contains(SkRect::MakeXYWH(x, y, 1, 1))) {
                fSelectedSlider = i;
                fSliders[i].fGeometry.offset(dx, 0);

                this->updateFromSelectedSlider();

                fLightsChanged = true;

                this->inval(nullptr);
                break;
            }
        }

        return true;
    }

private:
    static constexpr int kNumTestRects = 3;
    static constexpr int kNumSliders = 3;

    static const int kWidth = 400;
    static const int kHeight = 400;

    struct {
        SkRect  fGeometry;
        int     fDepth;
        SkColor fColor;
    } fTestRects[kNumTestRects];
    int fSelectedRect;

    struct {
        SkRect   fGeometry;
        SkScalar fOffset;
        SkScalar fScale;
    } fSliders[kNumSliders];
    int fSelectedSlider;

    bool fClearShadowMaps;
    bool fMoveLight;
    bool fSceneChanged;
    bool fLightsChanged;

    sk_sp<SkPicture> fPicture;
    SkShadowParams fShadowParams;
    sk_sp<SkLights> fLights;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
static SkView* MyFactory() { return new ShadowingView; }
static SkViewRegister reg(MyFactory);

#endif
