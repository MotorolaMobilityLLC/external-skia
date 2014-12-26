/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#include "sk_canvas.h"
#include "sk_data.h"
#include "sk_image.h"
#include "sk_paint.h"
#include "sk_shader.h"
#include "sk_surface.h"

extern void sk_test_c_api(sk_canvas_t*);

#define W   256
#define H   256

static sk_shader_t* make_shader() {
    sk_point_t pts[] = { { 0, 0 }, { W, H } };
    sk_color_t colors[] = { 0xFF00FF00, 0xFF0000FF };
    return sk_shader_new_linear_gradient(pts, colors, NULL, 2, CLAMP_SK_SHADER_TILEMODE, NULL);
}

static void do_draw(sk_canvas_t* canvas) {
    sk_paint_t* paint = sk_paint_new();
    sk_paint_set_antialias(paint, true);
    
    sk_paint_set_color(paint, 0xFFFFFFFF);
    sk_canvas_draw_paint(canvas, paint);
    
    sk_rect_t r = { 10, 10, W - 10, H - 10 };
    
    sk_paint_set_color(paint, 0xFFFF0000);
    sk_canvas_draw_rect(canvas, &r, paint);
    
    sk_shader_t* shader = make_shader();
    sk_paint_set_shader(paint, shader);
    sk_shader_unref(shader);
    
    sk_canvas_draw_oval(canvas, &r, paint);
    
    sk_paint_delete(paint);
}

void sk_test_c_api(sk_canvas_t* canvas) {
    do_draw(canvas);

    sk_imageinfo_t info = {
        W, H, sk_colortype_get_default_8888(), OPAQUE_SK_ALPHATYPE
    };
    sk_surface_t* surf = sk_surface_new_raster(&info);
    do_draw(sk_surface_get_canvas(surf));

    sk_image_t* img0 = sk_surface_new_image_snapshot(surf);
    sk_surface_unref(surf);

    sk_canvas_draw_image(canvas, img0, W + 10, 10, NULL);

    sk_data_t* data = sk_image_encode(img0);
    sk_image_unref(img0);

    sk_image_t* img1 = sk_image_new_from_data(data);
    sk_data_unref(data);

    if (img1) {
        sk_canvas_draw_image(canvas, img1, W/2, H/2, NULL);
        sk_image_unref(img1);
    }
}


