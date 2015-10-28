
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmapHeap.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkGPipe.h"
#include "SkGPipePriv.h"
#include "SkReader32.h"
#include "SkStream.h"

#include "SkAnnotation.h"
#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPatchUtils.h"
#include "SkPathEffect.h"
#include "SkRasterizer.h"
#include "SkReadBuffer.h"
#include "SkRRect.h"
#include "SkRSXform.h"
#include "SkShader.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

static SkFlattenable::Type paintflat_to_flattype(PaintFlats pf) {
    static const uint8_t gEffectTypesInPaintFlatsOrder[] = {
        SkFlattenable::kSkColorFilter_Type,
        SkFlattenable::kSkDrawLooper_Type,
        SkFlattenable::kSkImageFilter_Type,
        SkFlattenable::kSkMaskFilter_Type,
        SkFlattenable::kSkPathEffect_Type,
        SkFlattenable::kSkRasterizer_Type,
        SkFlattenable::kSkShader_Type,
        SkFlattenable::kSkXfermode_Type,
    };

    SkASSERT((size_t)pf < SK_ARRAY_COUNT(gEffectTypesInPaintFlatsOrder));
    return (SkFlattenable::Type)gEffectTypesInPaintFlatsOrder[pf];
}

static void set_paintflat(SkPaint* paint, SkFlattenable* obj, unsigned paintFlat) {
    SkASSERT(paintFlat < kCount_PaintFlats);
    switch (paintFlat) {
        case kColorFilter_PaintFlat:
            paint->setColorFilter((SkColorFilter*)obj);
            break;
        case kDrawLooper_PaintFlat:
            paint->setLooper((SkDrawLooper*)obj);
            break;
        case kMaskFilter_PaintFlat:
            paint->setMaskFilter((SkMaskFilter*)obj);
            break;
        case kPathEffect_PaintFlat:
            paint->setPathEffect((SkPathEffect*)obj);
            break;
        case kRasterizer_PaintFlat:
            paint->setRasterizer((SkRasterizer*)obj);
            break;
        case kShader_PaintFlat:
            paint->setShader((SkShader*)obj);
            break;
        case kImageFilter_PaintFlat:
            paint->setImageFilter((SkImageFilter*)obj);
            break;
        case kXfermode_PaintFlat:
            paint->setXfermode((SkXfermode*)obj);
            break;
        default:
            SkDEBUGFAIL("never gets here");
    }
}

template <typename T> class SkRefCntTDArray : public SkTDArray<T> {
public:
    ~SkRefCntTDArray() { this->unrefAll(); }
};

class SkGPipeState : public SkBitmapHeapReader {
public:
    SkGPipeState();
    ~SkGPipeState();

    void setSilent(bool silent) {
        fSilent = silent;
    }

    bool shouldDraw() {
        return !fSilent;
    }

    void setFlags(unsigned flags) {
        if (fFlags != flags) {
            fFlags = flags;
            this->updateReader();
        }
    }

    unsigned getFlags() const {
        return fFlags;
    }

    void setReader(SkReadBuffer* reader) {
        fReader = reader;
        this->updateReader();
    }

    const SkPaint& paint() const { return fPaint; }
    SkPaint* editPaint() { return &fPaint; }

    SkFlattenable* getFlat(unsigned index) const {
        if (0 == index) {
            return nullptr;
        }
        return fFlatArray[index - 1];
    }

    void defFlattenable(PaintFlats pf, int index) {
        index--;
        SkFlattenable* obj = fReader->readFlattenable(paintflat_to_flattype(pf));
        if (fFlatArray.count() == index) {
            *fFlatArray.append() = obj;
        } else {
            SkSafeUnref(fFlatArray[index]);
            fFlatArray[index] = obj;
        }
    }

    void defFactory(const char* name) {
        SkFlattenable::Factory factory = SkFlattenable::NameToFactory(name);
        if (factory) {
            SkASSERT(fFactoryArray.find(factory) < 0);
            *fFactoryArray.append() = factory;
        }
    }

    /**
     * Add a bitmap to the array of bitmaps, or replace an existing one.
     * This is only used when in cross process mode without a shared heap.
     */
    void addBitmap(int index) {
        SkASSERT(shouldFlattenBitmaps(fFlags));
        SkBitmap* bm;
        if(fBitmaps.count() == index) {
            bm = new SkBitmap;
            *fBitmaps.append() = bm;
        } else {
            bm = fBitmaps[index];
        }
        fReader->readBitmap(bm);
    }

