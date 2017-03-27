/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProducer_DEFINED
#define GrTextureProducer_DEFINED

#include "GrSamplerParams.h"
#include "GrResourceKey.h"

class GrColorSpaceXform;
class GrResourceProvider;
class GrTexture;
class GrTextureProxy;

/**
 * Different GPUs and API extensions have different requirements with respect to what texture
 * sampling parameters may be used with textures of various types. This class facilitates making
 * texture compatible with a given GrSamplerParams. There are two immediate subclasses defined
 * below. One is a base class for sources that are inherently texture-backed (e.g. a texture-backed
 * SkImage). It supports subsetting the original texture. The other is for use cases where the
 * source can generate a texture that represents some content (e.g. cpu pixels, SkPicture, ...).
 */
class GrTextureProducer : public SkNoncopyable {
public:
    struct CopyParams {
        GrSamplerParams::FilterMode fFilter;
        int                         fWidth;
        int                         fHeight;
    };

    enum FilterConstraint {
        kYes_FilterConstraint,
        kNo_FilterConstraint,
    };

    /**
     * Helper for creating a fragment processor to sample the texture with a given filtering mode.
     * It attempts to avoid making texture copies or using domains whenever possible.
     *
     * @param textureMatrix                    Matrix used to access the texture. It is applied to
     *                                         the local coords. The post-transformed coords should
     *                                         be in texel units (rather than normalized) with
     *                                         respect to this Producer's bounds (width()/height()).
     * @param constraintRect                   A rect that represents the area of the texture to be
     *                                         sampled. It must be contained in the Producer's
     *                                         bounds as defined by width()/height().
     * @param filterConstriant                 Indicates whether filtering is limited to
     *                                         constraintRect.
     * @param coordsLimitedToConstraintRect    Is it known that textureMatrix*localCoords is bound
     *                                         by the portion of the texture indicated by
     *                                         constraintRect (without consideration of filter
     *                                         width, just the raw coords).
     * @param filterOrNullForBicubic           If non-null indicates the filter mode. If null means
     *                                         use bicubic filtering.
     **/
    virtual sk_sp<GrFragmentProcessor> createFragmentProcessor(
                                    const SkMatrix& textureMatrix,
                                    const SkRect& constraintRect,
                                    FilterConstraint filterConstraint,
                                    bool coordsLimitedToConstraintRect,
                                    const GrSamplerParams::FilterMode* filterOrNullForBicubic,
                                    SkColorSpace* dstColorSpace) = 0;

    virtual ~GrTextureProducer() {}

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    bool isAlphaOnly() const { return fIsAlphaOnly; }
    virtual SkAlphaType alphaType() const = 0;

protected:
    friend class GrTextureProducer_TestAccess;

    GrTextureProducer(int width, int height, bool isAlphaOnly)
        : fWidth(width)
        , fHeight(height)
        , fIsAlphaOnly(isAlphaOnly) {}

    /** Helper for creating a key for a copy from an original key. */
    static void MakeCopyKeyFromOrigKey(const GrUniqueKey& origKey,
                                       const CopyParams& copyParams,
                                       GrUniqueKey* copyKey) {
        SkASSERT(!copyKey->isValid());
        if (origKey.isValid()) {
            static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
            GrUniqueKey::Builder builder(copyKey, origKey, kDomain, 3);
            builder[0] = copyParams.fFilter;
            builder[1] = copyParams.fWidth;
            builder[2] = copyParams.fHeight;
        }
    }

    /**
    *  If we need to make a copy in order to be compatible with GrTextureParams producer is asked to
    *  return a key that identifies its original content + the CopyParms parameter. If the producer
    *  does not want to cache the stretched version (e.g. the producer is volatile), this should
    *  simply return without initializing the copyKey. If the texture generated by this producer
    *  depends on the destination color space, then that information should also be incorporated
    *  in the key.
    */
    virtual void makeCopyKey(const CopyParams&, GrUniqueKey* copyKey,
                             SkColorSpace* dstColorSpace) = 0;

    /**
    *  If a stretched version of the texture is generated, it may be cached (assuming that
    *  makeCopyKey() returns true). In that case, the maker is notified in case it
    *  wants to note that for when the maker is destroyed.
    */
    virtual void didCacheCopy(const GrUniqueKey& copyKey) = 0;


    enum DomainMode {
        kNoDomain_DomainMode,
        kDomain_DomainMode,
        kTightCopy_DomainMode
    };

    static GrTexture* CopyOnGpu(GrTexture* inputTexture, const SkIRect* subset,
                                const CopyParams& copyParams);

    static sk_sp<GrTextureProxy> CopyOnGpu(GrContext*, sk_sp<GrTextureProxy> inputProxy,
                                           const SkIRect* subset, const CopyParams& copyParams);

    static DomainMode DetermineDomainMode(
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        int texW, int texH,
        const SkIRect* textureContentArea,
        const GrSamplerParams::FilterMode* filterModeOrNullForBicubic,
        SkRect* domainRect);

    static DomainMode DetermineDomainMode(
        const SkRect& constraintRect,
        FilterConstraint filterConstraint,
        bool coordsLimitedToConstraintRect,
        GrTextureProxy*,
        const SkIRect* textureContentArea,
        const GrSamplerParams::FilterMode* filterModeOrNullForBicubic,
        SkRect* domainRect);

    static sk_sp<GrFragmentProcessor> CreateFragmentProcessorForDomainAndFilter(
        GrTexture* texture,
        sk_sp<GrColorSpaceXform> colorSpaceXform,
        const SkMatrix& textureMatrix,
        DomainMode domainMode,
        const SkRect& domain,
        const GrSamplerParams::FilterMode* filterOrNullForBicubic);

    static sk_sp<GrFragmentProcessor> CreateFragmentProcessorForDomainAndFilter(
        GrResourceProvider*,
        sk_sp<GrTextureProxy> proxy,
        sk_sp<GrColorSpaceXform>,
        const SkMatrix& textureMatrix,
        DomainMode,
        const SkRect& domain,
        const GrSamplerParams::FilterMode* filterOrNullForBicubic);

private:
    const int   fWidth;
    const int   fHeight;
    const bool  fIsAlphaOnly;

    typedef SkNoncopyable INHERITED;
};

#endif
