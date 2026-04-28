#include "engine/core/editor/editor_panel_registry.h"
#include "engine/core/maker/maker_wysiwyg_feature.h"
#include "editor/maker/maker_wysiwyg_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
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
        {"project_search_everywhere", "project_search_everywhere"},
        {"broken_reference_repair", "broken_reference_repair"},
        {"mass_edit_batch_operations", "mass_edit_batch_operations"},
        {"project_diff_change_review", "project_diff_change_review"},
        {"template_instance_sync", "template_instance_sync"},
        {"parallax_mapping_editor", "parallax_mapping_editor"},
        {"collision_passability_visualizer", "collision_passability_visualizer"},
        {"world_map_route_planner", "world_map_route_planner"},
        {"secret_hidden_object_authoring", "secret_hidden_object_authoring"},
        {"biome_rule_system", "biome_rule_system"},
        {"dialogue_relationship_matrix", "dialogue_relationship_matrix"},
        {"branch_coverage_preview", "branch_coverage_preview"},
        {"cutscene_blocking_tool", "cutscene_blocking_tool"},
        {"localization_context_review", "localization_context_review"},
        {"damage_formula_visual_lab", "damage_formula_visual_lab"},
        {"enemy_troop_timeline_preview", "enemy_troop_timeline_preview"},
        {"skill_combo_chain_builder", "skill_combo_chain_builder"},
        {"bestiary_discovery_system", "bestiary_discovery_system"},
        {"formation_positioning_system", "formation_positioning_system"},
        {"menu_builder", "menu_builder"},
        {"journal_hub", "journal_hub"},
        {"notification_toast_system", "notification_toast_system"},
        {"input_prompt_skinner", "input_prompt_skinner"},
        {"playtest_session_recorder", "playtest_session_recorder"},
        {"bug_report_packager", "bug_report_packager"},
        {"performance_heatmap", "performance_heatmap"},
        {"release_checklist_dashboard", "release_checklist_dashboard"},
        {"store_page_asset_generator", "store_page_asset_generator"},
        {"mod_conflict_visualizer", "mod_conflict_visualizer"},
        {"mod_packaging_wizard", "mod_packaging_wizard"},
        {"plugin_to_native_migration_advisor", "plugin_to_native_migration_advisor"},
    };
    return fixtures;
}

bool containsOperation(const std::vector<std::string>& operations, const std::string& operation) {
    return std::find(operations.begin(), operations.end(), operation) != operations.end();
}

} // namespace

TEST_CASE("Maker WYSIWYG features expose runtime editor saved-data and registry hooks",
          "[maker][wysiwyg][features]") {
    const auto features = urpg::maker::makerWysiwygFeatureTypes();
    REQUIRE(features.size() == 31);

    for (const auto& feature : features) {
        CAPTURE(feature);
        const auto document = urpg::maker::MakerWysiwygFeatureDocument::fromJson(loadFixture(fixtureByFeature().at(feature)));

        REQUIRE(document.feature_type == feature);
        REQUIRE(document.validate().empty());
        REQUIRE_FALSE(document.visual_layers.empty());
        REQUIRE_FALSE(document.actions.empty());

        const auto* entry = urpg::editor::findEditorPanelRegistryEntry(feature);
        REQUIRE(entry != nullptr);
        REQUIRE(entry->exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel);

        urpg::maker::MakerFeatureRuntimeState state;
        const auto trigger = document.actions.front().trigger;
        const auto preview = document.preview(state, trigger);
        REQUIRE(preview.active_actions.size() == 1);
        REQUIRE(preview.resulting_state.emitted_operations.size() == 1);

        const auto executed = document.execute(state, trigger);
        REQUIRE(executed.active_actions == preview.active_actions);
        REQUIRE_FALSE(state.emitted_operations.empty());

        urpg::editor::maker::MakerWysiwygPanel panel;
        panel.loadDocument(document);
        panel.setPreviewContext({}, trigger);
        panel.render();

        REQUIRE(panel.snapshot().feature_type == feature);
        REQUIRE(panel.snapshot().visual_layer_count == document.visual_layers.size());
        REQUIRE(panel.snapshot().active_action_count == 1);
        REQUIRE(panel.snapshot().emitted_operation_count == 1);
        REQUIRE(panel.saveProjectData() == document.toJson());
        REQUIRE(urpg::maker::makerFeaturePreviewToJson(panel.preview())["active_actions"].size() == 1);
    }
}

TEST_CASE("Maker WYSIWYG feature-specific operations match the requested maker backlog",
          "[maker][wysiwyg][features]") {
    std::map<std::string, std::pair<std::string, std::string>> expected = {
        {"project_search_everywhere", {"search_query_changed", "search_project:project_index"}},
        {"broken_reference_repair", {"scan_references", "repair_reference:missing_asset"}},
        {"parallax_mapping_editor", {"map_preview", "render_parallax_layers:current_map"}},
        {"damage_formula_visual_lab", {"formula_changed", "plot_damage_formula:formula_graph"}},
        {"playtest_session_recorder", {"playtest_started", "start_session_recording:playtest_trace"}},
        {"release_checklist_dashboard", {"release_check", "evaluate_release_checklist:release_dashboard"}},
        {"mod_packaging_wizard", {"package_mod", "build_mod_package:mod_manifest"}},
        {"plugin_to_native_migration_advisor", {"analyze_plugin", "suggest_native_migration:plugin_report"}},
    };

    for (const auto& [feature, triggerAndOperation] : expected) {
        CAPTURE(feature);
        auto document = urpg::maker::MakerWysiwygFeatureDocument::fromJson(loadFixture(fixtureByFeature().at(feature)));
        urpg::maker::MakerFeatureRuntimeState state;
        const auto preview = document.execute(state, triggerAndOperation.first);
        REQUIRE(preview.active_actions.size() == 1);
        REQUIRE(containsOperation(state.emitted_operations, triggerAndOperation.second));
    }
}

TEST_CASE("Maker WYSIWYG features report broken authoring diagnostics",
          "[maker][wysiwyg][features]") {
    urpg::maker::MakerWysiwygFeatureDocument document;
    document.id = "broken";
    document.feature_type = "project_search_everywhere";
    document.display_name = "Broken";
    document.actions.push_back({"", "", "", "", "", {}, {}});

    const auto diagnostics = document.validate();
    REQUIRE(diagnostics.size() >= 5);

    const auto preview = document.preview({}, "missing_trigger");
    REQUIRE(preview.active_actions.empty());
    REQUIRE_FALSE(preview.diagnostics.empty());
}
