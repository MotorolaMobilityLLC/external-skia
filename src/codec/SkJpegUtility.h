/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkJpegUtility_codec_DEFINED
#define SkJpegUtility_codec_DEFINED

#include "SkStream.h"

#include <setjmp.h>
// stdio is needed for jpeglib
#include <stdio.h>

extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

static constexpr uint32_t kICCMarker = JPEG_APP0 + 2;
static constexpr uint32_t kICCMarkerHeaderSize = 14;
static constexpr uint8_t kICCSig[] = {
        'I', 'C', 'C', '_', 'P', 'R', 'O', 'F', 'I', 'L', 'E', '\0',
};

/*
 * Error handling struct
 */
struct skjpeg_error_mgr : jpeg_error_mgr {
    jmp_buf fJmpBuf;
};

/*
 * Error handling function
 */
void skjpeg_err_exit(j_common_ptr cinfo);

/*
 * Source handling struct for that allows libjpeg to use our stream object
 */
struct skjpeg_source_mgr : jpeg_source_mgr {
    skjpeg_source_mgr(SkStream* stream);

    SkStream* fStream; // unowned
    enum {
        // TODO (msarett): Experiment with different buffer sizes.
        // This size was chosen because it matches SkImageDecoder.
        kBufferSize = 1024
    };
    uint8_t fBuffer[kBufferSize];
};

#endif
