# Include this gypi to include all public header files that exist in the
# include directory.
#
# The list is computed by running 'find include -name *.h' in the root dir of
# the project.
#
{
  'variables': {
    'header_filenames': [
      'animator/SkAnimator.h',
      'animator/SkAnimatorView.h',
      'config/SkUserConfig.h',
      'config/sk_stdint.h',
      'core/SkAdvancedTypefaceMetrics.h',
      'core/SkAnnotation.h',
      'core/SkBitmap.h',
      'core/SkBlitRow.h',
      'core/SkCanvas.h',
      'core/SkChunkAlloc.h',
      'core/SkClipStack.h',
      'core/SkColor.h',
      'core/SkColorFilter.h',
      'core/SkColorPriv.h',
      'core/SkColorShader.h',
      'core/SkColorTable.h',
      'core/SkComposeShader.h',
      'core/SkData.h',
      'core/SkDataTable.h',
      'core/SkDeque.h',
      'core/SkDevice.h',
      'core/SkDither.h',
      'core/SkDocument.h',
      'core/SkDraw.h',
      'core/SkDrawFilter.h',
      'core/SkDrawLooper.h',
      'core/SkEndian.h',
      'core/SkError.h',
      'core/SkFixed.h',
      'core/SkFlate.h',
      'core/SkFlattenable.h',
      'core/SkFlattenableBuffers.h',
      'core/SkFlattenableSerialization.h',
      'core/SkFloatBits.h',
      'core/SkFloatingPoint.h',
      'core/SkFontHost.h',
      'core/SkFontLCDConfig.h',
      'core/SkGeometry.h',
      'core/SkGraphics.h',
      'core/SkImage.h',
      'core/SkImageDecoder.h',
      'core/SkImageEncoder.h',
      'core/SkImageFilter.h',
      'core/SkInstCnt.h',
      'core/SkLineClipper.h',
      'core/SkMallocPixelRef.h',
      'core/SkMask.h',
      'core/SkMaskFilter.h',
      'core/SkMath.h',
      'core/SkMatrix.h',
      'core/SkMetaData.h',
      'core/SkOSFile.h',
      'core/SkPackBits.h',
      'core/SkPaint.h',
      'core/SkPaintOptionsAndroid.h',
      'core/SkPath.h',
      'core/SkPathEffect.h',
      'core/SkPathMeasure.h',
      'core/SkPicture.h',
      'core/SkPixelRef.h',
      'core/SkPoint.h',
      'core/SkPostConfig.h',
      'core/SkPreConfig.h',
      'core/SkRRect.h',
      'core/SkRasterizer.h',
      'core/SkRect.h',
      'core/SkRefCnt.h',
      'core/SkRegion.h',
      'core/SkScalar.h',
      'core/SkShader.h',
      'core/SkSize.h',
      'core/SkStream.h',
      'core/SkString.h',
      'core/SkStringUtils.h',
      'core/SkStrokeRec.h',
      'core/SkSurface.h',
      'core/SkTArray.h',
      'core/SkTDArray.h',
      'core/SkTDStack.h',
      'core/SkTDict.h',
      'core/SkTInternalLList.h',
      'core/SkTLazy.h',
      'core/SkTRegistry.h',
      'core/SkTSearch.h',
      'core/SkTemplates.h',
      'core/SkThread.h',
      'core/SkTime.h',
      'core/SkTypeface.h',
      'core/SkTypes.h',
      'core/SkUnPreMultiply.h',
      'core/SkUtils.h',
      'core/SkWeakRefCnt.h',
      'core/SkWriter32.h',
      'core/SkXfermode.h',
      'device/xps/SkConstexprMath.h',
      'device/xps/SkXPSDevice.h',
      'effects/Sk1DPathEffect.h',
      'effects/Sk2DPathEffect.h',
      'effects/SkAlphaThresholdFilter.h',
      'effects/SkArithmeticMode.h',
      'effects/SkAvoidXfermode.h',
      'effects/SkBitmapSource.h',
      'effects/SkBlurDrawLooper.h',
      'effects/SkBlurImageFilter.h',
      'effects/SkBlurMaskFilter.h',
      'effects/SkColorFilterImageFilter.h',
      'effects/SkColorMatrix.h',
      'effects/SkColorMatrixFilter.h',
      'effects/SkComposeImageFilter.h',
      'effects/SkCornerPathEffect.h',
      'effects/SkDashPathEffect.h',
      'effects/SkDiscretePathEffect.h',
      'effects/SkDisplacementMapEffect.h',
      'effects/SkDrawExtraPathEffect.h',
      'effects/SkDropShadowImageFilter.h',
      'effects/SkEmbossMaskFilter.h',
      'effects/SkGradientShader.h',
      'effects/SkLayerDrawLooper.h',
      'effects/SkLayerRasterizer.h',
      'effects/SkLerpXfermode.h',
      'effects/SkLightingImageFilter.h',
      'effects/SkLumaColorFilter.h',
      'effects/SkMagnifierImageFilter.h',
      'effects/SkMatrixConvolutionImageFilter.h',
      'effects/SkMergeImageFilter.h',
      'effects/SkMorphologyImageFilter.h',
      'effects/SkOffsetImageFilter.h',
      'effects/SkPaintFlagsDrawFilter.h',
      'effects/SkPerlinNoiseShader.h',
      'effects/SkPixelXorXfermode.h',
      'effects/SkPorterDuff.h',
      'effects/SkRectShaderImageFilter.h',
      'effects/SkTableColorFilter.h',
      'effects/SkTableMaskFilter.h',
      'effects/SkTestImageFilters.h',
      'effects/SkTransparentShader.h',
      'effects/SkXfermodeImageFilter.h',
      'gpu/GrAARectRenderer.h',
      'gpu/GrBackendEffectFactory.h',
      'gpu/GrClipData.h',
      'gpu/GrColor.h',
      'gpu/GrConfig.h',
      'gpu/GrContext.h',
      'gpu/GrContextFactory.h',
      'gpu/GrDrawEffect.h',
      'gpu/GrEffect.h',
      'gpu/GrEffectStage.h',
      'gpu/GrEffectUnitTest.h',
      'gpu/GrFontScaler.h',
      'gpu/GrGlyph.h',
      'gpu/GrKey.h',
      'gpu/GrOvalRenderer.h',
      'gpu/GrPaint.h',
      'gpu/GrPathRendererChain.h',
      'gpu/GrRect.h',
      'gpu/GrRenderTarget.h',
      'gpu/GrResource.h',
      'gpu/GrSurface.h',
      'gpu/GrTBackendEffectFactory.h',
      'gpu/GrTextContext.h',
      'gpu/GrTexture.h',
      'gpu/GrTextureAccess.h',
      'gpu/GrTypes.h',
      'gpu/GrTypesPriv.h',
      'gpu/GrUserConfig.h',
      'gpu/SkGpuDevice.h',
      'gpu/SkGr.h',
      'gpu/SkGrPixelRef.h',
      'gpu/SkGrTexturePixelRef.h',
      'gpu/gl/GrGLConfig.h',
      'gpu/gl/GrGLConfig_chrome.h',
      'gpu/gl/GrGLExtensions.h',
      'gpu/gl/GrGLFunctions.h',
      'gpu/gl/GrGLInterface.h',
      'gpu/gl/SkANGLEGLContext.h',
      'gpu/gl/SkDebugGLContext.h',
      'gpu/gl/SkGLContextHelper.h',
      'gpu/gl/SkMesaGLContext.h',
      'gpu/gl/SkNativeGLContext.h',
      'gpu/gl/SkNullGLContext.h',
      'images/SkForceLinking.h',
      'images/SkMovie.h',
      'images/SkPageFlipper.h',
      'pathops/SkPathOps.h',
      'pdf/SkPDFDevice.h',
      'pdf/SkPDFDocument.h',
      'pipe/SkGPipe.h',
      'ports/SkFontConfigInterface.h',
      'ports/SkFontMgr.h',
      'ports/SkFontStyle.h',
      'ports/SkTypeface_android.h',
      'ports/SkTypeface_mac.h',
      'ports/SkTypeface_win.h',
      'svg/SkSVGAttribute.h',
      'svg/SkSVGBase.h',
      'svg/SkSVGPaintState.h',
      'svg/SkSVGParser.h',
      'svg/SkSVGTypes.h',
      'utils/SkBoundaryPatch.h',
      'utils/SkCamera.h',
      'utils/SkCubicInterval.h',
      'utils/SkCullPoints.h',
      'utils/SkDebugUtils.h',
      'utils/SkDeferredCanvas.h',
      'utils/SkDumpCanvas.h',
      'utils/SkInterpolator.h',
      'utils/SkLayer.h',
      'utils/SkLua.h',
      'utils/SkLuaCanvas.h',
      'utils/SkMatrix44.h',
      'utils/SkMeshUtils.h',
      'utils/SkNWayCanvas.h',
      'utils/SkNinePatch.h',
      'utils/SkNullCanvas.h',
      'utils/SkParse.h',
      'utils/SkParsePaint.h',
      'utils/SkParsePath.h',
      'utils/SkPathUtils.h',
      'utils/SkPictureUtils.h',
      'utils/SkProxyCanvas.h',
      'utils/SkRTConf.h',
      'utils/SkRandom.h',
      'utils/SkWGL.h',
      'utils/ios/SkStream_NSData.h',
      'utils/mac/SkCGUtils.h',
      'utils/win/SkAutoCoInitialize.h',
      'utils/win/SkHRESULT.h',
      'utils/win/SkIStream.h',
      'utils/win/SkTScopedComPtr.h',
      'views/SkApplication.h',
      'views/SkBGViewArtist.h',
      'views/SkEvent.h',
      'views/SkEventSink.h',
      'views/SkKey.h',
      'views/SkOSMenu.h',
      'views/SkOSWindow_Android.h',
      'views/SkOSWindow_Mac.h',
      'views/SkOSWindow_NaCl.h',
      'views/SkOSWindow_SDL.h',
      'views/SkOSWindow_Unix.h',
      'views/SkOSWindow_Win.h',
      'views/SkOSWindow_iOS.h',
      'views/SkStackViewLayout.h',
      'views/SkSystemEventTypes.h',
      'views/SkTextBox.h',
      'views/SkTouchGesture.h',
      'views/SkView.h',
      'views/SkViewInflate.h',
      'views/SkWidget.h',
      'views/SkWindow.h',
      'views/android/AndroidKeyToSkKey.h',
      'views/animated/SkBorderView.h',
      'views/animated/SkImageView.h',
      'views/animated/SkProgressBarView.h',
      'views/animated/SkScrollBarView.h',
      'views/animated/SkWidgetViews.h',
      'views/unix/XkeysToSkKeys.h',
      'views/unix/keysym2ucs.h',
      'xml/SkBML_WXMLParser.h',
      'xml/SkBML_XMLParser.h',
      'xml/SkDOM.h',
      'xml/SkJS.h',
      'xml/SkXMLParser.h',
      'xml/SkXMLWriter.h',
    ],
  },
}
