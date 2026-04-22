#include "engine/core/render/raycast_renderer.h"
#include "editor/productivity/editor_utility_task.h"
#include "engine/core/presentation/presentation_migrate.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/presentation/presentation_types.h"

#include <catch2/catch_test_macros.hpp>

namespace {

class SpatialOnlyUtility final : public urpg::editor::EditorUtilityTask {
public:
    std::string getName() const override { return "Spatial Audit"; }
    ModeRequirement requiredMode() const override { return ModeRequirement::SpatialOnly; }
    void execute(const Context&) override {}
};

} // namespace

TEST_CASE("Raycast renderer stays behind explicit presentation gating", "[render][optional]") {
    urpg::render::RaycastRenderer::Camera camera{2.5f, 2.5f, -1.0f, 0.0f, 0.0f, 0.66f};
    urpg::render::RaycastRenderer::Config config;
    config.screenWidth = 8;
    config.screenHeight = 6;

    const auto isBlocking = [](int32_t x, int32_t y) {
        return x <= 0 || y <= 0 || x >= 4 || y >= 4;
    };

    SECTION("Classic 2D mode does not run the optional raycast lane") {
        config.presentationMode = urpg::presentation::PresentationMode::Classic2D;
        const auto results = urpg::render::RaycastRenderer::castFrame(camera, config, isBlocking);
        REQUIRE(results.empty());
    }

    SECTION("Spatial mode opt-in enables raycast results") {
        config.presentationMode = urpg::presentation::PresentationMode::Spatial;
        const auto results = urpg::render::RaycastRenderer::castFrame(camera, config, isBlocking);
        REQUIRE(results.size() == static_cast<size_t>(config.screenWidth));
    }

    SECTION("Spatial overlays can be adapted into a raycast-ready authored map") {
        urpg::presentation::SpatialMapOverlay overlay;
        overlay.elevation.width = 4;
        overlay.elevation.height = 4;
        overlay.elevation.levels = {
            0, 0, 0, 0,
            0, 2, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0
        };

        const auto adapter = urpg::render::RaycastRenderer::buildAuthoringAdapter(overlay);
        REQUIRE(adapter.width == 4);
        REQUIRE(adapter.height == 4);
        REQUIRE(adapter.isBlocking(1, 1));
        REQUIRE_FALSE(adapter.isBlocking(0, 0));
    }

    SECTION("Migrated legacy maps produce deterministic 2.5D blocking adapters") {
        urpg::presentation::PresentationMigrationTool::LegacyMapData legacy;
        legacy.id = "legacy_castle";
        legacy.width = 4;
        legacy.height = 4;
        legacy.tileData = {
            0, 0, 0, 0,
            0, 150, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0
        };

        const auto overlay = urpg::presentation::PresentationMigrationTool::MigrateMap(legacy);
        const auto adapter = urpg::render::RaycastRenderer::buildAuthoringAdapter(overlay);
        REQUIRE(adapter.mapId == "legacy_castle");
        REQUIRE(adapter.isBlocking(1, 1));
        REQUIRE_FALSE(adapter.isBlocking(0, 0));
    }
}

TEST_CASE("Editor utilities respect project-mode requirements", "[editor][utility]") {
    urpg::editor::EditorUtilityManager manager;
    manager.registerTask(std::make_unique<SpatialOnlyUtility>());

    urpg::editor::EditorUtilityTask::Context classicContext;
    classicContext.presentationMode = urpg::presentation::PresentationMode::Classic2D;

    urpg::editor::EditorUtilityTask::Context spatialContext;
    spatialContext.presentationMode = urpg::presentation::PresentationMode::Spatial;

    REQUIRE(manager.getRunnableTasks(classicContext).empty());
    REQUIRE(manager.getRunnableTasks(spatialContext).size() == 1);
    REQUIRE(manager.getRunnableTasks(spatialContext).front()->getName() == "Spatial Audit");
}
