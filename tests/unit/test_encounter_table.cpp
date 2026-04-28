#include "engine/core/balance/encounter_table.h"
#include "editor/balance/encounter_designer_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <fstream>

namespace {

nlohmann::json loadEncounterDesignerFixture() {
    std::ifstream stream(std::string(URPG_SOURCE_DIR) + "/content/fixtures/encounter_designer_fixture.json");
    REQUIRE(stream.good());
    return nlohmann::json::parse(stream);
}

} // namespace

TEST_CASE("Encounter table rejects zero-weight pools and previews deterministically", "[balance][encounter][ffs12]") {
    urpg::balance::EncounterTable invalid;
    invalid.addEncounter({"slime", "forest", 0, 1});
    invalid.addEncounter({"bat", "forest", 0, 2});

    REQUIRE(invalid.validate().front().code == "zero_weight_pool");

    urpg::balance::EncounterTable table;
    table.addEncounter({"slime", "forest", 3, 1});
    table.addEncounter({"orc", "forest", 1, 4});

    const auto first = table.preview("forest", 42, 4);
    const auto second = table.preview("forest", 42, 4);

    REQUIRE(first == second);
    REQUIRE(first.size() == 4);
}

TEST_CASE("Encounter designer previews saved encounter pools and exports runtime table", "[balance][encounter][designer]") {
    const auto document = urpg::balance::EncounterDesignerDocument::fromJson(loadEncounterDesignerFixture());

    REQUIRE(document.validate({"slime", "wolf"}, {"forest_unlocked"}).empty());

    const auto preview = document.preview("forest_day", 3, {"forest_unlocked"}, 44, 5);
    REQUIRE(preview.encounters.size() == 5);
    REQUIRE(preview.diagnostics.empty());

    const auto runtime = document.toRuntimeTable().preview("forest_day", 44, 5);
    REQUIRE(runtime.size() == 5);

    urpg::editor::balance::EncounterDesignerPanel panel;
    panel.loadDocument(document);
    panel.setPreviewContext("forest_day", 3, {"forest_unlocked"}, 44, 5);
    panel.render();

    REQUIRE(panel.snapshot().preview_count == 5);
    REQUIRE(panel.saveProjectData() == document.toJson());
    REQUIRE(urpg::balance::encounterDesignerPreviewToJson(panel.preview())["encounters"].size() == 5);
}

TEST_CASE("Encounter designer reports locked and invalid encounter authoring", "[balance][encounter][designer]") {
    urpg::balance::EncounterDesignerDocument document;
    document.id = "bad";
    document.regions.push_back({"forest_day", "", "forest", 5, 2, {"missing_flag"}});
    document.encounters.push_back({"bad_pack", "missing_enemy", "missing_region", 0, 0, 4, 1, {}, {}});

    const auto diagnostics = document.validate({"slime"}, {"forest_unlocked"});
    REQUIRE(diagnostics.size() >= 6);

    const auto preview = document.preview("forest_day", 1, {}, 1, 3);
    REQUIRE(preview.encounters.empty());
    REQUIRE_FALSE(preview.diagnostics.empty());
}
