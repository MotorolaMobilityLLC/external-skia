/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlPipelineStateBuilder.h"

#include "include/gpu/GrDirectContext.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAutoLocaleSetter.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrPersistentCacheUtils.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrShaderUtils.h"

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlPipelineState.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#import <simd/simd.h>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlPipelineState* GrMtlPipelineStateBuilder::CreatePipelineState(
                                                                GrMtlGpu* gpu,
                                                                const GrProgramDesc& desc,
                                                                const GrProgramInfo& programInfo) {
    GrAutoLocaleSetter als("C");
    GrMtlPipelineStateBuilder builder(gpu, desc, programInfo);

    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }
    return builder.finalize(desc, programInfo);
}

GrMtlPipelineStateBuilder::GrMtlPipelineStateBuilder(GrMtlGpu* gpu,
                                                     const GrProgramDesc& desc,
                                                     const GrProgramInfo& programInfo)
        : INHERITED(nullptr, desc, programInfo)
        , fGpu(gpu)
        , fUniformHandler(this)
        , fVaryingHandler(this) {
}

const GrCaps* GrMtlPipelineStateBuilder::caps() const {
    return fGpu->caps();
}

SkSL::Compiler* GrMtlPipelineStateBuilder::shaderCompiler() const {
    return fGpu->shaderCompiler();
}

void GrMtlPipelineStateBuilder::finalizeFragmentOutputColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 0");
}

void GrMtlPipelineStateBuilder::finalizeFragmentSecondaryColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 1");
}

static constexpr SkFourByteTag kMSL_Tag = SkSetFourByteTag('M', 'S', 'L', ' ');
static constexpr SkFourByteTag kSKSL_Tag = SkSetFourByteTag('S', 'K', 'S', 'L');


void GrMtlPipelineStateBuilder::storeShadersInCache(const SkSL::String shaders[],
                                                    const SkSL::Program::Inputs inputs[],
                                                    SkSL::Program::Settings* settings,
                                                    bool isSkSL) {
    // Here we shear off the Mtl-specific portion of the Desc in order to create the
    // persistent key. This is because Mtl only caches the MSL code, not the fully compiled
    // program, and that only depends on the base GrProgramDesc data.
    sk_sp<SkData> key = SkData::MakeWithoutCopy(this->desc().asKey(),
                                                this->desc().initialKeyLength());
    const SkString& description = this->desc().description();
    sk_sp<SkData> data;
    if (isSkSL) {
        // source cache, plus metadata to allow for a complete precompile
        GrPersistentCacheUtils::ShaderMetadata meta;
        meta.fSettings = settings;
        data = GrPersistentCacheUtils::PackCachedShaders(kSKSL_Tag, shaders, inputs,
                                                         kGrShaderTypeCount, &meta);
    } else {
        data = GrPersistentCacheUtils::PackCachedShaders(kMSL_Tag, shaders, inputs,
                                                         kGrShaderTypeCount);
    }
    fGpu->getContext()->priv().getPersistentCache()->store(*key, *data, description);
}

id<MTLLibrary> GrMtlPipelineStateBuilder::compileMtlShaderLibrary(
        const SkSL::String& shader, SkSL::Program::Inputs inputs,
        GrContextOptions::ShaderErrorHandler* errorHandler) {
    id<MTLLibrary> shaderLibrary = GrCompileMtlShaderLibrary(fGpu, shader, errorHandler);
    if (shaderLibrary != nil && inputs.fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }
    return shaderLibrary;
}

