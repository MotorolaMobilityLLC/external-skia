/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ctype.h>

#include "Benchmark.h"
#include "CrashHandler.h"
#include "GMBench.h"
#include "ResultsWriter.h"
#include "SKPBench.h"
#include "Stats.h"
#include "Timer.h"

#include "SkOSFile.h"
#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkString.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
    #include "gl/GrGLDefines.h"
    #include "GrContextFactory.h"
    GrContextFactory gGrFactory;
#endif

__SK_FORCE_IMAGE_DECODER_LINKING;

#if SK_DEBUG
    DEFINE_bool(runOnce, true, "Run each benchmark just once?");
#else
    DEFINE_bool(runOnce, false, "Run each benchmark just once?");
#endif

DEFINE_int32(samples, 10, "Number of samples to measure for each bench.");
DEFINE_int32(overheadLoops, 100000, "Loops to estimate timer overhead.");
DEFINE_double(overheadGoal, 0.0001,
              "Loop until timer overhead is at most this fraction of our measurments.");
DEFINE_double(gpuMs, 5, "Target bench time in millseconds for GPU.");
DEFINE_int32(gpuFrameLag, 5, "Overestimate of maximum number of frames GPU allows to lag.");

DEFINE_string(outResultsFile, "", "If given, write results here as JSON.");
DEFINE_int32(maxCalibrationAttempts, 3,
             "Try up to this many times to guess loops for a bench, or skip the bench.");
DEFINE_int32(maxLoops, 1000000, "Never run a bench more times than this.");
DEFINE_string(key, "", "Space-separated key/value pairs to add to JSON.");
DEFINE_string(gitHash, "", "Git hash to add to JSON.");

DEFINE_string(clip, "0,0,1000,1000", "Clip for SKPs.");
DEFINE_string(scales, "1.0", "Space-separated scales for SKPs.");

static SkString humanize(double ms) {
    if (ms > 1e+3) return SkStringPrintf("%.3gs",  ms/1e3);
    if (ms < 1e-3) return SkStringPrintf("%.3gns", ms*1e6);
#ifdef SK_BUILD_FOR_WIN
    if (ms < 1)    return SkStringPrintf("%.3gus", ms*1e3);
#else
    if (ms < 1)    return SkStringPrintf("%.3gµs", ms*1e3);
#endif
    return SkStringPrintf("%.3gms", ms);
}
#define HUMANIZE(ms) humanize(ms).c_str()

static double time(int loops, Benchmark* bench, SkCanvas* canvas, SkGLContextHelper* gl) {
    WallTimer timer;
    timer.start();
    if (bench) {
        bench->draw(loops, canvas);
    }
    if (canvas) {
        canvas->flush();
    }
#if SK_SUPPORT_GPU
    if (gl) {
        SK_GL(*gl, Flush());
        gl->swapBuffers();
    }
#endif
    timer.end();
    return timer.fWall;
}

static double estimate_timer_overhead() {
    double overhead = 0;
    for (int i = 0; i < FLAGS_overheadLoops; i++) {
        overhead += time(1, NULL, NULL, NULL);
    }
    return overhead / FLAGS_overheadLoops;
}

static int clamp_loops(int loops) {
    if (loops < 1) {
        SkDebugf("ERROR: clamping loops from %d to 1.\n", loops);
        return 1;
    }
    if (loops > FLAGS_maxLoops) {
        SkDebugf("WARNING: clamping loops from %d to FLAGS_maxLoops, %d.\n", loops, FLAGS_maxLoops);
        return FLAGS_maxLoops;
    }
    return loops;
}

static int cpu_bench(const double overhead, Benchmark* bench, SkCanvas* canvas, double* samples) {
    // First figure out approximately how many loops of bench it takes to make overhead negligible.
    double bench_plus_overhead = 0.0;
    int round = 0;
    while (bench_plus_overhead < overhead) {
        if (round++ == FLAGS_maxCalibrationAttempts) {
            SkDebugf("WARNING: Can't estimate loops for %s (%s vs. %s); skipping.\n",
                     bench->getName(), HUMANIZE(bench_plus_overhead), HUMANIZE(overhead));
            return 0;
        }
        bench_plus_overhead = time(1, bench, canvas, NULL);
    }

    // Later we'll just start and stop the timer once but loop N times.
    // We'll pick N to make timer overhead negligible:
    //
    //          overhead
    //  -------------------------  < FLAGS_overheadGoal
    //  overhead + N * Bench Time
    //
    // where bench_plus_overhead ≈ overhead + Bench Time.
    //
    // Doing some math, we get:
    //
    //  (overhead / FLAGS_overheadGoal) - overhead
    //  ------------------------------------------  < N
    //       bench_plus_overhead - overhead)
    //
    // Luckily, this also works well in practice. :)
    const double numer = overhead / FLAGS_overheadGoal - overhead;
    const double denom = bench_plus_overhead - overhead;
    const int loops = clamp_loops(FLAGS_runOnce ? 1 : (int)ceil(numer / denom));

    for (int i = 0; i < FLAGS_samples; i++) {
        samples[i] = time(loops, bench, canvas, NULL) / loops;
    }
    return loops;
}

