#ifndef DMBenchTask_DEFINED
#define DMBenchTask_DEFINED

#include "DMReporter.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "SkBenchmark.h"
#include "SkString.h"
#include "SkTemplates.h"

// Tasks that run an SkBenchmark once as a check that it doesn't crash.

namespace DM {

class NonRenderingBenchTask : public Task {
public:
    NonRenderingBenchTask(const char* config, Reporter*, TaskRunner*, BenchRegistry::Factory);

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return false; }
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    SkAutoTDelete<SkBenchmark> fBench;
    const SkString fName;
};

class CpuBenchTask : public Task {
public:
    CpuBenchTask(const char* config, Reporter*, TaskRunner*, BenchRegistry::Factory, SkColorType);

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return false; }
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    SkAutoTDelete<SkBenchmark> fBench;
    const SkString fName;
    const SkColorType fColorType;
};

class GpuBenchTask : public Task {
public:
    GpuBenchTask(const char* config,
                 Reporter*,
                 TaskRunner*,
                 BenchRegistry::Factory,
                 GrContextFactory::GLContextType,
                 int sampleCount);

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return true; }
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    SkAutoTDelete<SkBenchmark> fBench;
    const SkString fName;
    const GrContextFactory::GLContextType fContextType;
    int fSampleCount;
};

}  // namespace DM

#endif // DMBenchTask_DEFINED
