#include "editor/assets/editor_thumbnail_cache.h"
#include "editor/ui/editor_theme.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <memory>
#include <vector>

TEST_CASE("editor creator workflow bounds visible thumbnail work and supports DPI tokens", "[editor][creator workflow][perf]") {
    urpg::editor::EditorThumbnailCache::Configuration configuration;
    configuration.asyncDecode = false;
    configuration.memoryBudgetBytes = 16u * 1024u;
    urpg::editor::EditorThumbnailCache cache(
        configuration,
        [](const urpg::editor::EditorThumbnailRequest& request) {
            urpg::editor::EditorThumbnailDecodedImage image;
            image.success = true;
            image.diagnosticCode = "thumbnail_ready";
            image.width = request.requestedWidth;
            image.height = request.requestedHeight;
            image.rgba.assign(static_cast<size_t>(image.width) * image.height * 4u, 255);
            return image;
        },
        [](const urpg::editor::EditorThumbnailDecodedImage&) { return std::make_shared<urpg::Texture>(); });

    std::vector<urpg::editor::EditorThumbnailRequest> visible;
    for (uint32_t index = 0; index < 32; ++index) {
        visible.push_back({"visible_" + std::to_string(index) + ".png", index, index, 24, 24});
    }
    const auto started = std::chrono::steady_clock::now();
    cache.setVisibleRequests(visible);
    cache.pumpUploads();
    const auto elapsed = std::chrono::steady_clock::now() - started;

    INFO("visible_workflow_ms=" << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
    REQUIRE(cache.residentBytes() <= configuration.memoryBudgetBytes);
    REQUIRE(cache.residentCount() <= visible.size());
    REQUIRE(elapsed < std::chrono::seconds(2));
    REQUIRE(urpg::editor::ui::scaledEditorTheme(urpg::editor::ui::EditorUiScale::Percent200).controlHeight ==
            urpg::editor::ui::defaultEditorTheme().controlHeight * 2.0f);
}
