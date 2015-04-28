# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# include this gypi to include all the golden master slides.
{
  'include_dirs': [
    '../gm',
    # include dirs needed by particular GMs
    '../src/utils/debugger',
    '../src/images',
    '../src/lazy',
  ],
  'conditions': [
    # If we're building SampleApp on the bots, no need to link in the GM slides.
    # We're not going to run it; we're only making sure it still builds.
    # It'd be nice to do this in SampleApp.gypi, but I can't find a way to make it work.
    [ 'not ("<(_target_name)" == "SampleApp" and skia_is_bot)', {
      'sources': [
        '../gm/aaclip.cpp',
        '../gm/aarectmodes.cpp',
        '../gm/addarc.cpp',
        '../gm/all_bitmap_configs.cpp',
        '../gm/alphagradients.cpp',
        '../gm/anisotropic.cpp',
        '../gm/arcofzorro.cpp',
        '../gm/arithmode.cpp',
        '../gm/astcbitmap.cpp',
        '../gm/badpaint.cpp',
        '../gm/beziereffects.cpp',
        '../gm/beziers.cpp',
        '../gm/bigblurs.cpp',
        '../gm/bigmatrix.cpp',
        '../gm/bigtext.cpp',
        '../gm/bitmapcopy.cpp',
        '../gm/bitmapfilters.cpp',
        '../gm/bitmappremul.cpp',
        '../gm/bitmaprect.cpp',
        '../gm/bitmaprecttest.cpp',
        '../gm/bitmapscroll.cpp',
        '../gm/bitmapshader.cpp',
        '../gm/bitmapsource.cpp',
        '../gm/bitmapsource2.cpp',
        '../gm/bleed.cpp',
        '../gm/blend.cpp',
        '../gm/blurcircles.cpp',
        '../gm/blurs.cpp',
        '../gm/blurquickreject.cpp',
        '../gm/blurrect.cpp',
        '../gm/blurroundrect.cpp',
        '../gm/bmpfilterqualityrepeat.cpp',
        '../gm/circles.cpp',
        '../gm/circularclips.cpp',
        '../gm/clipdrawdraw.cpp',
        '../gm/clip_strokerect.cpp',
        '../gm/clippedbitmapshaders.cpp',
        '../gm/cgms.cpp',
        '../gm/cgm.c',
        '../gm/colorcube.cpp',
        '../gm/coloremoji.cpp',
        '../gm/colorfilterimagefilter.cpp',
        '../gm/colorfilters.cpp',
        '../gm/colormatrix.cpp',
        '../gm/colortype.cpp',
        '../gm/colortypexfermode.cpp',
        '../gm/colorwheel.cpp',
        '../gm/concavepaths.cpp',
        '../gm/complexclip.cpp',
        '../gm/complexclip2.cpp',
        '../gm/complexclip3.cpp',
        '../gm/composeshader.cpp',
        '../gm/conicpaths.cpp',
        '../gm/constcolorprocessor.cpp',
        '../gm/convex_all_line_paths.cpp',
        '../gm/convexpaths.cpp',
        '../gm/convexpolyclip.cpp',
        '../gm/convexpolyeffect.cpp',
        '../gm/copyTo4444.cpp',
        '../gm/cubicpaths.cpp',
        '../gm/cmykjpeg.cpp',
        '../gm/dstreadshuffle.cpp',
        '../gm/degeneratesegments.cpp',
        '../gm/dcshader.cpp',
        '../gm/discard.cpp',
        '../gm/dashcubics.cpp',
        '../gm/dashing.cpp',
        '../gm/distantclip.cpp',
        '../gm/dftext.cpp',
        '../gm/displacement.cpp',
        '../gm/downsamplebitmap.cpp',
        '../gm/drawbitmaprect.cpp',
        '../gm/drawfilter.cpp',
        '../gm/drawlooper.cpp',
        '../gm/dropshadowimagefilter.cpp',
        '../gm/drrect.cpp',
        '../gm/dstreadshuffle.cpp',
        '../gm/etc1bitmap.cpp',
        '../gm/extractbitmap.cpp',
        '../gm/emboss.cpp',
        '../gm/emptypath.cpp',
        '../gm/fadefilter.cpp',
        '../gm/fatpathfill.cpp',
        '../gm/factory.cpp',
        '../gm/filltypes.cpp',
        '../gm/filltypespersp.cpp',
        '../gm/filterbitmap.cpp',
        '../gm/filterfastbounds.cpp',
        '../gm/filterindiabox.cpp',
        '../gm/fontcache.cpp',
        '../gm/fontmgr.cpp',
        '../gm/fontscaler.cpp',
        '../gm/gammatext.cpp',
        '../gm/getpostextpath.cpp',
        '../gm/giantbitmap.cpp',
        '../gm/glyph_pos.cpp',
        '../gm/glyph_pos_align.cpp',
        '../gm/gradients.cpp',
        '../gm/gradients_2pt_conical.cpp',
        '../gm/gradients_no_texture.cpp',
        '../gm/gradientDirtyLaundry.cpp',
        '../gm/gradient_matrix.cpp',
        '../gm/gradtext.cpp',
        '../gm/grayscalejpg.cpp',
        '../gm/hairlines.cpp',
        '../gm/hairmodes.cpp',
        '../gm/hittestpath.cpp',
        '../gm/imagealphathreshold.cpp',
        '../gm/imageblur.cpp',
        '../gm/imageblur2.cpp',
        '../gm/imageblurtiled.cpp',
        '../gm/imagemagnifier.cpp',
        '../gm/imageresizetiled.cpp',
        '../gm/inversepaths.cpp',
        '../gm/lerpmode.cpp',
        '../gm/lighting.cpp',
        '../gm/lumafilter.cpp',
        '../gm/image.cpp',
        '../gm/imagefilters.cpp',
        '../gm/imagefiltersbase.cpp',
        '../gm/imagefiltersclipped.cpp',
        '../gm/imagefilterscropped.cpp',
        '../gm/imagefilterscropexpand.cpp',
        '../gm/imagefiltersgraph.cpp',
        '../gm/imagefiltersscaled.cpp',
        '../gm/imagefilterstransformed.cpp',
        '../gm/internal_links.cpp',
        '../gm/lcdtext.cpp',
        '../gm/linepaths.cpp',
        '../gm/matrixconvolution.cpp',
        '../gm/matriximagefilter.cpp',
        '../gm/megalooper.cpp',
        '../gm/mixedxfermodes.cpp',
        '../gm/mixedtextblobs.cpp',
        '../gm/mipmap.cpp',
        '../gm/modecolorfilters.cpp',
        '../gm/morphology.cpp',
        '../gm/multipicturedraw.cpp',
        '../gm/nested.cpp',
        '../gm/ninepatchstretch.cpp',
        '../gm/nonclosedpaths.cpp',
        '../gm/offsetimagefilter.cpp',
        '../gm/ovals.cpp',
        '../gm/patch.cpp',
        '../gm/patchgrid.cpp',
        '../gm/patheffects.cpp',
        '../gm/pathfill.cpp',
        '../gm/pathinterior.cpp',
        '../gm/pathopsinverse.cpp',
        '../gm/pathopsskpclip.cpp',
        '../gm/pathreverse.cpp',
        '../gm/peekpixels.cpp',
        '../gm/perlinnoise.cpp',
        '../gm/picture.cpp',
        '../gm/pictureimagefilter.cpp',
        '../gm/pictureshader.cpp',
        '../gm/pictureshadertile.cpp',
        '../gm/pixelsnap.cpp',
        '../gm/points.cpp',
        '../gm/poly2poly.cpp',
        '../gm/polygons.cpp',
        '../gm/quadpaths.cpp',
        '../gm/recordopts.cpp',
        '../gm/rects.cpp',
        '../gm/repeated_bitmap.cpp',
        '../gm/resizeimagefilter.cpp',
        '../gm/rrect.cpp',
        '../gm/rrects.cpp',
        '../gm/roundrects.cpp',
        '../gm/samplerstress.cpp',
        # '../gm/scalebitmap.cpp',
        '../gm/shaderbounds.cpp',
        '../gm/selftest.cpp',
        '../gm/shadertext.cpp',
        '../gm/shadertext2.cpp',
        '../gm/shadertext3.cpp',
        '../gm/shadows.cpp',
        '../gm/shallowgradient.cpp',
        '../gm/simpleaaclip.cpp',
        '../gm/skbug1719.cpp',
        '../gm/smallarc.cpp',
        '../gm/smallimage.cpp',
        '../gm/spritebitmap.cpp',
        '../gm/srcmode.cpp',
        '../gm/stlouisarch.cpp',
        '../gm/stringart.cpp',
        '../gm/strokefill.cpp',
        '../gm/strokerect.cpp',
        '../gm/strokerects.cpp',
        '../gm/strokes.cpp',
        '../gm/stroketext.cpp',
        '../gm/surface.cpp',
        '../gm/tablecolorfilter.cpp',
        '../gm/texteffects.cpp',
        '../gm/testimagefilters.cpp',
        '../gm/texdata.cpp',
        '../gm/variedtext.cpp',
        '../gm/tallstretchedbitmaps.cpp',
        '../gm/textblob.cpp',
        '../gm/textbloblooper.cpp',
        '../gm/textblobcolortrans.cpp',
        '../gm/textblobshader.cpp',
        '../gm/textblobtransforms.cpp',
        '../gm/texturedomaineffect.cpp',
        '../gm/thinrects.cpp',
        '../gm/thinstrokedrects.cpp',
        '../gm/tiledscaledbitmap.cpp',
        '../gm/tileimagefilter.cpp',
        '../gm/tilemodes.cpp',
        '../gm/tilemodes_scaled.cpp',
        '../gm/tinybitmap.cpp',
        '../gm/transparency.cpp',
        '../gm/twopointradial.cpp',
        '../gm/typeface.cpp',
        '../gm/vertices.cpp',
        '../gm/verttext.cpp',
        '../gm/verttext2.cpp',
        '../gm/verylargebitmap.cpp',
        '../gm/xfermodeimagefilter.cpp',
        '../gm/xfermodes.cpp',
        '../gm/xfermodes2.cpp',
        '../gm/xfermodes3.cpp',
        '../gm/yuvtorgbeffect.cpp',

        # Files needed by particular GMs
        '../src/gpu/GrTestBatch.h',
        '../src/utils/debugger/SkDrawCommand.h',
        '../src/utils/debugger/SkDrawCommand.cpp',
        '../src/utils/debugger/SkDebugCanvas.h',
        '../src/utils/debugger/SkDebugCanvas.cpp',
        '../src/utils/debugger/SkObjectParser.h',
        '../src/utils/debugger/SkObjectParser.cpp',
      ],
    }],
    # TODO: Several GMs are known to cause particular problems on Android, so
    # we disable them on Android.  See http://skbug.com/2326
    [ 'skia_os == "android"', {
      'sources!': [
        # TODO(borenet): Causes assertion failure on Nexus S.
        # See http://skbug.com/705
        '../gm/bitmapcopy.cpp',

        # SOME of the bitmaprect tests are disabled on Android; see
        # ../gm/bitmaprect.cpp

        # We skip GPU tests in this GM; see
        # ../gm/deviceproperties.cpp

        # TODO(bsalomon): Hangs on Xoom and Nexus S. See http://skbug.com/637
        '../gm/drawbitmaprect.cpp',

        # TODO(epoger): Crashes on Nexus 10. See http://skbug.com/2313
        '../gm/imagefilterscropexpand.cpp',

        # TODO(borenet): Causes Nexus S to reboot. See http://skbug.com/665
        '../gm/shadertext.cpp',
        '../gm/shadertext2.cpp',
        '../gm/shadertext3.cpp',

        # TODO(reed): Allocates more memory than Android devices are capable of
        # fulfilling. See http://skbug.com/1978
        '../gm/verylargebitmap.cpp',
      ],
    }],
  ],
}
