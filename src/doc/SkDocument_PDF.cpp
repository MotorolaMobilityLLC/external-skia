/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDocument.h"
#include "SkPDFDocument.h"
#include "SkPDFDevice.h"

class SkDocument_PDF : public SkDocument {
public:
    SkDocument_PDF(SkWStream* stream,
                   void (*doneProc)(SkWStream*,bool),
                   SkScalar rasterDpi)
        : SkDocument(stream, doneProc)
        , fDoc(SkNEW(SkPDFDocument))
        , fDevice(NULL)
        , fCanvas(NULL)
        , fRasterDpi(rasterDpi) {}

    virtual ~SkDocument_PDF() {
        // subclasses must call close() in their destructors
        this->close();
    }

protected:
    virtual SkCanvas* onBeginPage(SkScalar width, SkScalar height,
                                  const SkRect& trimBox) SK_OVERRIDE {
        SkASSERT(NULL == fCanvas);
        SkASSERT(NULL == fDevice);

        SkISize mediaBoxSize;
        mediaBoxSize.set(SkScalarRoundToInt(width), SkScalarRoundToInt(height));

        fDevice = SkNEW_ARGS(SkPDFDevice, (mediaBoxSize, mediaBoxSize, SkMatrix::I()));
        if (fRasterDpi != 0) {
            fDevice->setRasterDpi(fRasterDpi);
        }
        fCanvas = SkNEW_ARGS(SkCanvas, (fDevice));
        fCanvas->clipRect(trimBox);
        fCanvas->translate(trimBox.x(), trimBox.y());
        return fCanvas;
    }

    void onEndPage() SK_OVERRIDE {
        SkASSERT(fCanvas);
        SkASSERT(fDevice);

        fCanvas->flush();
        fDoc->appendPage(fDevice);

        fCanvas->unref();
        fDevice->unref();

        fCanvas = NULL;
        fDevice = NULL;
    }

    bool onClose(SkWStream* stream) SK_OVERRIDE {
        SkASSERT(NULL == fCanvas);
        SkASSERT(NULL == fDevice);

        bool success = fDoc->emitPDF(stream);
        SkDELETE(fDoc);
        fDoc = NULL;
        return success;
    }

    void onAbort() SK_OVERRIDE {
        SkDELETE(fDoc);
        fDoc = NULL;
    }

private:
    SkPDFDocument*  fDoc;
    SkPDFDevice*    fDevice;
    SkCanvas*       fCanvas;
    SkScalar        fRasterDpi;
};

///////////////////////////////////////////////////////////////////////////////

SkDocument* SkDocument::CreatePDF(SkWStream* stream, SkScalar dpi) {
    return stream ? SkNEW_ARGS(SkDocument_PDF, (stream, NULL, dpi)) : NULL;
}

static void delete_wstream(SkWStream* stream, bool aborted) {
    SkDELETE(stream);
}

SkDocument* SkDocument::CreatePDF(const char path[], SkScalar dpi) {
    SkFILEWStream* stream = SkNEW_ARGS(SkFILEWStream, (path));
    if (!stream->isValid()) {
        SkDELETE(stream);
        return NULL;
    }
    return SkNEW_ARGS(SkDocument_PDF, (stream, delete_wstream, dpi));
}
