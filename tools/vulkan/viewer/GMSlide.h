/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GMSlide_DEFINED
#define GMSlide_DEFINED

#include "Slide.h"
#include "gm.h"

class GMSlide : public Slide {
public:
    GMSlide(skiagm::GM* gm);
    ~GMSlide() override;

    void draw(SkCanvas* canvas) override;

private:
    skiagm::GM* fGM;
};


#endif
