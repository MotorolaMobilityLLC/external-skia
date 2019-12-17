/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_Base_DEFINED
#define GrContext_Base_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrTypes.h"

class GrBaseContextPriv;
class GrCaps;
class GrContext;
class GrImageContext;
class GrRecordingContext;

class GrContext_Base : public SkRefCnt {
public:
    virtual ~GrContext_Base();

    /*
     * The 3D API backing this context
     */
    SK_API GrBackendApi backend() const { return fBackend; }

    /*
     * Retrieve the default GrBackendFormat for a given SkColorType and renderability.
     * It is guaranteed that this backend format will be the one used by the GrContext
     * SkColorType and SkSurfaceCharacterization-based createBackendTexture methods.
     *
     * The caller should check that the returned format is valid.
     */
    SK_API GrBackendFormat defaultBackendFormat(SkColorType, GrRenderable) const;

    // Provides access to functions that aren't part of the public API.
    GrBaseContextPriv priv();
    const GrBaseContextPriv priv() const;

protected:
    friend class GrBaseContextPriv; // for hidden functions

    GrContext_Base(GrBackendApi backend, const GrContextOptions& options, uint32_t contextID);

    virtual bool init(sk_sp<const GrCaps>);

    /**
     * An identifier for this context. The id is used by all compatible contexts. For example,
     * if SkImages are created on one thread using an image creation context, then fed into a
     * DDL Recorder on second thread (which has a recording context) and finally replayed on
     * a third thread with a direct context, then all three contexts will report the same id.
     * It is an error for an image to be used with contexts that report different ids.
     */
    uint32_t contextID() const { return fContextID; }

    bool matches(GrContext_Base* candidate) const {
        return candidate->contextID() == this->contextID();
    }

    /*
     * The options in effect for this context
     */
    const GrContextOptions& options() const { return fOptions; }

    const GrCaps* caps() const;
    sk_sp<const GrCaps> refCaps() const;

    virtual GrImageContext* asImageContext() { return nullptr; }
    virtual GrRecordingContext* asRecordingContext() { return nullptr; }
    virtual GrContext* asDirectContext() { return nullptr; }

private:
    const GrBackendApi          fBackend;
    const GrContextOptions      fOptions;
    const uint32_t              fContextID;
    sk_sp<const GrCaps>         fCaps;

    typedef SkRefCnt INHERITED;
};

#endif
