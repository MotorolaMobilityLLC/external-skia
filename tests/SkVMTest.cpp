/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorPriv.h"
#include "include/private/SkColorData.h"
#include "src/core/SkVM.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/SkVMBuilders.h"

using Fmt = SrcoverBuilder_F32::Fmt;
const char* fmt_name(Fmt fmt) {
    switch (fmt) {
        case Fmt::A8:        return "A8";
        case Fmt::G8:        return "G8";
        case Fmt::RGBA_8888: return "RGBA_8888";
    }
    return "";
}

DEF_TEST(SkVM, r) {
    SkDynamicMemoryWStream buf;

    // Write all combinations of SrcoverBuilder_F32
    for (int s = 0; s < 3; s++)
    for (int d = 0; d < 3; d++) {
        auto srcFmt = (Fmt)s,
             dstFmt = (Fmt)d;
        SrcoverBuilder_F32 builder{srcFmt, dstFmt};
        skvm::Program program = builder.done();

        buf.writeText(fmt_name(srcFmt));
        buf.writeText(" over ");
        buf.writeText(fmt_name(dstFmt));
        buf.writeText("\n");
        builder.dump(&buf);
        buf.writeText("\n");
        program.dump(&buf);
        buf.writeText("\n");
    }

    // Write the I32 Srcovers also.
    {
        skvm::Program program = SrcoverBuilder_I32{}.done();
        buf.writeText("I32 8888 over 8888\n");
        program.dump(&buf);
        buf.writeText("\n");
    }
    {
        skvm::Program program = SrcoverBuilder_I32_SWAR{}.done();
        buf.writeText("I32 (SWAR) 8888 over 8888\n");
        program.dump(&buf);
        buf.writeText("\n");
    }

    sk_sp<SkData> blob = buf.detachAsData();
    {

        sk_sp<SkData> expected = GetResourceAsData("SkVMTest.expected");
        REPORTER_ASSERT(r, expected, "Couldn't load SkVMTest.expected.");
        if (expected) {
            if (blob->size() != expected->size()
                    || 0 != memcmp(blob->data(), expected->data(), blob->size())) {

                ERRORF(r, "SkVMTest expected\n%.*s\nbut got\n%.*s\n",
                       expected->size(), expected->data(),
                       blob->size(), blob->data());
            }

            SkFILEWStream out(GetResourcePath("SkVMTest.expected").c_str());
            if (out.isValid()) {
                out.write(blob->data(), blob->size());
            }
        }
    }

    auto test_8888 = [&](const skvm::Program& program) {
        uint32_t src[9];
        uint32_t dst[SK_ARRAY_COUNT(src)];

        for (int i = 0; i < (int)SK_ARRAY_COUNT(src); i++) {
            src[i] = 0xbb007733;
            dst[i] = 0xffaaccee;
        }

        SkPMColor expected = SkPMSrcOver(src[0], dst[0]);  // 0xff2dad73

        program.eval((int)SK_ARRAY_COUNT(src), src, dst);

        // dst is probably 0xff2dad72.
        for (auto got : dst) {
            auto want = expected;
            for (int i = 0; i < 4; i++) {
                uint8_t d = got  & 0xff,
                        w = want & 0xff;
                REPORTER_ASSERT(r, abs(d-w) < 2);
                got  >>= 8;
                want >>= 8;
            }
        }
    };

    test_8888(SrcoverBuilder_F32{Fmt::RGBA_8888, Fmt::RGBA_8888}.done());
    //test_8888(SrcoverBuilder_I32{}.done());
    test_8888(SrcoverBuilder_I32_SWAR{}.done());

    {
        skvm::Program program = SrcoverBuilder_F32{Fmt::RGBA_8888, Fmt::G8}.done();

        uint32_t src[9];
        uint8_t  dst[SK_ARRAY_COUNT(src)];

        for (int i = 0; i < (int)SK_ARRAY_COUNT(src); i++) {
            src[i] = 0xbb007733;
            dst[i] = 0x42;
        }

        SkPMColor over = SkPMSrcOver(SkPackARGB32(0xbb, 0x33, 0x77, 0x00),
                                     0xff424242);

        uint8_t want = SkComputeLuminance(SkGetPackedR32(over),
                                          SkGetPackedG32(over),
                                          SkGetPackedB32(over));
        program.eval((int)SK_ARRAY_COUNT(src), src, dst);

        for (auto got : dst) {
            REPORTER_ASSERT(r, abs(got-want) < 3);
        }
    }

    {
        skvm::Program program = SrcoverBuilder_F32{Fmt::A8, Fmt::A8}.done();

        uint8_t src[256],
                dst[256];
        for (int i = 0; i < 256; i++) {
            src[i] = 255 - i;
            dst[i] = i;
        }

        program.eval(256, src, dst);

        for (int i = 0; i < 256; i++) {
            uint8_t want = SkGetPackedA32(SkPMSrcOver(SkPackARGB32(src[i], 0,0,0),
                                                      SkPackARGB32(     i, 0,0,0)));
            REPORTER_ASSERT(r, abs(dst[i]-want) < 2);
        }
    }
}

DEF_TEST(SkVM_LoopCounts, r) {
    // Make sure we cover all the exact N we want.

    int buf[64];
    for (int N = 0; N <= (int)SK_ARRAY_COUNT(buf); N++) {
        for (int i = 0; i < (int)SK_ARRAY_COUNT(buf); i++) {
            buf[i] = i;
        }

        // buf[i] += 1
        skvm::Builder b;
        b.store32(b.arg(0),
                  b.add(b.splat(1),
                        b.load32(b.arg(0))));

        skvm::Program program = b.done();
        program.eval(N, buf);

        for (int i = 0; i < N; i++) {
            REPORTER_ASSERT(r, buf[i] == i+1);
        }
        for (int i = N; i < (int)SK_ARRAY_COUNT(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == i);
        }
    }
}
