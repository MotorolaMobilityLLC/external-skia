#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

ImagePairSet class; see its docstring below.
"""

import column

# Keys used within dictionary representation of ImagePairSet.
KEY__COLUMNHEADERS = 'columnHeaders'
KEY__IMAGEPAIRS = 'imagePairs'
KEY__IMAGESETS = 'imageSets'
KEY__IMAGESETS__BASE_URL = 'baseUrl'
KEY__IMAGESETS__DESCRIPTION = 'description'

DEFAULT_DESCRIPTIONS = ('setA', 'setB')


class ImagePairSet(object):
  """A collection of ImagePairs, representing two arbitrary sets of images.

  These could be:
  - images generated before and after a code patch
  - expected and actual images for some tests
  - or any other pairwise set of images.
  """

  def __init__(self, descriptions=None):
    """
    Args:
      descriptions: a (string, string) tuple describing the two image sets.
          If not specified, DEFAULT_DESCRIPTIONS will be used.
    """
    self._column_header_factories = {}
    self._descriptions = descriptions or DEFAULT_DESCRIPTIONS
    self._extra_column_tallies = {}  # maps column_id -> values
                                     #                -> instances_per_value
    self._image_pair_dicts = []

  def add_image_pair(self, image_pair):
    """Adds an ImagePair; this may be repeated any number of times."""
    # Special handling when we add the first ImagePair...
    if not self._image_pair_dicts:
      self._base_url = image_pair.base_url

    if image_pair.base_url != self._base_url:
      raise Exception('added ImagePair with base_url "%s" instead of "%s"' % (
          image_pair.base_url, self._base_url))
    self._image_pair_dicts.append(image_pair.as_dict())
    extra_columns_dict = image_pair.extra_columns_dict
    if extra_columns_dict:
      for column_id, value in extra_columns_dict.iteritems():
        self._add_extra_column_entry(column_id, value)

  def set_column_header_factory(self, column_id, column_header_factory):
    """Overrides the default settings for one of the extraColumn headers.

    Args:
      column_id: string; unique ID of this column (must match a key within
          an ImagePair's extra_columns dictionary)
      column_header_factory: a ColumnHeaderFactory object
    """
    self._column_header_factories[column_id] = column_header_factory

  def get_column_header_factory(self, column_id):
    """Returns the ColumnHeaderFactory object for a particular extraColumn.

    Args:
      column_id: string; unique ID of this column (must match a key within
          an ImagePair's extra_columns dictionary)
    """
    column_header_factory = self._column_header_factories.get(column_id, None)
    if not column_header_factory:
      column_header_factory = column.ColumnHeaderFactory(header_text=column_id)
      self._column_header_factories[column_id] = column_header_factory
    return column_header_factory

  def _add_extra_column_entry(self, column_id, value):
    """Records one column_id/value extraColumns pair found within an ImagePair.

    We use this information to generate tallies within the column header
    (how many instances we saw of a particular value, within a particular
    extraColumn).
    """
    known_values_for_column = self._extra_column_tallies.get(column_id, None)
    if not known_values_for_column:
      known_values_for_column = {}
      self._extra_column_tallies[column_id] = known_values_for_column
    instances_of_this_value = known_values_for_column.get(value, 0)
    instances_of_this_value += 1
    known_values_for_column[value] = instances_of_this_value

  def _column_headers_as_dict(self):
    """Returns all column headers as a dictionary."""
    asdict = {}
    for column_id, values_for_column in self._extra_column_tallies.iteritems():
      column_header_factory = self.get_column_header_factory(column_id)
      asdict[column_id] = column_header_factory.create_as_dict(
          values_for_column)
    return asdict

  def as_dict(self):
    """Returns a dictionary describing this package of ImagePairs.

    Uses the KEY__* constants as keys.
    """
    return {
        KEY__COLUMNHEADERS: self._column_headers_as_dict(),
        KEY__IMAGEPAIRS: self._image_pair_dicts,
        KEY__IMAGESETS: [{
            KEY__IMAGESETS__BASE_URL: self._base_url,
            KEY__IMAGESETS__DESCRIPTION: self._descriptions[0],
        }, {
            KEY__IMAGESETS__BASE_URL: self._base_url,
            KEY__IMAGESETS__DESCRIPTION: self._descriptions[1],
        }],
    }
