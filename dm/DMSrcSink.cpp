#include "DMSrcSink.h"
#include "SamplePipeControllers.h"
#include "SkCommonFlags.h"
#include "SkDocument.h"
#include "SkMultiPictureDraw.h"
#include "SkOSFile.h"
#include "SkPictureRecorder.h"
#include "SkRandom.h"
#include "SkTLS.h"

namespace DM {

void SafeUnref(SkPicture* p) { SkSafeUnref(p); }
void SafeUnref(SkData*    d) { SkSafeUnref(d); }

// FIXME: the GM objects themselves are not threadsafe, so we create and destroy them as needed.

GMSrc::GMSrc(skiagm::GMRegistry::Factory factory) : fFactory(factory) {}

Error GMSrc::draw(SkCanvas* canvas) const {
    SkAutoTDelete<skiagm::GM> gm(fFactory(NULL));
    canvas->concat(gm->getInitialTransform());
    gm->draw(canvas);
    return "";
}

SkISize GMSrc::size() const {
    SkAutoTDelete<skiagm::GM> gm(fFactory(NULL));
    return gm->getISize();
}

Name GMSrc::name() const {
    SkAutoTDelete<skiagm::GM> gm(fFactory(NULL));
    return gm->getName();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// The first call to draw() or size() will mmap the file to an SkData.  ~ImageSrc unrefs it.
struct LazyLoadImage {
    LazyLoadImage(const char* path) : path(path) {}
    const char* path;

    SkData* operator()() const { return SkData::NewFromFileName(path); }
};

ImageSrc::ImageSrc(SkString path, int subsets) : fPath(path), fSubsets(subsets) {}

Error ImageSrc::draw(SkCanvas* canvas) const {
    const SkData* encoded = fEncoded.get(LazyLoadImage(fPath.c_str()));
    if (!encoded) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }
    if (fSubsets == 0) {
        // Decode the full image.
        SkBitmap bitmap;
        if (!SkImageDecoder::DecodeMemory(encoded->data(), encoded->size(), &bitmap)) {
            return SkStringPrintf("Couldn't decode %s.", fPath.c_str());
        }
        canvas->drawBitmap(bitmap, 0,0);
        return "";
    }
    // Decode random subsets.  This is a little involved.
    SkMemoryStream stream(encoded->data(), encoded->size());
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(&stream));
    if (!decoder) {
        return SkStringPrintf("Can't find a good decoder for %s.", fPath.c_str());
    }
    int w,h;
    if (!decoder->buildTileIndex(&stream, &w, &h) || w*h == 1) {
        return "";  // Not an error.  Subset decoding is not always supported.
    }
    SkRandom rand;
    for (int i = 0; i < fSubsets; i++) {
        SkIRect rect;
        do {
            rect.fLeft   = rand.nextULessThan(w);
            rect.fTop    = rand.nextULessThan(h);
            rect.fRight  = rand.nextULessThan(w);
            rect.fBottom = rand.nextULessThan(h);
            rect.sort();
        } while (rect.isEmpty());
        SkBitmap subset;
        if (!decoder->decodeSubset(&subset, rect, kUnknown_SkColorType/*use best fit*/)) {
            return SkStringPrintf("Could not decode subset %d.\n", i);
        }
        canvas->drawBitmap(subset, SkIntToScalar(rect.fLeft), SkIntToScalar(rect.fTop));
    }
    return "";
}

SkISize ImageSrc::size() const {
    const SkData* encoded = fEncoded.get(LazyLoadImage(fPath.c_str()));
    SkBitmap bitmap;
    if (!encoded || !SkImageDecoder::DecodeMemory(encoded->data(),
                                                  encoded->size(),
                                                  &bitmap,
                                                  kUnknown_SkColorType,
                                                  SkImageDecoder::kDecodeBounds_Mode)) {
        return SkISize::Make(0,0);
    }
    return bitmap.dimensions();
}

Name ImageSrc::name() const { return SkOSPath::Basename(fPath.c_str()); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static const SkRect kSKPViewport = {0,0, 1000,1000};

// The first call to draw() or size() will read the file into an SkPicture.  ~SKPSrc unrefs it.
struct LazyLoadPicture {
    LazyLoadPicture(const char* path) : path(path) {}
    const char* path;

    SkPicture* operator()() const {
        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path));
        if (!stream) {
            return NULL;
        }
        return SkPicture::CreateFromStream(stream);
    }
};

SKPSrc::SKPSrc(SkString path) : fPath(path) {}

Error SKPSrc::draw(SkCanvas* canvas) const {
    const SkPicture* pic = fPic.get(LazyLoadPicture(fPath.c_str()));
    if (!pic) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }
    canvas->clipRect(kSKPViewport);
    canvas->drawPicture(pic);
    return "";
}

