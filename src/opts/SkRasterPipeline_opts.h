/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_opts_DEFINED
#define SkRasterPipeline_opts_DEFINED

#include "SkHalf.h"
#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkSRGB.h"

using SkNf = SkRasterPipeline::V;
static constexpr auto N = sizeof(SkNf) / sizeof(float);
using SkNi = SkNx<N, int>;
using SkNh = SkNx<N, uint16_t>;

#define SI static inline

#define STAGE(name, kCallNext)                                                             \
    static SK_ALWAYS_INLINE void name##_kernel(void* ctx, size_t x, size_t tail,           \
                                               SkNf&  r, SkNf&  g, SkNf&  b, SkNf&  a,     \
                                               SkNf& dr, SkNf& dg, SkNf& db, SkNf& da);    \
    SI void SK_VECTORCALL name(SkRasterPipeline::Stage* st, size_t x, size_t tail,         \
                               SkNf  r, SkNf  g, SkNf  b, SkNf  a,                         \
                               SkNf dr, SkNf dg, SkNf db, SkNf da) {                       \
        name##_kernel(st->ctx<void*>(), x,0, r,g,b,a, dr,dg,db,da);                        \
        if (kCallNext) {                                                                   \
            st->next(x,tail, r,g,b,a, dr,dg,db,da);                                        \
        }                                                                                  \
    }                                                                                      \
    SI void SK_VECTORCALL name##_tail(SkRasterPipeline::Stage* st, size_t x, size_t tail,  \
                                      SkNf  r, SkNf  g, SkNf  b, SkNf  a,                  \
                                      SkNf dr, SkNf dg, SkNf db, SkNf da) {                \
        name##_kernel(st->ctx<void*>(), x,tail, r,g,b,a, dr,dg,db,da);                     \
        if (kCallNext) {                                                                   \
            st->next(x,tail, r,g,b,a, dr,dg,db,da);                                        \
        }                                                                                  \
    }                                                                                      \
    static SK_ALWAYS_INLINE void name##_kernel(void* ctx, size_t x, size_t tail,           \
                                               SkNf&  r, SkNf&  g, SkNf&  b, SkNf&  a,     \
                                               SkNf& dr, SkNf& dg, SkNf& db, SkNf& da)


// Many xfermodes apply the same logic to each channel.
#define RGBA_XFERMODE(name)                                                                \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,              \
                                               const SkNf& d, const SkNf& da);             \
    SI void SK_VECTORCALL name(SkRasterPipeline::Stage* st, size_t x, size_t tail,         \
                               SkNf  r, SkNf  g, SkNf  b, SkNf  a,                         \
                               SkNf dr, SkNf dg, SkNf db, SkNf da) {                       \
        r = name##_kernel(r,a,dr,da);                                                      \
        g = name##_kernel(g,a,dg,da);                                                      \
        b = name##_kernel(b,a,db,da);                                                      \
        a = name##_kernel(a,a,da,da);                                                      \
        st->next(x,tail, r,g,b,a, dr,dg,db,da);                                            \
    }                                                                                      \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,              \
                                               const SkNf& d, const SkNf& da)

// Most of the rest apply the same logic to color channels and use srcover's alpha logic.
#define RGB_XFERMODE(name)                                                                 \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,              \
                                               const SkNf& d, const SkNf& da);             \
    SI void SK_VECTORCALL name(SkRasterPipeline::Stage* st, size_t x, size_t tail,         \
                               SkNf  r, SkNf  g, SkNf  b, SkNf  a,                         \
                               SkNf dr, SkNf dg, SkNf db, SkNf da) {                       \
        r = name##_kernel(r,a,dr,da);                                                      \
        g = name##_kernel(g,a,dg,da);                                                      \
        b = name##_kernel(b,a,db,da);                                                      \
        a = a + (da * (1.0f-a));                                                           \
        st->next(x,tail, r,g,b,a, dr,dg,db,da);                                            \
    }                                                                                      \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,              \
                                               const SkNf& d, const SkNf& da)


namespace SK_OPTS_NS {

    SI void run_pipeline(size_t x, size_t n,
                         SkRasterPipeline::Fn bodyStart, SkRasterPipeline::Stage* body,
                         SkRasterPipeline::Fn tailStart, SkRasterPipeline::Stage* tail) {
        SkNf v;  // Fastest to start uninitialized.
        while (n >= N) {
            bodyStart(body, x,0, v,v,v,v, v,v,v,v);
            x += N;
            n -= N;
        }
        if (n > 0) {
            tailStart(tail, x,n, v,v,v,v, v,v,v,v);
        }
    }

