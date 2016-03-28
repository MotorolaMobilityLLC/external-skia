/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkCaps.h"

#include "GrVkUtil.h"
#include "glsl/GrGLSLCaps.h"
#include "vk/GrVkInterface.h"
#include "vk/GrVkBackendContext.h"

GrVkCaps::GrVkCaps(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
                   VkPhysicalDevice physDev, uint32_t featureFlags, uint32_t extensionFlags)
    : INHERITED(contextOptions) {
    fCanUseGLSLForShaderModule = false;

    /**************************************************************************
    * GrDrawTargetCaps fields
    **************************************************************************/
    fMipMapSupport = false; //TODO: figure this out
    fNPOTTextureTileSupport = false; //TODO: figure this out
    fTwoSidedStencilSupport = false; //TODO: figure this out
    fStencilWrapOpsSupport = false; //TODO: figure this out
    fDiscardRenderTargetSupport = false; //TODO: figure this out
    fReuseScratchTextures = true; //TODO: figure this out
    fGpuTracingSupport = false; //TODO: figure this out
    fCompressedTexSubImageSupport = false; //TODO: figure this out
    fOversizedStencilSupport = false; //TODO: figure this out

    fUseDrawInsteadOfClear = false; //TODO: figure this out

    fMapBufferFlags = kNone_MapFlags; //TODO: figure this out
    fBufferMapThreshold = SK_MaxS32;  //TODO: figure this out

    fMaxRenderTargetSize = 4096; // minimum required by spec
    fMaxTextureSize = 4096; // minimum required by spec
    fMaxColorSampleCount = 4; // minimum required by spec
    fMaxStencilSampleCount = 4; // minimum required by spec


    fShaderCaps.reset(new GrGLSLCaps(contextOptions));

    this->init(contextOptions, vkInterface, physDev, featureFlags, extensionFlags);
}

void GrVkCaps::init(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
                    VkPhysicalDevice physDev, uint32_t featureFlags, uint32_t extensionFlags) {

    VkPhysicalDeviceProperties properties;
    GR_VK_CALL(vkInterface, GetPhysicalDeviceProperties(physDev, &properties));

    VkPhysicalDeviceMemoryProperties memoryProperties;
    GR_VK_CALL(vkInterface, GetPhysicalDeviceMemoryProperties(physDev, &memoryProperties));

    this->initGrCaps(properties, memoryProperties, featureFlags);
    this->initGLSLCaps(properties, featureFlags);
    this->initConfigTexturableTable(vkInterface, physDev);
    this->initConfigRenderableTable(vkInterface, physDev);
    this->initStencilFormats(vkInterface, physDev);

    if (SkToBool(extensionFlags & kNV_glsl_shader_GrVkExtensionFlag)) {
        fCanUseGLSLForShaderModule = true;
    }

    this->applyOptionsOverrides(contextOptions);
    GrGLSLCaps* glslCaps = static_cast<GrGLSLCaps*>(fShaderCaps.get());
    glslCaps->applyOptionsOverrides(contextOptions);
}

int get_max_sample_count(VkSampleCountFlags flags) {
    SkASSERT(flags & VK_SAMPLE_COUNT_1_BIT);
    if (!(flags & VK_SAMPLE_COUNT_2_BIT)) {
        return 0;
    }
    if (!(flags & VK_SAMPLE_COUNT_4_BIT)) {
        return 2;
    }
    if (!(flags & VK_SAMPLE_COUNT_8_BIT)) {
        return 4;
    }
    if (!(flags & VK_SAMPLE_COUNT_16_BIT)) {
        return 8;
    }
    if (!(flags & VK_SAMPLE_COUNT_32_BIT)) {
        return 16;
    }
    if (!(flags & VK_SAMPLE_COUNT_64_BIT)) {
        return 32;
    }
    return 64;
}

void GrVkCaps::initSampleCount(const VkPhysicalDeviceProperties& properties) {
    VkSampleCountFlags colorSamples = properties.limits.framebufferColorSampleCounts;
    VkSampleCountFlags stencilSamples = properties.limits.framebufferStencilSampleCounts;

    fMaxColorSampleCount = get_max_sample_count(colorSamples);
    fMaxStencilSampleCount = get_max_sample_count(stencilSamples);
}

