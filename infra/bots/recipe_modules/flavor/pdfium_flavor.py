# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re

import default_flavor


"""PDFium flavor utils, used for building PDFium with Skia."""


class PDFiumFlavorUtils(default_flavor.DefaultFlavorUtils):

  def compile(self, target):
    """Build PDFium with Skia."""
    pdfium_dir = self.m.vars.checkout_root.join('pdfium')

    # Runhook to generate the gn binary in buildtools.
    with self.m.context(cwd=pdfium_dir, env=self.m.vars.gclient_env):
      # TODO(borenet): Remove this hack and replace with
      # 'self.m.gclient.runhooks()' after the transition to Kitchen:
      # https://bugs.chromium.org/p/skia/issues/detail?id=7050
      depot_tools = self.m.vars.checkout_root.join('depot_tools')
      self.m.git.checkout(
          'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
          dir_path=depot_tools, ref='06493714339009216197d59c5413da2a1efdf4a2')
      self.m.run(
          self.m.step,
          'runhook',
          cmd=[depot_tools.join('gclient'), 'runhook', 'gn_linux64'])

      # Install the sysroot.
      self.m.run(
          self.m.step,
          'sysroot',
          cmd=['python', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=amd64'])

      # Setup gn args.
      gn_args = [
          'pdf_is_standalone=true',
          'clang_use_chrome_plugins=false',
          'is_component_build=false',
          'is_debug=false',
      ]
      if 'SkiaPaths' in self.m.vars.builder_name:
        gn_args.append('pdf_use_skia_paths=true')
      else:
        gn_args.append('pdf_use_skia=true')


      env = self.m.context.env
      env['CHROMIUM_BUILDTOOLS_PATH'] = str(pdfium_dir.join('buildtools'))
      with self.m.context(env=env):
        self.m.run(
            self.m.step,
            'gn_gen',
            cmd=['gn', 'gen', 'out/skia', '--args=%s' % ' '.join(gn_args)])

        # Build PDFium.
        self.m.run(
            self.m.step,
            'build_pdfium',
            cmd=['ninja', '-C', 'out/skia', '-j100'])
