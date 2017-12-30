/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottySlide_DEFINED
#define SkottySlide_DEFINED

#include "Slide.h"

namespace skotty { class Animation; }

class SkottySlide : public Slide {
public:
    SkottySlide(const SkString& name, const SkString& path);
    ~SkottySlide() override = default;

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas*) override;
    bool animate(const SkAnimTimer&) override;

    bool onChar(SkUnichar) override;

private:
    SkString                           fPath;
    std::unique_ptr<skotty::Animation> fAnimation;
    SkMSec                             fTimeBase  = 0;
    bool                               fShowAnimationInval = false;

    typedef Slide INHERITED;
};

#endif // SkottySlide_DEFINED