    /**
     * Override of SkBitmapHeapReader, so that SkReadBuffer can use
     * these SkBitmaps for bitmap shaders. Used only in cross process mode
     * without a shared heap.
     */
    SkBitmap* getBitmap(int32_t index) const override {
        SkASSERT(shouldFlattenBitmaps(fFlags));
        return fBitmaps[index];
    }

    /**
     * Needed to be a non-abstract subclass of SkBitmapHeapReader.
     */
    void releaseRef(int32_t) override {}

    void setSharedHeap(SkBitmapHeap* heap) {
        SkASSERT(!shouldFlattenBitmaps(fFlags) || nullptr == heap);
        SkRefCnt_SafeAssign(fSharedHeap, heap);
        this->updateReader();
    }

    void setImageHeap(SkImageHeap* heap) {
        fImageHeap.reset(SkRef(heap));
    }

    /**
     * Access the shared heap. Only used in the case when bitmaps are not
     * flattened.
     */
    SkBitmapHeap* getSharedHeap() const {
        SkASSERT(!shouldFlattenBitmaps(fFlags));
        return fSharedHeap;
    }

    void addTypeface() {
        size_t size = fReader->read32();
        const void* data = fReader->skip(SkAlign4(size));
        SkMemoryStream stream(data, size, false);
        *fTypefaces.append() = SkTypeface::Deserialize(&stream);
    }

    SkTypeface* getTypeface(unsigned id) const {
        return id ? fTypefaces[id - 1] : nullptr;
    }

    const SkImage* getImage(int32_t slot) const {
        return fImageHeap->get(slot);
    }

private:
    void updateReader() {
        if (nullptr == fReader) {
            return;
        }
        bool crossProcess = SkToBool(fFlags & SkGPipeWriter::kCrossProcess_Flag);
        fReader->setFlags(SkSetClearMask(fReader->getFlags(), crossProcess,
                                         SkReadBuffer::kCrossProcess_Flag));
        if (crossProcess) {
            fReader->setFactoryArray(&fFactoryArray);
        } else {
            fReader->setFactoryArray(nullptr);
        }

        if (shouldFlattenBitmaps(fFlags)) {
            fReader->setBitmapStorage(this);
        } else {
            fReader->setBitmapStorage(fSharedHeap);
        }
    }
    SkReadBuffer*             fReader;
    SkPaint                   fPaint;
    SkTDArray<SkFlattenable*> fFlatArray;
    SkTDArray<SkTypeface*>    fTypefaces;
    SkTDArray<SkFlattenable::Factory> fFactoryArray;
    SkTDArray<SkBitmap*>      fBitmaps;
    bool                      fSilent;
    // Only used when sharing bitmaps with the writer.
    SkBitmapHeap*             fSharedHeap;
    SkAutoTUnref<SkImageHeap> fImageHeap;
    unsigned                  fFlags;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T> const T* skip(SkReader32* reader, size_t count = 1) {
    size_t size = sizeof(T) * count;
    SkASSERT(SkAlign4(size) == size);
    return reinterpret_cast<const T*>(reader->skip(size));
}

template <typename T> const T* skipAlign(SkReader32* reader, size_t count = 1) {
    size_t size = SkAlign4(sizeof(T) * count);
    return reinterpret_cast<const T*>(reader->skip(size));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void clipPath_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    SkPath path;
    reader->readPath(&path);
    bool doAA = SkToBool(DrawOp_unpackFlags(op32) & kClip_HasAntiAlias_DrawOpFlag);
    canvas->clipPath(path, (SkRegion::Op)DrawOp_unpackData(op32), doAA);
}

static void clipRegion_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    SkRegion rgn;
    reader->readRegion(&rgn);
    canvas->clipRegion(rgn, (SkRegion::Op)DrawOp_unpackData(op32));
}

static void clipRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    const SkRect* rect = skip<SkRect>(reader);
    bool doAA = SkToBool(DrawOp_unpackFlags(op32) & kClip_HasAntiAlias_DrawOpFlag);
    canvas->clipRect(*rect, (SkRegion::Op)DrawOp_unpackData(op32), doAA);
}

