/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/layers/shapelayer/ShapeLayer.h"

#include "include/core/SkPath.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGeometryTransform.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGMerge.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGPath.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGRoundEffect.h"
#include "modules/sksg/include/SkSGTransform.h"
#include "modules/sksg/include/SkSGTrimEffect.h"
#include "src/utils/SkJSON.h"

#include <algorithm>
#include <iterator>

namespace skottie {
namespace internal {

namespace {

using GeometryAttacherT = sk_sp<sksg::GeometryNode> (*)(const skjson::ObjectValue&,
                                                        const AnimationBuilder*);
static constexpr GeometryAttacherT gGeometryAttachers[] = {
    ShapeBuilder::AttachPathGeometry,
    ShapeBuilder::AttachRRectGeometry,
    ShapeBuilder::AttachEllipseGeometry,
    ShapeBuilder::AttachPolystarGeometry,
};

using PaintAttacherT = sk_sp<sksg::PaintNode> (*)(const skjson::ObjectValue&,
                                                  const AnimationBuilder*);
static constexpr PaintAttacherT gPaintAttachers[] = {
    ShapeBuilder::AttachColorFill,
    ShapeBuilder::AttachColorStroke,
    ShapeBuilder::AttachGradientFill,
    ShapeBuilder::AttachGradientStroke,
};

using GeometryEffectAttacherT =
    std::vector<sk_sp<sksg::GeometryNode>> (*)(const skjson::ObjectValue&,
                                               const AnimationBuilder*,
                                               std::vector<sk_sp<sksg::GeometryNode>>&&);
static constexpr GeometryEffectAttacherT gGeometryEffectAttachers[] = {
    ShapeBuilder::AttachMergeGeometryEffect,
    ShapeBuilder::AttachTrimGeometryEffect,
    ShapeBuilder::AttachRoundGeometryEffect,
};

using DrawEffectAttacherT =
    std::vector<sk_sp<sksg::RenderNode>> (*)(const skjson::ObjectValue&,
                                             const AnimationBuilder*,
                                             std::vector<sk_sp<sksg::RenderNode>>&&);

static constexpr DrawEffectAttacherT gDrawEffectAttachers[] = {
    ShapeBuilder::AttachRepeaterDrawEffect,
};

enum class ShapeType {
    kGeometry,
    kGeometryEffect,
    kPaint,
    kGroup,
    kTransform,
    kDrawEffect,
};

struct ShapeInfo {
    const char* fTypeString;
    ShapeType   fShapeType;
    uint32_t    fAttacherIndex; // index into respective attacher tables
};

const ShapeInfo* FindShapeInfo(const skjson::ObjectValue& jshape) {
    static constexpr ShapeInfo gShapeInfo[] = {
        { "el", ShapeType::kGeometry      , 2 }, // ellipse   -> AttachEllipseGeometry
        { "fl", ShapeType::kPaint         , 0 }, // fill      -> AttachColorFill
        { "gf", ShapeType::kPaint         , 2 }, // gfill     -> AttachGradientFill
        { "gr", ShapeType::kGroup         , 0 }, // group     -> Inline handler
        { "gs", ShapeType::kPaint         , 3 }, // gstroke   -> AttachGradientStroke
        { "mm", ShapeType::kGeometryEffect, 0 }, // merge     -> AttachMergeGeometryEffect
        { "rc", ShapeType::kGeometry      , 1 }, // rrect     -> AttachRRectGeometry
        { "rd", ShapeType::kGeometryEffect, 2 }, // round     -> AttachRoundGeometryEffect
        { "rp", ShapeType::kDrawEffect    , 0 }, // repeater  -> AttachRepeaterDrawEffect
        { "sh", ShapeType::kGeometry      , 0 }, // shape     -> AttachPathGeometry
        { "sr", ShapeType::kGeometry      , 3 }, // polystar  -> AttachPolyStarGeometry
        { "st", ShapeType::kPaint         , 1 }, // stroke    -> AttachColorStroke
        { "tm", ShapeType::kGeometryEffect, 1 }, // trim      -> AttachTrimGeometryEffect
        { "tr", ShapeType::kTransform     , 0 }, // transform -> Inline handler
    };

    const skjson::StringValue* type = jshape["ty"];
    if (!type) {
        return nullptr;
    }

    const auto* info = bsearch(type->begin(),
                               gShapeInfo,
                               SK_ARRAY_COUNT(gShapeInfo),
                               sizeof(ShapeInfo),
                               [](const void* key, const void* info) {
                                  return strcmp(static_cast<const char*>(key),
                                                static_cast<const ShapeInfo*>(info)->fTypeString);
                               });

    return static_cast<const ShapeInfo*>(info);
}

struct GeometryEffectRec {
    const skjson::ObjectValue& fJson;
    GeometryEffectAttacherT    fAttach;
};

} // namespace

sk_sp<sksg::GeometryNode> ShapeBuilder::AttachPathGeometry(const skjson::ObjectValue& jpath,
                                                           const AnimationBuilder* abuilder) {
    return abuilder->attachPath(jpath["ks"]);
}

struct AnimationBuilder::AttachShapeContext {
    AttachShapeContext(std::vector<sk_sp<sksg::GeometryNode>>* geos,
                       std::vector<GeometryEffectRec>* effects,
                       size_t committedAnimators)
        : fGeometryStack(geos)
        , fGeometryEffectStack(effects)
        , fCommittedAnimators(committedAnimators) {}

