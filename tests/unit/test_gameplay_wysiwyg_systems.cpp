#include "engine/core/editor/editor_panel_registry.h"
#include "engine/core/gameplay/gameplay_wysiwyg_system.h"
#include "editor/gameplay/gameplay_wysiwyg_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <map>

namespace {

nlohmann::json loadFixture(const std::string& name) {
    const auto path = std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" / (name + "_fixture.json");
    std::ifstream stream(path);
    REQUIRE(stream.good());
    return nlohmann::json::parse(stream);
}

const std::map<std::string, std::string>& fixtureByFeature() {
    static const std::map<std::string, std::string> fixtures = {
        {"status_effect_designer", "status_effect_designer"},
        {"enemy_ai_behavior_tree", "enemy_ai_behavior_tree"},
        {"boss_phase_script", "boss_phase_script"},
        {"equipment_set_bonus", "equipment_set_bonus"},
        {"dungeon_room_flow", "dungeon_room_flow"},
        {"companion_banter", "companion_banter"},
        {"quest_choice_consequence", "quest_choice_consequence"},
        {"shop_economy_sim_lab", "shop_economy_sim_lab"},
        {"puzzle_mechanic_builder", "puzzle_mechanic_builder"},
        {"world_state_timeline", "world_state_timeline"},
        {"tactical_terrain_effects", "tactical_terrain_effects"},
        {"procedural_content_rules", "procedural_content_rules"},
    };
    return fixtures;
}

urpg::gameplay::GameplayWysiwygState stateForFeature(const std::string& feature) {
    urpg::gameplay::GameplayWysiwygState state;
    if (feature == "enemy_ai_behavior_tree") {
        state.flags.insert("ally_injured");
    } else if (feature == "equipment_set_bonus") {
        state.flags.insert("sun_guard_3pc");
    } else if (feature == "dungeon_room_flow") {
        state.flags.insert("mine_key");
        state.resources["keys"] = 1;
    } else if (feature == "companion_banter") {
        state.flags.insert("guide_recruited");
    } else if (feature == "shop_economy_sim_lab") {
        state.flags.insert("festival_day");
    } else if (feature == "procedural_content_rules") {
        state.flags.insert("treasure_room_placed");
    }
    return state;
}

} // namespace

TEST_CASE("Gameplay WYSIWYG systems expose runtime editor saved-data and registry hooks",
          "[gameplay][wysiwyg][systems]") {
    const auto features = urpg::gameplay::gameplayWysiwygFeatureTypes();
    REQUIRE(features.size() == 12);

    for (const auto& feature : features) {
        const auto fixture = fixtureByFeature().at(feature);
        const auto document = urpg::gameplay::GameplayWysiwygDocument::fromJson(loadFixture(fixture));
        CAPTURE(feature);

        REQUIRE(document.feature_type == feature);
        REQUIRE(document.validate().empty());
        REQUIRE_FALSE(document.visual_layers.empty());
        REQUIRE_FALSE(document.rules.empty());
        REQUIRE(urpg::editor::findEditorPanelRegistryEntry(feature) != nullptr);
        REQUIRE(urpg::editor::findEditorPanelRegistryEntry(feature)->exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel);

        auto state = stateForFeature(feature);
        const auto trigger = document.rules.front().trigger;
        const auto preview = document.preview(state, trigger);
        REQUIRE(preview.active_rules.size() == 1);
        REQUIRE(preview.events.size() == 1);

        const auto executed = document.execute(state, trigger);
        REQUIRE(executed.events == preview.events);
        REQUIRE(state.flags.contains(document.rules.front().grants_flags.front()));

        urpg::editor::gameplay::GameplayWysiwygPanel panel;
        panel.loadDocument(document);
        panel.setPreviewContext(stateForFeature(feature), trigger);
        panel.render();

        REQUIRE(panel.snapshot().feature_type == feature);
        REQUIRE(panel.snapshot().visual_layer_count == document.visual_layers.size());
        REQUIRE(panel.snapshot().active_rule_count == 1);
        REQUIRE(panel.saveProjectData() == document.toJson());
        REQUIRE(urpg::gameplay::gameplayWysiwygPreviewToJson(panel.preview())["events"].size() == 1);
    }
}

TEST_CASE("Gameplay WYSIWYG systems report broken authoring diagnostics",
          "[gameplay][wysiwyg][systems]") {
    urpg::gameplay::GameplayWysiwygDocument document;
    document.id = "broken";
    document.feature_type = "status_effect_designer";
    document.display_name = "Broken";
    document.rules.push_back({"", "", "", "", "", 0, -1, {}, {}, {}, {}});

    const auto diagnostics = document.validate();
    REQUIRE(diagnostics.size() >= 7);

    const auto preview = document.preview({}, "missing_trigger");
    REQUIRE(preview.active_rules.empty());
    REQUIRE_FALSE(preview.diagnostics.empty());
}