void GrVkCaps::initGrCaps(const VkPhysicalDeviceProperties& properties,
                          const VkPhysicalDeviceMemoryProperties& memoryProperties,
                          uint32_t featureFlags) {
    fMaxVertexAttributes = properties.limits.maxVertexInputAttributes;
    // We could actually query and get a max size for each config, however maxImageDimension2D will
    // give the minimum max size across all configs. So for simplicity we will use that for now.
    fMaxRenderTargetSize = properties.limits.maxImageDimension2D;
    fMaxTextureSize = properties.limits.maxImageDimension2D;

    this->initSampleCount(properties);

    // Assuming since we will always map in the end to upload the data we might as well just map
    // from the get go. There is no hard data to suggest this is faster or slower.
    fBufferMapThreshold = 0;

    fMapBufferFlags = kCanMap_MapFlag | kSubset_MapFlag;

    fStencilWrapOpsSupport = true;
    fOversizedStencilSupport = true;
    fSampleShadingSupport = SkToBool(featureFlags & kSampleRateShading_GrVkFeatureFlag);
}

void GrVkCaps::initGLSLCaps(const VkPhysicalDeviceProperties& properties,
                            uint32_t featureFlags) {
    GrGLSLCaps* glslCaps = static_cast<GrGLSLCaps*>(fShaderCaps.get());
    glslCaps->fVersionDeclString = "#version 310 es\n";

    // fConfigOutputSwizzle will default to RGBA so we only need to set it for alpha only config.
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        if (GrPixelConfigIsAlphaOnly(config)) {
            glslCaps->fConfigTextureSwizzle[i] = GrSwizzle::RRRR();
            glslCaps->fConfigOutputSwizzle[i] = GrSwizzle::AAAA();
        } else {
            glslCaps->fConfigTextureSwizzle[i] = GrSwizzle::RGBA();
        }
    }

    // Vulkan is based off ES 3.0 so the following should all be supported
    glslCaps->fUsesPrecisionModifiers = true;
    glslCaps->fFlatInterpolationSupport = true;

    // GrShaderCaps

    glslCaps->fShaderDerivativeSupport = true;
    glslCaps->fGeometryShaderSupport = SkToBool(featureFlags & kGeometryShader_GrVkFeatureFlag);
#if 0
    // For now disabling dual source blending till we get it hooked up in the rest of system
    glslCaps->fDualSourceBlendingSupport = SkToBool(featureFlags & kDualSrcBlend_GrVkFeatureFlag);
#endif
    glslCaps->fIntegerSupport = true;

    glslCaps->fMaxVertexSamplers =
    glslCaps->fMaxGeometrySamplers =
    glslCaps->fMaxFragmentSamplers = SkTMin(properties.limits.maxPerStageDescriptorSampledImages,
                                            properties.limits.maxPerStageDescriptorSamplers);
    glslCaps->fMaxCombinedSamplers = SkTMin(properties.limits.maxDescriptorSetSampledImages,
                                            properties.limits.maxDescriptorSetSamplers);
}

static void format_supported_for_feature(const GrVkInterface* interface,
                                         VkPhysicalDevice physDev,
                                         VkFormat format,
                                         VkFormatFeatureFlagBits featureBit,
                                         bool* linearSupport,
                                         bool* optimalSupport) {
    VkFormatProperties props;
    memset(&props, 0, sizeof(VkFormatProperties));
    GR_VK_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &props));
    *linearSupport = SkToBool(props.linearTilingFeatures & featureBit);
    *optimalSupport = SkToBool(props.optimalTilingFeatures & featureBit);
}

static void config_supported_for_feature(const GrVkInterface* interface,
                                         VkPhysicalDevice physDev,
                                         GrPixelConfig config,
                                         VkFormatFeatureFlagBits featureBit,
                                         bool* linearSupport,
                                         bool* optimalSupport) {
    VkFormat format;
    if (!GrPixelConfigToVkFormat(config, &format)) {
        *linearSupport = false;
        *optimalSupport = false;
        return;
    }
    format_supported_for_feature(interface, physDev, format, featureBit,
                                 linearSupport, optimalSupport);
}

// Currently just assumeing if something can be rendered to without MSAA it also works for MSAAA
#define SET_CONFIG_IS_RENDERABLE(config)                                                          \
    config_supported_for_feature(interface,                                                       \
                                 physDev,                                                         \
                                 config,                                    \
                                 VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT,                    \
                                 &fConfigLinearRenderSupport[config][kNo_MSAA],                   \
                                 &fConfigRenderSupport[config][kNo_MSAA] );                       \
    fConfigRenderSupport[config][kYes_MSAA] = fConfigRenderSupport[config][kNo_MSAA];             \
    fConfigLinearRenderSupport[config][kYes_MSAA] = fConfigLinearRenderSupport[config][kNo_MSAA];


