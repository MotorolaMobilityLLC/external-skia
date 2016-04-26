
###############################################################################
#
# THIS FILE IS AUTOGENERATED BY GYP_TO_ANDROID.PY. DO NOT EDIT.
#
# For bugs, please contact scroggo@google.com or djsollen@google.com
#
###############################################################################

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CFLAGS += \
	-fPIC \
	-Wno-unused-parameter \
	-U_FORTIFY_SOURCE \
	-D_FORTIFY_SOURCE=1 \
	-DSKIA_IMPLEMENTATION=1 \
	-O2

LOCAL_CPPFLAGS := \
	-std=c++11 \
	-fno-threadsafe-statics

LOCAL_SRC_FILES := \
	DM.cpp \
	DMSrcSink.cpp \
	DMJsonWriter.cpp \
	../gm/gm.cpp \
	../tools/debugger/SkDebugCanvas.cpp \
	../tools/debugger/SkDrawCommand.cpp \
	../tools/debugger/SkObjectParser.cpp \
	../tools/debugger/SkOverdrawMode.cpp \
	../tests/Test.cpp \
	../tests/PathOpsAngleTest.cpp \
	../tests/PathOpsBoundsTest.cpp \
	../tests/PathOpsBuilderConicTest.cpp \
	../tests/PathOpsBuilderTest.cpp \
	../tests/PathOpsBuildUseTest.cpp \
	../tests/PathOpsConicIntersectionTest.cpp \
	../tests/PathOpsConicLineIntersectionTest.cpp \
	../tests/PathOpsCubicConicIntersectionTest.cpp \
	../tests/PathOpsCubicIntersectionTest.cpp \
	../tests/PathOpsCubicIntersectionTestData.cpp \
	../tests/PathOpsCubicLineIntersectionTest.cpp \
	../tests/PathOpsCubicQuadIntersectionTest.cpp \
	../tests/PathOpsCubicReduceOrderTest.cpp \
	../tests/PathOpsDCubicTest.cpp \
	../tests/PathOpsDLineTest.cpp \
	../tests/PathOpsDPointTest.cpp \
	../tests/PathOpsDRectTest.cpp \
	../tests/PathOpsDVectorTest.cpp \
	../tests/PathOpsExtendedTest.cpp \
	../tests/PathOpsFuzz763Test.cpp \
	../tests/PathOpsInverseTest.cpp \
	../tests/PathOpsIssue3651.cpp \
	../tests/PathOpsLineIntersectionTest.cpp \
	../tests/PathOpsLineParametetersTest.cpp \
	../tests/PathOpsOpCircleThreadedTest.cpp \
	../tests/PathOpsOpCubicThreadedTest.cpp \
	../tests/PathOpsOpRectThreadedTest.cpp \
	../tests/PathOpsOpTest.cpp \
	../tests/PathOpsQuadIntersectionTest.cpp \
	../tests/PathOpsQuadIntersectionTestData.cpp \
	../tests/PathOpsQuadLineIntersectionTest.cpp \
	../tests/PathOpsQuadLineIntersectionThreadedTest.cpp \
	../tests/PathOpsQuadReduceOrderTest.cpp \
	../tests/PathOpsSimplifyDegenerateThreadedTest.cpp \
	../tests/PathOpsSimplifyFailTest.cpp \
	../tests/PathOpsSimplifyQuadralateralsThreadedTest.cpp \
	../tests/PathOpsSimplifyQuadThreadedTest.cpp \
	../tests/PathOpsSimplifyRectThreadedTest.cpp \
	../tests/PathOpsSimplifyTest.cpp \
	../tests/PathOpsSimplifyTrianglesThreadedTest.cpp \
	../tests/PathOpsSkpTest.cpp \
	../tests/PathOpsTestCommon.cpp \
	../tests/PathOpsThreadedCommon.cpp \
	../tests/PathOpsThreeWayTest.cpp \
	../tests/PathOpsTightBoundsTest.cpp \
	../tests/PathOpsTypesTest.cpp \
	../tests/SubsetPath.cpp \
	../tests/AAClipTest.cpp \
	../tests/ARGBImageEncoderTest.cpp \
	../tests/AnnotationTest.cpp \
	../tests/AsADashTest.cpp \
	../tests/AtomicTest.cpp \
	../tests/BadIcoTest.cpp \
	../tests/BitSetTest.cpp \
	../tests/BitmapCopyTest.cpp \
	../tests/BitmapGetColorTest.cpp \
	../tests/BitmapHeapTest.cpp \
	../tests/BitmapTest.cpp \
	../tests/BlendTest.cpp \
	../tests/BlitMaskClip.cpp \
	../tests/BlitRowTest.cpp \
	../tests/BlurTest.cpp \
	../tests/CPlusPlusEleven.cpp \
	../tests/CTest.cpp \
	../tests/CachedDataTest.cpp \
	../tests/CachedDecodingPixelRefTest.cpp \
	../tests/CanvasStateHelpers.cpp \
	../tests/CanvasStateTest.cpp \
	../tests/CanvasTest.cpp \
	../tests/ChecksumTest.cpp \
	../tests/ClampRangeTest.cpp \
	../tests/ClearTest.cpp \
	../tests/ClipBoundsTest.cpp \
	../tests/ClipCubicTest.cpp \
	../tests/ClipStackTest.cpp \
	../tests/ClipperTest.cpp \
	../tests/CodecTest.cpp \
	../tests/ColorFilterTest.cpp \
	../tests/ColorMatrixTest.cpp \
	../tests/ColorPrivTest.cpp \
	../tests/ColorSpaceTest.cpp \
	../tests/ColorTest.cpp \
	../tests/CopySurfaceTest.cpp \
	../tests/DashPathEffectTest.cpp \
	../tests/DataRefTest.cpp \
	../tests/DequeTest.cpp \
	../tests/DeviceLooperTest.cpp \
	../tests/DiscardableMemoryPoolTest.cpp \
	../tests/DiscardableMemoryTest.cpp \
	../tests/DrawBitmapRectTest.cpp \
	../tests/DrawFilterTest.cpp \
	../tests/DrawPathTest.cpp \
	../tests/DrawTextTest.cpp \
	../tests/DynamicHashTest.cpp \
	../tests/EGLImageTest.cpp \
	../tests/EmptyPathTest.cpp \
	../tests/ErrorTest.cpp \
	../tests/ExifTest.cpp \
	../tests/FillPathTest.cpp \
	../tests/FitsInTest.cpp \
	../tests/FlattenDrawableTest.cpp \
	../tests/FlattenableCustomFactory.cpp \
	../tests/FlattenableFactoryToName.cpp \
	../tests/Float16Test.cpp \
	../tests/FloatingPointTextureTest.cpp \
	../tests/FontHostStreamTest.cpp \
	../tests/FontHostTest.cpp \
	../tests/FontMgrAndroidParserTest.cpp \
	../tests/FontMgrTest.cpp \
	../tests/FontNamesTest.cpp \
	../tests/FontObjTest.cpp \
	../tests/FrontBufferedStreamTest.cpp \
	../tests/GLProgramsTest.cpp \
	../tests/GeometryTest.cpp \
	../tests/GifTest.cpp \
	../tests/GpuColorFilterTest.cpp \
	../tests/GpuDrawPathTest.cpp \
	../tests/GpuLayerCacheTest.cpp \
	../tests/GpuRectanizerTest.cpp \
	../tests/GrAllocatorTest.cpp \
	../tests/GrContextAbandonTest.cpp \
	../tests/GrContextFactoryTest.cpp \
	../tests/GrDrawTargetTest.cpp \
	../tests/GrGLSLPrettyPrintTest.cpp \
	../tests/GrGetCoeffBlendKnownComponentsTest.cpp \
	../tests/GrMemoryPoolTest.cpp \
	../tests/GrPorterDuffTest.cpp \
	../tests/GrShapeTest.cpp \
	../tests/GrSurfaceTest.cpp \
	../tests/GrTRecorderTest.cpp \
	../tests/GrTextureMipMapInvalidationTest.cpp \
	../tests/GradientTest.cpp \
	../tests/HashTest.cpp \
	../tests/ImageCacheTest.cpp \
	../tests/ImageFilterCacheTest.cpp \
	../tests/ImageFilterTest.cpp \
	../tests/ImageFrom565Bitmap.cpp \
	../tests/ImageGeneratorTest.cpp \
	../tests/ImageIsOpaqueTest.cpp \
	../tests/ImageNewShaderTest.cpp \
	../tests/ImageTest.cpp \
	../tests/IndexedPngOverflowTest.cpp \
	../tests/InfRectTest.cpp \
	../tests/InterpolatorTest.cpp \
	../tests/InvalidIndexedPngTest.cpp \
	../tests/IsClosedSingleContourTest.cpp \
	../tests/LListTest.cpp \
	../tests/LayerDrawLooperTest.cpp \
	../tests/LayerRasterizerTest.cpp \
	../tests/MD5Test.cpp \
	../tests/MallocPixelRefTest.cpp \
	../tests/MaskCacheTest.cpp \
	../tests/MathTest.cpp \
	../tests/Matrix44Test.cpp \
	../tests/MatrixClipCollapseTest.cpp \
	../tests/MatrixTest.cpp \
	../tests/MemoryTest.cpp \
	../tests/MemsetTest.cpp \
	../tests/MessageBusTest.cpp \
	../tests/MetaDataTest.cpp \
	../tests/MipMapTest.cpp \
	../tests/OSPathTest.cpp \
	../tests/OncePtrTest.cpp \
	../tests/OnceTest.cpp \
	../tests/PDFDeflateWStreamTest.cpp \
	../tests/PDFDocumentTest.cpp \
	../tests/PDFGlyphsToUnicodeTest.cpp \
	../tests/PDFInvalidBitmapTest.cpp \
	../tests/PDFJpegEmbedTest.cpp \
	../tests/PDFMetadataAttributeTest.cpp \
	../tests/PDFOpaqueSrcModeToSrcOverTest.cpp \
	../tests/PDFPrimitivesTest.cpp \
	../tests/PackBitsTest.cpp \
	../tests/PaintBreakTextTest.cpp \
	../tests/PaintImageFilterTest.cpp \
	../tests/PaintTest.cpp \
	../tests/ParsePathTest.cpp \
	../tests/PathCoverageTest.cpp \
	../tests/PathMeasureTest.cpp \
	../tests/PathTest.cpp \
	../tests/PictureBBHTest.cpp \
	../tests/PictureShaderTest.cpp \
	../tests/PictureTest.cpp \
	../tests/PixelRefTest.cpp \
	../tests/Point3Test.cpp \
	../tests/PointTest.cpp \
	../tests/PremulAlphaRoundTripTest.cpp \
	../tests/PrimitiveProcessorTest.cpp \
	../tests/QuickRejectTest.cpp \
	../tests/RRectInPathTest.cpp \
	../tests/RTConfRegistryTest.cpp \
	../tests/RTreeTest.cpp \
	../tests/RandomTest.cpp \
	../tests/ReadPixelsTest.cpp \
	../tests/ReadWriteAlphaTest.cpp \
	../tests/Reader32Test.cpp \
	../tests/RecordDrawTest.cpp \
	../tests/RecordOptsTest.cpp \
	../tests/RecordPatternTest.cpp \
	../tests/RecordReplaceDrawTest.cpp \
	../tests/RecordTest.cpp \
	../tests/RecorderTest.cpp \
	../tests/RecordingXfermodeTest.cpp \
	../tests/RectTest.cpp \
	../tests/RectangleTextureTest.cpp \
	../tests/RefCntTest.cpp \
	../tests/RefDictTest.cpp \
	../tests/RegionTest.cpp \
	../tests/ResourceCacheTest.cpp \
	../tests/RoundRectTest.cpp \
	../tests/RuntimeConfigTest.cpp \
	../tests/SRGBReadWritePixelsTest.cpp \
	../tests/SVGDeviceTest.cpp \
	../tests/ScalarTest.cpp \
	../tests/ScaleToSidesTest.cpp \
	../tests/SerializationTest.cpp \
	../tests/ShaderOpacityTest.cpp \
	../tests/SizeTest.cpp \
	../tests/Sk4x4fTest.cpp \
	../tests/SkBase64Test.cpp \
	../tests/SkColor4fTest.cpp \
	../tests/SkImageTest.cpp \
	../tests/SkLinearBitmapPipelineTest.cpp \
	../tests/SkNxTest.cpp \
	../tests/SkResourceCacheTest.cpp \
	../tests/SkSharedMutexTest.cpp \
	../tests/SmallAllocatorTest.cpp \
	../tests/SortTest.cpp \
	../tests/SpecialImageTest.cpp \
	../tests/SpecialSurfaceTest.cpp \
	../tests/SrcOverTest.cpp \
	../tests/StreamTest.cpp \
	../tests/StringTest.cpp \
	../tests/StrokeTest.cpp \
	../tests/StrokerTest.cpp \
	../tests/SurfaceTest.cpp \
	../tests/SwizzlerTest.cpp \
	../tests/TArrayTest.cpp \
	../tests/TDPQueueTest.cpp \
	../tests/TLSTest.cpp \
	../tests/TemplatesTest.cpp \
	../tests/TessellatingPathRendererTests.cpp \
	../tests/TestConfigParsing.cpp \
	../tests/TestTest.cpp \
	../tests/TextBlobCacheTest.cpp \
	../tests/TextBlobTest.cpp \
	../tests/TextureCompressionTest.cpp \
	../tests/Time.cpp \
	../tests/TopoSortTest.cpp \
	../tests/TraceMemoryDumpTest.cpp \
	../tests/TracingTest.cpp \
	../tests/TypefaceTest.cpp \
	../tests/UnicodeTest.cpp \
	../tests/UtilsTest.cpp \
	../tests/VarAllocTest.cpp \
	../tests/VkClearTests.cpp \
	../tests/VkUploadPixelsTests.cpp \
	../tests/VkWrapTests.cpp \
	../tests/WArrayTest.cpp \
	../tests/WritePixelsTest.cpp \
	../tests/Writer32Test.cpp \
	../tests/XfermodeTest.cpp \
	../tests/YUVCacheTest.cpp \
	../tests/YUVTest.cpp \
	../tests/image-bitmap.cpp \
	DMSrcSinkAndroid.cpp \
	../gm/SkLinearBitmapPipelineGM.cpp \
	../gm/aaclip.cpp \
	../gm/aarectmodes.cpp \
	../gm/aaxfermodes.cpp \
	../gm/addarc.cpp \
	../gm/all_bitmap_configs.cpp \
	../gm/alphagradients.cpp \
	../gm/anisotropic.cpp \
	../gm/annotated_text.cpp \
	../gm/arcofzorro.cpp \
	../gm/arcto.cpp \
	../gm/arithmode.cpp \
	../gm/astcbitmap.cpp \
	../gm/badpaint.cpp \
	../gm/beziereffects.cpp \
	../gm/beziers.cpp \
	../gm/bigblurs.cpp \
	../gm/bigmatrix.cpp \
	../gm/bigrrectaaeffect.cpp \
	../gm/bigtext.cpp \
	../gm/bigtileimagefilter.cpp \
	../gm/bitmapcopy.cpp \
	../gm/bitmapfilters.cpp \
	../gm/bitmappremul.cpp \
	../gm/bitmaprect.cpp \
	../gm/bitmaprecttest.cpp \
	../gm/bitmapshader.cpp \
	../gm/bleed.cpp \
	../gm/blend.cpp \
	../gm/blurcircles.cpp \
	../gm/blurquickreject.cpp \
	../gm/blurrect.cpp \
	../gm/blurredclippedcircle.cpp \
	../gm/blurroundrect.cpp \
	../gm/blurs.cpp \
	../gm/bmpfilterqualityrepeat.cpp \
	../gm/bug530095.cpp \
	../gm/cgm.c \
	../gm/cgms.cpp \
	../gm/circles.cpp \
	../gm/circularclips.cpp \
	../gm/clip_strokerect.cpp \
	../gm/clipdrawdraw.cpp \
	../gm/clippedbitmapshaders.cpp \
	../gm/color4f.cpp \
	../gm/colorcube.cpp \
	../gm/coloremoji.cpp \
	../gm/colorfilterimagefilter.cpp \
	../gm/colorfilters.cpp \
	../gm/colormatrix.cpp \
	../gm/colortype.cpp \
	../gm/colortypexfermode.cpp \
	../gm/colorwheel.cpp \
	../gm/complexclip.cpp \
	../gm/complexclip2.cpp \
	../gm/complexclip3.cpp \
	../gm/complexclip_blur_tiled.cpp \
	../gm/composeshader.cpp \
	../gm/concavepaths.cpp \
	../gm/conicpaths.cpp \
	../gm/constcolorprocessor.cpp \
	../gm/convex_all_line_paths.cpp \
	../gm/convexpaths.cpp \
	../gm/convexpolyclip.cpp \
	../gm/convexpolyeffect.cpp \
	../gm/copyTo4444.cpp \
	../gm/cubicpaths.cpp \
	../gm/dashcircle.cpp \
	../gm/dashcubics.cpp \
	../gm/dashing.cpp \
	../gm/dcshader.cpp \
	../gm/degeneratesegments.cpp \
	../gm/dftext.cpp \
	../gm/discard.cpp \
	../gm/displacement.cpp \
	../gm/distantclip.cpp \
	../gm/downsamplebitmap.cpp \
	../gm/draw_bitmap_rect_skbug4374.cpp \
	../gm/drawable.cpp \
	../gm/drawatlas.cpp \
	../gm/drawatlascolor.cpp \
	../gm/drawbitmaprect.cpp \
	../gm/drawfilter.cpp \
	../gm/drawlooper.cpp \
	../gm/drawminibitmaprect.cpp \
	../gm/dropshadowimagefilter.cpp \
	../gm/drrect.cpp \
	../gm/dstreadshuffle.cpp \
	../gm/emboss.cpp \
	../gm/emptypath.cpp \
	../gm/extractbitmap.cpp \
	../gm/factory.cpp \
	../gm/fadefilter.cpp \
	../gm/fatpathfill.cpp \
	../gm/filltypes.cpp \
	../gm/filltypespersp.cpp \
	../gm/filterbitmap.cpp \
	../gm/filterfastbounds.cpp \
	../gm/filterindiabox.cpp \
	../gm/fontcache.cpp \
	../gm/fontmgr.cpp \
	../gm/fontscaler.cpp \
	../gm/fontscalerdistortable.cpp \
	../gm/gamma.cpp \
	../gm/gammatext.cpp \
	../gm/getpostextpath.cpp \
	../gm/giantbitmap.cpp \
	../gm/glyph_pos.cpp \
	../gm/glyph_pos_align.cpp \
	../gm/gradientDirtyLaundry.cpp \
	../gm/gradient_matrix.cpp \
	../gm/gradients.cpp \
	../gm/gradients_2pt_conical.cpp \
	../gm/gradients_no_texture.cpp \
	../gm/gradtext.cpp \
	../gm/grayscalejpg.cpp \
	../gm/hairlines.cpp \
	../gm/hairmodes.cpp \
	../gm/hittestpath.cpp \
	../gm/image.cpp \
	../gm/image_pict.cpp \
	../gm/image_shader.cpp \
	../gm/imagealphathreshold.cpp \
	../gm/imageblur.cpp \
	../gm/imageblur2.cpp \
	../gm/imageblurtiled.cpp \
	../gm/imagefilters.cpp \
	../gm/imagefiltersbase.cpp \
	../gm/imagefiltersclipped.cpp \
	../gm/imagefilterscropexpand.cpp \
	../gm/imagefilterscropped.cpp \
	../gm/imagefiltersgraph.cpp \
	../gm/imagefiltersscaled.cpp \
	../gm/imagefiltersstroked.cpp \
	../gm/imagefilterstransformed.cpp \
	../gm/imagefromyuvtextures.cpp \
	../gm/imagemagnifier.cpp \
	../gm/imageresizetiled.cpp \
	../gm/imagescalealigned.cpp \
	../gm/imagesource.cpp \
	../gm/imagesource2.cpp \
	../gm/imagetoyuvplanes.cpp \
	../gm/internal_links.cpp \
	../gm/inversepaths.cpp \
	../gm/largeglyphblur.cpp \
	../gm/lcdblendmodes.cpp \
	../gm/lcdoverlap.cpp \
	../gm/lcdtext.cpp \
	../gm/lighting.cpp \
	../gm/lightingshader.cpp \
	../gm/linepaths.cpp \
	../gm/localmatriximagefilter.cpp \
	../gm/lumafilter.cpp \
	../gm/matrixconvolution.cpp \
	../gm/matriximagefilter.cpp \
	../gm/megalooper.cpp \
	../gm/mipmap.cpp \
	../gm/mixedtextblobs.cpp \
	../gm/modecolorfilters.cpp \
	../gm/morphology.cpp \
	../gm/multipicturedraw.cpp \
	../gm/nested.cpp \
	../gm/ninepatchstretch.cpp \
	../gm/nonclosedpaths.cpp \
	../gm/offsetimagefilter.cpp \
	../gm/ovals.cpp \
	../gm/patch.cpp \
	../gm/patchgrid.cpp \
	../gm/path_stroke_with_zero_length.cpp \
	../gm/pathcontourstart.cpp \
	../gm/patheffects.cpp \
	../gm/pathfill.cpp \
	../gm/pathinterior.cpp \
	../gm/pathopsinverse.cpp \
	../gm/pathopsskpclip.cpp \
	../gm/pathreverse.cpp \
	../gm/pdf_never_embed.cpp \
	../gm/perlinnoise.cpp \
	../gm/perspshaders.cpp \
	../gm/picture.cpp \
	../gm/pictureimagefilter.cpp \
	../gm/pictureimagegenerator.cpp \
	../gm/pictureshader.cpp \
	../gm/pictureshadertile.cpp \
	../gm/pixelsnap.cpp \
	../gm/plus.cpp \
	../gm/points.cpp \
	../gm/poly2poly.cpp \
	../gm/polygons.cpp \
	../gm/quadpaths.cpp \
	../gm/recordopts.cpp \
	../gm/rectangletexture.cpp \
	../gm/rects.cpp \
	../gm/repeated_bitmap.cpp \
	../gm/resizeimagefilter.cpp \
	../gm/roundrects.cpp \
	../gm/rrect.cpp \
	../gm/rrects.cpp \
	../gm/samplerstress.cpp \
	../gm/scaledstrokes.cpp \
	../gm/selftest.cpp \
	../gm/shaderbounds.cpp \
	../gm/shadertext.cpp \
	../gm/shadertext2.cpp \
	../gm/shadertext3.cpp \
	../gm/shadows.cpp \
	../gm/shallowgradient.cpp \
	../gm/shapes.cpp \
	../gm/showmiplevels.cpp \
	../gm/simpleaaclip.cpp \
	../gm/skbug1719.cpp \
	../gm/skbug_257.cpp \
	../gm/skbug_4868.cpp \
	../gm/smallarc.cpp \
	../gm/smallimage.cpp \
	../gm/smallpaths.cpp \
	../gm/spritebitmap.cpp \
	../gm/srcmode.cpp \
	../gm/stlouisarch.cpp \
	../gm/stringart.cpp \
	../gm/strokefill.cpp \
	../gm/strokerect.cpp \
	../gm/strokerects.cpp \
	../gm/strokes.cpp \
	../gm/stroketext.cpp \
	../gm/subsetshader.cpp \
	../gm/surface.cpp \
	../gm/tablecolorfilter.cpp \
	../gm/tallstretchedbitmaps.cpp \
	../gm/testimagefilters.cpp \
	../gm/texdata.cpp \
	../gm/textblob.cpp \
	../gm/textblobblockreordering.cpp \
	../gm/textblobcolortrans.cpp \
	../gm/textblobgeometrychange.cpp \
	../gm/textbloblooper.cpp \
	../gm/textblobmixedsizes.cpp \
	../gm/textblobrandomfont.cpp \
	../gm/textblobshader.cpp \
	../gm/textblobtransforms.cpp \
	../gm/textblobuseaftergpufree.cpp \
	../gm/texteffects.cpp \
	../gm/texturedomaineffect.cpp \
	../gm/thinrects.cpp \
	../gm/thinstrokedrects.cpp \
	../gm/tiledscaledbitmap.cpp \
	../gm/tileimagefilter.cpp \
	../gm/tilemodes.cpp \
	../gm/tilemodes_scaled.cpp \
	../gm/tinybitmap.cpp \
	../gm/transparency.cpp \
	../gm/typeface.cpp \
	../gm/variedtext.cpp \
	../gm/vertices.cpp \
	../gm/verttext.cpp \
	../gm/verttext2.cpp \
	../gm/verylargebitmap.cpp \
	../gm/xfermodeimagefilter.cpp \
	../gm/xfermodes.cpp \
	../gm/xfermodes2.cpp \
	../gm/xfermodes3.cpp \
	../gm/yuvtorgbeffect.cpp \
	../tools/AndroidSkDebugToStdOut.cpp \
	../tools/flags/SkCommandLineFlags.cpp \
	../src/svg/SkSVGCanvas.cpp \
	../src/svg/SkSVGDevice.cpp \
	../tools/CrashHandler.cpp \
	../tools/picture_utils.cpp \
	../tools/ProcStats.cpp \
	../tools/sk_tool_utils.cpp \
	../tools/sk_tool_utils_font.cpp \
	../tools/random_parse_path.cpp \
	../tools/UrlDataManager.cpp \
	../tools/timer/Timer.cpp \
	../src/xml/SkDOM.cpp \
	../src/xml/SkXMLParser.cpp \
	../src/xml/SkXMLWriter.cpp \
	../src/xps/SkDocument_XPS_None.cpp \
	../tools/Resources.cpp \
	../experimental/SkSetPoly3To3.cpp \
	../experimental/SkSetPoly3To3_A.cpp \
	../experimental/SkSetPoly3To3_D.cpp \
	../tools/flags/SkCommonFlags.cpp \
	../tools/flags/SkCommonFlagsConfig.cpp \
	../tools/android/SkAndroidSDKCanvas.cpp \
	../tools/gpu/GrContextFactory.cpp \
	../tools/gpu/GrTest.cpp \
	../tools/gpu/gl/GLTestContext.cpp \
	../tools/gpu/gl/debug/DebugGLTestContext.cpp \
	../tools/gpu/gl/debug/GrBufferObj.cpp \
	../tools/gpu/gl/debug/GrFrameBufferObj.cpp \
	../tools/gpu/gl/debug/GrProgramObj.cpp \
	../tools/gpu/gl/debug/GrShaderObj.cpp \
	../tools/gpu/gl/debug/GrTextureObj.cpp \
	../tools/gpu/gl/debug/GrTextureUnitObj.cpp \
	../tools/gpu/gl/null/NullGLTestContext.cpp \
	../tools/gpu/gl/egl/CreatePlatformGLTestContext_egl.cpp

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libpng \
	libz \
	libexpat \
	libGLESv2 \
	libEGL

