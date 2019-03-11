#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Generate Android.bp for Skia from GN configuration.

import os
import pprint
import string
import subprocess
import tempfile

import gn_to_bp_utils

# First we start off with a template for Android.bp,
# with holes for source lists and include directories.
bp = string.Template('''// This file is autogenerated by gn_to_bp.py.

cc_library_static {
    name: "libskia",
    host_supported: true,
    cflags: [
        $cflags
    ],

    cppflags:[
        $cflags_cc
    ],

    export_include_dirs: [
        $export_includes
    ],

    local_include_dirs: [
        $local_includes
    ],

    srcs: [
        $srcs
    ],

    arch: {
        arm: {
            srcs: [
                $arm_srcs
            ],

            neon: {
                srcs: [
                    $arm_neon_srcs
                ],
            },
        },

        arm64: {
            srcs: [
                $arm64_srcs
            ],
        },

        mips: {
            srcs: [
                $none_srcs
            ],
        },

        mips64: {
            srcs: [
                $none_srcs
            ],
        },

        x86: {
            srcs: [
                $x86_srcs
            ],
        },

        x86_64: {
            srcs: [
                $x86_srcs
            ],
        },
    },

    target: {
      android: {
        srcs: [
          $android_srcs
          "third_party/vulkanmemoryallocator/GrVulkanMemoryAllocator.cpp",
        ],
        local_include_dirs: [
          "include/config/android",
          "third_party/vulkanmemoryallocator/",
        ],
        export_include_dirs: [
          "include/config/android",
        ],
      },
      linux_glibc: {
        cflags: [
          "-mssse3",
        ],
        srcs: [
          $linux_srcs
        ],
        local_include_dirs: [
          "include/config/linux",
        ],
        export_include_dirs: [
          "include/config/linux",
        ],
      },
      darwin: {
        cflags: [
          "-mssse3",
        ],
        srcs: [
          $mac_srcs
        ],
        local_include_dirs: [
          "include/config/mac",
        ],
        export_include_dirs: [
          "include/config/mac",
        ],
      },
      windows: {
        cflags: [
          "-mssse3",
          "-Wno-unknown-pragmas",
        ],
        srcs: [
          $win_srcs
        ],
        local_include_dirs: [
          "include/config/win",
        ],
        export_include_dirs: [
          "include/config/win",
        ],
      },
    },

    defaults: ["skia_deps",
               "skia_pgo",
    ],
}

// Build libskia with PGO by default.
// Location of PGO profile data is defined in build/soong/cc/pgo.go
// and is separate from skia.
// To turn it off, set ANDROID_PGO_NO_PROFILE_USE environment variable
// or set enable_profile_use property to false.
cc_defaults {
    name: "skia_pgo",
    pgo: {
        instrumentation: true,
        profile_file: "hwui/hwui.profdata",
        benchmarks: ["hwui", "skia"],
        enable_profile_use: true,
    },
}

// "defaults" property to disable profile use for Skia tools and benchmarks.
cc_defaults {
    name: "skia_pgo_no_profile_use",
    defaults: [
        "skia_pgo",
    ],
    pgo: {
        enable_profile_use: false,
    },
}

cc_defaults {
    name: "skia_deps",
    shared_libs: [
        "libandroidicu",
        "libdng_sdk",
        "libexpat",
        "libft2",
        "libjpeg",
        "liblog",
        "libpiex",
        "libpng",
        "libz",
    ],
    static_libs: [
        "libarect",
        "libsfntly",
        "libwebp-decode",
        "libwebp-encode",
    ],
    group_static_libs: true,
    target: {
      android: {
        shared_libs: [
            "libcutils",
            "libEGL",
            "libGLESv2",
            "libheif",
            "libvulkan",
            "libnativewindow",
        ],
        export_shared_lib_headers: [
            "libvulkan",
        ],
      },
      host: {
        static_libs: [
          "libcutils",
        ],
      },
      darwin: {
        host_ldlibs: [
            "-framework AppKit",
        ],
      },
      windows: {
        enabled: true,
        host_ldlibs: [
            "-lgdi32",
            "-loleaut32",
            "-lole32",
            "-lopengl32",
            "-luuid",
            "-lwindowscodecs",
        ],
      },
    },
}

cc_defaults {
    name: "skia_tool_deps",
    defaults: [
        "skia_deps",
        "skia_pgo_no_profile_use"
    ],
    static_libs: [
        "libskia",
    ],
    cflags: [
        "-Wno-implicit-fallthrough",
        "-Wno-unused-parameter",
        "-Wno-unused-variable",
    ],
}

cc_test {
    name: "skia_dm",

    defaults: [
        "skia_tool_deps"
    ],

    local_include_dirs: [
        $dm_includes
    ],

    srcs: [
        $dm_srcs
    ],

    shared_libs: [
        "libbinder",
        "libutils",
    ],
}

cc_test {
    name: "skia_nanobench",

    defaults: [
        "skia_tool_deps"
    ],

    local_include_dirs: [
        $nanobench_includes
    ],

    srcs: [
        $nanobench_srcs
    ],

    data: [
        "resources/*",
    ],
}''')

