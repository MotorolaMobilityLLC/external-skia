/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkDraw.h"
#include "SkGlyphCache.h"
#include "SkGraphics.h"
#include "SkMutex.h"
#include "SkRemoteGlyphCache.h"
#include "SkRemoteGlyphCacheImpl.h"
#include "SkStrikeCache.h"
#include "SkSurface.h"
#include "SkSurfacePriv.h"
#include "SkTestEmptyTypeface.h"
#include "SkTextBlob.h"
#include "SkTypeface_remote.h"
#include "Test.h"

#include "text/GrTextContext.h"

class DiscardableManager : public SkStrikeServer::DiscardableHandleManager,
                           public SkStrikeClient::DiscardableHandleManager {
public:
    DiscardableManager() { sk_bzero(&fCacheMissCount, sizeof(fCacheMissCount)); }
    ~DiscardableManager() override = default;

    // Server implementation.
    SkDiscardableHandleId createHandle() override {
        // Handles starts as locked.
        fLockedHandles.add(++fNextHandleId);
        return fNextHandleId;
    }
    bool lockHandle(SkDiscardableHandleId id) override {
        if (id <= fLastDeletedHandleId) return false;
        fLockedHandles.add(id);
        return true;
    }

    // Client implementation.
    bool deleteHandle(SkDiscardableHandleId id) override { return id <= fLastDeletedHandleId; }
    void notifyCacheMiss(SkStrikeClient::CacheMissType type) override { fCacheMissCount[type]++; }
    bool isHandleDeleted(SkDiscardableHandleId id) override { return id <= fLastDeletedHandleId; }

    void unlockAll() { fLockedHandles.reset(); }
    void unlockAndDeleteAll() {
        unlockAll();
        fLastDeletedHandleId = fNextHandleId;
    }
    const SkTHashSet<SkDiscardableHandleId>& lockedHandles() const { return fLockedHandles; }
    SkDiscardableHandleId handleCount() { return fNextHandleId; }
    int cacheMissCount(uint32_t type) { return fCacheMissCount[type]; }
    bool hasCacheMiss() const {
        for (uint32_t i = 0; i <= SkStrikeClient::CacheMissType::kLast; ++i) {
            if (fCacheMissCount[i] > 0) return true;
        }
        return false;
    }

private:
    SkDiscardableHandleId fNextHandleId = 0u;
    SkDiscardableHandleId fLastDeletedHandleId = 0u;
    SkTHashSet<SkDiscardableHandleId> fLockedHandles;
    int fCacheMissCount[SkStrikeClient::CacheMissType::kLast + 1u];
};

sk_sp<SkTextBlob> buildTextBlob(sk_sp<SkTypeface> tf, int glyphCount) {
    SkFont font;
    font.setTypeface(tf);
    font.setHinting(kNormal_SkFontHinting);
    font.setSize(1u);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);

    SkTextBlobBuilder builder;
    SkRect bounds = SkRect::MakeWH(10, 10);
    const auto& runBuffer = builder.allocRunPosH(font, glyphCount, 0, &bounds);
    SkASSERT(runBuffer.utf8text == nullptr);
    SkASSERT(runBuffer.clusters == nullptr);

    for (int i = 0; i < glyphCount; i++) {
        runBuffer.glyphs[i] = static_cast<SkGlyphID>(i);
        runBuffer.pos[i] = SkIntToScalar(i);
    }
    return builder.make();
}

static void compare_blobs(const SkBitmap& expected, const SkBitmap& actual,
                          skiatest::Reporter* reporter, int tolerance = 0) {
    SkASSERT(expected.width() == actual.width());
    SkASSERT(expected.height() == actual.height());
    for (int i = 0; i < expected.width(); ++i) {
        for (int j = 0; j < expected.height(); ++j) {
            SkColor expectedColor = expected.getColor(i, j);
            SkColor actualColor = actual.getColor(i, j);
            if (0 == tolerance) {
                REPORTER_ASSERT(reporter, expectedColor == actualColor);
            } else {
                for (int k = 0; k < 4; ++k) {
                    int expectedChannel = (expectedColor >> (k*8)) & 0xff;
                    int actualChannel = (actualColor >> (k*8)) & 0xff;
                    REPORTER_ASSERT(reporter, abs(expectedChannel - actualChannel) <= tolerance);
                }
            }
        }
    }
}

