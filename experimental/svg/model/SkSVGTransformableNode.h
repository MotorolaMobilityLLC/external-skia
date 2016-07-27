/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGTransformableNode_DEFINED
#define SkSVGTransformableNode_DEFINED

#include "SkMatrix.h"
#include "SkSVGNode.h"

class SkSVGTransformableNode : public SkSVGNode {
public:
    virtual ~SkSVGTransformableNode() = default;

    void setTransform(const SkMatrix& m) { fMatrix = m; }

protected:
    SkSVGTransformableNode(SkSVGTag);

    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    const SkMatrix& onLocalMatrix() const override { return fMatrix; }

private:
    // FIXME: should be sparse
    SkMatrix fMatrix;

    typedef SkSVGNode INHERITED;
};

#endif // SkSVGTransformableNode_DEFINED
