#include <catch2/catch_test_macros.hpp>

#include "editor/assets/editor_thumbnail_cache.h"

namespace {
urpg::editor::EditorThumbnailRequest requestFor(const char* path, uint64_t size = 10, int64_t modified = 20) {
    urpg::editor::EditorThumbnailRequest request;
    request.sourcePath = path;
    request.sizeBytes = size;
    request.modifiedTimeNs = modified;
    request.requestedWidth = 8;
    request.requestedHeight = 8;
    return request;
}

urpg::editor::EditorThumbnailDecodedImage readyImage(uint32_t width = 8, uint32_t height = 8) {
    urpg::editor::EditorThumbnailDecodedImage image;
    image.success = true;
    image.diagnosticCode = "thumbnail_ready";
    image.width = width;
    image.height = height;
    image.rgba.assign(static_cast<size_t>(width) * height * 4u, 255);
    return image;
}
} // namespace

TEST_CASE("EditorThumbnailCache only decodes visible requests and exposes fallbacks", "[thumbnail][assets]") {
    int decodeCount = 0;
    urpg::editor::EditorThumbnailCache::Configuration config;
    config.asyncDecode = false;
    urpg::editor::EditorThumbnailCache cache(config, [&](const auto& request) {
        ++decodeCount;
        return request.hashPending ? urpg::editor::EditorThumbnailDecodedImage{false, "thumbnail_hash_pending", 0, 0, 1, 0, {}}
                                   : readyImage();
    }, [](const auto&) { return std::shared_ptr<urpg::Texture>{}; });

    auto first = requestFor("first.png");
    auto pending = requestFor("pending.png");
    pending.hashPending = true;
    cache.setVisibleRequests({first, pending});
    cache.pumpUploads();

    REQUIRE(decodeCount == 2);
    REQUIRE(cache.snapshotFor(first).state == urpg::editor::EditorThumbnailState::Fallback);
    REQUIRE(cache.snapshotFor(first).diagnosticCode == "thumbnail_texture_upload_failed");
    REQUIRE(cache.snapshotFor(pending).diagnosticCode == "thumbnail_hash_pending");
}

TEST_CASE("EditorThumbnailCache bounds resident previews and keys source revisions", "[thumbnail][assets]") {
    urpg::editor::EditorThumbnailCache::Configuration config;
    config.asyncDecode = false;
    config.memoryBudgetBytes = 300;
    urpg::editor::EditorThumbnailCache cache(config, [](const auto&) { return readyImage(8, 8); },
                                             [](const auto&) { return std::make_shared<urpg::Texture>(); });
    const auto first = requestFor("one.png", 100, 1);
    const auto revised = requestFor("one.png", 101, 2);
    const auto second = requestFor("two.png", 100, 1);

    REQUIRE(urpg::editor::EditorThumbnailCache::cacheKey(first) !=
            urpg::editor::EditorThumbnailCache::cacheKey(revised));
    cache.setVisibleRequests({first, second});
    cache.pumpUploads();

    REQUIRE(cache.residentBytes() <= config.memoryBudgetBytes);
    REQUIRE(cache.residentCount() <= 2);
    cache.shutdown();
    REQUIRE(cache.residentBytes() == 0);
}