static inline MTLVertexFormat attribute_type_to_mtlformat(GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
            return MTLVertexFormatFloat;
        case kFloat2_GrVertexAttribType:
            return MTLVertexFormatFloat2;
        case kFloat3_GrVertexAttribType:
            return MTLVertexFormatFloat3;
        case kFloat4_GrVertexAttribType:
            return MTLVertexFormatFloat4;
        case kHalf_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatHalf;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kHalf2_GrVertexAttribType:
            return MTLVertexFormatHalf2;
        case kHalf4_GrVertexAttribType:
            return MTLVertexFormatHalf4;
        case kInt2_GrVertexAttribType:
            return MTLVertexFormatInt2;
        case kInt3_GrVertexAttribType:
            return MTLVertexFormatInt3;
        case kInt4_GrVertexAttribType:
            return MTLVertexFormatInt4;
        case kByte_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kByte2_GrVertexAttribType:
            return MTLVertexFormatChar2;
        case kByte4_GrVertexAttribType:
            return MTLVertexFormatChar4;
        case kUByte_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kUByte2_GrVertexAttribType:
            return MTLVertexFormatUChar2;
        case kUByte4_GrVertexAttribType:
            return MTLVertexFormatUChar4;
        case kUByte_norm_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUCharNormalized;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kUByte4_norm_GrVertexAttribType:
            return MTLVertexFormatUChar4Normalized;
        case kShort2_GrVertexAttribType:
            return MTLVertexFormatShort2;
        case kShort4_GrVertexAttribType:
            return MTLVertexFormatShort4;
        case kUShort2_GrVertexAttribType:
            return MTLVertexFormatUShort2;
        case kUShort2_norm_GrVertexAttribType:
            return MTLVertexFormatUShort2Normalized;
        case kInt_GrVertexAttribType:
            return MTLVertexFormatInt;
        case kUint_GrVertexAttribType:
            return MTLVertexFormatUInt;
        case kUShort_norm_GrVertexAttribType:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUShortNormalized;
            } else {
                return MTLVertexFormatInvalid;
            }
        case kUShort4_norm_GrVertexAttribType:
            return MTLVertexFormatUShort4Normalized;
    }
    SK_ABORT("Unknown vertex attribute type");
}

