/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLCaps_DEFINED
#define GrGLCaps_DEFINED

#include "GrCaps.h"
#include "glsl/GrGLSL.h"
#include "GrGLStencilAttachment.h"
#include "SkChecksum.h"
#include "SkTHash.h"
#include "SkTArray.h"

class GrGLContextInfo;
class GrGLSLCaps;

/**
 * Stores some capabilities of a GL context. Most are determined by the GL
 * version and the extensions string. It also tracks formats that have passed
 * the FBO completeness test.
 */
class GrGLCaps : public GrCaps {
public:
    typedef GrGLStencilAttachment::Format StencilFormat;

    /** Provides information about the mappiing from GrPixelConfig to GL formats. */
    struct ConfigFormats {
        ConfigFormats() {
            // Inits to known bad GL enum values.
            memset(this, 0xAB, sizeof(ConfigFormats));
        }
        GrGLenum fBaseInternalFormat;
        GrGLenum fSizedInternalFormat;
        GrGLenum fExternalFormat;
        GrGLenum fExternalType;

        // The <format> parameter to use for glTexImage and glTexSubImage.
        // This is usually the same as fExternalFormat except for kSRGBA on some GL contexts.
        GrGLenum fExternalFormatForTexImage;
        // Either the base or sized internal format depending on the GL and config.
        GrGLenum fInternalFormatTexImage;
    };

    /**
     * The type of MSAA for FBOs supported. Different extensions have different
     * semantics of how / when a resolve is performed.
     */
    enum MSFBOType {
        /**
         * no support for MSAA FBOs
         */
        kNone_MSFBOType = 0,
        /**
         * GL3.0-style MSAA FBO (GL_ARB_framebuffer_object).
         */
        kDesktop_ARB_MSFBOType,
        /**
         * earlier GL_EXT_framebuffer* extensions
         */
        kDesktop_EXT_MSFBOType,
        /**
         * Similar to kDesktop_ARB but with additional restrictions on glBlitFramebuffer.
         */
        kES_3_0_MSFBOType,
        /**
         * GL_APPLE_framebuffer_multisample ES extension
         */
        kES_Apple_MSFBOType,
        /**
         * GL_IMG_multisampled_render_to_texture. This variation does not have MSAA renderbuffers.
         * Instead the texture is multisampled when bound to the FBO and then resolved automatically
         * when read. It also defines an alternate value for GL_MAX_SAMPLES (which we call
         * GR_GL_MAX_SAMPLES_IMG).
         */
        kES_IMG_MsToTexture_MSFBOType,
        /**
         * GL_EXT_multisampled_render_to_texture. Same as the IMG one above but uses the standard
         * GL_MAX_SAMPLES value.
         */
        kES_EXT_MsToTexture_MSFBOType,
        /**
         * GL_NV_framebuffer_mixed_samples.
         */
        kMixedSamples_MSFBOType,

        kLast_MSFBOType = kMixedSamples_MSFBOType
    };

    enum InvalidateFBType {
        kNone_InvalidateFBType,
        kDiscard_InvalidateFBType,       //<! glDiscardFramebuffer()
        kInvalidate_InvalidateFBType,     //<! glInvalidateFramebuffer()

        kLast_InvalidateFBType = kInvalidate_InvalidateFBType
    };

    enum MapBufferType {
        kNone_MapBufferType,
        kMapBuffer_MapBufferType,         // glMapBuffer()
        kMapBufferRange_MapBufferType,    // glMapBufferRange()
        kChromium_MapBufferType,          // GL_CHROMIUM_map_sub

        kLast_MapBufferType = kChromium_MapBufferType,
    };

    enum TransferBufferType {
        kNone_TransferBufferType,
        kPBO_TransferBufferType,          // ARB_pixel_buffer_object
        kChromium_TransferBufferType,     // CHROMIUM_pixel_transfer_buffer_object

        kLast_TransferBufferType = kChromium_TransferBufferType,
    };

    /**
     * Initializes the GrGLCaps to the set of features supported in the current
     * OpenGL context accessible via ctxInfo.
     */
    GrGLCaps(const GrContextOptions& contextOptions, const GrGLContextInfo& ctxInfo,
             const GrGLInterface* glInterface);

