#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

# We'll recursively search each include directory for headers,
# then write them to skia.h with a small blacklist.

# We'll also write skia.h.deps, which Ninja uses to track dependencies. It's the
# very same mechanism Ninja uses to know which .h files affect which .cpp files.

skia_h       = sys.argv[1]
include_dirs = sys.argv[2:]

blacklist = {
  "GrGLConfig_chrome.h",
  "SkFontMgr_fontconfig.h",
}

headers = []
for directory in include_dirs:
  for d, _, files in os.walk(directory):
    for f in files:
      if f.endswith('.h') and f not in blacklist:
        headers.append(os.path.join(d,f))
headers.sort()

with open(skia_h, "w") as f:
  f.write('// skia.h generated by GN.\n')
  f.write('#ifndef skia_h_DEFINED\n')
  f.write('#define skia_h_DEFINED\n')
  for h in headers:
    f.write('#include "' + h + '"\n')
  f.write('#endif//skia_h_DEFINED\n')

with open(skia_h + '.deps', "w") as f:
  f.write(skia_h + ':')
  for h in headers:
    f.write(' ' + h)
  f.write('\n')

# Temporary: during development this file wrote skia.h.d, not skia.h.deps,
# and I think we have some bad versions of those files laying around.
if os.path.exists(skia_h + '.d'):
  os.remove(skia_h + '.d')
