/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkJpegUtility_MTK.h"

/////////////////////////////////////////////////////////////////////
static void sk_init_source_MTK(j_decompress_ptr_MTK cinfo) {
    skjpeg_source_mgr_MTK*  src = (skjpeg_source_mgr_MTK*)cinfo->src;
    src->next_input_byte = (const JOCTET_MTK*)src->fBuffer;
    src->bytes_in_buffer = 0;
#ifdef SK_JPEG_INDEX_SUPPORTED_MTK
    src->current_offset = 0;
#endif
    if (!src->fStream->rewind()) {
        SkDebugf("xxxxxxxxxxxxxx failure to rewind\n");
        cinfo->err->error_exit((j_common_ptr_MTK)cinfo);
    }
}

#ifdef SK_JPEG_INDEX_SUPPORTED_MTK
static boolean_MTK sk_seek_input_data_MTK(j_decompress_ptr_MTK cinfo, long byte_offset) {
    skjpeg_source_mgr_MTK* src = (skjpeg_source_mgr_MTK*)cinfo->src;
    size_t bo = (size_t) byte_offset;

    if (bo > src->current_offset) {
        (void)src->fStream->skip(bo - src->current_offset);
    } else {
        if (!src->fStream->rewind()) {
            SkDebugf("xxxxxxxxxxxxxx failure to rewind\n");
            cinfo->err->error_exit((j_common_ptr_MTK)cinfo);
            return false;
        }
        (void)src->fStream->skip(bo);
    }

    src->current_offset = bo;
    src->next_input_byte = (const JOCTET_MTK*)src->fBuffer;
    src->bytes_in_buffer = 0;
    return true;
}
#endif

static boolean_MTK sk_fill_input_buffer_MTK(j_decompress_ptr_MTK cinfo) {
    skjpeg_source_mgr_MTK* src = (skjpeg_source_mgr_MTK*)cinfo->src;
    if (src->fDecoder != nullptr && src->fDecoder->shouldCancelDecode()) {
        return FALSE_MTK;
    }
    size_t bytes = src->fStream->read(src->fBuffer, skjpeg_source_mgr_MTK::kBufferSize);
    // note that JPEG is happy with less than the full read,
    // as long as the result is non-zero
    if (bytes == 0) {
        return FALSE_MTK;
    }

#ifdef SK_JPEG_INDEX_SUPPORTED_MTK
    src->current_offset += bytes;
#endif
    src->next_input_byte = (const JOCTET_MTK*)src->fBuffer;
    src->bytes_in_buffer = bytes;
    return TRUE_MTK;
}

static void sk_skip_input_data_MTK(j_decompress_ptr_MTK cinfo, long num_bytes) {
    skjpeg_source_mgr_MTK*  src = (skjpeg_source_mgr_MTK*)cinfo->src;

    if (num_bytes > (long)src->bytes_in_buffer) {
        size_t bytesToSkip = num_bytes - src->bytes_in_buffer;
        while (bytesToSkip > 0) {
            size_t bytes = src->fStream->skip(bytesToSkip);
            if (bytes <= 0 || bytes > bytesToSkip) {
//              SkDebugf("xxxxxxxxxxxxxx failure to skip request %d returned %d\n", bytesToSkip, bytes);
                cinfo->err->error_exit((j_common_ptr_MTK)cinfo);
                return;
            }
#ifdef SK_JPEG_INDEX_SUPPORTED_MTK
            src->current_offset += bytes;
#endif
            bytesToSkip -= bytes;
        }
        src->next_input_byte = (const JOCTET_MTK*)src->fBuffer;
        src->bytes_in_buffer = 0;
    } else {
        src->next_input_byte += num_bytes;
        src->bytes_in_buffer -= num_bytes;
    }
}

static void sk_term_source_MTK(j_decompress_ptr_MTK /*cinfo*/) {}


///////////////////////////////////////////////////////////////////////////////

skjpeg_source_mgr_MTK::skjpeg_source_mgr_MTK(SkStream* stream, SkImageDecoder* decoder)
    : fStream(stream)
    , fDecoder(decoder) {

    init_source = sk_init_source_MTK;
    fill_input_buffer = sk_fill_input_buffer_MTK;
    skip_input_data = sk_skip_input_data_MTK;
    resync_to_restart = jpeg_resync_to_restart_MTK;
    term_source = sk_term_source_MTK;
#ifdef SK_JPEG_INDEX_SUPPORTED_MTK
    seek_input_data = sk_seek_input_data_MTK;
#endif
//    SkDebugf("**************** use memorybase %p %d\n", fMemoryBase, fMemoryBaseSize);
}

///////////////////////////////////////////////////////////////////////////////

static void sk_init_destination_MTK(j_compress_ptr_MTK cinfo) {
    skjpeg_destination_mgr_MTK* dest = (skjpeg_destination_mgr_MTK*)cinfo->dest;

    dest->next_output_byte = dest->fBuffer;
    dest->free_in_buffer = skjpeg_destination_mgr_MTK::kBufferSize;
}

static boolean_MTK sk_empty_output_buffer_MTK(j_compress_ptr_MTK cinfo) {
    skjpeg_destination_mgr_MTK* dest = (skjpeg_destination_mgr_MTK*)cinfo->dest;

//  if (!dest->fStream->write(dest->fBuffer, skjpeg_destination_mgr::kBufferSize - dest->free_in_buffer))
    if (!dest->fStream->write(dest->fBuffer,
            skjpeg_destination_mgr_MTK::kBufferSize)) {
        ERREXIT_MTK(cinfo, JERR_FILE_WRITE_MTK);
        return false;
    }

    dest->next_output_byte = dest->fBuffer;
    dest->free_in_buffer = skjpeg_destination_mgr_MTK::kBufferSize;
    return TRUE_MTK;
}

static void sk_term_destination_MTK (j_compress_ptr_MTK cinfo) {
    skjpeg_destination_mgr_MTK* dest = (skjpeg_destination_mgr_MTK*)cinfo->dest;

    size_t size = skjpeg_destination_mgr_MTK::kBufferSize - dest->free_in_buffer;
    if (size > 0) {
        if (!dest->fStream->write(dest->fBuffer, size)) {
            ERREXIT_MTK(cinfo, JERR_FILE_WRITE_MTK);
            return;
        }
    }
    dest->fStream->flush();
}

skjpeg_destination_mgr_MTK::skjpeg_destination_mgr_MTK(SkWStream* stream)
        : fStream(stream) {
    this->init_destination = sk_init_destination_MTK;
    this->empty_output_buffer = sk_empty_output_buffer_MTK;
    this->term_destination = sk_term_destination_MTK;
}

void skjpeg_error_exit_MTK(j_common_ptr_MTK cinfo) {
    skjpeg_error_mgr_MTK* error = (skjpeg_error_mgr_MTK*)cinfo->err;

    (*error->output_message) (cinfo);

    /* Let the memory manager delete any temp files before we die */
    jpeg_destroy_MTK(cinfo);

    longjmp(error->fJmpBuf, -1);
}
