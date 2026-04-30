#include "engine/core/community/community_wysiwyg_feature.h"
#include "engine/core/editor/editor_panel_registry.h"
#include "editor/community/community_wysiwyg_panel.h"

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
        {"smart_event_workflow", "smart_event_workflow"},
        {"event_template_library", "event_template_library"},
        {"interaction_prompt_system", "interaction_prompt_system"},
        {"message_log_history", "message_log_history"},
        {"minimap_fog_of_war", "minimap_fog_of_war"},
        {"picture_hotspot_common_event", "picture_hotspot_common_event"},
        {"common_event_menu_builder", "common_event_menu_builder"},
        {"developer_debug_overlay", "developer_debug_overlay"},
        {"switch_variable_inspector", "switch_variable_inspector"},
        {"asset_dlc_library_manager", "asset_dlc_library_manager"},
        {"hud_maker", "hud_maker"},
        {"plugin_conflict_resolver", "plugin_conflict_resolver"},
    };
    return fixtures;
}

urpg::community::CommunityFeatureRuntimeState stateForFeature(const std::string& feature) {
    urpg::community::CommunityFeatureRuntimeState state;
    if (feature == "picture_hotspot_common_event") {
        state.flags.insert("gallery_unlocked");
    } else if (feature == "common_event_menu_builder") {
        state.flags.insert("has_campfire");
    } else if (feature == "developer_debug_overlay") {
        state.flags.insert("dev_mode");
    }
    return state;
}

bool containsCommand(const std::vector<std::string>& commands, const std::string& command) {
    return std::find(commands.begin(), commands.end(), command) != commands.end();
}

} // namespace

TEST_CASE("Community WYSIWYG features expose runtime editor saved-data and registry hooks",
          "[community][wysiwyg][features]") {
    const auto features = urpg::community::communityWysiwygFeatureTypes();
    REQUIRE(features.size() == 12);

    for (const auto& feature : features) {
        CAPTURE(feature);
        const auto document = urpg::community::CommunityWysiwygFeatureDocument::fromJson(loadFixture(fixtureByFeature().at(feature)));

        REQUIRE(document.feature_type == feature);
        REQUIRE(document.validate().empty());
        REQUIRE_FALSE(document.visual_layers.empty());
        REQUIRE_FALSE(document.actions.empty());

        const auto* entry = urpg::editor::findEditorPanelRegistryEntry(feature);
        REQUIRE(entry != nullptr);
        if (feature == "developer_debug_overlay") {
            REQUIRE(entry->exposure == urpg::editor::EditorPanelExposure::DevOnly);
        } else {
            REQUIRE(entry->exposure == urpg::editor::EditorPanelExposure::Deferred);
        }

        auto state = stateForFeature(feature);
        const auto trigger = document.actions.front().trigger;
        const auto preview = document.preview(state, trigger);
        REQUIRE(preview.active_actions.size() == 1);
        REQUIRE(preview.resulting_state.emitted_commands.size() == 1);

        const auto executed = document.execute(state, trigger);
        REQUIRE(executed.active_actions == preview.active_actions);
        REQUIRE_FALSE(state.emitted_commands.empty());

        urpg::editor::community::CommunityWysiwygPanel panel;
        panel.loadDocument(document);
        panel.setPreviewContext(stateForFeature(feature), trigger);
        panel.render();

        REQUIRE(panel.snapshot().feature_type == feature);
        REQUIRE(panel.snapshot().visual_layer_count == document.visual_layers.size());
        REQUIRE(panel.snapshot().active_action_count == 1);
        REQUIRE(panel.snapshot().emitted_command_count == 1);
        REQUIRE(panel.saveProjectData() == document.toJson());
        REQUIRE(urpg::community::communityFeaturePreviewToJson(panel.preview())["active_actions"].size() == 1);
    }
}

TEST_CASE("Community WYSIWYG feature-specific commands match researched RPG Maker needs",
          "[community][wysiwyg][features]") {
    std::map<std::string, std::pair<std::string, std::string>> expected = {
        {"message_log_history", {"message_displayed", "record_message:message_log"}},
        {"minimap_fog_of_war", {"map_entered", "render_minimap:current_map"}},
        {"hud_maker", {"hud_preview", "render_hud:map_hud"}},
        {"switch_variable_inspector", {"rename_variable", "refactor_variable:variable.story_route"}},
        {"asset_dlc_library_manager", {"mount_asset_pack", "mount_asset_pack:dlc.fantasy_town"}},
        {"plugin_conflict_resolver", {"analyze_plugins", "analyze_plugin_conflicts:plugin_stack"}},
    };

    for (const auto& [feature, triggerAndCommand] : expected) {
        CAPTURE(feature);
        auto document = urpg::community::CommunityWysiwygFeatureDocument::fromJson(loadFixture(fixtureByFeature().at(feature)));
        auto state = stateForFeature(feature);
        const auto preview = document.execute(state, triggerAndCommand.first);
        REQUIRE(preview.active_actions.size() == 1);
        REQUIRE(containsCommand(state.emitted_commands, triggerAndCommand.second));
    }
}

TEST_CASE("Developer debug overlay is gated to dev mode",
          "[community][wysiwyg][features]") {
    const auto document = urpg::community::CommunityWysiwygFeatureDocument::fromJson(loadFixture("developer_debug_overlay"));

    urpg::community::CommunityFeatureRuntimeState locked_state;
    const auto locked = document.preview(locked_state, "debug_hotkey");
    REQUIRE(locked.active_actions.empty());
    REQUIRE_FALSE(locked.diagnostics.empty());

    auto dev_state = stateForFeature("developer_debug_overlay");
    const auto active = document.preview(dev_state, "debug_hotkey");
    REQUIRE(active.active_actions.size() == 1);
    REQUIRE(containsCommand(active.resulting_state.emitted_commands, "open_debug_console:debug_overlay"));
}

TEST_CASE("Community WYSIWYG features report broken authoring diagnostics",
          "[community][wysiwyg][features]") {
    urpg::community::CommunityWysiwygFeatureDocument document;
    document.id = "broken";
    document.feature_type = "smart_event_workflow";
    document.display_name = "Broken";
    document.actions.push_back({"", "", "", "", "", {}, {}});

    const auto diagnostics = document.validate();
    REQUIRE(diagnostics.size() >= 5);

    const auto preview = document.preview({}, "missing_trigger");
    REQUIRE(preview.active_actions.empty());
    REQUIRE_FALSE(preview.diagnostics.empty());
}
