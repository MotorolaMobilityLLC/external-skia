#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test results.py

TODO(epoger): Create a command to update the expected results (in
self._output_dir_expected) when appropriate.  For now, you should:
1. examine the results in self._output_dir_actual and make sure they are ok
2. rm -rf self._output_dir_expected
3. mv self._output_dir_actual self._output_dir_expected
Although, if you're using an SVN checkout, this will blow away .svn directories
within self._output_dir_expected, which wouldn't be good...

"""

import os
import sys

# Imports from within Skia
import base_unittest
import results
import gm_json  # must import results first, so that gm_json will be in sys.path


class ResultsTest(base_unittest.TestCase):

  def test_gm(self):
    """Process results of a GM run with the Results object."""
    results_obj = results.Results(
        actuals_root=os.path.join(self._input_dir, 'gm-actuals'),
        expected_root=os.path.join(self._input_dir, 'gm-expectations'),
        generated_images_root=self._temp_dir)
    gm_json.WriteToFile(results_obj.get_results_of_type(results.RESULTS_ALL),
                        os.path.join(self._output_dir_actual, 'gm.json'))


def main():
  base_unittest.main(ResultsTest)


if __name__ == '__main__':
  main()
