# GYP for "dm" (Diamond Master, a.k.a Dungeon master, a.k.a GM 2).
{
    'includes': [ 'apptype_console.gypi' ],

    'targets': [{
        'target_name': 'dm',
        'type': 'executable',
        'includes': [
          'dm.gypi',
        ],
        'dependencies': ['xps.gyp:xps',],
        'conditions': [
          ['skia_android_framework', {
              'libraries': [
                  '-lskia',
                  '-landroid',
                  '-lgui',
                  '-lhwui',
                  '-lutils',
              ],
              'include_dirs': [
                  '../../../frameworks/base/libs/hwui/',
                  '../../../frameworks/native/include/',
              ],
              'sources': [
                '../dm/DMSrcSinkAndroid.cpp',
              ],
          }],
          ['skia_poppler_enabled', {
              'sources':      [ '../src/utils/SkPDFRasterizer.cpp' ],
              'defines':      [ 'SK_BUILD_POPPLER' ],
              'dependencies': [ 'poppler.gyp:*' ],
          }],
        ],
    }]
}