static void clipRRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    SkRRect rrect;
    reader->readRRect(&rrect);
    bool doAA = SkToBool(DrawOp_unpackFlags(op32) & kClip_HasAntiAlias_DrawOpFlag);
    canvas->clipRRect(rrect, (SkRegion::Op)DrawOp_unpackData(op32), doAA);
}

///////////////////////////////////////////////////////////////////////////////

static void setMatrix_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    SkMatrix matrix;
    reader->readMatrix(&matrix);
    canvas->setMatrix(matrix);
}

static void concat_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    SkMatrix matrix;
    reader->readMatrix(&matrix);
    canvas->concat(matrix);
}

static void scale_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    const SkScalar* param = skip<SkScalar>(reader, 2);
    canvas->scale(param[0], param[1]);
}

static void skew_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    const SkScalar* param = skip<SkScalar>(reader, 2);
    canvas->skew(param[0], param[1]);
}

static void rotate_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    canvas->rotate(reader->readScalar());
}

static void translate_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    const SkScalar* param = skip<SkScalar>(reader, 2);
    canvas->translate(param[0], param[1]);
}

///////////////////////////////////////////////////////////////////////////////

static void save_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                    SkGPipeState* state) {
    canvas->save();
}

static void saveLayer_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    unsigned flags = DrawOp_unpackFlags(op32);
    SkCanvas::SaveFlags saveFlags = (SkCanvas::SaveFlags)DrawOp_unpackData(op32);

    const SkRect* bounds = nullptr;
    if (flags & kSaveLayer_HasBounds_DrawOpFlag) {
        bounds = skip<SkRect>(reader);
    }
    const SkPaint* paint = nullptr;
    if (flags & kSaveLayer_HasPaint_DrawOpFlag) {
        paint = &state->paint();
    }
    canvas->saveLayer(bounds, paint, saveFlags);
}

static void restore_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                       SkGPipeState* state) {
    canvas->restore();
}

///////////////////////////////////////////////////////////////////////////////

static void drawPaint_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    if (state->shouldDraw()) {
        canvas->drawPaint(state->paint());
    }
}

static void drawPoints_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    SkCanvas::PointMode mode = (SkCanvas::PointMode)DrawOp_unpackFlags(op32);
    size_t count = reader->readU32();
    const SkPoint* pts = skip<SkPoint>(reader, count);
    if (state->shouldDraw()) {
        canvas->drawPoints(mode, count, pts, state->paint());
    }
}

static void drawOval_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    const SkRect* rect = skip<SkRect>(reader);
    if (state->shouldDraw()) {
        canvas->drawOval(*rect, state->paint());
    }
}

static void drawRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    const SkRect* rect = skip<SkRect>(reader);
    if (state->shouldDraw()) {
        canvas->drawRect(*rect, state->paint());
    }
}

static void drawRRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    SkRRect rrect;
    reader->readRRect(&rrect);
    if (state->shouldDraw()) {
        canvas->drawRRect(rrect, state->paint());
    }
}

static void drawDRRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    SkRRect outer, inner;
    reader->readRRect(&outer);
    reader->readRRect(&inner);
    if (state->shouldDraw()) {
        canvas->drawDRRect(outer, inner, state->paint());
    }
}

static void drawPatch_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    
    unsigned flags = DrawOp_unpackFlags(op32);
    
    const SkPoint* cubics = skip<SkPoint>(reader, SkPatchUtils::kNumCtrlPts);
    
    const SkColor* colors = nullptr;
    if (flags & kDrawVertices_HasColors_DrawOpFlag) {
        colors = skip<SkColor>(reader, SkPatchUtils::kNumCorners);
    }
    const SkPoint* texCoords = nullptr;
    if (flags & kDrawVertices_HasTexs_DrawOpFlag) {
        texCoords = skip<SkPoint>(reader, SkPatchUtils::kNumCorners);
    }
    SkAutoTUnref<SkXfermode> xfer;
    if (flags & kDrawVertices_HasXfermode_DrawOpFlag) {
        int mode = reader->readInt();
        if (mode < 0 || mode > SkXfermode::kLastMode) {
            mode = SkXfermode::kModulate_Mode;
        }
        xfer.reset(SkXfermode::Create((SkXfermode::Mode)mode));
    }
    if (state->shouldDraw()) {
        canvas->drawPatch(cubics, colors, texCoords, xfer, state->paint());
    }
}

