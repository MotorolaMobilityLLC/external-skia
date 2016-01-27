# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP file to build performance testbench.
#
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'skiaserve',
      'type': 'executable',
      'sources': [ 
        '<!@(python find.py ../tools/skiaserve "*.cpp")',
      ],
      'include_dirs': [
        '../tools/json',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'gputest.gyp:skgputest',
        'json.gyp:json',
        'jsoncpp.gyp:jsoncpp',
        'microhttpd.gyp:microhttpd',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:proc_stats',
        'tools.gyp:resources',
      ],
    },
  ],
}
