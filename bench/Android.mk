
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  benchmain.cpp \
  SkBenchmark.cpp \
  BenchTimer.cpp \
  BenchSysTimer_posix.cpp \
  BenchGpuTimer_gl.cpp \
  SkBenchLogger.cpp \
  TimerData.cpp

LOCAL_SRC_FILES += \
  AAClipBench.cpp \
  BitmapBench.cpp \
  BitmapRectBench.cpp \
  BlurBench.cpp \
  ChecksumBench.cpp \
  ChromeBench.cpp \
  DashBench.cpp \
  DecodeBench.cpp \
  DeferredCanvasBench.cpp \
  FontScalerBench.cpp \
  GradientBench.cpp \
  GrMemoryPoolBench.cpp \
  InterpBench.cpp \
  MathBench.cpp \
  MatrixBench.cpp \
  MatrixConvolutionBench.cpp \
  MemoryBench.cpp \
  MorphologyBench.cpp \
  MutexBench.cpp \
  PathBench.cpp \
  PathIterBench.cpp \
  PicturePlaybackBench.cpp \
  PictureRecordBench.cpp \
  ReadPixBench.cpp \
  RectBench.cpp \
  RegionBench.cpp \
  RepeatTileBench.cpp \
  RTreeBench.cpp \
  ScalarBench.cpp \
  ShaderMaskBench.cpp \
  TableBench.cpp \
  TextBench.cpp \
  VertBench.cpp \
  WriterBench.cpp

# TODO: tests that currently are causing build problems
LOCAL_SRC_FILES += \
  RefCntBench.cpp

LOCAL_SHARED_LIBRARIES := libcutils libskia libGLESv2 libEGL 

LOCAL_STATIC_LIBRARIES := libstlport_static

LOCAL_C_INCLUDES := \
  external/skia/include/core \
  external/skia/include/config \
  external/skia/include/effects \
  external/skia/include/gpu \
  external/skia/include/images \
  external/skia/include/pipe \
  external/skia/include/utils \
  external/skia/src/core \
  external/skia/src/gpu \
  external/stlport/stlport \
  bionic

LOCAL_MODULE := skia_bench

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