#if SK_SUPPORT_GPU
static int gpu_bench(SkGLContextHelper* gl,
                     Benchmark* bench,
                     SkCanvas* canvas,
                     double* samples) {
    gl->makeCurrent();
    // Make sure we're done with whatever came before.
    SK_GL(*gl, Finish());

    // First, figure out how many loops it'll take to get a frame up to FLAGS_gpuMs.
    int loops = 1;
    if (!FLAGS_runOnce) {
        double elapsed = 0;
        do {
            loops *= 2;
            // If the GPU lets frames lag at all, we need to make sure we're timing
            // _this_ round, not still timing last round.  We force this by looping
            // more times than any reasonable GPU will allow frames to lag.
            for (int i = 0; i < FLAGS_gpuFrameLag; i++) {
                elapsed = time(loops, bench, canvas, gl);
            }
        } while (elapsed < FLAGS_gpuMs);

        // We've overshot at least a little.  Scale back linearly.
        loops = (int)ceil(loops * FLAGS_gpuMs / elapsed);

        // Might as well make sure we're not still timing our calibration.
        SK_GL(*gl, Finish());
    }
    loops = clamp_loops(loops);

    // Pretty much the same deal as the calibration: do some warmup to make
    // sure we're timing steady-state pipelined frames.
    for (int i = 0; i < FLAGS_gpuFrameLag; i++) {
        time(loops, bench, canvas, gl);
    }

    // Now, actually do the timing!
    for (int i = 0; i < FLAGS_samples; i++) {
        samples[i] = time(loops, bench, canvas, gl) / loops;
    }
    return loops;
}
#endif

static SkString to_lower(const char* str) {
    SkString lower(str);
    for (size_t i = 0; i < lower.size(); i++) {
        lower[i] = tolower(lower[i]);
    }
    return lower;
}

struct Config {
    const char* name;
    Benchmark::Backend backend;
    SkColorType color;
    SkAlphaType alpha;
    int samples;
#if SK_SUPPORT_GPU
    GrContextFactory::GLContextType ctxType;
#else
    int bogusInt;
#endif
};

struct Target {
    explicit Target(const Config& c) : config(c) {}
    const Config config;
    SkAutoTDelete<SkSurface> surface;
#if SK_SUPPORT_GPU
    SkGLContextHelper* gl;
#endif
};

static bool is_cpu_config_allowed(const char* name) {
    for (int i = 0; i < FLAGS_config.count(); i++) {
        if (to_lower(FLAGS_config[i]).equals(name)) {
            return true;
        }
    }
    return false;
}

#if SK_SUPPORT_GPU
static bool is_gpu_config_allowed(const char* name, GrContextFactory::GLContextType ctxType,
                                  int sampleCnt) {
    if (!is_cpu_config_allowed(name)) {
        return false;
    }
    if (const GrContext* ctx = gGrFactory.get(ctxType)) {
        return sampleCnt <= ctx->getMaxSampleCount();
    }
    return false;
}
#endif

#if SK_SUPPORT_GPU
#define kBogusGLContextType GrContextFactory::kNative_GLContextType
#else
#define kBogusGLContextType 0
#endif

