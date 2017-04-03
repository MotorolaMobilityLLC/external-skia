/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkColorSpaceXformer.h"
#include "SkGradientShader.h"
#include "SkImage_Base.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkSurface.h"

class SkColorSpaceXformCanvas : public SkNoDrawCanvas {
public:
    SkColorSpaceXformCanvas(SkCanvas* target,
                            std::unique_ptr<SkColorSpaceXformer> xformer)
        : SkNoDrawCanvas(SkIRect::MakeSize(target->getBaseLayerSize()))
        , fTarget(target)
        , fXformer(std::move(xformer))
    {
        // Set the matrix and clip to match |fTarget|.  Otherwise, we'll answer queries for
        // bounds/matrix differently than |fTarget| would.
        SkCanvas::onClipRect(SkRect::MakeFromIRect(fTarget->getDeviceClipBounds()),
                             SkClipOp::kIntersect, kHard_ClipEdgeStyle);
        SkCanvas::setMatrix(fTarget->getTotalMatrix());
    }

    SkImageInfo onImageInfo() const override {
        return fTarget->imageInfo();
    }

    void onDrawPaint(const SkPaint& paint) override {
        fTarget->drawPaint(fXformer->apply(paint));
    }

    void onDrawRect(const SkRect& rect, const SkPaint& paint) override {
        fTarget->drawRect(rect, fXformer->apply(paint));
    }
    void onDrawOval(const SkRect& oval, const SkPaint& paint) override {
        fTarget->drawOval(oval, fXformer->apply(paint));
    }
    void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override {
        fTarget->drawRRect(rrect, fXformer->apply(paint));
    }
    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override {
        fTarget->drawDRRect(outer, inner, fXformer->apply(paint));
    }
    void onDrawPath(const SkPath& path, const SkPaint& paint) override {
        fTarget->drawPath(path, fXformer->apply(paint));
    }
    void onDrawArc(const SkRect& oval, SkScalar start, SkScalar sweep, bool useCenter,
                   const SkPaint& paint) override {
        fTarget->drawArc(oval, start, sweep, useCenter, fXformer->apply(paint));
    }
    void onDrawRegion(const SkRegion& region, const SkPaint& paint) override {
        fTarget->drawRegion(region, fXformer->apply(paint));
    }
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4], const SkPoint texs[4],
                     SkBlendMode mode, const SkPaint& paint) override {
        SkColor xformed[4];
        if (colors) {
            fXformer->apply(xformed, colors, 4);
            colors = xformed;
        }

        fTarget->drawPatch(cubics, colors, texs, mode, fXformer->apply(paint));
    }
    void onDrawPoints(PointMode mode, size_t count, const SkPoint* pts,
                      const SkPaint& paint) override {
        fTarget->drawPoints(mode, count, pts, fXformer->apply(paint));
    }
    void onDrawVerticesObject(const SkVertices* vertices, SkBlendMode mode,
                              const SkPaint& paint) override {
        sk_sp<SkVertices> copy;
        if (vertices->hasColors()) {
            int count = vertices->vertexCount();
            SkSTArray<8, SkColor> xformed(count);
            fXformer->apply(xformed.begin(), vertices->colors(), count);
            copy = SkVertices::MakeCopy(vertices->mode(), count, vertices->positions(),
                                        vertices->texCoords(), xformed.begin(),
                                        vertices->indexCount(), vertices->indices());
            vertices = copy.get();
        }

        fTarget->drawVertices(vertices, mode, fXformer->apply(paint));
    }

    void onDrawText(const void* ptr, size_t len,
                    SkScalar x, SkScalar y,
                    const SkPaint& paint) override {
        fTarget->drawText(ptr, len, x, y, fXformer->apply(paint));
    }
    void onDrawPosText(const void* ptr, size_t len,
                       const SkPoint* xys,
                       const SkPaint& paint) override {
        fTarget->drawPosText(ptr, len, xys, fXformer->apply(paint));
    }
    void onDrawPosTextH(const void* ptr, size_t len,
                        const SkScalar* xs, SkScalar y,
                        const SkPaint& paint) override {
        fTarget->drawPosTextH(ptr, len, xs, y, fXformer->apply(paint));
    }
    void onDrawTextOnPath(const void* ptr, size_t len,
                          const SkPath& path, const SkMatrix* matrix,
                          const SkPaint& paint) override {
        fTarget->drawTextOnPath(ptr, len, path, matrix, fXformer->apply(paint));
    }
    void onDrawTextRSXform(const void* ptr, size_t len,
                           const SkRSXform* xforms, const SkRect* cull,
                           const SkPaint& paint) override {
        fTarget->drawTextRSXform(ptr, len, xforms, cull, fXformer->apply(paint));
    }
    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x, SkScalar y,
                        const SkPaint& paint) override {
        fTarget->drawTextBlob(blob, x, y, fXformer->apply(paint));
    }

    void onDrawImage(const SkImage* img,
                     SkScalar l, SkScalar t,
                     const SkPaint* paint) override {
        fTarget->drawImage(fXformer->apply(img).get(),
                           l, t,
                           fXformer->apply(paint));
    }
    void onDrawImageRect(const SkImage* img,
                         const SkRect* src, const SkRect& dst,
                         const SkPaint* paint, SrcRectConstraint constraint) override {
        fTarget->drawImageRect(fXformer->apply(img).get(),
                               src ? *src : dst, dst,
                               fXformer->apply(paint), constraint);
    }
    void onDrawImageNine(const SkImage* img,
                         const SkIRect& center, const SkRect& dst,
                         const SkPaint* paint) override {
        fTarget->drawImageNine(fXformer->apply(img).get(),
                               center, dst,
                               fXformer->apply(paint));
    }
    void onDrawImageLattice(const SkImage* img,
                            const Lattice& lattice, const SkRect& dst,
                            const SkPaint* paint) override {
        fTarget->drawImageLattice(fXformer->apply(img).get(),
                                  lattice, dst,
                                  fXformer->apply(paint));
    }
    void onDrawAtlas(const SkImage* atlas, const SkRSXform* xforms, const SkRect* tex,
                     const SkColor* colors, int count, SkBlendMode mode,
                     const SkRect* cull, const SkPaint* paint) override {
        SkSTArray<8, SkColor> xformed;
        if (colors) {
            xformed.reset(count);
            fXformer->apply(xformed.begin(), colors, count);
            colors = xformed.begin();
        }

        fTarget->drawAtlas(fXformer->apply(atlas).get(), xforms, tex, colors, count, mode, cull,
                           fXformer->apply(paint));
    }

    void onDrawBitmap(const SkBitmap& bitmap,
                      SkScalar l, SkScalar t,
                      const SkPaint* paint) override {
        if (auto image = SkImage::MakeFromBitmap(bitmap)) {
            this->onDrawImage(image.get(), l, t, paint);
        }
    }
    void onDrawBitmapRect(const SkBitmap& bitmap,
                          const SkRect* src, const SkRect& dst,
                          const SkPaint* paint, SrcRectConstraint constraint) override {
        if (auto image = SkImage::MakeFromBitmap(bitmap)) {
            this->onDrawImageRect(image.get(), src, dst, paint, constraint);
        }
    }
    void onDrawBitmapNine(const SkBitmap& bitmap,
                          const SkIRect& center, const SkRect& dst,
                          const SkPaint* paint) override {
        if (auto image = SkImage::MakeFromBitmap(bitmap)) {
            this->onDrawImageNine(image.get(), center, dst, paint);
        }
    }
    void onDrawBitmapLattice(const SkBitmap& bitmap,
                             const Lattice& lattice, const SkRect& dst,
                             const SkPaint* paint) override {
        if (auto image = SkImage::MakeFromBitmap(bitmap)) {
            this->onDrawImageLattice(image.get(), lattice, dst, paint);
        }
    }

    // TODO: May not be ideal to unfurl pictures.
    void onDrawPicture(const SkPicture* pic,
                       const SkMatrix* matrix,
                       const SkPaint* paint) override {
        SkCanvas::onDrawPicture(pic, matrix, fXformer->apply(paint));
    }
    void onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) override {
        SkCanvas::onDrawDrawable(drawable, matrix);
    }

    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
        fTarget->saveLayer({
            rec.fBounds,
            fXformer->apply(rec.fPaint),
            rec.fBackdrop,  // TODO: this is an image filter
            rec.fSaveLayerFlags,
        });
        return kNoLayer_SaveLayerStrategy;
    }

    // Everything from here on should be uninteresting strictly proxied state-change calls.
    void willSave()    override { fTarget->save(); }
    void willRestore() override { fTarget->restore(); }

    void didConcat   (const SkMatrix& m) override { fTarget->concat   (m); }
    void didSetMatrix(const SkMatrix& m) override { fTarget->setMatrix(m); }

    void onClipRect(const SkRect& clip, SkClipOp op, ClipEdgeStyle style) override {
        SkCanvas::onClipRect(clip, op, style);
        fTarget->clipRect(clip, op, style);
    }
    void onClipRRect(const SkRRect& clip, SkClipOp op, ClipEdgeStyle style) override {
        SkCanvas::onClipRRect(clip, op, style);
        fTarget->clipRRect(clip, op, style);
    }
    void onClipPath(const SkPath& clip, SkClipOp op, ClipEdgeStyle style) override {
        SkCanvas::onClipPath(clip, op, style);
        fTarget->clipPath(clip, op, style);
    }
    void onClipRegion(const SkRegion& clip, SkClipOp op) override {
        SkCanvas::onClipRegion(clip, op);
        fTarget->clipRegion(clip, op);
    }

    void onDrawAnnotation(const SkRect& rect, const char* key, SkData* val) override {
        fTarget->drawAnnotation(rect, key, val);
    }

    sk_sp<SkSurface> onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) override {
        return fTarget->makeSurface(info, &props);
    }

private:
    SkCanvas*                            fTarget;
    std::unique_ptr<SkColorSpaceXformer> fXformer;
};

std::unique_ptr<SkCanvas> SkCreateColorSpaceXformCanvas(SkCanvas* target,
                                                        sk_sp<SkColorSpace> targetCS) {
    std::unique_ptr<SkColorSpaceXformer> xformer = SkColorSpaceXformer::Make(std::move(targetCS));
    if (!xformer) {
        return nullptr;
    }

    return skstd::make_unique<SkColorSpaceXformCanvas>(target, std::move(xformer));
}
