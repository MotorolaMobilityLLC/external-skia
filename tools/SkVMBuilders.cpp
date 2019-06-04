/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkVMBuilders.h"

// Some parts of this builder code are written less fluently than possible,
// to avoid any ambiguity of function argument evaluation order.  This lets
// our golden tests work portably.  In general there's no reason to fear
// nesting calls to Builder routines.

SrcoverBuilder_F32::SrcoverBuilder_F32(Fmt srcFmt, Fmt dstFmt) {
    skvm::Arg src = arg(0),
              dst = arg(1);

    auto byte_to_f32 = [&](skvm::I32 byte) {
        skvm::F32 _1_255 = splat(1/255.0f);
        return mul(_1_255, to_f32(byte));
    };

    auto load = [&](skvm::Arg ptr, Fmt fmt,
                    skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32* a) {
        switch (fmt) {
            case Fmt::A8: {
                *r = *g = *b = splat(0.0f);
                *a = byte_to_f32(load8(ptr));
            } break;

            case Fmt::G8: {
                *r = *g = *b = byte_to_f32(load8(ptr));
                *a = splat(1.0f);
            } break;

            case Fmt::RGBA_8888: {
                skvm::I32 rgba = load32(ptr);
                *r = byte_to_f32(extract(rgba, 0xff));
                *g = byte_to_f32(extract(rgba, 0xff00));
                *b = byte_to_f32(extract(rgba, 0xff0000));
                *a = byte_to_f32(extract(rgba, 0xff000000));
            } break;
        }
    };

    skvm::F32 r,g,b,a;
    load(src, srcFmt, &r,&g,&b,&a);

    skvm::F32 dr,dg,db,da;
    load(dst, dstFmt, &dr,&dg,&db,&da);

    skvm::F32 invA = sub(splat(1.0f), a);
    r = mad(dr, invA, r);
    g = mad(dg, invA, g);
    b = mad(db, invA, b);
    a = mad(da, invA, a);

    auto f32_to_byte = [&](skvm::F32 f32) {
        skvm::F32 _255 = splat(255.0f),
                  _0_5 = splat(0.5f);
        return to_i32(mad(f32, _255, _0_5));
    };
    switch (dstFmt) {
        case Fmt::A8: {
            store8(dst, f32_to_byte(a));
        } break;

        case Fmt::G8: {
            skvm::F32 _2126 = splat(0.2126f),
                      _7152 = splat(0.7152f),
                      _0722 = splat(0.0722f);
            store8(dst, f32_to_byte(mad(r, _2126,
                                    mad(g, _7152,
                                    mul(b, _0722)))));
        } break;

        case Fmt::RGBA_8888: {
            skvm::I32 R = f32_to_byte(r),
                      G = f32_to_byte(g),
                      B = f32_to_byte(b),
                      A = f32_to_byte(a);

            R = pack(R, G, 8);
            B = pack(B, A, 8);
            R = pack(R, B, 16);

            store32(dst, R);
        } break;
    }
}

SrcoverBuilder_I32::SrcoverBuilder_I32() {
    skvm::Arg src = arg(0),
              dst = arg(1);

    auto load = [&](skvm::Arg ptr,
                    skvm::I32* r, skvm::I32* g, skvm::I32* b, skvm::I32* a) {
        skvm::I32 rgba = load32(ptr);
        *r = extract(rgba, 0xff);
        *g = extract(rgba, 0xff00);
        *b = extract(rgba, 0xff0000);
        *a = extract(rgba, 0xff000000);
    };

    skvm::I32 r,g,b,a;
    load(src, &r,&g,&b,&a);

    skvm::I32 dr,dg,db,da;
    load(dst, &dr,&dg,&db,&da);

    skvm::I32 invA = sub(splat(0xff), a);
    r = add(r, mul_unorm8(dr, invA));
    g = add(g, mul_unorm8(dg, invA));
    b = add(b, mul_unorm8(db, invA));
    a = add(a, mul_unorm8(da, invA));

    r = pack(r, g, 8);
    b = pack(b, a, 8);
    r = pack(r, b, 16);
    store32(dst, r);
}

SrcoverBuilder_I32_SWAR::SrcoverBuilder_I32_SWAR() {
    skvm::Arg src = arg(0),
              dst = arg(1);

    auto load = [&](skvm::Arg ptr,
                    skvm::I32* rb, skvm::I32* ga) {
        skvm::I32 rgba = load32(ptr);
        *rb = extract(rgba, 0x00ff00ff);
        *ga = extract(rgba, 0xff00ff00);
    };

    auto mul_unorm8_SWAR = [&](skvm::I32 x, skvm::I32 y) {
        // As above, assuming x is two SWAR bytes in lanes 0 and 2, and y is a byte.
        skvm::I32 _255 = splat(0x00ff00ff);
        return extract(add(mul(x, y), _255),
                       0xff00ff00);
    };

    skvm::I32 rb, ga;
    load(src, &rb, &ga);

    skvm::I32 drb, dga;
    load(dst, &drb, &dga);

    skvm::I32 _255 = splat(0xff),
              invA = sub(_255, shr(ga, 16));
    rb = add(rb, mul_unorm8_SWAR(drb, invA));
    ga = add(ga, mul_unorm8_SWAR(dga, invA));

    store32(dst, pack(rb, ga, 8));
}
