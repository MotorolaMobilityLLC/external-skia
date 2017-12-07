
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkJpegUtility_DEFINED
#define SkJpegUtility_DEFINED

#include "SkImageDecoder.h"
#include "SkStream.h"

extern "C" {
    #include "jpeglib_MTK.h"
    #include "jerror_MTK.h"
}

#include <setjmp.h>

/* Our error-handling struct.
 *
*/
struct skjpeg_error_mgr_MTK : jpeg_error_mgr_MTK {
    jmp_buf fJmpBuf;
};


void skjpeg_error_exit_MTK(j_common_ptr_MTK cinfo);

///////////////////////////////////////////////////////////////////////////
/* Our source struct for directing jpeg to our stream object.
*/
struct skjpeg_source_mgr_MTK : jpeg_source_mgr_MTK {
    skjpeg_source_mgr_MTK(SkStream* stream, SkImageDecoder* decoder);

    // Unowned.
    SkStream*       fStream;
    // Unowned pointer to the decoder, used to check if the decoding process
    // has been cancelled.
    SkImageDecoder* fDecoder;
    enum {
        kBufferSize = 1024
    };
    char    fBuffer[kBufferSize];
};

/////////////////////////////////////////////////////////////////////////////
/* Our destination struct for directing decompressed pixels to our stream
 * object.
 */
struct skjpeg_destination_mgr_MTK : jpeg_destination_mgr_MTK {
    skjpeg_destination_mgr_MTK(SkWStream* stream);

    SkWStream*  fStream;

    enum {
        kBufferSize = 1024
    };
    uint8_t fBuffer[kBufferSize];
};

#endif
