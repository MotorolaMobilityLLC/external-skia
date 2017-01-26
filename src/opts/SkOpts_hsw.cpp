/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// It is not safe to #include any header file here unless it has been vetted for ODR safety:
// all symbols used must be file-scoped static or in an anonymous namespace.  This applies
// to _all_ header files:  C standard library, C++ standard library, Skia... everything.

#include <immintrin.h>   // ODR safe
#include <stdint.h>      // ODR safe

namespace hsw {

    void convolve_vertically(const int16_t* filter, int filterLen,
                             uint8_t* const* srcRows, int width,
                             uint8_t* out, bool hasAlpha) {
        // It's simpler to work with the output array in terms of 4-byte pixels.
        auto dst = (int*)out;

        // Output up to eight pixels per iteration.
        for (int x = 0; x < width; x += 8) {
            // Accumulated result for 4 adjacent pairs of pixels, in signed 17.14 fixed point.
            auto accum01 = _mm256_setzero_si256(),
                 accum23 = _mm256_setzero_si256(),
                 accum45 = _mm256_setzero_si256(),
                 accum67 = _mm256_setzero_si256();

            // Convolve with the filter.  (This inner loop is where we spend ~all our time.)
            for (int i = 0; i < filterLen; i++) {
                auto coeffs = _mm256_set1_epi16(filter[i]);
                auto pixels = _mm256_loadu_si256((const __m256i*)(srcRows[i] + x*4));

                auto pixels_0123 = _mm256_unpacklo_epi8(pixels, _mm256_setzero_si256()),
                     pixels_4567 = _mm256_unpackhi_epi8(pixels, _mm256_setzero_si256());

                auto lo_0123 = _mm256_mullo_epi16(pixels_0123, coeffs),
                     hi_0123 = _mm256_mulhi_epi16(pixels_0123, coeffs),
                     lo_4567 = _mm256_mullo_epi16(pixels_4567, coeffs),
                     hi_4567 = _mm256_mulhi_epi16(pixels_4567, coeffs);

                accum01 = _mm256_add_epi32(accum01, _mm256_unpacklo_epi16(lo_0123, hi_0123));
                accum23 = _mm256_add_epi32(accum23, _mm256_unpackhi_epi16(lo_0123, hi_0123));
                accum45 = _mm256_add_epi32(accum45, _mm256_unpacklo_epi16(lo_4567, hi_4567));
                accum67 = _mm256_add_epi32(accum67, _mm256_unpackhi_epi16(lo_4567, hi_4567));
            }

            // Trim the fractional parts.
            accum01 = _mm256_srai_epi32(accum01, 14);
            accum23 = _mm256_srai_epi32(accum23, 14);
            accum45 = _mm256_srai_epi32(accum45, 14);
            accum67 = _mm256_srai_epi32(accum67, 14);

            // Pack back down to 8-bit channels.
            auto pixels = _mm256_packus_epi16(_mm256_packs_epi32(accum01, accum23),
                                              _mm256_packs_epi32(accum45, accum67));

            if (hasAlpha) {
                // Clamp alpha to the max of r,g,b to make sure we stay premultiplied.
                __m256i max_rg  = _mm256_max_epu8(pixels, _mm256_srli_epi32(pixels,  8)),
                        max_rgb = _mm256_max_epu8(max_rg, _mm256_srli_epi32(pixels, 16));
                pixels = _mm256_max_epu8(pixels, _mm256_slli_epi32(max_rgb, 24));
            } else {
                // Force opaque.
                pixels = _mm256_or_si256(pixels, _mm256_set1_epi32(0xff000000));
            }

            // Normal path to store 8 pixels.
            if (x + 8 <= width) {
                _mm256_storeu_si256((__m256i*)dst, pixels);
                dst += 8;
                continue;
            }

            // Store one pixel at a time on the last iteration.
            for (int i = x; i < width; i++) {
                *dst++ = _mm_cvtsi128_si32(_mm256_castsi256_si128(pixels));
                pixels = _mm256_permutevar8x32_epi32(pixels, _mm256_setr_epi32(1,2,3,4,5,6,7,0));
            }
        }
    }

}

namespace SkOpts {
    // See SkOpts.h, writing SkConvolutionFilter1D::ConvolutionFixed as the underlying type.
    extern void (*convolve_vertically)(const int16_t* filter, int filterLen,
                                       uint8_t* const* srcRows, int width,
                                       uint8_t* out, bool hasAlpha);
    void Init_hsw() {
        convolve_vertically = hsw::convolve_vertically;
    }
}
