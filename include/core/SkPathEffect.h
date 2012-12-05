
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPathEffect_DEFINED
#define SkPathEffect_DEFINED

#include "SkFlattenable.h"
#include "SkPaint.h"

class SkPath;

class SkStrokeRec {
public:
    enum InitStyle {
        kHairline_InitStyle,
        kFill_InitStyle
    };
    SkStrokeRec(InitStyle style);

    SkStrokeRec(const SkStrokeRec&);
    explicit SkStrokeRec(const SkPaint&);

    enum Style {
        kHairline_Style,
        kFill_Style,
        kStroke_Style,
        kStrokeAndFill_Style
    };

    Style getStyle() const;
    SkScalar getWidth() const { return fWidth; }
    SkScalar getMiter() const { return fMiterLimit; }
    SkPaint::Cap getCap() const { return fCap; }
    SkPaint::Join getJoin() const { return fJoin; }

    bool isHairlineStyle() const {
        return kHairline_Style == this->getStyle();
    }

    bool isFillStyle() const {
        return kFill_Style == this->getStyle();
    }

    void setFillStyle();
    void setHairlineStyle();
    /**
     *  Specify the strokewidth, and optionally if you want stroke + fill.
     *  Note, if width==0, then this request is taken to mean:
     *      strokeAndFill==true -> new style will be Fill
     *      strokeAndFill==false -> new style will be Hairline
     */
    void setStrokeStyle(SkScalar width, bool strokeAndFill = false);

    void setStrokeParams(SkPaint::Cap cap, SkPaint::Join join, SkScalar miterLimit) {
        fCap = cap;
        fJoin = join;
        fMiterLimit = miterLimit;
    }

    /**
     *  Returns true if this specifes any thick stroking, i.e. applyToPath()
     *  will return true.
     */
    bool needToApply() const {
        Style style = this->getStyle();
        return (kStroke_Style == style) || (kStrokeAndFill_Style == style);
    }

    /**
     *  Apply these stroke parameters to the src path, returning the result
     *  in dst.
     *
     *  If there was no change (i.e. style == hairline or fill) this returns
     *  false and dst is unchanged. Otherwise returns true and the result is
     *  stored in dst.
     *
     *  src and dst may be the same path.
     */
    bool applyToPath(SkPath* dst, const SkPath& src) const;

private:
    SkScalar        fWidth;
    SkScalar        fMiterLimit;
    SkPaint::Cap    fCap;
    SkPaint::Join   fJoin;
    bool            fStrokeAndFill;
};

/** \class SkPathEffect

    SkPathEffect is the base class for objects in the SkPaint that affect
    the geometry of a drawing primitive before it is transformed by the
    canvas' matrix and drawn.

    Dashing is implemented as a subclass of SkPathEffect.
*/
class SK_API SkPathEffect : public SkFlattenable {
public:
    SK_DECLARE_INST_COUNT(SkPathEffect)

    SkPathEffect() {}

    /**
     *  Given a src path (input) and a stroke-rec (input and output), apply
     *  this effect to the src path, returning the new path in dst, and return
     *  true. If this effect cannot be applied, return false and ignore dst
     *  and stroke-rec.
     *
     *  The stroke-rec specifies the initial request for stroking (if any).
     *  The effect can treat this as input only, or it can choose to change
     *  the rec as well. For example, the effect can decide to change the
     *  stroke's width or join, or the effect can change the rec from stroke
     *  to fill (or fill to stroke) in addition to returning a new (dst) path.
     *
     *  If this method returns true, the caller will apply (as needed) the
     *  resulting stroke-rec to dst and then draw.
     */
    virtual bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*) = 0;

    /**
     *  Compute a conservative bounds for its effect, given the src bounds.
     *  The baseline implementation just assigns src to dst.
     */
    virtual void computeFastBounds(SkRect* dst, const SkRect& src);

protected:
    SkPathEffect(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

private:
    // illegal
    SkPathEffect(const SkPathEffect&);
    SkPathEffect& operator=(const SkPathEffect&);

    typedef SkFlattenable INHERITED;
};

/** \class SkPairPathEffect

    Common baseclass for Compose and Sum. This subclass manages two pathEffects,
    including flattening them. It does nothing in filterPath, and is only useful
    for managing the lifetimes of its two arguments.
*/
class SkPairPathEffect : public SkPathEffect {
public:
    SkPairPathEffect(SkPathEffect* pe0, SkPathEffect* pe1);
    virtual ~SkPairPathEffect();

protected:
    SkPairPathEffect(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    // these are visible to our subclasses
    SkPathEffect* fPE0, *fPE1;

private:
    typedef SkPathEffect INHERITED;
};

/** \class SkComposePathEffect

    This subclass of SkPathEffect composes its two arguments, to create
    a compound pathEffect.
*/
class SkComposePathEffect : public SkPairPathEffect {
public:
    /** Construct a pathEffect whose effect is to apply first the inner pathEffect
        and the the outer pathEffect (e.g. outer(inner(path)))
        The reference counts for outer and inner are both incremented in the constructor,
        and decremented in the destructor.
    */
    SkComposePathEffect(SkPathEffect* outer, SkPathEffect* inner)
        : INHERITED(outer, inner) {}

    virtual bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposePathEffect)

protected:
    SkComposePathEffect(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

private:
    // illegal
    SkComposePathEffect(const SkComposePathEffect&);
    SkComposePathEffect& operator=(const SkComposePathEffect&);

    typedef SkPairPathEffect INHERITED;
};

/** \class SkSumPathEffect

    This subclass of SkPathEffect applies two pathEffects, one after the other.
    Its filterPath() returns true if either of the effects succeeded.
*/
class SkSumPathEffect : public SkPairPathEffect {
public:
    /** Construct a pathEffect whose effect is to apply two effects, in sequence.
        (e.g. first(path) + second(path))
        The reference counts for first and second are both incremented in the constructor,
        and decremented in the destructor.
    */
    SkSumPathEffect(SkPathEffect* first, SkPathEffect* second)
        : INHERITED(first, second) {}

    virtual bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSumPathEffect)

protected:
    SkSumPathEffect(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

private:
    // illegal
    SkSumPathEffect(const SkSumPathEffect&);
    SkSumPathEffect& operator=(const SkSumPathEffect&);

    typedef SkPairPathEffect INHERITED;
};

#endif

