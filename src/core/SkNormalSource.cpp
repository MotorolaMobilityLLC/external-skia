/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkError.h"
#include "SkErrorInternals.h"
#include "SkLightingShader.h"
#include "SkMatrix.h"
#include "SkNormalSource.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

// Genretating vtable
SkNormalSource::~SkNormalSource() {}

///////////////////////////////////////////////////////////////////////////////

class NormalMapSourceImpl : public SkNormalSource {
public:
    NormalMapSourceImpl(sk_sp<SkShader> mapShader, const SkMatrix& invCTM)
        : fMapShader(std::move(mapShader))
        , fInvCTM(invCTM) {}

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*,
                                                   const SkMatrix& viewM,
                                                   const SkMatrix* localMatrix,
                                                   SkFilterQuality,
                                                   SkSourceGammaTreatment) const override;
#endif

    SkNormalSource::Provider* asProvider(const SkShader::ContextRec& rec,
                                                         void* storage) const override;

    size_t providerSize(const SkShader::ContextRec& rec) const override;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(NormalMapSourceImpl)

protected:
    void flatten(SkWriteBuffer& buf) const override;

    bool computeNormTotalInverse(const SkShader::ContextRec& rec, SkMatrix* normTotalInverse) const;

private:
    class Provider : public SkNormalSource::Provider {
    public:
        Provider(const NormalMapSourceImpl& source, SkShader::Context* fMapContext);

        virtual ~Provider() override;

        void fillScanLine(int x, int y, SkPoint3 output[], int count) const override;
    private:
        const NormalMapSourceImpl& fSource;
        SkShader::Context* fMapContext;

        typedef SkNormalSource::Provider INHERITED;
    };

    sk_sp<SkShader> fMapShader;
    SkMatrix        fInvCTM; // Inverse of the canvas total matrix, used for rotating normals.

    friend class SkNormalSource;

    typedef SkNormalSource INHERITED;
};

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "GrTextureParams.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "SkGr.h"

class NormalMapFP : public GrFragmentProcessor {
public:
    NormalMapFP(sk_sp<GrFragmentProcessor> mapFP, const SkMatrix& invCTM)
        : fInvCTM(invCTM) {
        this->registerChildProcessor(mapFP);

        this->initClassID<NormalMapFP>();
    }

    class GLSLNormalMapFP : public GrGLSLFragmentProcessor {
    public:
        GLSLNormalMapFP()
            : fColumnMajorInvCTM22{0.0f} {}

        void emitCode(EmitArgs& args) override {
            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // add uniform
            const char* xformUniName = nullptr;
            fXformUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kMat22f_GrSLType,
                                                   kDefault_GrSLPrecision, "Xform", &xformUniName);

            SkString dstNormalColorName("dstNormalColor");
            this->emitChild(0, nullptr, &dstNormalColorName, args);
            fragBuilder->codeAppendf("vec3 normal = normalize(%s.rgb - vec3(0.5));",
                                     dstNormalColorName.c_str());

            // TODO: inverse map the light direction vectors in the vertex shader rather than
            // transforming all the normals here!

            // If there's no x & y components, return (0, 0, +/- 1) instead to avoid division by 0
            fragBuilder->codeAppend( "if (abs(normal.z) > 0.999) {");
            fragBuilder->codeAppendf("    %s = normalize(vec4(0.0, 0.0, normal.z, 0.0));",
                    args.fOutputColor);
            // Else, Normalizing the transformed X and Y, while keeping constant both Z and the
            // vector's angle in the XY plane. This maintains the "slope" for the surface while
            // appropriately rotating the normal for any anisotropic scaling that occurs.
            // Here, we call scaling factor the number that must divide the transformed X and Y so
            // that the normal's length remains equal to 1.
            fragBuilder->codeAppend( "} else {");
            fragBuilder->codeAppendf("    vec2 transformed = %s * normal.xy;",
                    xformUniName);
            fragBuilder->codeAppend( "    float scalingFactorSquared = "
                                                 "( (transformed.x * transformed.x) "
                                                   "+ (transformed.y * transformed.y) )"
                                                 "/(1.0 - (normal.z * normal.z));");
            fragBuilder->codeAppendf("    %s = vec4(transformed*inversesqrt(scalingFactorSquared),"
                                                   "normal.z, 0.0);",
                    args.fOutputColor);
            fragBuilder->codeAppend( "}");
        }

        static void GenKey(const GrProcessor& proc, const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
            b->add32(0x0);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {
            const NormalMapFP& normalMapFP = proc.cast<NormalMapFP>();

            const SkMatrix& invCTM = normalMapFP.invCTM();
            fColumnMajorInvCTM22[0] = invCTM.get(SkMatrix::kMScaleX);
            fColumnMajorInvCTM22[1] = invCTM.get(SkMatrix::kMSkewY);
            fColumnMajorInvCTM22[2] = invCTM.get(SkMatrix::kMSkewX);
            fColumnMajorInvCTM22[3] = invCTM.get(SkMatrix::kMScaleY);
            pdman.setMatrix2f(fXformUni, fColumnMajorInvCTM22);
        }

    private:
        float fColumnMajorInvCTM22[4];
        GrGLSLProgramDataManager::UniformHandle fXformUni;
    };

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLNormalMapFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "NormalMapFP"; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::ReadInput::kWillNot_ReadInput);
    }

    const SkMatrix& invCTM() const { return fInvCTM; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLSLNormalMapFP; }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const NormalMapFP& normalMapFP = proc.cast<NormalMapFP>();
        return fInvCTM == normalMapFP.fInvCTM;
    }

    SkMatrix fInvCTM;
};

