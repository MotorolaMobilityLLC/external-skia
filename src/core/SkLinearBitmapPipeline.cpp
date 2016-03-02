/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLinearBitmapPipeline.h"
#include "SkPM4f.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include "SkColor.h"
#include "SkSize.h"
#include <tuple>

// Tweak ABI of functions that pass Sk4f by value to pass them via registers.
  #if defined(_MSC_VER) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
     #define VECTORCALL __vectorcall
  #elif defined(SK_CPU_ARM32) && defined(SK_ARM_HAS_NEON)
     #define VECTORCALL __attribute__((pcs("aapcs-vfp")))
  #else
     #define VECTORCALL
  #endif

namespace {
struct X {
    explicit X(SkScalar val) : fVal{val} { }
    explicit X(SkPoint pt)   : fVal{pt.fX} { }
    explicit X(SkSize s)     : fVal{s.fWidth} { }
    explicit X(SkISize s)    : fVal(s.fWidth) { }
    operator SkScalar () const {return fVal;}
private:
    SkScalar fVal;
};

struct Y {
    explicit Y(SkScalar val) : fVal{val} { }
    explicit Y(SkPoint pt)   : fVal{pt.fY} { }
    explicit Y(SkSize s)     : fVal{s.fHeight} { }
    explicit Y(SkISize s)    : fVal(s.fHeight) { }
    operator SkScalar () const {return fVal;}
private:
    SkScalar fVal;
};

// The Span class enables efficient processing horizontal spans of pixels.
// * start - the point where to start the span.
// * length - the number of pixels to traverse in source space.
// * count - the number of pixels to produce in destination space.
// Both start and length are mapped through the inversion matrix to produce values in source
// space. After the matrix operation, the tilers may break the spans up into smaller spans.
// The tilers can produce spans that seem nonsensical.
// * The clamp tiler can create spans with length of 0. This indicates to copy an edge pixel out
//   to the edge of the destination scan.
// * The mirror tiler can produce spans with negative length. This indicates that the source
//   should be traversed in the opposite direction to the destination pixels.
class Span {
public:
    Span(SkPoint start, SkScalar length, int count)
            : fStart(start)
            , fLength(length)
            , fCount{count} {
        SkASSERT(std::isfinite(length));
    }

    operator std::tuple<SkPoint&, SkScalar&, int&>() {
        return std::tie(fStart, fLength, fCount);
    }

    bool isEmpty() const { return 0 == fCount; }
    SkScalar length() const { return fLength; }
    SkScalar startX() const { return X(fStart); }
    SkScalar endX() const { return startX() + length(); }
    void clear() {
        fCount = 0;
    }

    bool completelyWithin(SkScalar xMin, SkScalar xMax) const {
        SkScalar sMin, sMax;
        std::tie(sMin, sMax) = std::minmax(startX(), endX());
        return xMin <= sMin && sMax <= xMax;
    }

    void offset(SkScalar offsetX) {
        fStart.offset(offsetX, 0.0f);
    }

    Span breakAt(SkScalar breakX, SkScalar dx) {
        SkASSERT(std::isfinite(breakX));
        SkASSERT(std::isfinite(dx));
        SkASSERT(dx != 0.0f);

        if (this->isEmpty()) {
            return Span{{0.0, 0.0}, 0.0f, 0};
        }

        int dxSteps = SkScalarFloorToInt((breakX - this->startX()) / dx);
        if (dxSteps < 0) {
            // The span is wholly after breakX.
            return Span{{0.0, 0.0}, 0.0f, 0};
        } else if (dxSteps > fCount) {
            // The span is wholly before breakX.
            Span answer = *this;
            this->clear();
            return answer;
        }

        // Calculate the values for the span to cleave off.
        SkPoint newStart = fStart;
        SkScalar newLength = dxSteps * dx;
        int newCount = dxSteps + 1;
        SkASSERT(newCount > 0);

        // Update this span to reflect the break.
        SkScalar lengthToStart = newLength + dx;
        fLength -= lengthToStart;
        fCount -= newCount;
        fStart = {this->startX() + lengthToStart, Y(fStart)};

        return Span{newStart, newLength, newCount};
    }

    void clampToSinglePixel(SkPoint pixel) {
        fStart = pixel;
        fLength = 0.0f;
    }

private:
    SkPoint  fStart;
    SkScalar fLength;
    int      fCount;
};

// BilerpSpans are similar to Spans, but they represent four source samples converting to single
// destination pixel per count. The pixels for the four samples are collect along two horizontal
// lines; one starting at {x, y0} and the other starting at {x, y1}. There are two distinct lines
// to deal with the edge case of the tile mode. For example, y0 may be at the last y position in
// a tile while y1 would be at the first.
// The step of a Bilerp (dx) is still length / (count - 1) and the start to the next sample is
// still dx * count, but the bounds are complicated by the sampling kernel so that the pixels
// touched are from x to x + length + 1.
class BilerpSpan {
public:
    BilerpSpan(SkScalar x, SkScalar y0, SkScalar y1, SkScalar length, int count)
        : fX{x}, fY0{y0}, fY1{y1}, fLength{length}, fCount{count} {
        SkASSERT(count >= 0);
        SkASSERT(std::isfinite(length));
        SkASSERT(std::isfinite(x));
        SkASSERT(std::isfinite(y0));
        SkASSERT(std::isfinite(y1));
    }

