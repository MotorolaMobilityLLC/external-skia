/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDevice_DEFINED
#define SkDevice_DEFINED

#include "SkRefCnt.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkRegion.h"
#include "SkSurfaceProps.h"

class SkBitmap;
struct SkDrawShadowRec;
class SkGlyphRunList;
class SkGlyphRunBuilder;
class SkImageFilterCache;
struct SkIRect;
class SkMatrix;
class SkRasterHandleAllocator;
class SkSpecialImage;

class SkBaseDevice : public SkRefCnt {
public:
    SkBaseDevice(const SkImageInfo&, const SkSurfaceProps&);

    /**
     *  Return ImageInfo for this device. If the canvas is not backed by pixels
     *  (cpu or gpu), then the info's ColorType will be kUnknown_SkColorType.
     */
    const SkImageInfo& imageInfo() const { return fInfo; }

    /**
     *  Return SurfaceProps for this device.
     */
    const SkSurfaceProps& surfaceProps() const {
        return fSurfaceProps;
    }

    /**
     *  Return the bounds of the device in the coordinate space of the root
     *  canvas. The root device will have its top-left at 0,0, but other devices
     *  such as those associated with saveLayer may have a non-zero origin.
     */
    void getGlobalBounds(SkIRect* bounds) const {
        SkASSERT(bounds);
        const SkIPoint& origin = this->getOrigin();
        bounds->setXYWH(origin.x(), origin.y(), this->width(), this->height());
    }

    SkIRect getGlobalBounds() const {
        SkIRect bounds;
        this->getGlobalBounds(&bounds);
        return bounds;
    }

    int width() const {
        return this->imageInfo().width();
    }

    int height() const {
        return this->imageInfo().height();
    }

    bool isOpaque() const {
        return this->imageInfo().isOpaque();
    }

    bool writePixels(const SkPixmap&, int x, int y);

    /**
     *  Try to get write-access to the pixels behind the device. If successful, this returns true
     *  and fills-out the pixmap parameter. On success it also bumps the genID of the underlying
     *  bitmap.
     *
     *  On failure, returns false and ignores the pixmap parameter.
     */
    bool accessPixels(SkPixmap* pmap);

    /**
     *  Try to get read-only-access to the pixels behind the device. If successful, this returns
     *  true and fills-out the pixmap parameter.
     *
     *  On failure, returns false and ignores the pixmap parameter.
     */
    bool peekPixels(SkPixmap*);

    /**
     *  Return the device's origin: its offset in device coordinates from
     *  the default origin in its canvas' matrix/clip
     */
    const SkIPoint& getOrigin() const { return fOrigin; }

    virtual void* getRasterHandle() const { return nullptr; }

    void save() { this->onSave(); }
    void restore(const SkMatrix& ctm) {
        this->onRestore();
        this->setGlobalCTM(ctm);
    }
    void clipRect(const SkRect& rect, SkClipOp op, bool aa) {
        this->onClipRect(rect, op, aa);
    }
    void clipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
        this->onClipRRect(rrect, op, aa);
    }
    void clipPath(const SkPath& path, SkClipOp op, bool aa) {
        this->onClipPath(path, op, aa);
    }
    void clipRegion(const SkRegion& region, SkClipOp op) {
        this->onClipRegion(region, op);
    }
    void androidFramework_setDeviceClipRestriction(SkIRect* mutableClipRestriction) {
        this->onSetDeviceClipRestriction(mutableClipRestriction);
    }
    bool clipIsWideOpen() const;

    const SkMatrix& ctm() const { return fCTM; }
    void setCTM(const SkMatrix& ctm) {
        fCTM = ctm;
    }
    void setGlobalCTM(const SkMatrix& ctm);
    virtual void validateDevBounds(const SkIRect&) {}