LOCAL_STATIC_LIBRARIES := \
	libskia_static \
	libhwui_static \
	libjsoncpp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include/c \
	$(LOCAL_PATH)/../include/config \
	$(LOCAL_PATH)/../include/core \
	$(LOCAL_PATH)/../include/pathops \
	$(LOCAL_PATH)/../include/codec \
	$(LOCAL_PATH)/../include/android \
	$(LOCAL_PATH)/../include/effects \
	$(LOCAL_PATH)/../include/client/android \
	$(LOCAL_PATH)/../include/images \
	$(LOCAL_PATH)/../include/ports \
	$(LOCAL_PATH)/../src/sfnt \
	$(LOCAL_PATH)/../include/utils \
	$(LOCAL_PATH)/../src/utils \
	$(LOCAL_PATH)/../include/gpu \
	$(LOCAL_PATH)/../include/private \
	$(LOCAL_PATH)/../src/core \
	$(LOCAL_PATH)/../tools/gpu \
	$(LOCAL_PATH)/../include/svg \
	$(LOCAL_PATH)/../include/xml \
	$(LOCAL_PATH)/../src/fonts \
	$(LOCAL_PATH)/../tools \
	$(LOCAL_PATH)/../tools/flags \
	$(LOCAL_PATH)/../src/gpu \
	$(LOCAL_PATH)/../gm \
	$(LOCAL_PATH)/../src/effects \
	$(LOCAL_PATH)/../src/images \
	$(LOCAL_PATH)/../src/lazy \
	$(LOCAL_PATH)/../tools/debugger \
	$(LOCAL_PATH)/../tests \
	$(LOCAL_PATH)/../src/effects/gradients \
	$(LOCAL_PATH)/../src/pathops \
	$(LOCAL_PATH)/../src/pipe/utils \
	$(LOCAL_PATH)/../src/codec \
	$(LOCAL_PATH)/../src/image \
	$(LOCAL_PATH)/../src/pdf \
	$(LOCAL_PATH)/../src/ports \
	$(LOCAL_PATH)/../third_party/etc1 \
	external/libpng \
	$(LOCAL_PATH)/../tools/timer \
	$(LOCAL_PATH)/../experimental \
	external/zlib \
	$(LOCAL_PATH)/../tools/android

LOCAL_CFLAGS += \
	-DSK_CRASH_HANDLER

LOCAL_MODULE_TAGS := \
	tests

LOCAL_MODULE := \
	skia_dm


# Store skia's resources in the directory structure that the Android testing
# infrastructure expects.  This requires that Skia maintain a symlinked
# subdirectory in the DATA folder that points to the top level skia resources...
#  i.e. external/skia/DATA/skia_resources --> ../resources
LOCAL_PICKUP_FILES := $(LOCAL_PATH)/../DATA
include $(LOCAL_PATH)/../skia_static_deps.mk
include frameworks/base/libs/hwui/hwui_static_deps.mk
include $(BUILD_NATIVE_TEST)
