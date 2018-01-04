/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Skotty.h"

#include "SkCanvas.h"
#include "SkottyAnimator.h"
#include "SkottyPriv.h"
#include "SkottyProperties.h"
#include "SkData.h"
#include "SkMakeUnique.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGInvalidationController.h"
#include "SkSGGroup.h"
#include "SkSGMerge.h"
#include "SkSGPath.h"
#include "SkSGRect.h"
#include "SkSGTransform.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTHash.h"

#include <cmath>
#include <vector>

#include "stdlib.h"

namespace skotty {

namespace {

using AssetMap = SkTHashMap<SkString, const Json::Value*>;

struct AttachContext {
    const AssetMap&                          fAssets;
    SkTArray<std::unique_ptr<AnimatorBase>>& fAnimators;
};

bool LogFail(const Json::Value& json, const char* msg) {
    const auto dump = json.toStyledString();
    LOG("!! %s: %s", msg, dump.c_str());
    return false;
}

// This is the workhorse for binding properties: depending on whether the property is animated,
// it will either apply immediately or instantiate and attach a keyframe animator.
template <typename ValueT, typename AttrT, typename NodeT, typename ApplyFuncT>
bool AttachProperty(const Json::Value& jprop, AttachContext* ctx, const sk_sp<NodeT>& node,
                    ApplyFuncT&& apply) {
    if (!jprop.isObject())
        return false;

    if (!ParseBool(jprop["a"], false)) {
        // Static property.
        ValueT val;
        if (!ValueT::Parse(jprop["k"], &val)) {
            return LogFail(jprop, "Could not parse static property");
        }

        apply(node, val.template as<AttrT>());
    } else {
        // Keyframe property.
        using AnimatorT = Animator<ValueT, AttrT, NodeT>;
        auto animator = AnimatorT::Make(jprop["k"], node, std::move(apply));

        if (!animator) {
            return LogFail(jprop, "Could not instantiate keyframe animator");
        }

        ctx->fAnimators.push_back(std::move(animator));
    }

    return true;
}

sk_sp<sksg::RenderNode> AttachTransform(const Json::Value& t, AttachContext* ctx,
                                        sk_sp<sksg::RenderNode> wrapped_node) {
    if (!t.isObject() || !wrapped_node)
        return wrapped_node;

    auto xform = sk_make_sp<CompositeTransform>(wrapped_node);
    auto anchor_attached = AttachProperty<VectorValue, SkPoint>(t["a"], ctx, xform,
            [](const sk_sp<CompositeTransform>& node, const SkPoint& a) {
                node->setAnchorPoint(a);
            });
    auto position_attached = AttachProperty<VectorValue, SkPoint>(t["p"], ctx, xform,
            [](const sk_sp<CompositeTransform>& node, const SkPoint& p) {
                node->setPosition(p);
            });
    auto scale_attached = AttachProperty<VectorValue, SkVector>(t["s"], ctx, xform,
            [](const sk_sp<CompositeTransform>& node, const SkVector& s) {
                node->setScale(s);
            });
    auto rotation_attached = AttachProperty<ScalarValue, SkScalar>(t["r"], ctx, xform,
            [](const sk_sp<CompositeTransform>& node, SkScalar r) {
                node->setRotation(r);
            });
    auto skew_attached = AttachProperty<ScalarValue, SkScalar>(t["sk"], ctx, xform,
            [](const sk_sp<CompositeTransform>& node, SkScalar sk) {
                node->setSkew(sk);
            });
    auto skewaxis_attached = AttachProperty<ScalarValue, SkScalar>(t["sa"], ctx, xform,
            [](const sk_sp<CompositeTransform>& node, SkScalar sa) {
                node->setSkewAxis(sa);
            });

    if (!anchor_attached &&
        !position_attached &&
        !scale_attached &&
        !rotation_attached &&
        !skew_attached &&
        !skewaxis_attached) {
        LogFail(t, "Could not parse transform");
        return wrapped_node;
    }

    return xform->node();
}

sk_sp<sksg::RenderNode> AttachShape(const Json::Value&, AttachContext* ctx);
sk_sp<sksg::RenderNode> AttachComposition(const Json::Value&, AttachContext* ctx);

sk_sp<sksg::RenderNode> AttachShapeGroup(const Json::Value& jgroup, AttachContext* ctx) {
    SkASSERT(jgroup.isObject());

    return AttachShape(jgroup["it"], ctx);
}

sk_sp<sksg::GeometryNode> AttachPathGeometry(const Json::Value& jpath, AttachContext* ctx) {
    SkASSERT(jpath.isObject());

    auto path_node = sksg::Path::Make();
    auto path_attached = AttachProperty<ShapeValue, SkPath>(jpath["ks"], ctx, path_node,
        [](const sk_sp<sksg::Path>& node, const SkPath& p) { node->setPath(p); });

    if (path_attached)
        LOG("** Attached path geometry - verbs: %d\n", path_node->getPath().countVerbs());

    return path_attached ? path_node : nullptr;
}

sk_sp<sksg::GeometryNode> AttachRRectGeometry(const Json::Value& jrect, AttachContext* ctx) {
    SkASSERT(jrect.isObject());

    auto rect_node = sksg::RRect::Make();
    auto composite = sk_make_sp<CompositeRRect>(rect_node);

    auto p_attached = AttachProperty<VectorValue, SkPoint>(jrect["p"], ctx, composite,
            [](const sk_sp<CompositeRRect>& node, const SkPoint& pos) { node->setPosition(pos); });
    auto s_attached = AttachProperty<VectorValue, SkSize>(jrect["s"], ctx, composite,
            [](const sk_sp<CompositeRRect>& node, const SkSize& sz) { node->setSize(sz); });
    auto r_attached = AttachProperty<ScalarValue, SkScalar>(jrect["r"], ctx, composite,
            [](const sk_sp<CompositeRRect>& node, SkScalar radius) { node->setRadius(radius); });

    if (!p_attached && !s_attached && !r_attached) {
        return nullptr;
    }

    return rect_node;
}

sk_sp<sksg::Color> AttachColorPaint(const Json::Value& obj, AttachContext* ctx) {
    SkASSERT(obj.isObject());

    auto color_node = sksg::Color::Make(SK_ColorBLACK);
    color_node->setAntiAlias(true);

    auto color_attached = AttachProperty<VectorValue, SkColor>(obj["c"], ctx, color_node,
        [](const sk_sp<sksg::Color>& node, SkColor c) { node->setColor(c); });

    return color_attached ? color_node : nullptr;
}

sk_sp<sksg::PaintNode> AttachFillPaint(const Json::Value& jfill, AttachContext* ctx) {
    SkASSERT(jfill.isObject());

    auto color = AttachColorPaint(jfill, ctx);
    if (color) {
        LOG("** Attached color fill: 0x%x\n", color->getColor());
    }
    return color;
}

sk_sp<sksg::PaintNode> AttachStrokePaint(const Json::Value& jstroke, AttachContext* ctx) {
    SkASSERT(jstroke.isObject());

    auto stroke_node = AttachColorPaint(jstroke, ctx);
    if (!stroke_node)
        return nullptr;

    LOG("** Attached color stroke: 0x%x\n", stroke_node->getColor());

    stroke_node->setStyle(SkPaint::kStroke_Style);

    auto width_attached = AttachProperty<ScalarValue, SkScalar>(jstroke["w"], ctx, stroke_node,
        [](const sk_sp<sksg::Color>& node, SkScalar width) { node->setStrokeWidth(width); });
    if (!width_attached)
        return nullptr;

    stroke_node->setStrokeMiter(ParseScalar(jstroke["ml"], 4));

    static constexpr SkPaint::Join gJoins[] = {
        SkPaint::kMiter_Join,
        SkPaint::kRound_Join,
        SkPaint::kBevel_Join,
    };
    stroke_node->setStrokeJoin(gJoins[SkTPin<int>(ParseInt(jstroke["lj"], 1) - 1,
                                                  0, SK_ARRAY_COUNT(gJoins) - 1)]);

    static constexpr SkPaint::Cap gCaps[] = {
        SkPaint::kButt_Cap,
        SkPaint::kRound_Cap,
        SkPaint::kSquare_Cap,
    };
    stroke_node->setStrokeCap(gCaps[SkTPin<int>(ParseInt(jstroke["lc"], 1) - 1,
                                                0, SK_ARRAY_COUNT(gCaps) - 1)]);

    return stroke_node;
}

std::vector<sk_sp<sksg::GeometryNode>> AttachMergeGeometryEffect(
    const Json::Value& jmerge, AttachContext* ctx, std::vector<sk_sp<sksg::GeometryNode>>&& geos) {
    std::vector<sk_sp<sksg::GeometryNode>> merged;

    static constexpr sksg::Merge::Mode gModes[] = {
        sksg::Merge::Mode::kMerge,      // "mm": 1
        sksg::Merge::Mode::kUnion,      // "mm": 2
        sksg::Merge::Mode::kDifference, // "mm": 3
        sksg::Merge::Mode::kIntersect,  // "mm": 4
        sksg::Merge::Mode::kXOR      ,  // "mm": 5
    };

    const auto mode = gModes[SkTPin<int>(ParseInt(jmerge["mm"], 1) - 1, 0, SK_ARRAY_COUNT(gModes))];
    merged.push_back(sksg::Merge::Make(std::move(geos), mode));

    LOG("** Attached merge path effect, mode: %d\n", mode);

    return merged;
}

using GeometryAttacherT = sk_sp<sksg::GeometryNode> (*)(const Json::Value&, AttachContext*);
static constexpr GeometryAttacherT gGeometryAttachers[] = {
    AttachPathGeometry,
    AttachRRectGeometry,
};

using PaintAttacherT = sk_sp<sksg::PaintNode> (*)(const Json::Value&, AttachContext*);
static constexpr PaintAttacherT gPaintAttachers[] = {
    AttachFillPaint,
    AttachStrokePaint,
};

using GroupAttacherT = sk_sp<sksg::RenderNode> (*)(const Json::Value&, AttachContext*);
static constexpr GroupAttacherT gGroupAttachers[] = {
    AttachShapeGroup,
};

using TransformAttacherT = sk_sp<sksg::RenderNode> (*)(const Json::Value&, AttachContext*,
                                                       sk_sp<sksg::RenderNode>);
static constexpr TransformAttacherT gTransformAttachers[] = {
    AttachTransform,
};

using GeometryEffectAttacherT =
    std::vector<sk_sp<sksg::GeometryNode>> (*)(const Json::Value&,
                                               AttachContext*,
                                               std::vector<sk_sp<sksg::GeometryNode>>&&);
static constexpr GeometryEffectAttacherT gGeometryEffectAttachers[] = {
    AttachMergeGeometryEffect,
};

enum class ShapeType {
    kGeometry,
    kGeometryEffect,
    kPaint,
    kGroup,
    kTransform,
};

struct ShapeInfo {
    const char* fTypeString;
    ShapeType   fShapeType;
    uint32_t    fAttacherIndex; // index into respective attacher tables
};

const ShapeInfo* FindShapeInfo(const Json::Value& shape) {
    static constexpr ShapeInfo gShapeInfo[] = {
        { "fl", ShapeType::kPaint         , 0 }, // fill      -> AttachFillPaint
        { "gr", ShapeType::kGroup         , 0 }, // group     -> AttachShapeGroup
        { "mm", ShapeType::kGeometryEffect, 0 }, // merge     -> AttachMergeGeometryEffect
        { "rc", ShapeType::kGeometry      , 1 }, // shape     -> AttachRRectGeometry
        { "sh", ShapeType::kGeometry      , 0 }, // shape     -> AttachPathGeometry
        { "st", ShapeType::kPaint         , 1 }, // stroke    -> AttachStrokePaint
        { "tr", ShapeType::kTransform     , 0 }, // transform -> AttachTransform
    };

    if (!shape.isObject())
        return nullptr;

    const auto& type = shape["ty"];
    if (!type.isString())
        return nullptr;

    const auto* info = bsearch(type.asCString(),
                               gShapeInfo,
                               SK_ARRAY_COUNT(gShapeInfo),
                               sizeof(ShapeInfo),
                               [](const void* key, const void* info) {
                                  return strcmp(static_cast<const char*>(key),
                                                static_cast<const ShapeInfo*>(info)->fTypeString);
                               });

    return static_cast<const ShapeInfo*>(info);
}

sk_sp<sksg::RenderNode> AttachShape(const Json::Value& shapeArray, AttachContext* ctx) {
    if (!shapeArray.isArray())
        return nullptr;

    // (https://helpx.adobe.com/after-effects/using/overview-shape-layers-paths-vector.html#groups_and_render_order_for_shapes_and_shape_attributes)
    //
    // Render order for shapes within a shape layer
    //
    // The rules for rendering a shape layer are similar to the rules for rendering a composition
    // that contains nested compositions:
    //
    //   * Within a group, the shape at the bottom of the Timeline panel stacking order is rendered
    //     first.
    //
    //   * All path operations within a group are performed before paint operations. This means,
    //     for example, that the stroke follows the distortions in the path made by the Wiggle Paths
    //     path operation. Path operations within a group are performed from top to bottom.
    //
    //   * Paint operations within a group are performed from the bottom to the top in the Timeline
    //     panel stacking order. This means, for example, that a stroke is rendered on top of
    //     (in front of) a stroke that appears after it in the Timeline panel.
    //
    sk_sp<sksg::Group>        shape_group = sksg::Group::Make();
    sk_sp<sksg::RenderNode> xformed_group = shape_group;

    std::vector<sk_sp<sksg::GeometryNode>> geos;
    std::vector<sk_sp<sksg::RenderNode>> draws;

    for (const auto& s : shapeArray) {
        const auto* info = FindShapeInfo(s);
        if (!info) {
            LogFail(s.isObject() ? s["ty"] : s, "Unknown shape");
            continue;
        }

        switch (info->fShapeType) {
        case ShapeType::kGeometry: {
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gGeometryAttachers));
            if (auto geo = gGeometryAttachers[info->fAttacherIndex](s, ctx)) {
                geos.push_back(std::move(geo));
            }
        } break;
        case ShapeType::kGeometryEffect: {
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gGeometryEffectAttachers));
            geos = gGeometryEffectAttachers[info->fAttacherIndex](s, ctx, std::move(geos));
        } break;
        case ShapeType::kPaint: {
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gPaintAttachers));
            if (auto paint = gPaintAttachers[info->fAttacherIndex](s, ctx)) {
                for (const auto& geo : geos) {
                    draws.push_back(sksg::Draw::Make(geo, paint));
                }
            }
        } break;
        case ShapeType::kGroup: {
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gGroupAttachers));
            if (auto group = gGroupAttachers[info->fAttacherIndex](s, ctx)) {
                draws.push_back(std::move(group));
            }
        } break;
        case ShapeType::kTransform: {
            // TODO: BM appears to transform the geometry, not the draw op itself.
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gTransformAttachers));
            xformed_group = gTransformAttachers[info->fAttacherIndex](s, ctx, xformed_group);
        } break;
        }
    }

    if (draws.empty()) {
        return nullptr;
    }

    for (auto draw = draws.rbegin(); draw != draws.rend(); ++draw) {
        shape_group->addChild(std::move(*draw));
    }

    LOG("** Attached shape: %zd draws.\n", draws.size());
    return xformed_group;
}

