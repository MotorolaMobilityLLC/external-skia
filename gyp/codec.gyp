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
        'libpng.gyp:libpng',
      ],
      'include_dirs': [
        '../include/codec',
        '../src/codec',
        '../src/core',
      ],
      'sources': [
        '../src/codec/SkCodec.cpp',
        '../src/codec/SkCodec_libbmp.cpp',
        '../src/codec/SkCodec_libico.cpp',
        '../src/codec/SkCodec_libpng.cpp',
        '../src/codec/SkMaskSwizzler.cpp',
        '../src/codec/SkMasks.cpp',
        '../src/codec/SkSwizzler.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/codec',
        ],
      },
    },
  ],
}