    /** Returns conversions to various GL format parameters for a GrPixelCfonig. */
    const ConfigFormats& configGLFormats(GrPixelConfig config) const {
        return fConfigTable[config].fFormats;
    }


    /**
    * Gets an array of legal stencil formats. These formats are not guaranteed
    * to be supported by the driver but are legal GLenum names given the GL
    * version and extensions supported.
    */
    const SkTArray<StencilFormat, true>& stencilFormats() const {
        return fStencilFormats;
    }

    /**
     * Has a stencil format index been found for the config (or we've found that no format works).
     */
    bool hasStencilFormatBeenDeterminedForConfig(GrPixelConfig config) const {
        return fConfigTable[config].fStencilFormatIndex != ConfigInfo::kUnknown_StencilIndex;
    }

    /**
     * Gets the stencil format index for the config. This assumes
     * hasStencilFormatBeenDeterminedForConfig has already been checked. Returns a value < 0 if
     * no stencil format is supported with the config. Otherwise, returned index refers to the array
     * returned by stencilFormats().
     */
    int getStencilFormatIndexForConfig(GrPixelConfig config) const {
        SkASSERT(this->hasStencilFormatBeenDeterminedForConfig(config));
        return fConfigTable[config].fStencilFormatIndex;
    }

    /**
     * If index is >= 0 this records an index into stencilFormats() as the best stencil format for
     * the config. If < 0 it records that the config has no supported stencil format index.
     */
    void setStencilFormatIndexForConfig(GrPixelConfig config, int index) {
        SkASSERT(!this->hasStencilFormatBeenDeterminedForConfig(config));
        if (index < 0) {
            fConfigTable[config].fStencilFormatIndex = ConfigInfo::kUnsupported_StencilFormatIndex;
        } else {
            fConfigTable[config].fStencilFormatIndex = index;
        }
    }

    /**
     * Call to note that a color config has been verified as a valid color
     * attachment. This may save future calls to glCheckFramebufferStatus
     * using isConfigVerifiedColorAttachment().
     */
    void markConfigAsValidColorAttachment(GrPixelConfig config) {
        fConfigTable[config].fFlags |= ConfigInfo::kVerifiedColorAttachment_Flag;
    }

