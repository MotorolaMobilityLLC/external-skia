/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Request.h"

#include "png.h"

const int Request::kImageWidth = 1920;
const int Request::kImageHeight = 1080;

static void write_png_callback(png_structp png_ptr, png_bytep data, png_size_t length) {
    SkWStream* out = (SkWStream*) png_get_io_ptr(png_ptr);
    out->write(data, length);
}

static void write_png(const png_bytep rgba, png_uint_32 width, png_uint_32 height, SkWStream& out) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    SkASSERT(png != nullptr);
    png_infop info_ptr = png_create_info_struct(png);
    SkASSERT(info_ptr != nullptr);
    if (setjmp(png_jmpbuf(png))) {
        SkFAIL("png encode error");
    }
    png_set_IHDR(png, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_compression_level(png, 1);
    png_bytepp rows = (png_bytepp) sk_malloc_throw(height * sizeof(png_byte*));
    png_bytep pixels = (png_bytep) sk_malloc_throw(width * height * 3);
    for (png_size_t y = 0; y < height; ++y) {
        const png_bytep src = rgba + y * width * 4;
        rows[y] = pixels + y * width * 3;
        // convert from RGBA to RGB
        for (png_size_t x = 0; x < width; ++x) {
            rows[y][x * 3] = src[x * 4];
            rows[y][x * 3 + 1] = src[x * 4 + 1];
            rows[y][x * 3 + 2] = src[x * 4 + 2];
        }
    }
    png_set_filter(png, 0, PNG_NO_FILTERS);
    png_set_rows(png, info_ptr, &rows[0]);
    png_set_write_fn(png, &out, write_png_callback, NULL);
    png_write_png(png, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png, NULL);
    sk_free(rows);
}

Request::Request(SkString rootUrl)
    : fUploadContext(nullptr)
    , fUrlDataManager(rootUrl)
    , fGPUEnabled(false) {
    // create surface
    GrContextOptions grContextOpts;
    fContextFactory.reset(new GrContextFactory(grContextOpts));
    fSurface.reset(this->createCPUSurface());
}

SkBitmap* Request::getBitmapFromCanvas(SkCanvas* canvas) {
    SkBitmap* bmp = new SkBitmap();
    SkImageInfo info = SkImageInfo::Make(kImageWidth, kImageHeight, kRGBA_8888_SkColorType,
                kOpaque_SkAlphaType);
    bmp->setInfo(info);
    if (!canvas->readPixels(bmp, 0, 0)) {
        fprintf(stderr, "Can't read pixels\n");
        return nullptr;
    }
    return bmp;
}

SkData* Request::writeCanvasToPng(SkCanvas* canvas) {
    // capture pixels
    SkAutoTDelete<SkBitmap> bmp(getBitmapFromCanvas(canvas));
    SkASSERT(bmp);

    // write to png
    SkDynamicMemoryWStream buffer;
    write_png((const png_bytep) bmp->getPixels(), bmp->width(), bmp->height(), buffer);
    return buffer.copyToData();
}

SkCanvas* Request::getCanvas() {
    GrContextFactory* factory = fContextFactory;
    SkGLContext* gl = factory->getContextInfo(GrContextFactory::kNative_GLContextType,
                                              GrContextFactory::kNone_GLContextOptions).fGLContext;
    gl->makeCurrent();
    SkASSERT(fDebugCanvas);
    SkCanvas* target = fSurface->getCanvas();
    return target;
}

void Request::drawToCanvas(int n, int m) {
    SkCanvas* target = this->getCanvas();
    fDebugCanvas->drawTo(target, n, m);
}

SkData* Request::drawToPng(int n, int m) {
    this->drawToCanvas(n, m);
    return writeCanvasToPng(this->getCanvas());
}

SkSurface* Request::createCPUSurface() {
    SkImageInfo info = SkImageInfo::Make(kImageWidth, kImageHeight, kN32_SkColorType,
                                         kPremul_SkAlphaType);
    return SkSurface::NewRaster(info);
}

SkSurface* Request::createGPUSurface() {
    GrContext* context = fContextFactory->get(GrContextFactory::kNative_GLContextType,
                                              GrContextFactory::kNone_GLContextOptions);
    int maxRTSize = context->caps()->maxRenderTargetSize();
    SkImageInfo info = SkImageInfo::Make(SkTMin(kImageWidth, maxRTSize),
                                         SkTMin(kImageHeight, maxRTSize),
                                         kN32_SkColorType, kPremul_SkAlphaType);
    uint32_t flags = 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    SkSurface* surface = SkSurface::NewRenderTarget(context, SkBudgeted::kNo, info, 0,
                                                    &props);
    return surface;
}

