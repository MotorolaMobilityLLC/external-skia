/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGPath.h"

namespace skottie {
namespace internal {

namespace  {

class PathAdapter final : public DiscardableAdapterBase<PathAdapter, sksg::Path> {
public:
    PathAdapter(const skjson::Value& jpath, const AnimationBuilder& abuilder)
        : INHERITED(sksg::Path::Make()) {
        this->bind(abuilder, jpath, fShape);
    }

private:
    void onSync() override {
        const auto& path_node = this->node();

        auto path = ValueTraits<ShapeValue>::As<SkPath>(fShape);

        // FillType is tracked in the SG node, not in keyframes -- make sure we preserve it.
        path.setFillType(path_node->getFillType());
        path_node->setPath(path);
    }

    ShapeValue fShape;

    using INHERITED = DiscardableAdapterBase<PathAdapter, sksg::Path>;
};

} // namespace

sk_sp<sksg::Path> AnimationBuilder::attachPath(const skjson::Value& jpath) const {
    return this->attachDiscardableAdapter<PathAdapter, sk_sp<sksg::Path>>(jpath, *this);
}

} // namespace internal
} // namespace skottie