SkTextBlobCacheDiffCanvas::Settings MakeSettings(GrContext* context) {
    SkTextBlobCacheDiffCanvas::Settings settings;
    settings.fContextSupportsDistanceFieldText = context->supportsDistanceFieldText();
    settings.fMaxTextureSize = context->maxTextureSize();
    settings.fMaxTextureBytes = GrContextOptions().fGlyphCacheTextureMaximumBytes;
    return settings;
}

sk_sp<SkSurface> MakeSurface(int width, int height, GrContext* context) {
    const SkImageInfo info =
            SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
    return SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info);
}

const SkSurfaceProps FindSurfaceProps(GrContext* context) {
    auto surface = MakeSurface(1, 1, context);
    return surface->props();
}

SkBitmap RasterBlob(sk_sp<SkTextBlob> blob, int width, int height, const SkPaint& paint,
                    GrContext* context, const SkMatrix* matrix = nullptr,
                    SkScalar x = 0) {
    auto surface = MakeSurface(width, height, context);
    if (matrix) surface->getCanvas()->concat(*matrix);
    surface->getCanvas()->drawTextBlob(blob.get(), x, 0, paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(width, height);
    surface->readPixels(bitmap, 0, 0);
    return bitmap;
}

DEF_TEST(SkRemoteGlyphCache_TypefaceSerialization, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto server_tf = SkTypeface::MakeDefault();
    auto tf_data = server.serializeTypeface(server_tf.get());

    auto client_tf = client.deserializeTypeface(tf_data->data(), tf_data->size());
    REPORTER_ASSERT(reporter, client_tf);
    REPORTER_ASSERT(reporter, static_cast<SkTypefaceProxy*>(client_tf.get())->remoteTypefaceID() ==
                                      server_tf->uniqueID());

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_StrikeSerialization, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    const SkPaint paint;

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    auto props = FindSurfaceProps(ctxInfo.grContext());
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server,
                                                MakeSettings(ctxInfo.grContext()));
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, ctxInfo.grContext());
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext());
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_ReleaseTypeFace, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTf = SkTestEmptyTypeface::Make();
    auto serverTfData = server.serializeTypeface(serverTf.get());
    REPORTER_ASSERT(reporter, serverTf->unique());

    {
        const SkPaint paint;
        int glyphCount = 10;
        auto serverBlob = buildTextBlob(serverTf, glyphCount);
        const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
        SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server,
                                                    MakeSettings(ctxInfo.grContext()));
        cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
        REPORTER_ASSERT(reporter, !serverTf->unique());

        std::vector<uint8_t> serverStrikeData;
        server.writeStrikeData(&serverStrikeData);
    }
    REPORTER_ASSERT(reporter, serverTf->unique());

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikeLockingServer, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    server.serializeTypeface(serverTf.get());
    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server);
    SkPaint paint;
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    // The strike from the blob should be locked after it has been drawn on the canvas.
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 1u);
    REPORTER_ASSERT(reporter, discardableManager->lockedHandles().count() == 1u);

    // Write the strike data and unlock everything. Re-analyzing the blob should lock the handle
    // again.
    std::vector<uint8_t> fontData;
    server.writeStrikeData(&fontData);
    discardableManager->unlockAll();
    REPORTER_ASSERT(reporter, discardableManager->lockedHandles().count() == 0u);

    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 1u);
    REPORTER_ASSERT(reporter, discardableManager->lockedHandles().count() == 1u);

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikeDeletionServer, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    server.serializeTypeface(serverTf.get());
    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server);
    SkPaint paint;
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 1u);

    // Write the strike data and delete all the handles. Re-analyzing the blob should create new
    // handles.
    std::vector<uint8_t> fontData;
    server.writeStrikeData(&fontData);
    discardableManager->unlockAndDeleteAll();
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 2u);

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikePinningClient, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server);
    SkPaint paint;
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto* clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size()).get();

    // The cache remains alive until it is pinned in the discardable manager.
    SkGraphics::PurgeFontCache();
    REPORTER_ASSERT(reporter, !clientTf->unique());

    // Once the strike is unpinned and purged, SkStrikeClient should be the only owner of the
    // clientTf.
    discardableManager->unlockAndDeleteAll();
    SkGraphics::PurgeFontCache();
    REPORTER_ASSERT(reporter, clientTf->unique());

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_ClientMemoryAccounting, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server);
    SkPaint paint;
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    SkStrikeCache::ValidateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_PurgesServerEntries, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    server.setMaxEntriesInDescriptorMapForTesting(1u);
    SkStrikeClient client(discardableManager, false);

    {
        auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
        int glyphCount = 10;
        auto serverBlob = buildTextBlob(serverTf, glyphCount);

        const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
        SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server);
        SkPaint paint;
        REPORTER_ASSERT(reporter, server.remoteGlyphStateMapSizeForTesting() == 0u);
        cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
        REPORTER_ASSERT(reporter, server.remoteGlyphStateMapSizeForTesting() == 1u);
    }

    // Serialize to release the lock from the strike server and delete all current
    // handles.
    std::vector<uint8_t> fontData;
    server.writeStrikeData(&fontData);
    discardableManager->unlockAndDeleteAll();

    // Use a different typeface. Creating a new strike should evict the previous
    // one.
    {
        auto serverTf = SkTypeface::MakeFromName("Georgia", SkFontStyle());
        int glyphCount = 10;
        auto serverBlob = buildTextBlob(serverTf, glyphCount);

        const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
        SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server);
        SkPaint paint;
        REPORTER_ASSERT(reporter, server.remoteGlyphStateMapSizeForTesting() == 1u);
        cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
        REPORTER_ASSERT(reporter, server.remoteGlyphStateMapSizeForTesting() == 1u);
    }

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsPath, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    REPORTER_ASSERT(reporter, SkDraw::ShouldDrawTextAsPaths(paint, SkMatrix::I()));

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    auto props = FindSurfaceProps(ctxInfo.grContext());
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server,
                                                MakeSettings(ctxInfo.grContext()));
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, ctxInfo.grContext());
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext());
    compare_blobs(expected, actual, reporter, 1);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
    SkStrikeCache::ValidateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

