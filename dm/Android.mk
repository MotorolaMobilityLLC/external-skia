
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
	-DSKIA_IMPLEMENTATION=1

LOCAL_CPPFLAGS := \
	-std=c++11 \
	-Wno-invalid-offsetof

LOCAL_SRC_FILES := \
	DM.cpp \
	DMSrcSink.cpp \
	DMJsonWriter.cpp \
	../gm/gm.cpp \
	../src/pipe/utils/SamplePipeControllers.cpp \
	../src/utils/debugger/SkDebugCanvas.cpp \
	../src/utils/debugger/SkDrawCommand.cpp \
	../src/utils/debugger/SkObjectParser.cpp \
	../tests/Test.cpp \
	../tests/PathOpsAngleTest.cpp \
	../tests/PathOpsBoundsTest.cpp \
	../tests/PathOpsCubicIntersectionTest.cpp \
	../tests/PathOpsCubicIntersectionTestData.cpp \
	../tests/PathOpsCubicLineIntersectionTest.cpp \
	../tests/PathOpsCubicQuadIntersectionTest.cpp \
	../tests/PathOpsCubicReduceOrderTest.cpp \
	../tests/PathOpsCubicToQuadsTest.cpp \
	../tests/PathOpsDCubicTest.cpp \
	../tests/PathOpsDLineTest.cpp \
	../tests/PathOpsDPointTest.cpp \
	../tests/PathOpsDQuadTest.cpp \
	../tests/PathOpsDRectTest.cpp \
	../tests/PathOpsDTriangleTest.cpp \
	../tests/PathOpsDVectorTest.cpp \
	../tests/PathOpsExtendedTest.cpp \
	../tests/PathOpsFuzz763Test.cpp \
	../tests/PathOpsInverseTest.cpp \
	../tests/PathOpsLineIntersectionTest.cpp \
	../tests/PathOpsLineParametetersTest.cpp \
	../tests/PathOpsOpCubicThreadedTest.cpp \
	../tests/PathOpsOpRectThreadedTest.cpp \
	../tests/PathOpsOpTest.cpp \
	../tests/PathOpsQuadIntersectionTest.cpp \
	../tests/PathOpsQuadIntersectionTestData.cpp \
	../tests/PathOpsQuadLineIntersectionTest.cpp \
	../tests/PathOpsQuadLineIntersectionThreadedTest.cpp \
	../tests/PathOpsQuadParameterizationTest.cpp \
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
	../tests/PathOpsTightBoundsTest.cpp \
	../tests/AAClipTest.cpp \
	../tests/ARGBImageEncoderTest.cpp \
	../tests/AnnotationTest.cpp \
	../tests/AsADashTest.cpp \
	../tests/AtomicTest.cpp \
	../tests/BadIcoTest.cpp \
	../tests/BitSetTest.cpp \
	../tests/BitmapCopyTest.cpp \
	../tests/BitmapGetColorTest.cpp \
	../tests/BitmapHasherTest.cpp \
	../tests/BitmapHeapTest.cpp \
	../tests/BitmapTest.cpp \
	../tests/BlendTest.cpp \
	../tests/BlitRowTest.cpp \
	../tests/BlurTest.cpp \
	../tests/CTest.cpp \
	../tests/CachedDataTest.cpp \
	../tests/CachedDecodingPixelRefTest.cpp \
	../tests/CanvasStateHelpers.cpp \
	../tests/CanvasStateTest.cpp \
	../tests/CanvasTest.cpp \
	../tests/ChecksumTest.cpp \
	../tests/ClampRangeTest.cpp \
	../tests/ClipCacheTest.cpp \
	../tests/ClipCubicTest.cpp \
	../tests/ClipStackTest.cpp \
	../tests/ClipperTest.cpp \
	../tests/ColorFilterTest.cpp \
	../tests/ColorPrivTest.cpp \
	../tests/ColorTest.cpp \
	../tests/CPlusPlusEleven.cpp \
	../tests/DashPathEffectTest.cpp \
	../tests/DataRefTest.cpp \
	../tests/DeferredCanvasTest.cpp \
	../tests/DeflateWStream.cpp \
	../tests/DequeTest.cpp \
	../tests/DeviceLooperTest.cpp \
	../tests/DiscardableMemoryPoolTest.cpp \
	../tests/DiscardableMemoryTest.cpp \
	../tests/DocumentTest.cpp \
	../tests/DrawBitmapRectTest.cpp \
	../tests/DrawPathTest.cpp \
	../tests/DrawTextTest.cpp \
	../tests/DynamicHashTest.cpp \
	../tests/EmptyPathTest.cpp \
	../tests/ErrorTest.cpp \
	../tests/FillPathTest.cpp \
	../tests/FitsInTest.cpp \
	../tests/FlateTest.cpp \
	../tests/FloatingPointTextureTest.cpp \
	../tests/FontHostStreamTest.cpp \
	../tests/FontHostTest.cpp \
	../tests/FontMgrTest.cpp \
	../tests/FontNamesTest.cpp \
	../tests/FontObjTest.cpp \
	../tests/FrontBufferedStreamTest.cpp \
	../tests/GLInterfaceValidationTest.cpp \
	../tests/GLProgramsTest.cpp \
	../tests/GeometryTest.cpp \
	../tests/GifTest.cpp \
	../tests/GpuColorFilterTest.cpp \
	../tests/GpuDrawPathTest.cpp \
	../tests/GpuLayerCacheTest.cpp \
	../tests/GpuRectanizerTest.cpp \
	../tests/GrContextFactoryTest.cpp \
	../tests/GrDrawTargetTest.cpp \
	../tests/GrAllocatorTest.cpp \
	../tests/GrMemoryPoolTest.cpp \
	../tests/GrOrderedSetTest.cpp \
	../tests/GrGLSLPrettyPrintTest.cpp \
	../tests/GrRedBlackTreeTest.cpp \
	../tests/GrSurfaceTest.cpp \
	../tests/GrTBSearchTest.cpp \
	../tests/GrTRecorderTest.cpp \
	../tests/GradientTest.cpp \
	../tests/HashTest.cpp \
	../tests/ImageCacheTest.cpp \
	../tests/ImageDecodingTest.cpp \
	../tests/ImageFilterTest.cpp \
	../tests/ImageGeneratorTest.cpp \
	../tests/ImageIsOpaqueTest.cpp \
	../tests/ImageNewShaderTest.cpp \
	../tests/InfRectTest.cpp \
	../tests/InterpolatorTest.cpp \
	../tests/InvalidIndexedPngTest.cpp \
	../tests/JpegTest.cpp \
	../tests/KtxTest.cpp \
	../tests/LListTest.cpp \
	../tests/LayerDrawLooperTest.cpp \
	../tests/LayerRasterizerTest.cpp \
	../tests/LazyPtrTest.cpp \
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
	../tests/NameAllocatorTest.cpp \
	../tests/OSPathTest.cpp \
	../tests/OnceTest.cpp \
	../tests/PDFInvalidBitmapTest.cpp \
	../tests/PDFJpegEmbedTest.cpp \
	../tests/PDFPrimitivesTest.cpp \
	../tests/PMFloatTest.cpp \
	../tests/PackBitsTest.cpp \
	../tests/PaintTest.cpp \
	../tests/ParsePathTest.cpp \
	../tests/PathCoverageTest.cpp \
	../tests/PathMeasureTest.cpp \
	../tests/PathTest.cpp \
	../tests/PathUtilsTest.cpp \
	../tests/PictureBBHTest.cpp \
	../tests/PictureShaderTest.cpp \
	../tests/PictureTest.cpp \
	../tests/PixelRefTest.cpp \
	../tests/PointTest.cpp \
	../tests/PremulAlphaRoundTripTest.cpp \
	../tests/QuickRejectTest.cpp \
	../tests/RTConfRegistryTest.cpp \
	../tests/RTreeTest.cpp \
	../tests/RandomTest.cpp \
	../tests/ReadPixelsTest.cpp \
	../tests/ReadWriteAlphaTest.cpp \
	../tests/Reader32Test.cpp \
	../tests/RecordDrawTest.cpp \
	../tests/RecordReplaceDrawTest.cpp \
	../tests/RecordOptsTest.cpp \
	../tests/RecordPatternTest.cpp \
	../tests/RecordTest.cpp \
	../tests/RecorderTest.cpp \
	../tests/RecordingXfermodeTest.cpp \
	../tests/RectTest.cpp \
	../tests/RefCntTest.cpp \
	../tests/RefDictTest.cpp \
	../tests/RegionTest.cpp \
	../tests/ResourceCacheTest.cpp \
	../tests/RoundRectTest.cpp \
	../tests/RuntimeConfigTest.cpp \
	../tests/SHA1Test.cpp \
	../tests/ScalarTest.cpp \
	../tests/SerializationTest.cpp \
	../tests/ShaderImageFilterTest.cpp \
	../tests/ShaderOpacityTest.cpp \
	../tests/SizeTest.cpp \
	../tests/Sk4xTest.cpp \
	../tests/SkBase64Test.cpp \
	../tests/SkImageTest.cpp \
	../tests/SkResourceCacheTest.cpp \
	../tests/SmallAllocatorTest.cpp \
	../tests/SortTest.cpp \
	../tests/SrcOverTest.cpp \
	../tests/StreamTest.cpp \
	../tests/StringTest.cpp \
	../tests/StrokeTest.cpp \
	../tests/StrokerTest.cpp \
	../tests/SurfaceTest.cpp \
	../tests/SVGDeviceTest.cpp \
	../tests/TessellatingPathRendererTests.cpp \
	../tests/TArrayTest.cpp \
	../tests/TDPQueueTest.cpp \
	../tests/Time.cpp \
	../tests/TLSTest.cpp \
	../tests/TSetTest.cpp \
	../tests/TextBlobTest.cpp \
	../tests/TextureCompressionTest.cpp \
	../tests/ToUnicodeTest.cpp \
	../tests/TracingTest.cpp \
	../tests/TypefaceTest.cpp \
	../tests/UnicodeTest.cpp \
	../tests/UtilsTest.cpp \
	../tests/VarAllocTest.cpp \
	../tests/WArrayTest.cpp \
	../tests/WritePixelsTest.cpp \
	../tests/Writer32Test.cpp \
	../tests/XfermodeTest.cpp \
	../tests/YUVCacheTest.cpp \
	../tests/PipeTest.cpp \
	../tests/TDStackNesterTest.cpp \
	DMSrcSinkAndroid.cpp \
	../gm/aaclip.cpp \
	../gm/aarectmodes.cpp \
	../gm/addarc.cpp \
	../gm/alphagradients.cpp \
	../gm/arcofzorro.cpp \
	../gm/arithmode.cpp \
	../gm/astcbitmap.cpp \
	../gm/beziereffects.cpp \
	../gm/beziers.cpp \
	../gm/bigblurs.cpp \
	../gm/bigmatrix.cpp \
	../gm/bigtext.cpp \
	../gm/bitmapfilters.cpp \
	../gm/bitmappremul.cpp \
	../gm/bitmaprect.cpp \
	../gm/bitmaprecttest.cpp \
	../gm/bitmapscroll.cpp \
	../gm/bitmapshader.cpp \
	../gm/bitmapsource.cpp \
	../gm/bleed.cpp \
	../gm/blurcircles.cpp \
	../gm/blurs.cpp \
	../gm/blurquickreject.cpp \
	../gm/blurrect.cpp \
	../gm/blurroundrect.cpp \
	../gm/circles.cpp \
	../gm/circularclips.cpp \
	../gm/clipdrawdraw.cpp \
	../gm/clip_strokerect.cpp \
	../gm/clippedbitmapshaders.cpp \
	../gm/cgms.cpp \
	../gm/cgm.c \
	../gm/colorcube.cpp \
	../gm/coloremoji.cpp \
	../gm/colorfilterimagefilter.cpp \
	../gm/colorfilters.cpp \
	../gm/colormatrix.cpp \
	../gm/colortype.cpp \
	../gm/colortypexfermode.cpp \
	../gm/colorwheel.cpp \
	../gm/concavepaths.cpp \
	../gm/complexclip.cpp \
	../gm/complexclip2.cpp \
	../gm/complexclip3.cpp \
	../gm/composeshader.cpp \
	../gm/conicpaths.cpp \
	../gm/convexpaths.cpp \
	../gm/convexpolyclip.cpp \
	../gm/convexpolyeffect.cpp \
	../gm/copyTo4444.cpp \
	../gm/cubicpaths.cpp \
	../gm/cmykjpeg.cpp \
	../gm/dstreadshuffle.cpp \
	../gm/degeneratesegments.cpp \
	../gm/dcshader.cpp \
	../gm/discard.cpp \
	../gm/dashcubics.cpp \
	../gm/dashing.cpp \
	../gm/distantclip.cpp \
	../gm/dftext.cpp \
	../gm/displacement.cpp \
	../gm/downsamplebitmap.cpp \
	../gm/drawfilter.cpp \
	../gm/drawlooper.cpp \
	../gm/dropshadowimagefilter.cpp \
	../gm/drrect.cpp \
	../gm/etc1bitmap.cpp \
	../gm/extractbitmap.cpp \
	../gm/emboss.cpp \
	../gm/emptypath.cpp \
	../gm/fatpathfill.cpp \
	../gm/factory.cpp \
	../gm/filltypes.cpp \
	../gm/filltypespersp.cpp \
	../gm/filterbitmap.cpp \
	../gm/filterfastbounds.cpp \
	../gm/filterindiabox.cpp \
	../gm/fontcache.cpp \
	../gm/fontmgr.cpp \
	../gm/fontscaler.cpp \
	../gm/gammatext.cpp \
	../gm/getpostextpath.cpp \
	../gm/giantbitmap.cpp \
	../gm/glyph_pos.cpp \
	../gm/glyph_pos_align.cpp \
	../gm/gradients.cpp \
	../gm/gradients_2pt_conical.cpp \
	../gm/gradients_no_texture.cpp \
	../gm/gradientDirtyLaundry.cpp \
	../gm/gradient_matrix.cpp \
	../gm/gradtext.cpp \
	../gm/grayscalejpg.cpp \
	../gm/hairlines.cpp \
	../gm/hairmodes.cpp \
	../gm/hittestpath.cpp \
	../gm/imagealphathreshold.cpp \
	../gm/imageblur.cpp \
	../gm/imageblur2.cpp \
	../gm/imageblurtiled.cpp \
	../gm/imagemagnifier.cpp \
	../gm/imageresizetiled.cpp \
	../gm/inversepaths.cpp \
	../gm/lerpmode.cpp \
	../gm/lighting.cpp \
	../gm/lumafilter.cpp \
	../gm/image.cpp \
	../gm/imagefiltersbase.cpp \
	../gm/imagefiltersclipped.cpp \
	../gm/imagefilterscropped.cpp \
	../gm/imagefiltersgraph.cpp \
	../gm/imagefiltersscaled.cpp \
	../gm/internal_links.cpp \
	../gm/lcdtext.cpp \
	../gm/linepaths.cpp \
	../gm/matrixconvolution.cpp \
	../gm/matriximagefilter.cpp \
	../gm/megalooper.cpp \
	../gm/mixedxfermodes.cpp \
	../gm/mipmap.cpp \
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
	../gm/patheffects.cpp \
	../gm/pathfill.cpp \
	../gm/pathinterior.cpp \
	../gm/pathopsinverse.cpp \
	../gm/pathopsskpclip.cpp \
	../gm/pathreverse.cpp \
	../gm/peekpixels.cpp \
	../gm/perlinnoise.cpp \
	../gm/picture.cpp \
	../gm/pictureimagefilter.cpp \
	../gm/pictureshader.cpp \
	../gm/pictureshadertile.cpp \
	../gm/points.cpp \
	../gm/poly2poly.cpp \
	../gm/polygons.cpp \
	../gm/quadpaths.cpp \
	../gm/recordopts.cpp \
	../gm/rects.cpp \
	../gm/repeated_bitmap.cpp \
	../gm/resizeimagefilter.cpp \
	../gm/rrect.cpp \
	../gm/rrects.cpp \
	../gm/roundrects.cpp \
	../gm/samplerstress.cpp \
	../gm/shaderbounds.cpp \
	../gm/selftest.cpp \
	../gm/shadows.cpp \
	../gm/shallowgradient.cpp \
	../gm/simpleaaclip.cpp \
	../gm/skbug1719.cpp \
	../gm/smallarc.cpp \
	../gm/smallimage.cpp \
	../gm/spritebitmap.cpp \
	../gm/srcmode.cpp \
	../gm/stlouisarch.cpp \
	../gm/stringart.cpp \
	../gm/strokefill.cpp \
	../gm/strokerect.cpp \
	../gm/strokerects.cpp \
	../gm/strokes.cpp \
	../gm/stroketext.cpp \
	../gm/surface.cpp \
	../gm/tablecolorfilter.cpp \
	../gm/texteffects.cpp \
	../gm/testimagefilters.cpp \
	../gm/texdata.cpp \
	../gm/variedtext.cpp \
	../gm/tallstretchedbitmaps.cpp \
	../gm/textblob.cpp \
	../gm/textblobshader.cpp \
	../gm/texturedomaineffect.cpp \
	../gm/thinrects.cpp \
	../gm/thinstrokedrects.cpp \
	../gm/tiledscaledbitmap.cpp \
	../gm/tileimagefilter.cpp \
	../gm/tilemodes.cpp \
	../gm/tilemodes_scaled.cpp \
	../gm/tinybitmap.cpp \
	../gm/transparency.cpp \
	../gm/twopointradial.cpp \
	../gm/typeface.cpp \
	../gm/vertices.cpp \
	../gm/verttext.cpp \
	../gm/verttext2.cpp \
	../gm/xfermodeimagefilter.cpp \
	../gm/xfermodes.cpp \
	../gm/xfermodes2.cpp \
	../gm/xfermodes3.cpp \
	../gm/yuvtorgbeffect.cpp \
	../tests/FontConfigParser.cpp \
	../tools/AndroidSkDebugToStdOut.cpp \
	../tools/flags/SkCommandLineFlags.cpp \
	../src/svg/SkSVGCanvas.cpp \
	../src/svg/SkSVGDevice.cpp \
	../tools/CrashHandler.cpp \
	../tools/ProcStats.cpp \
	../tools/sk_tool_utils.cpp \
	../tools/sk_tool_utils_font.cpp \
	../tools/timer/Timer.cpp \
	../tools/timer/TimerData.cpp \
	../tools/timer/GpuTimer.cpp \
	../tools/timer/SysTimer_posix.cpp \
	../src/xml/SkBML_XMLParser.cpp \
	../src/xml/SkDOM.cpp \
	../src/xml/SkXMLParser.cpp \
	../src/xml/SkXMLWriter.cpp \
	../src/doc/SkDocument_XPS_None.cpp \
	../tools/Resources.cpp \
	../experimental/SkSetPoly3To3.cpp \
	../experimental/SkSetPoly3To3_A.cpp \
	../experimental/SkSetPoly3To3_D.cpp \
	../tools/flags/SkCommonFlags.cpp \
	../tools/picture_utils.cpp \
	../src/gpu/GrContextFactory.cpp \
	../src/gpu/GrTest.cpp

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libskia \
	libandroid \
	libgui \
	libhwui \
	libutils \
	libdl \
	libGLESv2 \
	libEGL \
	libz

