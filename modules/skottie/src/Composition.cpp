/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottiePriv.h"

#include "include/core/SkCanvas.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGTransform.h"

#include "src/core/SkMakeUnique.h"

#include <algorithm>

namespace skottie {
namespace internal {

sk_sp<sksg::RenderNode> AnimationBuilder::attachNestedAnimation(const char* name,
                                                                AnimatorScope* ascope) const {
    class SkottieSGAdapter final : public sksg::RenderNode {
    public:
        explicit SkottieSGAdapter(sk_sp<Animation> animation)
            : fAnimation(std::move(animation)) {
            SkASSERT(fAnimation);
        }

    protected:
        SkRect onRevalidate(sksg::InvalidationController*, const SkMatrix&) override {
            return SkRect::MakeSize(fAnimation->size());
        }

        const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; }

        void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
            const auto local_scope =
                ScopedRenderContext(canvas, ctx).setIsolation(this->bounds(),
                                                              canvas->getTotalMatrix(),
                                                              true);
            fAnimation->render(canvas);
        }

    private:
        const sk_sp<Animation> fAnimation;
    };

    class SkottieAnimatorAdapter final : public sksg::Animator {
    public:
        SkottieAnimatorAdapter(sk_sp<Animation> animation, float time_scale)
            : fAnimation(std::move(animation))
            , fTimeScale(time_scale) {
            SkASSERT(fAnimation);
        }

    protected:
        void onTick(float t) {
            // TODO: we prolly need more sophisticated timeline mapping for nested animations.
            fAnimation->seek(t * fTimeScale);
        }

    private:
        const sk_sp<Animation> fAnimation;
        const float            fTimeScale;
    };

    const auto data = fResourceProvider->load("", name);
    if (!data) {
        this->log(Logger::Level::kError, nullptr, "Could not load: %s.", name);
        return nullptr;
    }

    auto animation = Animation::Builder()
            .setResourceProvider(fResourceProvider)
            .setFontManager(fLazyFontMgr.getMaybeNull())
            .make(static_cast<const char*>(data->data()), data->size());
    if (!animation) {
        this->log(Logger::Level::kError, nullptr, "Could not parse nested animation: %s.", name);
        return nullptr;
    }

    ascope->push_back(
        skstd::make_unique<SkottieAnimatorAdapter>(animation, animation->duration() / fDuration));

    return sk_make_sp<SkottieSGAdapter>(std::move(animation));
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachAssetRef(
    const skjson::ObjectValue& jlayer, AnimatorScope* ascope,
    const std::function<sk_sp<sksg::RenderNode>(const skjson::ObjectValue&,
                                                AnimatorScope*)>& func) const {

    const auto refId = ParseDefault<SkString>(jlayer["refId"], SkString());
    if (refId.isEmpty()) {
        this->log(Logger::Level::kError, nullptr, "Layer missing refId.");
        return nullptr;
    }

    if (refId.startsWith("$")) {
        return this->attachNestedAnimation(refId.c_str() + 1, ascope);
    }

    const auto* asset_info = fAssets.find(refId);
    if (!asset_info) {
        this->log(Logger::Level::kError, nullptr, "Asset not found: '%s'.", refId.c_str());
        return nullptr;
    }

    if (asset_info->fIsAttaching) {
        this->log(Logger::Level::kError, nullptr,
                  "Asset cycle detected for: '%s'", refId.c_str());
        return nullptr;
    }

    asset_info->fIsAttaching = true;
    auto asset = func(*asset_info->fAsset, ascope);
    asset_info->fIsAttaching = false;

    return asset;
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachComposition(const skjson::ObjectValue& jcomp,
                                                            AnimatorScope* scope) const {
    const skjson::ArrayValue* jlayers = jcomp["layers"];
    if (!jlayers) return nullptr;

    std::vector<sk_sp<sksg::RenderNode>> layers;
    AttachLayerContext                   layerCtx(*jlayers);

    // Optional motion blur params.
    if (const skjson::ObjectValue* jmb = jcomp["mb"]) {
        static constexpr size_t kMaxSamplesPerFrame = 64;
        layerCtx.fMotionBlurSamples = std::min(ParseDefault<size_t>((*jmb)["spf"], 1ul),
                                               kMaxSamplesPerFrame);
        layerCtx.fMotionBlurAngle = SkTPin(ParseDefault((*jmb)["sa"], 0.0f),    0.0f, 720.0f);
        layerCtx.fMotionBlurPhase = SkTPin(ParseDefault((*jmb)["sp"], 0.0f), -360.0f, 360.0f);
    }

    layers.reserve(jlayers->size());
    for (const auto& l : *jlayers) {
        if (auto layer = this->attachLayer(l, scope, &layerCtx)) {
            layers.push_back(std::move(layer));
        }
    }

    if (layers.empty()) {
        return nullptr;
    }

    sk_sp<sksg::RenderNode> comp;
    if (layers.size() == 1) {
        comp = std::move(layers[0]);
    } else {
        // Layers are painted in bottom->top order.
        std::reverse(layers.begin(), layers.end());
        layers.shrink_to_fit();
        comp = sksg::Group::Make(std::move(layers));
    }

    // Optional camera.
    if (layerCtx.fCameraTransform) {
        comp = sksg::TransformEffect::Make(std::move(comp), std::move(layerCtx.fCameraTransform));
    }

    return comp;
}

} // namespace internal
} // namespace skottie