sk_sp<SkTextBlob> make_blob_causing_fallback(
        sk_sp<SkTypeface> targetTf, const SkTypeface* glyphTf, skiatest::Reporter* reporter) {
    SkFont font;
    font.setSubpixel(true);
    font.setSize(96);
    font.setHinting(kNormal_SkFontHinting);
    font.setTypeface(targetTf);

    REPORTER_ASSERT(reporter, !SkDraw::ShouldDrawTextAsPaths(font, SkPaint(), SkMatrix::I()));

    char s[] = "Skia";
    int runSize = strlen(s);

    SkTextBlobBuilder builder;
    SkRect bounds = SkRect::MakeIWH(100, 100);
    const auto& runBuffer = builder.allocRunPosH(font, runSize, 10, &bounds);
    SkASSERT(runBuffer.utf8text == nullptr);
    SkASSERT(runBuffer.clusters == nullptr);

    glyphTf->charsToGlyphs(s, SkTypeface::kUTF8_Encoding, runBuffer.glyphs, runSize);

    SkRect glyphBounds;
    font.getWidths(runBuffer.glyphs, 1, nullptr, &glyphBounds);

    REPORTER_ASSERT(reporter, glyphBounds.width() > SkGlyphCacheCommon::kSkSideTooBigForAtlas);

    for (int i = 0; i < runSize; i++) {
        runBuffer.pos[i] = i * 10;
    }

    return builder.make();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsMaskWithPathFallback,
        reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    SkPaint paint;

    auto serverTf = MakeResourceAsTypeface("fonts/HangingS.ttf");
    // TODO: when the cq bots can handle this font remove the check.
    if (serverTf == nullptr) {
        return;
    }
    auto serverTfData = server.serializeTypeface(serverTf.get());

    auto serverBlob = make_blob_causing_fallback(serverTf, serverTf.get(), reporter);

    auto props = FindSurfaceProps(ctxInfo.grContext());
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server,
                                                MakeSettings(ctxInfo.grContext()));
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));

    auto clientBlob = make_blob_causing_fallback(clientTf, serverTf.get(), reporter);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, ctxInfo.grContext());
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext());
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
    SkStrikeCache::ValidateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextXY, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setSubpixelText(true);
    paint.setLCDRenderText(true);

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    auto props = FindSurfaceProps(ctxInfo.grContext());
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server,
                                                MakeSettings(ctxInfo.grContext()));
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0.5, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, ctxInfo.grContext(), nullptr, 0.5);
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext(), nullptr, 0.5);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
    SkStrikeCache::ValidateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsDFT, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    SkPaint paint;
    SkFont font;

    // A perspective transform forces fallback to dft.
    SkMatrix matrix = SkMatrix::I();
    matrix[SkMatrix::kMPersp0] = 0.5f;
    REPORTER_ASSERT(reporter, matrix.hasPerspective());
    SkSurfaceProps surfaceProps(0, kUnknown_SkPixelGeometry);
    GrTextContext::Options options;
    GrTextContext::SanitizeOptions(&options);
    REPORTER_ASSERT(reporter, GrTextContext::CanDrawAsDistanceFields(
                                      paint, font, matrix, surfaceProps, true, options));

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server,
                                                MakeSettings(ctxInfo.grContext()));
    cache_diff_canvas.concat(matrix);
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, ctxInfo.grContext(), &matrix);
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext(), &matrix);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
    SkStrikeCache::ValidateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_CacheMissReporting, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto tfData = server.serializeTypeface(serverTf.get());
    auto clientTf = client.deserializeTypeface(tfData->data(), tfData->size());
    REPORTER_ASSERT(reporter, clientTf);
    int glyphCount = 10;
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    // Raster the client-side blob without the glyph data, we should get cache miss notifications.
    SkPaint paint;
    SkMatrix matrix = SkMatrix::I();
    RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext(), &matrix);
    REPORTER_ASSERT(reporter,
                    discardableManager->cacheMissCount(SkStrikeClient::kFontMetrics) == 1);
    REPORTER_ASSERT(reporter,
                    discardableManager->cacheMissCount(SkStrikeClient::kGlyphMetrics) == 10);

    // There shouldn't be any image or path requests, since we mark the glyph as empty on a cache
    // miss.
    REPORTER_ASSERT(reporter, discardableManager->cacheMissCount(SkStrikeClient::kGlyphImage) == 0);
    REPORTER_ASSERT(reporter, discardableManager->cacheMissCount(SkStrikeClient::kGlyphPath) == 0);

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_SearchOfDesperation, reporter) {
    // Build proxy typeface on the client for initializing the cache.
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto tfData = server.serializeTypeface(serverTf.get());
    auto clientTf = client.deserializeTypeface(tfData->data(), tfData->size());
    REPORTER_ASSERT(reporter, clientTf);

    SkFont font;
    font.setTypeface(clientTf);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);

    auto lostGlyphID = SkPackedGlyphID(1, SK_FixedHalf, SK_FixedHalf);
    const uint8_t glyphImage[] = {0xFF, 0xFF};

    SkStrikeCache strikeCache;

    // Build a fallback cache.
    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        SkScalerContext::MakeRecAndEffects(
                font, paint, SkSurfacePropsCopyOrDefault(nullptr), flags,
                SkMatrix::I(), &rec, &effects, false);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto fallbackCache = strikeCache.findOrCreateStrikeExclusive(*desc, effects, *clientTf);
        auto glyph = fallbackCache->getRawGlyphByID(lostGlyphID);
        glyph->fMaskFormat = SkMask::kA8_Format;
        glyph->fHeight = 1;
        glyph->fWidth = 2;
        fallbackCache->initializeImage(glyphImage, glyph->computeImageSize(), glyph);
        glyph->fImage = (void *)glyphImage;
    }

    // Make sure we can find the fall back cache.
    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        SkScalerContext::MakeRecAndEffects(
                font, paint, SkSurfacePropsCopyOrDefault(nullptr), flags,
                SkMatrix::I(), &rec, &effects, false);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);
        auto testCache = strikeCache.findStrikeExclusive(*desc);
        REPORTER_ASSERT(reporter, !(testCache == nullptr));
    }

    // Create the target cache.
    SkExclusiveStrikePtr testCache;
    SkAutoDescriptor ad;
    SkScalerContextRec rec;
    SkScalerContextEffects effects;
    SkScalerContextFlags flags = SkScalerContextFlags::kNone;
    SkScalerContext::MakeRecAndEffects(
            font, paint, SkSurfacePropsCopyOrDefault(nullptr), flags,
            SkMatrix::I(),
            &rec, &effects, false);
    auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);
    testCache = strikeCache.findStrikeExclusive(*desc);
    REPORTER_ASSERT(reporter, testCache == nullptr);
    testCache = strikeCache.createStrikeExclusive(*desc,
                                                     clientTf->createScalerContext(effects, desc));
    auto scalerProxy = static_cast<SkScalerContextProxy*>(testCache->getScalerContext());
    scalerProxy->initCache(testCache.get(), &strikeCache);

    // Look for the lost glyph.
    {
        const auto& lostGlyph = testCache->getGlyphIDMetrics(
                lostGlyphID.code(), lostGlyphID.getSubXFixed(), lostGlyphID.getSubYFixed());
        testCache->findImage(lostGlyph);

        REPORTER_ASSERT(reporter, lostGlyph.fHeight == 1);
        REPORTER_ASSERT(reporter, lostGlyph.fWidth == 2);
        REPORTER_ASSERT(reporter, lostGlyph.fMaskFormat == SkMask::kA8_Format);
        REPORTER_ASSERT(reporter, memcmp(lostGlyph.fImage, glyphImage, sizeof(glyphImage)) == 0);
    }

    // Look for the lost glyph with a different sub-pix position.
    {
        const auto& lostGlyph =
                testCache->getGlyphIDMetrics(lostGlyphID.code(), SK_FixedQuarter, SK_FixedQuarter);
        testCache->findImage(lostGlyph);

        REPORTER_ASSERT(reporter, lostGlyph.fHeight == 1);
        REPORTER_ASSERT(reporter, lostGlyph.fWidth == 2);
        REPORTER_ASSERT(reporter, lostGlyph.fMaskFormat == SkMask::kA8_Format);
        REPORTER_ASSERT(reporter, memcmp(lostGlyph.fImage, glyphImage, sizeof(glyphImage)) == 0);
    }

    for (uint32_t i = 0; i <= SkStrikeClient::CacheMissType::kLast; ++i) {
        if (i == SkStrikeClient::CacheMissType::kGlyphMetricsFallback ||
            i == SkStrikeClient::CacheMissType::kFontMetrics) {
            REPORTER_ASSERT(reporter, discardableManager->cacheMissCount(i) == 2);
        } else {
            REPORTER_ASSERT(reporter, discardableManager->cacheMissCount(i) == 0);
        }
    }
    strikeCache.validateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_ReWriteGlyph, reporter) {
    // Build proxy typeface on the client for initializing the cache.
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto tfData = server.serializeTypeface(serverTf.get());
    auto clientTf = client.deserializeTypeface(tfData->data(), tfData->size());
    REPORTER_ASSERT(reporter, clientTf);

    SkFont font;
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);

    auto lostGlyphID = SkPackedGlyphID(1, SK_FixedHalf, SK_FixedHalf);
    const uint8_t glyphImage[] = {0xFF, 0xFF};
    uint32_t realMask;
    uint32_t fakeMask;

    SkStrikeCache strikeCache;

    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        paint.setTypeface(serverTf);
        SkScalerContext::MakeRecAndEffects(
                font, paint, SkSurfacePropsCopyOrDefault(nullptr), flags,
                SkMatrix::I(), &rec, &effects, false);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto context = serverTf->createScalerContext(effects, desc, false);
        SkGlyph glyph;
        glyph.initWithGlyphID(lostGlyphID);
        context->getMetrics(&glyph);
        realMask = glyph.fMaskFormat;
        REPORTER_ASSERT(reporter, realMask != MASK_FORMAT_UNKNOWN);
    }

    // Build a fallback cache.
    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        paint.setTypeface(clientTf);
        SkScalerContext::MakeRecAndEffects(
                font, paint, SkSurfacePropsCopyOrDefault(nullptr), flags,
                SkMatrix::I(), &rec, &effects, false);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto fallbackCache = strikeCache.findOrCreateStrikeExclusive(*desc, effects, *clientTf);
        auto glyph = fallbackCache->getRawGlyphByID(lostGlyphID);
        fakeMask = (realMask == SkMask::kA8_Format) ? SkMask::kBW_Format : SkMask::kA8_Format;
        glyph->fMaskFormat = fakeMask;
        glyph->fHeight = 1;
        glyph->fWidth = 2;
        fallbackCache->initializeImage(glyphImage, glyph->computeImageSize(), glyph);
    }

    // Send over the real glyph and make sure the client cache stays intact.
    {
        SkAutoDescriptor ad;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        paint.setTypeface(serverTf);
        auto* cacheState = server.getOrCreateCache(
                paint, SkSurfacePropsCopyOrDefault(nullptr),
                SkMatrix::I(), flags, &effects);
        cacheState->addGlyph(lostGlyphID, false);

        std::vector<uint8_t> serverStrikeData;
        server.writeStrikeData(&serverStrikeData);
        REPORTER_ASSERT(reporter,
                        client.readStrikeData(
                                serverStrikeData.data(),
                                serverStrikeData.size()));
    }

    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        paint.setTypeface(clientTf);
        SkScalerContext::MakeRecAndEffects(
                font, paint, SkSurfaceProps(0, kUnknown_SkPixelGeometry), flags,
                SkMatrix::I(), &rec, &effects, false);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto fallbackCache = strikeCache.findStrikeExclusive(*desc);
        REPORTER_ASSERT(reporter, fallbackCache.get() != nullptr);
        auto glyph = fallbackCache->getRawGlyphByID(lostGlyphID);
        REPORTER_ASSERT(reporter, glyph->fMaskFormat == fakeMask);

        // Try overriding the image, it should stay the same.
        REPORTER_ASSERT(reporter,
                        memcmp(glyph->fImage, glyphImage, glyph->computeImageSize()) == 0);
        const uint8_t newGlyphImage[] = {0, 0};
        fallbackCache->initializeImage(newGlyphImage, glyph->computeImageSize(), glyph);
        REPORTER_ASSERT(reporter,
                        memcmp(glyph->fImage, glyphImage, glyph->computeImageSize()) == 0);
    }

    strikeCache.validateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}
