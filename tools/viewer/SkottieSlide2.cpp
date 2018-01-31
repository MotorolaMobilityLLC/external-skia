/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieSlide.h"

#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "Skottie.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkSGGroup.h"
#include "SkSGRenderNode.h"
#include "SkSGScene.h"
#include "SkSGTransform.h"
#include "SkStream.h"

static constexpr int CELL_WIDTH  = 240;
static constexpr int CELL_HEIGHT = 160;
static constexpr int COL_COUNT   = 4;
static constexpr int SPACER_X    = 12;
static constexpr int SPACER_Y    = 24;
static constexpr int MARGIN      = 8;

class SkottieSlide2::AnimationWrapper final : public sksg::RenderNode {
public:
    explicit AnimationWrapper(std::unique_ptr<skottie::Animation> anim)
        : fAnimation(std::move(anim)) {
        SkASSERT(fAnimation);
    }

    void tick(SkMSec t) {
        fAnimation->animationTick(t);
        this->invalidate();
    }

    void setShowInval(bool show) { fAnimation->setShowInval(show); }

protected:
    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        return SkRect::MakeSize(fAnimation->size());
    }

    void onRender(SkCanvas* canvas) const override {
        fAnimation->render(canvas);
    }

private:
    const std::unique_ptr<skottie::Animation> fAnimation;

    using INHERITED = sksg::RenderNode;
};

SkottieSlide2::Rec::Rec(sk_sp<AnimationWrapper> wrapper)
    : fWrapper(std::move(wrapper)) {}

SkottieSlide2::Rec::Rec(Rec&& o) = default;

SkottieSlide2::SkottieSlide2(const SkString& path)
    : fPath(path)
{
    fName.set("skottie-dir");
}

void SkottieSlide2::load(SkScalar, SkScalar) {
    SkString name;
    SkOSFile::Iter iter(fPath.c_str(), "json");

    int x = 0, y = 0;

    // Build a global scene using tranformed animation fragments:
    //
    // [Group]
    //     [Transform]
    //         [AnimationWrapper]
    //     [Transform]
    //         [AnimationWrapper]
    //     ...
    //
    // Note: for now animation wrappers are also tracked externally in fAnims, for tick dispatching.
    auto scene_root = sksg::Group::Make();

    while (iter.next(&name)) {
        SkString path = SkOSPath::Join(fPath.c_str(), name.c_str());
        if (auto anim  = skottie::Animation::MakeFromFile(path.c_str())) {
            const SkRect src = SkRect::MakeSize(anim->size()),
                         dst = SkRect::MakeXYWH(MARGIN + x * (CELL_WIDTH + SPACER_X),
                                                MARGIN + y * (CELL_HEIGHT + SPACER_Y),
                                                CELL_WIDTH, CELL_HEIGHT);
            auto wrapper = sk_make_sp<AnimationWrapper>(std::move(anim));
            auto matrix  = sksg::Matrix::Make(
                SkMatrix::MakeRectToRect(src, dst, SkMatrix::kCenter_ScaleToFit));
            auto xform   = sksg::Transform::Make(wrapper, std::move(matrix));

            scene_root->addChild(xform);
            fAnims.emplace_back(std::move(wrapper)).fName = name;

            if (++x == COL_COUNT) {
                x = 0;
                y += 1;
            }
        }
    }

    fScene = sksg::Scene::Make(std::move(scene_root), sksg::AnimatorList());
}

void SkottieSlide2::unload() {
    fAnims.reset();
    fScene.reset();
}

SkISize SkottieSlide2::getDimensions() const {
    const int rows = (fAnims.count() + COL_COUNT - 1) / COL_COUNT;
    return {
        MARGIN + (COL_COUNT - 1) * SPACER_X + COL_COUNT * CELL_WIDTH + MARGIN,
        MARGIN + (rows - 1) * SPACER_Y + rows * CELL_HEIGHT + MARGIN,
    };
}

void SkottieSlide2::draw(SkCanvas* canvas) {
    fScene->render(canvas);

    // TODO: this is all only to draw labels; replace with sksg::Text nodes, when available.
    SkPaint paint;
    paint.setTextSize(12);
    paint.setAntiAlias(true);
    paint.setTextAlign(SkPaint::kCenter_Align);

    const SkRect dst = SkRect::MakeIWH(CELL_WIDTH, CELL_HEIGHT);
    int x = 0, y = 0;

    canvas->translate(MARGIN, MARGIN);
    for (const auto& rec : fAnims) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(x * (CELL_WIDTH + SPACER_X), y * (CELL_HEIGHT + SPACER_Y));
        canvas->drawText(rec.fName.c_str(), rec.fName.size(),
                         dst.centerX(), dst.bottom() + paint.getTextSize(), paint);
        if (++x == COL_COUNT) {
            x = 0;
            y += 1;
        }
    }
}

bool SkottieSlide2::animate(const SkAnimTimer& timer) {
    for (auto& rec : fAnims) {
        if (rec.fTimeBase == 0) {
            // Reset the animation time.
            rec.fTimeBase = timer.msec();
        }
        rec.fWrapper->tick(timer.msec() - rec.fTimeBase);
    }
    return true;
}

bool SkottieSlide2::onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state,
                           uint32_t modifiers) {
    if (fTrackingCell < 0 && state == sk_app::Window::kDown_InputState) {
        fTrackingCell = this->findCell(x, y);
    }
    if (fTrackingCell >= 0 && state == sk_app::Window::kUp_InputState) {
        int index = this->findCell(x, y);
        if (fTrackingCell == index) {
            fAnims[index].fShowAnimationInval = !fAnims[index].fShowAnimationInval;
            fAnims[index].fWrapper->setShowInval(fAnims[index].fShowAnimationInval);
        }
        fTrackingCell = -1;
    }
    return fTrackingCell >= 0;
}

int SkottieSlide2::findCell(float x, float y) const {
    x -= MARGIN;
    y -= MARGIN;
    int index = -1;
    if (x >= 0 && y >= 0) {
        int ix = (int)x;
        int iy = (int)y;
        int col = ix / (CELL_WIDTH + SPACER_X);
        int row = iy / (CELL_HEIGHT + SPACER_Y);
        index = row * COL_COUNT + col;
        if (index >= fAnims.count()) {
            index = -1;
        }
    }
    return index;
}
