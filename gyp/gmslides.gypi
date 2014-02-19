# include this gypi to include all the golden master slides.
{
  'include_dirs': [
    '../gm',
    # include dirs needed by particular GMs
    '../src/utils/debugger',
    '../src/images',
    '../src/lazy',
  ],
  'sources': [
    # base class for GMs
    '../gm/gm.cpp',
    '../gm/gm.h',

    '../gm/aaclip.cpp',
    '../gm/aarectmodes.cpp',
    '../gm/alphagradients.cpp',
    '../gm/androidfallback.cpp',
    '../gm/arcofzorro.cpp',
    '../gm/arithmode.cpp',
    '../gm/beziereffects.cpp',
    '../gm/bicubicfilter.cpp',
    '../gm/bigblurs.cpp',
    '../gm/bigmatrix.cpp',
    '../gm/bigtext.cpp',
    '../gm/bitmapcopy.cpp',
    '../gm/bitmapmatrix.cpp',
    '../gm/bitmapfilters.cpp',
    '../gm/bitmappremul.cpp',
    '../gm/bitmaprect.cpp',
    '../gm/bitmaprecttest.cpp',
    '../gm/bitmapscroll.cpp',
    '../gm/bitmapshader.cpp',
    '../gm/bitmapsource.cpp',
    '../gm/bleed.cpp',
    '../gm/blurs.cpp',
    '../gm/blurquickreject.cpp',
    '../gm/blurrect.cpp',
    '../gm/blurroundrect.cpp',
    '../gm/canvasstate.cpp',
    '../gm/circles.cpp',
    '../gm/circularclips.cpp',
    '../gm/clippedbitmapshaders.cpp',
    '../gm/coloremoji.cpp',
    '../gm/colorfilterimagefilter.cpp',
    '../gm/colorfilters.cpp',
    '../gm/colormatrix.cpp',
    '../gm/colortype.cpp',
    '../gm/complexclip.cpp',
    '../gm/complexclip2.cpp',
    '../gm/composeshader.cpp',
    #'../gm/conicpaths.cpp',
    '../gm/convexpaths.cpp',
    '../gm/convexpolyclip.cpp',
    '../gm/convexpolyeffect.cpp',
    '../gm/copyTo4444.cpp',
    '../gm/cubicpaths.cpp',
    '../gm/cmykjpeg.cpp',
    '../gm/degeneratesegments.cpp',
    '../gm/dashcubics.cpp',
    '../gm/dashing.cpp',
    '../gm/deviceproperties.cpp',
    '../gm/distantclip.cpp',
    '../gm/displacement.cpp',
    '../gm/downsamplebitmap.cpp',
    '../gm/drawbitmaprect.cpp',
    '../gm/drawlooper.cpp',
    '../gm/dropshadowimagefilter.cpp',
    '../gm/extractbitmap.cpp',
    '../gm/emptypath.cpp',
    '../gm/fatpathfill.cpp',
    '../gm/factory.cpp',
    '../gm/filltypes.cpp',
    '../gm/filltypespersp.cpp',
    '../gm/filterbitmap.cpp',
    '../gm/fontcache.cpp',
    '../gm/fontmgr.cpp',
    '../gm/fontscaler.cpp',
    '../gm/gammatext.cpp',
    '../gm/getpostextpath.cpp',
    '../gm/giantbitmap.cpp',
    '../gm/gradients.cpp',
    '../gm/gradients_no_texture.cpp',
    '../gm/gradientDirtyLaundry.cpp',
    '../gm/gradient_matrix.cpp',
    '../gm/gradtext.cpp',
    '../gm/hairlines.cpp',
    '../gm/hairmodes.cpp',
    '../gm/hittestpath.cpp',
    '../gm/imagealphathreshold.cpp',
    '../gm/imageblur.cpp',
    '../gm/imageblurtiled.cpp',
    '../gm/imagemagnifier.cpp',
    # This GM seems to have some issues with rtree and tilegrid; disabled for now.
    #'../gm/imageresizetiled.cpp',
    '../gm/inversepaths.cpp',
    '../gm/lerpmode.cpp',
    '../gm/lighting.cpp',
    '../gm/lumafilter.cpp',
    '../gm/image.cpp',
    '../gm/imagefiltersbase.cpp',
    '../gm/imagefiltersclipped.cpp',
    '../gm/imagefilterscropped.cpp',
    '../gm/imagefiltersgraph.cpp',
    '../gm/imagefiltersscaled.cpp',
    '../gm/internal_links.cpp',
    '../gm/lcdtext.cpp',
    '../gm/linepaths.cpp',
    '../gm/matrixconvolution.cpp',
    '../gm/megalooper.cpp',
    '../gm/mixedxfermodes.cpp',
    '../gm/modecolorfilters.cpp',
    '../gm/morphology.cpp',
    '../gm/nested.cpp',
    '../gm/ninepatchstretch.cpp',
    '../gm/nonclosedpaths.cpp',
    '../gm/offsetimagefilter.cpp',
    '../gm/optimizations.cpp',
    '../gm/ovals.cpp',
    '../gm/patheffects.cpp',
    '../gm/pathfill.cpp',
    '../gm/pathinterior.cpp',
    '../gm/pathopsinverse.cpp',
    '../gm/pathopsskpclip.cpp',
    '../gm/pathreverse.cpp',
    '../gm/peekpixels.cpp',
    '../gm/perlinnoise.cpp',
    '../gm/pictureimagefilter.cpp',
    '../gm/points.cpp',
    '../gm/poly2poly.cpp',
    '../gm/polygons.cpp',
    '../gm/quadpaths.cpp',
    '../gm/rects.cpp',
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
    '../gm/stringart.cpp',
    '../gm/spritebitmap.cpp',
    '../gm/srcmode.cpp',
    '../gm/strokefill.cpp',
    '../gm/strokerect.cpp',
    '../gm/strokerects.cpp',
    '../gm/strokes.cpp',
    '../gm/tablecolorfilter.cpp',
    '../gm/texteffects.cpp',
    '../gm/testimagefilters.cpp',
    '../gm/texdata.cpp',
    '../gm/thinrects.cpp',
    '../gm/thinstrokedrects.cpp',
    '../gm/tileimagefilter.cpp',
    '../gm/tilemodes.cpp',
    '../gm/tilemodes_scaled.cpp',
    '../gm/tinybitmap.cpp',
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

    # Files needed by particular GMs
    '../src/utils/debugger/SkDrawCommand.h',
    '../src/utils/debugger/SkDrawCommand.cpp',
    '../src/utils/debugger/SkDebugCanvas.h',
    '../src/utils/debugger/SkDebugCanvas.cpp',
    '../src/utils/debugger/SkObjectParser.h',
    '../src/utils/debugger/SkObjectParser.cpp',

  ],
}
