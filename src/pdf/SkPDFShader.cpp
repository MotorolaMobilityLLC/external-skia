/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFShader.h"

#include "SkData.h"
#include "SkPDFCanon.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"
#include "SkPDFFormXObject.h"
#include "SkPDFGradientShader.h"
#include "SkPDFGraphicState.h"
#include "SkPDFResourceDict.h"
#include "SkPDFUtils.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTemplates.h"


static void draw_bitmap_matrix(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& matrix) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(matrix);
    canvas->drawBitmap(bm, 0, 0);
}

static sk_sp<SkPDFStream> make_image_shader(SkPDFDocument* doc,
                                            const SkPDFShader::State& state,
                                            SkBitmap image) {
    SkASSERT(state.fBitmapKey ==
             (SkBitmapKey{image.getSubset(), image.getGenerationID()}));

    // The image shader pattern cell will be drawn into a separate device
    // in pattern cell space (no scaling on the bitmap, though there may be
    // translations so that all content is in the device, coordinates > 0).

    // Map clip bounds to shader space to ensure the device is large enough
    // to handle fake clamping.
    SkMatrix finalMatrix = state.fCanvasTransform;
    finalMatrix.preConcat(state.fShaderTransform);
    SkRect deviceBounds;
    deviceBounds.set(state.fBBox);
    if (!SkPDFUtils::InverseTransformBBox(finalMatrix, &deviceBounds)) {
        return nullptr;
    }

    SkRect bitmapBounds;
    image.getBounds(&bitmapBounds);

    // For tiling modes, the bounds should be extended to include the bitmap,
    // otherwise the bitmap gets clipped out and the shader is empty and awful.
    // For clamp modes, we're only interested in the clip region, whether
    // or not the main bitmap is in it.
    SkShader::TileMode tileModes[2];
    tileModes[0] = state.fImageTileModes[0];
    tileModes[1] = state.fImageTileModes[1];
    if (tileModes[0] != SkShader::kClamp_TileMode ||
            tileModes[1] != SkShader::kClamp_TileMode) {
        deviceBounds.join(bitmapBounds);
    }

    SkISize size = SkISize::Make(SkScalarRoundToInt(deviceBounds.width()),
                                 SkScalarRoundToInt(deviceBounds.height()));
    auto patternDevice = sk_make_sp<SkPDFDevice>(size, doc);
    SkCanvas canvas(patternDevice.get());

    SkRect patternBBox;
    image.getBounds(&patternBBox);

    // Translate the canvas so that the bitmap origin is at (0, 0).
    canvas.translate(-deviceBounds.left(), -deviceBounds.top());
    patternBBox.offset(-deviceBounds.left(), -deviceBounds.top());
    // Undo the translation in the final matrix
    finalMatrix.preTranslate(deviceBounds.left(), deviceBounds.top());

    // If the bitmap is out of bounds (i.e. clamp mode where we only see the
    // stretched sides), canvas will clip this out and the extraneous data
    // won't be saved to the PDF.
    canvas.drawBitmap(image, 0, 0);

    SkScalar width = SkIntToScalar(image.width());
    SkScalar height = SkIntToScalar(image.height());

    // Tiling is implied.  First we handle mirroring.
    if (tileModes[0] == SkShader::kMirror_TileMode) {
        SkMatrix xMirror;
        xMirror.setScale(-1, 1);
        xMirror.postTranslate(2 * width, 0);
        draw_bitmap_matrix(&canvas, image, xMirror);
        patternBBox.fRight += width;
    }
    if (tileModes[1] == SkShader::kMirror_TileMode) {
        SkMatrix yMirror;
        yMirror.setScale(SK_Scalar1, -SK_Scalar1);
        yMirror.postTranslate(0, 2 * height);
        draw_bitmap_matrix(&canvas, image, yMirror);
        patternBBox.fBottom += height;
    }
    if (tileModes[0] == SkShader::kMirror_TileMode &&
            tileModes[1] == SkShader::kMirror_TileMode) {
        SkMatrix mirror;
        mirror.setScale(-1, -1);
        mirror.postTranslate(2 * width, 2 * height);
        draw_bitmap_matrix(&canvas, image, mirror);
    }

    // Then handle Clamping, which requires expanding the pattern canvas to
    // cover the entire surfaceBBox.

    // If both x and y are in clamp mode, we start by filling in the corners.
    // (Which are just a rectangles of the corner colors.)
    if (tileModes[0] == SkShader::kClamp_TileMode &&
            tileModes[1] == SkShader::kClamp_TileMode) {
        SkPaint paint;
        SkRect rect;
        rect = SkRect::MakeLTRB(deviceBounds.left(), deviceBounds.top(), 0, 0);
        if (!rect.isEmpty()) {
            paint.setColor(image.getColor(0, 0));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(width, deviceBounds.top(),
                                deviceBounds.right(), 0);
        if (!rect.isEmpty()) {
            paint.setColor(image.getColor(image.width() - 1, 0));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(width, height,
                                deviceBounds.right(), deviceBounds.bottom());
        if (!rect.isEmpty()) {
            paint.setColor(image.getColor(image.width() - 1,
                                           image.height() - 1));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(deviceBounds.left(), height,
                                0, deviceBounds.bottom());
        if (!rect.isEmpty()) {
            paint.setColor(image.getColor(0, image.height() - 1));
            canvas.drawRect(rect, paint);
        }
    }

    // Then expand the left, right, top, then bottom.
    if (tileModes[0] == SkShader::kClamp_TileMode) {
        SkIRect subset = SkIRect::MakeXYWH(0, 0, 1, image.height());
        if (deviceBounds.left() < 0) {
            SkBitmap left;
            SkAssertResult(image.extractSubset(&left, subset));

            SkMatrix leftMatrix;
            leftMatrix.setScale(-deviceBounds.left(), 1);
            leftMatrix.postTranslate(deviceBounds.left(), 0);
            draw_bitmap_matrix(&canvas, left, leftMatrix);

            if (tileModes[1] == SkShader::kMirror_TileMode) {
                leftMatrix.postScale(SK_Scalar1, -SK_Scalar1);
                leftMatrix.postTranslate(0, 2 * height);
                draw_bitmap_matrix(&canvas, left, leftMatrix);
            }
            patternBBox.fLeft = 0;
        }

        if (deviceBounds.right() > width) {
            SkBitmap right;
            subset.offset(image.width() - 1, 0);
            SkAssertResult(image.extractSubset(&right, subset));

            SkMatrix rightMatrix;
            rightMatrix.setScale(deviceBounds.right() - width, 1);
            rightMatrix.postTranslate(width, 0);
            draw_bitmap_matrix(&canvas, right, rightMatrix);

            if (tileModes[1] == SkShader::kMirror_TileMode) {
                rightMatrix.postScale(SK_Scalar1, -SK_Scalar1);
                rightMatrix.postTranslate(0, 2 * height);
                draw_bitmap_matrix(&canvas, right, rightMatrix);
            }
            patternBBox.fRight = deviceBounds.width();
        }
    }

    if (tileModes[1] == SkShader::kClamp_TileMode) {
        SkIRect subset = SkIRect::MakeXYWH(0, 0, image.width(), 1);
        if (deviceBounds.top() < 0) {
            SkBitmap top;
            SkAssertResult(image.extractSubset(&top, subset));

            SkMatrix topMatrix;
            topMatrix.setScale(SK_Scalar1, -deviceBounds.top());
            topMatrix.postTranslate(0, deviceBounds.top());
            draw_bitmap_matrix(&canvas, top, topMatrix);

            if (tileModes[0] == SkShader::kMirror_TileMode) {
                topMatrix.postScale(-1, 1);
                topMatrix.postTranslate(2 * width, 0);
                draw_bitmap_matrix(&canvas, top, topMatrix);
            }
            patternBBox.fTop = 0;
        }

        if (deviceBounds.bottom() > height) {
            SkBitmap bottom;
            subset.offset(0, image.height() - 1);
            SkAssertResult(image.extractSubset(&bottom, subset));

            SkMatrix bottomMatrix;
            bottomMatrix.setScale(SK_Scalar1, deviceBounds.bottom() - height);
            bottomMatrix.postTranslate(0, height);
            draw_bitmap_matrix(&canvas, bottom, bottomMatrix);

            if (tileModes[0] == SkShader::kMirror_TileMode) {
                bottomMatrix.postScale(-1, 1);
                bottomMatrix.postTranslate(2 * width, 0);
                draw_bitmap_matrix(&canvas, bottom, bottomMatrix);
            }
            patternBBox.fBottom = deviceBounds.height();
        }
    }

    auto imageShader = sk_make_sp<SkPDFStream>(patternDevice->content());
    SkPDFUtils::PopulateTilingPatternDict(imageShader->dict(), patternBBox,
                                 patternDevice->makeResourceDict(), finalMatrix);
    return imageShader;
}

// Generic fallback for unsupported shaders:
//  * allocate a surfaceBBox-sized bitmap
//  * shade the whole area
//  * use the result as a bitmap shader
static sk_sp<SkPDFObject> make_fallback_shader(SkPDFDocument* doc,
                                               SkShader* shader,
                                               const SkMatrix& canvasTransform,
                                               const SkIRect& surfaceBBox) {
    // TODO(vandebo) This drops SKComposeShader on the floor.  We could
    // handle compose shader by pulling things up to a layer, drawing with
    // the first shader, applying the xfer mode and drawing again with the
    // second shader, then applying the layer to the original drawing.
    SkPDFShader::State state = {
        canvasTransform,
        SkMatrix::I(),
        surfaceBBox,
        {{0, 0, 0, 0}, 0},
        {SkShader::kClamp_TileMode, SkShader::kClamp_TileMode}};

    state.fShaderTransform = shader->getLocalMatrix();

    // surfaceBBox is in device space. While that's exactly what we
    // want for sizing our bitmap, we need to map it into
    // shader space for adjustments (to match
    // MakeImageShader's behavior).
    SkRect shaderRect = SkRect::Make(surfaceBBox);
    if (!SkPDFUtils::InverseTransformBBox(canvasTransform, &shaderRect)) {
        return nullptr;
    }
    // Clamp the bitmap size to about 1M pixels
    static const SkScalar kMaxBitmapArea = 1024 * 1024;
    SkScalar rasterScale = SkIntToScalar(doc->rasterDpi()) / SkPDFUtils::kDpiForRasterScaleOne;
    SkScalar bitmapArea = rasterScale * surfaceBBox.width() * rasterScale * surfaceBBox.height();
    if (bitmapArea > kMaxBitmapArea) {
        rasterScale *= SkScalarSqrt(kMaxBitmapArea / bitmapArea);
    }

    SkISize size = {SkScalarRoundToInt(rasterScale * surfaceBBox.width()),
                    SkScalarRoundToInt(rasterScale * surfaceBBox.height())};
    SkSize scale = {SkIntToScalar(size.width()) / shaderRect.width(),
                    SkIntToScalar(size.height()) / shaderRect.height()};

    SkBitmap image;
    image.allocN32Pixels(size.width(), size.height());
    image.eraseColor(SK_ColorTRANSPARENT);

    SkPaint p;
    p.setShader(sk_ref_sp(shader));

    SkCanvas canvas(image);
    canvas.scale(scale.width(), scale.height());
    canvas.translate(-shaderRect.x(), -shaderRect.y());
    canvas.drawPaint(p);

    state.fShaderTransform.setTranslate(shaderRect.x(), shaderRect.y());
    state.fShaderTransform.preScale(1 / scale.width(), 1 / scale.height());
    state.fBitmapKey = SkBitmapKey{image.getSubset(), image.getGenerationID()};
    SkASSERT (!image.isNull());
    return make_image_shader(doc, state, std::move(image));
}

sk_sp<SkPDFObject> SkPDFShader::GetPDFShader(SkPDFDocument* doc,
                                             SkShader* shader,
                                             const SkMatrix& canvasTransform,
                                             const SkIRect& surfaceBBox) {
    SkASSERT(shader);
    SkASSERT(doc);
    if (SkShader::kNone_GradientType != shader->asAGradient(nullptr)) {
        return SkPDFGradientShader::Make(doc, shader, canvasTransform, surfaceBBox);
    }
    if (surfaceBBox.isEmpty()) {
        return nullptr;
    }
    SkBitmap image;
    SkPDFShader::State state = {
        canvasTransform,
        SkMatrix::I(),
        surfaceBBox,
        {{0, 0, 0, 0}, 0},
        {SkShader::kClamp_TileMode, SkShader::kClamp_TileMode}};

    SkASSERT(shader->asAGradient(nullptr) == SkShader::kNone_GradientType) ;
    SkImage* skimg;
    if ((skimg = shader->isAImage(&state.fShaderTransform, state.fImageTileModes))
            && skimg->asLegacyBitmap(&image, SkImage::kRO_LegacyBitmapMode)) {
        // TODO(halcanary): delay converting to bitmap.
        state.fBitmapKey = SkBitmapKey{image.getSubset(), image.getGenerationID()};
        if (image.isNull()) {
            return nullptr;
        }
        SkPDFCanon* canon = doc->canon();
        sk_sp<SkPDFObject>* shaderPtr = canon->fImageShaderMap.find(state);
        if (shaderPtr) {
            return *shaderPtr;
        }
        sk_sp<SkPDFObject> pdfShader = make_image_shader(doc, state, std::move(image));
        canon->fImageShaderMap.set(std::move(state), pdfShader);
        return pdfShader;
    }
    // Don't bother to de-dup fallback shader.
    return make_fallback_shader(doc, shader, canvasTransform, surfaceBBox);
}
