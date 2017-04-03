/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrXferProcessor_DEFINED
#define GrXferProcessor_DEFINED

#include "GrBlend.h"
#include "GrColor.h"
#include "GrProcessor.h"
#include "GrProcessorSet.h"
#include "GrTexture.h"
#include "GrTypes.h"

class GrShaderCaps;
class GrGLSLXferProcessor;

/**
 * Barriers for blending. When a shader reads the dst directly, an Xfer barrier is sometimes
 * required after a pixel has been written, before it can be safely read again.
 */
enum GrXferBarrierType {
    kNone_GrXferBarrierType = 0, //<! No barrier is required
    kTexture_GrXferBarrierType,  //<! Required when a shader reads and renders to the same texture.
    kBlend_GrXferBarrierType,    //<! Required by certain blend extensions.
};
/** Should be able to treat kNone as false in boolean expressions */
GR_STATIC_ASSERT(SkToBool(kNone_GrXferBarrierType) == false);

/**
 * GrXferProcessor is responsible for implementing the xfer mode that blends the src color and dst
 * color, and for applying any coverage. It does this by emitting fragment shader code and
 * controlling the fixed-function blend state. When dual-source blending is available, it may also
 * write a seconday fragment shader output color. GrXferProcessor has two modes of operation:
 *
 * Dst read: When allowed by the backend API, or when supplied a texture of the destination, the
 * GrXferProcessor may read the destination color. While operating in this mode, the subclass only
 * provides shader code that blends the src and dst colors, and the base class applies coverage.
 *
 * No dst read: When not performing a dst read, the subclass is given full control of the fixed-
 * function blend state and/or secondary output, and is responsible to apply coverage on its own.
 *
 * A GrXferProcessor is never installed directly into our draw state, but instead is created from a
 * GrXPFactory once we have finalized the state of our draw.
 */
class GrXferProcessor : public GrProcessor {
public:
    /**
     * A texture that contains the dst pixel values and an integer coord offset from device space
     * to the space of the texture. Depending on GPU capabilities a DstTexture may be used by a
     * GrXferProcessor for blending in the fragment shader.
     */
    class DstTexture {
    public:
        DstTexture() { fOffset.set(0, 0); }

        DstTexture(const DstTexture& other) {
            *this = other;
        }

        DstTexture(GrTexture* texture, const SkIPoint& offset)
                : fTexture(SkSafeRef(texture)), fOffset(texture ? offset : SkIPoint{0, 0}) {}

        DstTexture& operator=(const DstTexture& other) {
            fTexture = other.fTexture;
            fOffset = other.fOffset;
            return *this;
        }

        bool operator==(const DstTexture& that) const {
            return fTexture == that.fTexture && fOffset == that.fOffset;
        }
        bool operator!=(const DstTexture& that) const { return !(*this == that); }

        const SkIPoint& offset() const { return fOffset; }

        void setOffset(const SkIPoint& offset) { fOffset = offset; }
        void setOffset(int ox, int oy) { fOffset.set(ox, oy); }

        GrTexture* texture() const { return fTexture.get(); }

        void setTexture(sk_sp<GrTexture> texture) {
            fTexture = std::move(texture);
            if (!fTexture) {
                fOffset = {0, 0};
            }
        }

    private:
        sk_sp<GrTexture> fTexture;
        SkIPoint         fOffset;
    };

    /**
     * Sets a unique key on the GrProcessorKeyBuilder calls onGetGLSLProcessorKey(...) to get the
     * specific subclass's key.
     */
    void getGLSLProcessorKey(const GrShaderCaps&,
                             GrProcessorKeyBuilder*,
                             const GrSurfaceOrigin* originIfDstTexture) const;

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrXferProcessor; caller is responsible for deleting
        the object. */
    virtual GrGLSLXferProcessor* createGLSLInstance() const = 0;

    /**
     * Returns the barrier type, if any, that this XP will require. Note that the possibility
     * that a kTexture type barrier is required is handled by the GrPipeline and need not be
     * considered by subclass overrides of this function.
     */
    virtual GrXferBarrierType xferBarrierType(const GrCaps& caps) const {
        return kNone_GrXferBarrierType;
    }

    struct BlendInfo {
        void reset() {
            fEquation = kAdd_GrBlendEquation;
            fSrcBlend = kOne_GrBlendCoeff;
            fDstBlend = kZero_GrBlendCoeff;
            fBlendConstant = 0;
            fWriteColor = true;
        }

        SkDEBUGCODE(SkString dump() const;)

        GrBlendEquation fEquation;
        GrBlendCoeff    fSrcBlend;
        GrBlendCoeff    fDstBlend;
        GrColor         fBlendConstant;
        bool            fWriteColor;
    };

    void getBlendInfo(BlendInfo* blendInfo) const;

    bool willReadDstColor() const { return fWillReadDstColor; }

    /**
     * If we are performing a dst read, returns whether the base class will use mixed samples to
     * antialias the shader's final output. If not doing a dst read, the subclass is responsible
     * for antialiasing and this returns false.
     */
    bool dstReadUsesMixedSamples() const { return fDstReadUsesMixedSamples; }

    /**
     * Returns whether or not this xferProcossor will set a secondary output to be used with dual
     * source blending.
     */
    bool hasSecondaryOutput() const;

