/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieAdapter.h"

#include "SkFont.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGGradient.h"
#include "SkSGGroup.h"
#include "SkSGPath.h"
#include "SkSGRect.h"
#include "SkSGText.h"
#include "SkSGTransform.h"
#include "SkSGTrimEffect.h"
#include "SkTextBlob.h"
#include "SkTextUtils.h"
#include "SkTo.h"
#include "SkUTF.h"
#include "SkottieValue.h"

#include <cmath>
#include <utility>

namespace skottie {

RRectAdapter::RRectAdapter(sk_sp<sksg::RRect> wrapped_node)
    : fRRectNode(std::move(wrapped_node)) {}

void RRectAdapter::apply() {
    // BM "position" == "center position"
    auto rr = SkRRect::MakeRectXY(SkRect::MakeXYWH(fPosition.x() - fSize.width() / 2,
                                                   fPosition.y() - fSize.height() / 2,
                                                   fSize.width(), fSize.height()),
                                  fRadius.width(),
                                  fRadius.height());
   fRRectNode->setRRect(rr);
}

TransformAdapter::TransformAdapter(sk_sp<sksg::Matrix> matrix)
    : fMatrixNode(std::move(matrix)) {}

SkMatrix TransformAdapter::totalMatrix() const {
    SkMatrix t = SkMatrix::MakeTrans(-fAnchorPoint.x(), -fAnchorPoint.y());

    t.postScale(fScale.x() / 100, fScale.y() / 100); // 100% based
    t.postRotate(fRotation);
    t.postTranslate(fPosition.x(), fPosition.y());
    // TODO: skew

    return t;
}

void TransformAdapter::apply() {
    fMatrixNode->setMatrix(this->totalMatrix());
}

PolyStarAdapter::PolyStarAdapter(sk_sp<sksg::Path> wrapped_node, Type t)
    : fPathNode(std::move(wrapped_node))
    , fType(t) {}

void PolyStarAdapter::apply() {
    static constexpr int kMaxPointCount = 100000;
    const auto count = SkToUInt(SkTPin(SkScalarRoundToInt(fPointCount), 0, kMaxPointCount));
    const auto arc   = sk_ieee_float_divide(SK_ScalarPI * 2, count);

    const auto pt_on_circle = [](const SkPoint& c, SkScalar r, SkScalar a) {
        return SkPoint::Make(c.x() + r * std::cos(a),
                             c.y() + r * std::sin(a));
    };

    // TODO: inner/outer "roundness"?

    SkPath poly;

    auto angle = SkDegreesToRadians(fRotation - 90);
    poly.moveTo(pt_on_circle(fPosition, fOuterRadius, angle));
    poly.incReserve(fType == Type::kStar ? count * 2 : count);

    for (unsigned i = 0; i < count; ++i) {
        if (fType == Type::kStar) {
            poly.lineTo(pt_on_circle(fPosition, fInnerRadius, angle + arc * 0.5f));
        }
        angle += arc;
        poly.lineTo(pt_on_circle(fPosition, fOuterRadius, angle));
    }

    poly.close();
    fPathNode->setPath(poly);
}

GradientAdapter::GradientAdapter(sk_sp<sksg::Gradient> grad, size_t stopCount)
    : fGradient(std::move(grad))
    , fStopCount(stopCount) {}

void GradientAdapter::apply() {
    this->onApply();

    // |fColorStops| holds |fStopCount| x [ pos, r, g, g ] + ? x [ pos, alpha ]

    if (fColorStops.size() < fStopCount * 4 || ((fColorStops.size() - fStopCount * 4) % 2)) {
        // apply() may get called before the stops are set, so only log when we have some stops.
        if (!fColorStops.empty()) {
            SkDebugf("!! Invalid gradient stop array size: %zu\n", fColorStops.size());
        }
        return;
    }

    std::vector<sksg::Gradient::ColorStop> stops;

    // TODO: merge/lerp opacity stops
    const auto csEnd = fColorStops.cbegin() + fStopCount * 4;
    for (auto cs = fColorStops.cbegin(); cs != csEnd; cs += 4) {
        const auto pos = cs[0];
        const VectorValue rgb({ cs[1], cs[2], cs[3] });

        stops.push_back({ pos, ValueTraits<VectorValue>::As<SkColor>(rgb) });
    }

    fGradient->setColorStops(std::move(stops));
}

LinearGradientAdapter::LinearGradientAdapter(sk_sp<sksg::LinearGradient> grad, size_t stopCount)
    : INHERITED(std::move(grad), stopCount) {}

void LinearGradientAdapter::onApply() {
    auto* grad = static_cast<sksg::LinearGradient*>(fGradient.get());
    grad->setStartPoint(this->startPoint());
    grad->setEndPoint(this->endPoint());
}

RadialGradientAdapter::RadialGradientAdapter(sk_sp<sksg::RadialGradient> grad, size_t stopCount)
    : INHERITED(std::move(grad), stopCount) {}

void RadialGradientAdapter::onApply() {
    auto* grad = static_cast<sksg::RadialGradient*>(fGradient.get());
    grad->setStartCenter(this->startPoint());
    grad->setEndCenter(this->startPoint());
    grad->setStartRadius(0);
    grad->setEndRadius(SkPoint::Distance(this->startPoint(), this->endPoint()));
}

TrimEffectAdapter::TrimEffectAdapter(sk_sp<sksg::TrimEffect> trimEffect)
    : fTrimEffect(std::move(trimEffect)) {
    SkASSERT(fTrimEffect);
}

void TrimEffectAdapter::apply() {
    // BM semantics: start/end are percentages, offset is "degrees" (?!).
    const auto  start = fStart  / 100,
                  end = fEnd    / 100,
               offset = fOffset / 360;

    auto startT = SkTMin(start, end) + offset,
          stopT = SkTMax(start, end) + offset;
    auto   mode = SkTrimPathEffect::Mode::kNormal;

    if (stopT - startT < 1) {
        startT -= SkScalarFloorToScalar(startT);
        stopT  -= SkScalarFloorToScalar(stopT);

        if (startT > stopT) {
            using std::swap;
            swap(startT, stopT);
            mode = SkTrimPathEffect::Mode::kInverted;
        }
    } else {
        startT = 0;
        stopT  = 1;
    }

    fTrimEffect->setStart(startT);
    fTrimEffect->setStop(stopT);
    fTrimEffect->setMode(mode);
}

TextAdapter::TextAdapter(sk_sp<sksg::Group> root)
    : fRoot(std::move(root))
    , fTextNode(sksg::TextBlob::Make())
    , fFillColor(sksg::Color::Make(SK_ColorTRANSPARENT))
    , fStrokeColor(sksg::Color::Make(SK_ColorTRANSPARENT))
    , fFillNode(sksg::Draw::Make(fTextNode, fFillColor))
    , fStrokeNode(sksg::Draw::Make(fTextNode, fStrokeColor))
    , fHadFill(false)
    , fHadStroke(false) {
    // Build a SG fragment with the following general format:
    //
    // [Group]
    //   [Draw]
    //     [FillPaint]
    //     [Text]*
    //   [Draw]
    //     [StrokePaint]
    //     [Text]*
    //
    // * where the text node is shared

    fFillColor->setAntiAlias(true);
    fStrokeColor->setAntiAlias(true);
    fStrokeColor->setStyle(SkPaint::kStroke_Style);
}

sk_sp<SkTextBlob> TextAdapter::makeBlob() const {
    // TODO: convert to SkFont (missing getFontSpacing, measureText).
    SkPaint font;
    font.setTypeface(fText.fTypeface);
    font.setTextSize(fText.fTextSize);
    font.setHinting(kNo_SkFontHinting);
    font.setSubpixelText(true);
    font.setAntiAlias(true);
    font.setTextEncoding(kUTF8_SkTextEncoding);

    const auto align_fract = [](SkTextUtils::Align align) {
        switch (align) {
        case SkTextUtils::kLeft_Align:   return  0.0f;
        case SkTextUtils::kCenter_Align: return -0.5f;
        case SkTextUtils::kRight_Align:  return -1.0f;
        }
        return 0.0f; // go home, msvc...
    }(fText.fAlign);

    const auto line_spacing = font.getFontSpacing();
    const auto blob_font    = SkFont::LEGACY_ExtractFromPaint(font);
    float y_off             = 0;
    SkSTArray<256, SkGlyphID, true> line_glyph_buffer;
    SkTextBlobBuilder builder;

    const auto& push_line = [&](const char* start, const char* end) {
        if (end > start) {
            const auto len   = SkToSizeT(end - start);
            line_glyph_buffer.reset(font.textToGlyphs(start, len, nullptr));
            SkAssertResult(font.textToGlyphs(start, len, line_glyph_buffer.data())
                           == line_glyph_buffer.count());

            const auto x_off = align_fract != 0
                    ? align_fract * font.measureText(start, len)
                    : 0;
            const auto& buf  = builder.allocRun(blob_font, line_glyph_buffer.count(), x_off, y_off);
            if (!buf.glyphs) {
                return;
            }

            memcpy(buf.glyphs, line_glyph_buffer.data(),
                   SkToSizeT(line_glyph_buffer.count()) * sizeof(SkGlyphID));

            y_off += line_spacing;
        }
    };

    const auto& is_line_break = [](SkUnichar uch) {
        // TODO: other explicit breaks?
        return uch == '\r';
    };

    const char* ptr        = fText.fText.c_str();
    const char* line_start = ptr;
    const char* end        = ptr + fText.fText.size();

    while (ptr < end) {
        if (is_line_break(SkUTF::NextUTF8(&ptr, end))) {
            push_line(line_start, ptr - 1);
            line_start = ptr;
        }
    }
    push_line(line_start, ptr);

    return builder.make();
}

void TextAdapter::apply() {
    fTextNode->setBlob(this->makeBlob());
    fFillColor->setColor(fText.fFillColor);
    fStrokeColor->setColor(fText.fStrokeColor);
    fStrokeColor->setStrokeWidth(fText.fStrokeWidth);

    // Turn the state transition into a tri-state value:
    //   -1: detach node
    //    0: no change
    //    1: attach node
    const auto   fill_change = SkToInt(fText.fHasFill) - SkToInt(fHadFill);
    const auto stroke_change = SkToInt(fText.fHasStroke) - SkToInt(fHadStroke);

    // Sync SG topology.
    if (fill_change || stroke_change) {
        // This is trickier than it should be because sksg::Group only allows adding children
        // in paint-order.
        if (stroke_change < 0 || (fHadStroke && fill_change > 0)) {
            fRoot->removeChild(fStrokeNode);
        }

        if (fill_change < 0) {
            fRoot->removeChild(fFillNode);
        } else if (fill_change > 0) {
            fRoot->addChild(fFillNode);
        }

        if (stroke_change > 0 || (fHadStroke && fill_change > 0)) {
            fRoot->addChild(fStrokeNode);
        }
    }

    // Track current state.
    fHadFill   = fText.fHasFill;
    fHadStroke = fText.fHasStroke;
}

} // namespace skottie