# We'll run GN to get the main source lists and include directories for Skia.
def generate_args(target_os, enable_gpu):
  d = {
    'is_official_build':                  'true',

    # gn_to_bp_utils' GetArchSources will take care of architecture-specific
    # files.
    'target_cpu':                         '"none"',

    # Use the custom FontMgr, as the framework will handle fonts.
    'skia_enable_fontmgr_custom':         'false',
    'skia_enable_fontmgr_custom_empty':   'true',
    'skia_enable_fontmgr_android':        'false',
    'skia_enable_fontmgr_win':            'false',
    'skia_enable_fontmgr_win_gdi':        'false',
    'skia_use_fonthost_mac':              'false',

    'skia_use_freetype':                  'true',
    'skia_use_fontconfig':                'false',
    'skia_use_fixed_gamma_text':          'true',
  }
  d['target_os'] = target_os
  if target_os == '"android"':
    d['skia_enable_tools'] = 'true'
    d['skia_use_libheif']  = 'true'
  else:
    d['skia_use_libheif']  = 'false'

  if enable_gpu:
    d['skia_use_vulkan']   = 'true'
  else:
    d['skia_use_vulkan']   = 'false'
    d['skia_enable_gpu']   = 'false'

  if target_os == '"win"':
    # The Android Windows build system does not provide FontSub.h
    d['skia_use_xps'] = 'false'

    # BUILDCONFIG.gn expects these to be set when building for Windows, but
    # we're just creating Android.bp, so we don't need them. Populate with
    # some dummy values.
    d['win_vc'] = '"dummy_version"'
    d['win_sdk_version'] = '"dummy_version"'
  return d

gn_args       = generate_args('"android"', True)
gn_args_linux = generate_args('"linux"',   False)
gn_args_mac   = generate_args('"mac"',     False)
gn_args_win   = generate_args('"win"',     False)

js = gn_to_bp_utils.GenerateJSONFromGN(gn_args)

def strip_slashes(lst):
  return {str(p.lstrip('/')) for p in lst}

android_srcs    = strip_slashes(js['targets']['//:skia']['sources'])
cflags          = strip_slashes(js['targets']['//:skia']['cflags'])
cflags_cc       = strip_slashes(js['targets']['//:skia']['cflags_cc'])
local_includes  = strip_slashes(js['targets']['//:skia']['include_dirs'])
export_includes = strip_slashes(js['targets']['//:public']['include_dirs'])

dm_srcs         = strip_slashes(js['targets']['//:dm']['sources'])
dm_includes     = strip_slashes(js['targets']['//:dm']['include_dirs'])

nanobench_target = js['targets']['//:nanobench']
nanobench_srcs     = strip_slashes(nanobench_target['sources'])
nanobench_includes = strip_slashes(nanobench_target['include_dirs'])

gn_to_bp_utils.GrabDependentValues(js, '//:dm', 'sources', dm_srcs, 'skia')
gn_to_bp_utils.GrabDependentValues(js, '//:nanobench', 'sources',
                                   nanobench_srcs, 'skia')

# skcms is a little special, kind of a second-party library.
local_includes.add("third_party/skcms")
dm_includes   .add("third_party/skcms")

# Android's build will choke if we list headers.
def strip_headers(sources):
  return {s for s in sources if not s.endswith('.h')}

gn_to_bp_utils.GrabDependentValues(js, '//:skia', 'sources', android_srcs, None)
android_srcs    = strip_headers(android_srcs)

js_linux        = gn_to_bp_utils.GenerateJSONFromGN(gn_args_linux)
linux_srcs      = strip_slashes(js_linux['targets']['//:skia']['sources'])
gn_to_bp_utils.GrabDependentValues(js_linux, '//:skia', 'sources', linux_srcs,
                                   None)
linux_srcs      = strip_headers(linux_srcs)

js_mac          = gn_to_bp_utils.GenerateJSONFromGN(gn_args_mac)
mac_srcs        = strip_slashes(js_mac['targets']['//:skia']['sources'])
gn_to_bp_utils.GrabDependentValues(js_mac, '//:skia', 'sources', mac_srcs,
                                   None)
mac_srcs        = strip_headers(mac_srcs)

js_win          = gn_to_bp_utils.GenerateJSONFromGN(gn_args_win)
win_srcs        = strip_slashes(js_win['targets']['//:skia']['sources'])
gn_to_bp_utils.GrabDependentValues(js_win, '//:skia', 'sources', win_srcs,
                                   None)
win_srcs        = strip_headers(win_srcs)

