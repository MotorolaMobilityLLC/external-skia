#! /bin/sh
# Copyright 2019 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
exec python "$(dirname "$0")"/make_universal_apk.py "$@"
