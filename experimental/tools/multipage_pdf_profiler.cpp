/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkNullCanvas.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "ProcStats.h"
#include "flags/SkCommandLineFlags.h"

#ifdef SK_ENABLE_NEW_SKPDF_BACKEND
#include "skpdf.h"
#endif

DEFINE_string2(readPath,
               r,
               "",
               "(Required)  The path to a .skp Skia Picture file.");
DEFINE_string2(writePath, w, "", "If set, write PDF output to this file.");
DEFINE_bool(newPdf, false, "Use the new PDF backend.");
DEFINE_bool(nullCanvas, true, "Render to a SkNullCanvas as a control.");

__SK_FORCE_IMAGE_DECODER_LINKING;

namespace {
class NullWStream : public SkWStream {
public:
    NullWStream() : fBytesWritten(0) {
    }
    virtual bool write(const void*, size_t size) SK_OVERRIDE {
        fBytesWritten += size;
        return true;
    }
    virtual size_t bytesWritten() const SK_OVERRIDE {
        return fBytesWritten;
    }
    size_t fBytesWritten;
};

SkDocument* CreatePDFDocument(SkWStream* out) {
#ifdef SK_ENABLE_NEW_SKPDF_BACKEND
    if (FLAGS_newPdf) {
        return skpdf::CreatePDFDocument(out);
    }
#endif
    return SkDocument::CreatePDF(out);
}
}  // namespace

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    if (FLAGS_readPath.isEmpty()) {
        SkDebugf("Error: missing requires --readPath option\n");
        return 1;
    }
    const char* path = FLAGS_readPath[0];
    SkFILEStream inputStream(path);

    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", path);
        return 2;
    }
    SkAutoGraphics ag;
    SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromStream(&inputStream));
    if (NULL == picture.get()) {
        SkDebugf("Could not read an SkPicture from %s\n", path);
        return 3;
    }

    int width = picture->cullRect().width();
    int height = picture->cullRect().height();

    const int kLetterWidth = 612;   // 8.5 * 72
    const int kLetterHeight = 792;  // 11 * 72
    SkRect letterRect = SkRect::MakeWH(SkIntToScalar(kLetterWidth),
                                       SkIntToScalar(kLetterHeight));

    int xPages = ((width - 1) / kLetterWidth) + 1;
    int yPages = ((height - 1) / kLetterHeight) + 1;

    SkAutoTDelete<SkWStream> out(SkNEW(NullWStream));

    if (!FLAGS_writePath.isEmpty()) {
        SkAutoTDelete<SkFILEWStream> fileStream(
            SkNEW_ARGS(SkFILEWStream, (FLAGS_writePath[0])));
        if (!fileStream->isValid()) {
            SkDebugf("Can't open file \"%s\" for writing.", FLAGS_writePath[0]);
            return 1;
        }
        out.reset(fileStream.detach());
    }

    SkCanvas* nullCanvas = SkCreateNullCanvas();

    SkAutoTUnref<SkDocument> pdfDocument;
    if (!FLAGS_nullCanvas) {
        pdfDocument.reset(CreatePDFDocument(out.get()));
    }

    for (int y = 0; y < yPages; ++y) {
        for (int x = 0; x < xPages; ++x) {
            SkCanvas* canvas;
            if (FLAGS_nullCanvas) {
                canvas = nullCanvas;
            } else {
                int w = SkTMin(kLetterWidth, width - (x * kLetterWidth));
                int h = SkTMin(kLetterHeight, height - (y * kLetterHeight));
                canvas = pdfDocument->beginPage(w, h);
            }
            {
                SkAutoCanvasRestore autoCanvasRestore(canvas, true);
                canvas->clipRect(letterRect);
                canvas->translate(SkIntToScalar(-kLetterWidth * x),
                                  SkIntToScalar(-kLetterHeight * y));
                canvas->drawPicture(picture);
            }
            canvas->flush();
            if (!FLAGS_nullCanvas) {
                pdfDocument->endPage();
            }
        }
    }
    if (!FLAGS_nullCanvas) {
        pdfDocument->close();
        pdfDocument.reset(NULL);
    }
    printf(SK_SIZE_T_SPECIFIER "\t%4d\n",
           inputStream.getLength(),
           sk_tools::getMaxResidentSetSizeMB());
    return 0;
}
