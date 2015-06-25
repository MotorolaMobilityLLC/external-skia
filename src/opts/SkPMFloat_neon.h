/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

namespace { // See SkPMFloat.h

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    SkPMColorAssert(c);
    uint8x8_t   fix8    = (uint8x8_t)vdup_n_u32(c);
    uint16x8_t  fix8_16 = vmovl_u8(fix8);
    uint32x4_t  fix8_32 = vmovl_u16(vget_low_u16(fix8_16));
    fVec = vcvtq_n_f32_u32(fix8_32, 8);
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::round() const {
    // vcvtq_n_u32_f32 truncates, so we round manually by adding a half before converting.
    float32x4_t rounded = vaddq_f32(fVec, vdupq_n_f32(0.5f/255));
    uint32x4_t  fix8_32 = vcvtq_n_u32_f32(rounded, 8);
    uint16x4_t  fix8_16 = vqmovn_u32(fix8_32);
    uint8x8_t   fix8    = vqmovn_u16(vcombine_u16(fix8_16, vdup_n_u16(0)));
    SkPMColor c = vget_lane_u32((uint32x2_t)fix8, 0);
    SkPMColorAssert(c);
    return c;
}

}  // namespace
