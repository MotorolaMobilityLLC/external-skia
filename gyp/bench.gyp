# GYP file to build performance testbench.
#
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'nanobench',
      'type': 'executable',
      'sources': [
        '../bench/GMBench.cpp',
        '../bench/SKPBench.cpp',
        '../bench/ResultsWriter.cpp',
        '../bench/nanobench.cpp',
      ],
      'includes': [
        'bench.gypi',
        'gmslides.gypi',
      ],
      'dependencies': [
        'flags.gyp:flags_common',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:timer',
      ],
    },
  ],
}
