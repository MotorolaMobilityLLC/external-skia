use_relative_paths = True

vars = {
  "checkout_chromium": False,

  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling different
  # dependencies without interference from each other.
  'sk_tool_revision': 'git_revision:bccbf0e3ae0f6382cb48473f13f4751b9d56d9cc',
}

deps = {
  "buildtools"                            : "https://chromium.googlesource.com/chromium/buildtools.git@505de88083136eefd056e5ee4ca0f01fe9b33de8",
  "third_party/externals/angle2"          : "https://chromium.googlesource.com/angle/angle.git@930db294639dc67d0c971029c290cf819b731aa9",
  "third_party/externals/brotli"          : "https://skia.googlesource.com/external/github.com/google/brotli.git@e61745a6b7add50d380cfd7d3883dd6c62fc2c71",
  "third_party/externals/d3d12allocator"  : "https://skia.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git@169895d529dfce00390a20e69c2f516066fe7a3b",
  # Dawn requires jinja2 and markupsafe for the code generator, and tint for SPIRV compilation.
  # When the Dawn revision is updated these should be updated from the Dawn DEPS as well.
  "third_party/externals/dawn"            : "https://dawn.googlesource.com/dawn.git@f0fdfa0d61dfa9486914fd278985d2de880be51a",
  "third_party/externals/jinja2"          : "https://chromium.googlesource.com/chromium/src/third_party/jinja2@a82a4944a7f2496639f34a89c9923be5908b80aa",
  "third_party/externals/markupsafe"      : "https://chromium.googlesource.com/chromium/src/third_party/markupsafe@0944e71f4b2cb9a871bcbe353f95e889b64a611a",
  "third_party/externals/tint"            : "https://dawn.googlesource.com/tint@9fdfa1e32374f8d862c98ad241757853386f66e6",
  "third_party/externals/dng_sdk"         : "https://android.googlesource.com/platform/external/dng_sdk.git@c8d0c9b1d16bfda56f15165d39e0ffa360a11123",
  "third_party/externals/egl-registry"    : "https://skia.googlesource.com/external/github.com/KhronosGroup/EGL-Registry@a0bca08de07c7d7651047bedc0b653cfaaa4f2ae",
  "third_party/externals/expat"           : "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git@e976867fb57a0cd87e3b0fe05d59e0ed63c6febb",
  "third_party/externals/freetype"        : "https://chromium.googlesource.com/chromium/src/third_party/freetype2.git@f9350be1e45baa1c29f7551274982262f8e769ce",
  "third_party/externals/harfbuzz"        : "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git@3a74ee528255cc027d84b204a87b5c25e47bff79",
  "third_party/externals/icu"             : "https://chromium.googlesource.com/chromium/deps/icu.git@dbd3825b31041d782c5b504c59dcfb5ac7dda08c",
  "third_party/externals/imgui"           : "https://skia.googlesource.com/external/github.com/ocornut/imgui.git@9418dcb69355558f70de260483424412c5ca2fce",
  "third_party/externals/libgifcodec"     : "https://skia.googlesource.com/libgifcodec@fd59fa92a0c86788dcdd84d091e1ce81eda06a77",
  "third_party/externals/libjpeg-turbo"   : "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git@24e310554f07c0fdb8ee52e3e708e4f3e9eb6e20",
  "third_party/externals/libpng"          : "https://skia.googlesource.com/third_party/libpng.git@386707c6d19b974ca2e3db7f5c61873813c6fe44",
  "third_party/externals/libwebp"         : "https://chromium.googlesource.com/webm/libwebp.git@fedac6cc69cda3e9e04b780d324cf03921fb3ff4",
  "third_party/externals/lua"             : "https://skia.googlesource.com/external/github.com/lua/lua.git@e354c6355e7f48e087678ec49e340ca0696725b1",
  "third_party/externals/microhttpd"      : "https://android.googlesource.com/platform/external/libmicrohttpd@748945ec6f1c67b7efc934ab0808e1d32f2fb98d",
  "third_party/externals/oboe"            : "https://chromium.googlesource.com/external/github.com/google/oboe.git@b02a12d1dd821118763debec6b83d00a8a0ee419",
  "third_party/externals/opencl-lib"      : "https://skia.googlesource.com/external/github.com/GPUOpen-Tools/common-lib-amd-APPSDK-3.0@4e6d30e406d2e5a65e1d65e404fe6df5f772a32b",
  "third_party/externals/opencl-registry" : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenCL-Registry@932ed55c85f887041291cef8019e54280c033c35",
  "third_party/externals/opengl-registry" : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenGL-Registry@14b80ebeab022b2c78f84a573f01028c96075553",
  "third_party/externals/piex"            : "https://android.googlesource.com/platform/external/piex.git@bb217acdca1cc0c16b704669dd6f91a1b509c406",
  "third_party/externals/sdl"             : "https://skia.googlesource.com/third_party/sdl@5d7cfcca344034aff9327f77fc181ae3754e7a90",
  "third_party/externals/sfntly"          : "https://chromium.googlesource.com/external/github.com/googlei18n/sfntly.git@b55ff303ea2f9e26702b514cf6a3196a2e3e2974",
  "third_party/externals/spirv-cross"     : "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross@bdbef7b1f3982fe99a62d076043036abe6dd6d80",
  "third_party/externals/spirv-headers"   : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git@bcf55210f13a4fa3c3d0963b509ff1070e434c79",
  "third_party/externals/spirv-tools"     : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git@5d8c40399e1a483c168246215e4637e19551cd4f",
  "third_party/externals/swiftshader"     : "https://swiftshader.googlesource.com/SwiftShader@be169ef352382a3884eb3e7a72235e5f04255f1d",
  #"third_party/externals/v8"              : "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",
  "third_party/externals/wuffs"           : "https://skia.googlesource.com/external/github.com/google/wuffs.git@f49c38202914c289621f547ff016e5f02c994dda",
  "third_party/externals/zlib"            : "https://chromium.googlesource.com/chromium/src/third_party/zlib@c876c8f87101c5a75f6014b0f832499afeb65b73",

  "../src": {
    "url": "https://chromium.googlesource.com/chromium/src.git@51f0f14b5681f82da2be45b1017083d073572658",
    "condition": "checkout_chromium",
  },

  'bin': {
    'packages': [
      {
        'package': 'skia/tools/sk/${{platform}}',
        'version': Var('sk_tool_revision'),
      }
    ],
    'dep_type': 'cipd',
  },
}

recursedeps = [
  "../src",
]

gclient_gn_args_from = 'src'
