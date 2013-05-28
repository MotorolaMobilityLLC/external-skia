LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_SRC_FILES:= \
  AAClipTest.cpp \
  AtomicTest.cpp \
  BitmapCopyTest.cpp \
  BitmapGetColorTest.cpp \
  BitmapHeapTest.cpp \
  BitSetTest.cpp \
  BlitRowTest.cpp \
  BlurTest.cpp \
  ClampRangeTest.cpp \
  ClipCacheTest.cpp \
  ClipCubicTest.cpp \
  ClipStackTest.cpp \
  ClipperTest.cpp \
  ColorFilterTest.cpp \
  ColorTest.cpp \
  DataRefTest.cpp \
  DequeTest.cpp \
  DrawBitmapRectTest.cpp \
  DrawPathTest.cpp \
  DrawTextTest.cpp \
  EmptyPathTest.cpp \
  ErrorTest.cpp \
  FillPathTest.cpp \
  FlatDataTest.cpp \
  FlateTest.cpp \
  FontHostStreamTest.cpp \
  FontHostTest.cpp \
  GeometryTest.cpp \
  GLInterfaceValidation.cpp \
  GLProgramsTest.cpp \
  GpuBitmapCopyTest.cpp \
  GrContextFactoryTest.cpp \
  GradientTest.cpp \
  GrMemoryPoolTest.cpp \
  GrSurfaceTest.cpp \
  HashCacheTest.cpp \
  InfRectTest.cpp \
  LListTest.cpp \
  MD5Test.cpp \
  MathTest.cpp \
  Matrix44Test.cpp \
  MatrixTest.cpp \
  MemsetTest.cpp \
  MetaDataTest.cpp \
  PackBitsTest.cpp \
  PaintTest.cpp \
  ParsePathTest.cpp \
  PathCoverageTest.cpp \
  PathMeasureTest.cpp \
  PathTest.cpp \
  PipeTest.cpp \
  PointTest.cpp \
  PremulAlphaRoundTripTest.cpp \
  QuickRejectTest.cpp \
  RandomTest.cpp \
  Reader32Test.cpp \
  ReadPixelsTest.cpp \
  ReadWriteAlphaTest.cpp \
  RefCntTest.cpp \
  RefDictTest.cpp \
  RegionTest.cpp \
  RoundRectTest.cpp \
  RTreeTest.cpp \
  SHA1Test.cpp \
  ScalarTest.cpp \
  ShaderImageFilterTest.cpp \
  ShaderOpacityTest.cpp \
  Sk64Test.cpp \
  skia_test.cpp \
  SortTest.cpp \
  SrcOverTest.cpp \
  StreamTest.cpp \
  StringTest.cpp \
  StrokeTest.cpp \
  Test.cpp \
  TestSize.cpp \
  TileGridTest.cpp \
  TLSTest.cpp \
  TSetTest.cpp \
  UnicodeTest.cpp \
  UtilsTest.cpp \
  WArrayTest.cpp \
  WritePixelsTest.cpp \
  Writer32Test.cpp \
  XfermodeTest.cpp

LOCAL_SRC_FILES += \
  ../src/utils/SkCondVar.cpp \
	../src/utils/SkCountdown.cpp \
	../src/utils/SkThreadPool.cpp \
  ../tools/flags/SkCommandLineFlags.cpp

# Needed for PipeTest
LOCAL_SRC_FILES += \
  ../src/pipe/utils/SamplePipeControllers.cpp

# TODO: tests that currently are causing build problems
#LOCAL_SRC_FILES += \
#  AnnotationTest.cpp \
#  ARGBImageEncoderTest.cpp \
#  BitmapFactoryTest.cpp \
#  BitmapHasherTest.cpp \
#  CanvasTest.cpp \
#  ChecksumTest.cpp \
#  DeferredCanvasTest.cpp \
#  FontMgrTest.cpp \
#  FontNamesTest.cpp \
#  PDFPrimitivesTest.cpp \
#  PictureTest.cpp \
#  PictureUtilsTest.cpp \
#  SurfaceTest.cpp \
#  ToUnicode.cpp

LOCAL_MODULE:= skia_test

LOCAL_C_INCLUDES := \
   external/freetype/include \
   external/skia/src/core \
   external/skia/src/effects \
   external/skia/src/gpu \
   external/skia/src/lazy \
   external/skia/src/pdf \
   external/skia/src/pipe/utils \
   external/skia/src/utils \
   external/skia/tools/flags

LOCAL_SHARED_LIBRARIES := libcutils libskia libGLESv2 libEGL

LOCAL_MODULE_TAGS := eng tests

include $(BUILD_EXECUTABLE)