protected:
    enum TileUsage {
        kPossible_TileUsage,    //!< the created device may be drawn tiled
        kNever_TileUsage,       //!< the created device will never be drawn tiled
    };

    struct TextFlags {
        uint32_t    fFlags;     // SkPaint::getFlags()
    };

    virtual void onSave() {}
    virtual void onRestore() {}
    virtual void onClipRect(const SkRect& rect, SkClipOp, bool aa) {}
    virtual void onClipRRect(const SkRRect& rrect, SkClipOp, bool aa) {}
    virtual void onClipPath(const SkPath& path, SkClipOp, bool aa) {}
    virtual void onClipRegion(const SkRegion& deviceRgn, SkClipOp) {}
    virtual void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) {}
    virtual bool onClipIsAA() const = 0;
    virtual void onAsRgnClip(SkRegion*) const = 0;
    enum ClipType {
        kEmpty_ClipType,
        kRect_ClipType,
        kComplex_ClipType
    };
    virtual ClipType onGetClipType() const = 0;

    /** These are called inside the per-device-layer loop for each draw call.
     When these are called, we have already applied any saveLayer operations,
     and are handling any looping from the paint.
     */
    virtual void drawPaint(const SkPaint& paint) = 0;
    virtual void drawPoints(SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) = 0;
    virtual void drawRect(const SkRect& r,
                          const SkPaint& paint) = 0;
    virtual void drawRegion(const SkRegion& r,
                            const SkPaint& paint);
    virtual void drawOval(const SkRect& oval,
                          const SkPaint& paint) = 0;
    /** By the time this is called we know that abs(sweepAngle) is in the range [0, 360). */
    virtual void drawArc(const SkRect& oval, SkScalar startAngle,
                         SkScalar sweepAngle, bool useCenter, const SkPaint& paint);
    virtual void drawRRect(const SkRRect& rr,
                           const SkPaint& paint) = 0;

    // Default impl calls drawPath()
    virtual void drawDRRect(const SkRRect& outer,
                            const SkRRect& inner, const SkPaint&);

    /**
     *  If pathIsMutable, then the implementation is allowed to cast path to a
     *  non-const pointer and modify it in place (as an optimization). Canvas
     *  may do this to implement helpers such as drawOval, by placing a temp
     *  path on the stack to hold the representation of the oval.
     *
     *  If prePathMatrix is not null, it should logically be applied before any
     *  stroking or other effects. If there are no effects on the paint that
     *  affect the geometry/rasterization, then the pre matrix can just be
     *  pre-concated with the current matrix.
     */
    virtual void drawPath(const SkPath& path,
                          const SkPaint& paint,
                          const SkMatrix* prePathMatrix = nullptr,
                          bool pathIsMutable = false) = 0;
    virtual void drawBitmap(const SkBitmap& bitmap,
                            SkScalar x,
                            SkScalar y,
                            const SkPaint& paint) = 0;
    virtual void drawSprite(const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) = 0;

    /**
     *  The default impl. will create a bitmap-shader from the bitmap,
     *  and call drawRect with it.
     */
    virtual void drawBitmapRect(const SkBitmap&,
                                const SkRect* srcOrNull, const SkRect& dst,
                                const SkPaint& paint,
                                SkCanvas::SrcRectConstraint) = 0;
    virtual void drawBitmapNine(const SkBitmap&, const SkIRect& center,
                                const SkRect& dst, const SkPaint&);
    virtual void drawBitmapLattice(const SkBitmap&, const SkCanvas::Lattice&,
                                   const SkRect& dst, const SkPaint&);

    virtual void drawImage(const SkImage*, SkScalar x, SkScalar y, const SkPaint&);
    virtual void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                               const SkPaint&, SkCanvas::SrcRectConstraint);
    virtual void drawImageNine(const SkImage*, const SkIRect& center,
                               const SkRect& dst, const SkPaint&);
    virtual void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                                  const SkRect& dst, const SkPaint&);

    /**
     *  Does not handle text decoration.
     *  Decorations (underline and stike-thru) will be handled by SkCanvas.
     */
    virtual void drawGlyphRunList(const SkPaint& paint, SkGlyphRunList* glyphRunList);
    virtual void drawVertices(const SkVertices*, const SkMatrix* bones, int boneCount, SkBlendMode,
                              const SkPaint&) = 0;
    virtual void drawShadow(const SkPath&, const SkDrawShadowRec&);

    // default implementation unrolls the blob runs.
    virtual void drawTextBlob(const SkTextBlob*, SkScalar x, SkScalar y, const SkPaint& paint);
    // default implementation calls drawVertices
    virtual void drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], SkBlendMode, const SkPaint& paint);

    // default implementation calls drawPath
    virtual void drawAtlas(const SkImage* atlas, const SkRSXform[], const SkRect[],
                           const SkColor[], int count, SkBlendMode, const SkPaint&);

    virtual void drawAnnotation(const SkRect&, const char[], SkData*) {}

    /** The SkDevice passed will be an SkDevice which was returned by a call to
        onCreateDevice on this device with kNeverTile_TileExpectation.
     */
    virtual void drawDevice(SkBaseDevice*, int x, int y,
                            const SkPaint&) = 0;

    virtual void drawTextOnPath(const void* text, size_t len, const SkPath&,
                                const SkMatrix*, const SkPaint&);
    virtual void drawTextRSXform(const void* text, size_t len, const SkRSXform[],
                                 const SkPaint&);

    virtual void drawSpecial(SkSpecialImage*, int x, int y, const SkPaint&,
                             SkImage* clipImage, const SkMatrix& clipMatrix);
    virtual sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&);
    virtual sk_sp<SkSpecialImage> makeSpecial(const SkImage*);
    virtual sk_sp<SkSpecialImage> snapSpecial();

    bool readPixels(const SkPixmap&, int x, int y);

    ///////////////////////////////////////////////////////////////////////////

    virtual GrContext* context() const { return nullptr; }

    virtual sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&);
    virtual bool onPeekPixels(SkPixmap*) { return false; }

    /**
     *  The caller is responsible for "pre-clipping" the dst. The impl can assume that the dst
     *  image at the specified x,y offset will fit within the device's bounds.
     *
     *  This is explicitly asserted in readPixels(), the public way to call this.
     */
    virtual bool onReadPixels(const SkPixmap&, int x, int y);

    /**
     *  The caller is responsible for "pre-clipping" the src. The impl can assume that the src
     *  image at the specified x,y offset will fit within the device's bounds.
     *
     *  This is explicitly asserted in writePixelsDirect(), the public way to call this.
     */
    virtual bool onWritePixels(const SkPixmap&, int x, int y);

    virtual bool onAccessPixels(SkPixmap*) { return false; }

    struct CreateInfo {
        static SkPixelGeometry AdjustGeometry(const SkImageInfo&, TileUsage, SkPixelGeometry,
                                              bool preserveLCDText);

        // The constructor may change the pixel geometry based on other parameters.
        CreateInfo(const SkImageInfo& info,
                   TileUsage tileUsage,
                   SkPixelGeometry geo)
            : fInfo(info)
            , fTileUsage(tileUsage)
            , fPixelGeometry(AdjustGeometry(info, tileUsage, geo, false))
        {}

        CreateInfo(const SkImageInfo& info,
                   TileUsage tileUsage,
                   SkPixelGeometry geo,
                   bool preserveLCDText,
                   bool trackCoverage,
                   SkRasterHandleAllocator* allocator)
            : fInfo(info)
            , fTileUsage(tileUsage)
            , fPixelGeometry(AdjustGeometry(info, tileUsage, geo, preserveLCDText))
            , fTrackCoverage(trackCoverage)
            , fAllocator(allocator)
        {}

        const SkImageInfo       fInfo;
        const TileUsage         fTileUsage;
        const SkPixelGeometry   fPixelGeometry;
        const bool              fTrackCoverage = false;
        SkRasterHandleAllocator* fAllocator = nullptr;
    };

    /**
     *  Create a new device based on CreateInfo. If the paint is not null, then it represents a
     *  preview of how the new device will be composed with its creator device (this).
     *
     *  The subclass may be handed this device in drawDevice(), so it must always return
     *  a device that it knows how to draw, and that it knows how to identify if it is not of the
     *  same subclass (since drawDevice is passed a SkBaseDevice*). If the subclass cannot fulfill
     *  that contract (e.g. PDF cannot support some settings on the paint) it should return NULL,
     *  and the caller may then decide to explicitly create a bitmapdevice, knowing that later
     *  it could not call drawDevice with it (but it could call drawSprite or drawBitmap).
     */
    virtual SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) {
        return nullptr;
    }

    // A helper function used by derived classes to log the scale factor of a bitmap or image draw.
    static void LogDrawScaleFactor(const SkMatrix&, SkFilterQuality);