// Append all configs that are enabled and supported.
static void create_configs(SkTDArray<Config>* configs) {
    #define CPU_CONFIG(name, backend, color, alpha)                                               \
        if (is_cpu_config_allowed(#name)) {                                                       \
            Config config = { #name, Benchmark::backend, color, alpha, 0, kBogusGLContextType };  \
            configs->push(config);                                                                \
        }

    if (FLAGS_cpu) {
        CPU_CONFIG(nonrendering, kNonRendering_Backend, kUnknown_SkColorType, kUnpremul_SkAlphaType)
        CPU_CONFIG(8888, kRaster_Backend, kN32_SkColorType, kPremul_SkAlphaType)
        CPU_CONFIG(565, kRaster_Backend, kRGB_565_SkColorType, kOpaque_SkAlphaType)
    }

#if SK_SUPPORT_GPU
    #define GPU_CONFIG(name, ctxType, samples)                                   \
        if (is_gpu_config_allowed(#name, GrContextFactory::ctxType, samples)) {  \
            Config config = {                                                    \
                #name,                                                           \
                Benchmark::kGPU_Backend,                                         \
                kN32_SkColorType,                                                \
                kPremul_SkAlphaType,                                             \
                samples,                                                         \
                GrContextFactory::ctxType };                                     \
            configs->push(config);                                               \
        }

    if (FLAGS_gpu) {
        GPU_CONFIG(gpu, kNative_GLContextType, 0)
        GPU_CONFIG(msaa4, kNative_GLContextType, 4)
        GPU_CONFIG(msaa16, kNative_GLContextType, 16)
        GPU_CONFIG(nvprmsaa4, kNVPR_GLContextType, 4)
        GPU_CONFIG(nvprmsaa16, kNVPR_GLContextType, 16)
        GPU_CONFIG(debug, kDebug_GLContextType, 0)
        GPU_CONFIG(nullgpu, kNull_GLContextType, 0)
    }
#endif
}

// If bench is enabled for config, returns a Target* for it, otherwise NULL.
static Target* is_enabled(Benchmark* bench, const Config& config) {
    if (!bench->isSuitableFor(config.backend)) {
        return NULL;
    }

    SkImageInfo info;
    info.fAlphaType = config.alpha;
    info.fColorType = config.color;
    info.fWidth = bench->getSize().fX;
    info.fHeight = bench->getSize().fY;

    Target* target = new Target(config);

    if (Benchmark::kRaster_Backend == config.backend) {
        target->surface.reset(SkSurface::NewRaster(info));
    }
#if SK_SUPPORT_GPU
    else if (Benchmark::kGPU_Backend == config.backend) {
        target->surface.reset(SkSurface::NewRenderTarget(gGrFactory.get(config.ctxType), info,
                                                         config.samples));
        target->gl = gGrFactory.getGLContext(config.ctxType);
    }
#endif

    if (Benchmark::kNonRendering_Backend != config.backend && !target->surface.get()) {
        delete target;
        return NULL;
    }
    return target;
}

// Creates targets for a benchmark and a set of configs.
static void create_targets(SkTDArray<Target*>* targets, Benchmark* b,
                           const SkTDArray<Config>& configs) {
    for (int i = 0; i < configs.count(); ++i) {
        if (Target* t = is_enabled(b, configs[i])) {
            targets->push(t);
        }

    }
}

static void fill_static_options(ResultsWriter* log) {
#if defined(SK_BUILD_FOR_WIN32)
    log->option("system", "WIN32");
#elif defined(SK_BUILD_FOR_MAC)
    log->option("system", "MAC");
#elif defined(SK_BUILD_FOR_ANDROID)
    log->option("system", "ANDROID");
#elif defined(SK_BUILD_FOR_UNIX)
    log->option("system", "UNIX");
#else
    log->option("system", "other");
#endif
}

#if SK_SUPPORT_GPU
static void fill_gpu_options(ResultsWriter* log, SkGLContextHelper* ctx) {
    const GrGLubyte* version;
    SK_GL_RET(*ctx, version, GetString(GR_GL_VERSION));
    log->configOption("GL_VERSION", (const char*)(version));

    SK_GL_RET(*ctx, version, GetString(GR_GL_RENDERER));
    log->configOption("GL_RENDERER", (const char*) version);

    SK_GL_RET(*ctx, version, GetString(GR_GL_VENDOR));
    log->configOption("GL_VENDOR", (const char*) version);

    SK_GL_RET(*ctx, version, GetString(GR_GL_SHADING_LANGUAGE_VERSION));
    log->configOption("GL_SHADING_LANGUAGE_VERSION", (const char*) version);
}
#endif

class BenchmarkStream {
public:
    BenchmarkStream() : fBenches(BenchRegistry::Head())
                      , fGMs(skiagm::GMRegistry::Head())
                      , fCurrentScale(0)
                      , fCurrentSKP(0) {
        for (int i = 0; i < FLAGS_skps.count(); i++) {
            if (SkStrEndsWith(FLAGS_skps[i], ".skp")) {
                fSKPs.push_back() = FLAGS_skps[i];
            } else {
                SkOSFile::Iter it(FLAGS_skps[i], ".skp");
                SkString path;
                while (it.next(&path)) {
                    fSKPs.push_back() = SkOSPath::Join(FLAGS_skps[0], path.c_str());
                }
            }
        }

        if (4 != sscanf(FLAGS_clip[0], "%d,%d,%d,%d",
                        &fClip.fLeft, &fClip.fTop, &fClip.fRight, &fClip.fBottom)) {
            SkDebugf("Can't parse %s from --clip as an SkIRect.\n", FLAGS_clip[0]);
            exit(1);
        }

        for (int i = 0; i < FLAGS_scales.count(); i++) {
            if (1 != sscanf(FLAGS_scales[i], "%f", &fScales.push_back())) {
                SkDebugf("Can't parse %s from --scales as an SkScalar.\n", FLAGS_scales[i]);
                exit(1);
            }
        }
    }

    Benchmark* next() {
        if (fBenches) {
            Benchmark* bench = fBenches->factory()(NULL);
            fBenches = fBenches->next();
            fSourceType = "bench";
            return bench;
        }

        while (fGMs) {
            SkAutoTDelete<skiagm::GM> gm(fGMs->factory()(NULL));
            fGMs = fGMs->next();
            if (gm->getFlags() & skiagm::GM::kAsBench_Flag) {
                fSourceType = "gm";
                return SkNEW_ARGS(GMBench, (gm.detach()));
            }
        }

        while (fCurrentScale < fScales.count()) {
            while (fCurrentSKP < fSKPs.count()) {
                const SkString& path = fSKPs[fCurrentSKP++];

                // Not strictly necessary, as it will be checked again later,
                // but helps to avoid a lot of pointless work if we're going to skip it.
                if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path.c_str())) {
                    continue;
                }

                SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path.c_str()));
                if (stream.get() == NULL) {
                    SkDebugf("Could not read %s.\n", path.c_str());
                    exit(1);
                }

                SkAutoTUnref<SkPicture> pic(SkPicture::CreateFromStream(stream.get()));
                if (pic.get() == NULL) {
                    SkDebugf("Could not read %s as an SkPicture.\n", path.c_str());
                    exit(1);
                }

                SkString name = SkOSPath::Basename(path.c_str());

                fSourceType = "skp";
                return SkNEW_ARGS(SKPBench,
                        (name.c_str(), pic.get(), fClip, fScales[fCurrentScale]));
            }
            fCurrentSKP = 0;
            fCurrentScale++;
        }

        return NULL;
    }

    void fillCurrentOptions(ResultsWriter* log) const {
        log->configOption("source_type", fSourceType);
        if (0 == strcmp(fSourceType, "skp")) {
            log->configOption("clip",
                    SkStringPrintf("%d %d %d %d", fClip.fLeft, fClip.fTop,
                                                  fClip.fRight, fClip.fBottom).c_str());
            log->configOption("scale", SkStringPrintf("%.2g", fScales[fCurrentScale]).c_str());
        }
    }

