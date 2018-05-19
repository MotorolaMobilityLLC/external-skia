# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'core',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()

  bot_update = True
  if 'NoDEPS' in api.properties['buildername']:
    bot_update = False

  if bot_update:
    checkout_root = api.core.default_checkout_root
    if 'Flutter' in api.vars.builder_name:
      checkout_root = checkout_root.join('flutter')
    api.core.checkout_bot_update(checkout_root=checkout_root)
  else:
    api.core.checkout_git(checkout_root=api.path['start_dir'])
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)


TEST_BUILDERS = [
  'Build-Win-Clang-x86_64-Release-ParentRevision',
  'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
  'Housekeeper-Weekly-RecreateSKPs',
]


def GenTests(api):
  for buildername in TEST_BUILDERS:
    yield (
        api.test(buildername) +
        api.properties(buildername=buildername,
                       repository='https://skia.googlesource.com/skia.git',
                       revision='abc123',
                       path_config='kitchen',
                       swarm_out_dir='[SWARM_OUT_DIR]')
    )

  buildername = 'Build-Win-Clang-x86_64-Release-Vulkan'
  yield (
      api.test('test') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.properties(patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
    )

  buildername = 'Build-Win-Clang-x86_64-Release-ParentRevision'
  yield (
      api.test('parent_revision_trybot') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_issue=500,
                     patch_set=1,
                     patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
  )

  buildername = 'Build-Debian9-GCC-x86_64-Release-Flutter_Android'
  yield (
      api.test('flutter_trybot') +
      api.properties(
          repository='https://skia.googlesource.com/skia.git',
          buildername=buildername,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          patch_issue=500,
          patch_set=1,
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      )
  )

  builder = 'Build-Debian9-Clang-x86_64-Release-NoDEPS'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_issue=500,
                     patch_repo='https://skia.googlesource.com/skia.git',
                     patch_set=1,
                     patch_storage='gerrit') +
      api.path.exists(api.path['start_dir'].join('skp_output'))
  )

  buildername = 'Build-Debian9-GCC-x86_64-Release'
  yield (
      api.test('cross_repo_trybot') +
      api.properties(
          repository='https://skia.googlesource.com/parent_repo.git',
          buildername=buildername,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          patch_issue=500,
          patch_repo='https://skia.googlesource.com/skia.git',
          patch_set=1,
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      )
  )