private:
    friend class SkAndroidFrameworkUtils;
    friend class SkCanvas;
    friend struct DeviceCM; //for setMatrixClip
    friend class SkDraw;
    friend class SkDrawIter;
    friend class SkDeviceFilteredPaint;
    friend class SkSurface_Raster;
    friend class DeviceTestingAccess;

    // Temporarily friend the SkGlyphRunBuilder until drawPosText is gone.
    friend class SkGlyphRun;
    virtual void drawPosText(const void* text, size_t len,
                             const SkScalar pos[], int scalarsPerPos,
                             const SkPoint& offset, const SkPaint& paint) = 0;

    // used to change the backend's pixels (and possibly config/rowbytes)
    // but cannot change the width/height, so there should be no change to
    // any clip information.
    // TODO: move to SkBitmapDevice
    virtual void replaceBitmapBackendForRasterSurface(const SkBitmap&) {}

    virtual bool forceConservativeRasterClip() const { return false; }

    /**
     * Don't call this!
     */
    virtual GrRenderTargetContext* accessRenderTargetContext() { return nullptr; }

    // just called by SkCanvas when built as a layer
    void setOrigin(const SkMatrix& ctm, int x, int y);

    /** Causes any deferred drawing to the device to be completed.
     */
    virtual void flush() {}

    virtual SkImageFilterCache* getImageFilterCache() { return nullptr; }

    friend class SkNoPixelsDevice;
    friend class SkBitmapDevice;
    void privateResize(int w, int h) {
        *const_cast<SkImageInfo*>(&fInfo) = fInfo.makeWH(w, h);
    }

    SkIPoint             fOrigin;
    const SkImageInfo    fInfo;
    const SkSurfaceProps fSurfaceProps;
    SkMatrix             fCTM;

    typedef SkRefCnt INHERITED;
};

