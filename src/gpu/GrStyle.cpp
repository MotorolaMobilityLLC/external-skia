/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStyle.h"

int GrStyle::KeySize(const GrStyle &style, Apply apply, uint32_t flags) {
    GR_STATIC_ASSERT(sizeof(uint32_t) == sizeof(SkScalar));
    int size = 0;
    if (style.isDashed()) {
        // One scalar for scale, one for dash phase, and one for each dash value.
        size += 2 + style.dashIntervalCnt();
    } else if (style.pathEffect()) {
        // No key for a generic path effect.
        return -1;
    }

    if (Apply::kPathEffectOnly == apply) {
        return size;
    }

    if (style.strokeRec().needToApply()) {
        // One for res scale, one for style/cap/join, one for miter limit, and one for width.
        size += 4;
    }
    return size;
}

void GrStyle::WriteKey(uint32_t *key, const GrStyle &style, Apply apply, SkScalar scale,
                       uint32_t flags) {
    SkASSERT(key);
    SkASSERT(KeySize(style, apply) >= 0);
    GR_STATIC_ASSERT(sizeof(uint32_t) == sizeof(SkScalar));

    int i = 0;
    // The scale can influence both the path effect and stroking. We want to preserve the
    // property that the following two are equal:
    // 1. WriteKey with apply == kPathEffectAndStrokeRec
    // 2. WriteKey with apply == kPathEffectOnly followed by WriteKey of a GrStyle made
    //    from SkStrokeRec output by the the path effect (and no additional path effect).
    // Since the scale can affect both parts of 2 we write it into the key twice.
    if (style.isDashed()) {
        GR_STATIC_ASSERT(sizeof(style.dashPhase()) == sizeof(uint32_t));
        SkScalar phase = style.dashPhase();
        memcpy(&key[i++], &scale, sizeof(SkScalar));
        memcpy(&key[i++], &phase, sizeof(SkScalar));

        int32_t count = style.dashIntervalCnt();
        // Dash count should always be even.
        SkASSERT(0 == (count & 0x1));
        const SkScalar *intervals = style.dashIntervals();
        int intervalByteCnt = count * sizeof(SkScalar);
        memcpy(&key[i], intervals, intervalByteCnt);
        i += count;
    } else {
        SkASSERT(!style.pathEffect());
    }

    if (Apply::kPathEffectAndStrokeRec == apply && style.strokeRec().needToApply()) {
        memcpy(&key[i++], &scale, sizeof(SkScalar));
        enum {
            kStyleBits = 2,
            kJoinBits = 2,
            kCapBits = 32 - kStyleBits - kJoinBits,

            kJoinShift = kStyleBits,
            kCapShift = kJoinShift + kJoinBits,
        };
        GR_STATIC_ASSERT(SkStrokeRec::kStyleCount <= (1 << kStyleBits));
        GR_STATIC_ASSERT(SkPaint::kJoinCount <= (1 << kJoinBits));
        GR_STATIC_ASSERT(SkPaint::kCapCount <= (1 << kCapBits));
        // The cap type only matters for unclosed shapes. However, a path effect could unclose
        // the shape before it is stroked.
        SkPaint::Cap cap;
        if ((flags & kClosed_KeyFlag) && !style.pathEffect()) {
            cap = SkPaint::kButt_Cap;
        } else {
            cap = style.strokeRec().getCap();
        }
        key[i++] = style.strokeRec().getStyle() |
                   style.strokeRec().getJoin() << kJoinShift |
                   cap << kCapShift;

        SkScalar scalar;
        // Miter limit only affects miter joins
        scalar = SkPaint::kMiter_Join == style.strokeRec().getJoin()
                 ? style.strokeRec().getMiter()
                 : -1.f;
        memcpy(&key[i++], &scalar, sizeof(scalar));

        scalar = style.strokeRec().getWidth();
        memcpy(&key[i++], &scalar, sizeof(scalar));
    }
    SkASSERT(KeySize(style, apply) == i);
}

void GrStyle::initPathEffect(SkPathEffect* pe) {
    SkASSERT(!fPathEffect)
    SkASSERT(SkPathEffect::kNone_DashType == fDashInfo.fType);
    SkASSERT(0 == fDashInfo.fIntervals.count());
    if (!pe) {
        return;
    }
    SkPathEffect::DashInfo info;
    if (SkPathEffect::kDash_DashType == pe->asADash(&info)) {
        SkStrokeRec::Style recStyle = fStrokeRec.getStyle();
        if (recStyle != SkStrokeRec::kFill_Style && recStyle != SkStrokeRec::kStrokeAndFill_Style) {
            fDashInfo.fType = SkPathEffect::kDash_DashType;
            fDashInfo.fIntervals.reset(info.fCount);
            fDashInfo.fPhase = info.fPhase;
            info.fIntervals = fDashInfo.fIntervals.get();
            pe->asADash(&info);
            fPathEffect.reset(SkSafeRef(pe));
        }
    } else {
        fPathEffect.reset(SkSafeRef(pe));
    }
}

static inline bool apply_path_effect(SkPath* dst, SkStrokeRec* strokeRec,
                                     const sk_sp<SkPathEffect>& pe, const SkPath& src) {
    if (!pe) {
        return false;
    }
    if (!pe->filterPath(dst, src, strokeRec, nullptr)) {
        return false;
    }
    dst->setIsVolatile(true);
    return true;
}

bool GrStyle::applyPathEffectToPath(SkPath *dst, SkStrokeRec *remainingStroke,
                                    const SkPath &src, SkScalar resScale) const {
    SkASSERT(dst);
    SkStrokeRec strokeRec = fStrokeRec;
    strokeRec.setResScale(resScale);
    if (!apply_path_effect(dst, &strokeRec, fPathEffect, src)) {
        return false;
    }
    *remainingStroke = strokeRec;
    return true;
}

bool GrStyle::applyToPath(SkPath* dst, SkStrokeRec::InitStyle* style, const SkPath& src,
                          SkScalar resScale) const {
    SkASSERT(style);
    SkASSERT(dst);
    SkStrokeRec strokeRec = fStrokeRec;
    strokeRec.setResScale(resScale);
    const SkPath* pathForStrokeRec = &src;
    if (apply_path_effect(dst, &strokeRec, fPathEffect, src)) {
        pathForStrokeRec = dst;
    } else if (fPathEffect) {
        return false;
    }
    if (strokeRec.needToApply()) {
        if (!strokeRec.applyToPath(dst, *pathForStrokeRec)) {
            return false;
        }
        *style = SkStrokeRec::kFill_InitStyle;
    } else if (!fPathEffect) {
        // Nothing to do for path effect or stroke, fail.
        return false;
    } else {
        SkASSERT(SkStrokeRec::kFill_Style == strokeRec.getStyle() ||
                 SkStrokeRec::kHairline_Style == strokeRec.getStyle());
        *style = strokeRec.getStyle() == SkStrokeRec::kFill_Style
                 ? SkStrokeRec::kFill_InitStyle
                 : SkStrokeRec::kHairline_InitStyle;
    }
    return true;
}
