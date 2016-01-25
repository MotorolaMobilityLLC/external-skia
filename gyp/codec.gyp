# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# GYP file for codec project.
{
  'targets': [
    {
      'target_name': 'codec',
      'product_name': 'skia_codec',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
        'giflib.gyp:giflib',
        'libjpeg-turbo-selector.gyp:libjpeg-turbo-selector',
        'libpng.gyp:libpng',
        'libwebp.gyp:libwebp',
      ],
      'cflags':[
        # FIXME: This gets around a longjmp warning. See
        # http://build.chromium.org/p/client.skia.compile/builders/Build-Ubuntu-GCC-x86_64-Release-Trybot/builds/113/steps/build%20most/logs/stdio
        '-Wno-clobbered -Wno-error',
      ],
      'include_dirs': [
        '../include/codec',
        '../include/private',
        '../src/codec',
        '../src/core',
        '../src/utils',
      ],
      'sources': [
        '../src/codec/SkAndroidCodec.cpp',
        '../src/codec/SkBmpCodec.cpp',
        '../src/codec/SkBmpMaskCodec.cpp',
        '../src/codec/SkBmpRLECodec.cpp',
        '../src/codec/SkBmpStandardCodec.cpp',
        '../src/codec/SkCodec.cpp',
        '../src/codec/SkGifCodec.cpp',
        '../src/codec/SkIcoCodec.cpp',
        '../src/codec/SkJpegCodec.cpp',
        '../src/codec/SkJpegDecoderMgr.cpp',
        '../src/codec/SkJpegUtility_codec.cpp',
        '../src/codec/SkMaskSwizzler.cpp',
        '../src/codec/SkMasks.cpp',
        '../src/codec/SkPngCodec.cpp',
        '../src/codec/SkSampler.cpp',
        '../src/codec/SkSampledCodec.cpp',
        '../src/codec/SkSwizzler.cpp',
        '../src/codec/SkWbmpCodec.cpp',
        '../src/codec/SkWebpAdapterCodec.cpp',
        '../src/codec/SkWebpCodec.cpp',

        '../src/codec/SkCodecImageGenerator.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/codec',
        ],
      },
      'defines': [
        'TURBO_HAS_SKIP',
      ],
      'conditions': [
        # FIXME: fix the support for Windows. (Issue with _hypot in DNG SDK).
        ['skia_codec_decodes_raw and skia_os != "win" and skia_os != "chromeos"', {
          'dependencies': [
            'raw_codec',
          ],
        },],
      ],
    }, {
      # RAW codec needs exceptions. Due to that, it is a separate target. Its usage can be
      # controlled by SK_CODEC_DECODES_RAW flag.
      'target_name': 'raw_codec',
      'product_name': 'raw_codec',
      'type': 'static_library',
      'dependencies': [
        'core.gyp:*',
        'dng_sdk.gyp:dng_sdk-selector',
        'libjpeg-turbo-selector.gyp:libjpeg-turbo-selector',
        'piex.gyp:piex-selector',
      ],
      'cflags':[
        '-fexceptions',
      ],
      'include_dirs': [
        '../include/codec',
        '../include/private',
        '../src/codec',
        '../src/core',
      ],
      'sources': [
        '../src/codec/SkRawAdapterCodec.cpp',
        '../src/codec/SkRawCodec.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/codec',
        ],
        'defines': [
          'SK_CODEC_DECODES_RAW',
        ],
      },
      'defines': [
        'SK_CODEC_DECODES_RAW',
      ],
      'conditions': [
        ['skia_arch_type == "x86" or skia_arch_type == "arm"', {
          'defines': [
            'qDNGBigEndian=0',
          ],
        }],
        ['skia_os == "ios" or skia_os == "mac"', {
          'xcode_settings': {
            'OTHER_CFLAGS': ['-fexceptions'],
            'OTHER_CPLUSPLUSFLAGS': ['-fexceptions'],
          },
        }],
      ],
    },
  ],
}
