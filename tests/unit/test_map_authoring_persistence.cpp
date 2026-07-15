#include <catch2/catch_test_macros.hpp>

#include "editor/spatial/map_authoring_persistence.h"

#include <chrono>
#include <filesystem>
#include <fstream>

TEST_CASE("Map authoring persistence publishes paired documents", "[editor][spatial][MapAuthoring]") {
    const auto root = std::filesystem::temp_directory_path() /
                      ("urpg_map_persistence_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto grid = root / "maps" / "starter.grid.json";
    const auto p2d = root / "maps" / "starter.p2d.json";
    const auto result = urpg::editor::publishMapAuthoringDocuments({{grid, "{\"grid\":true}\n"}, {p2d, "{\"p2d\":true}\n"}});

    REQUIRE(result.success);
    REQUIRE(result.code == "map_documents_published");
    std::ifstream gridInput(grid);
    std::ifstream p2dInput(p2d);
    REQUIRE(std::string{std::istreambuf_iterator<char>(gridInput), {}} == "{\"grid\":true}\n");
    REQUIRE(std::string{std::istreambuf_iterator<char>(p2dInput), {}} == "{\"p2d\":true}\n");
    gridInput.close();
    p2dInput.close();
    std::filesystem::remove_all(root);
}

TEST_CASE("Map authoring persistence rejects duplicate targets before writing", "[editor][spatial][MapAuthoring]") {
    const auto target = std::filesystem::temp_directory_path() / "urpg_map_persistence_duplicate.json";
    const auto result = urpg::editor::publishMapAuthoringDocuments({{target, "first"}, {target, "second"}});
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "map_save_invalid_targets");
}
