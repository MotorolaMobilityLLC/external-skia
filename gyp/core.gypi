# Include this gypi to include all 'core' files
# The parent gyp/gypi file must define
#       'skia_src_path'     e.g. skia/trunk/src
#       'skia_include_path' e.g. skia/trunk/include
#
# The skia build defines these in common_variables.gypi
#
{
    'sources': [
        '<(skia_src_path)/c/sk_paint.cpp',
        '<(skia_src_path)/c/sk_surface.cpp',
        '<(skia_src_path)/c/sk_types_priv.h',

        '<(skia_src_path)/core/SkAAClip.cpp',
        '<(skia_src_path)/core/SkAnnotation.cpp',
        '<(skia_src_path)/core/SkAdvancedTypefaceMetrics.cpp',
        '<(skia_src_path)/core/SkAlphaRuns.cpp',
        '<(skia_src_path)/core/SkAntiRun.h',
        '<(skia_src_path)/core/SkBBHFactory.cpp',
        '<(skia_src_path)/core/SkBBoxHierarchy.h',
        '<(skia_src_path)/core/SkBitmap.cpp',
        '<(skia_src_path)/core/SkBitmapCache.cpp',
        '<(skia_src_path)/core/SkBitmapDevice.cpp',
        '<(skia_src_path)/core/SkBitmapFilter.h',
        '<(skia_src_path)/core/SkBitmapFilter.cpp',
        '<(skia_src_path)/core/SkBitmapHeap.cpp',
        '<(skia_src_path)/core/SkBitmapHeap.h',
        '<(skia_src_path)/core/SkBitmapProcShader.cpp',
        '<(skia_src_path)/core/SkBitmapProcShader.h',
        '<(skia_src_path)/core/SkBitmapProcState.cpp',
        '<(skia_src_path)/core/SkBitmapProcState.h',
        '<(skia_src_path)/core/SkBitmapProcState_matrix.h',
        '<(skia_src_path)/core/SkBitmapProcState_matrixProcs.cpp',
        '<(skia_src_path)/core/SkBitmapProcState_sample.h',
        '<(skia_src_path)/core/SkBitmapScaler.h',
        '<(skia_src_path)/core/SkBitmapScaler.cpp',
        '<(skia_src_path)/core/SkBitmap_scroll.cpp',
        '<(skia_src_path)/core/SkBlitBWMaskTemplate.h',
        '<(skia_src_path)/core/SkBlitMask_D32.cpp',
        '<(skia_src_path)/core/SkBlitRow_D16.cpp',
        '<(skia_src_path)/core/SkBlitRow_D32.cpp',
        '<(skia_src_path)/core/SkBlitter.h',
        '<(skia_src_path)/core/SkBlitter.cpp',
        '<(skia_src_path)/core/SkBlitter_A8.cpp',
        '<(skia_src_path)/core/SkBlitter_ARGB32.cpp',
        '<(skia_src_path)/core/SkBlitter_RGB16.cpp',
        '<(skia_src_path)/core/SkBlitter_Sprite.cpp',
        '<(skia_src_path)/core/SkBuffer.cpp',
        '<(skia_src_path)/core/SkCachedData.cpp',
        '<(skia_src_path)/core/SkCanvas.cpp',
        '<(skia_src_path)/core/SkCanvasDrawable.cpp',
        '<(skia_src_path)/core/SkCanvasDrawable.h',
        '<(skia_src_path)/core/SkChunkAlloc.cpp',
        '<(skia_src_path)/core/SkClipStack.cpp',
        '<(skia_src_path)/core/SkColor.cpp',
        '<(skia_src_path)/core/SkColorFilter.cpp',
        '<(skia_src_path)/core/SkColorShader.h',
        '<(skia_src_path)/core/SkColorTable.cpp',
        '<(skia_src_path)/core/SkComposeShader.cpp',
        '<(skia_src_path)/core/SkConfig8888.cpp',
        '<(skia_src_path)/core/SkConfig8888.h',
        '<(skia_src_path)/core/SkConvolver.cpp',
        '<(skia_src_path)/core/SkConvolver.h',
        '<(skia_src_path)/core/SkCoreBlitters.h',
        '<(skia_src_path)/core/SkCubicClipper.cpp',
        '<(skia_src_path)/core/SkCubicClipper.h',
        '<(skia_src_path)/core/SkData.cpp',
        '<(skia_src_path)/core/SkDataTable.cpp',
        '<(skia_src_path)/core/SkDebug.cpp',
        '<(skia_src_path)/core/SkDeque.cpp',
        '<(skia_src_path)/core/SkDevice.cpp',
        '<(skia_src_path)/core/SkDeviceLooper.cpp',
        '<(skia_src_path)/core/SkDeviceProfile.cpp',
        '<(skia_src_path)/core/SkDeviceProperties.h',
        '<(skia_src_path)/lazy/SkDiscardableMemoryPool.cpp',
        '<(skia_src_path)/lazy/SkDiscardablePixelRef.cpp',
        '<(skia_src_path)/core/SkDistanceFieldGen.cpp',
        '<(skia_src_path)/core/SkDistanceFieldGen.h',
        '<(skia_src_path)/core/SkDither.cpp',
        '<(skia_src_path)/core/SkDraw.cpp',
        '<(skia_src_path)/core/SkDrawLooper.cpp',
        '<(skia_src_path)/core/SkDrawProcs.h',
        '<(skia_src_path)/core/SkEdgeBuilder.cpp',
        '<(skia_src_path)/core/SkEdgeClipper.cpp',
        '<(skia_src_path)/core/SkEdge.cpp',
        '<(skia_src_path)/core/SkEdge.h',
        '<(skia_src_path)/core/SkError.cpp',
        '<(skia_src_path)/core/SkErrorInternals.h',
        '<(skia_src_path)/core/SkFilterProc.cpp',
        '<(skia_src_path)/core/SkFilterProc.h',
        '<(skia_src_path)/core/SkFilterShader.cpp',
        '<(skia_src_path)/core/SkFlattenable.cpp',
        '<(skia_src_path)/core/SkFlattenableSerialization.cpp',
        '<(skia_src_path)/core/SkFloatBits.cpp',
        '<(skia_src_path)/core/SkFont.cpp',
        '<(skia_src_path)/core/SkFontHost.cpp',
        '<(skia_src_path)/core/SkFontMgr.cpp',
        '<(skia_src_path)/core/SkFontStyle.cpp',
        '<(skia_src_path)/core/SkFontDescriptor.cpp',
        '<(skia_src_path)/core/SkFontDescriptor.h',
        '<(skia_src_path)/core/SkFontStream.cpp',
        '<(skia_src_path)/core/SkFontStream.h',
        '<(skia_src_path)/core/SkGeometry.cpp',
        '<(skia_src_path)/core/SkGeometry.h',
        '<(skia_src_path)/core/SkGlyphCache.cpp',
        '<(skia_src_path)/core/SkGlyphCache.h',
        '<(skia_src_path)/core/SkGlyphCache_Globals.h',
        '<(skia_src_path)/core/SkGraphics.cpp',
        '<(skia_src_path)/core/SkHalf.cpp',
        '<(skia_src_path)/core/SkHalf.h',
        '<(skia_src_path)/core/SkInstCnt.cpp',
        '<(skia_src_path)/core/SkImageFilter.cpp',
        '<(skia_src_path)/core/SkImageInfo.cpp',
        '<(skia_src_path)/core/SkImageGenerator.cpp',
        '<(skia_src_path)/core/SkLayerInfo.h',
        '<(skia_src_path)/core/SkLayerInfo.cpp',
        '<(skia_src_path)/core/SkLocalMatrixShader.cpp',
        '<(skia_src_path)/core/SkLineClipper.cpp',
        '<(skia_src_path)/core/SkMallocPixelRef.cpp',
        '<(skia_src_path)/core/SkMask.cpp',
        '<(skia_src_path)/core/SkMaskCache.cpp',
        '<(skia_src_path)/core/SkMaskFilter.cpp',
        '<(skia_src_path)/core/SkMaskGamma.cpp',
        '<(skia_src_path)/core/SkMaskGamma.h',
        '<(skia_src_path)/core/SkMath.cpp',
        '<(skia_src_path)/core/SkMatrix.cpp',
        '<(skia_src_path)/core/SkMessageBus.h',
        '<(skia_src_path)/core/SkMetaData.cpp',
        '<(skia_src_path)/core/SkMipMap.cpp',
        '<(skia_src_path)/core/SkMultiPictureDraw.cpp',
        '<(skia_src_path)/core/SkPackBits.cpp',
        '<(skia_src_path)/core/SkPaint.cpp',
        '<(skia_src_path)/core/SkPaintPriv.cpp',
        '<(skia_src_path)/core/SkPaintPriv.h',
        '<(skia_src_path)/core/SkPath.cpp',
        '<(skia_src_path)/core/SkPathEffect.cpp',
        '<(skia_src_path)/core/SkPathMeasure.cpp',
        '<(skia_src_path)/core/SkPathRef.cpp',
        '<(skia_src_path)/core/SkPicture.cpp',
        '<(skia_src_path)/core/SkPictureContentInfo.cpp',
        '<(skia_src_path)/core/SkPictureContentInfo.h',
        '<(skia_src_path)/core/SkPictureData.cpp',
        '<(skia_src_path)/core/SkPictureData.h',
        '<(skia_src_path)/core/SkPictureFlat.cpp',
        '<(skia_src_path)/core/SkPictureFlat.h',
        '<(skia_src_path)/core/SkPicturePlayback.cpp',
        '<(skia_src_path)/core/SkPicturePlayback.h',
        '<(skia_src_path)/core/SkPictureRecord.cpp',
        '<(skia_src_path)/core/SkPictureRecord.h',
        '<(skia_src_path)/core/SkPictureRecorder.cpp',
        '<(skia_src_path)/core/SkPictureShader.cpp',
        '<(skia_src_path)/core/SkPictureShader.h',
        '<(skia_src_path)/core/SkPixelRef.cpp',
        '<(skia_src_path)/core/SkPoint.cpp',
        '<(skia_src_path)/core/SkProcSpriteBlitter.cpp',
        '<(skia_src_path)/core/SkPtrRecorder.cpp',
        '<(skia_src_path)/core/SkQuadClipper.cpp',
        '<(skia_src_path)/core/SkQuadClipper.h',
        '<(skia_src_path)/core/SkRasterClip.cpp',
        '<(skia_src_path)/core/SkRasterizer.cpp',
        '<(skia_src_path)/core/SkReadBuffer.h',
        '<(skia_src_path)/core/SkReadBuffer.cpp',
        '<(skia_src_path)/core/SkReader32.h',
        '<(skia_src_path)/core/SkRecord.cpp',
        '<(skia_src_path)/core/SkRecordDraw.cpp',
        '<(skia_src_path)/core/SkRecordOpts.cpp',
        '<(skia_src_path)/core/SkRecorder.cpp',
        '<(skia_src_path)/core/SkRect.cpp',
        '<(skia_src_path)/core/SkRefDict.cpp',
        '<(skia_src_path)/core/SkRegion.cpp',
        '<(skia_src_path)/core/SkRegionPriv.h',
        '<(skia_src_path)/core/SkRegion_path.cpp',
        '<(skia_src_path)/core/SkResourceCache.cpp',
        '<(skia_src_path)/core/SkRRect.cpp',
        '<(skia_src_path)/core/SkRTree.h',
        '<(skia_src_path)/core/SkRTree.cpp',
        '<(skia_src_path)/core/SkScalar.cpp',
        '<(skia_src_path)/core/SkScalerContext.cpp',
        '<(skia_src_path)/core/SkScalerContext.h',
        '<(skia_src_path)/core/SkScan.cpp',
        '<(skia_src_path)/core/SkScan.h',
        '<(skia_src_path)/core/SkScanPriv.h',
        '<(skia_src_path)/core/SkScan_AntiPath.cpp',
        '<(skia_src_path)/core/SkScan_Antihair.cpp',
        '<(skia_src_path)/core/SkScan_Hairline.cpp',
        '<(skia_src_path)/core/SkScan_Path.cpp',
        '<(skia_src_path)/core/SkShader.cpp',
        '<(skia_src_path)/core/SkSpriteBlitter_ARGB32.cpp',
        '<(skia_src_path)/core/SkSpriteBlitter_RGB16.cpp',
        '<(skia_src_path)/core/SkSpriteBlitter.h',
        '<(skia_src_path)/core/SkSpriteBlitterTemplate.h',
        '<(skia_src_path)/core/SkStream.cpp',
        '<(skia_src_path)/core/SkStreamPriv.h',
        '<(skia_src_path)/core/SkString.cpp',
        '<(skia_src_path)/core/SkStringUtils.cpp',
        '<(skia_src_path)/core/SkStroke.h',
        '<(skia_src_path)/core/SkStroke.cpp',
        '<(skia_src_path)/core/SkStrokeRec.cpp',
        '<(skia_src_path)/core/SkStrokerPriv.cpp',
        '<(skia_src_path)/core/SkStrokerPriv.h',
        '<(skia_src_path)/core/SkTaskGroup.cpp',
        '<(skia_src_path)/core/SkTaskGroup.h',
        '<(skia_src_path)/core/SkTextBlob.cpp',
        '<(skia_src_path)/core/SkTextFormatParams.h',
        '<(skia_src_path)/core/SkTextMapStateProc.h',
        '<(skia_src_path)/core/SkTHashCache.h',
        '<(skia_src_path)/core/SkTLList.h',
        '<(skia_src_path)/core/SkTLS.cpp',
        '<(skia_src_path)/core/SkTraceEvent.h',
        '<(skia_src_path)/core/SkTSearch.cpp',
        '<(skia_src_path)/core/SkTSort.h',
        '<(skia_src_path)/core/SkTypeface.cpp',
        '<(skia_src_path)/core/SkTypefaceCache.cpp',
        '<(skia_src_path)/core/SkTypefaceCache.h',
        '<(skia_src_path)/core/SkUnPreMultiply.cpp',
        '<(skia_src_path)/core/SkUtils.cpp',
        '<(skia_src_path)/core/SkValidatingReadBuffer.cpp',
        '<(skia_src_path)/core/SkVarAlloc.cpp',
        '<(skia_src_path)/core/SkVertState.cpp',
        '<(skia_src_path)/core/SkWriteBuffer.cpp',
        '<(skia_src_path)/core/SkWriter32.cpp',
        '<(skia_src_path)/core/SkXfermode.cpp',
        '<(skia_src_path)/core/SkYUVPlanesCache.cpp',
        '<(skia_src_path)/core/SkYUVPlanesCache.h',

        '<(skia_src_path)/doc/SkDocument.cpp',

        '<(skia_src_path)/image/SkImage.cpp',
        '<(skia_src_path)/image/SkImagePriv.cpp',
#        '<(skia_src_path)/image/SkImage_Gpu.cpp',
        '<(skia_src_path)/image/SkImage_Raster.cpp',
        '<(skia_src_path)/image/SkSurface.cpp',
        '<(skia_src_path)/image/SkSurface_Base.h',
#        '<(skia_src_path)/image/SkSurface_Gpu.cpp',
        '<(skia_src_path)/image/SkSurface_Raster.cpp',

        '<(skia_src_path)/pipe/SkGPipeRead.cpp',
        '<(skia_src_path)/pipe/SkGPipeWrite.cpp',

        '<(skia_include_path)/core/SkAdvancedTypefaceMetrics.h',
        '<(skia_include_path)/core/SkBBHFactory.h',
        '<(skia_include_path)/core/SkBitmap.h',
        '<(skia_include_path)/core/SkBitmapDevice.h',
        '<(skia_include_path)/core/SkBlitRow.h',
        '<(skia_include_path)/core/SkCanvas.h',
        '<(skia_include_path)/core/SkChunkAlloc.h',
        '<(skia_include_path)/core/SkClipStack.h',
        '<(skia_include_path)/core/SkColor.h',
        '<(skia_include_path)/core/SkColorFilter.h',
        '<(skia_include_path)/core/SkColorPriv.h',
        '<(skia_include_path)/core/SkComposeShader.h',
        '<(skia_include_path)/core/SkData.h',
        '<(skia_include_path)/core/SkDeque.h',
        '<(skia_include_path)/core/SkDevice.h',
        '<(skia_include_path)/core/SkDither.h',
        '<(skia_include_path)/core/SkDraw.h',
        '<(skia_include_path)/core/SkDrawFilter.h',
        '<(skia_include_path)/core/SkDrawLooper.h',
        '<(skia_include_path)/core/SkEndian.h',
        '<(skia_include_path)/core/SkError.h',
        '<(skia_include_path)/core/SkFixed.h',
        '<(skia_include_path)/core/SkFlattenable.h',
        '<(skia_include_path)/core/SkFlattenableSerialization.h',
        '<(skia_include_path)/core/SkFloatBits.h',
        '<(skia_include_path)/core/SkFloatingPoint.h',
        '<(skia_include_path)/core/SkFontHost.h',
        '<(skia_include_path)/core/SkFontStyle.h',
        '<(skia_include_path)/core/SkGraphics.h',
        '<(skia_include_path)/core/SkImage.h',
        '<(skia_include_path)/core/SkImageDecoder.h',
        '<(skia_include_path)/core/SkImageEncoder.h',
        '<(skia_include_path)/core/SkImageFilter.h',
        '<(skia_include_path)/core/SkImageInfo.h',
        '<(skia_include_path)/core/SkInstCnt.h',
        '<(skia_include_path)/core/SkMallocPixelRef.h',
        '<(skia_include_path)/core/SkMask.h',
        '<(skia_include_path)/core/SkMaskFilter.h',
        '<(skia_include_path)/core/SkMath.h',
        '<(skia_include_path)/core/SkMatrix.h',
        '<(skia_include_path)/core/SkMetaData.h',
        '<(skia_include_path)/core/SkMultiPictureDraw.h',
        '<(skia_include_path)/core/SkOnce.h',
        '<(skia_include_path)/core/SkOSFile.h',
        '<(skia_include_path)/core/SkPackBits.h',
        '<(skia_include_path)/core/SkPaint.h',
        '<(skia_include_path)/core/SkPath.h',
        '<(skia_include_path)/core/SkPathEffect.h',
        '<(skia_include_path)/core/SkPathMeasure.h',
        '<(skia_include_path)/core/SkPathRef.h',
        '<(skia_include_path)/core/SkPicture.h',
        '<(skia_include_path)/core/SkPictureRecorder.h',
        '<(skia_include_path)/core/SkPixelRef.h',
        '<(skia_include_path)/core/SkPoint.h',
        '<(skia_include_path)/core/SkPreConfig.h',
        '<(skia_include_path)/core/SkRasterizer.h',
        '<(skia_include_path)/core/SkRect.h',
        '<(skia_include_path)/core/SkRefCnt.h',
        '<(skia_include_path)/core/SkRegion.h',
        '<(skia_include_path)/core/SkRRect.h',
        '<(skia_include_path)/core/SkScalar.h',
        '<(skia_include_path)/core/SkShader.h',
        '<(skia_include_path)/core/SkStream.h',
        '<(skia_include_path)/core/SkString.h',
        '<(skia_include_path)/core/SkStrokeRec.h',
        '<(skia_include_path)/core/SkSurface.h',
        '<(skia_include_path)/core/SkTArray.h',
        '<(skia_include_path)/core/SkTDArray.h',
        '<(skia_include_path)/core/SkTDStack.h',
        '<(skia_include_path)/core/SkTDict.h',
        '<(skia_include_path)/core/SkTInternalLList.h',
        '<(skia_include_path)/core/SkTRegistry.h',
        '<(skia_include_path)/core/SkTSearch.h',
        '<(skia_include_path)/core/SkTemplates.h',
        '<(skia_include_path)/core/SkTextBlob.h',
        '<(skia_include_path)/core/SkThread.h',
        '<(skia_include_path)/core/SkTime.h',
        '<(skia_include_path)/core/SkTLazy.h',
        '<(skia_include_path)/core/SkTypeface.h',
        '<(skia_include_path)/core/SkTypes.h',
        '<(skia_include_path)/core/SkUnPreMultiply.h',
        '<(skia_include_path)/core/SkUtils.h',
        '<(skia_include_path)/core/SkWeakRefCnt.h',
        '<(skia_include_path)/core/SkWriter32.h',
        '<(skia_include_path)/core/SkXfermode.h',

        # Lazy decoding:
        '<(skia_src_path)/lazy/SkCachingPixelRef.cpp',
        '<(skia_src_path)/lazy/SkCachingPixelRef.h',

        # Path ops
        '<(skia_include_path)/pathops/SkPathOps.h',

        '<(skia_src_path)/pathops/SkAddIntersections.cpp',
        '<(skia_src_path)/pathops/SkDCubicIntersection.cpp',
        '<(skia_src_path)/pathops/SkDCubicLineIntersection.cpp',
        '<(skia_src_path)/pathops/SkDCubicToQuads.cpp',
        '<(skia_src_path)/pathops/SkDLineIntersection.cpp',
        '<(skia_src_path)/pathops/SkDQuadImplicit.cpp',
        '<(skia_src_path)/pathops/SkDQuadIntersection.cpp',
        '<(skia_src_path)/pathops/SkDQuadLineIntersection.cpp',
        '<(skia_src_path)/pathops/SkIntersections.cpp',
        '<(skia_src_path)/pathops/SkOpAngle.cpp',
        '<(skia_src_path)/pathops/SkOpContour.cpp',
        '<(skia_src_path)/pathops/SkOpEdgeBuilder.cpp',
        '<(skia_src_path)/pathops/SkOpSegment.cpp',
        '<(skia_src_path)/pathops/SkPathOpsBounds.cpp',
        '<(skia_src_path)/pathops/SkPathOpsCommon.cpp',
        '<(skia_src_path)/pathops/SkPathOpsCubic.cpp',
        '<(skia_src_path)/pathops/SkPathOpsDebug.cpp',
        '<(skia_src_path)/pathops/SkPathOpsLine.cpp',
        '<(skia_src_path)/pathops/SkPathOpsOp.cpp',
        '<(skia_src_path)/pathops/SkPathOpsPoint.cpp',
        '<(skia_src_path)/pathops/SkPathOpsQuad.cpp',
        '<(skia_src_path)/pathops/SkPathOpsRect.cpp',
        '<(skia_src_path)/pathops/SkPathOpsSimplify.cpp',
        '<(skia_src_path)/pathops/SkPathOpsTightBounds.cpp',
        '<(skia_src_path)/pathops/SkPathOpsTriangle.cpp',
        '<(skia_src_path)/pathops/SkPathOpsTypes.cpp',
        '<(skia_src_path)/pathops/SkPathWriter.cpp',
        '<(skia_src_path)/pathops/SkQuarticRoot.cpp',
        '<(skia_src_path)/pathops/SkReduceOrder.cpp',
        '<(skia_src_path)/pathops/SkAddIntersections.h',
        '<(skia_src_path)/pathops/SkDQuadImplicit.h',
        '<(skia_src_path)/pathops/SkIntersectionHelper.h',
        '<(skia_src_path)/pathops/SkIntersections.h',
        '<(skia_src_path)/pathops/SkLineParameters.h',
        '<(skia_src_path)/pathops/SkOpAngle.h',
        '<(skia_src_path)/pathops/SkOpContour.h',
        '<(skia_src_path)/pathops/SkOpEdgeBuilder.h',
        '<(skia_src_path)/pathops/SkOpSegment.h',
        '<(skia_src_path)/pathops/SkOpSpan.h',
        '<(skia_src_path)/pathops/SkPathOpsBounds.h',
        '<(skia_src_path)/pathops/SkPathOpsCommon.h',
        '<(skia_src_path)/pathops/SkPathOpsCubic.h',
        '<(skia_src_path)/pathops/SkPathOpsCurve.h',
        '<(skia_src_path)/pathops/SkPathOpsDebug.h',
        '<(skia_src_path)/pathops/SkPathOpsLine.h',
        '<(skia_src_path)/pathops/SkPathOpsPoint.h',
        '<(skia_src_path)/pathops/SkPathOpsQuad.h',
        '<(skia_src_path)/pathops/SkPathOpsRect.h',
        '<(skia_src_path)/pathops/SkPathOpsTriangle.h',
        '<(skia_src_path)/pathops/SkPathOpsTypes.h',
        '<(skia_src_path)/pathops/SkPathWriter.h',
        '<(skia_src_path)/pathops/SkQuarticRoot.h',
        '<(skia_src_path)/pathops/SkReduceOrder.h',
    ],
}
