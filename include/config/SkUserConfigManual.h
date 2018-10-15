/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUserConfigManual_DEFINED
#define SkUserConfigManual_DEFINED
  #define GR_GL_CUSTOM_SETUP_HEADER "gl/GrGLConfig_chrome.h"
  #define GR_TEST_UTILS 1
  #define SK_BUILD_FOR_ANDROID_FRAMEWORK
  #define SK_DEFAULT_FONT_CACHE_LIMIT   (768 * 1024)
  #define SK_DEFAULT_GLOBAL_DISCARDABLE_MEMORY_POOL_SIZE (512 * 1024)
  #define SK_USE_FREETYPE_EMBOLDEN

  // Disable these Ganesh features
  #define SK_DISABLE_EXPLICIT_GPU_RESOURCE_ALLOCATION
  #define SK_DISABLE_RENDER_TARGET_SORTING

  // Legacy flags
  #define SK_IGNORE_GPU_DITHER
  #define SK_SUPPORT_DEPRECATED_CLIPOPS
  // Needed until we fix https://bug.skia.org/2440
  #define SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
  #define SK_SUPPORT_LEGACY_EMBOSSMASKFILTER
  #define SK_SUPPORT_LEGACY_GRADIENT_DITHERING
  #define SK_SUPPORT_LEGACY_TILED_BITMAPS
  #define SK_SUPPORT_LEGACY_AA_CHOICE
  #define SK_SUPPORT_LEGACY_A8_MASKBLITTER

  // While working on updating skias vulkan interface
  #define SK_SUPPORT_LEGACY_VULKAN_INTERFACE
  #define SK_SUPPORT_LEGACY_AAA_CHOICE
#endif // SkUserConfigManual_DEFINED