    /**
     * Call to check whether a config has been verified as a valid color
     * attachment.
     */
    bool isConfigVerifiedColorAttachment(GrPixelConfig config) const {
        return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kVerifiedColorAttachment_Flag);
    }

    /**
     * Reports the type of MSAA FBO support.
     */
    MSFBOType msFBOType() const { return fMSFBOType; }

    /**
     * Does the supported MSAA FBO extension have MSAA renderbuffers?
     */
    bool usesMSAARenderBuffers() const {
        return kNone_MSFBOType != fMSFBOType &&
               kES_IMG_MsToTexture_MSFBOType != fMSFBOType &&
               kES_EXT_MsToTexture_MSFBOType != fMSFBOType &&
               kMixedSamples_MSFBOType != fMSFBOType;
    }

    /**
     * Is the MSAA FBO extension one where the texture is multisampled when bound to an FBO and
     * then implicitly resolved when read.
     */
    bool usesImplicitMSAAResolve() const {
        return kES_IMG_MsToTexture_MSFBOType == fMSFBOType ||
               kES_EXT_MsToTexture_MSFBOType == fMSFBOType;
    }

    InvalidateFBType invalidateFBType() const { return fInvalidateFBType; }

    /// What type of buffer mapping is supported?
    MapBufferType mapBufferType() const { return fMapBufferType; }

    /// What type of transfer buffer is supported?
    TransferBufferType transferBufferType() const { return fTransferBufferType; }

    /// The maximum number of fragment uniform vectors (GLES has min. 16).
    int maxFragmentUniformVectors() const { return fMaxFragmentUniformVectors; }

    /// maximum number of attribute values per vertex
    int maxVertexAttributes() const { return fMaxVertexAttributes; }

    /// maximum number of texture units accessible in the fragment shader.
    int maxFragmentTextureUnits() const { return fMaxFragmentTextureUnits; }

    /// ES requires an extension to support RGBA8 in RenderBufferStorage
    bool rgba8RenderbufferSupport() const { return fRGBA8RenderbufferSupport; }

    /**
     * Depending on the ES extensions present the BGRA external format may
     * correspond either a BGRA or RGBA internalFormat. On desktop GL it is
     * RGBA.
     */
    bool bgraIsInternalFormat() const { return fBGRAIsInternalFormat; }

    /// Is there support for GL_UNPACK_ROW_LENGTH
    bool unpackRowLengthSupport() const { return fUnpackRowLengthSupport; }

    /// Is there support for GL_UNPACK_FLIP_Y
    bool unpackFlipYSupport() const { return fUnpackFlipYSupport; }

    /// Is there support for GL_PACK_ROW_LENGTH
    bool packRowLengthSupport() const { return fPackRowLengthSupport; }

    /// Is there support for GL_PACK_REVERSE_ROW_ORDER
    bool packFlipYSupport() const { return fPackFlipYSupport; }

    /// Is there support for texture parameter GL_TEXTURE_USAGE
    bool textureUsageSupport() const { return fTextureUsageSupport; }

    /// Is there support for glTexStorage
    bool texStorageSupport() const { return fTexStorageSupport; }

    /// Is there support for GL_RED and GL_R8
    bool textureRedSupport() const { return fTextureRedSupport; }

    /// Is GL_ARB_IMAGING supported
    bool imagingSupport() const { return fImagingSupport; }

    /// Is there support for Vertex Array Objects?
    bool vertexArrayObjectSupport() const { return fVertexArrayObjectSupport; }

    /// Is there support for GL_EXT_direct_state_access?
    bool directStateAccessSupport() const { return fDirectStateAccessSupport; }

    /// Is there support for GL_KHR_debug?
    bool debugSupport() const { return fDebugSupport; }

    /// Is there support for ES2 compatability?
    bool ES2CompatibilitySupport() const { return fES2CompatibilitySupport; }

    /// Can we call glDisable(GL_MULTISAMPLE)?
    bool multisampleDisableSupport() const {
        return fMultisampleDisableSupport;
    }

    /// Use indices or vertices in CPU arrays rather than VBOs for dynamic content.
    bool useNonVBOVertexAndIndexDynamicData() const {
        return fUseNonVBOVertexAndIndexDynamicData;
    }

    /// Does ReadPixels support the provided format/type combo?
    bool readPixelsSupported(const GrGLInterface* intf,
                             GrGLenum format,
                             GrGLenum type,
                             GrGLenum currFboFormat) const;

    bool isCoreProfile() const { return fIsCoreProfile; }

    bool bindFragDataLocationSupport() const { return fBindFragDataLocationSupport; }

    bool bindUniformLocationSupport() const { return fBindUniformLocationSupport; }

    /// Are textures with GL_TEXTURE_EXTERNAL_OES type supported.
    bool externalTextureSupport() const { return fExternalTextureSupport; }

    /**
     * Is there support for enabling/disabling sRGB writes for sRGB-capable color attachments?
     * If false this does not mean sRGB is not supported but rather that if it is supported
     * it cannot be turned off for configs that support it.
     */
    bool srgbWriteControl() const { return fSRGBWriteControl; }

    /**
     * Returns a string containing the caps info.
     */
    SkString dump() const override;

    /**
     * LATC can appear under one of three possible names. In order to know
     * which GL internal format to use, we need to keep track of which name
     * we found LATC under. The default is LATC.
     */
    enum LATCAlias {
        kLATC_LATCAlias,
        kRGTC_LATCAlias,
        k3DC_LATCAlias
    };

    LATCAlias latcAlias() const { return fLATCAlias; }

    bool rgba8888PixelsOpsAreSlow() const { return fRGBA8888PixelsOpsAreSlow; }
    bool partialFBOReadIsSlow() const { return fPartialFBOReadIsSlow; }

    const GrGLSLCaps* glslCaps() const { return reinterpret_cast<GrGLSLCaps*>(fShaderCaps.get()); }