private:
    const BenchRegistry* fBenches;
    const skiagm::GMRegistry* fGMs;
    SkIRect            fClip;
    SkTArray<SkScalar> fScales;
    SkTArray<SkString> fSKPs;

    const char* fSourceType;
    int fCurrentScale;
    int fCurrentSKP;
};

int nanobench_main();
int nanobench_main() {
    SetupCrashHandler();
    SkAutoGraphics ag;

    if (FLAGS_runOnce) {
        FLAGS_samples     = 1;
        FLAGS_gpuFrameLag = 0;
    }

    MultiResultsWriter log;
    SkAutoTDelete<NanoJSONResultsWriter> json;
    if (!FLAGS_outResultsFile.isEmpty()) {
        const char* gitHash = FLAGS_gitHash.isEmpty() ? "unknown-revision" : FLAGS_gitHash[0];
        json.reset(SkNEW(NanoJSONResultsWriter(FLAGS_outResultsFile[0], gitHash)));
        log.add(json.get());
    }
    CallEnd<MultiResultsWriter> ender(log);

    if (1 == FLAGS_key.count() % 2) {
        SkDebugf("ERROR: --key must be passed with an even number of arguments.\n");
        return 1;
    }
    for (int i = 1; i < FLAGS_key.count(); i += 2) {
        log.key(FLAGS_key[i-1], FLAGS_key[i]);
    }
    fill_static_options(&log);

    const double overhead = estimate_timer_overhead();
    SkDebugf("Timer overhead: %s\n", HUMANIZE(overhead));

    SkAutoTMalloc<double> samples(FLAGS_samples);

    if (FLAGS_runOnce) {
        SkDebugf("--runOnce is true; times would only be misleading so we won't print them.\n");
    } else if (FLAGS_verbose) {
        // No header.
    } else if (FLAGS_quiet) {
        SkDebugf("median\tbench\tconfig\n");
    } else {
        SkDebugf("loops\tmin\tmedian\tmean\tmax\tstddev\tsamples\tconfig\tbench\n");
    }

    SkTDArray<Config> configs;
    create_configs(&configs);

    BenchmarkStream benchStream;
    while (Benchmark* b = benchStream.next()) {
        SkAutoTDelete<Benchmark> bench(b);
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, bench->getName())) {
            continue;
        }

        SkTDArray<Target*> targets;
        create_targets(&targets, bench.get(), configs);

        if (!targets.isEmpty()) {
            log.bench(bench->getName(), bench->getSize().fX, bench->getSize().fY);
            bench->preDraw();
        }
        for (int j = 0; j < targets.count(); j++) {
            SkCanvas* canvas = targets[j]->surface.get() ? targets[j]->surface->getCanvas() : NULL;
            const char* config = targets[j]->config.name;

#if SK_DEBUG
            // skia:2797  Some SKPs SkASSERT in debug mode.  Skip them for now.
            if (0 == strcmp("565", config) && SkStrContains(bench->getName(), ".skp")) {
                SkDebugf("Skipping 565 %s.  See skia:2797\n", bench->getName());
                continue;
            }
#endif

            const int loops =
#if SK_SUPPORT_GPU
                Benchmark::kGPU_Backend == targets[j]->config.backend
                ? gpu_bench(targets[j]->gl, bench.get(), canvas, samples.get())
                :
#endif
                 cpu_bench(       overhead, bench.get(), canvas, samples.get());

            if (loops == 0) {
                // Can't be timed.  A warning note has already been printed.
                continue;
            }

            Stats stats(samples.get(), FLAGS_samples);
            log.config(config);
            benchStream.fillCurrentOptions(&log);
#if SK_SUPPORT_GPU
            if (Benchmark::kGPU_Backend == targets[j]->config.backend) {
                fill_gpu_options(&log, targets[j]->gl);
            }
#endif
            log.timer("min_ms",    stats.min);
            log.timer("median_ms", stats.median);
            log.timer("mean_ms",   stats.mean);
            log.timer("max_ms",    stats.max);
            log.timer("stddev_ms", sqrt(stats.var));

            if (FLAGS_runOnce) {
                if (targets.count() == 1) {
                    config = ""; // Only print the config if we run the same bench on more than one.
                }
                SkDebugf("%s\t%s\n", bench->getName(), config);
            } else if (FLAGS_verbose) {
                for (int i = 0; i < FLAGS_samples; i++) {
                    SkDebugf("%s  ", HUMANIZE(samples[i]));
                }
                SkDebugf("%s\n", bench->getName());
            } else if (FLAGS_quiet) {
                if (targets.count() == 1) {
                    config = ""; // Only print the config if we run the same bench on more than one.
                }
                SkDebugf("%s\t%s\t%s\n", HUMANIZE(stats.median), bench->getName(), config);
            } else {
                const double stddev_percent = 100 * sqrt(stats.var) / stats.mean;
                SkDebugf("%d\t%s\t%s\t%s\t%s\t%.0f%%\t%s\t%s\t%s\n"
                        , loops
                        , HUMANIZE(stats.min)
                        , HUMANIZE(stats.median)
                        , HUMANIZE(stats.mean)
                        , HUMANIZE(stats.max)
                        , stddev_percent
                        , stats.plot.c_str()
                        , config
                        , bench->getName()
                        );
            }
        }
        targets.deleteAll();

    #if SK_SUPPORT_GPU
        if (FLAGS_abandonGpuContext) {
            gGrFactory.abandonContexts();
        }
        if (FLAGS_resetGpuContext || FLAGS_abandonGpuContext) {
            gGrFactory.destroyContexts();
        }
    #endif
    }

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return nanobench_main();
}
#endif
