/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_surface.h"

#include "SkCanvas.h"
#include "SkImage.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkSurface.h"

static SkImageInfo make(const sk_imageinfo_t& cinfo) {
    return SkImageInfo::Make(cinfo.width, cinfo.height,
                             (SkColorType)cinfo.colorType, (SkAlphaType)cinfo.alphaType);
}

static const SkRect& AsRect(const sk_rect_t& crect) {
    return reinterpret_cast<const SkRect&>(crect);
}

static const SkPath& AsPath(const sk_path_t& cpath) {
    return reinterpret_cast<const SkPath&>(cpath);
}

static const SkImage* AsImage(const sk_image_t* cimage) {
    return reinterpret_cast<const SkImage*>(cimage);
}

static const SkPaint& AsPaint(const sk_paint_t& cpaint) {
    return reinterpret_cast<const SkPaint&>(cpaint);
}

static const SkPaint* AsPaint(const sk_paint_t* cpaint) {
    return reinterpret_cast<const SkPaint*>(cpaint);
}

static SkPaint* AsPaint(sk_paint_t* cpaint) {
    return reinterpret_cast<SkPaint*>(cpaint);
}

static SkCanvas* AsCanvas(sk_canvas_t* ccanvas) {
    return reinterpret_cast<SkCanvas*>(ccanvas);
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_image_t* sk_image_new_raster_copy(const sk_imageinfo_t* cinfo, const void* pixels,
                                     size_t rowBytes) {
    return (sk_image_t*)SkImage::NewRasterCopy(make(*cinfo), pixels, rowBytes);
}

void sk_image_ref(const sk_image_t* cimage) {
    AsImage(cimage)->ref();
}

void sk_image_unref(const sk_image_t* cimage) {
    AsImage(cimage)->unref();
}

int sk_image_get_width(const sk_image_t* cimage) {
    return AsImage(cimage)->width();
}

int sk_image_get_height(const sk_image_t* cimage) {
    return AsImage(cimage)->height();
}

uint32_t sk_image_get_unique_id(const sk_image_t* cimage) {
    return AsImage(cimage)->uniqueID();
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_paint_t* sk_paint_new() {
    return (sk_paint_t*)SkNEW(SkPaint);
}

void sk_paint_delete(sk_paint_t* cpaint) {
    SkDELETE(AsPaint(cpaint));
}

bool sk_paint_is_antialias(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).isAntiAlias();
}

void sk_paint_set_antialias(sk_paint_t* cpaint, bool aa) {
    AsPaint(cpaint)->setAntiAlias(aa);
}

sk_color_t sk_paint_get_color(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).getColor();
}

void sk_paint_set_color(sk_paint_t* cpaint, sk_color_t c) {
    AsPaint(cpaint)->setColor(c);
}

///////////////////////////////////////////////////////////////////////////////////////////

void sk_canvas_save(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->save();
}

void sk_canvas_save_layer(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRect(AsRect(*crect), AsPaint(*cpaint));
}

void sk_canvas_restore(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->restore();
}

void sk_canvas_translate(sk_canvas_t* ccanvas, float dx, float dy) {
    AsCanvas(ccanvas)->translate(dx, dy);
}

void sk_canvas_scale(sk_canvas_t* ccanvas, float sx, float sy) {
    AsCanvas(ccanvas)->scale(sx, sy);
}

void sk_canvas_draw_paint(sk_canvas_t* ccanvas, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPaint(AsPaint(*cpaint));
}

void sk_canvas_draw_rect(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRect(AsRect(*crect), AsPaint(*cpaint));
}

void sk_canvas_draw_oval(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawOval(AsRect(*crect), AsPaint(*cpaint));
}

void sk_canvas_draw_path(sk_canvas_t* ccanvas, const sk_path_t* cpath, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPath(AsPath(*cpath), AsPaint(*cpaint));
}

void sk_canvas_draw_image(sk_canvas_t* ccanvas, const sk_image_t* cimage, float x, float y,
                          const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawImage(AsImage(cimage), x, y, AsPaint(cpaint));
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_surface_t* sk_surface_new_raster(const sk_imageinfo_t* cinfo) {
    return (sk_surface_t*)SkSurface::NewRaster(make(*cinfo));
}

sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t* cinfo, void* pixels,
                                           size_t rowBytes) {
    return (sk_surface_t*)SkSurface::NewRasterDirect(make(*cinfo), pixels, rowBytes);
}

void sk_surface_delete(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    SkSafeUnref(surf);
}

sk_canvas_t* sk_surface_get_canvas(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    return (sk_canvas_t*)surf->getCanvas();
}

sk_image_t* sk_surface_new_image_snapshot(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    return (sk_image_t*)surf->newImageSnapshot();
}


///////////////////

void sk_test_capi(SkCanvas* canvas) {
    sk_imageinfo_t cinfo;
    cinfo.width = 100;
    cinfo.height = 100;
    cinfo.colorType = (sk_colortype_t)kN32_SkColorType;
    cinfo.alphaType = (sk_alphatype_t)kPremul_SkAlphaType;
    
    sk_surface_t* csurface = sk_surface_new_raster(&cinfo);
    sk_canvas_t* ccanvas = sk_surface_get_canvas(csurface);
    
    sk_paint_t* cpaint = sk_paint_new();
    sk_paint_set_antialias(cpaint, true);
    sk_paint_set_color(cpaint, 0xFFFF0000);
    
    sk_rect_t cr = { 5, 5, 95, 95 };
    sk_canvas_draw_oval(ccanvas, &cr, cpaint);
    
    cr.left += 25;
    cr.top += 25;
    cr.right -= 25;
    cr.bottom -= 25;
    sk_paint_set_color(cpaint, 0xFF00FF00);
    sk_canvas_draw_rect(ccanvas, &cr, cpaint);
    
    sk_image_t* cimage = sk_surface_new_image_snapshot(csurface);
    
    // HERE WE CROSS THE C..C++ boundary
    canvas->drawImage((const SkImage*)cimage, 20, 20, NULL);
    
    sk_paint_delete(cpaint);
    sk_image_unref(cimage);
    sk_surface_delete(csurface);
}
