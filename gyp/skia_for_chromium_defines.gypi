# Copyright 2014 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is included by chrome's skia/skia_common.gypi, and is intended to
# augment the skia flags that are set there.

{
  'variables': {

    # These flags will be defined in chromium
    #
    # If these become 'permanent', they should be moved into skia_common.gypi
    #
    'skia_for_chromium_defines': [
      'SK_SUPPORT_LEGACY_GETTOPDEVICE',
      'SK_SUPPORT_LEGACY_ASIMAGEINFO',
      'SK_SUPPORT_LEGACY_SETCONFIG_INFO',
      'SK_SUPPORT_LEGACY_N32_NAME',
      'SK_IGNORE_CORRECT_HIGH_QUALITY_IMAGE_SCALE',
      'SK_SUPPORT_LEGACY_INSTALLPIXELSPARAMS',
      'SK_SUPPORT_LEGACY_IMAGEGENERATORAPI',
    ],
  },
}
