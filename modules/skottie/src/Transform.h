/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTransform_DEFINED
#define SkottieTransform_DEFINED

#include "modules/skottie/src/SkottieAdapter.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkMatrix44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "modules/skottie/src/Adapter.h"

namespace skjson {

class ObjectValue;

} // namespace skjson

namespace skottie {
namespace internal {

class TransformAdapter2D final : public DiscardableAdapterBase<TransformAdapter2D,
                                                               sksg::Matrix<SkMatrix>> {
public:
    TransformAdapter2D(const AnimationBuilder&,
                       const skjson::ObjectValue* janchor_point,
                       const skjson::ObjectValue* jposition,
                       const skjson::ObjectValue* jscale,
                       const skjson::ObjectValue* jrotation,
                       const skjson::ObjectValue* jskew,
                       const skjson::ObjectValue* jskew_axis);
    ~TransformAdapter2D() override;

    // Accessors needed for public property APIs.
    // TODO: introduce a separate public type.
    SkPoint getAnchorPoint() const;
    void    setAnchorPoint(const SkPoint&);

    SkPoint getPosition() const;
    void    setPosition(const SkPoint&);

    SkVector getScale() const;
    void     setScale(const SkVector&);

    float getRotation() const  { return fRotation; }
    void  setRotation(float r);

    float getSkew() const   { return fSkew; }
    void  setSkew(float sk);

    float getSkewAxis() const    { return fSkewAxis; }
    void  setSkewAxis(float sa );

    SkMatrix totalMatrix() const;

private:
    void onSync() override;

    VectorValue fAnchorPoint,
                fPosition,
                fScale    = { 100, 100 };
    ScalarValue fRotation = 0,
                fSkew     = 0,
                fSkewAxis = 0;

    using INHERITED = DiscardableAdapterBase<TransformAdapter2D, sksg::Matrix<SkMatrix>>;
};

class TransformAdapter3D : public DiscardableAdapterBase<TransformAdapter3D,
                                                         sksg::Matrix<SkMatrix44>> {
public:
    TransformAdapter3D(const skjson::ObjectValue&, const AnimationBuilder&);
    ~TransformAdapter3D() override;

    virtual SkMatrix44 totalMatrix() const;

protected:
    SkPoint3  anchor_point() const;
    SkPoint3  position() const;
    SkVector3 rotation() const;

private:
    void onSync() final;

    VectorValue fAnchorPoint,
                fPosition,
                fOrientation,
                fScale = { 100, 100, 100 };
    ScalarValue fRx = 0,
                fRy = 0,
                fRz = 0;

    using INHERITED = DiscardableAdapterBase<TransformAdapter3D, sksg::Matrix<SkMatrix44>>;
};

} // namespace internal
} // namespace skottie

#endif // SkottieTransform_DEFINED
