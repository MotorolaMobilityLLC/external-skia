# Common gypi for unit tests.
{
  'include_dirs': [
    '../src/core',
    '../src/effects',
    '../src/image',
    '../src/lazy',
    '../src/images',
    '../src/pathops',
    '../src/pdf',
    '../src/pipe/utils',
    '../src/utils',
    '../src/utils/debugger',

    # Needed for TDStackNesterTest.
    '../experimental/PdfViewer',
    '../experimental/PdfViewer/src',
  ],
  'dependencies': [
    'experimental.gyp:experimental',
    'flags.gyp:flags_common',
    'pdf.gyp:pdf',
    'skia_lib.gyp:skia_lib',
    'tools.gyp:picture_utils',
    'tools.gyp:resources',
    'tools.gyp:sk_tool_utils',
  ],
  'conditions': [
    [ 'skia_os == "android"', {
      'include_dirs': [
        '../src/ports',
      ],
      'sources': [
        '../tests/FontConfigParser.cpp',
      ],
    }],
    [ 'skia_android_framework == 1', {
      'libraries': [
        '-ldl',
      ],
    }],
  ],
  'sources': [
    '../tests/Test.cpp',
    '../tests/Test.h',

    '../tests/AAClipTest.cpp',
    '../tests/ARGBImageEncoderTest.cpp',
    '../tests/AnnotationTest.cpp',
    '../tests/AsADashTest.cpp',
    '../tests/AtomicTest.cpp',
    '../tests/BBoxHierarchyTest.cpp',
    '../tests/BitSetTest.cpp',
    '../tests/BitmapCopyTest.cpp',
    '../tests/BitmapGetColorTest.cpp',
    '../tests/BitmapHasherTest.cpp',
    '../tests/BitmapHeapTest.cpp',
    '../tests/BitmapTest.cpp',
    '../tests/BlendTest.cpp',
    '../tests/BlitRowTest.cpp',
    '../tests/BlurTest.cpp',
    '../tests/CachedDecodingPixelRefTest.cpp',
    '../tests/CanvasStateHelpers.cpp',
    '../tests/CanvasStateTest.cpp',
    '../tests/CanvasTest.cpp',
    '../tests/ChecksumTest.cpp',
    '../tests/ClampRangeTest.cpp',
    '../tests/ClipCacheTest.cpp',
    '../tests/ClipCubicTest.cpp',
    '../tests/ClipStackTest.cpp',
    '../tests/ClipperTest.cpp',
    '../tests/ColorFilterTest.cpp',
    '../tests/ColorPrivTest.cpp',
    '../tests/ColorTest.cpp',
    '../tests/DashPathEffectTest.cpp',
    '../tests/DataRefTest.cpp',
    '../tests/DeferredCanvasTest.cpp',
    '../tests/DequeTest.cpp',
    '../tests/DeviceLooperTest.cpp',
    '../tests/DiscardableMemoryPoolTest.cpp',
    '../tests/DiscardableMemoryTest.cpp',
    '../tests/DocumentTest.cpp',
    '../tests/DrawBitmapRectTest.cpp',
    '../tests/DrawPathTest.cpp',
    '../tests/DrawTextTest.cpp',
    '../tests/DynamicHashTest.cpp',
    '../tests/EmptyPathTest.cpp',
    '../tests/ErrorTest.cpp',
    '../tests/FillPathTest.cpp',
    '../tests/FitsInTest.cpp',
    '../tests/FlatDataTest.cpp',
    '../tests/FlateTest.cpp',
    '../tests/FloatingPointTextureTest.cpp',
    '../tests/FontHostStreamTest.cpp',
    '../tests/FontHostTest.cpp',
    '../tests/FontMgrTest.cpp',
    '../tests/FontNamesTest.cpp',
    '../tests/FontObjTest.cpp',
    '../tests/FrontBufferedStreamTest.cpp',
    '../tests/GLInterfaceValidationTest.cpp',
    '../tests/GLProgramsTest.cpp',
    '../tests/GeometryTest.cpp',
    '../tests/GifTest.cpp',
    '../tests/GpuColorFilterTest.cpp',
    '../tests/GpuDrawPathTest.cpp',
    '../tests/GpuLayerCacheTest.cpp',
    '../tests/GpuRectanizerTest.cpp',
    '../tests/GrBinHashKeyTest.cpp',
    '../tests/GrContextFactoryTest.cpp',
    '../tests/GrDrawTargetTest.cpp',
    '../tests/GrAllocatorTest.cpp',
    '../tests/GrMemoryPoolTest.cpp',
    '../tests/GrOrderedSetTest.cpp',
    '../tests/GrGLSLPrettyPrintTest.cpp',
    '../tests/GrRedBlackTreeTest.cpp',
    '../tests/GrSurfaceTest.cpp',
    '../tests/GrTBSearchTest.cpp',
    '../tests/GradientTest.cpp',
    '../tests/ImageCacheTest.cpp',
    '../tests/ImageDecodingTest.cpp',
    '../tests/ImageFilterTest.cpp',
    '../tests/ImageGeneratorTest.cpp',
    '../tests/ImageIsOpaqueTest.cpp',
    '../tests/ImageNewShaderTest.cpp',
    '../tests/InfRectTest.cpp',
    '../tests/InterpolatorTest.cpp',
    '../tests/JpegTest.cpp',
    '../tests/KtxTest.cpp',
    '../tests/LListTest.cpp',
    '../tests/LayerDrawLooperTest.cpp',
    '../tests/LayerRasterizerTest.cpp',
    '../tests/MD5Test.cpp',
    '../tests/MallocPixelRefTest.cpp',
    '../tests/MathTest.cpp',
    '../tests/Matrix44Test.cpp',
    '../tests/MatrixClipCollapseTest.cpp',
    '../tests/MatrixTest.cpp',
    '../tests/MemoryTest.cpp',
    '../tests/MemsetTest.cpp',
    '../tests/MessageBusTest.cpp',
    '../tests/MetaDataTest.cpp',
    '../tests/MipMapTest.cpp',
    '../tests/NameAllocatorTest.cpp',
    '../tests/OSPathTest.cpp',
    '../tests/ObjectPoolTest.cpp',
    '../tests/OnceTest.cpp',
    '../tests/PDFJpegEmbedTest.cpp',
    '../tests/PDFPrimitivesTest.cpp',
    '../tests/PackBitsTest.cpp',
    '../tests/PaintTest.cpp',
    '../tests/ParsePathTest.cpp',
    '../tests/PathCoverageTest.cpp',
    '../tests/PathMeasureTest.cpp',
    '../tests/PathTest.cpp',
    '../tests/PathUtilsTest.cpp',
    '../tests/PictureShaderTest.cpp',
    '../tests/PictureTest.cpp',
    '../tests/PixelRefTest.cpp',
    '../tests/PointTest.cpp',
    '../tests/PremulAlphaRoundTripTest.cpp',
    '../tests/QuickRejectTest.cpp',
    '../tests/RTConfRegistryTest.cpp',
    '../tests/RTreeTest.cpp',
    '../tests/RandomTest.cpp',
    '../tests/ReadPixelsTest.cpp',
    '../tests/ReadWriteAlphaTest.cpp',
    '../tests/Reader32Test.cpp',
    '../tests/RecordDrawTest.cpp',
    '../tests/RecordReplaceDrawTest.cpp',
    '../tests/RecordOptsTest.cpp',
    '../tests/RecordPatternTest.cpp',
    '../tests/RecordTest.cpp',
    '../tests/RecorderTest.cpp',
    '../tests/RecordingTest.cpp',
    '../tests/RecordingXfermodeTest.cpp',
    '../tests/RefCntTest.cpp',
    '../tests/RefDictTest.cpp',
    '../tests/RegionTest.cpp',
    '../tests/ResourceCacheTest.cpp',
    '../tests/RoundRectTest.cpp',
    '../tests/RuntimeConfigTest.cpp',
    '../tests/SHA1Test.cpp',
    '../tests/SListTest.cpp',
    '../tests/ScalarTest.cpp',
    '../tests/SerializationTest.cpp',
    '../tests/ShaderImageFilterTest.cpp',
    '../tests/ShaderOpacityTest.cpp',
    '../tests/SizeTest.cpp',
    '../tests/SkBase64Test.cpp',
    '../tests/SkResourceCacheTest.cpp',
    '../tests/SmallAllocatorTest.cpp',
    '../tests/SortTest.cpp',
    '../tests/SrcOverTest.cpp',
    '../tests/StreamTest.cpp',
    '../tests/StringTest.cpp',
    '../tests/StrokeTest.cpp',
    '../tests/SurfaceTest.cpp',
    '../tests/TArrayTest.cpp',
    '../tests/TLSTest.cpp',
    '../tests/TSetTest.cpp',
    '../tests/TextBlobTest.cpp',
    '../tests/TextureCompressionTest.cpp',
    '../tests/TileGridTest.cpp',
    '../tests/ToUnicodeTest.cpp',
    '../tests/TracingTest.cpp',
    '../tests/TypefaceTest.cpp',
    '../tests/UnicodeTest.cpp',
    '../tests/UtilsTest.cpp',
    '../tests/WArrayTest.cpp',
    '../tests/WritePixelsTest.cpp',
    '../tests/Writer32Test.cpp',
    '../tests/XfermodeTest.cpp',

    '../tests/MatrixClipCollapseTest.cpp',
    '../src/utils/debugger/SkDrawCommand.h',
    '../src/utils/debugger/SkDrawCommand.cpp',
    '../src/utils/debugger/SkDebugCanvas.h',
    '../src/utils/debugger/SkDebugCanvas.cpp',
    '../src/utils/debugger/SkObjectParser.h',
    '../src/utils/debugger/SkObjectParser.cpp',

    '../tests/PipeTest.cpp',
    '../src/pipe/utils/SamplePipeControllers.cpp',

    '../tests/TDStackNesterTest.cpp',
    '../experimental/PdfViewer/src/SkTDStackNester.h',
  ],
}
