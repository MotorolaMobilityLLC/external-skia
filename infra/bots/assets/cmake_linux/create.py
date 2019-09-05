#!/usr/bin/env python
#
# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import common
import subprocess
import utils

VERSION = '3.11.0'
URL = 'https://cmake.org/files/v%s/cmake-%s-Linux-x86_64.tar.gz' % (
    VERSION.rsplit('.', 1)[0], VERSION)


def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    subprocess.check_call(['curl', URL, '-o', 'cmake.tar.gz'])
    subprocess.check_call(['tar', '--extract', '--gunzip', '--file',
                           'cmake.tar.gz', '--directory', target_dir,
                           '--strip-components', '1'])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