    std::vector<sk_sp<sksg::GeometryNode>>* fGeometryStack;
    std::vector<GeometryEffectRec>*         fGeometryEffectStack;
    size_t                                  fCommittedAnimators;
};

sk_sp<sksg::RenderNode> AnimationBuilder::attachShape(const skjson::ArrayValue* jshape,
                                                      AttachShapeContext* ctx) const {
    if (!jshape)
        return nullptr;

    SkDEBUGCODE(const auto initialGeometryEffects = ctx->fGeometryEffectStack->size();)

    const skjson::ObjectValue* jtransform = nullptr;

    struct ShapeRec {
        const skjson::ObjectValue& fJson;
        const ShapeInfo&           fInfo;
    };

    // First pass (bottom->top):
    //
    //   * pick up the group transform and opacity
    //   * push local geometry effects onto the stack
    //   * store recs for next pass
    //
    std::vector<ShapeRec> recs;
    for (size_t i = 0; i < jshape->size(); ++i) {
        const skjson::ObjectValue* shape = (*jshape)[jshape->size() - 1 - i];
        if (!shape) continue;

        const auto* info = FindShapeInfo(*shape);
        if (!info) {
            this->log(Logger::Level::kError, &(*shape)["ty"], "Unknown shape.");
            continue;
        }

        if (ParseDefault<bool>((*shape)["hd"], false)) {
            // Ignore hidden shapes.
            continue;
        }

        recs.push_back({ *shape, *info });

        switch (info->fShapeType) {
        case ShapeType::kTransform:
            // Just track the transform property for now -- we'll deal with it later.
            jtransform = shape;
            break;
        case ShapeType::kGeometryEffect:
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gGeometryEffectAttachers));
            ctx->fGeometryEffectStack->push_back(
                { *shape, gGeometryEffectAttachers[info->fAttacherIndex] });
            break;
        default:
            break;
        }
    }

    // Second pass (top -> bottom, after 2x reverse):
    //
    //   * track local geometry
    //   * emit local paints
    //
    std::vector<sk_sp<sksg::GeometryNode>> geos;
    std::vector<sk_sp<sksg::RenderNode  >> draws;

    const auto add_draw = [this, &draws](sk_sp<sksg::RenderNode> draw, const ShapeRec& rec) {
        // All draws can have an optional blend mode.
        draws.push_back(this->attachBlendMode(rec.fJson, std::move(draw)));
    };