sk_sp<sksg::RenderNode> AttachCompLayer(const Json::Value& layer, AttachContext* ctx) {
    SkASSERT(layer.isObject());

    auto refId = ParseString(layer["refId"], "");
    if (refId.isEmpty()) {
        LOG("!! Comp layer missing refId\n");
        return nullptr;
    }

    const auto* comp = ctx->fAssets.find(refId);
    if (!comp) {
        LOG("!! Pre-comp not found: '%s'\n", refId.c_str());
        return nullptr;
    }

    // TODO: cycle detection
    return AttachComposition(**comp, ctx);
}

sk_sp<sksg::RenderNode> AttachSolidLayer(const Json::Value& layer, AttachContext*) {
    SkASSERT(layer.isObject());

    LOG("?? Solid layer stub\n");
    return nullptr;
}

sk_sp<sksg::RenderNode> AttachImageLayer(const Json::Value& layer, AttachContext*) {
    SkASSERT(layer.isObject());

    LOG("?? Image layer stub\n");
    return nullptr;
}

sk_sp<sksg::RenderNode> AttachNullLayer(const Json::Value& layer, AttachContext*) {
    SkASSERT(layer.isObject());

    LOG("?? Null layer stub\n");
    return nullptr;
}

sk_sp<sksg::RenderNode> AttachShapeLayer(const Json::Value& layer, AttachContext* ctx) {
    SkASSERT(layer.isObject());

    LOG("** Attaching shape layer ind: %d\n", ParseInt(layer["ind"], 0));

    return AttachShape(layer["shapes"], ctx);
}

