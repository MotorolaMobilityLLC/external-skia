# Include this gypi to include all 'core' files
# The parent gyp/gypi file must define
#       'skia_src_path'     e.g. skia/trunk/src
#       'skia_include_path' e.g. skia/trunk/include
#
# The skia build defines these in common_variables.gypi
#
{
    'sources': [
        '<(skia_src_path)/core/ARGB32_Clamp_Bilinear_BitmapShader.h',
        '<(skia_src_path)/core/Sk64.cpp',
        '<(skia_src_path)/core/SkAAClip.cpp',
        '<(skia_src_path)/core/SkAnnotation.cpp',
        '<(skia_src_path)/core/SkAdvancedTypefaceMetrics.cpp',
        '<(skia_src_path)/core/SkAlphaRuns.cpp',
        '<(skia_src_path)/core/SkAntiRun.h',
        '<(skia_src_path)/core/SkBBoxHierarchy.cpp',
        '<(skia_src_path)/core/SkBBoxHierarchy.h',
        '<(skia_src_path)/core/SkBBoxRecord.cpp',
        '<(skia_src_path)/core/SkBBoxRecord.h',
        '<(skia_src_path)/core/SkBBoxHierarchyRecord.cpp',
        '<(skia_src_path)/core/SkBBoxHierarchyRecord.h',
        '<(skia_src_path)/core/SkBitmap.cpp',
        '<(skia_src_path)/core/SkBitmapHeap.cpp',
        '<(skia_src_path)/core/SkBitmapHeap.h',
        '<(skia_src_path)/core/SkBitmapProcShader.cpp',
        '<(skia_src_path)/core/SkBitmapProcShader.h',
        '<(skia_src_path)/core/SkBitmapProcState.cpp',
        '<(skia_src_path)/core/SkBitmapProcState.h',
        '<(skia_src_path)/core/SkBitmapProcState_matrix.h',
        '<(skia_src_path)/core/SkBitmapProcState_matrixProcs.cpp',
        '<(skia_src_path)/core/SkBitmapProcState_sample.h',
        '<(skia_src_path)/core/SkBitmapSampler.cpp',
        '<(skia_src_path)/core/SkBitmapSampler.h',
        '<(skia_src_path)/core/SkBitmapSamplerTemplate.h',
        '<(skia_src_path)/core/SkBitmapShader16BilerpTemplate.h',
        '<(skia_src_path)/core/SkBitmapShaderTemplate.h',
        '<(skia_src_path)/core/SkBitmap_scroll.cpp',
        '<(skia_src_path)/core/SkBlitBWMaskTemplate.h',
        '<(skia_src_path)/core/SkBlitMask_D32.cpp',
        '<(skia_src_path)/core/SkBlitRow_D16.cpp',
        '<(skia_src_path)/core/SkBlitRow_D32.cpp',
        '<(skia_src_path)/core/SkBlitRow_D4444.cpp',
        '<(skia_src_path)/core/SkBlitter.h',
        '<(skia_src_path)/core/SkBlitter.cpp',
        '<(skia_src_path)/core/SkBlitter_4444.cpp',
        '<(skia_src_path)/core/SkBlitter_A1.cpp',
        '<(skia_src_path)/core/SkBlitter_A8.cpp',
        '<(skia_src_path)/core/SkBlitter_ARGB32.cpp',
        '<(skia_src_path)/core/SkBlitter_RGB16.cpp',
        '<(skia_src_path)/core/SkBlitter_Sprite.cpp',
        '<(skia_src_path)/core/SkBuffer.cpp',
        '<(skia_src_path)/core/SkCanvas.cpp',
        '<(skia_src_path)/core/SkChunkAlloc.cpp',
        '<(skia_src_path)/core/SkClipStack.cpp',
        '<(skia_src_path)/core/SkColor.cpp',
        '<(skia_src_path)/core/SkColorFilter.cpp',
        '<(skia_src_path)/core/SkColorTable.cpp',
        '<(skia_src_path)/core/SkComposeShader.cpp',
        '<(skia_src_path)/core/SkConcaveToTriangles.cpp',
        '<(skia_src_path)/core/SkConcaveToTriangles.h',
        '<(skia_src_path)/core/SkConfig8888.cpp',
        '<(skia_src_path)/core/SkConfig8888.h',
        '<(skia_src_path)/core/SkCordic.cpp',
        '<(skia_src_path)/core/SkCordic.h',
        '<(skia_src_path)/core/SkCoreBlitters.h',
        '<(skia_src_path)/core/SkCubicClipper.cpp',
        '<(skia_src_path)/core/SkCubicClipper.h',
        '<(skia_src_path)/core/SkData.cpp',
        '<(skia_src_path)/core/SkDebug.cpp',
        '<(skia_src_path)/core/SkDeque.cpp',
        '<(skia_src_path)/core/SkDevice.cpp',
        '<(skia_src_path)/core/SkDeviceProfile.cpp',
        '<(skia_src_path)/core/SkDither.cpp',
        '<(skia_src_path)/core/SkDraw.cpp',
        '<(skia_src_path)/core/SkDrawProcs.h',
        '<(skia_src_path)/core/SkEdgeBuilder.cpp',
        '<(skia_src_path)/core/SkEdgeClipper.cpp',
        '<(skia_src_path)/core/SkEdge.cpp',
        '<(skia_src_path)/core/SkEdge.h',
        '<(skia_src_path)/core/SkFP.h',
        '<(skia_src_path)/core/SkFilterProc.cpp',
        '<(skia_src_path)/core/SkFilterProc.h',
        '<(skia_src_path)/core/SkFlattenable.cpp',
        '<(skia_src_path)/core/SkFlattenableBuffers.cpp',
        '<(skia_src_path)/core/SkFloat.cpp',
        '<(skia_src_path)/core/SkFloat.h',
        '<(skia_src_path)/core/SkFloatBits.cpp',
        '<(skia_src_path)/core/SkFontHost.cpp',
        '<(skia_src_path)/core/SkGeometry.cpp',
        '<(skia_src_path)/core/SkGlyphCache.cpp',
        '<(skia_src_path)/core/SkGlyphCache.h',
        '<(skia_src_path)/core/SkGraphics.cpp',
        '<(skia_src_path)/core/SkInstCnt.cpp',
        '<(skia_src_path)/core/SkImageFilter.cpp',
        '<(skia_src_path)/core/SkLineClipper.cpp',
        '<(skia_src_path)/core/SkMallocPixelRef.cpp',
        '<(skia_src_path)/core/SkMask.cpp',
        '<(skia_src_path)/core/SkMaskFilter.cpp',
        '<(skia_src_path)/core/SkMaskGamma.cpp',
        '<(skia_src_path)/core/SkMaskGamma.h',
        '<(skia_src_path)/core/SkMath.cpp',
        '<(skia_src_path)/core/SkMatrix.cpp',
        '<(skia_src_path)/core/SkMetaData.cpp',
        '<(skia_src_path)/core/SkMMapStream.cpp',
        '<(skia_src_path)/core/SkOrderedReadBuffer.cpp',
        '<(skia_src_path)/core/SkOrderedWriteBuffer.cpp',
        '<(skia_src_path)/core/SkPackBits.cpp',
        '<(skia_src_path)/core/SkPaint.cpp',
        '<(skia_src_path)/core/SkPath.cpp',
        '<(skia_src_path)/core/SkPathEffect.cpp',
        '<(skia_src_path)/core/SkPathHeap.cpp',
        '<(skia_src_path)/core/SkPathHeap.h',
        '<(skia_src_path)/core/SkPathMeasure.cpp',
        '<(skia_src_path)/core/SkPathRef.h',
        '<(skia_src_path)/core/SkPicture.cpp',
        '<(skia_src_path)/core/SkPictureFlat.cpp',
        '<(skia_src_path)/core/SkPictureFlat.h',
        '<(skia_src_path)/core/SkPicturePlayback.cpp',
        '<(skia_src_path)/core/SkPicturePlayback.h',
        '<(skia_src_path)/core/SkPictureRecord.cpp',
        '<(skia_src_path)/core/SkPictureRecord.h',
        '<(skia_src_path)/core/SkPictureStateTree.cpp',
        '<(skia_src_path)/core/SkPictureStateTree.h',
        '<(skia_src_path)/core/SkPixelRef.cpp',
        '<(skia_src_path)/core/SkPoint.cpp',
        '<(skia_src_path)/core/SkProcSpriteBlitter.cpp',
        '<(skia_src_path)/core/SkPtrRecorder.cpp',
        '<(skia_src_path)/core/SkQuadClipper.cpp',
        '<(skia_src_path)/core/SkQuadClipper.h',
        '<(skia_src_path)/core/SkRasterClip.cpp',
        '<(skia_src_path)/core/SkRasterizer.cpp',
        '<(skia_src_path)/core/SkRect.cpp',
        '<(skia_src_path)/core/SkRefCnt.cpp',
        '<(skia_src_path)/core/SkRefDict.cpp',
        '<(skia_src_path)/core/SkRegion.cpp',
        '<(skia_src_path)/core/SkRegionPriv.h',
        '<(skia_src_path)/core/SkRegion_path.cpp',
        '<(skia_src_path)/core/SkRTree.h',
        '<(skia_src_path)/core/SkRTree.cpp',
        '<(skia_src_path)/core/SkScalar.cpp',
        '<(skia_src_path)/core/SkScalerContext.cpp',
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
        '<(skia_src_path)/core/SkSinTable.h',
        '<(skia_src_path)/core/SkSpriteBlitter.h',
        '<(skia_src_path)/core/SkSpriteBlitterTemplate.h',
        '<(skia_src_path)/core/SkStream.cpp',
        '<(skia_src_path)/core/SkString.cpp',
        '<(skia_src_path)/core/SkStroke.h',
        '<(skia_src_path)/core/SkStroke.cpp',
        '<(skia_src_path)/core/SkStrokerPriv.cpp',
        '<(skia_src_path)/core/SkStrokerPriv.h',
        '<(skia_src_path)/core/SkTextFormatParams.h',
        '<(skia_src_path)/core/SkTLS.cpp',
        '<(skia_src_path)/core/SkTSearch.cpp',
        '<(skia_src_path)/core/SkTSort.h',
        '<(skia_src_path)/core/SkTemplatesPriv.h',
        '<(skia_src_path)/core/SkTypeface.cpp',
        '<(skia_src_path)/core/SkTypefaceCache.cpp',
        '<(skia_src_path)/core/SkTypefaceCache.h',
        '<(skia_src_path)/core/SkUnPreMultiply.cpp',
        '<(skia_src_path)/core/SkUtils.cpp',
        '<(skia_src_path)/core/SkWriter32.cpp',
        '<(skia_src_path)/core/SkXfermode.cpp',

        '<(skia_src_path)/image/SkDataPixelRef.cpp',
        '<(skia_src_path)/image/SkImage.cpp',
        '<(skia_src_path)/image/SkImagePriv.cpp',
        '<(skia_src_path)/image/SkImage_Codec.cpp',
#        '<(skia_src_path)/image/SkImage_Gpu.cpp',
        '<(skia_src_path)/image/SkImage_Picture.cpp',
        '<(skia_src_path)/image/SkImage_Raster.cpp',
        '<(skia_src_path)/image/SkSurface.cpp',
#        '<(skia_src_path)/image/SkSurface_Gpu.cpp',
        '<(skia_src_path)/image/SkSurface_Picture.cpp',
        '<(skia_src_path)/image/SkSurface_Raster.cpp',

        '<(skia_src_path)/pipe/SkGPipeRead.cpp',
        '<(skia_src_path)/pipe/SkGPipeWrite.cpp',

        '<(skia_include_path)/core/Sk64.h',
        '<(skia_include_path)/core/SkAdvancedTypefaceMetrics.h',
        '<(skia_include_path)/core/SkBitmap.h',
        '<(skia_include_path)/core/SkBlitRow.h',
        '<(skia_include_path)/core/SkBounder.h',
        '<(skia_include_path)/core/SkCanvas.h',
        '<(skia_include_path)/core/SkChecksum.h',
        '<(skia_include_path)/core/SkChunkAlloc.h',
        '<(skia_include_path)/core/SkClipStack.h',
        '<(skia_include_path)/core/SkColor.h',
        '<(skia_include_path)/core/SkColorFilter.h',
        '<(skia_include_path)/core/SkColorPriv.h',
        '<(skia_include_path)/core/SkColorShader.h',
        '<(skia_include_path)/core/SkComposeShader.h',
        '<(skia_include_path)/core/SkData.h',
        '<(skia_include_path)/core/SkDeque.h',
        '<(skia_include_path)/core/SkDevice.h',
        '<(skia_include_path)/core/SkDither.h',
        '<(skia_include_path)/core/SkDraw.h',
        '<(skia_include_path)/core/SkDrawFilter.h',
        '<(skia_include_path)/core/SkDrawLooper.h',
        '<(skia_include_path)/core/SkEndian.h',
        '<(skia_include_path)/core/SkFixed.h',
        '<(skia_include_path)/core/SkFlattenable.h',
        '<(skia_include_path)/core/SkFloatBits.h',
        '<(skia_include_path)/core/SkFloatingPoint.h',
        '<(skia_include_path)/core/SkFontHost.h',
        '<(skia_include_path)/core/SkGeometry.h',
        '<(skia_include_path)/core/SkGraphics.h',
        '<(skia_include_path)/core/SkImageFilter.h',
        '<(skia_include_path)/core/SkInstCnt.h',
        '<(skia_include_path)/core/SkMallocPixelRef.h',
        '<(skia_include_path)/core/SkMask.h',
        '<(skia_include_path)/core/SkMaskFilter.h',
        '<(skia_include_path)/core/SkMath.h',
        '<(skia_include_path)/core/SkMatrix.h',
        '<(skia_include_path)/core/SkMetaData.h',
        '<(skia_include_path)/core/SkMMapStream.h',
        '<(skia_include_path)/core/SkOSFile.h',
        '<(skia_include_path)/core/SkPackBits.h',
        '<(skia_include_path)/core/SkPaint.h',
        '<(skia_include_path)/core/SkPath.h',
        '<(skia_include_path)/core/SkPathEffect.h',
        '<(skia_include_path)/core/SkPathMeasure.h',
        '<(skia_include_path)/core/SkPicture.h',
        '<(skia_include_path)/core/SkPixelRef.h',
        '<(skia_include_path)/core/SkPoint.h',
        '<(skia_include_path)/core/SkRandom.h',
        '<(skia_include_path)/core/SkRasterizer.h',
        '<(skia_include_path)/core/SkReader32.h',
        '<(skia_include_path)/core/SkRect.h',
        '<(skia_include_path)/core/SkRefCnt.h',
        '<(skia_include_path)/core/SkRegion.h',
        '<(skia_include_path)/core/SkScalar.h',
        '<(skia_include_path)/core/SkScalarCompare.h',
        '<(skia_include_path)/core/SkShader.h',
        '<(skia_include_path)/core/SkStream.h',
        '<(skia_include_path)/core/SkString.h',
        '<(skia_include_path)/core/SkTArray.h',
        '<(skia_include_path)/core/SkTDArray.h',
        '<(skia_include_path)/core/SkTDStack.h',
        '<(skia_include_path)/core/SkTDict.h',
        '<(skia_include_path)/core/SkTDLinkedList.h',
        '<(skia_include_path)/core/SkTRegistry.h',
        '<(skia_include_path)/core/SkTScopedPtr.h',
        '<(skia_include_path)/core/SkTSearch.h',
        '<(skia_include_path)/core/SkTemplates.h',
        '<(skia_include_path)/core/SkThread.h',
        '<(skia_include_path)/core/SkThread_platform.h',
        '<(skia_include_path)/core/SkTime.h',
        '<(skia_include_path)/core/SkTLazy.h',
        '<(skia_include_path)/core/SkTrace.h',
        '<(skia_include_path)/core/SkTypeface.h',
        '<(skia_include_path)/core/SkTypes.h',
        '<(skia_include_path)/core/SkUnPreMultiply.h',
        '<(skia_include_path)/core/SkUnitMapper.h',
        '<(skia_include_path)/core/SkUtils.h',
        '<(skia_include_path)/core/SkWeakRefCnt.h',
        '<(skia_include_path)/core/SkWriter32.h',
        '<(skia_include_path)/core/SkXfermode.h',
    ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
