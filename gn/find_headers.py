#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections
import json
import os
import subprocess
import sys

# Finds all public sources in include directories then write them to skia.h.

# Also write skia.h.deps, which Ninja uses to track dependencies. It's the
# very same mechanism Ninja uses to know which .h files affect which .cpp files.

gn              = sys.argv[1]
absolute_source = sys.argv[2]
skia_h          = sys.argv[3]
include_dirs    = sys.argv[4:]

absolute_source = os.path.normpath(absolute_source)

include_dirs = [os.path.join(os.path.normpath(include_dir), '')
                for include_dir in include_dirs]
include_dirs.sort(key=len, reverse=True)

gn_desc_cmd = [gn, 'desc', '.', '--root=%s' % absolute_source, '--format=json',
               '*']

desc_json_txt = ''
try:
  desc_json_txt = subprocess.check_output(gn_desc_cmd)
except subprocess.CalledProcessError as e:
  print e.output
  raise

desc_json = {}
try:
  desc_json = json.loads(desc_json_txt)
except ValueError:
  print desc_json_txt
  raise

sources = set()

for target in desc_json.itervalues():
  # We'll use `public` headers if they're listed, or pull them from `sources`
  # if not.  GN sneaks in a default "public": "*" into the JSON if you don't
  # set one explicitly.
  search_list = target.get('public')
  if search_list == '*':
    search_list = target.get('sources', [])

  for name in search_list:
    sources.add(os.path.join(absolute_source, os.path.normpath(name[2:])))

Header = collections.namedtuple('Header', ['absolute', 'include'])
headers = {}
for source in sources:
  source_as_include = [os.path.relpath(source, absolute_source)
                       for include_dir in include_dirs
                       if source.startswith(include_dir)]
  if not source_as_include:
    continue
  statinfo = os.stat(source)
  key = str(statinfo.st_ino) + ':' + str(statinfo.st_dev)
  # On Windows os.stat st_ino is 0 until 3.3.4 and st_dev is 0 until 3.4.0.
  if key == '0:0':
    key = source
  include_path = source_as_include[0]
  if key not in headers or len(include_path) < len(headers[key].include):
    headers[key] = Header(source, include_path)

headers = headers.values()
headers.sort(key=lambda x: x.include)

with open(skia_h, 'w') as f:
  f.write('// skia.h generated by GN.\n')
  f.write('#ifndef skia_h_DEFINED\n')
  f.write('#define skia_h_DEFINED\n')
  for header in headers:
    f.write('#include "' + header.include + '"\n')
  f.write('#endif//skia_h_DEFINED\n')

with open(skia_h + '.deps', 'w') as f:
  f.write(skia_h + ':')
  for header in headers:
    f.write(' ' + header.absolute)
  f.write(' build.ninja.d')
  f.write('\n')

# Temporary: during development this file wrote skia.h.d, not skia.h.deps,
# and I think we have some bad versions of those files laying around.
if os.path.exists(skia_h + '.d'):
  os.remove(skia_h + '.d')
