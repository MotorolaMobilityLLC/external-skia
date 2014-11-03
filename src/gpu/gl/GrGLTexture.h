/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLTexture_DEFINED
#define GrGLTexture_DEFINED

#include "GrGpu.h"
#include "GrTexture.h"
#include "GrGLUtil.h"

/**
 * A ref counted tex id that deletes the texture in its destructor.
 */
class GrGLTexID : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrGLTexID)

    GrGLTexID(const GrGLInterface* gl, GrGLuint texID, bool isWrapped)
        : fGL(gl)
        , fTexID(texID)
        , fIsWrapped(isWrapped) {
    }

    virtual ~GrGLTexID() {
        if (0 != fTexID && !fIsWrapped) {
            GR_GL_CALL(fGL, DeleteTextures(1, &fTexID));
        }
    }

    void abandon() { fTexID = 0; }
    GrGLuint id() const { return fTexID; }

private:
    const GrGLInterface* fGL;
    GrGLuint             fTexID;
    bool                 fIsWrapped;

    typedef SkRefCnt INHERITED;
};

////////////////////////////////////////////////////////////////////////////////


class GrGLTexture : public GrTexture {

public:
    struct TexParams {
        GrGLenum fMinFilter;
        GrGLenum fMagFilter;
        GrGLenum fWrapS;
        GrGLenum fWrapT;
        GrGLenum fSwizzleRGBA[4];
        void invalidate() { memset(this, 0xff, sizeof(TexParams)); }
    };

    struct IDDesc {
        GrGLuint        fTextureID;
        bool            fIsWrapped;
    };

    GrGLTexture(GrGpuGL*, const GrSurfaceDesc&, const IDDesc&);

    virtual ~GrGLTexture() { this->release(); }

    virtual GrBackendObject getTextureHandle() const SK_OVERRIDE;

    virtual void textureParamsModified() SK_OVERRIDE { fTexParams.invalidate(); }

    // These functions are used to track the texture parameters associated with the texture.
    const TexParams& getCachedTexParams(GrGpu::ResetTimestamp* timestamp) const {
        *timestamp = fTexParamsTimestamp;
        return fTexParams;
    }

    void setCachedTexParams(const TexParams& texParams,
                            GrGpu::ResetTimestamp timestamp) {
        fTexParams = texParams;
        fTexParamsTimestamp = timestamp;
    }

    GrGLuint textureID() const { return (fTexIDObj.get()) ? fTexIDObj->id() : 0; }

protected:
    // The public constructor registers this object with the cache. However, only the most derived
    // class should register with the cache. This constructor does not do the registration and
    // rather moves that burden onto the derived class.
    enum Derived { kDerived };
    GrGLTexture(GrGpuGL*, const GrSurfaceDesc&, const IDDesc&, Derived);

    void init(const GrSurfaceDesc&, const IDDesc&);

    virtual void onAbandon() SK_OVERRIDE;
    virtual void onRelease() SK_OVERRIDE;

private:
    TexParams                       fTexParams;
    GrGpu::ResetTimestamp           fTexParamsTimestamp;
    SkAutoTUnref<GrGLTexID>         fTexIDObj;

    typedef GrTexture INHERITED;
};

#endif
