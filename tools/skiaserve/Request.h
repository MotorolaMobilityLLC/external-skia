/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Request_DEFINED
#define Request_DEFINED

#include "GrContextFactory.h"

#include "SkDebugCanvas.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkSurface.h"

#include "UrlDataManager.h"

struct MHD_Connection;
struct MHD_PostProcessor;

struct UploadContext {
    SkDynamicMemoryWStream fStream;
    MHD_PostProcessor* fPostProcessor;
    MHD_Connection* connection;
};

struct Request {
    Request(SkString rootUrl); 

    SkData* drawToPng(int n);
    void drawToCanvas(int n);
    SkCanvas* getCanvas();
    SkData* writeCanvasToPng(SkCanvas* canvas);
    SkBitmap* getBitmapFromCanvas(SkCanvas* canvas);
    bool enableGPU(bool enable);
    bool hasPicture() const { return SkToBool(fPicture.get()); }
    int getLastOp() const { return fDebugCanvas->getSize() - 1; }

    // Returns the json list of ops as an SkData
    SkData* getJsonOps(int n);

    // Returns a json list of batches as an SkData
    SkData* getJsonBatchList(int n);

    // Returns json with the viewMatrix and clipRect
    SkData* getJsonInfo(int n);

    // TODO probably want to make this configurable
    static const int kImageWidth;
    static const int kImageHeight;

    UploadContext* fUploadContext;
    SkAutoTUnref<SkPicture> fPicture;
    SkAutoTUnref<SkDebugCanvas> fDebugCanvas;
    UrlDataManager fUrlDataManager;
    
private:
    SkSurface* createCPUSurface();
    SkSurface* createGPUSurface();
    GrAuditTrail* getAuditTrail(SkCanvas*);
    void cleanupAuditTrail(SkCanvas*);
    
    SkAutoTDelete<GrContextFactory> fContextFactory;
    SkAutoTUnref<SkSurface> fSurface;
    bool fGPUEnabled;
};

#endif

