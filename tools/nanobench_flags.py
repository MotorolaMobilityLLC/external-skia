#!/usr/bin/env python

usage = '''
Write extra flags to outfile for nanobench based on the bot name:
  $ python nanobench_flags.py outfile Perf-Android-GalaxyS3-Mali400-Arm7-Release
Or run self-tests:
  $ python nanobench_flags.py test
'''

import inspect
import json
import os
import sys


def lineno():
  caller = inspect.stack()[1]  # Up one level to our caller.
  return inspect.getframeinfo(caller[0]).lineno


cov_start = lineno()+1   # We care about coverage starting just past this def.
def get_args(bot):
  args = []

  args.extend(['--scales', '1.0', '1.1'])

  if 'Valgrind' in bot:
    # Don't care about Valgrind performance.
    args.extend(['--loops',   '1'])
    args.extend(['--samples', '1'])

  match = []
  if 'Android' in bot:
    # Segfaults when run as GPU bench. Very large texture?
    match.append('~blurroundrect')
    match.append('~patch_grid')  # skia:2847
    match.append('~desk_carsvg')
  if 'HD2000' in bot:
    match.extend(['~gradient', '~etc1bitmap'])  # skia:2895
  if 'Nexus7' in bot:
    match = ['skp']  # skia:2774
  if match:
    args.append('--match')
    args.extend(match)

  if ('GalaxyS3' in bot or
      'GalaxyS4' in bot):
    args.append('--nocpu')
  return args
cov_end = lineno()   # Don't care about code coverage past here.


def self_test():
  import coverage  # This way the bots don't need coverage.py to be installed.
  args = {}
  cases = [
    'Perf-Android-GalaxyS3-Mali400-Arm7-Release',
    'Perf-Android-Nexus7-Tegra3-Arm7-Release',
    'Test-Ubuntu12-ShuttleA-GTX550Ti-x86_64-Release-Valgrind',
    'Test-Win7-ShuttleA-HD2000-x86-Debug-ANGLE',
  ]

  cov = coverage.coverage()
  cov.start()
  for case in cases:
    args[case] = get_args(case)
  cov.stop()

  this_file = os.path.basename(__file__)
  _, _, not_run, _ = cov.analysis(this_file)
  filtered = [line for line in not_run if line > cov_start and line < cov_end]
  if filtered:
    print 'Lines not covered by test cases: ', filtered
    sys.exit(1)

  golden = this_file.replace('.py', '.json')
  with open(os.path.join(os.path.dirname(__file__), golden), 'w') as f:
    json.dump(args, f, indent=2, sort_keys=True)


if __name__ == '__main__':
  if len(sys.argv) == 2 and sys.argv[1] == 'test':
    self_test()
    sys.exit(0)

  if len(sys.argv) != 3:
    print usage
    sys.exit(1)

  with open(sys.argv[1], 'w') as out:
    json.dump(get_args(sys.argv[2]), out)
