#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Generate Android.bp for Skia from GN configuration.

import json
import os
import pprint
import string
import subprocess
import tempfile

tool_cflags = [
    '-fexceptions',
    '-Wno-unused-parameter',
]

# It's easier to maintain one list instead of separate lists.
tool_shared_libs = [
    'liblog',
    'libGLESv2',
    'libEGL',
    'libvulkan',
    'libz',
    'libjpeg',
    'libpng',
    'libicuuc',
    'libicui18n',
    'libexpat',
    'libft2',
    'libheif',
    'libdng_sdk',
    'libpiex',
    'libcutils',
    'libnativewindow',
]

# The ordering here is important: libsfntly needs to come after libskia.
tool_static_libs = [
    'libarect',
    'libjsoncpp',
    'libskia',
    'libsfntly',
    'libwebp-decode',
    'libwebp-encode',
]

# First we start off with a template for Android.bp,
# with holes for source lists and include directories.
bp = string.Template('''// This file is autogenerated by gn_to_bp.py.

cc_library {
    name: "libskia",
    cflags: [
        "-fexceptions",
        "-Wno-unused-parameter",
        "-U_FORTIFY_SOURCE",
        "-D_FORTIFY_SOURCE=1",
        "-DSKIA_IMPLEMENTATION=1",
        "-DATRACE_TAG=ATRACE_TAG_VIEW",
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

    shared_libs: [
        "libEGL",
        "libGLESv2",
        "libdng_sdk",
        "libexpat",
        "libft2",
        "libheif",
        "libicui18n",
        "libicuuc",
        "libjpeg",
        "liblog",
        "libpiex",
        "libpng",
        "libvulkan",
        "libz",
        "libcutils",
        "libnativewindow",
    ],
    static_libs: [
        "libarect",
        "libsfntly",
        "libwebp-decode",
        "libwebp-encode",
    ],
}

cc_test {
    name: "skia_dm",

    cflags: [
        $tool_cflags
    ],

    local_include_dirs: [
        $dm_includes
    ],

    srcs: [
        $dm_srcs
    ],

    shared_libs: [
        $tool_shared_libs
    ],

    static_libs: [
        $tool_static_libs
    ],
}

cc_test {
    name: "skia_nanobench",

    cflags: [
        $tool_cflags
    ],

    local_include_dirs: [
        $nanobench_includes
    ],

    srcs: [
        $nanobench_srcs
    ],

    shared_libs: [
        $tool_shared_libs
    ],

    static_libs: [
        $tool_static_libs
    ],
}''')

# We'll run GN to get the main source lists and include directories for Skia.
gn_args = {
  'is_official_build':  'true',
  'skia_enable_tools':  'true',
  'skia_use_libheif':   'true',
  'skia_use_vulkan':    'true',
  'target_cpu':         '"none"',
  'target_os':          '"android"',
}
gn_args = ' '.join(sorted('%s=%s' % (k,v) for (k,v) in gn_args.iteritems()))

tmp = tempfile.mkdtemp()
subprocess.check_call(['gn', 'gen', tmp, '--args=%s' % gn_args, '--ide=json'])

js = json.load(open(os.path.join(tmp, 'project.json')))

def strip_slashes(lst):
  return {str(p.lstrip('/')) for p in lst}

srcs            = strip_slashes(js['targets']['//:skia']['sources'])
local_includes  = strip_slashes(js['targets']['//:skia']['include_dirs'])
export_includes = strip_slashes(js['targets']['//:public']['include_dirs'])

dm_srcs         = strip_slashes(js['targets']['//:dm']['sources'])
dm_includes     = strip_slashes(js['targets']['//:dm']['include_dirs'])

nanobench_target = js['targets']['//:nanobench']
nanobench_srcs     = strip_slashes(nanobench_target['sources'])
nanobench_includes = strip_slashes(nanobench_target['include_dirs'])

def GrabDependentSrcs(name, srcs_to_extend, exclude):
  # Grab the sources from other targets that $name depends on (e.g. optional
  # Skia components, gms, tests, etc).
  for dep in js['targets'][name]['deps']:
    if 'third_party' in dep:
      continue   # We've handled all third-party DEPS as static or shared_libs.
    if 'none' in dep:
      continue   # We'll handle all cpu-specific sources manually later.
    if exclude and exclude in dep:
      continue
    srcs_to_extend.update(strip_slashes(js['targets'][dep].get('sources', [])))
    GrabDependentSrcs(dep, srcs_to_extend, exclude)

