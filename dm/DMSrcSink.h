#ifndef DMSrcSink_DEFINED
#define DMSrcSink_DEFINED

#include "DMGpuSupport.h"
#include "SkBBHFactory.h"
#include "SkBBoxHierarchy.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkGPipe.h"
#include "SkPicture.h"
#include "gm.h"

namespace DM {

// This is just convenience.  It lets you use either return "foo" or return SkStringPrintf(...).
struct ImplicitString : public SkString {
    template <typename T>
    ImplicitString(const T& s) : SkString(s) {}
};
typedef ImplicitString Error;
typedef ImplicitString Name;

struct Src {
    // All Srcs must be thread safe.
    virtual ~Src() {}
    virtual Error SK_WARN_UNUSED_RESULT draw(SkCanvas*) const = 0;
    virtual SkISize size() const = 0;
    virtual Name name() const = 0;
};

struct Sink {
    virtual ~Sink() {}
    // You may write to either the bitmap or stream.
    virtual Error SK_WARN_UNUSED_RESULT draw(const Src&, SkBitmap*, SkWStream*) const
        = 0;
    // Sinks in the same enclave (except kAnyThread_Enclave) will run serially on the same thread.
    virtual int enclave() const = 0;

    // File extension for the content draw() outputs, e.g. "png", "pdf".
    virtual const char* fileExtension() const  = 0;
};

enum { kAnyThread_Enclave, kGPU_Enclave, kPDFSink_Enclave };
static const int kNumEnclaves = kPDFSink_Enclave + 1;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class GMSrc : public Src {
public:
    explicit GMSrc(skiagm::GMRegistry::Factory);

    Error draw(SkCanvas*) const SK_OVERRIDE;
    SkISize size() const SK_OVERRIDE;
    Name name() const SK_OVERRIDE;
private:
    skiagm::GMRegistry::Factory fFactory;
};

class ImageSrc : public Src {
public:
    explicit ImageSrc(SkString path, int subsets = 0);

    Error draw(SkCanvas*) const SK_OVERRIDE;
    SkISize size() const SK_OVERRIDE;
    Name name() const SK_OVERRIDE;
private:
    SkString                     fPath;
    int                          fSubsets;
};

class SKPSrc : public Src {
public:
    explicit SKPSrc(SkString path);

    Error draw(SkCanvas*) const SK_OVERRIDE;
    SkISize size() const SK_OVERRIDE;
    Name name() const SK_OVERRIDE;
private:
    SkString                        fPath;
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class GPUSink : public Sink {
public:
    GPUSink(GrContextFactory::GLContextType, GrGLStandard, int samples, bool dfText, bool threaded);

    Error draw(const Src&, SkBitmap*, SkWStream*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE;
    const char* fileExtension() const SK_OVERRIDE { return "png"; }
private:
    GrContextFactory::GLContextType fContextType;
    GrGLStandard                    fGpuAPI;
    int                             fSampleCount;
    bool                            fUseDFText;
    bool                            fThreaded;
};

class PDFSink : public Sink {
public:
    PDFSink();

    Error draw(const Src&, SkBitmap*, SkWStream*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE { return kPDFSink_Enclave; }
    const char* fileExtension() const SK_OVERRIDE { return "pdf"; }
};

class RasterSink : public Sink {
public:
    explicit RasterSink(SkColorType);

    Error draw(const Src&, SkBitmap*, SkWStream*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE { return kAnyThread_Enclave; }
    const char* fileExtension() const SK_OVERRIDE { return "png"; }
private:
    SkColorType    fColorType;
};

class SKPSink : public Sink {
public:
    SKPSink();

    Error draw(const Src&, SkBitmap*, SkWStream*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE { return kAnyThread_Enclave; }
    const char* fileExtension() const SK_OVERRIDE { return "skp"; }
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class ViaMatrix : public Sink {
public:
    ViaMatrix(SkMatrix, Sink*);

    Error draw(const Src&, SkBitmap*, SkWStream*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE { return fSink->enclave(); }
    const char* fileExtension() const SK_OVERRIDE { return fSink->fileExtension(); }
private:
    SkMatrix            fMatrix;
    SkAutoTDelete<Sink> fSink;
};

class ViaPipe : public Sink {
public:
    explicit ViaPipe(Sink*);

    Error draw(const Src&, SkBitmap*, SkWStream*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE { return fSink->enclave(); }
    const char* fileExtension() const SK_OVERRIDE { return fSink->fileExtension(); }
private:
    SkAutoTDelete<Sink>  fSink;
};

class ViaSerialization : public Sink {
public:
    explicit ViaSerialization(Sink*);

    Error draw(const Src&, SkBitmap*, SkWStream*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE { return fSink->enclave(); }
    const char* fileExtension() const SK_OVERRIDE { return fSink->fileExtension(); }
private:
    SkAutoTDelete<Sink> fSink;
};

class ViaTiles : public Sink {
public:
    ViaTiles(int w, int h, SkBBHFactory*, Sink*);

    Error draw(const Src&, SkBitmap*, SkWStream*) const SK_OVERRIDE;
    int enclave() const SK_OVERRIDE { return fSink->enclave(); }
    const char* fileExtension() const SK_OVERRIDE { return fSink->fileExtension(); }
private:
    const int                   fW, fH;
    SkAutoTDelete<SkBBHFactory> fFactory;
    SkAutoTDelete<Sink>         fSink;
};

}  // namespace DM

#endif//DMSrcSink_DEFINED