private:
    void init(const GrContextOptions&, const GrGLContextInfo&, const GrGLInterface*);
    void initGLSL(const GrGLContextInfo&);
    bool hasPathRenderingSupport(const GrGLContextInfo&, const GrGLInterface*);

    void onApplyOptionsOverrides(const GrContextOptions& options) override;

    void initFSAASupport(const GrGLContextInfo&, const GrGLInterface*);
    void initBlendEqationSupport(const GrGLContextInfo&);
    void initStencilFormats(const GrGLContextInfo&);
    // This must be called after initFSAASupport().
    void initConfigRenderableTable(const GrGLContextInfo&, bool srgbSupport);
    void initConfigTexturableTable(const GrGLContextInfo&, const GrGLInterface*, bool srgbSupport);

    bool doReadPixelsSupported(const GrGLInterface* intf, GrGLenum format, GrGLenum type) const;

    void initShaderPrecisionTable(const GrGLContextInfo& ctxInfo,
                                  const GrGLInterface* intf,
                                  GrGLSLCaps* glslCaps);

    void initConfigSwizzleTable(const GrGLContextInfo& ctxInfo, GrGLSLCaps* glslCaps);

    void initConfigTable(const GrGLContextInfo&);

    SkTArray<StencilFormat, true> fStencilFormats;

    int fMaxFragmentUniformVectors;
    int fMaxVertexAttributes;
    int fMaxFragmentTextureUnits;

    MSFBOType           fMSFBOType;
    InvalidateFBType    fInvalidateFBType;
    MapBufferType       fMapBufferType;
    TransferBufferType  fTransferBufferType;
    LATCAlias           fLATCAlias;

    bool fRGBA8RenderbufferSupport : 1;
    bool fBGRAIsInternalFormat : 1;
    bool fUnpackRowLengthSupport : 1;
    bool fUnpackFlipYSupport : 1;
    bool fPackRowLengthSupport : 1;
    bool fPackFlipYSupport : 1;
    bool fTextureUsageSupport : 1;
    bool fTexStorageSupport : 1;
    bool fTextureRedSupport : 1;
    bool fImagingSupport  : 1;
    bool fTwoFormatLimit : 1;
    bool fVertexArrayObjectSupport : 1;
    bool fDirectStateAccessSupport : 1;
    bool fDebugSupport : 1;
    bool fES2CompatibilitySupport : 1;
    bool fMultisampleDisableSupport : 1;
    bool fUseNonVBOVertexAndIndexDynamicData : 1;
    bool fIsCoreProfile : 1;
    bool fBindFragDataLocationSupport : 1;
    bool fSRGBWriteControl : 1;
    bool fRGBA8888PixelsOpsAreSlow : 1;
    bool fPartialFBOReadIsSlow : 1;
    bool fBindUniformLocationSupport : 1;
    bool fExternalTextureSupport : 1;

    struct ConfigInfo {
        ConfigInfo() : fStencilFormatIndex(kUnknown_StencilIndex), fFlags(0) {};

        ConfigFormats fFormats;

        enum {
            // This indicates that a stencil format has not yet been determined for the config.
            kUnknown_StencilIndex = -1,
            // This indicates that there is no supported stencil format for the config.
            kUnsupported_StencilFormatIndex = -2
        };

        // Index fStencilFormats.
        int      fStencilFormatIndex;

        enum {
            kVerifiedColorAttachment_Flag = 0x1
        };
        uint32_t fFlags;
    };

    ConfigInfo fConfigTable[kGrPixelConfigCnt];

    struct ReadPixelsSupportedFormat {
        GrGLenum fFormat;
        GrGLenum fType;
        GrGLenum fFboFormat;

        bool operator==(const ReadPixelsSupportedFormat& rhs) const {
            return fFormat    == rhs.fFormat
                && fType      == rhs.fType
                && fFboFormat == rhs.fFboFormat;
        }
    };
    mutable SkTHashMap<ReadPixelsSupportedFormat, bool> fReadPixelsSupportedCache;

    typedef GrCaps INHERITED;
};

#endif
