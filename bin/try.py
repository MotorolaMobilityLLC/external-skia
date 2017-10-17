#!/usr/bin/env python

# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Submit one or more try jobs."""


import argparse
import json
import os
import re
import subprocess
import sys


BUCKET_SKIA_PRIMARY = 'skia.primary'
CHECKOUT_ROOT = os.path.realpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir))
INFRA_BOTS = os.path.join(CHECKOUT_ROOT, 'infra', 'bots')
JOBS_JSON = os.path.join(INFRA_BOTS, 'jobs.json')

sys.path.insert(0, INFRA_BOTS)

import update_meta_config


def main():
  # Parse arguments.
  d = 'Helper script for triggering try jobs defined in %s.' % JOBS_JSON
  parser = argparse.ArgumentParser(description=d)
  parser.add_argument('--list', action='store_true', default=False,
                      help='Just list the jobs; do not trigger anything.')
  parser.add_argument('job', nargs='?', default=None,
                      help='Job name or regular expression to match job names.')
  args = parser.parse_args()

  # Load and filter the list of Skia jobs.
  jobs = []
  with open(JOBS_JSON) as f:
    jobs.append((BUCKET_SKIA_PRIMARY, json.load(f)))
  jobs.extend(update_meta_config.CQ_INCLUDE_CHROMIUM_TRYBOTS)
  if args.job:
    new_jobs = []
    for bucket, job_list in jobs:
      filtered = [j for j in job_list if re.search(args.job, j)]
      if len(filtered) > 0:
        new_jobs.append((bucket, filtered))
    jobs = new_jobs

  # Display the list of jobs.
  if len(jobs) == 0:
    print 'Found no jobs matching "%s"' % repr(args.job)
    sys.exit(1)
  count = 0
  for bucket, job_list in jobs:
    count += len(job_list)
  print 'Found %d jobs:' % count
  for bucket, job_list in jobs:
    print '  %s:' % bucket
    for j in job_list:
      print '    %s' % j
  if args.list:
    return

  # Prompt before triggering jobs.
  resp = raw_input('\nDo you want to trigger these jobs? (y/n) ')
  if resp != 'y':
    sys.exit(1)

  # Trigger the try jobs.
  for bucket, job_list in jobs:
    cmd = ['git', 'cl', 'try', '-B', bucket]
    for j in job_list:
      cmd.extend(['-b', j])
    try:
      subprocess.check_call(cmd)
    except subprocess.CalledProcessError:
      # Output from the command will fall through, so just exit here rather than
      # printing a stack trace.
      sys.exit(1)


if __name__ == '__main__':
  main()
