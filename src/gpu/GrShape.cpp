/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShape.h"

GrShape& GrShape::operator=(const GrShape& that) {
    bool wasPath = Type::kPath == fType;
    fStyle = that.fStyle;
    fType = that.fType;
    switch (fType) {
        case Type::kEmpty:
            if (wasPath) {
                fPath.reset();
            }
            break;
        case Type::kRRect:
            if (wasPath) {
                fPath.reset();
            }
            fRRect = that.fRRect;
            fRRectDir = that.fRRectDir;
            fRRectStart = that.fRRectStart;
            fRRectIsInverted = that.fRRectIsInverted;
            break;
        case Type::kPath:
            if (wasPath) {
                *fPath.get() = *that.fPath.get();
            } else {
                fPath.set(*that.fPath.get());
            }
            break;
    }
    fInheritedKey.reset(that.fInheritedKey.count());
    sk_careful_memcpy(fInheritedKey.get(), that.fInheritedKey.get(),
                      sizeof(uint32_t) * fInheritedKey.count());
    return *this;
}

const SkRect& GrShape::bounds() const {
    static constexpr SkRect kEmpty = SkRect::MakeEmpty();
    switch (fType) {
        case Type::kEmpty:
            return kEmpty;
        case Type::kRRect:
            return fRRect.getBounds();
        case Type::kPath:
            return fPath.get()->getBounds();
    }
    SkFAIL("Unknown shape type");
    return kEmpty;
}

void GrShape::styledBounds(SkRect* bounds) const {
    if (Type::kEmpty == fType && !fStyle.hasNonDashPathEffect()) {
        *bounds = SkRect::MakeEmpty();
    } else {
        fStyle.adjustBounds(bounds, this->bounds());
    }
}

int GrShape::unstyledKeySize() const {
    if (fInheritedKey.count()) {
        return fInheritedKey.count();
    }
    switch (fType) {
        case Type::kEmpty:
            return 1;
        case Type::kRRect:
            SkASSERT(!fInheritedKey.count());
            SkASSERT(0 == SkRRect::kSizeInMemory % sizeof(uint32_t));
            // + 1 for the direction, start index, and inverseness.
            return SkRRect::kSizeInMemory / sizeof(uint32_t) + 1;
        case Type::kPath:
            if (fPath.get()->isVolatile()) {
                return -1;
            } else {
                // The key is the path ID and fill type.
                return 2;
            }
    }
    SkFAIL("Should never get here.");
    return 0;
}

void GrShape::writeUnstyledKey(uint32_t* key) const {
    SkASSERT(this->unstyledKeySize());
    SkDEBUGCODE(uint32_t* origKey = key;)
    if (fInheritedKey.count()) {
        memcpy(key, fInheritedKey.get(), sizeof(uint32_t) * fInheritedKey.count());
        SkDEBUGCODE(key += fInheritedKey.count();)
    } else {
        switch (fType) {
            case Type::kEmpty:
                *key++ = 1;
                break;
            case Type::kRRect:
                fRRect.writeToMemory(key);
                key += SkRRect::kSizeInMemory / sizeof(uint32_t);
                *key = (fRRectDir == SkPath::kCCW_Direction) ? (1 << 31) : 0;
                *key |= fRRectIsInverted ? (1 << 30) : 0;
                *key++ |= fRRectStart;
                SkASSERT(fRRectStart < 8);
                break;
            case Type::kPath:
                SkASSERT(!fPath.get()->isVolatile());
                *key++ = fPath.get()->getGenerationID();
                // We could canonicalize the fill rule for paths that don't differentiate between
                // even/odd or winding fill (e.g. convex).
                *key++ = fPath.get()->getFillType();
                break;
        }
    }
    SkASSERT(key - origKey == this->unstyledKeySize());
}