GrabDependentSrcs('//:skia', srcs, None)
GrabDependentSrcs('//:dm', dm_srcs, 'skia')
GrabDependentSrcs('//:nanobench', nanobench_srcs, 'skia')

# No need to list headers.
srcs            = {s for s in srcs           if not s.endswith('.h')}
dm_srcs         = {s for s in dm_srcs        if not s.endswith('.h')}
nanobench_srcs  = {s for s in nanobench_srcs if not s.endswith('.h')}

# Most defines go into SkUserConfig.h, where they're seen by Skia and its users.
defines = [str(d) for d in js['targets']['//:skia']['defines']]
defines.remove('NDEBUG')                 # Let the Android build control this.
defines.remove('SKIA_IMPLEMENTATION=1')  # Only libskia should have this define.

# For architecture specific files, it's easier to just read the same source
# that GN does (opts.gni) rather than re-run GN once for each architecture.

# This .gni file we want to read is close enough to Python syntax
# that we can use execfile() if we supply definitions for GN builtins.

def get_path_info(path, kind):
  assert kind == "abspath"
  # While we want absolute paths in GN, relative paths work best here.
  return path

builtins = { 'get_path_info': get_path_info }
defs = {}
here = os.path.dirname(__file__)
execfile(os.path.join(here,                      'opts.gni'), builtins, defs)

# Turn paths from opts.gni into paths relative to external/skia.
def scrub(lst):
  # Perform any string substitutions.
  for var in defs:
    if type(defs[var]) is str:
      lst = [ p.replace('$'+var, defs[var]) for p in lst ]
  # Relativize paths to top-level skia/ directory.
  return [os.path.relpath(p, '..') for p in lst]

# Turn a list of strings into the style bpfmt outputs.
def bpfmt(indent, lst, sort=True):
  if sort:
    lst = sorted(lst)
  return ('\n' + ' '*indent).join('"%s",' % v for v in lst)

# OK!  We have everything to fill in Android.bp...
with open('Android.bp', 'w') as f:
  print >>f, bp.substitute({
    'export_includes': bpfmt(8, export_includes),
    'local_includes':  bpfmt(8, local_includes),
    'srcs':            bpfmt(8, srcs),

    'arm_srcs':      bpfmt(16, scrub(defs['armv7'])),
    'arm_neon_srcs': bpfmt(20, scrub(defs['neon'])),
    'arm64_srcs':    bpfmt(16, scrub(defs['arm64'] +
                                     defs['crc32'])),
    'none_srcs':     bpfmt(16, scrub(defs['none'])),
    'x86_srcs':      bpfmt(16, scrub(defs['sse2'] +
                                     defs['ssse3'] +
                                     defs['sse41'] +
                                     defs['sse42'] +
                                     defs['avx'  ])),

    'tool_cflags'       : bpfmt(8, tool_cflags),
    'tool_shared_libs'  : bpfmt(8, tool_shared_libs),
    'tool_static_libs'  : bpfmt(8, tool_static_libs, False),

    'dm_includes'       : bpfmt(8, dm_includes),
    'dm_srcs'           : bpfmt(8, dm_srcs),

    'nanobench_includes'    : bpfmt(8, nanobench_includes),
    'nanobench_srcs'        : bpfmt(8, nanobench_srcs),
  })

#... and all the #defines we want to put in SkUserConfig.h.
with open('include/config/SkUserConfig.h', 'w') as f:
  print >>f, '// DO NOT MODIFY! This file is autogenerated by gn_to_bp.py.'
  print >>f, '// If need to change a define, modify SkUserConfigManual.h'
  print >>f, '#ifndef SkUserConfig_DEFINED'
  print >>f, '#define SkUserConfig_DEFINED'
  print >>f, '#include "SkUserConfigManual.h"'
  for define in sorted(defines):
    print >>f, '  #define', define.replace('=', ' ')
  print >>f, '#endif//SkUserConfig_DEFINED'
