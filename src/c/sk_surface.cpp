/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_canvas.h"
#include "sk_image.h"
#include "sk_paint.h"
#include "sk_path.h"
#include "sk_surface.h"

#include "SkCanvas.h"
#include "SkImage.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkSurface.h"

const struct {
    sk_colortype_t  fC;
    SkColorType     fSK;
} gColorTypeMap[] = {
    { UNKNOWN_SK_COLORTYPE,     kUnknown_SkColorType    },
    { RGBA_8888_SK_COLORTYPE,   kRGBA_8888_SkColorType  },
    { BGRA_8888_SK_COLORTYPE,   kBGRA_8888_SkColorType  },
    { ALPHA_8_SK_COLORTYPE,     kAlpha_8_SkColorType    },
};

const struct {
    sk_alphatype_t  fC;
    SkAlphaType     fSK;
} gAlphaTypeMap[] = {
    { OPAQUE_SK_ALPHATYPE,      kOpaque_SkAlphaType     },
    { PREMUL_SK_ALPHATYPE,      kPremul_SkAlphaType     },
    { UNPREMUL_SK_ALPHATYPE,    kUnpremul_SkAlphaType   },
};

static bool from_c_colortype(sk_colortype_t cCT, SkColorType* skCT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gColorTypeMap); ++i) {
        if (gColorTypeMap[i].fC == cCT) {
            if (skCT) {
                *skCT = gColorTypeMap[i].fSK;
            }
            return true;
        }
    }
    return false;
}

static bool to_c_colortype(SkColorType skCT, sk_colortype_t* cCT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gColorTypeMap); ++i) {
        if (gColorTypeMap[i].fSK == skCT) {
            if (cCT) {
                *cCT = gColorTypeMap[i].fC;
            }
            return true;
        }
    }
    return false;
}

static bool from_c_alphatype(sk_alphatype_t cAT, SkAlphaType* skAT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gAlphaTypeMap); ++i) {
        if (gAlphaTypeMap[i].fC == cAT) {
            if (skAT) {
                *skAT = gAlphaTypeMap[i].fSK;
            }
            return true;
        }
    }
    return false;
}

static bool from_c_info(const sk_imageinfo_t& cinfo, SkImageInfo* info) {
    SkColorType ct;
    SkAlphaType at;

    if (!from_c_colortype(cinfo.colorType, &ct)) {
        // optionally report error to client?
        return false;
    }
    if (!from_c_alphatype(cinfo.alphaType, &at)) {
        // optionally report error to client?
        return false;
    }
    if (info) {
        *info = SkImageInfo::Make(cinfo.width, cinfo.height, ct, at);
    }
    return true;
}

const struct {
    sk_path_direction_t fC;
    SkPath::Direction   fSk;
} gPathDirMap[] = {
    { CW_SK_PATH_DIRECTION,  SkPath::kCW_Direction },
    { CCW_SK_PATH_DIRECTION, SkPath::kCCW_Direction },
};

static bool from_c_path_direction(sk_path_direction_t cdir, SkPath::Direction* dir) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPathDirMap); ++i) {
        if (gPathDirMap[i].fC == cdir) {
            if (dir) {
                *dir = gPathDirMap[i].fSk;
            }
            return true;
        }
    }
    return false;
}

static const SkRect& AsRect(const sk_rect_t& crect) {
    return reinterpret_cast<const SkRect&>(crect);
}

static SkRect* as_rect(sk_rect_t* crect) {
    return reinterpret_cast<SkRect*>(crect);
}

static const SkPath& AsPath(const sk_path_t& cpath) {
    return reinterpret_cast<const SkPath&>(cpath);
}