SkISize SKPSrc::size() const {
    const SkPicture* pic = fPic.get(LazyLoadPicture(fPath.c_str()));
    if (!pic) {
        return SkISize::Make(0,0);
    }
    SkRect cull = pic->cullRect();
    if (!cull.intersect(kSKPViewport)) {
        sk_throw();
    }
    SkIRect bounds;
    cull.roundOut(&bounds);
    SkISize size = { bounds.width(), bounds.height() };
    return size;
}

Name SKPSrc::name() const { return SkOSPath::Basename(fPath.c_str()); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

DEFINE_string(gpu_threading, "none",
        "none:  single thread,\n"
        "tls:   any thread, GrContextFactory in TLS   (crashy),\n"
        "stack: any thread, GrContextFactory on stack (less crashy, differently so)");

GPUSink::GPUSink(GrContextFactory::GLContextType ct, GrGLStandard api, int samples, bool dfText)
    : fContextType(ct)
    , fGpuAPI(api)
    , fSampleCount(samples)
    , fUseDFText(dfText) {}

int GPUSink::enclave() const {
    return FLAGS_gpu_threading.contains("none") ? kGPUSink_Enclave : kAnyThread_Enclave;
}

static void* CreateGrFactory()        { return new GrContextFactory; }
static void  DeleteGrFactory(void* p) { delete (GrContextFactory*)p; }

Error GPUSink::draw(const Src& src, SkBitmap* dst, SkWStream*) const {
    GrContextFactory local, *factory = &local;
    if (!FLAGS_gpu_threading.contains("stack")) {
        factory = (GrContextFactory*)SkTLS::Get(CreateGrFactory, DeleteGrFactory);
    }
    // Does abandoning / resetting contexts make any sense if we have stack-scoped factories?
    if (FLAGS_abandonGpuContext) {
        factory->abandonContexts();
    }
    if (FLAGS_resetGpuContext || FLAGS_abandonGpuContext) {
        factory->destroyContexts();
    }
    const SkISize size = src.size();
    const SkImageInfo info =
        SkImageInfo::Make(size.width(), size.height(), kN32_SkColorType, kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(
            NewGpuSurface(factory, fContextType, fGpuAPI, info, fSampleCount, fUseDFText));
    if (!surface) {
        return "Could not create a surface.";
    }
    SkCanvas* canvas = surface->getCanvas();
    Error err = src.draw(canvas);
    if (!err.isEmpty()) {
        return err;
    }
    canvas->flush();
    dst->allocPixels(info);
    canvas->readPixels(dst, 0,0);
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

PDFSink::PDFSink() {}

Error PDFSink::draw(const Src& src, SkBitmap*, SkWStream* dst) const {
    SkSize size;
    size = src.size();
    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(dst));
    SkCanvas* canvas = doc->beginPage(size.width(), size.height());

    Error err = src.draw(canvas);
    if (!err.isEmpty()) {
        return err;
    }
    canvas->flush();
    doc->endPage();
    doc->close();
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

RasterSink::RasterSink(SkColorType colorType) : fColorType(colorType) {}

Error RasterSink::draw(const Src& src, SkBitmap* dst, SkWStream*) const {
    const SkISize size = src.size();
    // If there's an appropriate alpha type for this color type, use it, otherwise use premul.
    SkAlphaType alphaType = kPremul_SkAlphaType;
    (void)SkColorTypeValidateAlphaType(fColorType, alphaType, &alphaType);

    dst->allocPixels(SkImageInfo::Make(size.width(), size.height(), fColorType, alphaType));
    dst->eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(*dst);
    return src.draw(&canvas);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaMatrix::ViaMatrix(SkMatrix matrix, Sink* sink) : fMatrix(matrix), fSink(sink) {}

Error ViaMatrix::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream) const {
    // We turn our arguments into a Src, then draw that Src into our Sink to fill bitmap or stream.
    struct ProxySrc : public Src {
        const Src& fSrc;
        SkMatrix fMatrix;
        ProxySrc(const Src& src, SkMatrix matrix) : fSrc(src), fMatrix(matrix) {}

        Error draw(SkCanvas* canvas) const SK_OVERRIDE {
            canvas->concat(fMatrix);
            return fSrc.draw(canvas);
        }
        SkISize size() const SK_OVERRIDE { return fSrc.size(); }
        Name name() const SK_OVERRIDE { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(src, fMatrix);
    return fSink->draw(proxy, bitmap, stream);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaPipe::ViaPipe(int flags, Sink* sink) : fFlags((SkGPipeWriter::Flags)flags), fSink(sink) {}

Error ViaPipe::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream) const {
    // We turn our arguments into a Src, then draw that Src into our Sink to fill bitmap or stream.
    struct ProxySrc : public Src {
        const Src& fSrc;
        SkGPipeWriter::Flags fFlags;
        ProxySrc(const Src& src, SkGPipeWriter::Flags flags) : fSrc(src), fFlags(flags) {}

        Error draw(SkCanvas* canvas) const SK_OVERRIDE {
            SkISize size = this->size();
            // TODO: is DecodeMemory really required? Might help RAM usage to be lazy if we can.
            PipeController controller(canvas, &SkImageDecoder::DecodeMemory);
            SkGPipeWriter pipe;
            return fSrc.draw(pipe.startRecording(&controller, fFlags, size.width(), size.height()));
        }
        SkISize size() const SK_OVERRIDE { return fSrc.size(); }
        Name name() const SK_OVERRIDE { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(src, fFlags);
    return fSink->draw(proxy, bitmap, stream);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaSerialization::ViaSerialization(Sink* sink) : fSink(sink) {}

Error ViaSerialization::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream) const {
    // Record our Src into a picture.
    SkSize size;
    size = src.size();
    SkPictureRecorder recorder;
    Error err = src.draw(recorder.beginRecording(size.width(), size.height()));
    if (!err.isEmpty()) {
        return err;
    }
    SkAutoTUnref<SkPicture> pic(recorder.endRecording());

    // Serialize it and then deserialize it.
    SkDynamicMemoryWStream wStream;
    pic->serialize(&wStream);
    SkAutoTUnref<SkStream> rStream(wStream.detachAsStream());
    SkAutoTUnref<SkPicture> deserialized(SkPicture::CreateFromStream(rStream));

    // Turn that deserialized picture into a Src, draw it into our Sink to fill bitmap or stream.
    struct ProxySrc : public Src {
        const SkPicture* fPic;
        const SkISize fSize;
        ProxySrc(const SkPicture* pic, SkISize size) : fPic(pic), fSize(size) {}

        Error draw(SkCanvas* canvas) const SK_OVERRIDE {
            canvas->drawPicture(fPic);
            return "";
        }
        SkISize size() const SK_OVERRIDE { return fSize; }
        Name name() const SK_OVERRIDE { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(deserialized, src.size());
    return fSink->draw(proxy, bitmap, stream);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaTiles::ViaTiles(int w, int h, SkBBHFactory* factory, Sink* sink)
    : fW(w)
    , fH(h)
    , fFactory(factory)
    , fSink(sink) {}

Error ViaTiles::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream) const {
    // Record our Src into a picture.
    SkSize size;
    size = src.size();
    SkPictureRecorder recorder;
    Error err = src.draw(recorder.beginRecording(size.width(), size.height(), fFactory.get()));
    if (!err.isEmpty()) {
        return err;
    }
    SkAutoTUnref<SkPicture> pic(recorder.endRecording());

    // Turn that picture into a Src that draws into our Sink via tiles + MPD.
    struct ProxySrc : public Src {
        const int fW, fH;
        const SkPicture* fPic;
        const SkISize fSize;
        ProxySrc(int w, int h, const SkPicture* pic, SkISize size)
            : fW(w), fH(h), fPic(pic), fSize(size) {}

        Error draw(SkCanvas* canvas) const SK_OVERRIDE {
            const int xTiles = (fSize.width()  + fW - 1) / fW,
                      yTiles = (fSize.height() + fH - 1) / fH;
            SkMultiPictureDraw mpd(xTiles*yTiles);
            SkTDArray<SkSurface*> surfaces;
            surfaces.setReserve(xTiles*yTiles);

            SkImageInfo info = canvas->imageInfo().makeWH(fW, fH);
            for (int j = 0; j < yTiles; j++) {
                for (int i = 0; i < xTiles; i++) {
                    // This lets our ultimate Sink determine the best kind of surface.
                    // E.g., if it's a GpuSink, the surfaces and images are textures.
                    SkSurface* s = canvas->newSurface(info);
                    if (!s) {
                        s = SkSurface::NewRaster(info);  // Some canvases can't create surfaces.
                    }
                    surfaces.push(s);
                    SkCanvas* c = s->getCanvas();
                    c->translate(SkIntToScalar(-i * fW),
                                 SkIntToScalar(-j * fH));  // Line up the canvas with this tile.
                    mpd.add(c, fPic);
                }
            }
            mpd.draw();
            for (int j = 0; j < yTiles; j++) {
                for (int i = 0; i < xTiles; i++) {
                    SkAutoTUnref<SkImage> image(surfaces[i+xTiles*j]->newImageSnapshot());
                    canvas->drawImage(image, SkIntToScalar(i*fW), SkIntToScalar(j*fH));
                }
            }
            surfaces.unrefAll();
            return "";
        }
        SkISize size() const SK_OVERRIDE { return fSize; }
        Name name() const SK_OVERRIDE { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(fW, fH, pic, src.size());
    return fSink->draw(proxy, bitmap, stream);
}

}  // namespace DM
