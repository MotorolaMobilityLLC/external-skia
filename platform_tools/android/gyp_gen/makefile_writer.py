#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Functions for creating an Android.mk from already created dictionaries.
"""

import os

def write_group(f, name, items, append):
  """Helper function to list all names passed to a variable.

  Args:
    f: File open for writing (Android.mk)
    name: Name of the makefile variable (e.g. LOCAL_CFLAGS)
    items: list of strings to be passed to the variable.
    append: Whether to append to the variable or overwrite it.
  """
  if not items:
    return

  # Copy the list so we can prepend it with its name.
  items_to_write = list(items)

  if append:
    items_to_write.insert(0, '%s +=' % name)
  else:
    items_to_write.insert(0, '%s :=' % name)

  f.write(' \\\n\t'.join(items_to_write))

  f.write('\n\n')


def write_local_vars(f, var_dict, append, name):
  """Helper function to write all the members of var_dict to the makefile.

  Args:
    f: File open for writing (Android.mk)
    var_dict: VarsDict holding the unique values for one configuration.
    append: Whether to append to each makefile variable or overwrite it.
    name: If not None, a string to be appended to each key.
  """
  for key in var_dict.keys():
    _key = key
    _items = var_dict[key]
    if key == 'LOCAL_CFLAGS':
      # Always append LOCAL_CFLAGS. This allows us to define some early on in
      # the makefile and not overwrite them.
      _append = True
    elif key == 'DEFINES':
      # For DEFINES, we want to append to LOCAL_CFLAGS.
      _append = True
      _key = 'LOCAL_CFLAGS'
      _items_with_D = []
      for define in _items:
        _items_with_D.append('-D' + define)
      _items = _items_with_D
    elif key == 'KNOWN_TARGETS':
      # KNOWN_TARGETS are not needed in the final make file.
      continue
    else:
      _append = append
    if name:
      _key += '_' + name
    write_group(f, _key, _items, _append)


AUTOGEN_WARNING = (
"""
###############################################################################
#
# THIS FILE IS AUTOGENERATED BY GYP_TO_ANDROID.PY. DO NOT EDIT.
#
###############################################################################

"""
)

DEBUGGING_HELP = (
"""
###############################################################################
#
# PROBLEMS WITH SKIA DEBUGGING?? READ THIS...
#
# The debug build results in changes to the Skia headers. This means that those
# using libskia must also be built with the debug version of the Skia headers.
# There are a few scenarios where this comes into play:
#
# (1) You're building debug code that depends on libskia.
#   (a) If libskia is built in release, then define SK_RELEASE when building
#       your sources.
#   (b) If libskia is built with debugging (see step 2), then no changes are
#       needed since your sources and libskia have been built with SK_DEBUG.
# (2) You're building libskia in debug mode.
#   (a) RECOMMENDED: You can build the entire system in debug mode. Do this by
#       updating your build/core/config.mk to include -DSK_DEBUG on the line
#       that defines COMMON_GLOBAL_CFLAGS
#   (b) You can update all the users of libskia to define SK_DEBUG when they are
#       building their sources.
#
# NOTE: If neither SK_DEBUG or SK_RELEASE are defined then Skia checks NDEBUG to
#       determine which build type to use.
###############################################################################

"""
)

SKIA_TOOLS = (
"""
#############################################################
# Build the skia tools
#

# benchmark (timings)
include $(BASE_PATH)/bench/Android.mk
include $(BASE_PATH)/tools/Android.mk

# golden-master (fidelity / regression test)
include $(BASE_PATH)/gm/Android.mk

# unit-tests
include $(BASE_PATH)/tests/Android.mk

# diamond-master (one test to rule them all)
include $(BASE_PATH)/dm/Android.mk
"""
)


class VarsDictData(object):
  """Helper class to keep a VarsDict along with a name and optional condition.
  """
  def __init__(self, vars_dict, name, condition=None):
    """Create a new VarsDictData.

    Args:
      vars_dict: A VarsDict. Can be accessed via self.vars_dict.
      name: Name associated with the VarsDict. Can be accessed via
        self.name.
      condition: Optional string representing a condition. If not None,
        used to create a conditional inside the makefile.
    """
    self.vars_dict = vars_dict
    self.condition = condition
    self.name = name

def write_local_path(f):
  """Add the LOCAL_PATH line to the makefile.

  Args:
    f: File open for writing.
  """
  f.write('LOCAL_PATH:= $(call my-dir)\n')

def write_clear_vars(f):
  """Add the CLEAR_VARS line to the makefile.

  Args:
    f: File open for writing.
  """
  f.write('include $(CLEAR_VARS)\n')

def write_include_stlport(f):
  """Add a line to include stlport.

  Args:
    f: File open for writing.
  """
  f.write('include external/stlport/libstlport.mk\n')

def write_android_mk(target_dir, common, deviations_from_common):
  """Given all the variables, write the final make file.

  Args:
    target_dir: The full path to the directory to write Android.mk, or None
      to use the current working directory.
    common: VarsDict holding variables definitions common to all
      configurations.
    deviations_from_common: List of VarsDictData, one for each possible
      configuration. VarsDictData.name will be appended to each key before
      writing it to the makefile. VarsDictData.condition, if not None, will be
      written to the makefile as a condition to determine whether to include
      VarsDictData.vars_dict.
  """
  target_file = 'Android.mk'
  if target_dir:
    target_file = os.path.join(target_dir, target_file)
  with open(target_file, 'w') as f:
    f.write(AUTOGEN_WARNING)
    f.write('BASE_PATH := $(call my-dir)\n')
    write_local_path(f)

    f.write(DEBUGGING_HELP)

    write_clear_vars(f)
    f.write('LOCAL_ARM_MODE := thumb\n')

    # need a flag to tell the C side when we're on devices with large memory
    # budgets (i.e. larger than the low-end devices that initially shipped)
    # On arm, only define the flag if it has VFP. For all other architectures,
    # always define the flag.
    f.write('ifeq ($(TARGET_ARCH),arm)\n')
    f.write('\tifeq ($(ARCH_ARM_HAVE_VFP),true)\n')
    f.write('\t\tLOCAL_CFLAGS += -DANDROID_LARGE_MEMORY_DEVICE\n')
    f.write('\tendif\n')
    f.write('else\n')
    f.write('\tLOCAL_CFLAGS += -DANDROID_LARGE_MEMORY_DEVICE\n')
    f.write('endif\n\n')

    f.write('# used for testing\n')
    f.write('#LOCAL_CFLAGS += -g -O0\n\n')

    f.write('ifeq ($(NO_FALLBACK_FONT),true)\n')
    f.write('\tLOCAL_CFLAGS += -DNO_FALLBACK_FONT\n')
    f.write('endif\n\n')

    write_local_vars(f, common, False, None)

    for data in deviations_from_common:
      if data.condition:
        f.write('ifeq ($(%s), true)\n' % data.condition)
      write_local_vars(f, data.vars_dict, True, data.name)
      if data.condition:
        f.write('endif\n\n')

    write_include_stlport(f)
    f.write('include $(BUILD_SHARED_LIBRARY)\n')
    f.write(SKIA_TOOLS)

