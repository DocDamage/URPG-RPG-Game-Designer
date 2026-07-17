#include "editor/ui/editor_route_scale_contract.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Primary editor routes expose valid geometry contracts at every supported scale",
          "[editor][ui][scale][snapshot]") {
    const auto matrix = urpg::editor::editorRouteScaleMatrixSnapshot();
    REQUIRE(matrix["schema"] == "urpg.editor_route_scale_matrix.v1");
    REQUIRE(matrix["rows"].size() == urpg::editor::primaryEditorRoutes().size() * 5);
    for (const auto& row : matrix["rows"]) {
        const auto snapshot = urpg::editor::editorRouteScaleSnapshot(
            row["route"].get<std::string>(), row["scale"].get<float>());
        REQUIRE(urpg::editor::validateEditorRouteScaleSnapshot(snapshot).empty());
        REQUIRE(snapshot.minimum_hit_target >= 30.0F);
        REQUIRE(snapshot.resizable_constraints);
        REQUIRE_FALSE(snapshot.fixed_window_geometry);
        REQUIRE(snapshot.workspace_regions.size() == 7);
        REQUIRE(snapshot.workspace_regions.front() == "project_navigator");
        REQUIRE(snapshot.workspace_regions.back() == "playtest_controls");
    }
    REQUIRE(urpg::editor::editorRouteScaleSnapshot("startup", 0.1F).scale == 0.5F);
    REQUIRE(urpg::editor::editorRouteScaleSnapshot("startup", 9.0F).scale == 3.0F);
    REQUIRE_FALSE(urpg::editor::validateEditorRouteScaleSnapshot(
                      urpg::editor::editorRouteScaleSnapshot("unknown", 1.0F)).empty());
}