bool Request::enableGPU(bool enable) {    
    if (enable) {
        SkSurface* surface = this->createGPUSurface();
        if (surface) {
            fSurface.reset(surface);
            fGPUEnabled = true;
            return true;
        }
        return false;
    }
    fSurface.reset(this->createCPUSurface());
    fGPUEnabled = false;
    return true;
} 

bool Request::initPictureFromStream(SkStream* stream) {
    // parse picture from stream
    fPicture.reset(SkPicture::CreateFromStream(stream));
    if (!fPicture.get()) {
        fprintf(stderr, "Could not create picture from stream.\n");
        return false;
    }

    // pour picture into debug canvas
    fDebugCanvas.reset(new SkDebugCanvas(kImageWidth, Request::kImageHeight));
    fDebugCanvas->drawPicture(fPicture);

    // for some reason we need to 'flush' the debug canvas by drawing all of the ops
    fDebugCanvas->drawTo(this->getCanvas(), this->getLastOp());
    this->getCanvas()->flush();
    return true;
}

GrAuditTrail* Request::getAuditTrail(SkCanvas* canvas) {
    GrAuditTrail* at = nullptr;
#if SK_SUPPORT_GPU
    GrRenderTarget* rt = canvas->internal_private_accessTopLayerRenderTarget();
    if (rt) {
        GrContext* ctx = rt->getContext();
        if (ctx) {
            at = ctx->getAuditTrail();
        }
    }
#endif
    return at;
}

void Request::cleanupAuditTrail(SkCanvas* canvas) {
    GrAuditTrail* at = this->getAuditTrail(canvas);
    if (at) {
        GrAuditTrail::AutoEnable ae(at);
        at->fullReset();
    }
}

SkData* Request::getJsonOps(int n) {
    SkCanvas* canvas = this->getCanvas();
    Json::Value root = fDebugCanvas->toJSON(fUrlDataManager, n, canvas);
    root["mode"] = Json::Value(fGPUEnabled ? "gpu" : "cpu");
    root["drawGpuBatchBounds"] = Json::Value(fDebugCanvas->getDrawGpuBatchBounds());
    SkDynamicMemoryWStream stream;
    stream.writeText(Json::FastWriter().write(root).c_str());

    this->cleanupAuditTrail(canvas);

    return stream.copyToData();
}

SkData* Request::getJsonBatchList(int n) {
    SkCanvas* canvas = this->getCanvas();
    SkASSERT(fGPUEnabled);

    // TODO if this is inefficient we could add a method to GrAuditTrail which takes
    // a Json::Value and is only compiled in this file
    Json::Value parsedFromString;
#if SK_SUPPORT_GPU
    // we use the toJSON method on debug canvas, but then just ignore the results and pull
    // the information we care about from the audit trail
    fDebugCanvas->toJSON(fUrlDataManager, n, canvas); 

    GrAuditTrail* at = this->getAuditTrail(canvas);
    GrAuditTrail::AutoManageBatchList enable(at);
    Json::Reader reader;
    SkDEBUGCODE(bool parsingSuccessful = )reader.parse(at->toJson().c_str(),
                                                       parsedFromString);
    SkASSERT(parsingSuccessful);
#endif

    SkDynamicMemoryWStream stream;
    stream.writeText(Json::FastWriter().write(parsedFromString).c_str());

    return stream.copyToData();
}

SkData* Request::getJsonInfo(int n) {
    // drawTo
    SkAutoTUnref<SkSurface> surface(this->createCPUSurface());
    SkCanvas* canvas = surface->getCanvas();

    // TODO this is really slow and we should cache the matrix and clip
    fDebugCanvas->drawTo(canvas, n);

    // make some json
    SkMatrix vm = fDebugCanvas->getCurrentMatrix();
    SkIRect clip = fDebugCanvas->getCurrentClip();
    Json::Value info(Json::objectValue);
    info["ViewMatrix"] = SkDrawCommand::MakeJsonMatrix(vm);
    info["ClipRect"] = SkDrawCommand::MakeJsonIRect(clip);

    std::string json = Json::FastWriter().write(info);

    // We don't want the null terminator so strlen is correct
    return SkData::NewWithCopy(json.c_str(), strlen(json.c_str()));
}