    // Clamp colors into [0,1] premul (e.g. just before storing back to memory).
    SI void clamp_01_premul(SkNf& r, SkNf& g, SkNf& b, SkNf& a) {
        a = SkNf::Max(a, 0.0f);
        r = SkNf::Max(r, 0.0f);
        g = SkNf::Max(g, 0.0f);
        b = SkNf::Max(b, 0.0f);

        a = SkNf::Min(a, 1.0f);
        r = SkNf::Min(r, a);
        g = SkNf::Min(g, a);
        b = SkNf::Min(b, a);
    }

    SI SkNf inv(const SkNf& x) { return 1.0f - x; }

    SI SkNf lerp(const SkNf& from, const SkNf& to, const SkNf& cov) {
        return SkNx_fma(to-from, cov, from);
    }

    template <typename T>
    SI SkNx<N,T> load_tail(size_t tail, const T* src) {
        T vals[N] = { 0 };
        switch (tail) {
            case 0: return SkNx<N,T>::Load(src);

            // TODO: maskload for 32- and 64-bit T
            case 7: if (N > 4) { vals[6] = src[6]; }
            case 6: if (N > 4) { vals[5] = src[5]; }
            case 5: if (N > 4) { vals[4] = src[4]; }
            case 4: if (N > 4) { vals[3] = src[3]; }
            case 3:              vals[2] = src[2];
            case 2:              vals[1] = src[1];
            case 1:              vals[0] = src[0];
        }
        return SkNx<N,T>::Load(vals);
    }

    template <typename T>
    SI void store_tail(size_t tail, const SkNx<N,T>& v, T* dst) {
        switch(tail) {
            case 0: return v.store(dst);

            // TODO: maskstore for 32- and 64-bit T
            case 7: if (N > 4) { dst[6] = v[6]; }
            case 6: if (N > 4) { dst[5] = v[5]; }
            case 5: if (N > 4) { dst[4] = v[4]; }
            case 4: if (N > 4) { dst[3] = v[3]; }
            case 3:              dst[2] = v[2];
            case 2:              dst[1] = v[1];
            case 1:              dst[0] = v[0];
        }
    }

    SI void from_565(const SkNh& _565, SkNf* r, SkNf* g, SkNf* b) {
        auto _32_bit = SkNx_cast<int>(_565);

        *r = SkNx_cast<float>(_32_bit & SK_R16_MASK_IN_PLACE) * (1.0f / SK_R16_MASK_IN_PLACE);
        *g = SkNx_cast<float>(_32_bit & SK_G16_MASK_IN_PLACE) * (1.0f / SK_G16_MASK_IN_PLACE);
        *b = SkNx_cast<float>(_32_bit & SK_B16_MASK_IN_PLACE) * (1.0f / SK_B16_MASK_IN_PLACE);
    }

    SI SkNh to_565(const SkNf& r, const SkNf& g, const SkNf& b) {
        return SkNx_cast<uint16_t>( SkNx_cast<int>(r * SK_R16_MASK + 0.5f) << SK_R16_SHIFT
                                  | SkNx_cast<int>(g * SK_G16_MASK + 0.5f) << SK_G16_SHIFT
                                  | SkNx_cast<int>(b * SK_B16_MASK + 0.5f) << SK_B16_SHIFT);
    }

    STAGE(just_return, false) { }

    // The default shader produces a constant color (from the SkPaint).
    STAGE(constant_color, true) {
        auto color = (const SkPM4f*)ctx;
        r = color->r();
        g = color->g();
        b = color->b();
        a = color->a();
    }

    // s' = d(1-c) + sc, for a constant c.
    STAGE(lerp_constant_float, true) {
        SkNf c = *(const float*)ctx;

        r = lerp(dr, r, c);
        g = lerp(dg, g, c);
        b = lerp(db, b, c);
        a = lerp(da, a, c);
    }

    // s' = sc for 8-bit c.
    STAGE(scale_u8, true) {
        auto ptr = (const uint8_t*)ctx + x;

        SkNf c = SkNx_cast<float>(load_tail(tail, ptr)) * (1/255.0f);
        r = r*c;
        g = g*c;
        b = b*c;
        a = a*c;
    }

