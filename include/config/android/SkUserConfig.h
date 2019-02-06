// DO NOT MODIFY! This file is autogenerated by gn_to_bp.py.
// If need to change a define, modify SkUserConfigManual.h
#pragma once
#include "SkUserConfigManual.h"
#define SK_ALLOW_STATIC_GLOBAL_INITIALIZERS 0
#define SK_CODEC_DECODES_RAW
#define SK_ENABLE_DISCRETE_GPU
#define SK_GAMMA_APPLY_TO_A8
#define SK_GAMMA_CONTRAST 0.0
#define SK_GAMMA_EXPONENT 1.4
#define SK_HAS_HEIF_LIBRARY
#define SK_HAS_JPEG_LIBRARY
#define SK_HAS_PNG_LIBRARY
#define SK_HAS_WEBP_LIBRARY
#define SK_SUPPORT_PDF
#define SK_VULKAN
#define SK_XML

#ifndef SK_BUILD_FOR_ANDROID
    #error "SK_BUILD_FOR_ANDROID must be defined!"
#endif
#if defined(SK_BUILD_FOR_IOS) || defined(SK_BUILD_FOR_MAC) || \
    defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN)
    #error "Only SK_BUILD_FOR_ANDROID should be defined!"
#endif
