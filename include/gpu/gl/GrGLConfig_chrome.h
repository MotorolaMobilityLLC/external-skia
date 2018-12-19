
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrGLConfig_chrome_DEFINED
#define GrGLConfig_chrome_DEFINED

// glGetError() forces a sync with gpu process on chrome
#define GR_GL_CHECK_ERROR_START                     0

// cmd buffer allocates memory and memsets it to zero when it sees glBufferData
// with NULL.
#define GR_GL_USE_BUFFER_DATA_NULL_HINT             0

// Check error is even more expensive in chrome (cmd buffer flush). The
// compositor also doesn't check its allocations.
#define GR_GL_CHECK_ALLOC_WITH_GET_ERROR            0

// Non-VBO vertices and indices are not allowed in Chromium.
#define GR_GL_MUST_USE_VBO                          1

#if !defined(GR_GL_IGNORE_ES3_MSAA)
    #define GR_GL_IGNORE_ES3_MSAA 1
#endif

#endif