    for (auto rec = recs.rbegin(); rec != recs.rend(); ++rec) {
        const AutoPropertyTracker apt(this, rec->fJson);

        switch (rec->fInfo.fShapeType) {
        case ShapeType::kGeometry: {
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gGeometryAttachers));
            if (auto geo = gGeometryAttachers[rec->fInfo.fAttacherIndex](rec->fJson, this)) {
                geos.push_back(std::move(geo));
            }
        } break;
        case ShapeType::kGeometryEffect: {
            // Apply the current effect and pop from the stack.
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gGeometryEffectAttachers));
            if (!geos.empty()) {
                geos = gGeometryEffectAttachers[rec->fInfo.fAttacherIndex](rec->fJson,
                                                                           this,
                                                                           std::move(geos));
            }

            SkASSERT(&ctx->fGeometryEffectStack->back().fJson == &rec->fJson);
            SkASSERT(ctx->fGeometryEffectStack->back().fAttach ==
                     gGeometryEffectAttachers[rec->fInfo.fAttacherIndex]);
            ctx->fGeometryEffectStack->pop_back();
        } break;
        case ShapeType::kGroup: {
            AttachShapeContext groupShapeCtx(&geos,
                                             ctx->fGeometryEffectStack,
                                             ctx->fCommittedAnimators);
            if (auto subgroup = this->attachShape(rec->fJson["it"], &groupShapeCtx)) {
                add_draw(std::move(subgroup), *rec);
                SkASSERT(groupShapeCtx.fCommittedAnimators >= ctx->fCommittedAnimators);
                ctx->fCommittedAnimators = groupShapeCtx.fCommittedAnimators;
            }
        } break;
        case ShapeType::kPaint: {
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gPaintAttachers));
            auto paint = gPaintAttachers[rec->fInfo.fAttacherIndex](rec->fJson, this);
            if (!paint || geos.empty())
                break;

            auto drawGeos = geos;

            // Apply all pending effects from the stack.
            for (auto it = ctx->fGeometryEffectStack->rbegin();
                 it != ctx->fGeometryEffectStack->rend(); ++it) {
                drawGeos = it->fAttach(it->fJson, this, std::move(drawGeos));
            }

            // If we still have multiple geos, reduce using 'merge'.
            auto geo = drawGeos.size() > 1
                ? ShapeBuilder::MergeGeometry(std::move(drawGeos), sksg::Merge::Mode::kMerge)
                : drawGeos[0];

            SkASSERT(geo);
            add_draw(sksg::Draw::Make(std::move(geo), std::move(paint)), *rec);
            ctx->fCommittedAnimators = fCurrentAnimatorScope->size();
        } break;
        case ShapeType::kDrawEffect: {
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gDrawEffectAttachers));
            if (!draws.empty()) {
                draws = gDrawEffectAttachers[rec->fInfo.fAttacherIndex](rec->fJson,
                                                                        this,
                                                                        std::move(draws));
                ctx->fCommittedAnimators = fCurrentAnimatorScope->size();
            }
        } break;
        default:
            break;
        }
    }

    // By now we should have popped all local geometry effects.
    SkASSERT(ctx->fGeometryEffectStack->size() == initialGeometryEffects);

    sk_sp<sksg::RenderNode> shape_wrapper;
    if (draws.size() == 1) {
        // For a single draw, we don't need a group.
        shape_wrapper = std::move(draws.front());
    } else if (!draws.empty()) {
        // Emit local draws reversed (bottom->top, per spec).
        std::reverse(draws.begin(), draws.end());
        draws.shrink_to_fit();

        // We need a group to dispatch multiple draws.
        shape_wrapper = sksg::Group::Make(std::move(draws));
    }

    sk_sp<sksg::Transform> shape_transform;
    if (jtransform) {
        const AutoPropertyTracker apt(this, *jtransform);

        // This is tricky due to the interaction with ctx->fCommittedAnimators: we want any
        // animators related to tranform/opacity to be committed => they must be inserted in front
        // of the dangling/uncommitted ones.
        AutoScope ascope(this);

        if ((shape_transform = this->attachMatrix2D(*jtransform, nullptr))) {
            shape_wrapper = sksg::TransformEffect::Make(std::move(shape_wrapper), shape_transform);
        }
        shape_wrapper = this->attachOpacity(*jtransform, std::move(shape_wrapper));

        auto local_scope = ascope.release();
        fCurrentAnimatorScope->insert(fCurrentAnimatorScope->begin() + ctx->fCommittedAnimators,
                                      std::make_move_iterator(local_scope.begin()),
                                      std::make_move_iterator(local_scope.end()));
        ctx->fCommittedAnimators += local_scope.size();
    }

    // Push transformed local geometries to parent list, for subsequent paints.
    for (auto& geo : geos) {
        ctx->fGeometryStack->push_back(shape_transform
            ? sksg::GeometryTransform::Make(std::move(geo), shape_transform)
            : std::move(geo));
    }

    return shape_wrapper;
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachShapeLayer(const skjson::ObjectValue& layer,
                                                           LayerInfo*) const {
    std::vector<sk_sp<sksg::GeometryNode>> geometryStack;
    std::vector<GeometryEffectRec> geometryEffectStack;
    AttachShapeContext shapeCtx(&geometryStack, &geometryEffectStack,
                                fCurrentAnimatorScope->size());
    auto shapeNode = this->attachShape(layer["shapes"], &shapeCtx);

    // Trim uncommitted animators: AttachShape consumes effects on the fly, and greedily attaches
    // geometries => at the end, we can end up with unused geometries, which are nevertheless alive
    // due to attached animators.  To avoid this, we track committed animators and discard the
    // orphans here.
    SkASSERT(shapeCtx.fCommittedAnimators <= fCurrentAnimatorScope->size());
    fCurrentAnimatorScope->resize(shapeCtx.fCommittedAnimators);

    return shapeNode;
}

} // namespace internal
} // namespace skottie