static void drawPath_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    SkPath path;
    reader->readPath(&path);
    if (state->shouldDraw()) {
        canvas->drawPath(path, state->paint());
    }
}

static void drawVertices_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                            SkGPipeState* state) {
    unsigned flags = DrawOp_unpackFlags(op32);

    SkCanvas::VertexMode vmode = (SkCanvas::VertexMode)reader->readU32();
    int vertexCount = reader->readU32();
    const SkPoint* verts = skip<SkPoint>(reader, vertexCount);

    const SkPoint* texs = nullptr;
    if (flags & kDrawVertices_HasTexs_DrawOpFlag) {
        texs = skip<SkPoint>(reader, vertexCount);
    }

    const SkColor* colors = nullptr;
    if (flags & kDrawVertices_HasColors_DrawOpFlag) {
        colors = skip<SkColor>(reader, vertexCount);
    }

    SkAutoTUnref<SkXfermode> xfer;
    if (flags & kDrawVertices_HasXfermode_DrawOpFlag) {
        SkXfermode::Mode mode = (SkXfermode::Mode)reader->readU32();
        xfer.reset(SkXfermode::Create(mode));
    }

    int indexCount = 0;
    const uint16_t* indices = nullptr;
    if (flags & kDrawVertices_HasIndices_DrawOpFlag) {
        indexCount = reader->readU32();
        indices = skipAlign<uint16_t>(reader, indexCount);
    }
    if (state->shouldDraw()) {
        canvas->drawVertices(vmode, vertexCount, verts, texs, colors, xfer,
                             indices, indexCount, state->paint());
    }
}

static void drawAtlas_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32, SkGPipeState* state) {
    unsigned flags = DrawOp_unpackFlags(op32);

    const SkPaint* paint = nullptr;
    if (flags & kDrawAtlas_HasPaint_DrawOpFlag) {
        paint = &state->paint();
    }
    const int slot = reader->readU32();
    const SkImage* atlas = state->getImage(slot);
    const int count = reader->readU32();
    SkXfermode::Mode mode = (SkXfermode::Mode)reader->readU32();
    const SkRSXform* xform = skip<SkRSXform>(reader, count);
    const SkRect* tex = skip<SkRect>(reader, count);
    const SkColor* colors = nullptr;
    if (flags & kDrawAtlas_HasColors_DrawOpFlag) {
        colors = skip<SkColor>(reader, count);
    }
    const SkRect* cull = nullptr;
    if (flags & kDrawAtlas_HasCull_DrawOpFlag) {
        cull = skip<SkRect>(reader, 1);
    }

    if (state->shouldDraw()) {
        canvas->drawAtlas(atlas, xform, tex, colors, count, mode, cull, paint);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void drawText_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));
    const SkScalar* xy = skip<SkScalar>(reader, 2);
    if (state->shouldDraw()) {
        canvas->drawText(text, len, xy[0], xy[1], state->paint());
    }
}

static void drawPosText_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));
    size_t posCount = reader->readU32();    // compute by our writer
    const SkPoint* pos = skip<SkPoint>(reader, posCount);
    if (state->shouldDraw()) {
        canvas->drawPosText(text, len, pos, state->paint());
    }
}

static void drawPosTextH_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));
    size_t posCount = reader->readU32();    // compute by our writer
    const SkScalar* xpos = skip<SkScalar>(reader, posCount);
    SkScalar constY = reader->readScalar();
    if (state->shouldDraw()) {
        canvas->drawPosTextH(text, len, xpos, constY, state->paint());
    }
}

static void drawTextOnPath_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                              SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));

    SkPath path;
    reader->readPath(&path);

    SkMatrix matrixStorage;
    const SkMatrix* matrix = nullptr;
    if (DrawOp_unpackFlags(op32) & kDrawTextOnPath_HasMatrix_DrawOpFlag) {
        reader->readMatrix(&matrixStorage);
        matrix = &matrixStorage;
    }
    if (state->shouldDraw()) {
        canvas->drawTextOnPath(text, len, path, matrix, state->paint());
    }
}

///////////////////////////////////////////////////////////////////////////////