    // s' = d(1-c) + sc for 8-bit c.
    STAGE(lerp_u8, true) {
        auto ptr = (const uint8_t*)ctx + x;

        SkNf c = SkNx_cast<float>(load_tail(tail, ptr)) * (1/255.0f);
        r = lerp(dr, r, c);
        g = lerp(dg, g, c);
        b = lerp(db, b, c);
        a = lerp(da, a, c);
    }

    // s' = d(1-c) + sc for 565 c.
    STAGE(lerp_565, true) {
        auto ptr = (const uint16_t*)ctx + x;
        SkNf cr, cg, cb;
        from_565(load_tail(tail, ptr), &cr, &cg, &cb);

        r = lerp(dr, r, cr);
        g = lerp(dg, g, cg);
        b = lerp(db, b, cb);
        a = 1.0f;
    }

    STAGE(load_d_565, true) {
        auto ptr = (const uint16_t*)ctx + x;
        from_565(load_tail(tail, ptr), &dr,&dg,&db);
        da = 1.0f;
    }

    STAGE(load_s_565, true) {
        auto ptr = (const uint16_t*)ctx + x;
        from_565(load_tail(tail, ptr), &r,&g,&b);
        a = 1.0f;
    }

    STAGE(store_565, false) {
        clamp_01_premul(r,g,b,a);
        auto ptr = (uint16_t*)ctx + x;
        store_tail(tail, to_565(r,g,b), ptr);
    }

    STAGE(load_d_f16, true) {
        auto ptr = (const uint64_t*)ctx + x;

        uint64_t buf[N] = {0};
        if (tail) {
            memcpy(buf, ptr, tail*sizeof(uint64_t));
            ptr = buf;
        }

        SkNh rh, gh, bh, ah;
        SkNh::Load4(ptr, &rh, &gh, &bh, &ah);
        dr = SkHalfToFloat_finite_ftz(rh);
        dg = SkHalfToFloat_finite_ftz(gh);
        db = SkHalfToFloat_finite_ftz(bh);
        da = SkHalfToFloat_finite_ftz(ah);
    }

    STAGE(load_s_f16, true) {
        auto ptr = (const uint64_t*)ctx + x;

        uint64_t buf[N] = {0};
        if (tail) {
            memcpy(buf, ptr, tail*sizeof(uint64_t));
            ptr = buf;
        }

        SkNh rh, gh, bh, ah;
        SkNh::Load4(ptr, &rh, &gh, &bh, &ah);
        r = SkHalfToFloat_finite_ftz(rh);
        g = SkHalfToFloat_finite_ftz(gh);
        b = SkHalfToFloat_finite_ftz(bh);
        a = SkHalfToFloat_finite_ftz(ah);
    }

    STAGE(store_f16, false) {
        clamp_01_premul(r,g,b,a);
        auto ptr = (uint64_t*)ctx + x;

        uint64_t buf[N] = {0};
        SkNh::Store4(tail ? buf : ptr, SkFloatToHalf_finite_ftz(r),
                                       SkFloatToHalf_finite_ftz(g),
                                       SkFloatToHalf_finite_ftz(b),
                                       SkFloatToHalf_finite_ftz(a));
        if (tail) {
            memcpy(ptr, buf, tail*sizeof(uint64_t));
        }
    }


    // Load 8-bit SkPMColor-order sRGB.
    STAGE(load_d_srgb, true) {
        auto ptr = (const uint32_t*)ctx + x;

        auto px = load_tail(tail, ptr);
        auto to_int = [](const SkNx<N, uint32_t>& v) { return SkNi::Load(&v); };
        dr =    sk_linear_from_srgb_math(to_int((px >> SK_R32_SHIFT) & 0xff));
        dg =    sk_linear_from_srgb_math(to_int((px >> SK_G32_SHIFT) & 0xff));
        db =    sk_linear_from_srgb_math(to_int((px >> SK_B32_SHIFT) & 0xff));
        da = (1/255.0f)*SkNx_cast<float>(to_int( px >> SK_A32_SHIFT        ));
    }

    STAGE(load_s_srgb, true) {
        auto ptr = (const uint32_t*)ctx + x;

        auto px = load_tail(tail, ptr);
        auto to_int = [](const SkNx<N, uint32_t>& v) { return SkNi::Load(&v); };
        r =    sk_linear_from_srgb_math(to_int((px >> SK_R32_SHIFT) & 0xff));
        g =    sk_linear_from_srgb_math(to_int((px >> SK_G32_SHIFT) & 0xff));
        b =    sk_linear_from_srgb_math(to_int((px >> SK_B32_SHIFT) & 0xff));
        a = (1/255.0f)*SkNx_cast<float>(to_int( px >> SK_A32_SHIFT        ));
    }

