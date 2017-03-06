Testing Skia with MSAN, ASAN, and TSAN
======================================

Get Clang binaries
------------------

    CLANGDIR="${HOME}/clang"

    tools/git-sync-deps
    CC= CXX= infra/bots/assets/clang_linux/create.py -t "$CLANGDIR"

Configure and Compile Skia with MSAN
------------------------------------

    CLANGDIR="${HOME}/clang"
    mkdir -p out/msan
    cat > out/msan/args.gn <<- EOF
        cc = "${CLANGDIR}/bin/clang"
        cxx = "${CLANGDIR}/bin/clang++"
        extra_ldflags = [ "-Wl,-rpath", "-Wl,${CLANGDIR}/msan" ]
        sanitize = "MSAN"
        skia_use_fontconfig = false
    EOF
    tools/git-sync-deps
    bin/gn gen out/msan
    ninja -C out/msan

Configure and Compile Skia with ASAN
------------------------------------

    CLANGDIR="${HOME}/clang"
    mkdir -p out/asan
    cat > out/asan/args.gn <<- EOF
        cc = "${CLANGDIR}/bin/clang"
        cxx = "${CLANGDIR}/bin/clang++"
        sanitize = "ASAN"
    EOF
    tools/git-sync-deps
    bin/gn gen out/asan
    ninja -C out/asan

Configure and Compile Skia with TSAN
------------------------------------

    CLANGDIR="${HOME}/clang"
    mkdir -p out/tsan
    cat > out/tsan/args.gn <<- EOF
        cc = "${CLANGDIR}/bin/clang"
        cxx = "${CLANGDIR}/bin/clang++"
        sanitize = "TSAN"
        is_debug = false
    EOF
    tools/git-sync-deps
    bin/gn gen out/tsan
    ninja -C out/tsan