class BitmapHolder : SkNoncopyable {
public:
    BitmapHolder(SkReader32* reader, uint32_t op32, SkGPipeState* state);
    ~BitmapHolder() {
        if (fHeapEntry != nullptr) {
            fHeapEntry->releaseRef();
        }
    }
    const SkBitmap* getBitmap() {
        return fBitmap;
    }
private:
    SkBitmapHeapEntry* fHeapEntry;
    const SkBitmap*    fBitmap;
    SkBitmap           fBitmapStorage;
};

BitmapHolder::BitmapHolder(SkReader32* reader, uint32_t op32,
                           SkGPipeState* state) {
    const unsigned flags = state->getFlags();
    const unsigned index = DrawOp_unpackData(op32);
    if (shouldFlattenBitmaps(flags)) {
        fHeapEntry = nullptr;
        fBitmap = state->getBitmap(index);
    } else {
        SkBitmapHeapEntry* entry = state->getSharedHeap()->getEntry(index);
        if (SkToBool(flags & SkGPipeWriter::kSimultaneousReaders_Flag)) {
            // Make a shallow copy for thread safety. Each thread will point to the same SkPixelRef,
            // which is thread safe.
            fBitmapStorage = *entry->getBitmap();
            fBitmap = &fBitmapStorage;
            // Release the ref on the bitmap now, since we made our own copy.
            entry->releaseRef();
            fHeapEntry = nullptr;
        } else {
            SkASSERT(!shouldFlattenBitmaps(flags));
            SkASSERT(!SkToBool(flags & SkGPipeWriter::kSimultaneousReaders_Flag));
            fHeapEntry = entry;
            fBitmap = fHeapEntry->getBitmap();
        }
    }
}

static void drawBitmap_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    BitmapHolder holder(reader, op32, state);
    bool hasPaint = SkToBool(DrawOp_unpackFlags(op32) & kDrawBitmap_HasPaint_DrawOpFlag);
    SkScalar left = reader->readScalar();
    SkScalar top = reader->readScalar();
    const SkBitmap* bitmap = holder.getBitmap();
    if (state->shouldDraw()) {
        canvas->drawBitmap(*bitmap, left, top, hasPaint ? &state->paint() : nullptr);
    }
}

static void drawBitmapNine_rp(SkCanvas* canvas, SkReader32* reader,
                              uint32_t op32, SkGPipeState* state) {
    BitmapHolder holder(reader, op32, state);
    bool hasPaint = SkToBool(DrawOp_unpackFlags(op32) & kDrawBitmap_HasPaint_DrawOpFlag);
    const SkIRect* center = skip<SkIRect>(reader);
    const SkRect* dst = skip<SkRect>(reader);
    const SkBitmap* bitmap = holder.getBitmap();
    if (state->shouldDraw()) {
        canvas->drawBitmapNine(*bitmap, *center, *dst,
                               hasPaint ? &state->paint() : nullptr);
    }
}

static void drawBitmapRect_rp(SkCanvas* canvas, SkReader32* reader,
                              uint32_t op32, SkGPipeState* state) {
    BitmapHolder holder(reader, op32, state);
    unsigned flags = DrawOp_unpackFlags(op32);
    bool hasPaint = SkToBool(flags & kDrawBitmap_HasPaint_DrawOpFlag);
    bool hasSrc = SkToBool(flags & kDrawBitmap_HasSrcRect_DrawOpFlag);
    const SkRect* src;
    if (hasSrc) {
        src = skip<SkRect>(reader);
    } else {
        src = nullptr;
    }
    SkCanvas::SrcRectConstraint constraint = SkCanvas::kStrict_SrcRectConstraint;
    if (flags & kDrawBitmap_Bleed_DrawOpFlag) {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    const SkRect* dst = skip<SkRect>(reader);
    const SkBitmap* bitmap = holder.getBitmap();
    if (state->shouldDraw()) {
        canvas->legacy_drawBitmapRect(*bitmap, src, *dst, hasPaint ? &state->paint() : nullptr, constraint);
    }
}

static void drawSprite_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    BitmapHolder holder(reader, op32, state);
    bool hasPaint = SkToBool(DrawOp_unpackFlags(op32) & kDrawBitmap_HasPaint_DrawOpFlag);
    const SkIPoint* point = skip<SkIPoint>(reader);
    const SkBitmap* bitmap = holder.getBitmap();
    if (state->shouldDraw()) {
        canvas->drawSprite(*bitmap, point->fX, point->fY, hasPaint ? &state->paint() : nullptr);
    }
}

