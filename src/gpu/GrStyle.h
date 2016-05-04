/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStyle_DEFINED
#define GrStyle_DEFINED

#include "GrTypes.h"
#include "SkPathEffect.h"
#include "SkStrokeRec.h"
#include "SkTemplates.h"

/**
 * Represents the various ways that a GrShape can be styled. It has fill/stroking information
 * as well as an optional path effect. If the path effect represents dashing, the dashing
 * information is extracted from the path effect and stored explicitly.
 *
 * This object does not support stroke-and-fill styling. It is expected that stroking and filling
 * is handled by drawing a stroke and a fill separately.
 *
 * This will replace GrStrokeInfo as GrShape is deployed.
 */
class GrStyle {
public:
    /**
     * A style object that represents a fill with no path effect.
     * TODO: constexpr with C++14
     */
    static const GrStyle& SimpleFill() {
        static const GrStyle kFill(SkStrokeRec::kFill_InitStyle);
        return kFill;
        }

    /**
     * A style object that represents a hairline stroke with no path effect.
     * TODO: constexpr with C++14
     */
    static const GrStyle& SimpleHairline() {
        static const GrStyle kHairline(SkStrokeRec::kHairline_InitStyle);
        return kHairline;
    }

    enum class Apply {
        kPathEffectOnly,
        kPathEffectAndStrokeRec
    };

    /**
     * Computes the key length for a GrStyle. The return will be negative if it cannot be turned
     * into a key. This occurs when there is a path effect that is not a dash. The key can
     * either reflect just the path effect (if one) or the path effect and the strokerec. Note
     * that a simple fill has a zero sized key.
     */
    static int KeySize(const GrStyle& , Apply);

    /**
     * Writes a unique key for the style into the provided buffer. This function assumes the buffer
     * has room for at least KeySize() values. It assumes that KeySize() returns a non-negative
     * value for the style and Apply param. This is written so that the key for just dash
     * application followed by the key for the remaining SkStrokeRec is the same as the key for
     * applying dashing and SkStrokeRec all at once.
     */
    static void WriteKey(uint32_t*, const GrStyle&, Apply);

    GrStyle() : GrStyle(SkStrokeRec::kFill_InitStyle) {}

    explicit GrStyle(SkStrokeRec::InitStyle initStyle) : fStrokeRec(initStyle) {}

    GrStyle(const SkStrokeRec& strokeRec, SkPathEffect* pe) : fStrokeRec(strokeRec) {
        SkASSERT(SkStrokeRec::kStrokeAndFill_Style != strokeRec.getStyle());
        this->initPathEffect(pe);
    }

    GrStyle(const GrStyle& that) : fStrokeRec(SkStrokeRec::kFill_InitStyle) { *this = that; }

    explicit GrStyle(const SkPaint& paint) : fStrokeRec(paint) {
        SkASSERT(SkStrokeRec::kStrokeAndFill_Style != fStrokeRec.getStyle());
        this->initPathEffect(paint.getPathEffect());
    }

    GrStyle& operator=(const GrStyle& that) {
        fPathEffect = that.fPathEffect;
        fDashInfo = that.fDashInfo;
        fStrokeRec = that.fStrokeRec;
        return *this;
    }

    void resetToInitStyle(SkStrokeRec::InitStyle fillOrHairline) {
        fDashInfo.reset();
        fPathEffect.reset(nullptr);
        if (SkStrokeRec::kFill_InitStyle == fillOrHairline) {
            fStrokeRec.setFillStyle();
        } else {
            fStrokeRec.setHairlineStyle();
        }
    }

    /** Is this style a fill with no path effect? */
    bool isSimpleFill() const { return fStrokeRec.isFillStyle() && !fPathEffect; }

    /** Is this style a hairline with no path effect? */
    bool isSimpleHairline() const { return fStrokeRec.isHairlineStyle() && !fPathEffect; }

    SkPathEffect* pathEffect() const { return fPathEffect.get(); }

    bool hasNonDashPathEffect() const { return fPathEffect.get() && !this->isDashed(); }

    bool isDashed() const { return SkPathEffect::kDash_DashType == fDashInfo.fType; }
    SkScalar dashPhase() const {
        SkASSERT(this->isDashed());
        return fDashInfo.fPhase;
    }
    int dashIntervalCnt() const {
        SkASSERT(this->isDashed());
        return fDashInfo.fIntervals.count();
    }
    const SkScalar* dashIntervals() const {
        SkASSERT(this->isDashed());
        return fDashInfo.fIntervals.get();
    }

    const SkStrokeRec& strokeRec() const { return fStrokeRec; }

    /** Hairline or fill styles without path effects make no alterations to a geometry. */
    bool applies() const {
        return this->pathEffect() || (!fStrokeRec.isFillStyle() && !fStrokeRec.isHairlineStyle());
    }

    /**
     * Applies just the path effect and returns remaining stroke information. This will fail if
     * there is no path effect.
     */
    bool applyPathEffectToPath(SkPath* dst, SkStrokeRec* remainingStoke, const SkPath& src) const;

    /** If this succeeds then the result path should be filled or hairlined as indicated by the
        returned SkStrokeRec::InitStyle value. Will fail if there is no path effect and the
        strokerec doesn't change the geometry. */
    bool applyToPath(SkPath* dst, SkStrokeRec::InitStyle* fillOrHairline, const SkPath& src) const;

    /** Given bounds of a path compute the bounds of path with the style applied. */
    void adjustBounds(SkRect* dst, const SkRect& src) const {
        if (this->pathEffect()) {
            this->pathEffect()->computeFastBounds(dst, src);
        } else {
            SkScalar radius = fStrokeRec.getInflationRadius();
            *dst = src.makeOutset(radius, radius);
        }
    }

private:
    void initPathEffect(SkPathEffect* pe);

    struct DashInfo {
        DashInfo() : fType(SkPathEffect::kNone_DashType) {}
        DashInfo& operator=(const DashInfo& that) {
            fType = that.fType;
            fPhase = that.fPhase;
            fIntervals.reset(that.fIntervals.count());
            memcpy(fIntervals.get(), that.fIntervals.get(),
                   sizeof(SkScalar) * that.fIntervals.count());
            return *this;
        }
        void reset() {
            fType = SkPathEffect::kNone_DashType;
            fIntervals.reset(0);
        }
        SkPathEffect::DashType      fType;
        SkScalar                    fPhase;
        SkAutoSTArray<4, SkScalar>  fIntervals;
    };

    SkStrokeRec         fStrokeRec;
    sk_sp<SkPathEffect> fPathEffect;
    DashInfo            fDashInfo;
};

#endif
