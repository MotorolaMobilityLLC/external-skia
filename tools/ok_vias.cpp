/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ProcStats.h"
#include "SkEventTracingPriv.h"
#include "SkImage.h"
#include "SkOSFile.h"
#include "SkPictureRecorder.h"
#include "SkPngEncoder.h"
#include "SkTraceEvent.h"
#include "SkTypeface.h"
#include "Timer.h"
#include "ok.h"
#include "sk_tool_utils.h"
#include <chrono>
#include <regex>

static std::unique_ptr<Src> proxy(Src* original, std::function<Status(SkCanvas*)> fn) {
    struct : Src {
        Src*                             original;
        std::function<Status(SkCanvas*)> fn;

        std::string name() override { return original->name(); }
        SkISize     size() override { return original->size(); }
        Status draw(SkCanvas* canvas) override { return fn(canvas); }
    } src;
    src.original = original;
    src.fn       = fn;
    return move_unique(src);
}

struct ViaPic : Dst {
    std::unique_ptr<Dst> target;
    bool                 rtree = false;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        ViaPic via;
        via.target = std::move(dst);
        if (options("bbh") == "rtree") { via.rtree = true; }
        return move_unique(via);
    }

    Status draw(Src* src) override {
        TRACE_EVENT0("ok", TRACE_FUNC);
        SkRTreeFactory factory;
        SkPictureRecorder rec;
        rec.beginRecording(SkRect::MakeSize(SkSize::Make(src->size())),
                           rtree ? &factory : nullptr);

        for (Status status = src->draw(rec.getRecordingCanvas()); status != Status::OK; ) {
            return status;
        }
        sk_sp<SkPicture> pic = rec.finishRecordingAsPicture();

        return target->draw(proxy(src, [=](SkCanvas* canvas) {
            pic->playback(canvas);
            return Status::OK;
        }).get());
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register via_pic{"via_pic", "record then play back an SkPicture", ViaPic::Create};

// When deserializing, we need to hook this to intercept "Toy Liberation ..."
// typefaces and return our portable test typeface.
extern sk_sp<SkTypeface> (*gCreateTypefaceDelegate)(const char[], SkFontStyle);

struct ViaSkp : Dst {
    std::unique_ptr<Dst> target;
    bool                 rtree = false;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        gCreateTypefaceDelegate = [](const char name[], SkFontStyle style) -> sk_sp<SkTypeface> {
            if (name == strstr(name, "Toy Liberation")) {
                return sk_tool_utils::create_portable_typeface(name, style);
            }
            return nullptr;
        };

        ViaSkp via;
        via.target = std::move(dst);
        if (options("bbh") == "rtree") { via.rtree = true; }
        return move_unique(via);
    }

    Status draw(Src* src) override {
        TRACE_EVENT0("ok", TRACE_FUNC);
        SkRTreeFactory factory;
        SkPictureRecorder rec;
        rec.beginRecording(SkRect::MakeSize(SkSize::Make(src->size())),
                           rtree ? &factory : nullptr);

        for (Status status = src->draw(rec.getRecordingCanvas()); status != Status::OK; ) {
            return status;
        }
        sk_sp<SkPicture> pic = rec.finishRecordingAsPicture();

        // Serialize and deserialize.
        pic = SkPicture::MakeFromData(pic->serialize().get());

        return target->draw(proxy(src, [=](SkCanvas* canvas) {
            pic->playback(canvas);
            return Status::OK;
        }).get());
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register via_skp{"via_skp", "serialize and deserialize an .skp", ViaSkp::Create};

struct Png : Dst {
    std::unique_ptr<Dst> target;
    std::string          dir;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        Png via;
        via.target = std::move(dst);
        via.dir    = options("dir", "ok");
        return move_unique(via);
    }

    Status draw(Src* src) override {
        TRACE_EVENT0("ok", TRACE_FUNC);
        for (auto status = target->draw(src); status != Status::OK; ) {
            return status;
        }

        SkBitmap bm;
        SkPixmap pm;
        if (!target->image()->asLegacyBitmap(&bm, SkImage::kRO_LegacyBitmapMode) ||
            !bm.peekPixels(&pm)) {
            return Status::Failed;
        }

        sk_mkdir(dir.c_str());
        SkFILEWStream dst{(dir + "/" + src->name() + ".png").c_str()};

        SkPngEncoder::Options options;
        options.fFilterFlags      = SkPngEncoder::FilterFlag::kNone;
        options.fZLibLevel        = 1;
        options.fUnpremulBehavior = pm.colorSpace() ? SkTransferFunctionBehavior::kRespect
                                                    : SkTransferFunctionBehavior::kIgnore;
        return SkPngEncoder::Encode(&dst, pm, options) ? Status::OK
                                                       : Status::Failed;
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register png{"png", "dump PNGs to dir=ok" , Png::Create};

struct Filter : Dst {
    std::unique_ptr<Dst> target;
    std::regex match, search;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        Filter via;
        via.target = std::move(dst);
        via.match  = options("match",  ".*");
        via.search = options("search", ".*");
        return move_unique(via);
    }

    Status draw(Src* src) override {
        auto name = src->name();
        if (!std::regex_match (name, match) ||
            !std::regex_search(name, search)) {
            return Status::Skipped;
        }
        return target->draw(src);
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register filter{"filter",
                       "run only srcs matching match=.* exactly and search=.* somewhere",
                       Filter::Create};

struct Time : Dst {
    std::unique_ptr<Dst> target;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        Time via;
        via.target = std::move(dst);
        return move_unique(via);
    }

    Status draw(Src* src) override {
        auto start = std::chrono::steady_clock::now();
            Status status = target->draw(src);
        std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now()
                                                          - start;

        if (status != Status::Skipped) {
            auto msg = HumanizeMs(elapsed.count());
            ok_log(msg.c_str());
        }
        return status;
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register _time{"time", "print wall run time", Time::Create};

struct Memory : Dst {
    std::unique_ptr<Dst> target;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        Memory via;
        via.target = std::move(dst);
        return move_unique(via);
    }

    Status draw(Src* src) override {
        Status status = target->draw(src);

        if (status != Status::Skipped) {
            auto msg = SkStringPrintf("%dMB", sk_tools::getMaxResidentSetSizeMB());
            ok_log(msg.c_str());
        }

        return status;
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register memory{"memory", "print process maximum memory usage", Memory::Create};

static SkOnce init_tracing_once;
struct Trace : Dst {
    std::unique_ptr<Dst> target;
    std::string trace_mode;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        Trace via;
        via.target = std::move(dst);
        via.trace_mode = options("mode", "trace.json");
        return move_unique(via);
    }

    Status draw(Src* src) override {
        init_tracing_once([&] { initializeEventTracingForTools(trace_mode.c_str()); });
        return target->draw(src);
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register trace {"trace",
                       "enable tracing in mode=atrace, mode=debugf, or mode=trace.json",
                       Trace::Create};
