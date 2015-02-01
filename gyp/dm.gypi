# GYP for "dm" (Diamond Master, a.k.a Dungeon master, a.k.a GM 2).
{
  'include_dirs': [
    '../dm',
    '../experimental/svg',
    '../gm',
    '../src/core',
    '../src/effects',
    '../src/images',
    '../src/lazy',
    '../src/pipe/utils/',
    '../src/utils',
    '../src/utils/debugger',
    '../tests',
    '../tools',
  ],
  'dependencies': [
    'etc1.gyp:libetc1',
    'flags.gyp:flags',
    'jsoncpp.gyp:jsoncpp',
    'skia_lib.gyp:skia_lib',
    'tools.gyp:crash_handler',
    'tools.gyp:proc_stats',
    'tools.gyp:sk_tool_utils',
    'tools.gyp:timer',
    'xml.gyp:xml',
  ],
  'includes': [
    'gmslides.gypi',
    'pathops_unittest.gypi',
    'tests.gypi',
  ],
  'sources': [
    '../dm/DM.cpp',
    '../dm/DMSrcSink.cpp',
    '../dm/DMJsonWriter.cpp',
    '../gm/gm.cpp',

    '../experimental/svg/SkSVGDevice.cpp',
    '../src/pipe/utils/SamplePipeControllers.cpp',
    '../src/utils/debugger/SkDebugCanvas.cpp',
    '../src/utils/debugger/SkDrawCommand.cpp',
    '../src/utils/debugger/SkObjectParser.cpp',
  ],
  'conditions': [
    [ 'skia_gpu == 1', {
      'dependencies': [ 'gputest.gyp:skgputest' ],
    }],
  ],
}