    operator std::tuple<SkScalar&, SkScalar&, SkScalar&, SkScalar&, int&>() {
        return std::tie(fX, fY0, fY1, fLength, fCount);
    }

    bool isEmpty() const { return 0 == fCount; }

private:
    SkScalar fX;
    SkScalar fY0;
    SkScalar fY1;
    SkScalar fLength;
    int      fCount;
};
}  // namespace

class SkLinearBitmapPipeline::PointProcessorInterface {
public:
    virtual ~PointProcessorInterface() { }
    virtual void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) = 0;
    virtual void VECTORCALL pointList4(Sk4s xs, Sk4s ys) = 0;
    virtual void pointSpan(Span span) = 0;
};

class SkLinearBitmapPipeline::BilerpProcessorInterface
    : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    // The x's and y's are setup in the following order:
    // +--------+--------+
    // |        |        |
    // |  px00  |  px10  |
    // |    0   |    1   |
    // +--------+--------+
    // |        |        |
    // |  px01  |  px11  |
    // |    2   |    3   |
    // +--------+--------+
    // These pixels coordinates are arranged in the following order in xs and ys:
    // px00  px10  px01  px11
    virtual void VECTORCALL bilerpList(Sk4s xs, Sk4s ys) = 0;
    virtual void bilerpSpan(BilerpSpan span) = 0;
};

class SkLinearBitmapPipeline::PixelPlacerInterface {
public:
    virtual ~PixelPlacerInterface() { }
    virtual void setDestination(SkPM4f* dst) = 0;
    virtual void VECTORCALL placePixel(Sk4f pixel0) = 0;
    virtual void VECTORCALL place4Pixels(Sk4f p0, Sk4f p1, Sk4f p2, Sk4f p3) = 0;
};

namespace  {
template <typename Stage>
void span_fallback(Span span, Stage* stage) {
    SkPoint start;
    SkScalar length;
    int count;
    std::tie(start, length, count) = span;
    Sk4f xs{X(start)};
    Sk4f ys{Y(start)};

    // Initializing this is not needed, but some compilers can't figure this out.
    Sk4s fourDx{0.0f};
    if (count > 1) {
        SkScalar dx = length / (count - 1);
        xs = xs + Sk4f{0.0f, 1.0f, 2.0f, 3.0f} * dx;
        // Only used if count is >= 4.
        fourDx = Sk4f{4.0f * dx};
    }

    while (count >= 4) {
        stage->pointList4(xs, ys);
        xs = xs + fourDx;
        count -= 4;
    }
    if (count > 0) {
        stage->pointListFew(count, xs, ys);
    }
}

template <typename Next>
void bilerp_span_fallback(BilerpSpan span, Next* next) {
    SkScalar x, y0, y1; SkScalar length; int count;
    std::tie(x, y0, y1, length, count) = span;

    SkASSERT(!span.isEmpty());
    float dx = length / (count - 1);

    Sk4f xs = Sk4f{x} + Sk4f{0.0f,  1.0f, 0.0f, 1.0f};
    Sk4f ys = Sk4f{y0, y0,  y1, y1};

    // If count == 1 then dx will be inf or NaN, but that is ok because the resulting addition is
    // never used.
    while (count > 0) {
        next->bilerpList(xs, ys);
        xs = xs + dx;
        count -= 1;
    }
}

// PointProcessor uses a strategy to help complete the work of the different stages. The strategy
// must implement the following methods:
// * processPoints(xs, ys) - must mutate the xs and ys for the stage.
// * maybeProcessSpan(span, next) - This represents a horizontal series of pixels
//   to work over.
//   span - encapsulation of span.
//   next - a pointer to the next stage.
//   maybeProcessSpan - returns false if it can not process the span and needs to fallback to
//                      point lists for processing.
template<typename Strategy, typename Next>
class PointProcessor final : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    template <typename... Args>
    PointProcessor(Next* next, Args&&... args)
        : fNext{next}
        , fStrategy{std::forward<Args>(args)...}{ }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointListFew(n, xs, ys);
    }

    void VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointList4(xs, ys);
    }

    // The span you pass must not be empty.
    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        if (!fStrategy.maybeProcessSpan(span, fNext)) {
            span_fallback(span, this);
        }
    }

private:
    Next* const fNext;
    Strategy fStrategy;
};

// See PointProcessor for responsibilities of Strategy.
template<typename Strategy, typename Next>
class BilerpProcessor final : public SkLinearBitmapPipeline::BilerpProcessorInterface  {
public:
    template <typename... Args>
    BilerpProcessor(Next* next, Args&&... args)
        : fNext{next}
        , fStrategy{std::forward<Args>(args)...}{ }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointListFew(n, xs, ys);
    }

    void VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointList4(xs, ys);
    }

    void VECTORCALL bilerpList(Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->bilerpList(xs, ys);
    }

    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        if (!fStrategy.maybeProcessSpan(span, fNext)) {
            span_fallback(span, this);
        }
    }

    void bilerpSpan(BilerpSpan bSpan) override {
        SkASSERT(!bSpan.isEmpty());
        if (!fStrategy.maybeProcessBilerpSpan(bSpan, fNext)) {
            bilerp_span_fallback(bSpan, this);
        }
    }

