/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLExtensions_DEFINED
#define GrGLExtensions_DEFINED

#include "GrGLFunctions.h"
#include "SkString.h"
#include "SkTArray.h"

struct GrGLInterface;

/**
 * This helper queries the current GL context for its extensions, remembers them, and can be
 * queried. It supports both glGetString- and glGetStringi-style extension string APIs and will
 * use the latter if it is available.
 */
class GrGLExtensions : public SkNoncopyable {
public:
    GrGLExtensions() : fInitialized(false), fStrings(SkNEW(SkTArray<SkString>)) {}

    void swap(GrGLExtensions* that) {
        fStrings.swap(&that->fStrings);
    }

    /**
     * We sometimes need to use this class without having yet created a GrGLInterface. This version
     * of init expects that getString is always non-NULL while getIntegerv and getStringi are non-
     * NULL if on desktop GL with version 3.0 or higher. Otherwise it will fail.
     */
    bool init(GrGLStandard standard,
              GrGLGetStringProc getString,
              GrGLGetStringiProc getStringi,
              GrGLGetIntegervProc getIntegerv);

    bool isInitialized() const { return fInitialized; }

    /**
     * Queries whether an extension is present. This will fail if init() has not been called.
     */
    bool has(const char*) const;

    void reset() { fStrings->reset(); }

    void print(const char* sep = "\n") const;

private:
    bool                                fInitialized;
    SkAutoTDelete<SkTArray<SkString> >  fStrings;
};

#endif
