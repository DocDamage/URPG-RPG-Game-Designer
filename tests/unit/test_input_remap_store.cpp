#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "engine/core/input/input_remap_store.h"

using namespace urpg::input;
using Catch::Matchers::ContainsSubstring;
using nlohmann::json;

TEST_CASE("InputRemapStore: Default mappings are populated on construction", "[input][remap]") {
    InputRemapStore store;

    REQUIRE(store.getMapping(38) == InputAction::MoveUp);
    REQUIRE(store.getMapping(40) == InputAction::MoveDown);
    REQUIRE(store.getMapping(37) == InputAction::MoveLeft);
    REQUIRE(store.getMapping(39) == InputAction::MoveRight);
    REQUIRE(store.getMapping(90) == InputAction::Confirm);
    REQUIRE(store.getMapping(13) == InputAction::Confirm);
    REQUIRE(store.getMapping(88) == InputAction::Cancel);
    REQUIRE(store.getMapping(27) == InputAction::Cancel);
    REQUIRE(store.getMapping(67) == InputAction::Menu);
}

TEST_CASE("InputRemapStore: setMapping overrides an existing mapping", "[input][remap]") {
    InputRemapStore store;

    REQUIRE(store.getMapping(38) == InputAction::MoveUp);
    store.setMapping(38, InputAction::Menu);
    REQUIRE(store.getMapping(38) == InputAction::Menu);
    REQUIRE(store.hasUnsavedChanges());
}

TEST_CASE("InputRemapStore: removeMapping deletes a binding", "[input][remap]") {
    InputRemapStore store;

    REQUIRE(store.getMapping(38).has_value());
    store.removeMapping(38);
    REQUIRE_FALSE(store.getMapping(38).has_value());
    REQUIRE(store.hasUnsavedChanges());
}

TEST_CASE("InputRemapStore: resetToDefaults restores defaults after clear", "[input][remap]") {
    InputRemapStore store;

    store.clear();
    REQUIRE(store.getAllMappings().empty());
    REQUIRE(store.hasUnsavedChanges());

    store.resetToDefaults();
    REQUIRE(store.getMapping(38) == InputAction::MoveUp);
    REQUIRE(store.getMapping(40) == InputAction::MoveDown);
    REQUIRE_FALSE(store.hasUnsavedChanges());
}

TEST_CASE("InputRemapStore: saveToJson / loadFromJson round-trip preserves custom mappings", "[input][remap]") {
    InputRemapStore store;
    store.clear();
    store.setMapping(100, InputAction::Confirm);
    store.setMapping(101, InputAction::Cancel);

    nlohmann::json j = store.saveToJson();

    REQUIRE(j["version"] == "1.0.0");
    REQUIRE(j["bindings"].is_array());
    REQUIRE(j["bindings"].size() == 2);

    InputRemapStore other;
    other.loadFromJson(j);

    REQUIRE(other.getMapping(100) == InputAction::Confirm);
    REQUIRE(other.getMapping(101) == InputAction::Cancel);
    REQUIRE_FALSE(other.hasUnsavedChanges());
}

TEST_CASE("InputRemapStore: loadFromJson throws on missing version", "[input][remap]") {
    InputRemapStore store;
    nlohmann::json j;
    j["bindings"] = nlohmann::json::array();

    REQUIRE_THROWS_MATCHES(
        store.loadFromJson(j),
        std::invalid_argument,
        Catch::Matchers::MessageMatches(ContainsSubstring("version"))
    );
}

TEST_CASE("InputRemapStore: hasUnsavedChanges is true after mutation and false after load/reset", "[input][remap]") {
    InputRemapStore store;
    REQUIRE_FALSE(store.hasUnsavedChanges());

    store.setMapping(200, InputAction::Debug);
    REQUIRE(store.hasUnsavedChanges());

    nlohmann::json j = store.saveToJson();
    InputRemapStore reloaded;
    reloaded.loadFromJson(j);
    REQUIRE_FALSE(reloaded.hasUnsavedChanges());

    reloaded.setMapping(201, InputAction::Menu);
    REQUIRE(reloaded.hasUnsavedChanges());

    reloaded.resetToDefaults();
    REQUIRE_FALSE(reloaded.hasUnsavedChanges());
}

TEST_CASE("InputRemapStore governance script validates artifacts", "[input][remap][project_audit_cli]") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    const auto scriptPath = repoRoot / "tools" / "ci" / "check_input_governance.ps1";
    const auto uniqueSuffix =
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto outputPath =
        std::filesystem::temp_directory_path() / ("urpg_input_gov_out_" + uniqueSuffix + ".json");

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
        "\" > \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.is_open());

    std::string jsonStr((std::istreambuf_iterator<char>(resultFile)),
                         std::istreambuf_iterator<char>());
    resultFile.close();

    const auto result = json::parse(jsonStr);
    REQUIRE(result["passed"].get<bool>() == true);
    REQUIRE(result["errors"].is_array());
    REQUIRE(result["errors"].empty());
}