private:
    Next* const fNext;
    Strategy fStrategy;
};

class TranslateMatrixStrategy {
public:
    TranslateMatrixStrategy(SkVector offset)
        : fXOffset{X(offset)}
        , fYOffset{Y(offset)} { }

    void processPoints(Sk4s* xs, Sk4s* ys) {
        *xs = *xs + fXOffset;
        *ys = *ys + fYOffset;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        next->pointSpan(Span{start + SkPoint{fXOffset[0], fYOffset[0]}, length, count});
        return true;
    }

private:
    const Sk4s fXOffset, fYOffset;
};
template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using TranslateMatrix = PointProcessor<TranslateMatrixStrategy, Next>;

class ScaleMatrixStrategy {
public:
    ScaleMatrixStrategy(SkVector offset, SkVector scale)
        : fXOffset{X(offset)}, fYOffset{Y(offset)}
        ,  fXScale{X(scale)},   fYScale{Y(scale)} { }
    void processPoints(Sk4s* xs, Sk4s* ys) {
        *xs = *xs * fXScale + fXOffset;
        *ys = *ys * fYScale + fYOffset;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkPoint newStart =
            SkPoint{X(start) * fXScale[0] + fXOffset[0], Y(start) * fYScale[0] + fYOffset[0]};
        SkScalar newLength = length * fXScale[0];
        next->pointSpan(Span{newStart, newLength, count});
        return true;
    }

private:
    const Sk4s fXOffset, fYOffset;
    const Sk4s fXScale, fYScale;
};
template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using ScaleMatrix = PointProcessor<ScaleMatrixStrategy, Next>;

class AffineMatrixStrategy {
public:
    AffineMatrixStrategy(SkVector offset, SkVector scale, SkVector skew)
        : fXOffset{X(offset)}, fYOffset{Y(offset)}
        , fXScale{X(scale)},   fYScale{Y(scale)}
        , fXSkew{X(skew)},     fYSkew{Y(skew)} { }
    void processPoints(Sk4s* xs, Sk4s* ys) {
        Sk4s newXs = fXScale * *xs +  fXSkew * *ys + fXOffset;
        Sk4s newYs =  fYSkew * *xs + fYScale * *ys + fYOffset;

        *xs = newXs;
        *ys = newYs;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) {
        return false;
    }

private:
    const Sk4s fXOffset, fYOffset;
    const Sk4s fXScale,  fYScale;
    const Sk4s fXSkew,   fYSkew;
};
template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using AffineMatrix = PointProcessor<AffineMatrixStrategy, Next>;

static SkLinearBitmapPipeline::PointProcessorInterface* choose_matrix(
    SkLinearBitmapPipeline::PointProcessorInterface* next,
    const SkMatrix& inverse,
    SkLinearBitmapPipeline::MatrixStage* matrixProc) {
    if (inverse.hasPerspective()) {
        SkFAIL("Not implemented.");
    } else if (inverse.getSkewX() != 0.0f || inverse.getSkewY() != 0.0f) {
        matrixProc->Initialize<AffineMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()},
            SkVector{inverse.getScaleX(), inverse.getScaleY()},
            SkVector{inverse.getSkewX(), inverse.getSkewY()});
    } else if (inverse.getScaleX() != 1.0f || inverse.getScaleY() != 1.0f) {
        matrixProc->Initialize<ScaleMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()},
            SkVector{inverse.getScaleX(), inverse.getScaleY()});
    } else if (inverse.getTranslateX() != 0.0f || inverse.getTranslateY() != 0.0f) {
        matrixProc->Initialize<TranslateMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()});
    } else {
        return next;
    }
    return matrixProc->get();
}

template <typename Next = SkLinearBitmapPipeline::BilerpProcessorInterface>
class ExpandBilerp final : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    ExpandBilerp(Next* next) : fNext{next} { }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(0 < n && n < 4);
        //                    px00   px10   px01  px11
        const Sk4s kXOffsets{-0.5f,  0.5f, -0.5f, 0.5f},
                   kYOffsets{-0.5f, -0.5f,  0.5f, 0.5f};
        if (n >= 1) fNext->bilerpList(Sk4s{xs[0]} + kXOffsets, Sk4s{ys[0]} + kYOffsets);
        if (n >= 2) fNext->bilerpList(Sk4s{xs[1]} + kXOffsets, Sk4s{ys[1]} + kYOffsets);
        if (n >= 3) fNext->bilerpList(Sk4s{xs[2]} + kXOffsets, Sk4s{ys[2]} + kYOffsets);
    }

    void VECTORCALL pointList4(Sk4f xs, Sk4f ys) override {
        //                    px00   px10   px01  px11
        const Sk4f kXOffsets{-0.5f,  0.5f, -0.5f, 0.5f},
                   kYOffsets{-0.5f, -0.5f,  0.5f, 0.5f};
        fNext->bilerpList(Sk4s{xs[0]} + kXOffsets, Sk4s{ys[0]} + kYOffsets);
        fNext->bilerpList(Sk4s{xs[1]} + kXOffsets, Sk4s{ys[1]} + kYOffsets);
        fNext->bilerpList(Sk4s{xs[2]} + kXOffsets, Sk4s{ys[2]} + kYOffsets);
        fNext->bilerpList(Sk4s{xs[3]} + kXOffsets, Sk4s{ys[3]} + kYOffsets);
    }

    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        // Adjust the span so that it is in the correct phase with the pixel.
        BilerpSpan bSpan{X(start) - 0.5f, Y(start) - 0.5f, Y(start) + 0.5f, length, count};
        fNext->bilerpSpan(bSpan);
    }

