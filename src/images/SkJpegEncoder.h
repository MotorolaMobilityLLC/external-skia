/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegEncoder_DEFINED
#define SkJpegEncoder_DEFINED

#include "SkEncoder.h"

class SkJpegEncoderMgr;
class SkWStream;

class SkJpegEncoder : public SkEncoder {
public:

    enum class AlphaOption {
        kIgnore,
        kBlendOnBlack,
    };

    struct Options {
        /**
         *  |fQuality| must be in [0, 100] where 0 corresponds to the lowest quality.
         */
        int fQuality = 100;

        /**
         *  Jpegs must be opaque.  This instructs the encoder on how to handle input
         *  images with alpha.
         *
         *  The default is to ignore the alpha channel and treat the image as opaque.
         *  Another option is to blend the pixels onto a black background before encoding.
         *  In the second case, the encoder supports linear or legacy blending.
         */
        AlphaOption fAlphaOption = AlphaOption::kIgnore;
        SkTransferFunctionBehavior fBlendBehavior = SkTransferFunctionBehavior::kRespect;
    };

    /**
     *  Encode the |src| pixels to the |dst| stream.
     *  |options| may be used to control the encoding behavior.
     *
     *  Returns true on success.  Returns false on an invalid or unsupported |src|.
     */
    static bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options);

    /**
     *  Create a jpeg encoder that will encode the |src| pixels to the |dst| stream.
     *  |options| may be used to control the encoding behavior.
     *
     *  |dst| is unowned but must remain valid for the lifetime of the object.
     *
     *  This returns nullptr on an invalid or unsupported |src|.
     */
    static std::unique_ptr<SkJpegEncoder> Make(SkWStream* dst, const SkPixmap& src,
                                               const Options& options);

    ~SkJpegEncoder() override;

protected:
    bool onEncodeRows(int numRows) override;

private:
    SkJpegEncoder(std::unique_ptr<SkJpegEncoderMgr>, const SkPixmap& src);

    std::unique_ptr<SkJpegEncoderMgr> fEncoderMgr;
    typedef SkEncoder INHERITED;
};

#endif
