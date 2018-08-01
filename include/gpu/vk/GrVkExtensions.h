/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkExtensions_DEFINED
#define GrVkExtensions_DEFINED

#include "../../private/SkTArray.h"
#include "SkString.h"
#include "vk/GrVkTypes.h"

/**
 * Helper class that eats in an array of extensions strings for instance and device and allows for
 * quicker querying if an extension is present.
 */
class SK_API GrVkExtensions {
public:
    GrVkExtensions() {}

    void init(GrVkGetProc, VkInstance, VkPhysicalDevice,
              uint32_t instanceExtensionCount, const char* const* instanceExtensions,
              uint32_t deviceExtensionCount, const char* const* deviceExtensions);

    bool hasExtension(const char[], uint32_t minVersion) const;

    struct Info {
        Info() {}
        Info(const char* name) : fName(name), fSpecVersion(0) {}

        SkString fName;
        uint32_t fSpecVersion;

        struct Less {
            bool operator() (const Info& a, const SkString& b) {
                return strcmp(a.fName.c_str(), b.c_str()) < 0;
            }
            bool operator() (const SkString& a, const GrVkExtensions::Info& b) {
                return strcmp(a.c_str(), b.fName.c_str()) < 0;
            }
        };
    };

private:
    void getSpecVersions(GrVkGetProc getProc, VkInstance, VkPhysicalDevice);

    SkTArray<Info>  fExtensions;
};

#endif