sk_sp<GrFragmentProcessor> NormalMapSourceImpl::asFragmentProcessor(
                                                     GrContext *context,
                                                     const SkMatrix &viewM,
                                                     const SkMatrix *localMatrix,
                                                     SkFilterQuality filterQuality,
                                                     SkSourceGammaTreatment gammaTreatment) const {
    sk_sp<GrFragmentProcessor> mapFP = fMapShader->asFragmentProcessor(context, viewM,
            localMatrix, filterQuality, gammaTreatment);
    if (!mapFP) {
        return nullptr;
    }

    return sk_make_sp<NormalMapFP>(std::move(mapFP), fInvCTM);
}

#endif // SK_SUPPORT_GPU

////////////////////////////////////////////////////////////////////////////

NormalMapSourceImpl::Provider::Provider(const NormalMapSourceImpl& source,
                                        SkShader::Context* mapContext)
    : fSource(source)
    , fMapContext(mapContext) {
}

NormalMapSourceImpl::Provider::~Provider() {
    fMapContext->~Context();
}

SkNormalSource::Provider* NormalMapSourceImpl::asProvider(
        const SkShader::ContextRec &rec, void *storage) const {
    SkMatrix normTotalInv;
    if (!this->computeNormTotalInverse(rec, &normTotalInv)) {
        return nullptr;
    }

    void* mapContextStorage = (char*)storage + sizeof(Provider);
    SkShader::Context* context = fMapShader->createContext(rec, mapContextStorage);
    if (!context) {
        return nullptr;
    }

    return new (storage) Provider(*this, context);
}

size_t NormalMapSourceImpl::providerSize(const SkShader::ContextRec& rec) const {
    return sizeof(Provider) + fMapShader->contextSize(rec);
}

bool NormalMapSourceImpl::computeNormTotalInverse(const SkShader::ContextRec& rec,
                                                  SkMatrix* normTotalInverse) const {
    SkMatrix total;
    total.setConcat(*rec.fMatrix, fMapShader->getLocalMatrix());

    const SkMatrix* m = &total;
    if (rec.fLocalMatrix) {
        total.setConcat(*m, *rec.fLocalMatrix);
        m = &total;
    }
    return m->invert(normTotalInverse);
}

#define BUFFER_MAX 16
void NormalMapSourceImpl::Provider::fillScanLine(int x, int y, SkPoint3 output[],
                                                 int count) const {
    SkPMColor tmpNormalColors[BUFFER_MAX];

    do {
        int n = SkTMin(count, BUFFER_MAX);

        fMapContext->shadeSpan(x, y, tmpNormalColors, n);

        for (int i = 0; i < n; i++) {
            SkPoint3 tempNorm;

            tempNorm.set(SkIntToScalar(SkGetPackedR32(tmpNormalColors[i])) - 127.0f,
                         SkIntToScalar(SkGetPackedG32(tmpNormalColors[i])) - 127.0f,
                         SkIntToScalar(SkGetPackedB32(tmpNormalColors[i])) - 127.0f);
            tempNorm.normalize();

            if (!SkScalarNearlyEqual(SkScalarAbs(tempNorm.fZ), 1.0f)) {
                SkVector transformed = fSource.fInvCTM.mapVector(tempNorm.fX, tempNorm.fY);

                // Normalizing the transformed X and Y, while keeping constant both Z and the
                // vector's angle in the XY plane. This maintains the "slope" for the surface while
                // appropriately rotating the normal for any anisotropic scaling that occurs.
                // Here, we call scaling factor the number that must divide the transformed X and Y
                // so that the normal's length remains equal to 1.
                SkScalar scalingFactorSquared =
                        (SkScalarSquare(transformed.fX) + SkScalarSquare(transformed.fY))
                        / (1.0f - SkScalarSquare(tempNorm.fZ));
                SkScalar invScalingFactor = SkScalarInvert(SkScalarSqrt(scalingFactorSquared));

                output[i].fX = transformed.fX * invScalingFactor;
                output[i].fY = transformed.fY * invScalingFactor;
                output[i].fZ = tempNorm.fZ;
            } else {
                output[i] = {0.0f, 0.0f, tempNorm.fZ};
                output[i].normalize();
            }

            SkASSERT(SkScalarNearlyEqual(output[i].length(), 1.0f))
        }

        output += n;
        x += n;
        count -= n;
    } while (count > 0);
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> NormalMapSourceImpl::CreateProc(SkReadBuffer& buf) {

    sk_sp<SkShader> mapShader = buf.readFlattenable<SkShader>();

    SkMatrix invCTM;
    buf.readMatrix(&invCTM);

    return sk_make_sp<NormalMapSourceImpl>(std::move(mapShader), invCTM);
}

void NormalMapSourceImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    buf.writeFlattenable(fMapShader.get());
    buf.writeMatrix(fInvCTM);
}

////////////////////////////////////////////////////////////////////////////

sk_sp<SkNormalSource> SkNormalSource::MakeFromNormalMap(sk_sp<SkShader> map, const SkMatrix& ctm) {
    SkMatrix invCTM;

    if (!ctm.invert(&invCTM) || !map) {
        return nullptr;
    }

    return sk_make_sp<NormalMapSourceImpl>(std::move(map), invCTM);
}

////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkNormalSource)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(NormalMapSourceImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

////////////////////////////////////////////////////////////////////////////