void GrVkCaps::initConfigRenderableTable(const GrVkInterface* interface, VkPhysicalDevice physDev) {
    enum {
        kNo_MSAA = 0,
        kYes_MSAA = 1,
    };

    // Base render support
    SET_CONFIG_IS_RENDERABLE(kAlpha_8_GrPixelConfig);
    SET_CONFIG_IS_RENDERABLE(kRGB_565_GrPixelConfig);
    SET_CONFIG_IS_RENDERABLE(kRGBA_4444_GrPixelConfig);
    SET_CONFIG_IS_RENDERABLE(kRGBA_8888_GrPixelConfig);
    SET_CONFIG_IS_RENDERABLE(kBGRA_8888_GrPixelConfig);

    SET_CONFIG_IS_RENDERABLE(kSRGBA_8888_GrPixelConfig);

    // Float render support
    SET_CONFIG_IS_RENDERABLE(kRGBA_float_GrPixelConfig);
    SET_CONFIG_IS_RENDERABLE(kRGBA_half_GrPixelConfig);
    SET_CONFIG_IS_RENDERABLE(kAlpha_half_GrPixelConfig);
}

#define SET_CONFIG_IS_TEXTURABLE(config)                                 \
    config_supported_for_feature(interface,                              \
                                 physDev,                                \
                                 config,                                 \
                                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,    \
                                 &fConfigLinearTextureSupport[config],   \
                                 &fConfigTextureSupport[config]);

void GrVkCaps::initConfigTexturableTable(const GrVkInterface* interface, VkPhysicalDevice physDev) {
    // Base texture support
    SET_CONFIG_IS_TEXTURABLE(kAlpha_8_GrPixelConfig);
    SET_CONFIG_IS_TEXTURABLE(kRGB_565_GrPixelConfig);
    SET_CONFIG_IS_TEXTURABLE(kRGBA_4444_GrPixelConfig);
    SET_CONFIG_IS_TEXTURABLE(kRGBA_8888_GrPixelConfig);
    SET_CONFIG_IS_TEXTURABLE(kBGRA_8888_GrPixelConfig);

    SET_CONFIG_IS_TEXTURABLE(kIndex_8_GrPixelConfig);
    SET_CONFIG_IS_TEXTURABLE(kSRGBA_8888_GrPixelConfig);

    // Compressed texture support
    SET_CONFIG_IS_TEXTURABLE(kETC1_GrPixelConfig);
    SET_CONFIG_IS_TEXTURABLE(kLATC_GrPixelConfig);
    SET_CONFIG_IS_TEXTURABLE(kR11_EAC_GrPixelConfig);
    SET_CONFIG_IS_TEXTURABLE(kASTC_12x12_GrPixelConfig);

    // Float texture support
    SET_CONFIG_IS_TEXTURABLE(kRGBA_float_GrPixelConfig);
    SET_CONFIG_IS_TEXTURABLE(kRGBA_half_GrPixelConfig);
    SET_CONFIG_IS_TEXTURABLE(kAlpha_half_GrPixelConfig);
}

#define SET_CONFIG_CAN_STENCIL(config)                                                    \
    bool SK_MACRO_APPEND_LINE(linearSupported);                                           \
    bool SK_MACRO_APPEND_LINE(optimalSupported);                                          \
    format_supported_for_feature(interface,                                               \
                                 physDev,                                                 \
                                 config.fInternalFormat,                                  \
                                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,          \
                                 &SK_MACRO_APPEND_LINE(linearSupported),                  \
                                 &SK_MACRO_APPEND_LINE(optimalSupported));                \
    if (SK_MACRO_APPEND_LINE(linearSupported)) fLinearStencilFormats.push_back(config);   \
    if (SK_MACRO_APPEND_LINE(optimalSupported)) fStencilFormats.push_back(config);

void GrVkCaps::initStencilFormats(const GrVkInterface* interface, VkPhysicalDevice physDev) {
    // Build up list of legal stencil formats (though perhaps not supported on
    // the particular gpu/driver) from most preferred to least.

    static const StencilFormat
                  // internal Format             stencil bits      total bits        packed?
        gS8    = { VK_FORMAT_S8_UINT,            8,                 8,               false },
        gD24S8 = { VK_FORMAT_D24_UNORM_S8_UINT,  8,                32,               true };

    // I'm simply assuming that these two will be supported since they are used in example code.
    // TODO: Actaully figure this out
    SET_CONFIG_CAN_STENCIL(gS8);
    SET_CONFIG_CAN_STENCIL(gD24S8);
}

