/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Classes for writing out bench results in various formats.
 */
#ifndef SkResultsWriter_DEFINED
#define SkResultsWriter_DEFINED

#include "SkBenchLogger.h"
#include "SkJSONCPP.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTypes.h"


/**
 * Base class for writing out the bench results.
 *
 * TODO(jcgregorio) Add info if tests fail to converge?
 */
class ResultsWriter : SkNoncopyable {
public:
    virtual ~ResultsWriter() {};

    // Records one option set for this run. All options must be set before
    // calling bench().
    virtual void option(const char name[], const char value[]) = 0;

    // Denotes the start of a specific benchmark. Once bench is called,
    // then config and timer can be called multiple times to record runs.
    virtual void bench(const char name[], int32_t x, int32_t y) = 0;

    // Records the specific configuration a bench is run under, such as "8888".
    virtual void config(const char name[]) = 0;

    // Records a single test metric.
    virtual void timer(const char name[], double ms) = 0;

    // Call when all results are finished.
    virtual void end() = 0;
};

/**
 * This ResultsWriter handles writing out the human readable format of the
 * bench results.
 */
class LoggerResultsWriter : public ResultsWriter {
public:
    explicit LoggerResultsWriter(SkBenchLogger& logger, const char* timeFormat)
        : fLogger(logger)
        , fTimeFormat(timeFormat) {
        fLogger.logProgress("skia bench:");
    }
    virtual void option(const char name[], const char value[]) {
        fLogger.logProgress(SkStringPrintf(" %s=%s", name, value));
    }
    virtual void bench(const char name[], int32_t x, int32_t y) {
        fLogger.logProgress(SkStringPrintf(
            "\nrunning bench [%3d %3d] %40s", x, y, name));
    }
    virtual void config(const char name[]) {
        fLogger.logProgress(SkStringPrintf("   %s:", name));
    }
    virtual void timer(const char name[], double ms) {
        fLogger.logProgress(SkStringPrintf("  %s = ", name));
        fLogger.logProgress(SkStringPrintf(fTimeFormat, ms));
    }
    virtual void end() {
        fLogger.logProgress("\n");
    }
private:
    SkBenchLogger& fLogger;
    const char* fTimeFormat;
};

#ifdef SK_BUILD_JSON_WRITER
/**
 * This ResultsWriter handles writing out the results in JSON.
 *
 * The output looks like:
 *
 *  {
 *   "options" : {
 *      "alpha" : "0xFF",
 *      "scale" : "0",
 *      ...
 *      "system" : "UNIX"
 *   },
 *   "results" : {
 *      "Xfermode_Luminosity_640_480" : {
 *         "565" : {
 *            "cmsecs" : 143.188128906250,
 *            "msecs" : 143.835957031250
 *         },
 *         ...
 */
class JSONResultsWriter : public ResultsWriter {
private:
    Json::Value* find_named_node(Json::Value* root, const char name[]) {
        Json::Value* search_results = NULL;
        for(Json::Value::iterator iter = root->begin();
                iter!= root->end(); ++iter) {
            if(SkString(name).equals((*iter)["name"].asCString())) {
                search_results = &(*iter);
                break;
            }
        }

        if(search_results != NULL) {
            return search_results;
        } else {
            return &(root->append(Json::Value()));
        }
    }
public:
    explicit JSONResultsWriter(const char filename[])
        : fFilename(filename)
        , fRoot()
        , fResults(fRoot["results"])
        , fBench(NULL)
        , fConfig(NULL) {
    }
    virtual void option(const char name[], const char value[]) {
        fRoot["options"][name] = value;
    }
    virtual void bench(const char name[], int32_t x, int32_t y) {
        const char* full_name = SkStringPrintf( "%s_%d_%d", name, x, y).c_str();
        Json::Value* bench_node = find_named_node(&fResults, full_name);
        (*bench_node)["name"] = full_name;
        fBench = &(*bench_node)["results"];
    }
    virtual void config(const char name[]) {
        SkASSERT(NULL != fBench);
        fConfig = find_named_node(fBench, name);
        (*fConfig)["name"] = name;
    }
    virtual void timer(const char name[], double ms) {
        SkASSERT(NULL != fConfig);
        (*fConfig)[name] = ms;
    }
    virtual void end() {
        SkFILEWStream stream(fFilename.c_str());
        stream.writeText(fRoot.toStyledString().c_str());
        stream.flush();
    }
private:
    SkString fFilename;
    Json::Value fRoot;
    Json::Value& fResults;
    Json::Value* fBench;
    Json::Value* fConfig;
};

#endif // SK_BUILD_JSON_WRITER
/**
 * This ResultsWriter writes out to multiple ResultsWriters.
 */
class MultiResultsWriter : public ResultsWriter {
public:
    MultiResultsWriter() : writers() {
    };
    void add(ResultsWriter* writer) {
      writers.push_back(writer);
    }
    virtual void option(const char name[], const char value[]) {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->option(name, value);
        }
    }
    virtual void bench(const char name[], int32_t x, int32_t y) {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->bench(name, x, y);
        }
    }
    virtual void config(const char name[]) {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->config(name);
        }
    }
    virtual void timer(const char name[], double ms) {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->timer(name, ms);
        }
    }
    virtual void end() {
        for (int i = 0; i < writers.count(); ++i) {
            writers[i]->end();
        }
    }
private:
    SkTArray<ResultsWriter *> writers;
};

/**
 * Calls the end() method of T on destruction.
 */
template <typename T> class CallEnd : SkNoncopyable {
public:
    CallEnd(T& obj) : fObj(obj) {}
    ~CallEnd() { fObj.end(); }
private:
    T&  fObj;
};

#endif