    /** Returns true if this and other processor conservatively draw identically. It can only return
        true when the two processor are of the same subclass (i.e. they return the same object from
        from getFactory()).

        A return value of true from isEqual() should not be used to test whether the processor would
        generate the same shader code. To test for identical code generation use getGLSLProcessorKey
      */
    
    bool isEqual(const GrXferProcessor& that) const {
        if (this->classID() != that.classID()) {
            return false;
        }
        if (this->fWillReadDstColor != that.fWillReadDstColor) {
            return false;
        }
        if (this->fDstReadUsesMixedSamples != that.fDstReadUsesMixedSamples) {
            return false;
        }
        return this->onIsEqual(that);
    }

protected:
    GrXferProcessor();
    GrXferProcessor(bool willReadDstColor, bool hasMixedSamples);

private:
    void notifyRefCntIsZero() const final {}

    /**
     * Sets a unique key on the GrProcessorKeyBuilder that is directly associated with this xfer
     * processor's GL backend implementation.
     */
    virtual void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const = 0;

    /**
     * If we are not performing a dst read, returns whether the subclass will set a secondary
     * output. When using dst reads, the base class controls the secondary output and this method
     * will not be called.
     */
    virtual bool onHasSecondaryOutput() const { return false; }

    /**
     * If we are not performing a dst read, retrieves the fixed-function blend state required by the
     * subclass. When using dst reads, the base class controls the fixed-function blend state and
     * this method will not be called. The BlendInfo struct comes initialized to "no blending".
     */
    virtual void onGetBlendInfo(BlendInfo*) const {}

    virtual bool onIsEqual(const GrXferProcessor&) const = 0;

    bool                    fWillReadDstColor;
    bool                    fDstReadUsesMixedSamples;

    typedef GrFragmentProcessor INHERITED;
};

/**
 * We install a GrXPFactory (XPF) early on in the pipeline before all the final draw information is
 * known (e.g. whether there is fractional pixel coverage, will coverage be 1 or 4 channel, is the
 * draw opaque, etc.). Once the state of the draw is finalized, we use the XPF along with all the
 * draw information to create a GrXferProcessor (XP) which can implement the desired blending for
 * the draw.
 *
 * Before the XP is created, the XPF is able to answer queries about what functionality the XPs it
 * creates will have. For example, can it create an XP that supports RGB coverage or will the XP
 * blend with the destination color.
 *
 * GrXPFactories are intended to be static immutable objects. We pass them around as raw pointers
 * and expect the pointers to always be valid and for the factories to be reusable and thread safe.
 * Equality is tested for using pointer comparison. GrXPFactory destructors must be no-ops.
 */

// In order to construct GrXPFactory subclass instances as constexpr the subclass, and therefore
// GrXPFactory, must be a literal type. One requirement is having a trivial destructor. This is ok
// since these objects have no need for destructors. However, GCC and clang throw a warning when a
// class has virtual functions and a non-virtual destructor. We suppress that warning here and
// for the subclasses.
#if defined(__GNUC__) || defined(__clang)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif
class GrXPFactory {
public:
    typedef GrXferProcessor::DstTexture DstTexture;

    GrXferProcessor* createXferProcessor(const GrProcessorAnalysisColor&,
                                         GrProcessorAnalysisCoverage,
                                         bool hasMixedSamples,
                                         const GrCaps& caps) const;

    enum class AnalysisProperties : unsigned {
        kNone = 0x0,
        /**
         * The fragment shader will require the destination color.
         */
        kReadsDstInShader = 0x1,
        /**
         * The op may apply coverage as alpha and still blend correctly.
         */
        kCompatibleWithAlphaAsCoverage = 0x2,
        /**
         * The color input to the GrXferProcessor will be ignored.
         */
        kIgnoresInputColor = 0x4,
        /**
         * If set overlapping stencil and cover operations can be replaced by a combined stencil
         * followed by a combined cover.
         */
        kCanCombineOverlappedStencilAndCover = 0x8,
        /**
         * The destination color will be provided to the fragment processor using a texture. This is
         * additional information about the implementation of kReadsDstInShader.
         */
        kRequiresDstTexture = 0x10,
        /**
         * If set overlapping draws may not be combined because a barrier must be inserted between
         * them.
         */
        kRequiresBarrierBetweenOverlappingDraws = 0x20,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(AnalysisProperties);

    static AnalysisProperties GetAnalysisProperties(const GrXPFactory*,
                                                    const GrProcessorAnalysisColor&,
                                                    const GrProcessorAnalysisCoverage&,
                                                    const GrCaps&);

protected:
    constexpr GrXPFactory() {}

private:
    virtual GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                                   const GrProcessorAnalysisColor&,
                                                   GrProcessorAnalysisCoverage,
                                                   bool hasMixedSamples) const = 0;

    /**
     * Subclass analysis implementation. This should not return kNeedsDstInTexture as that will be
     * inferred by the base class based on kReadsDstInShader and the caps.
     */
    virtual AnalysisProperties analysisProperties(const GrProcessorAnalysisColor&,
                                                  const GrProcessorAnalysisCoverage&,
                                                  const GrCaps&) const = 0;
};
#if defined(__GNUC__) || defined(__clang)
#pragma GCC diagnostic pop
#endif

GR_MAKE_BITFIELD_CLASS_OPS(GrXPFactory::AnalysisProperties);

#endif