class SkNoPixelsDevice : public SkBaseDevice {
public:
    SkNoPixelsDevice(const SkIRect& bounds, const SkSurfaceProps& props)
            : SkBaseDevice(SkImageInfo::MakeUnknown(bounds.width(), bounds.height()), props)
    {
        // this fails if we enable this assert: DiscardableImageMapTest.GetDiscardableImagesInRectMaxImage
        //SkASSERT(bounds.width() >= 0 && bounds.height() >= 0);
    }

    void resetForNextPicture(const SkIRect& bounds) {
        //SkASSERT(bounds.width() >= 0 && bounds.height() >= 0);
        this->privateResize(bounds.width(), bounds.height());
    }

protected:
    // We don't track the clip at all (for performance), but we have to respond to some queries.
    // We pretend to be wide-open. We could pretend to always be empty, but that *seems* worse.
    void onSave() override {}
    void onRestore() override {}
    void onClipRect(const SkRect& rect, SkClipOp, bool aa) override {}
    void onClipRRect(const SkRRect& rrect, SkClipOp, bool aa) override {}
    void onClipPath(const SkPath& path, SkClipOp, bool aa) override {}
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override {}
    void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) override {}
    bool onClipIsAA() const override { return false; }
    void onAsRgnClip(SkRegion* rgn) const override {
        rgn->setRect(SkIRect::MakeWH(this->width(), this->height()));
    }
    ClipType onGetClipType() const override {
        return kRect_ClipType;
    }

    void drawPaint(const SkPaint& paint) override {}
    void drawPoints(SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&) override {}
    void drawRect(const SkRect&, const SkPaint&) override {}
    void drawOval(const SkRect&, const SkPaint&) override {}
    void drawRRect(const SkRRect&, const SkPaint&) override {}
    void drawPath(const SkPath&, const SkPaint&, const SkMatrix*, bool) override {}
    void drawBitmap(const SkBitmap&, SkScalar x, SkScalar y, const SkPaint&) override {}
    void drawSprite(const SkBitmap&, int, int, const SkPaint&) override {}
    void drawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint&,
                        SkCanvas::SrcRectConstraint) override {}
    void drawPosText(const void*, size_t, const SkScalar[], int, const SkPoint&,
                     const SkPaint&) override {}
    void drawDevice(SkBaseDevice*, int, int, const SkPaint&) override {}
    void drawVertices(const SkVertices*, const SkMatrix*, int, SkBlendMode,
                      const SkPaint&) override {}

private:
    typedef SkBaseDevice INHERITED;
};

class SkAutoDeviceCTMRestore : SkNoncopyable {
public:
    SkAutoDeviceCTMRestore(SkBaseDevice* device, const SkMatrix& ctm)
        : fDevice(device)
        , fPrevCTM(device->ctm())
    {
        fDevice->setCTM(ctm);
    }
    ~SkAutoDeviceCTMRestore() {
        fDevice->setCTM(fPrevCTM);
    }

private:
    SkBaseDevice*   fDevice;
    const SkMatrix  fPrevCTM;
};

#endif
