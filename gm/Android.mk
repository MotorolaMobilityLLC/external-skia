
###############################################################################
#
# THIS FILE IS AUTOGENERATED BY GYP_TO_ANDROID.PY. DO NOT EDIT.
#
###############################################################################

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CFLAGS += \
	-fPIC \
	-Wno-c++11-extensions \
	-Wno-unused-parameter \
	-U_FORTIFY_SOURCE \
	-D_FORTIFY_SOURCE=1

LOCAL_CPPFLAGS := \
	-Wno-invalid-offsetof

LOCAL_SRC_FILES := \
	gm.cpp \
	gmmain.cpp \
	system_preferences_default.cpp \
	../src/pipe/utils/SamplePipeControllers.cpp \
	aaclip.cpp \
	aarectmodes.cpp \
	alphagradients.cpp \
	arcofzorro.cpp \
	arithmode.cpp \
	astcbitmap.cpp \
	beziereffects.cpp \
	beziers.cpp \
	bigblurs.cpp \
	bigmatrix.cpp \
	bigtext.cpp \
	bitmapmatrix.cpp \
	bitmapfilters.cpp \
	bitmappremul.cpp \
	bitmaprect.cpp \
	bitmaprecttest.cpp \
	bitmapscroll.cpp \
	bitmapshader.cpp \
	bitmapsource.cpp \
	bleed.cpp \
	blurcircles.cpp \
	blurs.cpp \
	blurquickreject.cpp \
	blurrect.cpp \
	blurroundrect.cpp \
	circles.cpp \
	circularclips.cpp \
	clip_strokerect.cpp \
	clippedbitmapshaders.cpp \
	cgms.cpp \
	cgm.c \
	colorcube.cpp \
	coloremoji.cpp \
	colorfilterimagefilter.cpp \
	colorfilters.cpp \
	colormatrix.cpp \
	colortype.cpp \
	colortypexfermode.cpp \
	colorwheel.cpp \
	complexclip.cpp \
	complexclip2.cpp \
	composeshader.cpp \
	convexpaths.cpp \
	convexpolyclip.cpp \
	convexpolyeffect.cpp \
	copyTo4444.cpp \
	cubicpaths.cpp \
	cmykjpeg.cpp \
	degeneratesegments.cpp \
	discard.cpp \
	dashcubics.cpp \
	dashing.cpp \
	distantclip.cpp \
	dftext.cpp \
	displacement.cpp \
	downsamplebitmap.cpp \
	drawlooper.cpp \
	dropshadowimagefilter.cpp \
	drrect.cpp \
	etc1bitmap.cpp \
	extractbitmap.cpp \
	emboss.cpp \
	emptypath.cpp \
	fatpathfill.cpp \
	factory.cpp \
	filltypes.cpp \
	filltypespersp.cpp \
	filterbitmap.cpp \
	filterfastbounds.cpp \
	filterindiabox.cpp \
	fontcache.cpp \
	fontmgr.cpp \
	fontscaler.cpp \
	gammatext.cpp \
	getpostextpath.cpp \
	giantbitmap.cpp \
	glyph_pos.cpp \
	glyph_pos_align.cpp \
	gradients.cpp \
	gradients_2pt_conical.cpp \
	gradients_no_texture.cpp \
	gradientDirtyLaundry.cpp \
	gradient_matrix.cpp \
	gradtext.cpp \
	grayscalejpg.cpp \
	hairlines.cpp \
	hairmodes.cpp \
	hittestpath.cpp \
	imagealphathreshold.cpp \
	imageblur.cpp \
	imageblur2.cpp \
	imageblurtiled.cpp \
	imagemagnifier.cpp \
	imageresizetiled.cpp \
	inversepaths.cpp \
	lerpmode.cpp \
	lighting.cpp \
	lumafilter.cpp \
	image.cpp \
	imagefiltersbase.cpp \
	imagefiltersclipped.cpp \
	imagefilterscropped.cpp \
	imagefiltersgraph.cpp \
	imagefiltersscaled.cpp \
	internal_links.cpp \
	lcdtext.cpp \
	linepaths.cpp \
	matrixconvolution.cpp \
	matriximagefilter.cpp \
	megalooper.cpp \
	mixedxfermodes.cpp \
	modecolorfilters.cpp \
	morphology.cpp \
	multipicturedraw.cpp \
	nested.cpp \
	ninepatchstretch.cpp \
	nonclosedpaths.cpp \
	offsetimagefilter.cpp \
	ovals.cpp \
	patch.cpp \
	patchgrid.cpp \
	patheffects.cpp \
	pathfill.cpp \
	pathinterior.cpp \
	pathopsinverse.cpp \
	pathopsskpclip.cpp \
	pathreverse.cpp \
	peekpixels.cpp \
	perlinnoise.cpp \
	picture.cpp \
	pictureimagefilter.cpp \
	pictureshader.cpp \
	pictureshadertile.cpp \
	points.cpp \
	poly2poly.cpp \
	polygons.cpp \
	quadpaths.cpp \
	rects.cpp \
	resizeimagefilter.cpp \
	rrect.cpp \
	rrects.cpp \
	roundrects.cpp \
	samplerstress.cpp \
	shaderbounds.cpp \
	selftest.cpp \
	shadows.cpp \
	shallowgradient.cpp \
	simpleaaclip.cpp \
	skbug1719.cpp \
	smallarc.cpp \
	smallimage.cpp \
	stringart.cpp \
	spritebitmap.cpp \
	srcmode.cpp \
	strokefill.cpp \
	strokerect.cpp \
	strokerects.cpp \
	strokes.cpp \
	stroketext.cpp \
	surface.cpp \
	tablecolorfilter.cpp \
	texteffects.cpp \
	testimagefilters.cpp \
	texdata.cpp \
	variedtext.cpp \
	textblob.cpp \
	textblobshader.cpp \
	texturedomaineffect.cpp \
	thinrects.cpp \
	thinstrokedrects.cpp \
	tiledscaledbitmap.cpp \
	tileimagefilter.cpp \
	tilemodes.cpp \
	tilemodes_scaled.cpp \
	tinybitmap.cpp \
	twopointradial.cpp \
	typeface.cpp \
	vertices.cpp \
	verttext.cpp \
	verttext2.cpp \
	xfermodeimagefilter.cpp \
	xfermodes.cpp \
	xfermodes2.cpp \
	xfermodes3.cpp \
	yuvtorgbeffect.cpp \
	../src/utils/debugger/SkDrawCommand.cpp \
	../src/utils/debugger/SkDebugCanvas.cpp \
	../src/utils/debugger/SkObjectParser.cpp \
	../tools/flags/SkCommandLineFlags.cpp \
	../tools/CrashHandler.cpp \
	gm_expectations.cpp \
	../tools/ProcStats.cpp \
	../tools/Resources.cpp \
	../tools/sk_tool_utils.cpp \
	../tools/sk_tool_utils_font.cpp \
	../src/gpu/GrContextFactory.cpp \
	../src/gpu/GrTest.cpp

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libskia \
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
	$(LOCAL_PATH)/../include/effects \
	$(LOCAL_PATH)/../include/images \
	$(LOCAL_PATH)/../include/ports \
	$(LOCAL_PATH)/../src/sfnt \
	$(LOCAL_PATH)/../include/utils \
	$(LOCAL_PATH)/../src/utils \
	$(LOCAL_PATH)/../include/gpu \
	$(LOCAL_PATH)/../tools \
	$(LOCAL_PATH)/../tools/flags \
	$(LOCAL_PATH)/../src/fonts \
	$(LOCAL_PATH)/../src/core \
	$(LOCAL_PATH)/../src/gpu \
	$(LOCAL_PATH)/../src/effects \
	$(LOCAL_PATH)/../src/images \
	$(LOCAL_PATH)/../src/pipe/utils \
	$(LOCAL_PATH)/../src/utils/debugger \
	$(LOCAL_PATH)/../src/lazy \
	$(LOCAL_PATH)/../third_party/etc1 \
	$(LOCAL_PATH)/../include/pdf

LOCAL_MODULE_TAGS := \
	tests

LOCAL_MODULE := \
	skia_gm

LOCAL_PICKUP_FILES := \
	$(LOCAL_PATH)/../resources

include $(BUILD_NATIVE_TEST)