static SkPath* as_path(sk_path_t* cpath) {
    return reinterpret_cast<SkPath*>(cpath);
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

static SkShader* AsShader(sk_shader_t* cshader) {
    return reinterpret_cast<SkShader*>(cshader);
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_colortype_t sk_colortype_get_default_8888() {
    sk_colortype_t ct;
    if (!to_c_colortype(kN32_SkColorType, &ct)) {
        ct = UNKNOWN_SK_COLORTYPE;
    }
    return ct;
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_image_t* sk_image_new_raster_copy(const sk_imageinfo_t* cinfo, const void* pixels,
                                     size_t rowBytes) {
    SkImageInfo info;
    if (!from_c_info(*cinfo, &info)) {
        return NULL;
    }
    return (sk_image_t*)SkImage::NewRasterCopy(info, pixels, rowBytes);
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

void sk_paint_set_shader(sk_paint_t* cpaint, sk_shader_t* cshader) {
    AsPaint(cpaint)->setShader(AsShader(cshader));
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_path_t* sk_path_new() {
    return (sk_path_t*)SkNEW(SkPath);
}

void sk_path_delete(sk_path_t* cpath) {
    SkDELETE(as_path(cpath));
}

void sk_path_move_to(sk_path_t* cpath, float x, float y) {
    as_path(cpath)->moveTo(x, y);
}

void sk_path_line_to(sk_path_t* cpath, float x, float y) {
    as_path(cpath)->lineTo(x, y);
}

void sk_path_quad_to(sk_path_t* cpath, float x0, float y0, float x1, float y1) {
    as_path(cpath)->quadTo(x0, y0, x1, y1);
}

void sk_path_conic_to(sk_path_t* cpath, float x0, float y0, float x1, float y1, float w) {
    as_path(cpath)->conicTo(x0, y0, x1, y1, w);
}

void sk_path_cubic_to(sk_path_t* cpath, float x0, float y0, float x1, float y1, float x2, float y2) {
    as_path(cpath)->cubicTo(x0, y0, x1, y1, x2, y2);
}

void sk_path_close(sk_path_t* cpath) {
    as_path(cpath)->close();
}

void sk_path_add_rect(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    SkPath::Direction dir;
    if (!from_c_path_direction(cdir, &dir)) {
        return;
    }
    as_path(cpath)->addRect(AsRect(*crect), dir);
}

void sk_path_add_oval(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    SkPath::Direction dir;
    if (!from_c_path_direction(cdir, &dir)) {
        return;
    }
    as_path(cpath)->addOval(AsRect(*crect), dir);
}

bool sk_path_get_bounds(const sk_path_t* cpath, sk_rect_t* crect) {
    const SkPath& path = AsPath(*cpath);
    SkRect* rect = as_rect(crect);

    if (path.isEmpty()) {
        if (rect) {
            rect->setEmpty();
        }
        return false;
    }
    *rect = path.getBounds();
    return true;
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
    SkImageInfo info;
    if (!from_c_info(*cinfo, &info)) {
        return NULL;
    }
    return (sk_surface_t*)SkSurface::NewRaster(info);
}

sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t* cinfo, void* pixels,
                                           size_t rowBytes) {
    SkImageInfo info;
    if (!from_c_info(*cinfo, &info)) {
        return NULL;
    }
    return (sk_surface_t*)SkSurface::NewRasterDirect(info, pixels, rowBytes);
}

void sk_surface_unref(sk_surface_t* csurf) {
    SkSafeUnref((SkSurface*)csurf);
}

sk_canvas_t* sk_surface_get_canvas(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    return (sk_canvas_t*)surf->getCanvas();
}

sk_image_t* sk_surface_new_image_snapshot(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    return (sk_image_t*)surf->newImageSnapshot();
}

///////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/effects/SkGradientShader.h"
#include "sk_shader.h"

const struct {
    sk_shader_tilemode_t    fC;
    SkShader::TileMode      fSK;
} gTileModeMap[] = {
    { CLAMP_SK_SHADER_TILEMODE,     SkShader::kClamp_TileMode },
    { REPEAT_SK_SHADER_TILEMODE,    SkShader::kRepeat_TileMode },
    { MIRROR_SK_SHADER_TILEMODE,    SkShader::kMirror_TileMode  },
};

static bool from_c_tilemode(sk_shader_tilemode_t cMode, SkShader::TileMode* skMode) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gTileModeMap); ++i) {
        if (cMode == gTileModeMap[i].fC) {
            if (skMode) {
                *skMode = gTileModeMap[i].fSK;
            }
            return true;
        }
    }
    return false;
}

void sk_shader_ref(sk_shader_t* cshader) {
    SkSafeRef(AsShader(cshader));
}

void sk_shader_unref(sk_shader_t* cshader) {
    SkSafeUnref(AsShader(cshader));
}

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t pts[2],
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t cmode,
                                           const sk_matrix_t* cmatrix) {
    SkShader::TileMode mode;
    if (!from_c_tilemode(cmode, &mode)) {
        return NULL;
    }
    SkMatrix matrix;
    if (cmatrix) {
        matrix.setAll(cmatrix->mat[0], cmatrix->mat[1], cmatrix->mat[2],
                      cmatrix->mat[3], cmatrix->mat[4], cmatrix->mat[5],
                      cmatrix->mat[6], cmatrix->mat[7], cmatrix->mat[8]);
    } else {
        matrix.setIdentity();
    }
    SkShader* s = SkGradientShader::CreateLinear(reinterpret_cast<const SkPoint*>(pts),
                                                 reinterpret_cast<const SkColor*>(colors),
                                                 colorPos, colorCount, mode, 0, &matrix);
    return (sk_shader_t*)s;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

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
    
    sk_path_t* cpath = sk_path_new();
    sk_path_move_to(cpath, 50, 50);
    sk_path_line_to(cpath, 100, 100);
    sk_path_line_to(cpath, 50, 100);
    sk_path_close(cpath);

    sk_canvas_draw_path(ccanvas, cpath, cpaint);

    sk_image_t* cimage = sk_surface_new_image_snapshot(csurface);
    
    // HERE WE CROSS THE C..C++ boundary
    canvas->drawImage((const SkImage*)cimage, 20, 20, NULL);
    
    sk_path_delete(cpath);
    sk_paint_delete(cpaint);
    sk_image_unref(cimage);
    sk_surface_unref(csurface);
}
