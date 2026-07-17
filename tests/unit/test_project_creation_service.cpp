#include "engine/core/diagnostics/startup_diagnostics.h"
#include "engine/core/project/project_creation_service.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace {

std::filesystem::path uniqueRoot() {
    return std::filesystem::temp_directory_path() /
           ("urpg_project_creation_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
}

} // namespace

TEST_CASE("ProjectCreationService atomically creates a runtime-valid starter project", "[project][project creation][preflight]") {
    const auto root = uniqueRoot();
    const auto destination = root / "CreatorDemo";
    urpg::project::ProjectCreationService service;
    urpg::project::ProjectCreationRequest request;
    request.project_id = "creator_demo";
    request.project_name = "Creator Demo";
    request.destination = destination;
    const auto result = service.createProject(request);

    REQUIRE(result.success);
    REQUIRE(result.code == "project_created");
    REQUIRE(std::filesystem::is_regular_file(destination / "project.json"));
    REQUIRE(std::filesystem::is_regular_file(destination / "content" / "maps" / "map_intro.json"));
    REQUIRE_FALSE(urpg::diagnostics::validateRuntimeProjectPreflight("runtime", destination, true).has_value());

    std::filesystem::remove_all(root);
}

TEST_CASE("ProjectCreationService leaves no partial project after validation failure", "[project][project creation]") {
    const auto root = uniqueRoot();
    const auto destination = root / "BadProject";
    urpg::project::ProjectCreationService service;
    urpg::project::ProjectCreationRequest request;
    request.template_id = "unknown";
    request.project_id = "bad_project";
    request.project_name = "Bad Project";
    request.destination = destination;
    const auto result = service.createProject(request);

    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "project_template_invalid");
    REQUIRE_FALSE(std::filesystem::exists(destination));
    std::filesystem::remove_all(root);
}

TEST_CASE("ProjectCreationService rejects an unsafe starter-map ID before creating a staging directory",
          "[project][project creation][validation]") {
    const auto root = uniqueRoot();
    const auto destination = root / "UnsafeMapProject";
    urpg::project::ProjectCreationRequest request;
    request.project_id = "unsafe_map_project";
    request.project_name = "Unsafe Map Project";
    request.destination = destination;
    request.starter_map = "../outside";

    const auto result = urpg::project::ProjectCreationService{}.createProject(request);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "project_starter_map_invalid");
    REQUIRE_FALSE(std::filesystem::exists(destination));
    REQUIRE_FALSE(std::filesystem::exists(root));
}

TEST_CASE("ProjectCreationService keeps the external asset-library root out of project data", "[project][project creation][assets]") {
    const auto root = uniqueRoot();
    const auto destination = root / "LocalLibraryProject";
    urpg::project::ProjectCreationRequest request;
    request.project_id = "local_library_project";
    request.project_name = "Local Library Project";
    request.destination = destination;
    request.external_asset_library_root = "G:/All 2D Assets Stay Here";

    const auto result = urpg::project::ProjectCreationService{}.createProject(request);
    REQUIRE(result.success);
    std::ifstream manifestInput(destination / "project.json", std::ios::binary);
    const auto manifest = nlohmann::json::parse(manifestInput);
    REQUIRE_FALSE(manifest["creator"].contains("external_asset_library_root"));
    manifestInput.close();
    std::filesystem::remove_all(root);
}

TEST_CASE("ProjectCreationService can create the draft creator vertical-slice seed through the native wizard contract",
          "[project][project creation][creator vertical slice]") {
    const auto root = uniqueRoot();
    const auto destination = root / "LanternSeed";
    urpg::project::ProjectCreationRequest request;
    request.project_id = "lantern_seed";
    request.project_name = "Lantern Seed";
    request.destination = destination;
    request.include_creator_vertical_slice_seed = true;
    request.starter_map = "willow_village";

    const auto result = urpg::project::ProjectCreationService{}.createProject(request);
    REQUIRE(result.success);
    REQUIRE(std::filesystem::is_regular_file(destination / "content" / "maps" / "moonwell_shrine.json"));
    REQUIRE(std::filesystem::is_regular_file(destination / "content" / "maps" / "willow_village.p2d.json"));
    REQUIRE(std::filesystem::is_regular_file(destination / "content" / "maps" / "moonwell_shrine.p2d.json"));
    REQUIRE(std::filesystem::is_regular_file(destination / "content" / "characters" / "willow_hero.json"));
    REQUIRE(std::filesystem::is_regular_file(destination / "content" / "quests" / "restore_moonwell_lantern.json"));
    REQUIRE(std::filesystem::is_regular_file(destination / "content" / "vendors" / "rowan_tonics.json"));
    REQUIRE(std::filesystem::is_regular_file(destination / "content" / "abilities" / "willow_strike.json"));
    std::ifstream seedInput(destination / "content" / "creator_vertical_slice_seed.json", std::ios::binary);
    const auto seed = nlohmann::json::parse(seedInput);
    REQUIRE(seed["schema"] == "urpg.creator_vertical_slice_seed.v1");
    REQUIRE(seed["status"] == "draft");
    REQUIRE(seed["seed_revision"] == "native_creator_seed.v2");
    REQUIRE(seed["maps"] == nlohmann::json::array({"willow_village", "moonwell_shrine"}));
    seedInput.close();
    std::ifstream villageDraftInput(destination / "content" / "maps" / "willow_village.p2d.json", std::ios::binary);
    const auto villageDraft = nlohmann::json::parse(villageDraftInput);
    REQUIRE(villageDraft["document_kind"] == "urpg.perspective_2d.map");
    REQUIRE(villageDraft["layers"].size() == 2);
    REQUIRE(villageDraft["tiles"].size() == 1);
    REQUIRE(villageDraft["events"].size() == 3);
    REQUIRE(villageDraft["events"][0]["event_id"] == "elder_mira_intro");
    REQUIRE(villageDraft["events"][0]["pages"][0]["commands"][1]["code"] == "show_choice");
    villageDraftInput.close();
    std::filesystem::remove_all(root);
}

TEST_CASE("ProjectCreationService rejects a vertical-slice seed with a mismatched starter map",
          "[project][project creation][creator vertical slice][validation]") {
    const auto root = uniqueRoot();
    urpg::project::ProjectCreationRequest request;
    request.project_id = "invalid_lantern_seed";
    request.project_name = "Invalid Lantern Seed";
    request.destination = root / "InvalidLanternSeed";
    request.include_creator_vertical_slice_seed = true;

    const auto result = urpg::project::ProjectCreationService{}.createProject(request);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "project_vertical_slice_seed_starter_map_invalid");
    REQUIRE_FALSE(std::filesystem::exists(request.destination));
    std::filesystem::remove_all(root);
}