srcs = android_srcs.intersection(linux_srcs).intersection(mac_srcs)
srcs = srcs.intersection(win_srcs)
android_srcs    = android_srcs.difference(srcs)
linux_srcs      =   linux_srcs.difference(srcs)
mac_srcs        =     mac_srcs.difference(srcs)
win_srcs        =     win_srcs.difference(srcs)
dm_srcs         = strip_headers(dm_srcs)
nanobench_srcs  = strip_headers(nanobench_srcs)

cflags = gn_to_bp_utils.CleanupCFlags(cflags)
cflags_cc = gn_to_bp_utils.CleanupCCFlags(cflags_cc)

here = os.path.dirname(__file__)
defs = gn_to_bp_utils.GetArchSources(os.path.join(here, 'opts.gni'))

def get_defines(json):
  return {str(d) for d in json['targets']['//:skia']['defines']}
android_defines = get_defines(js)
linux_defines   = get_defines(js_linux)
mac_defines     = get_defines(js_mac)
win_defines     = get_defines(js_win)

def mkdir_if_not_exists(path):
  if not os.path.exists(path):
    os.mkdir(path)
mkdir_if_not_exists('include/config/android/')
mkdir_if_not_exists('include/config/linux/')
mkdir_if_not_exists('include/config/mac/')
mkdir_if_not_exists('include/config/win/')

platforms = { 'IOS', 'MAC', 'WIN', 'ANDROID', 'UNIX' }

def disallow_platforms(config, desired):
  with open(config, 'a') as f:
    p = sorted(platforms.difference({ desired }))
    s = '#if '
    for i in range(len(p)):
      s = s + 'defined(SK_BUILD_FOR_%s)' % p[i]
      if i < len(p) - 1:
        s += ' || '
        if i % 2 == 1:
          s += '\\\n    '
    print >>f, s
    print >>f, '    #error "Only SK_BUILD_FOR_%s should be defined!"' % desired
    print >>f, '#endif'

def append_to_file(config, s):
  with open(config, 'a') as f:
    print >>f, s

android_config = 'include/config/android/SkUserConfig.h'
gn_to_bp_utils.WriteUserConfig(android_config, android_defines)
append_to_file(android_config, '''
#ifndef SK_BUILD_FOR_ANDROID
    #error "SK_BUILD_FOR_ANDROID must be defined!"
#endif''')
disallow_platforms(android_config, 'ANDROID')

def write_config(config_path, defines, platform):
  gn_to_bp_utils.WriteUserConfig(config_path, defines)
  append_to_file(config_path, '''
// Correct SK_BUILD_FOR flags that may have been set by
// SkPreConfig.h/Android.bp
#ifndef SK_BUILD_FOR_%s
    #define SK_BUILD_FOR_%s
#endif
#ifdef SK_BUILD_FOR_ANDROID
    #undef SK_BUILD_FOR_ANDROID
#endif''' % (platform, platform))
  disallow_platforms(config_path, platform)

write_config('include/config/linux/SkUserConfig.h', linux_defines, 'UNIX')
write_config('include/config/mac/SkUserConfig.h',   mac_defines, 'MAC')
write_config('include/config/win/SkUserConfig.h',   win_defines, 'WIN')

# Turn a list of strings into the style bpfmt outputs.
def bpfmt(indent, lst, sort=True):
  if sort:
    lst = sorted(lst)
  return ('\n' + ' '*indent).join('"%s",' % v for v in lst)

# OK!  We have everything to fill in Android.bp...
with open('Android.bp', 'w') as Android_bp:
  print >>Android_bp, bp.substitute({
    'export_includes': bpfmt(8, export_includes),
    'local_includes':  bpfmt(8, local_includes),
    'srcs':            bpfmt(8, srcs),
    'cflags':          bpfmt(8, cflags, False),
    'cflags_cc':       bpfmt(8, cflags_cc),

    'arm_srcs':      bpfmt(16, strip_headers(defs['armv7'])),
    'arm_neon_srcs': bpfmt(20, strip_headers(defs['neon'])),
    'arm64_srcs':    bpfmt(16, strip_headers(defs['arm64'] +
                                             defs['crc32'])),
    'none_srcs':     bpfmt(16, strip_headers(defs['none'])),
    'x86_srcs':      bpfmt(16, strip_headers(defs['sse2'] +
                                             defs['ssse3'] +
                                             defs['sse41'] +
                                             defs['sse42'] +
                                             defs['avx'  ] +
                                             defs['hsw'  ])),

    'dm_includes'       : bpfmt(8, dm_includes),
    'dm_srcs'           : bpfmt(8, dm_srcs),

    'nanobench_includes'    : bpfmt(8, nanobench_includes),
    'nanobench_srcs'        : bpfmt(8, nanobench_srcs),

    'android_srcs':  bpfmt(10, android_srcs),
    'linux_srcs':    bpfmt(10, linux_srcs),
    'mac_srcs':      bpfmt(10, mac_srcs),
    'win_srcs':      bpfmt(10, win_srcs),
  })
