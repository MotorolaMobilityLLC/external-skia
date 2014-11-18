/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCanvasDrawable_DEFINED
#define SkCanvasDrawable_DEFINED

#include "SkRefCnt.h"

class SkBBHFactory;
class SkCanvas;
struct SkRect;

/**
 *  Base-class for objects that draw into SkCanvas.
 *
 *  The object has a generation ID, which is guaranteed to be unique across all drawables. To
 *  allow for clients of the drawable that may want to cache the results, the drawable must
 *  change its generation ID whenever its internal state changes such that it will draw differently.
 */
class SkCanvasDrawable : public SkRefCnt {
public:
    SkCanvasDrawable();

    /**
     *  Draws into the specified content. The drawing sequence will be balanced upon return
     *  (i.e. the saveLevel() on the canvas will match what it was when draw() was called,
     *  and the current matrix and clip settings will not be changed.
     */
    void draw(SkCanvas*);

    SkPicture* newPictureSnapshot(SkBBHFactory* bbhFactory, uint32_t recordFlags);
    SkPicture* newPictureSnapshot() {
        return this->newPictureSnapshot(NULL, 0);
    }

    /**
     *  Return a unique value for this instance. If two calls to this return the same value,
     *  it is presumed that calling the draw() method will render the same thing as well.
     *
     *  Subclasses that change their state should call notifyDrawingChanged() to ensure that
     *  a new value will be returned the next time it is called.
     */
    uint32_t getGenerationID();

    /**
     *  Return the (conservative) bounds of what the drawable will draw. If the drawable can
     *  change what it draws (e.g. animation or in response to some external change), then this
     *  must return a bounds that is always valid for all possible states.
     */
    SkRect getBounds();

    /**
     *  Calling this invalidates the previous generation ID, and causes a new one to be computed
     *  the next time getGenerationID() is called. Typically this is called by the object itself,
     *  in response to its internal state changing.
     */
    void notifyDrawingChanged();

protected:
    virtual SkRect onGetBounds() = 0;
    virtual void onDraw(SkCanvas*) = 0;
    virtual SkPicture* onNewPictureSnapshot(SkBBHFactory*, uint32_t recordFlags);

private:
    int32_t fGenerationID;
};

#endif