private:
    Next* const fNext;
};

static SkLinearBitmapPipeline::PointProcessorInterface* choose_filter(
    SkLinearBitmapPipeline::BilerpProcessorInterface* next,
    SkFilterQuality filterQuailty,
    SkLinearBitmapPipeline::FilterStage* filterProc) {
    if (SkFilterQuality::kNone_SkFilterQuality == filterQuailty) {
        return next;
    } else {
        filterProc->Initialize<ExpandBilerp<>>(next);
        return filterProc->get();
    }
}

class ClampStrategy {
public:
    ClampStrategy(X max)
        : fXMin{0.0f}
        , fXMax{max - 1.0f} { }
    ClampStrategy(Y max)
        : fYMin{0.0f}
        , fYMax{max - 1.0f} { }
    ClampStrategy(SkSize max)
        : fXMin{0.0f}
        , fYMin{0.0f}
        , fXMax{X(max) - 1.0f}
        , fYMax{Y(max) - 1.0f} { }

    void processPoints(Sk4s* xs, Sk4s* ys) {
        *xs = Sk4s::Min(Sk4s::Max(*xs, fXMin), fXMax);
        *ys = Sk4s::Min(Sk4s::Max(*ys, fYMin), fYMax);
    }

    template <typename Next>
    bool maybeProcessSpan(Span originalSpan, Next* next) {
        SkASSERT(!originalSpan.isEmpty());
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = originalSpan;
        SkScalar xMin = fXMin[0];
        SkScalar xMax = fXMax[0] + 1.0f;
        SkScalar yMin = fYMin[0];
        SkScalar yMax = fYMax[0];
        SkScalar x = X(start);
        SkScalar y = std::min(std::max<SkScalar>(yMin, Y(start)), yMax);

        Span span{{x, y}, length, count};

        if (span.completelyWithin(xMin, xMax)) {
            next->pointSpan(span);
            return true;
        }
        if (1 == count || 0.0f == length) {
            return false;
        }

        SkScalar dx = length / (count - 1);

        //    A                 B     C
        // +-------+-------+-------++-------+-------+-------+     +-------+-------++------
        // |  *---*|---*---|*---*--||-*---*-|---*---|*---...|     |--*---*|---*---||*---*....
        // |       |       |       ||       |       |       | ... |       |       ||
        // |       |       |       ||       |       |       |     |       |       ||
        // +-------+-------+-------++-------+-------+-------+     +-------+-------++------
        //                         ^                                              ^
        //                         | xMin                                  xMax-1 | xMax
        //
        //     *---*---*---... - track of samples. * = sample
        //
        //     +-+                                 ||
        //     | |  - pixels in source space.      || - tile border.
        //     +-+                                 ||
        //
        // The length from A to B is the length in source space or 4 * dx or (count - 1) * dx
        // where dx is the distance between samples. There are 5 destination pixels
        // corresponding to 5 samples specified in the A, B span. The distance from A to the next
        // span starting at C is 5 * dx, so count * dx.
        // Remember, count is the number of pixels needed for the destination and the number of
        // samples.
        // Overall Strategy:
        // * Under - for portions of the span < xMin, take the color at pixel {xMin, y} and use it
        //   to fill in the 5 pixel sampled from A to B.
        // * Middle - for the portion of the span between xMin and xMax sample normally.
        // * Over - for the portion of the span > xMax, take the color at pixel {xMax-1, y} and
        //   use it to fill in the rest of the destination pixels.
        if (dx >= 0) {
            Span leftClamped = span.breakAt(xMin, dx);
            if (!leftClamped.isEmpty()) {
                leftClamped.clampToSinglePixel({xMin, y});
                next->pointSpan(leftClamped);
            }
            Span middle = span.breakAt(xMax, dx);
            if (!middle.isEmpty()) {
                next->pointSpan(middle);
            }
            if (!span.isEmpty()) {
                span.clampToSinglePixel({xMax - 1, y});
                next->pointSpan(span);
            }
        } else {
            Span rightClamped = span.breakAt(xMax, dx);
            if (!rightClamped.isEmpty()) {
                rightClamped.clampToSinglePixel({xMax - 1, y});
                next->pointSpan(rightClamped);
            }
            Span middle = span.breakAt(xMin, dx);
            if (!middle.isEmpty()) {
                next->pointSpan(middle);
            }
            if (!span.isEmpty()) {
                span.clampToSinglePixel({xMin, y});
                next->pointSpan(span);
            }
        }
        return true;
    }

