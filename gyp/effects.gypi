# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Include this gypi to include all 'effects' files
# The parent gyp/gypi file must define:
#       'skia_src_path'     e.g. skia/trunk/src
#       'skia_include_path' e.g. skia/trunk/include
#
# The skia build defines these in common_variables.gypi.
#
{
  'sources': [
    '<(skia_src_path)/effects/GrCircleBlurFragmentProcessor.cpp',
    '<(skia_src_path)/effects/GrCircleBlurFragmentProcessor.h',

    '<(skia_src_path)/effects/Sk1DPathEffect.cpp',
    '<(skia_src_path)/effects/Sk2DPathEffect.cpp',
    '<(skia_src_path)/effects/SkAlphaThresholdFilter.cpp',
    '<(skia_src_path)/effects/SkArcToPathEffect.cpp',
    '<(skia_src_path)/effects/SkArithmeticMode.cpp',
    '<(skia_src_path)/effects/SkArithmeticMode_gpu.cpp',
    '<(skia_src_path)/effects/SkArithmeticMode_gpu.h',
    '<(skia_src_path)/effects/SkAvoidXfermode.cpp',
    '<(skia_src_path)/effects/SkBlurDrawLooper.cpp',
    '<(skia_src_path)/effects/SkBlurMask.cpp',
    '<(skia_src_path)/effects/SkBlurMask.h',
    '<(skia_src_path)/effects/SkBlurImageFilter.cpp',
    '<(skia_src_path)/effects/SkBlurMaskFilter.cpp',
    '<(skia_src_path)/effects/SkColorCubeFilter.cpp',
    '<(skia_src_path)/effects/SkColorFilterImageFilter.cpp',
    '<(skia_src_path)/effects/SkColorMatrix.cpp',
    '<(skia_src_path)/effects/SkColorMatrixFilter.cpp',
    '<(skia_src_path)/effects/SkComposeImageFilter.cpp',
    '<(skia_src_path)/effects/SkCornerPathEffect.cpp',
    '<(skia_src_path)/effects/SkDashPathEffect.cpp',
    '<(skia_src_path)/effects/SkDiscretePathEffect.cpp',
    '<(skia_src_path)/effects/SkDisplacementMapEffect.cpp',
    '<(skia_src_path)/effects/SkDropShadowImageFilter.cpp',
    '<(skia_src_path)/effects/SkEmbossMask.cpp',
    '<(skia_src_path)/effects/SkEmbossMask.h',
    '<(skia_src_path)/effects/SkEmbossMask_Table.h',
    '<(skia_src_path)/effects/SkEmbossMaskFilter.cpp',
    '<(skia_src_path)/effects/SkImageSource.cpp',
    '<(skia_src_path)/effects/SkGpuBlurUtils.h',
    '<(skia_src_path)/effects/SkGpuBlurUtils.cpp',
    '<(skia_src_path)/effects/SkLayerDrawLooper.cpp',
    '<(skia_src_path)/effects/SkLayerRasterizer.cpp',
    '<(skia_src_path)/effects/SkLightingImageFilter.cpp',
    '<(skia_src_path)/effects/SkLumaColorFilter.cpp',
    '<(skia_src_path)/effects/SkMagnifierImageFilter.cpp',
    '<(skia_src_path)/effects/SkMatrixConvolutionImageFilter.cpp',
    '<(skia_src_path)/effects/SkMergeImageFilter.cpp',
    '<(skia_src_path)/effects/SkMorphologyImageFilter.cpp',
    '<(skia_src_path)/effects/SkOffsetImageFilter.cpp',
    '<(skia_src_path)/effects/SkPackBits.cpp',
    '<(skia_src_path)/effects/SkPackBits.h',
    '<(skia_src_path)/effects/SkPaintFlagsDrawFilter.cpp',
    '<(skia_src_path)/effects/SkPaintImageFilter.cpp',
    '<(skia_src_path)/effects/SkPerlinNoiseShader.cpp',
    '<(skia_src_path)/effects/SkPictureImageFilter.cpp',
    '<(skia_src_path)/effects/SkPixelXorXfermode.cpp',
    '<(skia_src_path)/effects/SkTableColorFilter.cpp',
    '<(skia_src_path)/effects/SkTableMaskFilter.cpp',
    '<(skia_src_path)/effects/SkTestImageFilters.cpp',
    '<(skia_src_path)/effects/SkTileImageFilter.cpp',
    '<(skia_src_path)/effects/SkXfermodeImageFilter.cpp',

    '<(skia_src_path)/effects/gradients/Sk4fGradientBase.cpp',
    '<(skia_src_path)/effects/gradients/Sk4fGradientBase.h',
    '<(skia_src_path)/effects/gradients/Sk4fLinearGradient.cpp',
    '<(skia_src_path)/effects/gradients/Sk4fLinearGradient.h',
    '<(skia_src_path)/effects/gradients/SkClampRange.cpp',
    '<(skia_src_path)/effects/gradients/SkClampRange.h',
    '<(skia_src_path)/effects/gradients/SkGradientBitmapCache.cpp',
    '<(skia_src_path)/effects/gradients/SkGradientBitmapCache.h',
    '<(skia_src_path)/effects/gradients/SkGradientShader.cpp',
    '<(skia_src_path)/effects/gradients/SkGradientShaderPriv.h',
    '<(skia_src_path)/effects/gradients/SkLinearGradient.cpp',
    '<(skia_src_path)/effects/gradients/SkLinearGradient.h',
    '<(skia_src_path)/effects/gradients/SkRadialGradient.cpp',
    '<(skia_src_path)/effects/gradients/SkRadialGradient.h',
    '<(skia_src_path)/effects/gradients/SkTwoPointConicalGradient.cpp',
    '<(skia_src_path)/effects/gradients/SkTwoPointConicalGradient.h',
    '<(skia_src_path)/effects/gradients/SkTwoPointConicalGradient_gpu.cpp',
    '<(skia_src_path)/effects/gradients/SkTwoPointConicalGradient_gpu.h',
    '<(skia_src_path)/effects/gradients/SkSweepGradient.cpp',
    '<(skia_src_path)/effects/gradients/SkSweepGradient.h',

    '<(skia_include_path)/effects/Sk1DPathEffect.h',
    '<(skia_include_path)/effects/Sk2DPathEffect.h',
    '<(skia_include_path)/effects/SkAlphaThresholdFilter.h',
    '<(skia_include_path)/effects/SkArithmeticMode.h',
    '<(skia_include_path)/effects/SkBlurDrawLooper.h',
    '<(skia_include_path)/effects/SkBlurImageFilter.h',
    '<(skia_include_path)/effects/SkBlurMaskFilter.h',
    '<(skia_include_path)/effects/SkColorCubeFilter.h',
    '<(skia_include_path)/effects/SkColorFilterImageFilter.h',
    '<(skia_include_path)/effects/SkColorMatrix.h',
    '<(skia_include_path)/effects/SkColorMatrixFilter.h',
    '<(skia_include_path)/effects/SkCornerPathEffect.h',
    '<(skia_include_path)/effects/SkDashPathEffect.h',
    '<(skia_include_path)/effects/SkDiscretePathEffect.h',
    '<(skia_include_path)/effects/SkDisplacementMapEffect.h',
    '<(skia_include_path)/effects/SkDropShadowImageFilter.h',
    '<(skia_include_path)/effects/SkEmbossMaskFilter.h',
    '<(skia_include_path)/effects/SkGradientShader.h',
    '<(skia_include_path)/effects/SkImageSource.h',
    '<(skia_include_path)/effects/SkLayerDrawLooper.h',
    '<(skia_include_path)/effects/SkLayerRasterizer.h',
    '<(skia_include_path)/effects/SkLightingImageFilter.h',
    '<(skia_include_path)/effects/SkLumaColorFilter.h',
    '<(skia_include_path)/effects/SkMagnifierImageFilter.h',
    '<(skia_include_path)/effects/SkMorphologyImageFilter.h',
    '<(skia_include_path)/effects/SkOffsetImageFilter.h',
    '<(skia_include_path)/effects/SkPaintFlagsDrawFilter.h',
    '<(skia_include_path)/effects/SkPaintImageFilter.h',
    '<(skia_include_path)/effects/SkPerlinNoiseShader.h',
    '<(skia_include_path)/effects/SkTableColorFilter.h',
    '<(skia_include_path)/effects/SkTableMaskFilter.h',
    '<(skia_include_path)/effects/SkTileImageFilter.h',
    '<(skia_include_path)/effects/SkXfermodeImageFilter.h',

    '<(skia_include_path)/client/android/SkAvoidXfermode.h',
    '<(skia_include_path)/client/android/SkPixelXorXfermode.h',
  ],
}