    STAGE(store_srgb, false) {
        clamp_01_premul(r,g,b,a);
        auto ptr = (uint32_t*)ctx + x;
        store_tail(tail, (      sk_linear_to_srgb_noclamp(r) << SK_R32_SHIFT
                         |      sk_linear_to_srgb_noclamp(g) << SK_G32_SHIFT
                         |      sk_linear_to_srgb_noclamp(b) << SK_B32_SHIFT
                         | SkNx_cast<int>(255.0f * a + 0.5f) << SK_A32_SHIFT ), (int*)ptr);
    }

    RGBA_XFERMODE(clear)    { return 0.0f; }
  //RGBA_XFERMODE(src)      { return s; }   // This would be a no-op stage, so we just omit it.
    RGBA_XFERMODE(dst)      { return d; }

    RGBA_XFERMODE(srcatop)  { return s*da + d*inv(sa); }
    RGBA_XFERMODE(srcin)    { return s * da; }
    RGBA_XFERMODE(srcout)   { return s * inv(da); }
    RGBA_XFERMODE(srcover)  { return SkNx_fma(d, inv(sa), s); }
    RGBA_XFERMODE(dstatop)  { return srcatop_kernel(d,da,s,sa); }
    RGBA_XFERMODE(dstin)    { return srcin_kernel  (d,da,s,sa); }
    RGBA_XFERMODE(dstout)   { return srcout_kernel (d,da,s,sa); }
    RGBA_XFERMODE(dstover)  { return srcover_kernel(d,da,s,sa); }

    RGBA_XFERMODE(modulate) { return s*d; }
    RGBA_XFERMODE(multiply) { return s*inv(da) + d*inv(sa) + s*d; }
    RGBA_XFERMODE(plus_)    { return s + d; }
    RGBA_XFERMODE(screen)   { return s + d - s*d; }
    RGBA_XFERMODE(xor_)     { return s*inv(da) + d*inv(sa); }

    RGB_XFERMODE(colorburn) {
        return (d == da  ).thenElse(d + s*inv(da),
               (s == 0.0f).thenElse(s + d*inv(sa),
                                    sa*(da - SkNf::Min(da, (da-d)*sa/s)) + s*inv(da) + d*inv(sa)));
    }
    RGB_XFERMODE(colordodge) {
        return (d == 0.0f).thenElse(d + s*inv(da),
               (s == sa  ).thenElse(s + d*inv(sa),
                                    sa*SkNf::Min(da, (d*sa)/(sa - s)) + s*inv(da) + d*inv(sa)));
    }
    RGB_XFERMODE(darken)     { return s + d - SkNf::Max(s*da, d*sa); }
    RGB_XFERMODE(difference) { return s + d - 2.0f*SkNf::Min(s*da,d*sa); }
    RGB_XFERMODE(exclusion)  { return s + d - 2.0f*s*d; }
    RGB_XFERMODE(hardlight) {
        return s*inv(da) + d*inv(sa)
             + (2.0f*s <= sa).thenElse(2.0f*s*d, sa*da - 2.0f*(da-d)*(sa-s));
    }
    RGB_XFERMODE(lighten) { return s + d - SkNf::Min(s*da, d*sa); }
    RGB_XFERMODE(overlay) { return hardlight_kernel(d,da,s,sa); }
    RGB_XFERMODE(softlight) {
        SkNf m  = (da > 0.0f).thenElse(d / da, 0.0f),
             s2 = 2.0f*s,
             m4 = 4.0f*m;

        // The logic forks three ways:
        //    1. dark src?
        //    2. light src, dark dst?
        //    3. light src, light dst?
        SkNf darkSrc = d*(sa + (s2 - sa)*(1.0f - m)),     // Used in case 1.
             darkDst = (m4*m4 + m4)*(m - 1.0f) + 7.0f*m,  // Used in case 2.
             liteDst = m.rsqrt().invert() - m,            // Used in case 3.
             liteSrc = d*sa + da*(s2 - sa) * (4.0f*d <= da).thenElse(darkDst, liteDst);  // 2 or 3?
        return s*inv(da) + d*inv(sa) + (s2 <= sa).thenElse(darkSrc, liteSrc);  // 1 or (2 or 3)?
    }
}

#undef SI
#undef STAGE
#undef RGBA_XFERMODE
#undef RGB_XFERMODE

#endif//SkRasterPipeline_opts_DEFINED