static MTLVertexDescriptor* create_vertex_descriptor(const GrPrimitiveProcessor& primProc) {
    uint32_t vertexBinding = 0, instanceBinding = 0;

    int nextBinding = GrMtlUniformHandler::kLastUniformBinding + 1;
    if (primProc.hasVertexAttributes()) {
        vertexBinding = nextBinding++;
    }

    if (primProc.hasInstanceAttributes()) {
        instanceBinding = nextBinding;
    }

    auto vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    int attributeIndex = 0;

    int vertexAttributeCount = primProc.numVertexAttributes();
    size_t vertexAttributeOffset = 0;
    for (const auto& attribute : primProc.vertexAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        mtlAttribute.format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != mtlAttribute.format);
        mtlAttribute.offset = vertexAttributeOffset;
        mtlAttribute.bufferIndex = vertexBinding;

        vertexAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(vertexAttributeOffset == primProc.vertexStride());

    if (vertexAttributeCount) {
        MTLVertexBufferLayoutDescriptor* vertexBufferLayout =
                vertexDescriptor.layouts[vertexBinding];
        vertexBufferLayout.stepFunction = MTLVertexStepFunctionPerVertex;
        vertexBufferLayout.stepRate = 1;
        vertexBufferLayout.stride = vertexAttributeOffset;
    }

    int instanceAttributeCount = primProc.numInstanceAttributes();
    size_t instanceAttributeOffset = 0;
    for (const auto& attribute : primProc.instanceAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        mtlAttribute.format = attribute_type_to_mtlformat(attribute.cpuType());
        mtlAttribute.offset = instanceAttributeOffset;
        mtlAttribute.bufferIndex = instanceBinding;

        instanceAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(instanceAttributeOffset == primProc.instanceStride());

    if (instanceAttributeCount) {
        MTLVertexBufferLayoutDescriptor* instanceBufferLayout =
                vertexDescriptor.layouts[instanceBinding];
        instanceBufferLayout.stepFunction = MTLVertexStepFunctionPerInstance;
        instanceBufferLayout.stepRate = 1;
        instanceBufferLayout.stride = instanceAttributeOffset;
    }
    return vertexDescriptor;
}

static MTLBlendFactor blend_coeff_to_mtl_blend(GrBlendCoeff coeff) {
    switch (coeff) {
        case kZero_GrBlendCoeff:
            return MTLBlendFactorZero;
        case kOne_GrBlendCoeff:
            return MTLBlendFactorOne;
        case kSC_GrBlendCoeff:
            return MTLBlendFactorSourceColor;
        case kISC_GrBlendCoeff:
            return MTLBlendFactorOneMinusSourceColor;
        case kDC_GrBlendCoeff:
            return MTLBlendFactorDestinationColor;
        case kIDC_GrBlendCoeff:
            return MTLBlendFactorOneMinusDestinationColor;
        case kSA_GrBlendCoeff:
            return MTLBlendFactorSourceAlpha;
        case kISA_GrBlendCoeff:
            return MTLBlendFactorOneMinusSourceAlpha;
        case kDA_GrBlendCoeff:
            return MTLBlendFactorDestinationAlpha;
        case kIDA_GrBlendCoeff:
            return MTLBlendFactorOneMinusDestinationAlpha;
        case kConstC_GrBlendCoeff:
            return MTLBlendFactorBlendColor;
        case kIConstC_GrBlendCoeff:
            return MTLBlendFactorOneMinusBlendColor;
        case kS2C_GrBlendCoeff:
            if (@available(macOS 10.12, iOS 11.0, *)) {
                return MTLBlendFactorSource1Color;
            } else {
                return MTLBlendFactorZero;
            }
        case kIS2C_GrBlendCoeff:
            if (@available(macOS 10.12, iOS 11.0, *)) {
                return MTLBlendFactorOneMinusSource1Color;
            } else {
                return MTLBlendFactorZero;
            }
        case kS2A_GrBlendCoeff:
            if (@available(macOS 10.12, iOS 11.0, *)) {
                return MTLBlendFactorSource1Alpha;
            } else {
                return MTLBlendFactorZero;
            }
        case kIS2A_GrBlendCoeff:
            if (@available(macOS 10.12, iOS 11.0, *)) {
                return MTLBlendFactorOneMinusSource1Alpha;
            } else {
                return MTLBlendFactorZero;
            }
        case kIllegal_GrBlendCoeff:
            return MTLBlendFactorZero;
    }

    SK_ABORT("Unknown blend coefficient");
}

static MTLBlendOperation blend_equation_to_mtl_blend_op(GrBlendEquation equation) {
    static const MTLBlendOperation gTable[] = {
        MTLBlendOperationAdd,              // kAdd_GrBlendEquation
        MTLBlendOperationSubtract,         // kSubtract_GrBlendEquation
        MTLBlendOperationReverseSubtract,  // kReverseSubtract_GrBlendEquation
    };
    static_assert(SK_ARRAY_COUNT(gTable) == kFirstAdvancedGrBlendEquation);
    static_assert(0 == kAdd_GrBlendEquation);
    static_assert(1 == kSubtract_GrBlendEquation);
    static_assert(2 == kReverseSubtract_GrBlendEquation);

    SkASSERT((unsigned)equation < kGrBlendEquationCnt);
    return gTable[equation];
}

static MTLRenderPipelineColorAttachmentDescriptor* create_color_attachment(
        MTLPixelFormat format, const GrPipeline& pipeline) {
    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    // pixel format
    mtlColorAttachment.pixelFormat = format;

    // blending
    const GrXferProcessor::BlendInfo& blendInfo = pipeline.getXferProcessor().getBlendInfo();

    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = GrBlendShouldDisable(equation, srcCoeff, dstCoeff);

    mtlColorAttachment.blendingEnabled = !blendOff;
    if (!blendOff) {
        mtlColorAttachment.sourceRGBBlendFactor = blend_coeff_to_mtl_blend(srcCoeff);
        mtlColorAttachment.destinationRGBBlendFactor = blend_coeff_to_mtl_blend(dstCoeff);
        mtlColorAttachment.rgbBlendOperation = blend_equation_to_mtl_blend_op(equation);
        mtlColorAttachment.sourceAlphaBlendFactor = blend_coeff_to_mtl_blend(srcCoeff);
        mtlColorAttachment.destinationAlphaBlendFactor = blend_coeff_to_mtl_blend(dstCoeff);
        mtlColorAttachment.alphaBlendOperation = blend_equation_to_mtl_blend_op(equation);
    }

    if (!blendInfo.fWriteColor) {
        mtlColorAttachment.writeMask = MTLColorWriteMaskNone;
    } else {
        mtlColorAttachment.writeMask = MTLColorWriteMaskAll;
    }
    return mtlColorAttachment;
}

uint32_t buffer_size(uint32_t offset, uint32_t maxAlignment) {
    // Metal expects the buffer to be padded at the end according to the alignment
    // of the largest element in the buffer.
    uint32_t offsetDiff = offset & maxAlignment;
    if (offsetDiff != 0) {
        offsetDiff = maxAlignment - offsetDiff + 1;
    }
    return offset + offsetDiff;
}

GrMtlPipelineState* GrMtlPipelineStateBuilder::finalize(const GrProgramDesc& desc,
                                                        const GrProgramInfo& programInfo) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    // Geometry shaders are not supported
    SkASSERT(!this->primitiveProcessor().willUseGeoShader());

    auto pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    id<MTLLibrary> shaderLibraries[kGrShaderTypeCount];

    fVS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fVS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");

    this->finalizeShaders();

    SkSL::Program::Settings settings;
    settings.fFlipY = this->origin() != kTopLeft_GrSurfaceOrigin;
    settings.fSharpenTextures = fGpu->getContext()->priv().options().fSharpenMipmappedTextures;
    SkASSERT(!this->fragColorIsInOut());

    sk_sp<SkData> cached;
    SkReadBuffer reader;
    SkFourByteTag shaderType = 0;
    auto persistentCache = fGpu->getContext()->priv().getPersistentCache();
    if (persistentCache) {
        // Here we shear off the Mtl-specific portion of the Desc in order to create the
        // persistent key. This is because Mtl only caches the MSL code, not the fully compiled
        // program, and that only depends on the base GrProgramDesc data.
        sk_sp<SkData> key = SkData::MakeWithoutCopy(desc.asKey(), desc.initialKeyLength());
        cached = persistentCache->load(*key);
        if (cached) {
            reader.setMemory(cached->data(), cached->size());
            shaderType = GrPersistentCacheUtils::GetType(&reader);
        }
    }

    auto errorHandler = fGpu->getContext()->priv().getShaderErrorHandler();
    SkSL::String msl[kGrShaderTypeCount];
    SkSL::Program::Inputs inputs[kGrShaderTypeCount];

    // Unpack any stored shaders from the persistent cache
    if (cached) {
        switch (shaderType) {
            case kMSL_Tag: {
                GrPersistentCacheUtils::UnpackCachedShaders(&reader, msl, inputs,
                                                            kGrShaderTypeCount);
                break;
            }

            case kSKSL_Tag: {
                SkSL::String cached_sksl[kGrShaderTypeCount];
                if (GrPersistentCacheUtils::UnpackCachedShaders(&reader, cached_sksl, inputs,
                                                                kGrShaderTypeCount)) {
                    bool success = GrSkSLToMSL(fGpu,
                                               cached_sksl[kVertex_GrShaderType],
                                               SkSL::ProgramKind::kVertex,
                                               settings,
                                               &msl[kVertex_GrShaderType],
                                               &inputs[kVertex_GrShaderType],
                                               errorHandler);
                    success = success && GrSkSLToMSL(fGpu,
                                                     cached_sksl[kFragment_GrShaderType],
                                                     SkSL::ProgramKind::kFragment,
                                                     settings,
                                                     &msl[kFragment_GrShaderType],
                                                     &inputs[kFragment_GrShaderType],
                                                     errorHandler);
                    if (!success) {
                        return nullptr;
                    }
                }
                break;
            }

            default: {
                break;
            }
        }
    }

    // Create any MSL shaders from pipeline data if necessary and cache
    if (msl[kVertex_GrShaderType].empty() || msl[kFragment_GrShaderType].empty()) {
        bool success = true;
        if (msl[kVertex_GrShaderType].empty()) {
            success = GrSkSLToMSL(fGpu,
                                  fVS.fCompilerString,
                                  SkSL::ProgramKind::kVertex,
                                  settings,
                                  &msl[kVertex_GrShaderType],
                                  &inputs[kVertex_GrShaderType],
                                  errorHandler);
        }
        if (success && msl[kFragment_GrShaderType].empty()) {
            success = GrSkSLToMSL(fGpu,
                                  fFS.fCompilerString,
                                  SkSL::ProgramKind::kFragment,
                                  settings,
                                  &msl[kFragment_GrShaderType],
                                  &inputs[kFragment_GrShaderType],
                                  errorHandler);
        }
        if (!success) {
            return nullptr;
        }

        if (persistentCache && !cached) {
            if (fGpu->getContext()->priv().options().fShaderCacheStrategy ==
                    GrContextOptions::ShaderCacheStrategy::kSkSL) {
                SkSL::String sksl[kGrShaderTypeCount];
                sksl[kVertex_GrShaderType] = GrShaderUtils::PrettyPrint(fVS.fCompilerString);
                sksl[kFragment_GrShaderType] = GrShaderUtils::PrettyPrint(fFS.fCompilerString);
                this->storeShadersInCache(sksl, inputs, &settings, true);
            } else {
                this->storeShadersInCache(msl, inputs, nullptr, false);
            }
        }
    }

    // Compile MSL to libraries
    shaderLibraries[kVertex_GrShaderType] = this->compileMtlShaderLibrary(
                                                    msl[kVertex_GrShaderType],
                                                    inputs[kVertex_GrShaderType],
                                                    errorHandler);
    shaderLibraries[kFragment_GrShaderType] = this->compileMtlShaderLibrary(
                                                    msl[kFragment_GrShaderType],
                                                    inputs[kFragment_GrShaderType],
                                                    errorHandler);
    if (!shaderLibraries[kVertex_GrShaderType] || !shaderLibraries[kFragment_GrShaderType]) {
        return nullptr;
    }

    id<MTLFunction> vertexFunction =
            [shaderLibraries[kVertex_GrShaderType] newFunctionWithName: @"vertexMain"];
    id<MTLFunction> fragmentFunction =
            [shaderLibraries[kFragment_GrShaderType] newFunctionWithName: @"fragmentMain"];

    if (vertexFunction == nil) {
        SkDebugf("Couldn't find vertexMain() in library\n");
        return nullptr;
    }
    if (fragmentFunction == nil) {
        SkDebugf("Couldn't find fragmentMain() in library\n");
        return nullptr;
    }

    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.vertexDescriptor = create_vertex_descriptor(programInfo.primProc());

    MTLPixelFormat pixelFormat = GrBackendFormatAsMTLPixelFormat(programInfo.backendFormat());
    if (pixelFormat == MTLPixelFormatInvalid) {
        return nullptr;
    }

    pipelineDescriptor.colorAttachments[0] = create_color_attachment(pixelFormat,
                                                                     programInfo.pipeline());
    pipelineDescriptor.sampleCount = programInfo.numRasterSamples();
    bool hasStencilAttachment = programInfo.isStencilEnabled();
    GrMtlCaps* mtlCaps = (GrMtlCaps*)this->caps();
    pipelineDescriptor.stencilAttachmentPixelFormat =
        hasStencilAttachment ? mtlCaps->preferredStencilFormat() : MTLPixelFormatInvalid;

    SkASSERT(pipelineDescriptor.vertexFunction);
    SkASSERT(pipelineDescriptor.fragmentFunction);
    SkASSERT(pipelineDescriptor.vertexDescriptor);
    SkASSERT(pipelineDescriptor.colorAttachments[0]);

    NSError* error = nil;
#if GR_METAL_SDK_VERSION >= 230
    if (@available(macOS 11.0, iOS 14.0, *)) {
        id<MTLBinaryArchive> archive = fGpu->binaryArchive();
        if (archive) {
            NSArray* archiveArray = [NSArray arrayWithObjects:archive, nil];
            pipelineDescriptor.binaryArchives = archiveArray;
            BOOL result;
            {
                TRACE_EVENT0("skia.gpu", "addRenderPipelineFunctionsWithDescriptor");
                result = [archive addRenderPipelineFunctionsWithDescriptor: pipelineDescriptor
                                                                            error: &error];
            }
            if (!result && error) {
                SkDebugf("Error storing pipeline: %s\n",
                        [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
            }
        }
    }
#endif
    id<MTLRenderPipelineState> pipelineState;
    {
        TRACE_EVENT0("skia.gpu", "newRenderPipelineStateWithDescriptor");
#if defined(SK_BUILD_FOR_MAC)
        pipelineState = GrMtlNewRenderPipelineStateWithDescriptor(
                                                     fGpu->device(), pipelineDescriptor, &error);
#else
        pipelineState =
            [fGpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                           error: &error];
#endif
    }
    if (error) {
        SkDebugf("Error creating pipeline: %s\n",
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
        return nullptr;
    }
    if (!pipelineState) {
        return nullptr;
    }

    uint32_t bufferSize = buffer_size(fUniformHandler.fCurrentUBOOffset,
                                      fUniformHandler.fCurrentUBOMaxAlignment);
    return new GrMtlPipelineState(fGpu,
                                  pipelineState,
                                  pipelineDescriptor.colorAttachments[0].pixelFormat,
                                  fUniformHandles,
                                  fUniformHandler.fUniforms,
                                  bufferSize,
                                  (uint32_t)fUniformHandler.numSamplers(),
                                  std::move(fGeometryProcessor),
                                  std::move(fXferProcessor),
                                  std::move(fFPImpls));
}

//////////////////////////////////////////////////////////////////////////////

bool GrMtlPipelineStateBuilder::PrecompileShaders(GrMtlGpu* gpu, const SkData& cachedData) {
    SkReadBuffer reader(cachedData.data(), cachedData.size());
    SkFourByteTag shaderType = GrPersistentCacheUtils::GetType(&reader);

    auto errorHandler = gpu->getContext()->priv().getShaderErrorHandler();

    SkSL::Program::Settings settings;
    settings.fSharpenTextures = gpu->getContext()->priv().options().fSharpenMipmappedTextures;
    GrPersistentCacheUtils::ShaderMetadata meta;
    meta.fSettings = &settings;

    SkSL::String shaders[kGrShaderTypeCount];
    SkSL::Program::Inputs inputs[kGrShaderTypeCount];
    if (!GrPersistentCacheUtils::UnpackCachedShaders(&reader, shaders, inputs, kGrShaderTypeCount,
                                                     &meta)) {
        return false;
    }

    switch (shaderType) {
        case kMSL_Tag: {
            GrPrecompileMtlShaderLibrary(gpu, shaders[kVertex_GrShaderType]);
            GrPrecompileMtlShaderLibrary(gpu, shaders[kFragment_GrShaderType]);
            break;
        }

        case kSKSL_Tag: {
            SkSL::String mslShaders[kGrShaderTypeCount];
            if (!GrSkSLToMSL(gpu,
                           shaders[kVertex_GrShaderType],
                           SkSL::ProgramKind::kVertex,
                           settings,
                           &mslShaders[kVertex_GrShaderType],
                           &inputs[kVertex_GrShaderType],
                           errorHandler)) {
                return false;
            }
            if (!GrSkSLToMSL(gpu,
                           shaders[kFragment_GrShaderType],
                           SkSL::ProgramKind::kFragment,
                           settings,
                           &mslShaders[kFragment_GrShaderType],
                           &inputs[kFragment_GrShaderType],
                           errorHandler)) {
                return false;
            }
            GrPrecompileMtlShaderLibrary(gpu, mslShaders[kVertex_GrShaderType]);
            GrPrecompileMtlShaderLibrary(gpu, mslShaders[kFragment_GrShaderType]);
            break;
        }

        default: {
            return false;
        }
    }

    return true;
}