static void drawImage_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32, SkGPipeState* state) {
    unsigned slot = DrawOp_unpackData(op32);
    unsigned flags = DrawOp_unpackFlags(op32);
    bool hasPaint = SkToBool(flags & kDrawBitmap_HasPaint_DrawOpFlag);
    SkScalar x = reader->readScalar();
    SkScalar y = reader->readScalar();
    const SkImage* image = state->getImage(slot);
    if (state->shouldDraw()) {
        canvas->drawImage(image, x, y, hasPaint ? &state->paint() : nullptr);
    }
}

static void drawImageRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                             SkGPipeState* state) {
    unsigned slot = DrawOp_unpackData(op32);
    unsigned flags = DrawOp_unpackFlags(op32);
    bool hasPaint = SkToBool(flags & kDrawBitmap_HasPaint_DrawOpFlag);
    bool hasSrc = SkToBool(flags & kDrawBitmap_HasSrcRect_DrawOpFlag);
    const SkRect* src = nullptr;
    if (hasSrc) {
        src = skip<SkRect>(reader);
    }
    const SkRect* dst = skip<SkRect>(reader);
    SkCanvas::SrcRectConstraint constraint = (SkCanvas::SrcRectConstraint)reader->readInt();

    const SkImage* image = state->getImage(slot);
    if (state->shouldDraw()) {
        canvas->legacy_drawImageRect(image, src, *dst, hasPaint ? &state->paint() : nullptr, constraint);
    }
}

static void drawImageNine_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                             SkGPipeState* state) {
    unsigned slot = DrawOp_unpackData(op32);
    unsigned flags = DrawOp_unpackFlags(op32);
    bool hasPaint = SkToBool(flags & kDrawBitmap_HasPaint_DrawOpFlag);
    const SkIRect* center = skip<SkIRect>(reader);
    const SkRect* dst = skip<SkRect>(reader);
    const SkImage* image = state->getImage(slot);
    if (state->shouldDraw()) {
        canvas->drawImageNine(image, *center, *dst, hasPaint ? &state->paint() : nullptr);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void drawPicture_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                           SkGPipeState* state) {
    UNIMPLEMENTED
}

static void drawTextBlob_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                            SkGPipeState* state) {
    SkScalar x = reader->readScalar();
    SkScalar y = reader->readScalar();

    int typefaceCount = reader->readU32();
    SkAutoSTMalloc<16, SkTypeface*> typefaceArray(typefaceCount);
    if (state->getFlags() & SkGPipeWriter::kCrossProcess_Flag) {
        for (int i = 0; i < typefaceCount; ++i) {
            typefaceArray[i] = state->getTypeface(reader->readU32());
        }
    } else {
        reader->read(typefaceArray.get(), typefaceCount * sizeof(SkTypeface*));
    }

    size_t blobSize = reader->readU32();
    const void* data = reader->skip(SkAlign4(blobSize));

    if (state->shouldDraw()) {
        SkReadBuffer blobBuffer(data, blobSize);
        blobBuffer.setTypefaceArray(typefaceArray.get(), typefaceCount);
        SkAutoTUnref<const SkTextBlob> blob(SkTextBlob::CreateFromBuffer(blobBuffer));
        SkASSERT(blob.get());

        canvas->drawTextBlob(blob, x, y, state->paint());
    }
}
///////////////////////////////////////////////////////////////////////////////

