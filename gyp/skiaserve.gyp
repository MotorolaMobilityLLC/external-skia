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
      'include_dirs': [
        '../src/core',
        #TODO make this a real project
        '../third_party/externals/microhttpd/src/include',
      ],
      'sources': [ 
        '<!@(python find.py ../tools/skiaserve "*.cpp")',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'gputest.gyp:skgputest',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:proc_stats',
        'tools.gyp:resources',
      ],
      #TODO real libmicrohttpd gyp
      'link_settings': {
        'ldflags': [
          '-L../../third_party/externals/microhttpd/src/microhttpd/.libs',
        ],
        'libraries': [
          '-lmicrohttpd',
        ],
      },
    },
  ],
}
