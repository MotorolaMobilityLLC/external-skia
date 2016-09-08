# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import default_flavor

"""GN flavor utils, used for building Skia with GN."""
class GNFlavorUtils(default_flavor.DefaultFlavorUtils):
  def supported(self):
    extra_config = self.m.vars.builder_cfg.get('extra_config', '')

    return any([
      'SAN' in extra_config,
      extra_config == 'Fast',
      extra_config == 'GN',
      extra_config.startswith('SK')
    ])

  def _run(self, title, cmd, env=None):
    self.m.vars.default_env = {k: v for (k,v)
                               in self.m.vars.default_env.iteritems()
                               if k in ['PATH']}
    self.m.run(self.m.step, title, cmd=cmd, env=env, cwd=self.m.vars.skia_dir)

  def compile(self, unused_target, **kwargs):
    """Build Skia with GN."""
    compiler      = self.m.vars.builder_cfg.get('compiler',      '')
    configuration = self.m.vars.builder_cfg.get('configuration', '')
    extra_config  = self.m.vars.builder_cfg.get('extra_config',  '')
    os            = self.m.vars.builder_cfg.get('os',            '')

    clang_linux = str(self.m.vars.slave_dir.join('clang_linux'))

    cc, cxx = None, None
    extra_cflags = []
    extra_ldflags = []

    if compiler == 'Clang' and os == 'Ubuntu':
      cc  = clang_linux + '/bin/clang'
      cxx = clang_linux + '/bin/clang++'
      extra_ldflags.append('-fuse-ld=lld')
    elif compiler == 'Clang':
      cc, cxx = 'clang', 'clang++'
    elif compiler == 'GCC':
      cc, cxx = 'gcc', 'g++'

    if extra_config == 'Fast':
      extra_cflags.extend(['-march=native', '-fomit-frame-pointer', '-O3'])
    if extra_config.startswith('SK'):
      extra_cflags.append('-D' + extra_config)
    if extra_config == 'MSAN':
      extra_ldflags.append('-L' + clang_linux + '/msan')

    args = {}

    if configuration != 'Debug':
      args['is_debug'] = 'false'
    if extra_config == 'MSAN':
      args['skia_use_fontconfig'] = 'false'

    for (k,v) in {
      'cc':  cc,
      'cxx': cxx,
      'extra_cflags':  ' '.join(extra_cflags),
      'extra_ldflags': ' '.join(extra_ldflags),
      'sanitize': extra_config if 'SAN' in extra_config else '',
    }.iteritems():
      if v:
        args[k] = '"%s"' % v

    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))

    self._run('fetch-gn', [self.m.vars.skia_dir.join('bin', 'fetch-gn')])
    self._run('gn gen', ['gn', 'gen', self.out_dir, '--args=' + gn_args])
    self._run('ninja', ['ninja', '-C', self.out_dir])

  def step(self, name, cmd, env=None, **kwargs):
    app = self.m.vars.skia_out.join(self.m.vars.configuration, cmd[0])
    cmd = [app] + cmd[1:]
    env = {}

    clang_linux = str(self.m.vars.slave_dir.join('clang_linux'))
    extra_config = self.m.vars.builder_cfg.get('extra_config', '')

    if 'SAN' in extra_config:
      # Sanitized binaries may want to run clang_linux/bin/llvm-symbolizer.
      self.m.vars.default_env['PATH'] = '%%(PATH)s:%s' % clang_linux + '/bin'
    elif 'Ubuntu' == self.m.vars.builder_cfg.get('os', ''):
      cmd = ['catchsegv'] + cmd

    if 'ASAN' == extra_config:
      env[ 'ASAN_OPTIONS'] = 'symbolize=1 detect_leaks=1'
      env[ 'LSAN_OPTIONS'] = 'symbolize=1 print_suppressions=1'
      env['UBSAN_OPTIONS'] = 'symbolize=1 print_stacktrace=1'

    if 'MSAN' == extra_config:
      # Find the MSAN-built libc++.
      env['LD_LIBRARY_PATH'] = clang_linux + '/msan'

    self._run(name, cmd, env=env)
