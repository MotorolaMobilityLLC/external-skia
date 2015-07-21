/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextBlobCache.h"

static const int kVerticesPerGlyph = 4;

GrTextBlobCache::~GrTextBlobCache() {
    this->freeAll();
}

GrAtlasTextBlob* GrTextBlobCache::createBlob(int glyphCount, int runCount,
                                                                size_t maxVASize) {
    // We allocate size for the GrAtlasTextBlob itself, plus size for the vertices array,
    // and size for the glyphIds array.
    size_t verticesCount = glyphCount * kVerticesPerGlyph * maxVASize;
    size_t size = sizeof(GrAtlasTextBlob) +
                  verticesCount +
                  glyphCount * sizeof(GrGlyph**) +
                  sizeof(GrAtlasTextBlob::Run) * runCount;

    GrAtlasTextBlob* cacheBlob = SkNEW_PLACEMENT(fPool.allocate(size), GrAtlasTextBlob);

    // setup offsets for vertices / glyphs
    cacheBlob->fVertices = sizeof(GrAtlasTextBlob) + reinterpret_cast<unsigned char*>(cacheBlob);
    cacheBlob->fGlyphs = reinterpret_cast<GrGlyph**>(cacheBlob->fVertices + verticesCount);
    cacheBlob->fRuns = reinterpret_cast<GrAtlasTextBlob::Run*>(cacheBlob->fGlyphs + glyphCount);

    // Initialize runs
    for (int i = 0; i < runCount; i++) {
        SkNEW_PLACEMENT(&cacheBlob->fRuns[i], GrAtlasTextBlob::Run);
    }
    cacheBlob->fRunCount = runCount;
    cacheBlob->fPool = &fPool;
    return cacheBlob;
}

void GrTextBlobCache::freeAll() {
    SkTDynamicHash<GrAtlasTextBlob, GrAtlasTextBlob::Key>::Iter iter(&fCache);
    while (!iter.done()) {
        GrAtlasTextBlob* blob = &(*iter);
        fBlobList.remove(blob);
        blob->unref();
        ++iter;
    }
    fCache.rewind();
}