    template <typename Next>
    bool maybeProcessBilerpSpan(BilerpSpan bSpan, Next* next) {
        return false;
    }

private:
    const Sk4s fXMin{SK_FloatNegativeInfinity};
    const Sk4s fYMin{SK_FloatNegativeInfinity};
    const Sk4s fXMax{SK_FloatInfinity};
    const Sk4s fYMax{SK_FloatInfinity};
};
template <typename Next = SkLinearBitmapPipeline::BilerpProcessorInterface>
using Clamp = BilerpProcessor<ClampStrategy, Next>;

static SkScalar tile_mod(SkScalar x, SkScalar base) {
    return x - std::floor(x / base) * base;
}

class RepeatStrategy {
public:
    RepeatStrategy(X max) : fXMax{max}, fXInvMax{1.0f/max} { }
    RepeatStrategy(Y max) : fYMax{max}, fYInvMax{1.0f/max} { }
    RepeatStrategy(SkSize max)
        : fXMax{X(max)}
        , fXInvMax{1.0f / X(max)}
        , fYMax{Y(max)}
        , fYInvMax{1.0f / Y(max)} { }

    void processPoints(Sk4s* xs, Sk4s* ys) {
        Sk4s divX = (*xs * fXInvMax).floor();
        Sk4s divY = (*ys * fYInvMax).floor();
        Sk4s baseX = (divX * fXMax);
        Sk4s baseY = (divY * fYMax);
        *xs = *xs - baseX;
        *ys = *ys - baseY;
    }

    template <typename Next>
    bool maybeProcessSpan(Span originalSpan, Next* next) {
        SkASSERT(!originalSpan.isEmpty());
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = originalSpan;
        // Make x and y in range on the tile.
        SkScalar x = tile_mod(X(start), fXMax[0]);
        SkScalar y = tile_mod(Y(start), fYMax[0]);
        SkScalar xMax = fXMax[0];
        SkScalar xMin = 0.0f;
        SkScalar dx = length / (count - 1);

        // No need trying to go fast because the steps are larger than a tile or there is one point.
        if (SkScalarAbs(dx) >= xMax || count <= 1) {
            return false;
        }

        //             A        B     C                  D                Z
        // +-------+-------+-------++-------+-------+-------++     +-------+-------++------
        // |       |   *---|*---*--||-*---*-|---*---|*---*--||     |--*---*|       ||
        // |       |       |       ||       |       |       || ... |       |       ||
        // |       |       |       ||       |       |       ||     |       |       ||
        // +-------+-------+-------++-------+-------+-------++     +-------+-------++------
        //                         ^^                       ^^                     ^^
        //                    xMax || xMin             xMax || xMin           xMax || xMin
        //
        //     *---*---*---... - track of samples. * = sample
        //
        //     +-+                                 ||
        //     | |  - pixels in source space.      || - tile border.
        //     +-+                                 ||
        //
        //
        // The given span starts at A and continues on through several tiles to sample point Z.
        // The idea is to break this into several spans one on each tile the entire span
        // intersects. The A to B span only covers a partial tile and has a count of 3 and the
        // distance from A to B is (count - 1) * dx or 2 * dx. The distance from A to the start of
        // the next span is count * dx or 3 * dx. Span C to D covers an entire tile has a count
        // of 5 and a length of 4 * dx. Remember, count is the number of pixels needed for the
        // destination and the number of samples.
        //
        // Overall Strategy:
        // While the span hangs over the edge of the tile, draw the span covering the tile then
        // slide the span over to the next tile.

        // The guard could have been count > 0, but then a bunch of math would be done in the
        // common case.

        Span span({x,y}, length, count);
        if (dx > 0) {
            while (!span.isEmpty() && span.endX() > xMax) {
                Span toDraw = span.breakAt(xMax, dx);
                next->pointSpan(toDraw);
                span.offset(-xMax);
            }
        } else {
            while (!span.isEmpty() && span.endX() < xMin) {
                Span toDraw = span.breakAt(xMin, dx);
                next->pointSpan(toDraw);
                span.offset(xMax);
            }
        }

        // All on a single tile.
        if (!span.isEmpty()) {
            next->pointSpan(span);
        }

        return true;
    }

    template <typename Next>
    bool maybeProcessBilerpSpan(BilerpSpan bSpan, Next* next) {
        return false;
    }

private:
    const Sk4s fXMax{0.0f};
    const Sk4s fXInvMax{0.0f};
    const Sk4s fYMax{0.0f};
    const Sk4s fYInvMax{0.0f};
};

template <typename Next = SkLinearBitmapPipeline::BilerpProcessorInterface>
using Repeat = BilerpProcessor<RepeatStrategy, Next>;