static void paintOp_rp(SkCanvas*, SkReader32* reader, uint32_t op32,
                       SkGPipeState* state) {
    size_t offset = reader->offset();
    size_t stop = offset + PaintOp_unpackData(op32);
    SkPaint* p = state->editPaint();

    do {
        uint32_t p32 = reader->readU32();
        unsigned op = PaintOp_unpackOp(p32);
        unsigned data = PaintOp_unpackData(p32);

//        SkDebugf(" read %08X op=%d flags=%d data=%d\n", p32, op, done, data);

        switch (op) {
            case kReset_PaintOp: p->reset(); break;
            case kFlags_PaintOp: p->setFlags(data); break;
            case kColor_PaintOp: p->setColor(reader->readU32()); break;
            case kFilterLevel_PaintOp: p->setFilterQuality((SkFilterQuality)data); break;
            case kStyle_PaintOp: p->setStyle((SkPaint::Style)data); break;
            case kJoin_PaintOp: p->setStrokeJoin((SkPaint::Join)data); break;
            case kCap_PaintOp: p->setStrokeCap((SkPaint::Cap)data); break;
            case kWidth_PaintOp: p->setStrokeWidth(reader->readScalar()); break;
            case kMiter_PaintOp: p->setStrokeMiter(reader->readScalar()); break;
            case kEncoding_PaintOp:
                p->setTextEncoding((SkPaint::TextEncoding)data);
                break;
            case kHinting_PaintOp: p->setHinting((SkPaint::Hinting)data); break;
            case kAlign_PaintOp: p->setTextAlign((SkPaint::Align)data); break;
            case kTextSize_PaintOp: p->setTextSize(reader->readScalar()); break;
            case kTextScaleX_PaintOp: p->setTextScaleX(reader->readScalar()); break;
            case kTextSkewX_PaintOp: p->setTextSkewX(reader->readScalar()); break;

            case kFlatIndex_PaintOp: {
                PaintFlats pf = (PaintFlats)PaintOp_unpackFlags(p32);
                unsigned index = data;
                set_paintflat(p, state->getFlat(index), pf);
                break;
            }

            case kTypeface_PaintOp:
                SkASSERT(SkToBool(state->getFlags() &
                                  SkGPipeWriter::kCrossProcess_Flag));
                p->setTypeface(state->getTypeface(data));
                break;
            default: SkDEBUGFAIL("bad paintop"); return;
        }
        SkASSERT(reader->offset() <= stop);
    } while (reader->offset() < stop);
}

static void typeface_rp(SkCanvas*, SkReader32* reader, uint32_t,
                        SkGPipeState* state) {
    SkASSERT(!SkToBool(state->getFlags() & SkGPipeWriter::kCrossProcess_Flag));
    SkPaint* p = state->editPaint();
    p->setTypeface(static_cast<SkTypeface*>(reader->readPtr()));
}

