# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Include this gypi to include all 'utils' files
# The parent gyp/gypi file must define
#       'skia_src_path'     e.g. skia/trunk/src
#       'skia_include_path' e.g. skia/trunk/include
#
# The skia build defines these in common_variables.gypi
#
{
    'sources': [
        '<(skia_include_path)/utils/SkBoundaryPatch.h',
        '<(skia_include_path)/utils/SkFrontBufferedStream.h',
        '<(skia_include_path)/utils/SkCamera.h',
        '<(skia_include_path)/utils/SkCanvasStateUtils.h',
        '<(skia_include_path)/utils/SkCubicInterval.h',
        '<(skia_include_path)/utils/SkCullPoints.h',
        '<(skia_include_path)/utils/SkDebugUtils.h',
        '<(skia_include_path)/utils/SkDumpCanvas.h',
        '<(skia_include_path)/utils/SkEventTracer.h',
        '<(skia_include_path)/utils/SkInterpolator.h',
        '<(skia_include_path)/utils/SkLayer.h',
        '<(skia_include_path)/utils/SkMatrix44.h',
        '<(skia_include_path)/utils/SkMeshUtils.h',
        '<(skia_include_path)/utils/SkNinePatch.h',
        '<(skia_include_path)/utils/SkNoSaveLayerCanvas.h',
        '<(skia_include_path)/utils/SkNWayCanvas.h',
        '<(skia_include_path)/utils/SkNullCanvas.h',
        '<(skia_include_path)/utils/SkPaintFilterCanvas.h',
        '<(skia_include_path)/utils/SkParse.h',
        '<(skia_include_path)/utils/SkParsePaint.h',
        '<(skia_include_path)/utils/SkParsePath.h',
        '<(skia_include_path)/utils/SkPictureUtils.h',
        '<(skia_include_path)/utils/SkRandom.h',
        '<(skia_include_path)/utils/SkRTConf.h',
        '<(skia_include_path)/utils/SkTextBox.h',

        '<(skia_src_path)/utils/SkBase64.cpp',
        '<(skia_src_path)/utils/SkBase64.h',
        '<(skia_src_path)/utils/SkBitmapRegionCanvas.cpp',
        '<(skia_src_path)/utils/SkBitmapRegionDecoderInterface.cpp',
        '<(skia_src_path)/utils/SkBitmapRegionSampler.cpp',
        '<(skia_src_path)/utils/SkBitmapHasher.cpp',
        '<(skia_src_path)/utils/SkBitmapHasher.h',
        '<(skia_src_path)/utils/SkBitSet.cpp',
        '<(skia_src_path)/utils/SkBitSet.h',
        '<(skia_src_path)/utils/SkBoundaryPatch.cpp',
        '<(skia_src_path)/utils/SkFrontBufferedStream.cpp',
        '<(skia_src_path)/utils/SkCamera.cpp',
        '<(skia_src_path)/utils/SkCanvasStack.h',
        '<(skia_src_path)/utils/SkCanvasStack.cpp',
        '<(skia_src_path)/utils/SkCanvasStateUtils.cpp',
        '<(skia_src_path)/utils/SkCubicInterval.cpp',
        '<(skia_src_path)/utils/SkCullPoints.cpp',
        '<(skia_src_path)/utils/SkDashPath.cpp',
        '<(skia_src_path)/utils/SkDashPathPriv.h',
        '<(skia_src_path)/utils/SkDumpCanvas.cpp',
        '<(skia_src_path)/utils/SkEventTracer.cpp',
        '<(skia_src_path)/utils/SkFloatUtils.h',
        '<(skia_src_path)/utils/SkImageGeneratorUtils.cpp',
        '<(skia_src_path)/utils/SkInterpolator.cpp',
        '<(skia_src_path)/utils/SkLayer.cpp',
        '<(skia_src_path)/utils/SkMatrix22.cpp',
        '<(skia_src_path)/utils/SkMatrix22.h',
        '<(skia_src_path)/utils/SkMatrix44.cpp',
        '<(skia_src_path)/utils/SkMD5.cpp',
        '<(skia_src_path)/utils/SkMD5.h',
        '<(skia_src_path)/utils/SkMeshUtils.cpp',
        '<(skia_src_path)/utils/SkNinePatch.cpp',
        '<(skia_src_path)/utils/SkNWayCanvas.cpp',
        '<(skia_src_path)/utils/SkNullCanvas.cpp',
        '<(skia_src_path)/utils/SkOSFile.cpp',
        '<(skia_src_path)/utils/SkPaintFilterCanvas.cpp',
        '<(skia_src_path)/utils/SkParse.cpp',
        '<(skia_src_path)/utils/SkParseColor.cpp',
        '<(skia_src_path)/utils/SkParsePath.cpp',
        '<(skia_src_path)/utils/SkPatchGrid.cpp',
        '<(skia_src_path)/utils/SkPatchGrid.h',
        '<(skia_src_path)/utils/SkPatchUtils.cpp',
        '<(skia_src_path)/utils/SkPatchUtils.h',
        '<(skia_src_path)/utils/SkSHA1.cpp',
        '<(skia_src_path)/utils/SkSHA1.h',
        '<(skia_src_path)/utils/SkRTConf.cpp',
        '<(skia_src_path)/utils/SkTextBox.cpp',
        '<(skia_src_path)/utils/SkTextureCompressor.cpp',
        '<(skia_src_path)/utils/SkTextureCompressor.h',
        '<(skia_src_path)/utils/SkTextureCompressor_Utils.h',
        '<(skia_src_path)/utils/SkTextureCompressor_ASTC.cpp',
        '<(skia_src_path)/utils/SkTextureCompressor_ASTC.h',
        '<(skia_src_path)/utils/SkTextureCompressor_Blitter.h',
        '<(skia_src_path)/utils/SkTextureCompressor_R11EAC.cpp',
        '<(skia_src_path)/utils/SkTextureCompressor_R11EAC.h',
        '<(skia_src_path)/utils/SkTextureCompressor_LATC.cpp',
        '<(skia_src_path)/utils/SkTextureCompressor_LATC.h',
        '<(skia_src_path)/utils/SkThreadUtils.h',
        '<(skia_src_path)/utils/SkThreadUtils_pthread.cpp',
        '<(skia_src_path)/utils/SkThreadUtils_pthread.h',
        '<(skia_src_path)/utils/SkThreadUtils_pthread_linux.cpp',
        '<(skia_src_path)/utils/SkThreadUtils_pthread_mach.cpp',
        '<(skia_src_path)/utils/SkThreadUtils_pthread_other.cpp',
        '<(skia_src_path)/utils/SkThreadUtils_win.cpp',
        '<(skia_src_path)/utils/SkThreadUtils_win.h',
        '<(skia_src_path)/utils/SkTFitsIn.h',
        '<(skia_src_path)/utils/SkWhitelistTypefaces.cpp',

        #mac
        '<(skia_include_path)/utils/mac/SkCGUtils.h',
        '<(skia_src_path)/utils/mac/SkCreateCGImageRef.cpp',

        #windows
        '<(skia_include_path)/utils/win/SkAutoCoInitialize.h',
        '<(skia_include_path)/utils/win/SkHRESULT.h',
        '<(skia_include_path)/utils/win/SkIStream.h',
        '<(skia_include_path)/utils/win/SkTScopedComPtr.h',
        '<(skia_src_path)/utils/win/SkAutoCoInitialize.cpp',
        '<(skia_src_path)/utils/win/SkDWrite.h',
        '<(skia_src_path)/utils/win/SkDWrite.cpp',
        '<(skia_src_path)/utils/win/SkDWriteFontFileStream.cpp',
        '<(skia_src_path)/utils/win/SkDWriteFontFileStream.h',
        '<(skia_src_path)/utils/win/SkDWriteGeometrySink.cpp',
        '<(skia_src_path)/utils/win/SkDWriteGeometrySink.h',
        '<(skia_src_path)/utils/win/SkHRESULT.cpp',
        '<(skia_src_path)/utils/win/SkIStream.cpp',
        '<(skia_src_path)/utils/win/SkWGL.h',
        '<(skia_src_path)/utils/win/SkWGL_win.cpp',

        #testing
        '<(skia_src_path)/fonts/SkGScalerContext.cpp',
        '<(skia_src_path)/fonts/SkGScalerContext.h',
        '<(skia_src_path)/fonts/SkRandomScalerContext.cpp',
        '<(skia_src_path)/fonts/SkRandomScalerContext.h',
        '<(skia_src_path)/fonts/SkTestScalerContext.cpp',
        '<(skia_src_path)/fonts/SkTestScalerContext.h',
    ],
}
