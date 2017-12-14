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

# First we start off with a template for Android.bp,
# with holes for source lists and include directories.
bp = string.Template('''// This file is autogenerated by gn_to_bp.py.

cc_library_static {
    name: "libskia",
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
            cflags: [
                // Clang seems to think new/malloc will only be 4-byte aligned
                // on x86 Android. We're pretty sure it's actually 8-byte
                // alignment. tests/OverAlignedTest.cpp has more information,
                // and should fail if we're wrong.
                "-Wno-over-aligned"
            ],
        },

        x86_64: {
            srcs: [
                $x86_srcs
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
        profile_file: "skia/skia.profdata",
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
    group_static_libs: true,
}

cc_defaults {
    name: "skia_tool_deps",
    defaults: [
        "skia_deps",
        "skia_pgo_no_profile_use"
    ],
    static_libs: [
        "libjsoncpp",
        "libskia",
    ],
    cflags: [
        "-Wno-unused-parameter"
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
}''')

# We'll run GN to get the main source lists and include directories for Skia.
gn_args = {
  'is_official_build':  'true',
  'skia_enable_tools':  'true',
  'skia_use_libheif':   'true',
  'skia_use_vulkan':    'true',
  'target_cpu':         '"none"',
  'target_os':          '"android"',
  'skia_vulkan_header': '"Skia_Vulkan_Android.h"',
}
gn_args = ' '.join(sorted('%s=%s' % (k,v) for (k,v) in gn_args.iteritems()))

tmp = tempfile.mkdtemp()
subprocess.check_call(['gn', 'gen', tmp, '--args=%s' % gn_args, '--ide=json'])

js = json.load(open(os.path.join(tmp, 'project.json')))

def strip_slashes(lst):
  return {str(p.lstrip('/')) for p in lst}

srcs            = strip_slashes(js['targets']['//:skia']['sources'])
cflags          = strip_slashes(js['targets']['//:skia']['cflags'])
cflags_cc       = strip_slashes(js['targets']['//:skia']['cflags_cc'])
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

# Only use the generated flags related to warnings.
cflags          = {s for s in cflags         if s.startswith('-W')}
cflags_cc       = {s for s in cflags_cc      if s.startswith('-W')}
# Add the rest of the flags we want.
cflags = cflags.union([
    "-fvisibility=hidden",
    "-D_FORTIFY_SOURCE=1",
    "-DSKIA_DLL",
    "-DSKIA_IMPLEMENTATION=1",
    "-DATRACE_TAG=ATRACE_TAG_VIEW",
])
cflags_cc.add("-fexceptions")

# We need to undefine FORTIFY_SOURCE before we define it. Insert it at the
# beginning after sorting.
cflags = sorted(cflags)
cflags.insert(0, "-U_FORTIFY_SOURCE")

# We need to add the include path to the vulkan defines and header file set in
# then skia_vulkan_header gn arg that is used for framework builds.
local_includes.add("platform_tools/android/vulkan")
export_includes.add("platform_tools/android/vulkan")

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
    'cflags':          bpfmt(8, cflags, False),
    'cflags_cc':       bpfmt(8, cflags_cc),

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
