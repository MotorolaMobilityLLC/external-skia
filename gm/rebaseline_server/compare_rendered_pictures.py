#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Compare results of two render_pictures runs.

TODO(epoger): Start using this module to compare ALL images (whether they
were generated from GMs or SKPs), and rename it accordingly.
"""

# System-level imports
import logging
import os
import shutil
import tempfile
import time

# Must fix up PYTHONPATH before importing from within Skia
import fix_pythonpath  # pylint: disable=W0611

# Imports from within Skia
from py.utils import gs_utils
from py.utils import url_utils
import buildbot_globals
import column
import gm_json
import imagediffdb
import imagepair
import imagepairset
import results

# URL under which all render_pictures images can be found in Google Storage.
#
# TODO(epoger): In order to allow live-view of GMs and other images, read this
# from the input summary files, or allow the caller to set it within the
# GET_live_results call.
DEFAULT_IMAGE_BASE_GS_URL = 'gs://' + buildbot_globals.Get('skp_images_bucket')

# Column descriptors, and display preferences for them.
COLUMN__RESULT_TYPE = results.KEY__EXTRACOLUMNS__RESULT_TYPE
COLUMN__SOURCE_SKP = 'sourceSkpFile'
COLUMN__TILED_OR_WHOLE = 'tiledOrWhole'
COLUMN__TILENUM = 'tilenum'
FREEFORM_COLUMN_IDS = [
    COLUMN__TILENUM,
]
ORDERED_COLUMN_IDS = [
    COLUMN__RESULT_TYPE,
    COLUMN__SOURCE_SKP,
    COLUMN__TILED_OR_WHOLE,
    COLUMN__TILENUM,
]

# A special "repo:" URL type that we use to refer to Skia repo contents.
# (Useful for comparing against expectations files we store in our repo.)
REPO_URL_PREFIX = 'repo:'
REPO_BASEPATH = os.path.abspath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir))


class RenderedPicturesComparisons(results.BaseComparisons):
  """Loads results from multiple render_pictures runs into an ImagePairSet.
  """

  def __init__(self, setA_dirs, setB_dirs, image_diff_db,
               image_base_gs_url=DEFAULT_IMAGE_BASE_GS_URL,
               diff_base_url=None, setA_label='setA',
               setB_label='setB', gs=None,
               truncate_results=False, prefetch_only=False,
               download_all_images=False):
    """Constructor: downloads images and generates diffs.

    Once the object has been created (which may take a while), you can call its
    get_packaged_results_of_type() method to quickly retrieve the results...
    unless you have set prefetch_only to True, in which case we will
    asynchronously warm up the ImageDiffDB cache but not fill in self._results.

    Args:
      setA_dirs: list of root directories to copy all JSON summaries from,
          and to use as setA within the comparisons. These directories may be
          gs:// URLs, special "repo:" URLs, or local filepaths.
      setB_dirs: list of root directories to copy all JSON summaries from,
          and to use as setB within the comparisons. These directories may be
          gs:// URLs, special "repo:" URLs, or local filepaths.
      image_diff_db: ImageDiffDB instance
      image_base_gs_url: "gs://" URL pointing at the Google Storage bucket/dir
          under which all render_pictures result images can
          be found; this will be used to read images for comparison within
          this code, and included in the ImagePairSet (as an HTTP URL) so its
          consumers know where to download the images from
      diff_base_url: base URL within which the client should look for diff
          images; if not specified, defaults to a "file:///" URL representation
          of image_diff_db's storage_root
      setA_label: description to use for results in setA
      setB_label: description to use for results in setB
      gs: instance of GSUtils object we can use to download summary files
      truncate_results: FOR MANUAL TESTING: if True, truncate the set of images
          we process, to speed up testing.
      prefetch_only: if True, return the new object as quickly as possible
          with empty self._results (just queue up all the files to process,
          don't wait around for them to be processed and recorded); otherwise,
          block until the results have been assembled and recorded in
          self._results.
      download_all_images: if True, download all images, even if we don't
          need them to generate diffs.  This will take much longer to complete,
          but is useful for warming up the bitmap cache on local disk.
    """
    super(RenderedPicturesComparisons, self).__init__()
    self._image_diff_db = image_diff_db
    self._image_base_gs_url = image_base_gs_url
    self._diff_base_url = (
        diff_base_url or
        url_utils.create_filepath_url(image_diff_db.storage_root))
    self._setA_label = setA_label
    self._setB_label = setB_label
    self._gs = gs
    self.truncate_results = truncate_results
    self._prefetch_only = prefetch_only
    self._download_all_images = download_all_images

    tempdir = tempfile.mkdtemp()
    try:
      setA_root = os.path.join(tempdir, 'setA')
      setB_root = os.path.join(tempdir, 'setB')
      for source_dir in setA_dirs:
        self._copy_dir_contents(source_dir=source_dir, dest_dir=setA_root)
      for source_dir in setB_dirs:
        self._copy_dir_contents(source_dir=source_dir, dest_dir=setB_root)

      time_start = int(time.time())
      # TODO(epoger): For now, this assumes that we are always comparing two
      # sets of actual results, not actuals vs expectations.  Allow the user
      # to control this.
      self._results = self._load_result_pairs(
          setA_root=setA_root, setA_section=gm_json.JSONKEY_ACTUALRESULTS,
          setB_root=setB_root, setB_section=gm_json.JSONKEY_ACTUALRESULTS)
      if self._results:
        self._timestamp = int(time.time())
        logging.info('Number of download file collisions: %s' %
                     imagediffdb.global_file_collisions)
        logging.info('Results complete; took %d seconds.' %
                     (self._timestamp - time_start))
    finally:
      shutil.rmtree(tempdir)

  def _load_result_pairs(self, setA_root, setA_section, setB_root,
                         setB_section):
    """Loads all JSON image summaries from 2 directory trees and compares them.

    Args:
      setA_root: root directory containing JSON summaries of rendering results
      setA_section: which section (gm_json.JSONKEY_ACTUALRESULTS or
          gm_json.JSONKEY_EXPECTEDRESULTS) to load from the summaries in setA
      setB_root: root directory containing JSON summaries of rendering results
      setB_section: which section (gm_json.JSONKEY_ACTUALRESULTS or
          gm_json.JSONKEY_EXPECTEDRESULTS) to load from the summaries in setB

    Returns the summary of all image diff results (or None, depending on
    self._prefetch_only).
    """
    logging.info('Reading JSON image summaries from dirs %s and %s...' % (
        setA_root, setB_root))
    setA_dicts = self._read_dicts_from_root(setA_root)
    setB_dicts = self._read_dicts_from_root(setB_root)
    logging.info('Comparing summary dicts...')

    all_image_pairs = imagepairset.ImagePairSet(
        descriptions=(self._setA_label, self._setB_label),
        diff_base_url=self._diff_base_url)
    failing_image_pairs = imagepairset.ImagePairSet(
        descriptions=(self._setA_label, self._setB_label),
        diff_base_url=self._diff_base_url)

    # Override settings for columns that should be filtered using freeform text.
    for column_id in FREEFORM_COLUMN_IDS:
      factory = column.ColumnHeaderFactory(
          header_text=column_id, use_freeform_filter=True)
      all_image_pairs.set_column_header_factory(
          column_id=column_id, column_header_factory=factory)
      failing_image_pairs.set_column_header_factory(
          column_id=column_id, column_header_factory=factory)

    all_image_pairs.ensure_extra_column_values_in_summary(
        column_id=COLUMN__RESULT_TYPE, values=[
            results.KEY__RESULT_TYPE__FAILED,
            results.KEY__RESULT_TYPE__NOCOMPARISON,
            results.KEY__RESULT_TYPE__SUCCEEDED,
        ])
    failing_image_pairs.ensure_extra_column_values_in_summary(
        column_id=COLUMN__RESULT_TYPE, values=[
            results.KEY__RESULT_TYPE__FAILED,
            results.KEY__RESULT_TYPE__NOCOMPARISON,
        ])

    union_dict_paths = sorted(set(setA_dicts.keys() + setB_dicts.keys()))
    num_union_dict_paths = len(union_dict_paths)
    dict_num = 0
    for dict_path in union_dict_paths:
      dict_num += 1
      logging.info(
          'Asynchronously requesting pixel diffs for dict #%d of %d, "%s"...' %
          (dict_num, num_union_dict_paths, dict_path))

      dictA = self.get_default(setA_dicts, None, dict_path)
      self._validate_dict_version(dictA)
      dictA_results = self.get_default(dictA, {}, setA_section)

      dictB = self.get_default(setB_dicts, None, dict_path)
      self._validate_dict_version(dictB)
      dictB_results = self.get_default(dictB, {}, setB_section)

      skp_names = sorted(set(dictA_results.keys() + dictB_results.keys()))
      # Just for manual testing... truncate to an arbitrary subset.
      if self.truncate_results:
        skp_names = skp_names[1:3]
      for skp_name in skp_names:
        imagepairs_for_this_skp = []

        whole_image_A = self.get_default(
            dictA_results, None,
            skp_name, gm_json.JSONKEY_SOURCE_WHOLEIMAGE)
        whole_image_B = self.get_default(
            dictB_results, None,
            skp_name, gm_json.JSONKEY_SOURCE_WHOLEIMAGE)
        imagepairs_for_this_skp.append(self._create_image_pair(
            image_dict_A=whole_image_A, image_dict_B=whole_image_B,
            source_skp_name=skp_name, tilenum=None))

        tiled_images_A = self.get_default(
            dictA_results, None,
            skp_name, gm_json.JSONKEY_SOURCE_TILEDIMAGES)
        tiled_images_B = self.get_default(
            dictB_results, None,
            skp_name, gm_json.JSONKEY_SOURCE_TILEDIMAGES)
        # TODO(epoger): Report an error if we find tiles for A but not B?
        if tiled_images_A and tiled_images_B:
          # TODO(epoger): Report an error if we find a different number of tiles
          # for A and B?
          num_tiles = len(tiled_images_A)
          for tile_num in range(num_tiles):
            imagepairs_for_this_skp.append(self._create_image_pair(
                image_dict_A=tiled_images_A[tile_num],
                image_dict_B=tiled_images_B[tile_num],
                source_skp_name=skp_name, tilenum=tile_num))

        for one_imagepair in imagepairs_for_this_skp:
          if one_imagepair:
            all_image_pairs.add_image_pair(one_imagepair)
            result_type = one_imagepair.extra_columns_dict\
                [COLUMN__RESULT_TYPE]
            if result_type != results.KEY__RESULT_TYPE__SUCCEEDED:
              failing_image_pairs.add_image_pair(one_imagepair)

    if self._prefetch_only:
      return None
    else:
      return {
          results.KEY__HEADER__RESULTS_ALL: all_image_pairs.as_dict(
              column_ids_in_order=ORDERED_COLUMN_IDS),
          results.KEY__HEADER__RESULTS_FAILURES: failing_image_pairs.as_dict(
              column_ids_in_order=ORDERED_COLUMN_IDS),
      }

  def _validate_dict_version(self, result_dict):
    """Raises Exception if the dict is not the type/version we know how to read.

    Args:
      result_dict: dictionary holding output of render_pictures; if None,
          this method will return without raising an Exception
    """
    expected_header_type = 'ChecksummedImages'
    expected_header_revision = 1

    if result_dict == None:
      return
    header = result_dict[gm_json.JSONKEY_HEADER]
    header_type = header[gm_json.JSONKEY_HEADER_TYPE]
    if header_type != expected_header_type:
      raise Exception('expected header_type "%s", but got "%s"' % (
          expected_header_type, header_type))
    header_revision = header[gm_json.JSONKEY_HEADER_REVISION]
    if header_revision != expected_header_revision:
      raise Exception('expected header_revision %d, but got %d' % (
          expected_header_revision, header_revision))

  def _create_image_pair(self, image_dict_A, image_dict_B, source_skp_name,
                         tilenum):
    """Creates an ImagePair object for this pair of images.

    Args:
      image_dict_A: dict with JSONKEY_IMAGE_* keys, or None if no image
      image_dict_B: dict with JSONKEY_IMAGE_* keys, or None if no image
      source_skp_name: string; name of the source SKP file
      tilenum: which tile, or None if a wholeimage

    Returns:
      An ImagePair object, or None if both image_dict_A and image_dict_B are
      None.
    """
    if (not image_dict_A) and (not image_dict_B):
      return None

    def _checksum_and_relative_url(dic):
      if dic:
        return ((dic[gm_json.JSONKEY_IMAGE_CHECKSUMALGORITHM],
                 dic[gm_json.JSONKEY_IMAGE_CHECKSUMVALUE]),
                dic[gm_json.JSONKEY_IMAGE_FILEPATH])
      else:
        return None, None

    imageA_checksum, imageA_relative_url = _checksum_and_relative_url(
        image_dict_A)
    imageB_checksum, imageB_relative_url = _checksum_and_relative_url(
        image_dict_B)

    if not imageA_checksum:
      result_type = results.KEY__RESULT_TYPE__NOCOMPARISON
    elif not imageB_checksum:
      result_type = results.KEY__RESULT_TYPE__NOCOMPARISON
    elif imageA_checksum == imageB_checksum:
      result_type = results.KEY__RESULT_TYPE__SUCCEEDED
    else:
      result_type = results.KEY__RESULT_TYPE__FAILED

    extra_columns_dict = {
        COLUMN__RESULT_TYPE: result_type,
        COLUMN__SOURCE_SKP: source_skp_name,
    }
    if tilenum == None:
      extra_columns_dict[COLUMN__TILED_OR_WHOLE] = 'whole'
      extra_columns_dict[COLUMN__TILENUM] = 'N/A'
    else:
      extra_columns_dict[COLUMN__TILED_OR_WHOLE] = 'tiled'
      extra_columns_dict[COLUMN__TILENUM] = str(tilenum)

    try:
      return imagepair.ImagePair(
          image_diff_db=self._image_diff_db,
          base_url=self._image_base_gs_url,
          imageA_relative_url=imageA_relative_url,
          imageB_relative_url=imageB_relative_url,
          extra_columns=extra_columns_dict,
          download_all_images=self._download_all_images)
    except (KeyError, TypeError):
      logging.exception(
          'got exception while creating ImagePair for'
          ' urlPair=("%s","%s"), source_skp_name="%s", tilenum="%s"' % (
              imageA_relative_url, imageB_relative_url, source_skp_name,
              tilenum))
      return None

  def _copy_dir_contents(self, source_dir, dest_dir):
    """Copy all contents of source_dir into dest_dir, recursing into subdirs.

    Args:
      source_dir: path to source dir (GS URL, local filepath, or a special
          "repo:" URL type that points at a file within our Skia checkout)
      dest_dir: path to destination dir (local filepath)

    The copy operates as a "merge with overwrite": any files in source_dir will
    be "overlaid" on top of the existing content in dest_dir.  Existing files
    with the same names will be overwritten.
    """
    if gs_utils.GSUtils.is_gs_url(source_dir):
      (bucket, path) = gs_utils.GSUtils.split_gs_url(source_dir)
      self._gs.download_dir_contents(source_bucket=bucket, source_dir=path,
                                     dest_dir=dest_dir)
    elif source_dir.lower().startswith(REPO_URL_PREFIX):
      repo_dir = os.path.join(REPO_BASEPATH, source_dir[len(REPO_URL_PREFIX):])
      shutil.copytree(repo_dir, dest_dir)
    else:
      shutil.copytree(source_dir, dest_dir)
