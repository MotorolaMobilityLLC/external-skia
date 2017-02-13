/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "fiddle_main.h"

// Globals externed in fiddle_main.h
SkBitmap source;
sk_sp<SkImage> image;

// Globals used by the local impl of SkDebugf.
char formatbuffer[1024];
std::string textoutput("");

void SkDebugf(const char * fmt, ...) {
    int n;
    va_list args;
    va_start(args, fmt);
    n = vsnprintf(formatbuffer, sizeof(formatbuffer), fmt, args);
    va_end(args);
    if (n>=0 && n<=int(sizeof(formatbuffer))) {
        textoutput.append(formatbuffer);
        textoutput.append("\n");
    }
}

static void encode_to_base64(const void* data, size_t size, FILE* out) {
    const uint8_t* input = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* end = &input[size];
    static const char codes[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz0123456789+/";
    while (input != end) {
        uint8_t b = (*input & 0xFC) >> 2;
        fputc(codes[b], out);
        b = (*input & 0x03) << 4;
        ++input;
        if (input == end) {
            fputc(codes[b], out);
            fputs("==", out);
            return;
        }
        b |= (*input & 0xF0) >> 4;
        fputc(codes[b], out);
        b = (*input & 0x0F) << 2;
        ++input;
        if (input == end) {
            fputc(codes[b], out);
            fputc('=', out);
            return;
        }
        b |= (*input & 0xC0) >> 6;
        fputc(codes[b], out);
        b = *input & 0x3F;
        fputc(codes[b], out);
        ++input;
    }
}

static void dump_output(const sk_sp<SkData>& data,
                        const char* name, bool last = true) {
    if (data) {
        printf("\t\"%s\": \"", name);
        encode_to_base64(data->data(), data->size(), stdout);
        fputs(last ? "\"\n" : "\",\n", stdout);
    }
}

static void dump_text(const std::string& s,
                        const char* name, bool last = true) {
    printf("\t\"%s\": \"", name);
    encode_to_base64(s.c_str(), s.length(), stdout);
    fputs(last ? "\"\n" : "\",\n", stdout);
}

static SkData* encode_snapshot(const sk_sp<SkSurface>& surface) {
    sk_sp<SkImage> img(surface->makeImageSnapshot());
    return img ? img->encode() : nullptr;
}

#if defined(__linux) && !defined(__ANDROID__)
    #include <GL/osmesa.h>
    static sk_sp<GrContext> create_grcontext() {
        // We just leak the OSMesaContext... the process will die soon anyway.
        if (OSMesaContext osMesaContext = OSMesaCreateContextExt(OSMESA_BGRA, 0, 0, 0, nullptr)) {
            static uint32_t buffer[16 * 16];
            OSMesaMakeCurrent(osMesaContext, &buffer, GL_UNSIGNED_BYTE, 16, 16);
        }

        auto osmesa_get = [](void* ctx, const char name[]) {
            SkASSERT(nullptr == ctx);
            SkASSERT(OSMesaGetCurrentContext());
            return OSMesaGetProcAddress(name);
        };
        sk_sp<const GrGLInterface> mesa(GrGLAssembleInterface(nullptr, osmesa_get));
        if (!mesa) {
            return nullptr;
        }
        return sk_sp<GrContext>(GrContext::Create(
                                        kOpenGL_GrBackend,
                                        reinterpret_cast<intptr_t>(mesa.get())));
    }
#else
    static sk_sp<GrContext> create_grcontext() { return nullptr; }
#endif



int main() {
    DrawOptions options = GetDrawOptions();
    // If textOnly then only do one type of image, otherwise the text
    // output is duplicated for each type.
    if (options.textOnly) {
        options.raster = true;
        options.gpu = false;
        options.pdf = false;
        options.skp = false;
    }
    if (options.source) {
        sk_sp<SkData> data(SkData::MakeFromFileName(options.source));
        if (!data) {
            perror(options.source);
            return 1;
        } else {
            image = SkImage::MakeFromEncoded(std::move(data));
            if (!image) {
                perror("Unable to decode the source image.");
                return 1;
            }
            SkAssertResult(image->asLegacyBitmap(
                                   &source, SkImage::kRO_LegacyBitmapMode));
        }
    }
    sk_sp<SkData> rasterData, gpuData, pdfData, skpData;
    SkColorType colorType = kN32_SkColorType;
    sk_sp<SkColorSpace> colorSpace = nullptr;
    if (options.f16) {
        SkASSERT(options.srgb);
        colorType = kRGBA_F16_SkColorType;
        colorSpace = SkColorSpace::MakeSRGBLinear();
    } else if (options.srgb) {
        colorSpace = SkColorSpace::MakeSRGB();
    }
    SkImageInfo info = SkImageInfo::Make(options.size.width(), options.size.height(), colorType,
                                         kPremul_SkAlphaType, colorSpace);
    if (options.raster) {
        auto rasterSurface = SkSurface::MakeRaster(info);
        srand(0);
        draw(rasterSurface->getCanvas());
        rasterData.reset(encode_snapshot(rasterSurface));
    }
    if (options.gpu) {
        auto grContext = create_grcontext();
        if (!grContext) {
            fputs("Unable to get GrContext.\n", stderr);
        } else {
            auto surface = SkSurface::MakeRenderTarget(grContext.get(), SkBudgeted::kNo, info);
            if (!surface) {
                fputs("Unable to get render surface.\n", stderr);
                exit(1);
            }
            srand(0);
            draw(surface->getCanvas());
            gpuData.reset(encode_snapshot(surface));
        }
    }
    if (options.pdf) {
        SkDynamicMemoryWStream pdfStream;
        sk_sp<SkDocument> document(SkDocument::MakePDF(&pdfStream));
        if (document) {
            srand(0);
            draw(document->beginPage(options.size.width(), options.size.height()));
            document->close();
            pdfData = pdfStream.detachAsData();
        }
    }
    if (options.skp) {
        SkSize size;
        size = options.size;
        SkPictureRecorder recorder;
        srand(0);
        draw(recorder.beginRecording(size.width(), size.height()));
        auto picture = recorder.finishRecordingAsPicture();
        SkDynamicMemoryWStream skpStream;
        picture->serialize(&skpStream);
        skpData = skpStream.detachAsData();
    }
    bool textOnly = options.textOnly;

    printf("{\n");
    dump_output(rasterData, "Raster", !gpuData && !pdfData && !skpData && !textOnly);
    dump_output(gpuData, "Gpu", !pdfData && !skpData && !textOnly);
    dump_output(pdfData, "Pdf", !skpData && !textOnly);
    dump_output(skpData, "Skp", !textOnly);
    dump_text(textoutput, "Text");
    printf("}\n");

    return 0;
}
