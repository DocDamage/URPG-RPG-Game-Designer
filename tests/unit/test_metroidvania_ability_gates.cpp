#include "engine/core/metroidvania/ability_gate_system.h"
#include "editor/metroidvania/ability_gate_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <fstream>

namespace {

nlohmann::json loadAbilityGateFixture() {
    std::ifstream stream(std::string(URPG_SOURCE_DIR) + "/content/fixtures/metroidvania_ability_gates_fixture.json");
    REQUIRE(stream.good());
    return nlohmann::json::parse(stream);
}

} // namespace

TEST_CASE("Metroidvania ability gates preview reachable regions and unlock rewards", "[metroidvania][ability_gates]") {
    const auto document = urpg::metroidvania::AbilityGateDocument::fromJson(loadAbilityGateFixture());

    REQUIRE(document.validate({"dash", "wall_jump"}).empty());

    const auto preview = document.preview("entry", {});
    REQUIRE(preview.reachable_regions.contains("entry"));
    REQUIRE(preview.reachable_regions.contains("dash_shrine"));
    REQUIRE(preview.reachable_regions.contains("upper_cavern"));
    REQUIRE(preview.reachable_regions.contains("boss_door"));
    REQUIRE(preview.unlocked_abilities.contains("dash"));
    REQUIRE(preview.unlocked_abilities.contains("wall_jump"));
    REQUIRE(document.canReach("entry", "boss_door", {}));

    urpg::editor::metroidvania::AbilityGatePanel panel;
    panel.loadDocument(document);
    panel.setPreviewContext("entry", {});
    panel.render();

    REQUIRE(panel.snapshot().reachable_count == 4);
    REQUIRE(panel.saveProjectData() == document.toJson());
    REQUIRE(urpg::metroidvania::abilityGatePreviewToJson(panel.preview())["reachable_regions"].size() == 4);
}

TEST_CASE("Metroidvania ability gates report broken links and missing starts", "[metroidvania][ability_gates]") {
    urpg::metroidvania::AbilityGateDocument document;
    document.id = "bad";
    document.regions.push_back({"entry", "", {"unknown_ability"}});
    document.links.push_back({"bad_link", "entry", "missing", {"dash"}, false});

    const auto diagnostics = document.validate({"wall_jump"});
    REQUIRE(diagnostics.size() >= 3);

    const auto preview = document.preview("missing_start", {});
    REQUIRE(preview.reachable_regions.empty());
    REQUIRE_FALSE(preview.diagnostics.empty());
}
