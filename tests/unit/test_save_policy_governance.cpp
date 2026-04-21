#include <catch2/catch_test_macros.hpp>
#include "engine/core/save/save_catalog.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <string>

using namespace urpg;

namespace {

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string source_root = URPG_SOURCE_DIR;
    if (source_root.size() >= 2 && source_root.front() == '"' && source_root.back() == '"') {
        source_root = source_root.substr(1, source_root.size() - 2);
    }
    return std::filesystem::path(source_root);
#else
    return {};
#endif
}

std::filesystem::path fixturePath() {
    auto root = sourceRootFromMacro();
    if (!root.empty()) {
        return root / "content" / "fixtures" / "save_policies.json";
    }
    return std::filesystem::path("content") / "fixtures" / "save_policies.json";
}

} // namespace

TEST_CASE("Canonical save policy fixture exists and is valid JSON", "[save][policy][governance]") {
    std::ifstream file(fixturePath());
    REQUIRE(file.good());
    nlohmann::json policy;
    REQUIRE_NOTHROW(file >> policy);
    REQUIRE(policy.contains("_urpg_format_version"));
    REQUIRE(policy.contains("metadata_fields"));
    REQUIRE(policy.contains("retention"));
    REQUIRE(policy.contains("autosave"));
}

TEST_CASE("Canonical save policy fixture retention limits are non-negative", "[save][policy][governance]") {
    std::ifstream file(fixturePath());
    nlohmann::json policy;
    file >> policy;

    REQUIRE(policy["retention"]["max_autosave_slots"] >= 0);
    REQUIRE(policy["retention"]["max_quicksave_slots"] >= 0);
    REQUIRE(policy["retention"]["max_manual_slots"] >= 0);
}

TEST_CASE("Canonical save policy fixture autosave slot is non-negative", "[save][policy][governance]") {
    std::ifstream file(fixturePath());
    nlohmann::json policy;
    file >> policy;

    REQUIRE(policy["autosave"]["slot_id"] >= 0);
}

TEST_CASE("SaveSessionCoordinator can load canonical policy fixture", "[save][policy][governance]") {
    SaveCatalog catalog;
    SaveSessionCoordinator coordinator(catalog);
    REQUIRE(coordinator.loadSavePolicies(fixturePath()));

    const auto& autosave = coordinator.autosavePolicy();
    REQUIRE(autosave.enabled == true);
    REQUIRE(autosave.slot_id == 0);

    const auto& retention = coordinator.retentionPolicy();
    REQUIRE(retention.max_autosave_slots == 3);
    REQUIRE(retention.max_quicksave_slots == 5);
    REQUIRE(retention.max_manual_slots == 50);
    REQUIRE(retention.prune_excess_on_save == true);

    const auto& metadata = coordinator.metadataRegistry();
    const auto& fields = metadata.getFields();
    REQUIRE(fields.size() == 3);
    REQUIRE(fields.at("chapter").key == "chapter");
    REQUIRE(fields.at("chapter").required == true);
}