static SkLinearBitmapPipeline::BilerpProcessorInterface* choose_tiler(
    SkLinearBitmapPipeline::BilerpProcessorInterface* next,
    SkSize dimensions,
    SkShader::TileMode xMode,
    SkShader::TileMode yMode,
    SkLinearBitmapPipeline::TileStage* tileProcXOrBoth,
    SkLinearBitmapPipeline::TileStage* tileProcY) {
    if (xMode == yMode) {
        switch (xMode) {
            case SkShader::kClamp_TileMode:
                tileProcXOrBoth->Initialize<Clamp<>>(next, dimensions);
                break;
            case SkShader::kRepeat_TileMode:
                tileProcXOrBoth->Initialize<Repeat<>>(next, dimensions);
                break;
            case SkShader::kMirror_TileMode:
                SkFAIL("Not implemented.");
                break;
        }
    } else {
        switch (yMode) {
            case SkShader::kClamp_TileMode:
                tileProcY->Initialize<Clamp<>>(next, Y(dimensions));
                break;
            case SkShader::kRepeat_TileMode:
                tileProcY->Initialize<Repeat<>>(next, Y(dimensions));
                break;
            case SkShader::kMirror_TileMode:
                SkFAIL("Not implemented.");
                break;
        }
        switch (xMode) {
            case SkShader::kClamp_TileMode:
                tileProcXOrBoth->Initialize<Clamp<>>(tileProcY->get(), X(dimensions));
                break;
            case SkShader::kRepeat_TileMode:
                tileProcXOrBoth->Initialize<Repeat<>>(tileProcY->get(), X(dimensions));
                break;
            case SkShader::kMirror_TileMode:
                SkFAIL("Not implemented.");
                break;
        }
    }
    return tileProcXOrBoth->get();
}

class sRGBFast {
public:
    static Sk4s VECTORCALL sRGBToLinear(Sk4s pixel) {
        Sk4s l = pixel * pixel;
        return Sk4s{l[0], l[1], l[2], pixel[3]};
    }
};

enum class ColorOrder {
    kRGBA = false,
    kBGRA = true,
};
template <SkColorProfileType colorProfile, ColorOrder colorOrder>
class Pixel8888 {
public:
    Pixel8888(int width, const uint32_t* src) : fSrc{src}, fWidth{width}{ }
    Pixel8888(const SkPixmap& srcPixmap)
        : fSrc{srcPixmap.addr32()}
        , fWidth{static_cast<int>(srcPixmap.rowBytes() / 4)} { }

    void VECTORCALL getFewPixels(int n, Sk4s xs, Sk4s ys, Sk4f* px0, Sk4f* px1, Sk4f* px2) {
        Sk4i XIs = SkNx_cast<int, SkScalar>(xs);
        Sk4i YIs = SkNx_cast<int, SkScalar>(ys);
        Sk4i bufferLoc = YIs * fWidth + XIs;
        switch (n) {
            case 3:
                *px2 = this->getPixel(fSrc, bufferLoc[2]);
            case 2:
                *px1 = this->getPixel(fSrc, bufferLoc[1]);
            case 1:
                *px0 = this->getPixel(fSrc, bufferLoc[0]);
            default:
                break;
        }
    }

    void VECTORCALL get4Pixels(Sk4s xs, Sk4s ys, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) {
        Sk4i XIs = SkNx_cast<int, SkScalar>(xs);
        Sk4i YIs = SkNx_cast<int, SkScalar>(ys);
        Sk4i bufferLoc = YIs * fWidth + XIs;
        *px0 = this->getPixel(fSrc, bufferLoc[0]);
        *px1 = this->getPixel(fSrc, bufferLoc[1]);
        *px2 = this->getPixel(fSrc, bufferLoc[2]);
        *px3 = this->getPixel(fSrc, bufferLoc[3]);
    }

    void get4Pixels(const void* vsrc, int index, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) {
        const uint32_t* src = static_cast<const uint32_t*>(vsrc);
        *px0 = this->getPixel(src, index + 0);
        *px1 = this->getPixel(src, index + 1);
        *px2 = this->getPixel(src, index + 2);
        *px3 = this->getPixel(src, index + 3);
    }

    Sk4f getPixel(const void* vsrc, int index) {
        const uint32_t* src = static_cast<const uint32_t*>(vsrc);
        Sk4b bytePixel = Sk4b::Load((uint8_t *)(&src[index]));
        Sk4f pixel = SkNx_cast<float, uint8_t>(bytePixel);
        if (colorOrder == ColorOrder::kBGRA) {
            pixel = SkNx_shuffle<2, 1, 0, 3>(pixel);
        }
        pixel = pixel * Sk4f{1.0f/255.0f};
        if (colorProfile == kSRGB_SkColorProfileType) {
            pixel = sRGBFast::sRGBToLinear(pixel);
        }
        return pixel;
    }

    const uint32_t* row(int y) { return fSrc + y * fWidth[0]; }

private:
    const uint32_t* const fSrc;
    const Sk4i fWidth;
};

