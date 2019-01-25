/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMJsonWriter.h"

#include "ProcStats.h"
#include "SkCommonFlags.h"
#include "SkData.h"
#include "SkJSONCPP.h"
#include "SkJSONWriter.h"
#include "SkMutex.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkStream.h"
#include "SkTArray.h"

namespace DM {

SkTArray<JsonWriter::BitmapResult> gBitmapResults;
SK_DECLARE_STATIC_MUTEX(gBitmapResultLock);

void JsonWriter::AddBitmapResult(const BitmapResult& result) {
    SkAutoMutexAcquire lock(&gBitmapResultLock);
    gBitmapResults.push_back(result);
}

SkTArray<skiatest::Failure> gFailures;
SK_DECLARE_STATIC_MUTEX(gFailureLock);

void JsonWriter::AddTestFailure(const skiatest::Failure& failure) {
    SkAutoMutexAcquire lock(gFailureLock);
    gFailures.push_back(failure);
}

void JsonWriter::DumpJson() {
    if (FLAGS_writePath.isEmpty()) {
        return;
    }

    SkString path = SkOSPath::Join(FLAGS_writePath[0], "dm.json");
    sk_mkdir(FLAGS_writePath[0]);
    SkFILEWStream stream(path.c_str());
    SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);

    writer.beginObject(); // root

    for (int i = 1; i < FLAGS_properties.count(); i += 2) {
        writer.appendString(FLAGS_properties[i-1], FLAGS_properties[i]);
    }

    writer.beginObject("key");
    for (int i = 1; i < FLAGS_key.count(); i += 2) {
        writer.appendString(FLAGS_key[i-1], FLAGS_key[i]);
    }
    writer.endObject();

    int maxResidentSetSizeMB = sk_tools::getMaxResidentSetSizeMB();
    if (maxResidentSetSizeMB != -1) {
        writer.appendS32("max_rss_MB", maxResidentSetSizeMB);
    }

    {
        SkAutoMutexAcquire lock(&gBitmapResultLock);
        writer.beginArray("results");
        for (int i = 0; i < gBitmapResults.count(); i++) {
            writer.beginObject();

            writer.beginObject("key");
            writer.appendString("name"       , gBitmapResults[i].name.c_str());
            writer.appendString("config"     , gBitmapResults[i].config.c_str());
            writer.appendString("source_type", gBitmapResults[i].sourceType.c_str());

            // Source options only need to be part of the key if they exist.
            // Source type by source type, we either always set options or never set options.
            if (!gBitmapResults[i].sourceOptions.isEmpty()) {
                writer.appendString("source_options", gBitmapResults[i].sourceOptions.c_str());
            }
            writer.endObject(); // key

            writer.beginObject("options");
            writer.appendString("ext"          , gBitmapResults[i].ext.c_str());
            writer.appendString("gamma_correct", gBitmapResults[i].gammaCorrect ? "yes" : "no");
            writer.endObject(); // options

            writer.appendString("md5", gBitmapResults[i].md5.c_str());

            writer.endObject(); // 1 result
        }
        writer.endArray(); // results
    }

    {
        SkAutoMutexAcquire lock(gFailureLock);
        if (gFailures.count() > 0) {
            writer.beginObject("test_results");
            writer.beginArray("failures");
            for (int i = 0; i < gFailures.count(); i++) {
                writer.beginObject();
                writer.appendString("file_name", gFailures[i].fileName);
                writer.appendS32   ("line_no"  , gFailures[i].lineNo);
                writer.appendString("condition", gFailures[i].condition);
                writer.appendString("message"  , gFailures[i].message.c_str());
                writer.endObject(); // 1 failure
            }
            writer.endArray(); // failures
            writer.endObject(); // test_results
        }
    }

    writer.endObject(); // root
    writer.flush();
    stream.flush();
}

bool JsonWriter::ReadJson(const char* path, void(*callback)(BitmapResult)) {
    sk_sp<SkData> json(SkData::MakeFromFileName(path));
    if (!json) {
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    const char* data = (const char*)json->data();
    if (!reader.parse(data, data+json->size(), root)) {
        return false;
    }

    const Json::Value& results = root["results"];
    BitmapResult br;
    for (unsigned i = 0; i < results.size(); i++) {
        const Json::Value& r = results[i];
        br.name         = r["key"]["name"].asCString();
        br.config       = r["key"]["config"].asCString();
        br.sourceType   = r["key"]["source_type"].asCString();
        br.ext          = r["options"]["ext"].asCString();
        br.gammaCorrect = 0 == strcmp("yes", r["options"]["gamma_correct"].asCString());
        br.md5          = r["md5"].asCString();

        if (!r["key"]["source_options"].isNull()) {
            br.sourceOptions = r["key"]["source_options"].asCString();
        }
        callback(br);
    }
    return true;
}

} // namespace DM
