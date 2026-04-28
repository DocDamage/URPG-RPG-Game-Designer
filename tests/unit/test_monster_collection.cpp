#include "editor/monster/monster_collection_panel.h"
#include "engine/core/monster/monster_collection.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 && sourceRoot.front() == '"' && sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

} // namespace

TEST_CASE("Monster collection captures moves stores and previews editor state", "[monster][capture]") {
    const auto fixturePath = sourceRootFromMacro() / "content" / "fixtures" / "monster_collection_fixture.json";
    std::ifstream input(fixturePath);
    REQUIRE(input.is_open());
    nlohmann::json fixture;
    input >> fixture;

    auto document = urpg::monster::MonsterCollectionDocument::fromJson(fixture);
    REQUIRE(document.validate().empty());

    urpg::monster::CaptureAttempt attempt;
    attempt.species_id = "slime";
    attempt.target_hp_percent = 20;
    attempt.ball_bonus = 30;
    attempt.seed = 0;

    const auto preview = document.previewCapture(attempt);
    REQUIRE(preview.success);
    REQUIRE(preview.chance == 85);
    REQUIRE(document.capture(attempt, "slime_001"));
    REQUIRE(document.party.size() == 1);

    document.party[0].level = 5;
    REQUIRE(document.evolve("slime_001"));
    REQUIRE(document.party[0].species_id == "big_slime");
    REQUIRE(document.moveToStorage("slime_001"));
    REQUIRE(document.party.empty());
    REQUIRE(document.storage.size() == 1);
    REQUIRE(document.moveToParty("slime_001"));

    urpg::editor::MonsterCollectionPanel panel;
    panel.bindDocument(document);
    panel.setCaptureAttempt(attempt);
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["panel"] == "monster_collection");
    REQUIRE(snapshot["document"]["party"].size() == 1);
    REQUIRE(snapshot["capture_preview"]["success"] == true);
}

TEST_CASE("Monster collection reports invalid species references", "[monster][capture]") {
    auto document = urpg::monster::MonsterCollectionDocument::fromJson(nlohmann::json{
        {"party_limit", 1},
        {"species", {{{"id", "slime"}, {"display_name", "Slime"}, {"base_capture_rate", 40}, {"evolves_to", "missing"}}}},
        {"party", {{{"instance_id", "ghost_001"}, {"species_id", "ghost"}, {"level", 1}}}},
        {"storage", nlohmann::json::array()},
    });

    const auto diagnostics = document.validate();
    REQUIRE(diagnostics.size() == 2);
    REQUIRE(diagnostics[0].code == "missing_evolution_species");
    REQUIRE(diagnostics[1].code == "missing_party_species");
}