// Explaination of the math:
//              1 - x      x
//           +--------+--------+
//           |        |        |
//  1 - y    |  px00  |  px10  |
//           |        |        |
//           +--------+--------+
//           |        |        |
//    y      |  px01  |  px11  |
//           |        |        |
//           +--------+--------+
//
//
// Given a pixelxy each is multiplied by a different factor derived from the fractional part of x
// and y:
// * px00 -> (1 - x)(1 - y) = 1 - x - y + xy
// * px10 -> x(1 - y) = x - xy
// * px01 -> (1 - x)y = y - xy
// * px11 -> xy
// So x * y is calculated first and then used to calculate all the other factors.
static Sk4s VECTORCALL bilerp4(Sk4s xs, Sk4s ys, Sk4f px00, Sk4f px10,
                                                 Sk4f px01, Sk4f px11) {
    // Calculate fractional xs and ys.
    Sk4s fxs = xs - xs.floor();
    Sk4s fys = ys - ys.floor();
    Sk4s fxys{fxs * fys};
    Sk4f sum =  px11 * fxys;
    sum = sum + px01 * (fys - fxys);
    sum = sum + px10 * (fxs - fxys);
    sum = sum + px00 * (Sk4f{1.0f} - fxs - fys + fxys);
    return sum;
}

template <typename SourceStrategy>
class Sampler final : public SkLinearBitmapPipeline::BilerpProcessorInterface {
public:
    template <typename... Args>
    Sampler(SkLinearBitmapPipeline::PixelPlacerInterface* next, Args&&... args)
        : fNext{next}
        , fStrategy{std::forward<Args>(args)...} { }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(0 < n && n < 4);
        Sk4f px0, px1, px2;
        fStrategy.getFewPixels(n, xs, ys, &px0, &px1, &px2);
        if (n >= 1) fNext->placePixel(px0);
        if (n >= 2) fNext->placePixel(px1);
        if (n >= 3) fNext->placePixel(px2);
    }

    void VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        Sk4f px0, px1, px2, px3;
        fStrategy.get4Pixels(xs, ys, &px0, &px1, &px2, &px3);
        fNext->place4Pixels(px0, px1, px2, px3);
    }

    void VECTORCALL bilerpList(Sk4s xs, Sk4s ys) override {
        Sk4f px00, px10, px01, px11;
        fStrategy.get4Pixels(xs, ys, &px00, &px10, &px01, &px11);
        Sk4f pixel = bilerp4(xs, ys, px00, px10, px01, px11);
        fNext->placePixel(pixel);
    }

    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        if (length < (count - 1)) {
            this->pointSpanSlowRate(span);
        } else if (length == (count - 1)) {
            this->pointSpanUnitRate(span);
        } else {
            this->pointSpanFastRate(span);
        }
    }

private:
    // When moving through source space more slowly than dst space (zoomed in),
    // we'll be sampling from the same source pixel more than once.
    void pointSpanSlowRate(Span span) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkScalar x = X(start);
        SkFixed fx = SkScalarToFixed(x);
        SkScalar dx = length / (count - 1);
        SkFixed fdx = SkScalarToFixed(dx);

        const void* row = fStrategy.row((int)std::floor(Y(start)));
        SkLinearBitmapPipeline::PixelPlacerInterface* next = fNext;

        int ix = SkFixedFloorToInt(fx);
        int prevIX = ix;
        Sk4f fpixel = fStrategy.getPixel(row, ix);

        // When dx is less than one, each pixel is used more than once. Using the fixed point fx
        // allows the code to quickly check that the same pixel is being used. The code uses this
        // same pixel check to do the sRGB and normalization only once.
        auto getNextPixel = [&]() {
            if (ix != prevIX) {
                fpixel = fStrategy.getPixel(row, ix);
                prevIX = ix;
            }
            fx += fdx;
            ix = SkFixedFloorToInt(fx);
            return fpixel;
        };

        while (count >= 4) {
            Sk4f px0 = getNextPixel();
            Sk4f px1 = getNextPixel();
            Sk4f px2 = getNextPixel();
            Sk4f px3 = getNextPixel();
            next->place4Pixels(px0, px1, px2, px3);
            count -= 4;
        }
        while (count > 0) {
            next->placePixel(getNextPixel());
            count -= 1;
        }
    }

    // We're moving through source space at a rate of 1 source pixel per 1 dst pixel.
    // We'll never re-use pixels, but we can at least load contiguous pixels.
    void pointSpanUnitRate(Span span) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        int ix = SkScalarFloorToInt(X(start));
        const void* row = fStrategy.row((int)std::floor(Y(start)));
        SkLinearBitmapPipeline::PixelPlacerInterface* next = fNext;
        while (count >= 4) {
            Sk4f px0, px1, px2, px3;
            fStrategy.get4Pixels(row, ix, &px0, &px1, &px2, &px3);
            next->place4Pixels(px0, px1, px2, px3);
            ix += 4;
            count -= 4;
        }

        while (count > 0) {
            next->placePixel(fStrategy.getPixel(row, ix));
            ix += 1;
            count -= 1;
        }
    }

    // We're moving through source space faster than dst (zoomed out),
    // so we'll never reuse a source pixel or be able to do contiguous loads.
    void pointSpanFastRate(Span span) {
        span_fallback(span, this);
    }

    void bilerpSpan(BilerpSpan span) override {
        bilerp_span_fallback(span, this);
    }

private:
    SkLinearBitmapPipeline::PixelPlacerInterface* const fNext;
    SourceStrategy fStrategy;
};

