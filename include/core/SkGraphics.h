
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkGraphics_DEFINED
#define SkGraphics_DEFINED

#include "SkTypes.h"

class SK_API SkGraphics {
public:
    /**
     *  Call this at process initialization time if your environment does not
     *  permit static global initializers that execute code. Note that
     *  Init() is not thread-safe.
     */
    static void Init();

    /**
     *  Call this to release any memory held privately, such as the font cache.
     */
    static void Term();

    /**
     *  Return the version numbers for the library. If the parameter is not
     *  null, it is set to the version number.
     */
    static void GetVersion(int32_t* major, int32_t* minor, int32_t* patch);

    /**
     *  Return the max number of bytes that should be used by the font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *  This max can be changed by calling SetFontCacheLimit().
     */
    static size_t GetFontCacheLimit();

    /**
     *  Specify the max number of bytes that should be used by the font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *
     *  This function returns the previous setting, as if GetFontCacheLimit()
     *  had be called before the new limit was set.
     */
    static size_t SetFontCacheLimit(size_t bytes);

    /**
     *  Return the number of bytes currently used by the font cache.
     */
    static size_t GetFontCacheUsed();

    /**
     *  Return the number of entries in the font cache.
     *  A cache "entry" is associated with each typeface + pointSize + matrix.
     */
    static int GetFontCacheCountUsed();

    /**
     *  Return the current limit to the number of entries in the font cache.
     *  A cache "entry" is associated with each typeface + pointSize + matrix.
     */
    static int GetFontCacheCountLimit();

    /**
     *  Set the limit to the number of entries in the font cache, and return
     *  the previous value. If this new value is lower than the previous,
     *  it will automatically try to purge entries to meet the new limit.
     */
    static int SetFontCacheCountLimit(int count);

    /**
     *  For debugging purposes, this will attempt to purge the font cache. It
     *  does not change the limit, but will cause subsequent font measures and
     *  draws to be recreated, since they will no longer be in the cache.
     */
    static void PurgeFontCache();

    /**
     *  Scaling bitmaps with the SkPaint::kHigh_FilterLevel setting is
     *  expensive, so the result is saved in the global Scaled Image
     *  Cache.
     *
     *  This function returns the memory usage of the Scaled Image Cache.
     */
    static size_t GetImageCacheTotalBytesUsed();
    /**
     *  These functions get/set the memory usage limit for the Scaled
     *  Image Cache.  Bitmaps are purged from the cache when the
     *  memory useage exceeds this limit.
     */
    static size_t GetImageCacheTotalByteLimit();
    static size_t SetImageCacheTotalByteLimit(size_t newLimit);

    // DEPRECATED
    static size_t GetImageCacheBytesUsed() {
        return GetImageCacheTotalBytesUsed();
    }
    // DEPRECATED
    static size_t GetImageCacheByteLimit() {
        return GetImageCacheTotalByteLimit();
    }
    // DEPRECATED
    static size_t SetImageCacheByteLimit(size_t newLimit) {
        return SetImageCacheTotalByteLimit(newLimit);
    }

    /**
     *  Scaling bitmaps with the SkPaint::kHigh_FilterLevel setting is
     *  expensive, so the result is saved in the global Scaled Image
     *  Cache.  When the resulting bitmap is too large, this can
     *  overload the cache.  If the ImageCacheSingleAllocationByteLimit
     *  is set to a non-zero number, and the resulting bitmap would be
     *  larger than that value, the bitmap scaling algorithm falls
     *  back onto a cheaper algorithm and does not cache the result.
     *  Zero is the default value.
     */
    static size_t GetImageCacheSingleAllocationByteLimit();
    static size_t SetImageCacheSingleAllocationByteLimit(size_t newLimit);

    /**
     *  Applications with command line options may pass optional state, such
     *  as cache sizes, here, for instance:
     *  font-cache-limit=12345678
     *
     *  The flags format is name=value[;name=value...] with no spaces.
     *  This format is subject to change.
     */
    static void SetFlags(const char* flags);

    /**
     *  Return the max number of bytes that should be used by the thread-local
     *  font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *  This max can be changed by calling SetFontCacheLimit().
     *
     *  If this thread has never called SetTLSFontCacheLimit, or has called it
     *  with 0, then this thread is using the shared font cache. In that case,
     *  this function will always return 0, and the caller may want to call
     *  GetFontCacheLimit.
     */
    static size_t GetTLSFontCacheLimit();

    /**
     *  Specify the max number of bytes that should be used by the thread-local
     *  font cache. If this value is 0, then this thread will use the shared
     *  global font cache.
     */
    static void SetTLSFontCacheLimit(size_t bytes);

private:
    /** This is automatically called by SkGraphics::Init(), and must be
        implemented by the host OS. This allows the host OS to register a callback
        with the C++ runtime to call SkGraphics::FreeCaches()
    */
    static void InstallNewHandler();
};

class SkAutoGraphics {
public:
    SkAutoGraphics() {
        SkGraphics::Init();
    }
    ~SkAutoGraphics() {
        SkGraphics::Term();
    }
};

#endif