static void annotation_rp(SkCanvas*, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    SkPaint* p = state->editPaint();

    const size_t size = DrawOp_unpackData(op32);
    if (size > 0) {
        SkReadBuffer buffer(reader->skip(size), size);
        p->setAnnotation(SkAnnotation::Create(buffer))->unref();
        SkASSERT(buffer.offset() == size);
    } else {
        p->setAnnotation(nullptr);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void def_Typeface_rp(SkCanvas*, SkReader32*, uint32_t, SkGPipeState* state) {
    state->addTypeface();
}

static void def_PaintFlat_rp(SkCanvas*, SkReader32*, uint32_t op32,
                             SkGPipeState* state) {
    PaintFlats pf = (PaintFlats)DrawOp_unpackFlags(op32);
    unsigned index = DrawOp_unpackData(op32);
    state->defFlattenable(pf, index);
}

static void def_Bitmap_rp(SkCanvas*, SkReader32*, uint32_t op32,
                          SkGPipeState* state) {
    unsigned index = DrawOp_unpackData(op32);
    state->addBitmap(index);
}

static void def_Factory_rp(SkCanvas*, SkReader32* reader, uint32_t,
                           SkGPipeState* state) {
    state->defFactory(reader->readString());
}

///////////////////////////////////////////////////////////////////////////////

static void skip_rp(SkCanvas*, SkReader32* reader, uint32_t op32, SkGPipeState*) {
    size_t bytes = DrawOp_unpackData(op32);
    (void)reader->skip(bytes);
}

static void reportFlags_rp(SkCanvas*, SkReader32*, uint32_t op32,
                           SkGPipeState* state) {
    unsigned flags = DrawOp_unpackFlags(op32);
    state->setFlags(flags);
}

static void shareBitmapHeap_rp(SkCanvas*, SkReader32* reader, uint32_t,
                               SkGPipeState* state) {
    state->setSharedHeap(static_cast<SkBitmapHeap*>(reader->readPtr()));
}

static void shareImageHeap_rp(SkCanvas*, SkReader32* reader, uint32_t, SkGPipeState* state) {
    state->setImageHeap(static_cast<SkImageHeap*>(reader->readPtr()));
}

static void done_rp(SkCanvas*, SkReader32*, uint32_t, SkGPipeState*) {}

typedef void (*ReadProc)(SkCanvas*, SkReader32*, uint32_t op32, SkGPipeState*);

static const ReadProc gReadTable[] = {
    skip_rp,
    clipPath_rp,
    clipRegion_rp,
    clipRect_rp,
    clipRRect_rp,
    concat_rp,
    drawAtlas_rp,
    drawBitmap_rp,
    drawBitmapNine_rp,
    drawBitmapRect_rp,
    drawDRRect_rp,
    drawImage_rp,
    drawImageRect_rp,
    drawImageNine_rp,
    drawOval_rp,
    drawPaint_rp,
    drawPatch_rp,
    drawPath_rp,
    drawPicture_rp,
    drawPoints_rp,
    drawPosText_rp,
    drawPosTextH_rp,
    drawRect_rp,
    drawRRect_rp,
    drawSprite_rp,
    drawText_rp,
    drawTextBlob_rp,
    drawTextOnPath_rp,
    drawVertices_rp,
    restore_rp,
    rotate_rp,
    save_rp,
    saveLayer_rp,
    scale_rp,
    setMatrix_rp,
    skew_rp,
    translate_rp,

    paintOp_rp,
    typeface_rp,
    annotation_rp,

    def_Typeface_rp,
    def_PaintFlat_rp,
    def_Bitmap_rp,
    def_Factory_rp,

    reportFlags_rp,
    shareBitmapHeap_rp,
    shareImageHeap_rp,
    done_rp
};

///////////////////////////////////////////////////////////////////////////////

SkGPipeState::SkGPipeState()
    : fReader(0)
    , fSilent(false)
    , fSharedHeap(nullptr)
    , fFlags(0) {

}

SkGPipeState::~SkGPipeState() {
    fTypefaces.safeUnrefAll();
    fFlatArray.safeUnrefAll();
    fBitmaps.deleteAll();
    SkSafeUnref(fSharedHeap);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGPipe.h"

SkGPipeReader::SkGPipeReader() {
    fCanvas = nullptr;
    fState = nullptr;
    fProc = nullptr;
}

SkGPipeReader::SkGPipeReader(SkCanvas* target) {
    fCanvas = nullptr;
    this->setCanvas(target);
    fState = nullptr;
    fProc = nullptr;
}

void SkGPipeReader::setCanvas(SkCanvas *target) {
    SkRefCnt_SafeAssign(fCanvas, target);
}

SkGPipeReader::~SkGPipeReader() {
    SkSafeUnref(fCanvas);
    delete fState;
}

SkGPipeReader::Status SkGPipeReader::playback(const void* data, size_t length,
                                              uint32_t playbackFlags, size_t* bytesRead) {
    if (nullptr == fCanvas) {
        return kError_Status;
    }

    if (nullptr == fState) {
        fState = new SkGPipeState;
    }

    fState->setSilent(playbackFlags & kSilent_PlaybackFlag);

    SkASSERT(SK_ARRAY_COUNT(gReadTable) == (kDone_DrawOp + 1));

    const ReadProc* table = gReadTable;
    SkReadBuffer reader(data, length);
    reader.setBitmapDecoder(fProc);
    SkCanvas* canvas = fCanvas;
    Status status = kEOF_Status;

    fState->setReader(&reader);
    while (!reader.eof()) {
        uint32_t op32 = reader.readUInt();
        unsigned op = DrawOp_unpackOp(op32);
        // SkDEBUGCODE(DrawOps drawOp = (DrawOps)op;)

        if (op >= SK_ARRAY_COUNT(gReadTable)) {
            SkDebugf("---- bad op during GPipeState::playback\n");
            status = kError_Status;
            break;
        }
        if (kDone_DrawOp == op) {
            status = kDone_Status;
            break;
        }
        table[op](canvas, reader.getReader32(), op32, fState);
        if ((playbackFlags & kReadAtom_PlaybackFlag) &&
            (table[op] != paintOp_rp &&
             table[op] != def_Typeface_rp &&
             table[op] != def_PaintFlat_rp &&
             table[op] != def_Bitmap_rp
             )) {
                status = kReadAtom_Status;
                break;
            }
    }

    if (bytesRead) {
        *bytesRead = reader.offset();
    }
    return status;
}