using Pixel8888SRGB = Pixel8888<kSRGB_SkColorProfileType, ColorOrder::kRGBA>;
using Pixel8888LRGB = Pixel8888<kLinear_SkColorProfileType, ColorOrder::kRGBA>;
using Pixel8888SBGR = Pixel8888<kSRGB_SkColorProfileType, ColorOrder::kBGRA>;
using Pixel8888LBGR = Pixel8888<kLinear_SkColorProfileType, ColorOrder::kBGRA>;

static SkLinearBitmapPipeline::BilerpProcessorInterface* choose_pixel_sampler(
    SkLinearBitmapPipeline::PixelPlacerInterface* next,
    const SkPixmap& srcPixmap,
    SkLinearBitmapPipeline::SampleStage* sampleStage) {
    const SkImageInfo& imageInfo = srcPixmap.info();
    switch (imageInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            if (imageInfo.profileType() == kSRGB_SkColorProfileType) {
                sampleStage->Initialize<Sampler<Pixel8888SRGB>>(next, srcPixmap);
            } else {
                sampleStage->Initialize<Sampler<Pixel8888LRGB>>(next, srcPixmap);
            }
            break;
        case kBGRA_8888_SkColorType:
            if (imageInfo.profileType() == kSRGB_SkColorProfileType) {
                sampleStage->Initialize<Sampler<Pixel8888SBGR>>(next, srcPixmap);
            } else {
                sampleStage->Initialize<Sampler<Pixel8888LBGR>>(next, srcPixmap);
            }
            break;
        default:
            SkFAIL("Not implemented. Unsupported src");
            break;
    }
    return sampleStage->get();
}

template <SkAlphaType alphaType>
class PlaceFPPixel final : public SkLinearBitmapPipeline::PixelPlacerInterface {
public:
    void VECTORCALL placePixel(Sk4f pixel) override {
        PlacePixel(fDst, pixel, 0);
        fDst += 1;
    }

    void VECTORCALL place4Pixels(Sk4f p0, Sk4f p1, Sk4f p2, Sk4f p3) override {
        SkPM4f* dst = fDst;
        PlacePixel(dst, p0, 0);
        PlacePixel(dst, p1, 1);
        PlacePixel(dst, p2, 2);
        PlacePixel(dst, p3, 3);
        fDst += 4;
    }

    void setDestination(SkPM4f* dst) override {
        fDst = dst;
    }

private:
    static void VECTORCALL PlacePixel(SkPM4f* dst, Sk4f pixel, int index) {
        Sk4f newPixel = pixel;
        if (alphaType == kUnpremul_SkAlphaType) {
            newPixel = Premultiply(pixel);
        }
        newPixel.store(dst + index);
    }
    static Sk4f VECTORCALL Premultiply(Sk4f pixel) {
        float alpha = pixel[3];
        return pixel * Sk4f{alpha, alpha, alpha, 1.0f};
    }

    SkPM4f* fDst;
};

static SkLinearBitmapPipeline::PixelPlacerInterface* choose_pixel_placer(
    SkAlphaType alphaType,
    SkLinearBitmapPipeline::PixelStage* placerStage) {
    if (alphaType == kUnpremul_SkAlphaType) {
        placerStage->Initialize<PlaceFPPixel<kUnpremul_SkAlphaType>>();
    } else {
        // kOpaque_SkAlphaType is treated the same as kPremul_SkAlphaType
        placerStage->Initialize<PlaceFPPixel<kPremul_SkAlphaType>>();
    }
    return placerStage->get();
}
}  // namespace

SkLinearBitmapPipeline::~SkLinearBitmapPipeline() {}

SkLinearBitmapPipeline::SkLinearBitmapPipeline(
    const SkMatrix& inverse,
    SkFilterQuality filterQuality,
    SkShader::TileMode xTile, SkShader::TileMode yTile,
    const SkPixmap& srcPixmap) {
    SkSize size = SkSize::Make(srcPixmap.width(), srcPixmap.height());
    const SkImageInfo& srcImageInfo = srcPixmap.info();

    // As the stages are built, the chooser function may skip a stage. For example, with the
    // identity matrix, the matrix stage is skipped, and the tilerStage is the first stage.
    auto placementStage = choose_pixel_placer(srcImageInfo.alphaType(), &fPixelStage);
    auto samplerStage   = choose_pixel_sampler(placementStage, srcPixmap, &fSampleStage);
    auto tilerStage     = choose_tiler(samplerStage, size, xTile, yTile, &fTileXOrBothStage,
                                       &fTileYStage);
    auto filterStage    = choose_filter(tilerStage, filterQuality, &fFilterStage);
    fFirstStage         = choose_matrix(filterStage, inverse, &fMatrixStage);
}

void SkLinearBitmapPipeline::shadeSpan4f(int x, int y, SkPM4f* dst, int count) {
    SkASSERT(count > 0);
    fPixelStage->setDestination(dst);
    // The count and length arguments start out in a precise relation in order to keep the
    // math correct through the different stages. Count is the number of pixel to produce.
    // Since the code samples at pixel centers, length is the distance from the center of the
    // first pixel to the center of the last pixel. This implies that length is count-1.
    fFirstStage->pointSpan(Span{SkPoint{x + 0.5f, y + 0.5f}, count - 1.0f, count});
}
