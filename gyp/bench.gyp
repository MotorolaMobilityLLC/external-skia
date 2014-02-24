# GYP file to build performance testbench.
#
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'bench',
      'type': 'executable',
      'include_dirs' : [
        '../src/core',
        '../src/effects',
        '../src/utils',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'bench_timer',
        'flags.gyp:flags',
        'jsoncpp.gyp:jsoncpp',
      ],
      'sources': [
        '../bench/AAClipBench.cpp',
        '../bench/BicubicBench.cpp',
        '../bench/BitmapBench.cpp',
        '../bench/BitmapRectBench.cpp',
        '../bench/BitmapScaleBench.cpp',
        '../bench/BlurBench.cpp',
        '../bench/BlurImageFilterBench.cpp',
        '../bench/BlurRectBench.cpp',
        '../bench/BlurRoundRectBench.cpp',
        '../bench/ChartBench.cpp',
        '../bench/ChecksumBench.cpp',
        '../bench/ChromeBench.cpp',
        '../bench/CmapBench.cpp',
        '../bench/ColorFilterBench.cpp',
        '../bench/ColorPrivBench.cpp',
        '../bench/CoverageBench.cpp',
        '../bench/DashBench.cpp',
        '../bench/DecodeBench.cpp',
        '../bench/DeferredCanvasBench.cpp',
        '../bench/DeferredSurfaceCopyBench.cpp',
        '../bench/DisplacementBench.cpp',
        '../bench/FSRectBench.cpp',
        '../bench/FontCacheBench.cpp',
        '../bench/FontScalerBench.cpp',
        '../bench/GameBench.cpp',
        '../bench/GrMemoryPoolBench.cpp',
        '../bench/GrResourceCacheBench.cpp',
        '../bench/GradientBench.cpp',
        '../bench/HairlinePathBench.cpp',
        '../bench/ImageCacheBench.cpp',
        '../bench/ImageDecodeBench.cpp',
        '../bench/InterpBench.cpp',
        '../bench/LightingBench.cpp',
        '../bench/LineBench.cpp',
        '../bench/MagnifierBench.cpp',
        '../bench/MathBench.cpp',
        '../bench/Matrix44Bench.cpp',
        '../bench/MatrixBench.cpp',
        '../bench/MatrixConvolutionBench.cpp',
        '../bench/MemoryBench.cpp',
        '../bench/MemsetBench.cpp',
        '../bench/MergeBench.cpp',
        '../bench/MorphologyBench.cpp',
        '../bench/MutexBench.cpp',
        '../bench/PathBench.cpp',
        '../bench/PathIterBench.cpp',
        '../bench/PathUtilsBench.cpp',
        '../bench/PerlinNoiseBench.cpp',
        '../bench/PicturePlaybackBench.cpp',
        '../bench/PictureRecordBench.cpp',
        '../bench/PremulAndUnpremulAlphaOpsBench.cpp',
        '../bench/RTreeBench.cpp',
        '../bench/ReadPixBench.cpp',
        '../bench/RectBench.cpp',
        '../bench/RectoriBench.cpp',
        '../bench/RefCntBench.cpp',
        '../bench/RegionBench.cpp',
        '../bench/RegionContainBench.cpp',
        '../bench/RepeatTileBench.cpp',
        '../bench/ScalarBench.cpp',
        '../bench/ShaderMaskBench.cpp',
        '../bench/SkipZeroesBench.cpp',
        '../bench/SortBench.cpp',
        '../bench/StrokeBench.cpp',
        '../bench/TableBench.cpp',
        '../bench/TextBench.cpp',
        '../bench/TileBench.cpp',
        '../bench/VertBench.cpp',
        '../bench/WritePixelsBench.cpp',
        '../bench/WriterBench.cpp',
        '../bench/XfermodeBench.cpp',

        '../bench/SkBenchLogger.cpp',
        '../bench/SkBenchLogger.h',
        '../bench/SkBenchmark.cpp',
        '../bench/SkBenchmark.h',
        '../bench/benchmain.cpp',
      ],
      'conditions': [
        ['skia_gpu == 1',
          {
            'include_dirs' : [
              '../src/gpu',
            ],
          },
        ],
      ],
    },
    {
      'target_name' : 'bench_timer',
      'type': 'static_library',
      'sources': [
        '../bench/BenchTimer.h',
        '../bench/BenchTimer.cpp',
        '../bench/BenchSysTimer_mach.h',
        '../bench/BenchSysTimer_mach.cpp',
        '../bench/BenchSysTimer_posix.h',
        '../bench/BenchSysTimer_posix.cpp',
        '../bench/BenchSysTimer_windows.h',
        '../bench/BenchSysTimer_windows.cpp',
      ],
        'include_dirs': [
        '../src/core',
        '../src/gpu',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'conditions': [
        [ 'skia_os not in ["mac", "ios"]', {
          'sources!': [
            '../bench/BenchSysTimer_mach.h',
            '../bench/BenchSysTimer_mach.cpp',
          ],
        }],
        [ 'skia_os not in ["linux", "freebsd", "openbsd", "solaris", "android", "chromeos"]', {
          'sources!': [
            '../bench/BenchSysTimer_posix.h',
            '../bench/BenchSysTimer_posix.cpp',
          ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
          'link_settings': {
            'libraries': [
              '-lrt',
            ],
          },
        }],
        [ 'skia_os != "win"', {
          'sources!': [
            '../bench/BenchSysTimer_windows.h',
            '../bench/BenchSysTimer_windows.cpp',
          ],
        }],
        ['skia_gpu == 1', {
          'sources': [
            '../bench/BenchGpuTimer_gl.h',
            '../bench/BenchGpuTimer_gl.cpp',
          ],
        }],
      ],
    }
  ],
}