void GrShape::setInheritedKey(const GrShape &parent, GrStyle::Apply apply, SkScalar scale) {
    SkASSERT(!fInheritedKey.count());
    // If the output shape turns out to be simple, then we will just use its geometric key
    if (Type::kPath == fType) {
        // We want ApplyFullStyle(ApplyPathEffect(shape)) to have the same key as
        // ApplyFullStyle(shape).
        // The full key is structured as (geo,path_effect,stroke).
        // If we do ApplyPathEffect we get get,path_effect as the inherited key. If we then
        // do ApplyFullStyle we'll memcpy geo,path_effect into the new inherited key
        // and then append the style key (which should now be stroke only) at the end.
        int parentCnt = parent.fInheritedKey.count();
        bool useParentGeoKey = !parentCnt;
        if (useParentGeoKey) {
            parentCnt = parent.unstyledKeySize();
            if (parentCnt < 0) {
                // The parent's geometry has no key so we will have no key.
                fPath.get()->setIsVolatile(true);
                return;
            }
        }
        uint32_t styleKeyFlags = 0;
        if (parent.knownToBeClosed()) {
            styleKeyFlags |= GrStyle::kClosed_KeyFlag;
        }
        int styleCnt = GrStyle::KeySize(parent.fStyle, apply, styleKeyFlags);
        if (styleCnt < 0) {
            // The style doesn't allow a key, set the path to volatile so that we fail when
            // we try to get a key for the shape.
            fPath.get()->setIsVolatile(true);
            return;
        }
        fInheritedKey.reset(parentCnt + styleCnt);
        if (useParentGeoKey) {
            // This will be the geo key.
            parent.writeUnstyledKey(fInheritedKey.get());
        } else {
            // This should be (geo,path_effect).
            memcpy(fInheritedKey.get(), parent.fInheritedKey.get(),
                   parentCnt * sizeof(uint32_t));
        }
        // Now turn (geo,path_effect) or (geo) into (geo,path_effect,stroke)
        GrStyle::WriteKey(fInheritedKey.get() + parentCnt, parent.fStyle, apply, scale,
                          styleKeyFlags);
    }
}

GrShape::GrShape(const GrShape& that) : fType(that.fType), fStyle(that.fStyle) {
    switch (fType) {
        case Type::kEmpty:
            return;
        case Type::kRRect:
            fRRect = that.fRRect;
            fRRectDir = that.fRRectDir;
            fRRectStart = that.fRRectStart;
            fRRectIsInverted = that.fRRectIsInverted;
            return;
        case Type::kPath:
            fPath.set(*that.fPath.get());
            return;
    }
    fInheritedKey.reset(that.fInheritedKey.count());
    memcpy(fInheritedKey.get(), that.fInheritedKey.get(),
           sizeof(uint32_t) * fInheritedKey.count());
}

GrShape::GrShape(const GrShape& parent, GrStyle::Apply apply, SkScalar scale) {
    // TODO: Add some quantization of scale for better cache performance here or leave that up
    // to caller?
    // TODO: For certain shapes and stroke params we could ignore the scale. (e.g. miter or bevel
    // stroke of a rect).
    if (!parent.style().applies() ||
        (GrStyle::Apply::kPathEffectOnly == apply && !parent.style().pathEffect())) {
        fType = Type::kEmpty;
        *this = parent;
        return;
    }

    SkPathEffect* pe = parent.fStyle.pathEffect();
    SkTLazy<SkPath> tmpPath;
    const GrShape* parentForKey = &parent;
    SkTLazy<GrShape> tmpParent;
    fType = Type::kPath;
    fPath.init();
    if (pe) {
        SkPath* srcForPathEffect;
        if (parent.fType == Type::kPath) {
            srcForPathEffect = parent.fPath.get();
        } else {
            srcForPathEffect = tmpPath.init();
            parent.asPath(tmpPath.get());
        }
        // Should we consider bounds? Would have to include in key, but it'd be nice to know
        // if the bounds actually modified anything before including in key.
        SkStrokeRec strokeRec = parent.fStyle.strokeRec();
        if (!parent.fStyle.applyPathEffectToPath(fPath.get(), &strokeRec, *srcForPathEffect,
                                                 scale)) {
            // If the path effect fails then we continue as though there was no path effect.
            // If the original was a rrect that we couldn't canonicalize because of the path
            // effect, then do so now.
            if (parent.fType == Type::kRRect && (parent.fRRectDir != kDefaultRRectDir ||
                                                 parent.fRRectStart != kDefaultRRectStart)) {
                SkASSERT(srcForPathEffect == tmpPath.get());
                tmpPath.get()->reset();
                tmpPath.get()->addRRect(parent.fRRect, kDefaultRRectDir, kDefaultRRectDir);
            }
            *fPath.get() = *srcForPathEffect;
        }
        // A path effect has access to change the res scale but we aren't expecting it to and it
        // would mess up our key computation.
        SkASSERT(scale == strokeRec.getResScale());
        if (GrStyle::Apply::kPathEffectAndStrokeRec == apply && strokeRec.needToApply()) {
            // The intermediate shape may not be a general path. If we we're just applying
            // the path effect then attemptToReduceFromPath would catch it. This means that
            // when we subsequently applied the remaining strokeRec we would have a non-path
            // parent shape that would be used to determine the the stroked path's key.
            // We detect that case here and change parentForKey to a temporary that represents
            // the simpler shape so that applying both path effect and the strokerec all at
            // once produces the same key.
            tmpParent.init(*fPath.get(), GrStyle(strokeRec, nullptr));
            tmpParent.get()->setInheritedKey(parent, GrStyle::Apply::kPathEffectOnly, scale);
            if (!tmpPath.isValid()) {
                tmpPath.init();
            }
            tmpParent.get()->asPath(tmpPath.get());
            SkStrokeRec::InitStyle fillOrHairline;
            SkAssertResult(tmpParent.get()->style().applyToPath(fPath.get(), &fillOrHairline,
                                                                *tmpPath.get(), scale));
            fStyle.resetToInitStyle(fillOrHairline);
            parentForKey = tmpParent.get();
        } else {
            fStyle = GrStyle(strokeRec, nullptr);
        }
    } else {
        const SkPath* srcForParentStyle;
        if (parent.fType == Type::kPath) {
            srcForParentStyle = parent.fPath.get();
        } else {
            srcForParentStyle = tmpPath.init();
            parent.asPath(tmpPath.get());
        }
        SkStrokeRec::InitStyle fillOrHairline;
        SkASSERT(parent.fStyle.applies());
        SkASSERT(!parent.fStyle.pathEffect());
        SkAssertResult(parent.fStyle.applyToPath(fPath.get(), &fillOrHairline, *srcForParentStyle,
                                                 scale));
        fStyle.resetToInitStyle(fillOrHairline);
    }
    this->attemptToSimplifyPath();
    this->setInheritedKey(*parentForKey, apply, scale);
}

