#!/usr/bin/env python
#
# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Generate Android.bp for Skia from GN configuration.

import argparse
import json
import os
import pprint
import string
import subprocess
import tempfile

parser = argparse.ArgumentParser(description='Process some cmdline flags.')
parser.add_argument('--gn', dest='gn_cmd', default='gn')
args = parser.parse_args()

def GenerateJSONFromGN(gn_args):
  gn_args = ' '.join(sorted('%s=%s' % (k,v) for (k,v) in gn_args.iteritems()))
  tmp = tempfile.mkdtemp()
  subprocess.check_call([args.gn_cmd, 'gen', tmp, '--args=%s' % gn_args,
                         '--ide=json'])
  return json.load(open(os.path.join(tmp, 'project.json')))

def _strip_slash(lst):
  return {str(p.lstrip('/')) for p in lst}

def GrabDependentValues(js, name, value_type, list_to_extend, exclude):
  # Grab the values from other targets that $name depends on (e.g. optional
  # Skia components, gms, tests, etc).
  for dep in js['targets'][name]['deps']:
    if 'modules' in dep:
      continue   # Modules require special handling -- skip for now.
    if 'third_party' in dep:
      continue   # We've handled all third-party DEPS as static or shared_libs.
    if 'none' in dep:
      continue   # We'll handle all cpu-specific sources manually later.
    if exclude and exclude in dep:
      continue
    list_to_extend.update(_strip_slash(js['targets'][dep].get(value_type, [])))
    GrabDependentValues(js, dep, value_type, list_to_extend, exclude)

def CleanupCFlags(cflags):
  # Only use the generated flags related to warnings.
  cflags = {s for s in cflags if s.startswith('-W')}
  # Add additional warning suppressions so we can build
  # third_party/vulkanmemoryallocator
  cflags = cflags.union([
    "-Wno-implicit-fallthrough",
    "-Wno-missing-field-initializers",
    "-Wno-thread-safety-analysis",
    "-Wno-unused-variable",
  ])
  # Add the rest of the flags we want.
  cflags = cflags.union([
    "-fvisibility=hidden",
    "-D_FORTIFY_SOURCE=1",
    "-DSKIA_DLL",
    "-DSKIA_IMPLEMENTATION=1",
    "-DATRACE_TAG=ATRACE_TAG_VIEW",
    "-DSK_PRINT_CODEC_MESSAGES",
  ])

  # We need to undefine FORTIFY_SOURCE before we define it. Insert it at the
  # beginning after sorting.
  cflags = sorted(cflags)
  cflags.insert(0, "-U_FORTIFY_SOURCE")
  return cflags

def CleanupCCFlags(cflags_cc):
  # Only use the generated flags related to warnings.
  cflags_cc       = {s for s in cflags_cc      if s.startswith('-W')}
  # Add the rest of the flags we want.
  cflags_cc.add("-fexceptions")
  return cflags_cc

def _get_path_info(path, kind):
  assert path == "../src"
  assert kind == "abspath"
  # While we want absolute paths in GN, relative paths work best here.
  return "src"

def GetArchSources(opts_file):
  # For architecture specific files, it's easier to just read the same source
  # that GN does (opts.gni) rather than re-run GN once for each architecture.

  # This .gni file we want to read is close enough to Python syntax
  # that we can use execfile() if we supply definitions for GN builtins.
  builtins = { 'get_path_info': _get_path_info }
  defs = {}
  execfile(opts_file, builtins, defs)

  # Perform any string substitutions.
  for arch in defs:
    defs[arch] = [ p.replace('$_src', 'src') for p in defs[arch]]

  return defs

def WriteUserConfig(userConfigPath, defines):
  # Most defines go into SkUserConfig.h
  defines.remove('NDEBUG')                 # Controlled by the Android build
  defines.remove('SKIA_IMPLEMENTATION=1')  # don't export this define.

  #... and all the #defines we want to put in SkUserConfig.h.
  with open(userConfigPath, 'w') as f:
    print >>f, '// DO NOT MODIFY! This file is autogenerated by gn_to_bp.py.'
    print >>f, '// If need to change a define, modify SkUserConfigManual.h'
    print >>f, '#pragma once'
    print >>f, '#include "SkUserConfigManual.h"'
    for define in sorted(defines):
      print >>f, '#define', define.replace('=', ' ')