sk_sp<sksg::RenderNode> AttachTextLayer(const Json::Value& layer, AttachContext*) {
    SkASSERT(layer.isObject());

    LOG("?? Text layer stub\n");
    return nullptr;
}

sk_sp<sksg::RenderNode> AttachLayer(const Json::Value& layer, AttachContext* ctx) {
    if (!layer.isObject())
        return nullptr;

    using LayerAttacher = sk_sp<sksg::RenderNode> (*)(const Json::Value&, AttachContext*);
    static constexpr LayerAttacher gLayerAttachers[] = {
        AttachCompLayer,  // 'ty': 0
        AttachSolidLayer, // 'ty': 1
        AttachImageLayer, // 'ty': 2
        AttachNullLayer,  // 'ty': 3
        AttachShapeLayer, // 'ty': 4
        AttachTextLayer,  // 'ty': 5
    };

    int type = ParseInt(layer["ty"], -1);
    if (type < 0 || type >= SkTo<int>(SK_ARRAY_COUNT(gLayerAttachers))) {
        return nullptr;
    }

    return AttachTransform(layer["ks"], ctx, gLayerAttachers[type](layer, ctx));
}

sk_sp<sksg::RenderNode> AttachComposition(const Json::Value& comp, AttachContext* ctx) {
    if (!comp.isObject())
        return nullptr;

    SkSTArray<16, sk_sp<sksg::RenderNode>, true> layers;

    for (const auto& l : comp["layers"]) {
        if (auto layer_fragment = AttachLayer(l, ctx)) {
            layers.push_back(std::move(layer_fragment));
        }
    }

    if (layers.empty()) {
        return nullptr;
    }

    // Layers are painted in bottom->top order.
    auto comp_group = sksg::Group::Make();
    for (int i = layers.count() - 1; i >= 0; --i) {
        comp_group->addChild(std::move(layers[i]));
    }

    LOG("** Attached composition '%s': %d layers.\n",
        ParseString(comp["id"], "").c_str(), layers.count());

    return comp_group;
}

} // namespace