void GrShape::attemptToSimplifyPath() {
    SkASSERT(Type::kPath == fType);
    SkRect rect;
    if (fPath.get()->isEmpty()) {
        fType = Type::kEmpty;
    } else if (fPath.get()->isRRect(&fRRect, &fRRectDir, &fRRectStart)) {
        // Currently SkPath does not acknowledge that empty, rect, or oval subtypes as rrects.
        SkASSERT(!fRRect.isEmpty());
        SkASSERT(fRRect.getType() != SkRRect::kRect_Type);
        SkASSERT(fRRect.getType() != SkRRect::kOval_Type);
        fRRectIsInverted = fPath.get()->isInverseFillType();
        fType = Type::kRRect;
    } else if (fPath.get()->isOval(&rect, &fRRectDir, &fRRectStart)) {
        fRRect.setOval(rect);
        fRRectIsInverted = fPath.get()->isInverseFillType();
        // convert from oval indexing to rrect indexiing.
        fRRectStart *= 2;
        fType = Type::kRRect;
    } else if (SkPathPriv::IsSimpleClosedRect(*fPath.get(), &rect, &fRRectDir, &fRRectStart)) {
        // When there is a path effect we restrict rect detection to the narrower API that
        // gives us the starting position. Otherwise, we will retry with the more aggressive
        // isRect().
        fRRect.setRect(rect);
        fRRectIsInverted = fPath.get()->isInverseFillType();
        // convert from rect indexing to rrect indexiing.
        fRRectStart *= 2;
        fType = Type::kRRect;
    } else if (!this->style().hasPathEffect()) {
        bool closed;
        if (fPath.get()->isRect(&rect, &closed, nullptr)) {
            if (closed || this->style().isSimpleFill()) {
                fRRect.setRect(rect);
                // Since there is no path effect the dir and start index is immaterial.
                fRRectDir = kDefaultRRectDir;
                fRRectStart = kDefaultRRectStart;
                // There isn't dashing so we will have to preserver inverseness.
                fRRectIsInverted = fPath.get()->isInverseFillType();
                fType = Type::kRRect;
            }
        }
    }
    if (Type::kPath != fType) {
        fPath.reset();
        fInheritedKey.reset(0);
        if (Type::kRRect == fType) {
            this->attemptToSimplifyRRect();
        }
    }
}

void GrShape::attemptToSimplifyRRect() {
    SkASSERT(Type::kRRect == fType);
    SkASSERT(!fInheritedKey.count());
    if (fRRect.isEmpty()) {
        fType = Type::kEmpty;
        return;
    }
    if (!this->style().hasPathEffect()) {
        fRRectDir = kDefaultRRectDir;
        fRRectStart = kDefaultRRectStart;
    } else if (fStyle.isDashed()) {
        // Dashing ignores the inverseness (currently). skbug.com/5421
        fRRectIsInverted = false;
    }
}
