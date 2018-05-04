/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieParser.h"

#include "SkJSONCPP.h"
#include "SkScalar.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkString.h"
#include "SkottieValue.h"

#include <vector>

namespace skottie {

template <>
bool Parse<SkScalar>(const Json::Value& jv, SkScalar* v) {
    // Some versions wrap values as single-element arrays.
    if (jv.isArray() && jv.size() == 1) {
        return Parse(jv[0], v);
    }

    if (jv.isNull() || !jv.isConvertibleTo(Json::realValue))
        return false;

    *v = jv.asFloat();

    return true;
}

template <>
bool Parse<bool>(const Json::Value& jv, bool* v) {
    if (jv.isNull() || !jv.isConvertibleTo(Json::booleanValue))
        return false;

    *v = jv.asBool();

    return true;
}

template <>
bool Parse<int>(const Json::Value& jv, int* v) {
    if (jv.isNull() || !jv.isConvertibleTo(Json::intValue))
        return false;

    *v = jv.asInt();

    return true;
}

template <>
bool Parse<SkString>(const Json::Value& jv, SkString* v) {
    if (jv.isNull() || !jv.isConvertibleTo(Json::stringValue))
        return false;

    v->set(jv.isString() ? jv.asCString() : jv.asString().c_str());

    return true;
}

template <>
bool Parse<SkPoint>(const Json::Value& jv, SkPoint* v) {
    if (!jv.isObject())
        return false;

    const auto& jvx = jv["x"],
                jvy = jv["y"];

    // Some BM versions seem to store x/y as single-element arrays.
    return Parse(jvx.isArray() ? jvx[0] : jvx, &v->fX)
        && Parse(jvy.isArray() ? jvy[0] : jvy, &v->fY);
}

template <>
bool Parse<std::vector<float>>(const Json::Value& jv, std::vector<float>* v) {
    if (!jv.isArray())
        return false;

    v->resize(jv.size());

    for (Json::ArrayIndex i = 0; i < jv.size(); ++i) {
        if (!Parse(jv[i], v->data() + i)) {
            return false;
        }
    }

    return true;
}

namespace {

bool ParsePointVec(const Json::Value& jv, std::vector<SkPoint>* pts) {
    if (!jv.isArray())
        return false;

    pts->clear();
    pts->reserve(jv.size());

    std::vector<float> vec;
    for (Json::ArrayIndex i = 0; i < jv.size(); ++i) {
        if (!Parse(jv[i], &vec) || vec.size() != 2)
            return false;
        pts->push_back(SkPoint::Make(vec[0], vec[1]));
    }

    return true;
}

} // namespace

template <>
bool Parse<ShapeValue>(const Json::Value& jv, ShapeValue* v) {
    SkASSERT(v->fVertices.empty());

    // Some versions wrap values as single-element arrays.
    if (jv.isArray() && jv.size() == 1) {
        return Parse(jv[0], v);
    }

    std::vector<SkPoint> inPts,  // Cubic Bezier "in" control points, relative to vertices.
                         outPts, // Cubic Bezier "out" control points, relative to vertices.
                         verts;  // Cubic Bezier vertices.

    if (!jv.isObject() ||
        !ParsePointVec(jv["i"], &inPts) ||
        !ParsePointVec(jv["o"], &outPts) ||
        !ParsePointVec(jv["v"], &verts) ||
        inPts.size() != outPts.size() ||
        inPts.size() != verts.size()) {

        return false;
    }

    v->fVertices.reserve(inPts.size());
    for (size_t i = 0; i < inPts.size(); ++i) {
        v->fVertices.push_back(BezierVertex({inPts[i], outPts[i], verts[i]}));
    }
    v->fClosed = ParseDefault(jv["c"], false);

    return true;
}

} // nasmespace skottie