std::unique_ptr<Animation> Animation::Make(SkStream* stream) {
    if (!stream->hasLength()) {
        // TODO: handle explicit buffering?
        LOG("!! cannot parse streaming content\n");
        return nullptr;
    }

    Json::Value json;
    {
        auto data = SkData::MakeFromStream(stream, stream->getLength());
        if (!data) {
            LOG("!! could not read stream\n");
            return nullptr;
        }

        Json::Reader reader;

        auto dataStart = static_cast<const char*>(data->data());
        if (!reader.parse(dataStart, dataStart + data->size(), json, false) || !json.isObject()) {
            LOG("!! failed to parse json: %s\n", reader.getFormattedErrorMessages().c_str());
            return nullptr;
        }
    }

    const auto version = ParseString(json["v"], "");
    const auto size    = SkSize::Make(ParseScalar(json["w"], -1), ParseScalar(json["h"], -1));
    const auto fps     = ParseScalar(json["fr"], -1);

    if (size.isEmpty() || version.isEmpty() || fps < 0) {
        LOG("!! invalid animation params (version: %s, size: [%f %f], frame rate: %f)",
            version.c_str(), size.width(), size.height(), fps);
        return nullptr;
    }

    return std::unique_ptr<Animation>(new Animation(std::move(version), size, fps, json));
}

