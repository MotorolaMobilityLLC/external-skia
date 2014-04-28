/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecordDraw.h"

#include "SkRecordTraits.h"

namespace {

// All clip commands, Restore, and SaveLayer may change the clip.
template <typename T> struct ChangesClip { static const bool value = SkRecords::IsClip<T>::value; };
template <> struct ChangesClip<SkRecords::Restore> { static const bool value = true; };
template <> struct ChangesClip<SkRecords::SaveLayer> { static const bool value = true; };


// This is an SkRecord visitor that will draw that SkRecord to an SkCanvas.
class Draw : SkNoncopyable {
public:
    explicit Draw(SkCanvas* canvas) : fCanvas(canvas), fIndex(0), fClipEmpty(false) {}

    unsigned index() const { return fIndex; }
    void next() { ++fIndex; }

    template <typename T> void operator()(const T& r) {
        if (!this->skip(r)) {
            this->draw(r);
            this->updateClip<T>();
        }
    }

private:
    // No base case, so we'll be compile-time checked that we implemented all possibilities below.
    template <typename T> void draw(const T&);

    // skip() returns true if we can skip this command, false if not.
    // Update fIndex directly to skip more than just this one command.

    // If we're drawing into an empty clip, we can skip it.  Otherwise, run the command.
    template <typename T>
    SK_WHEN(SkRecords::IsDraw<T>, bool) skip(const T&) { return fClipEmpty; }

    template <typename T>
    SK_WHEN(!SkRecords::IsDraw<T>, bool) skip(const T&) { return false; }

    // Special versions for commands added by optimizations.
    bool skip(const SkRecords::PairedPushCull& r) {
        if (fCanvas->quickReject(r.base->rect)) {
            fIndex += r.skip;
            return true;
        }
        return this->skip(*r.base);
    }

    bool skip(const SkRecords::BoundedDrawPosTextH& r) {
        return this->skip(*r.base) || fCanvas->quickRejectY(r.minY, r.maxY);
    }

    // If we might have changed the clip, update it, else do nothing.
    template <typename T>
    SK_WHEN(ChangesClip<T>, void) updateClip() { fClipEmpty = fCanvas->isClipEmpty(); }
    template <typename T>
    SK_WHEN(!ChangesClip<T>, void) updateClip() {}

    SkCanvas* fCanvas;
    unsigned fIndex;
    bool fClipEmpty;
};

// NoOps draw nothing.
template <> void Draw::draw(const SkRecords::NoOp&) {}

#define DRAW(T, call) template <> void Draw::draw(const SkRecords::T& r) { fCanvas->call; }
DRAW(Restore, restore());
DRAW(Save, save(r.flags));
DRAW(SaveLayer, saveLayer(r.bounds, r.paint, r.flags));
DRAW(PopCull, popCull());
DRAW(PushCull, pushCull(r.rect));
DRAW(Clear, clear(r.color));
DRAW(Concat, concat(r.matrix));
DRAW(SetMatrix, setMatrix(r.matrix));

DRAW(ClipPath, clipPath(r.path, r.op, r.doAA));
DRAW(ClipRRect, clipRRect(r.rrect, r.op, r.doAA));
DRAW(ClipRect, clipRect(r.rect, r.op, r.doAA));
DRAW(ClipRegion, clipRegion(r.region, r.op));

DRAW(DrawBitmap, drawBitmap(r.bitmap, r.left, r.top, r.paint));
DRAW(DrawBitmapMatrix, drawBitmapMatrix(r.bitmap, r.matrix, r.paint));
DRAW(DrawBitmapNine, drawBitmapNine(r.bitmap, r.center, r.dst, r.paint));
DRAW(DrawBitmapRectToRect, drawBitmapRectToRect(r.bitmap, r.src, r.dst, r.paint, r.flags));
DRAW(DrawDRRect, drawDRRect(r.outer, r.inner, r.paint));
DRAW(DrawOval, drawOval(r.oval, r.paint));
DRAW(DrawPaint, drawPaint(r.paint));
DRAW(DrawPath, drawPath(r.path, r.paint));
DRAW(DrawPoints, drawPoints(r.mode, r.count, r.pts, r.paint));
DRAW(DrawPosText, drawPosText(r.text, r.byteLength, r.pos, r.paint));
DRAW(DrawPosTextH, drawPosTextH(r.text, r.byteLength, r.xpos, r.y, r.paint));
DRAW(DrawRRect, drawRRect(r.rrect, r.paint));
DRAW(DrawRect, drawRect(r.rect, r.paint));
DRAW(DrawSprite, drawSprite(r.bitmap, r.left, r.top, r.paint));
DRAW(DrawText, drawText(r.text, r.byteLength, r.x, r.y, r.paint));
DRAW(DrawTextOnPath, drawTextOnPath(r.text, r.byteLength, r.path, r.matrix, r.paint));
DRAW(DrawVertices, drawVertices(r.vmode, r.vertexCount, r.vertices, r.texs, r.colors,
                                r.xmode.get(), r.indices, r.indexCount, r.paint));
#undef DRAW

template <> void Draw::draw(const SkRecords::PairedPushCull& r) { this->draw(*r.base); }
template <> void Draw::draw(const SkRecords::BoundedDrawPosTextH& r) { this->draw(*r.base); }

}  // namespace

void SkRecordDraw(const SkRecord& record, SkCanvas* canvas) {
    for (Draw draw(canvas); draw.index() < record.count(); draw.next()) {
        record.visit(draw.index(), draw);
    }
}