LOCAL_STATIC_LIBRARIES := \
	libjsoncpp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include/c \
	$(LOCAL_PATH)/../include/config \
	$(LOCAL_PATH)/../include/core \
	$(LOCAL_PATH)/../include/pathops \
	$(LOCAL_PATH)/../include/pipe \
	$(LOCAL_PATH)/../include/codec \
	$(LOCAL_PATH)/../include/effects \
	$(LOCAL_PATH)/../include/images \
	$(LOCAL_PATH)/../include/ports \
	$(LOCAL_PATH)/../src/sfnt \
	$(LOCAL_PATH)/../include/utils \
	$(LOCAL_PATH)/../src/utils \
	$(LOCAL_PATH)/../include/gpu \
	$(LOCAL_PATH)/../src/core \
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
	$(LOCAL_PATH)/../src/pipe/utils \
	$(LOCAL_PATH)/../src/utils/debugger \
	$(LOCAL_PATH)/../tests \
	$(LOCAL_PATH)/../src/pathops \
	$(LOCAL_PATH)/../src/image \
	$(LOCAL_PATH)/../src/pdf \
	$(LOCAL_PATH)/../experimental/PdfViewer \
	$(LOCAL_PATH)/../experimental/PdfViewer/src \
	$(LOCAL_PATH)/../../../frameworks/base/libs/hwui \
	$(LOCAL_PATH)/../../../frameworks/native/include \
	$(LOCAL_PATH)/../src/ports \
	$(LOCAL_PATH)/../third_party/etc1 \
	$(LOCAL_PATH)/../tools/timer \
	$(LOCAL_PATH)/../experimental

LOCAL_CFLAGS += \
	-DSK_CRASH_HANDLER

LOCAL_MODULE_TAGS := \
	tests

LOCAL_MODULE := \
	skia_dm


# Setup directory to store skia's resources in the directory structure that
# the Android testing infrastructure expects
skia_res_dir := $(call intermediates-dir-for,PACKAGING,skia_resources)/DATA
$(shell mkdir -p $(skia_res_dir))
$(shell cp -r $(LOCAL_PATH)/../resources/. $(skia_res_dir)/skia_resources)
LOCAL_PICKUP_FILES := $(skia_res_dir)
skia_res_dir :=

include $(BUILD_NATIVE_TEST)
