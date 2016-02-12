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
# For bugs, please contact scroggo@google.com or djsollen@google.com
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

# diamond-master (one test to rule them all)
include $(BASE_PATH)/dm/Android.mk
"""
)

STATIC_HEADER = (
"""
###############################################################################
# STATIC LIBRARY
#
# This target is only to be used internally for only one of two purposes...
#  (1) statically linking into testing frameworks
#  (2) as an inclusion target for the libskia.so shared library
###############################################################################

"""
)

SHARED_HEADER = (
"""
###############################################################################
# SHARED LIBRARY
###############################################################################

"""
)

STATIC_DEPS_INFO = (
"""
###############################################################################
# 
# This file contains the shared and static dependencies needed by any target 
# that attempts to statically link Skia (i.e. libskia_static build target).
#
# This is a workaround for the fact that the build system does not add these
# transitive dependencies when it attempts to link libskia_static into another
# library.
#
###############################################################################
"""
)

CLEAR_VARS = ("""include $(CLEAR_VARS)\n""")
LOCAL_PATH = ("""LOCAL_PATH:= $(call my-dir)\n""")

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

def write_static_deps_mk(target_dir, common, deviations_from_common):
  """Given all the variables, write the final make file.

  Args:
    target_dir: The full path to the directory to write skia_static_includes.mk,
      or None to use the current working directory.
    common: VarsDict holding variables definitions common to all
      configurations.
    deviations_from_common: List of VarsDictData, one for each possible
      configuration. VarsDictData.name will be appended to each key before
      writing it to the makefile. VarsDictData.condition, if not None, will be
      written to the makefile as a condition to determine whether to include
      VarsDictData.vars_dict.
  """
  target_file = 'skia_static_deps.mk'
  if target_dir:
    target_file = os.path.join(target_dir, target_file)
  with open(target_file, 'w') as f:
    f.write(AUTOGEN_WARNING)
    f.write(STATIC_DEPS_INFO)

    for data in deviations_from_common:
      var_dict_shared = data.vars_dict['LOCAL_SHARED_LIBRARIES']
      var_dict_static = data.vars_dict['LOCAL_STATIC_LIBRARIES']
      if data.condition and (var_dict_shared or var_dict_static):
        f.write('ifeq ($(%s), true)\n' % data.condition)
      write_group(f, 'LOCAL_SHARED_LIBRARIES', var_dict_shared, True)
      write_group(f, 'LOCAL_STATIC_LIBRARIES', var_dict_static, True)
      if data.condition and (var_dict_shared or var_dict_static):
        f.write('endif\n\n')

    write_group(f, 'LOCAL_SHARED_LIBRARIES', common['LOCAL_SHARED_LIBRARIES'],
                True)
    write_group(f, 'LOCAL_STATIC_LIBRARIES', common['LOCAL_STATIC_LIBRARIES'],
                True)


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
    f.write(LOCAL_PATH)

    f.write(DEBUGGING_HELP)

    f.write(STATIC_HEADER)
    f.write(CLEAR_VARS)

    # need flags to enable feedback driven optimization (FDO) when requested
    # by the build system.
    f.write('LOCAL_FDO_SUPPORT := true\n')
    f.write('ifneq ($(strip $(TARGET_FDO_CFLAGS)),)\n')
    f.write('\t# This should be the last -Oxxx specified in LOCAL_CFLAGS\n')
    f.write('\tLOCAL_CFLAGS += -O2\n')
    f.write('endif\n\n')

    f.write('LOCAL_ARM_MODE := thumb\n')

    f.write('# used for testing\n')
    f.write('#LOCAL_CFLAGS += -g -O0\n\n')

    # update the provided LOCAL_MODULE with a _static suffix
    local_module = common['LOCAL_MODULE'][0]
    static_local_module = local_module + '_static'
    common['LOCAL_MODULE'].reset()
    common['LOCAL_MODULE'].add(static_local_module)

    write_local_vars(f, common, False, None)

    for data in deviations_from_common:
      if data.condition:
        f.write('ifeq ($(%s), true)\n' % data.condition)
      write_local_vars(f, data.vars_dict, True, data.name)
      if data.condition:
        f.write('endif\n\n')

    f.write('LOCAL_MODULE_CLASS := STATIC_LIBRARIES\n')
    f.write('include $(BUILD_STATIC_LIBRARY)\n\n')

    f.write(SHARED_HEADER)
    f.write(CLEAR_VARS)
    f.write('LOCAL_MODULE_CLASS := SHARED_LIBRARIES\n')
    f.write('LOCAL_MODULE := %s\n' % local_module)
    f.write('LOCAL_WHOLE_STATIC_LIBRARIES := %s\n' % static_local_module)
    write_group(f, 'LOCAL_EXPORT_C_INCLUDE_DIRS',
                common['LOCAL_EXPORT_C_INCLUDE_DIRS'], False)
    f.write('include $(BASE_PATH)/skia_static_deps.mk\n')
    f.write('include $(BUILD_SHARED_LIBRARY)\n')

    f.write(SKIA_TOOLS)

