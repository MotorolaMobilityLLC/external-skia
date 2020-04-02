/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVerticesPriv_DEFINED
#define SkVerticesPriv_DEFINED

#include "include/core/SkVertices.h"

struct SkVertices_DeprecatedBoneIndices { uint32_t values[4]; };
struct SkVertices_DeprecatedBoneWeights {    float values[4]; };
struct SkVertices_DeprecatedBone        {    float values[6]; };

/** Class that adds methods to SkVertices that are only intended for use internal to Skia.
    This class is purely a privileged window into SkVertices. It should never have additional
    data members or virtual methods. */
class SkVerticesPriv {
public:
    SkVertices::VertexMode mode() const { return fVertices->fMode; }

    bool hasPerVertexData() const { return SkToBool(fVertices->fPerVertexData); }
    bool hasColors() const { return SkToBool(fVertices->fColors); }
    bool hasTexCoords() const { return SkToBool(fVertices->fTexs); }
    bool hasIndices() const { return SkToBool(fVertices->fIndices); }

    int vertexCount() const { return fVertices->fVertexCount; }
    int indexCount() const { return fVertices->fIndexCount; }
    int perVertexDataCount() const { return fVertices->fPerVertexDataCount; }

    const SkPoint* positions() const { return fVertices->fPositions; }
    const float* perVertexData() const { return fVertices->fPerVertexData; }
    const SkPoint* texCoords() const { return fVertices->fTexs; }
    const SkColor* colors() const { return fVertices->fColors; }
    const uint16_t* indices() const { return fVertices->fIndices; }

    // Never called due to RVO in priv(), but must exist for MSVC 2017.
    SkVerticesPriv(const SkVerticesPriv&) = default;

private:
    explicit SkVerticesPriv(SkVertices* vertices) : fVertices(vertices) {}
    SkVerticesPriv& operator=(const SkVerticesPriv&) = delete;

    // No taking addresses of this type
    const SkVerticesPriv* operator&() const = delete;
    SkVerticesPriv* operator&() = delete;

    SkVertices* fVertices;

    friend class SkVertices; // to construct this type
};

inline SkVerticesPriv SkVertices::priv() { return SkVerticesPriv(this); }

inline const SkVerticesPriv SkVertices::priv() const {
    return SkVerticesPriv(const_cast<SkVertices*>(this));
}

#endif
