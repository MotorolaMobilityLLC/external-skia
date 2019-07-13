/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GMSlide_DEFINED
#define GMSlide_DEFINED

#include "gm/gm.h"
#include "tools/viewer/Slide.h"

class GMSlide : public Slide {
public:
    GMSlide(skiagm::GM* gm);
    ~GMSlide() override;

    SkISize getDimensions() const override { return fGM->getISize(); }

    void draw(SkCanvas* canvas) override;
    bool animate(double nanos) override;

    bool onChar(SkUnichar c) override;

    bool onGetControls(SkMetaData*) override;
    void onSetControls(const SkMetaData&) override;

private:
    std::unique_ptr<skiagm::GM> fGM;
};


#endif
