// DO NOT MODIFY! This file is autogenerated by gn_to_bp.py.
// If need to change a define, modify SkUserConfigManual.h
#pragma once
#include "SkUserConfigManual.h"
#define NOMINMAX
#define SK_ALLOW_STATIC_GLOBAL_INITIALIZERS 0
#define SK_ENABLE_DISCRETE_GPU
#define SK_GAMMA_APPLY_TO_A8
#define SK_GAMMA_CONTRAST 0.0
#define SK_GAMMA_EXPONENT 1.4
#define SK_HAS_JPEG_LIBRARY
#define SK_HAS_PNG_LIBRARY
#define SK_HAS_WEBP_LIBRARY
#define SK_SUPPORT_GPU 0
#define SK_SUPPORT_PDF
#define SK_XML
#define _CRT_SECURE_NO_WARNINGS
#define _HAS_EXCEPTIONS 0

// Correct SK_BUILD_FOR flags that may have been set by
// SkPreConfig.h/Android.bp
#ifndef SK_BUILD_FOR_WIN
    #define SK_BUILD_FOR_WIN
#endif
#ifdef SK_BUILD_FOR_ANDROID
    #undef SK_BUILD_FOR_ANDROID
#endif
#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_IOS) || \
    defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)
    #error "Only SK_BUILD_FOR_WIN should be defined!"
#endif