Animation::Animation(SkString version, const SkSize& size, SkScalar fps, const Json::Value& json)
    : fVersion(std::move(version))
    , fSize(size)
    , fFrameRate(fps)
    , fInPoint(ParseScalar(json["ip"], 0))
    , fOutPoint(SkTMax(ParseScalar(json["op"], SK_ScalarMax), fInPoint)) {

    AssetMap assets;
    for (const auto& asset : json["assets"]) {
        if (!asset.isObject()) {
            continue;
        }

        assets.set(ParseString(asset["id"], ""), &asset);
    }

    AttachContext ctx = { assets, fAnimators };
    fDom = AttachComposition(json, &ctx);

    LOG("** Attached %d animators\n", fAnimators.count());
}

Animation::~Animation() = default;

void Animation::render(SkCanvas* canvas) const {
    if (!fDom)
        return;

    sksg::InvalidationController ic;
    fDom->revalidate(&ic, SkMatrix::I());

    // TODO: proper inval
    fDom->render(canvas);

    if (!fShowInval)
        return;

    SkPaint fill, stroke;
    fill.setAntiAlias(true);
    fill.setColor(0x40ff0000);
    stroke.setAntiAlias(true);
    stroke.setColor(0xffff0000);
    stroke.setStyle(SkPaint::kStroke_Style);

    for (const auto& r : ic) {
        canvas->drawRect(r, fill);
        canvas->drawRect(r, stroke);
    }
}

void Animation::animationTick(SkMSec ms) {
    // 't' in the BM model really means 'frame #'
    auto t = static_cast<float>(ms) * fFrameRate / 1000;

    t = fInPoint + std::fmod(t, fOutPoint - fInPoint);

    // TODO: this can be optimized quite a bit with some sorting/state tracking.
    for (const auto& a : fAnimators) {
        a->tick(t);
    }
}

} // namespace skotty
