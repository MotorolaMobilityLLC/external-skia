# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Disable warning about setting self.device_dirs in install(); we need to.
# pylint: disable=W0201


from . import default


"""iOS flavor, used for running code on iOS."""


class iOSFlavor(default.DefaultFlavor):
  def __init__(self, m):
    super(iOSFlavor, self).__init__(m)
    self.device_dirs = default.DeviceDirs(
        bin_dir='[unused]',
        dm_dir='dm',
        perf_data_dir='perf',
        resource_dir='resources',
        images_dir='images',
        lotties_dir='lotties',
        skp_dir='skps',
        svg_dir='svgs',
        mskp_dir='mskp',
        tmp_dir='tmp',
        texttraces_dir='')

  def _run(self, title, *cmd, **kwargs):
    def sleep(attempt):
      self.m.python.inline('sleep before attempt %d' % attempt, """
import time
time.sleep(2)
""")  # pragma: nocover
    return self.m.run.with_retry(self.m.step, title, 3, cmd=list(cmd),
                                 between_attempts_fn=sleep, **kwargs)

  def install(self):
    # We assume a single device is connected.

    # Pair the device.
    try:
      self.m.run(self.m.step, 'check if device is paired',
                 cmd=['idevicepair', 'validate'],
                 infra_step=True, abort_on_failure=True,
                 fail_build_on_failure=False)
    except self.m.step.StepFailure:  # pragma: nocover
      self._run('pair device', 'idevicepair', 'pair')  # pragma: nocover

    # Mount developer image.
    image_info = self._run('list mounted image',
                           'ideviceimagemounter', '--list')
    image_info_out = image_info.stdout.strip() if image_info.stdout else ''
    if ('ImagePresent: true' not in image_info_out and
        'ImageSignature:' not in image_info_out):
      image_pkgs = self.m.file.glob_paths('locate ios-dev-image package',
                                          self.m.path['start_dir'],
                                          'ios-dev-image*',
                                          test_data=['ios-dev-image-13.2'])
      if len(image_pkgs) != 1:
        raise Exception('glob for ios-dev-image* returned %s'
                        % image_pkgs)  # pragma: nocover

      image_pkg = image_pkgs[0]
      contents = self.m.file.listdir(
          'locate image and signature', image_pkg,
          test_data=['DeveloperDiskImage.dmg',
                     'DeveloperDiskImage.dmg.signature'])
      image = None
      sig = None
      for f in contents:
        if str(f).endswith('.dmg'):
          image = f
        if str(f).endswith('.dmg.signature'):
          sig = f
      if not image or not sig:
        raise Exception('%s does not contain *.dmg and *.dmg.signature' %
                        image_pkg)  # pragma: nocover

      self._run('mount developer image', 'ideviceimagemounter', image, sig)

    # Install the apps (necessary before copying any resources to the device).
    for app_name in ['dm', 'nanobench']:
      app_package = self.host_dirs.bin_dir.join('%s.app' % app_name)

      def uninstall_app(attempt):
        # If app ID changes, upgrade will fail, so try uninstalling.
        self.m.run(self.m.step,
                   'uninstall %s' % app_name,
                   cmd=['ideviceinstaller', '-U', 'com.google.%s' % app_name],
                   infra_step=True,
                   # App may not be installed.
                   abort_on_failure=False, fail_build_on_failure=False)

      num_attempts = 2
      self.m.run.with_retry(self.m.step, 'install %s' % app_name, num_attempts,
                            cmd=['ideviceinstaller', '-i', app_package],
                            between_attempts_fn=uninstall_app,
                            infra_step=True)

  def step(self, name, cmd, **kwargs):
    app_name = cmd[0]
    bundle_id = 'com.google.%s' % app_name
    args = [bundle_id] + map(str, cmd[1:])
    success = False
    try:
      self.m.run(self.m.step, name, cmd=['idevicedebug', 'run'] + args)
      success = True
    finally:
      if not success:
        self.m.run(self.m.python, '%s with full debug output' % name,
                   script=self.module.resource('ios_debug_cmd.py'),
                   args=args)

  def _run_ios_script(self, script, first, *rest):
    full = self.m.path['start_dir'].join(
        'skia', 'platform_tools', 'ios', 'bin', 'ios_' + script)
    self.m.run(self.m.step,
               name = '%s %s' % (script, first),
               cmd = [full, first] + list(rest),
               infra_step=True)

  def copy_file_to_device(self, host, device):
    self._run_ios_script('push_file', host, device)

  def copy_directory_contents_to_device(self, host, device):
    self._run_ios_script('push_if_needed', host, device)

  def copy_directory_contents_to_host(self, device, host):
    self._run_ios_script('pull_if_needed', device, host)

  def remove_file_on_device(self, path):
    self._run_ios_script('rm', path)

  def create_clean_device_dir(self, path):
    self._run_ios_script('rm',    path)
    self._run_ios_script('mkdir', path)

  def read_file_on_device(self, path, **kwargs):
    full = self.m.path['start_dir'].join(
        'skia', 'platform_tools', 'ios', 'bin', 'ios_cat_file')
    rv = self.m.run(self.m.step,
                    name = 'cat_file %s' % path,
                    cmd = [full, path],
                    stdout=self.m.raw_io.output(),
                    infra_step=True,
                    **kwargs)
    return rv.stdout.rstrip() if rv and rv.stdout else None
