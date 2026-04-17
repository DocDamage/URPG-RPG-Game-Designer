#include "editor/diagnostics/diagnostics_workspace.h"
#include "editor/diagnostics/diagnostics_facade.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/battle/battle_core.h"
#include "engine/core/message/message_core.h"
#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_scene_graph.h"

#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>

namespace {

class WorkspaceAbility final : public urpg::ability::GameplayAbility {
public:
    explicit WorkspaceAbility(std::string ability_id) : ability_id_(std::move(ability_id)) {}

    const std::string& getId() const override { return ability_id_; }
    const ActivationInfo& getActivationInfo() const override { return info_; }
    void activate([[maybe_unused]] urpg::ability::AbilitySystemComponent& source) override {}

    ActivationInfo& editInfo() { return info_; }

private:
    std::string ability_id_;
    ActivationInfo info_;
};

} // namespace

TEST_CASE("DiagnosticsWorkspace - Refresh updates compat and save tabs", "[editor][diagnostics][integration]") {
    auto& pluginManager = urpg::compat::PluginManager::instance();
    pluginManager.unloadAllPlugins();
    pluginManager.clearFailureDiagnostics();

    const auto base = std::filesystem::temp_directory_path() / "urpg_diagnostics_workspace";
    std::filesystem::create_directories(base);

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);

    urpg::SaveSessionSaveRequest saveRequest;
    saveRequest.slot_id = 4;
    saveRequest.meta.category = urpg::SaveSlotCategory::Manual;
    saveRequest.meta.retention_class = urpg::SaveRetentionClass::Manual;
    saveRequest.meta.map_display_name = "Foundry";
    saveRequest.payload = "{\"slot\":4}";
    saveRequest.primary_save_path = base / "slot_4.json";
    REQUIRE(coordinator.save(saveRequest).ok);

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindSaveRuntime(catalog, coordinator);
    urpg::message::MessageFlowRunner messageRunner;
    messageRunner.begin({
        {"speaker_a", "Welcome back.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
        {"narration_b", "", urpg::message::variantFromCompatRoute("narration", "Alicia", 3), true, {}, 0},
    });
    urpg::message::RichTextLayoutEngine messageLayout;
    workspace.bindMessageRuntime(messageRunner, messageLayout);
    urpg::battle::BattleFlowController battleFlow;
    battleFlow.beginBattle(true);
    battleFlow.enterAction();
    battleFlow.noteEscapeFailure();
    urpg::battle::BattleActionQueue battleQueue;
    battleQueue.enqueue({"actor_main", "enemy_0", "attack", 120, 0});
    battleQueue.enqueue({"", "enemy_0", "attack", 90, 0});
    workspace.bindBattleRuntime(battleFlow, battleQueue);
    urpg::audio::AudioCore audioCore;
    audioCore.playSound("workspace_test_se", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(audioCore);
    workspace.bindMigrationWizardRuntime(nlohmann::json{
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    });
    workspace.ingestEventAuthorityDiagnosticsJsonl(
        "{\"ts\":\"2026-03-04T00:00:02Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_workspace\",\"block_id\":\"blk_workspace\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"workspace test\"}"
    );

    pluginManager.executeCommand("MissingPlugin", "missingCommand", {});
    REQUIRE_FALSE(pluginManager.exportFailureDiagnosticsJsonl().empty());

    workspace.refresh();

    const auto compatSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Compat);
    REQUIRE(compatSummary.active);
    REQUIRE(compatSummary.item_count == 1);
    REQUIRE(compatSummary.issue_count == 1);
    REQUIRE(compatSummary.has_data);

    const auto saveSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Save);
    REQUIRE_FALSE(saveSummary.active);
    REQUIRE(saveSummary.item_count == 1);
    REQUIRE(saveSummary.issue_count == 0);
    REQUIRE(saveSummary.has_data);

    const auto eventSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::EventAuthority);
    REQUIRE_FALSE(eventSummary.active);
    REQUIRE(eventSummary.item_count == 1);
    REQUIRE(eventSummary.issue_count == 1);
    REQUIRE(eventSummary.has_data);

    const auto messageSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::MessageText);
    REQUIRE_FALSE(messageSummary.active);
    REQUIRE(messageSummary.item_count == 2);
    REQUIRE(messageSummary.issue_count >= 1);
    REQUIRE(messageSummary.has_data);

    const auto battleSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Battle);
    REQUIRE_FALSE(battleSummary.active);
    REQUIRE(battleSummary.item_count == 2);
    REQUIRE(battleSummary.issue_count >= 1);
    REQUIRE(battleSummary.has_data);

    const auto menuSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Menu);
    REQUIRE_FALSE(menuSummary.active);

    const auto abilitiesSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE_FALSE(abilitiesSummary.active);

    const auto allSummaries = workspace.allTabSummaries();
    REQUIRE(allSummaries.size() == 9);

    urpg::editor::DiagnosticsFacade facade(workspace);
    const auto exportedJson = nlohmann::json::parse(facade.emitSnapshot());
    REQUIRE(exportedJson["active_tab"] == "compat");
    REQUIRE(exportedJson["visible"] == true);
    REQUIRE(exportedJson.contains("active_tab_detail"));
    REQUIRE(exportedJson["active_tab_detail"]["tab"] == "compat");
    REQUIRE(exportedJson["active_tab_detail"]["project_compatibility_score"] == 70);
    REQUIRE(exportedJson["active_tab_detail"]["selected_plugin"] == nullptr);
    REQUIRE(exportedJson["active_tab_detail"]["detail_view"] == false);
    REQUIRE(exportedJson["active_tab_detail"]["plugins"].is_array());
    REQUIRE(exportedJson["active_tab_detail"]["plugins"].size() == 1);
    REQUIRE(exportedJson["active_tab_detail"]["plugins"][0]["pluginId"] == "MissingPlugin");
    REQUIRE(exportedJson["active_tab_detail"]["plugins"][0]["partialCount"] == 1);
    REQUIRE(exportedJson["active_tab_detail"]["plugins"][0]["stubCount"] == 0);
    REQUIRE(exportedJson["active_tab_detail"]["plugins"][0]["unsupportedCount"] == 0);
    REQUIRE(exportedJson["active_tab_detail"]["plugins"][0]["scoreHistory"].is_array());
    REQUIRE(exportedJson["active_tab_detail"]["recent_events"].is_array());
    REQUIRE(exportedJson["active_tab_detail"]["recent_events"].size() == 1);
    REQUIRE(exportedJson["active_tab_detail"]["recent_events"][0]["pluginId"] == "MissingPlugin");
    REQUIRE(exportedJson["active_tab_detail"]["recent_events"][0]["severity"] == "WARNING");
    REQUIRE(exportedJson["tabs"].is_array());
    REQUIRE(exportedJson["tabs"].size() == 9);
    REQUIRE(exportedJson["tabs"][0]["name"] == "compat");
    REQUIRE(exportedJson["tabs"][0]["item_count"] == 1);
    REQUIRE(exportedJson["tabs"][1]["name"] == "save");
    REQUIRE(exportedJson["tabs"][2]["name"] == "event_authority");
    REQUIRE(exportedJson["tabs"][3]["name"] == "message_text");
    REQUIRE(exportedJson["tabs"][4]["name"] == "battle");
    REQUIRE(exportedJson["tabs"][5]["name"] == "menu");
    REQUIRE(exportedJson["tabs"][6]["name"] == "audio");
    REQUIRE(exportedJson["tabs"][7]["name"] == "migration_wizard");
    REQUIRE(exportedJson["tabs"][8]["name"] == "abilities");

    REQUIRE(workspace.activeTab() == urpg::editor::DiagnosticsTab::Compat);
    REQUIRE(workspace.compatPanel().isVisible());
    REQUIRE_FALSE(workspace.savePanel().isVisible());
    REQUIRE_FALSE(workspace.messagePanel().isVisible());
    REQUIRE_FALSE(workspace.battlePanel().isVisible());
    REQUIRE(workspace.compatPanel().getModel().getPluginEvents("MissingPlugin").size() == 1);
    workspace.compatPanel().selectPlugin("MissingPlugin");
    const auto compatDetailJson = nlohmann::json::parse(facade.emitSnapshot());
    REQUIRE(compatDetailJson["active_tab"] == "compat");
    REQUIRE(compatDetailJson["active_tab_detail"]["selected_plugin"] == "MissingPlugin");
    REQUIRE(compatDetailJson["active_tab_detail"]["detail_view"] == true);
    REQUIRE(compatDetailJson["active_tab_detail"]["selected_plugin_summary"]["pluginId"] == "MissingPlugin");
    REQUIRE(compatDetailJson["active_tab_detail"]["selected_plugin_calls"].is_array());
    REQUIRE(compatDetailJson["active_tab_detail"]["selected_plugin_calls"].size() == 1);
    REQUIRE(compatDetailJson["active_tab_detail"]["selected_plugin_calls"][0]["className"] == "PluginManager");
    REQUIRE(compatDetailJson["active_tab_detail"]["recent_events"][0]["methodName"] == "execute_command");
    REQUIRE(compatDetailJson["active_tab_detail"]["selected_plugin_events"].is_array());
    REQUIRE(compatDetailJson["active_tab_detail"]["selected_plugin_events"].size() == 1);
    REQUIRE(compatDetailJson["active_tab_detail"]["selected_plugin_events"][0]["pluginId"] == "MissingPlugin");
    REQUIRE(pluginManager.exportFailureDiagnosticsJsonl().empty());

    const auto& saveRows = workspace.savePanel().getModel().VisibleRows();
    REQUIRE(saveRows.size() == 1);
    REQUIRE(saveRows[0].slot_id == 4);
    REQUIRE(saveRows[0].category_label == "manual");

    const auto& eventRows = workspace.eventAuthorityPanel().getModel().VisibleRows();
    REQUIRE(eventRows.size() == 1);
    REQUIRE(eventRows[0].event_id == "evt_workspace");

    const auto& messageRows = workspace.messagePanel().getModel().VisibleRows();
    REQUIRE(messageRows.size() == 2);
    REQUIRE(messageRows[0].page_id == "speaker_a");

    const auto& battleRows = workspace.battlePanel().getModel().VisibleRows();
    REQUIRE(battleRows.size() == 2);
    REQUIRE(battleRows[0].subject_id == "actor_main");
    REQUIRE(workspace.battlePanel().previewPanel().snapshot().physical_damage > 0);

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Save);
    workspace.render();
    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::Save).active);
    REQUIRE_FALSE(workspace.compatPanel().isVisible());
    REQUIRE(workspace.savePanel().isVisible());
    REQUIRE_FALSE(workspace.eventAuthorityPanel().isVisible());
    REQUIRE_FALSE(workspace.messagePanel().isVisible());
    REQUIRE_FALSE(workspace.battlePanel().isVisible());
    REQUIRE(workspace.savePanel().getModel().SelectRow(0));
    const auto saveJson = nlohmann::json::parse(facade.emitSnapshot());
    REQUIRE(saveJson["active_tab"] == "save");
    REQUIRE(saveJson["active_tab_detail"]["tab"] == "save");
    REQUIRE(saveJson["active_tab_detail"]["save_summary"]["total_slots"] == 1);
    REQUIRE(saveJson["active_tab_detail"]["save_summary"]["manual_slots"] == 1);
    REQUIRE(saveJson["active_tab_detail"]["save_summary"]["autosave_enabled"] == true);
    REQUIRE(saveJson["active_tab_detail"]["selected_slot_id"] == 4);
    REQUIRE(saveJson["active_tab_detail"]["visible_rows"].is_array());
    REQUIRE(saveJson["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(saveJson["active_tab_detail"]["visible_rows"][0]["slot_id"] == 4);
    REQUIRE(saveJson["active_tab_detail"]["visible_rows"][0]["map_display_name"] == "Foundry");
    REQUIRE(saveJson["active_tab_detail"]["visible_rows"][0]["reserved_slot"] == false);
    REQUIRE(saveJson["active_tab_detail"]["visible_rows"][0]["corrupted"] == false);

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::EventAuthority);
    workspace.eventAuthorityPanel().setFilter("evt_workspace");
    workspace.eventAuthorityPanel().setLevelFilter("warn");
    workspace.eventAuthorityPanel().setModeFilter("compat");
    REQUIRE(workspace.eventAuthorityPanel().selectRow(0));
    workspace.render();
    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::EventAuthority).active);
    REQUIRE_FALSE(workspace.compatPanel().isVisible());
    REQUIRE_FALSE(workspace.savePanel().isVisible());
    REQUIRE(workspace.eventAuthorityPanel().isVisible());
    REQUIRE(workspace.eventAuthorityPanel().hasRenderedFrame());
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().visible_rows == 1);
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().warning_count == 1);
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().error_count == 0);
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().event_id_filter == "evt_workspace");
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().level_filter == "warn");
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().mode_filter == "compat");
    REQUIRE_FALSE(workspace.eventAuthorityPanel().lastRenderSnapshot().can_select_next_row);
    REQUIRE_FALSE(workspace.eventAuthorityPanel().lastRenderSnapshot().can_select_previous_row);
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().selected_row_index.has_value());
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().selected_row_index.value() == 0);
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().selected_row.has_value());
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().selected_row->event_id == "evt_workspace");
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().selected_row->block_id == "blk_workspace");
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().selected_navigation_target.has_value());
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().selected_navigation_target->event_id == "evt_workspace");
    REQUIRE_FALSE(workspace.messagePanel().isVisible());
    REQUIRE_FALSE(workspace.battlePanel().isVisible());

    const auto eventJson = nlohmann::json::parse(facade.emitSnapshot());
    REQUIRE(eventJson["active_tab"] == "event_authority");
    REQUIRE(eventJson["active_tab_detail"]["tab"] == "event_authority");
    REQUIRE(eventJson["active_tab_detail"]["event_id_filter"] == "evt_workspace");
    REQUIRE(eventJson["active_tab_detail"]["level_filter"] == "warn");
    REQUIRE(eventJson["active_tab_detail"]["mode_filter"] == "compat");
    REQUIRE(eventJson["active_tab_detail"]["visible_rows"] == 1);
    REQUIRE(eventJson["active_tab_detail"]["visible_row_entries"].is_array());
    REQUIRE(eventJson["active_tab_detail"]["visible_row_entries"].size() == 1);
    REQUIRE(eventJson["active_tab_detail"]["visible_row_entries"][0]["event_id"] == "evt_workspace");
    REQUIRE(eventJson["active_tab_detail"]["visible_row_entries"][0]["block_id"] == "blk_workspace");
    REQUIRE(eventJson["active_tab_detail"]["has_selection"] == true);
    REQUIRE(eventJson["active_tab_detail"]["can_select_next_row"] == false);
    REQUIRE(eventJson["active_tab_detail"]["can_select_previous_row"] == false);
    REQUIRE(eventJson["active_tab_detail"]["selected_row_index"] == 0);
    REQUIRE(eventJson["active_tab_detail"]["selected_row"]["event_id"] == "evt_workspace");
    REQUIRE(eventJson["active_tab_detail"]["selected_row"]["block_id"] == "blk_workspace");
    REQUIRE(eventJson["active_tab_detail"]["selected_navigation_target"]["event_id"] == "evt_workspace");

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MessageText);
    workspace.update();
    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::MessageText).active);
    REQUIRE_FALSE(workspace.compatPanel().isVisible());
    REQUIRE_FALSE(workspace.savePanel().isVisible());
    REQUIRE_FALSE(workspace.eventAuthorityPanel().isVisible());
    REQUIRE(workspace.messagePanel().isVisible());
    REQUIRE_FALSE(workspace.battlePanel().isVisible());
    const auto messageJson = nlohmann::json::parse(facade.emitSnapshot());
    REQUIRE(messageJson["active_tab"] == "message_text");
    REQUIRE(messageJson["active_tab_detail"]["tab"] == "message_text");
    REQUIRE(messageJson["active_tab_detail"]["message_summary"]["total_pages"] == 2);
    REQUIRE(messageJson["active_tab_detail"]["message_summary"]["speaker_pages"] == 1);
    REQUIRE(messageJson["active_tab_detail"]["message_summary"]["narration_pages"] == 1);
    REQUIRE(messageJson["active_tab_detail"]["message_summary"]["issue_count"] == 1);
    REQUIRE(messageJson["active_tab_detail"]["message_summary"]["has_active_flow"] == true);
    REQUIRE(messageJson["active_tab_detail"]["message_summary"]["current_page_index"] == 0);
    REQUIRE(messageJson["active_tab_detail"]["selected_page_id"] == nullptr);
    REQUIRE(messageJson["active_tab_detail"]["visible_rows"].is_array());
    REQUIRE(messageJson["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(messageJson["active_tab_detail"]["visible_rows"][0]["page_id"] == "speaker_a");
    REQUIRE(messageJson["active_tab_detail"]["visible_rows"][0]["speaker"] == "Alicia");
    REQUIRE(messageJson["active_tab_detail"]["visible_rows"][0]["has_choices"] == false);
    REQUIRE(messageJson["active_tab_detail"]["issues"].is_array());
    REQUIRE(messageJson["active_tab_detail"]["issues"].size() == 1);
    REQUIRE(messageJson["active_tab_detail"]["issues"][0]["severity"] == "warning");
    REQUIRE(messageJson["active_tab_detail"]["issues"][0]["page_id"] == "narration_b");

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Battle);
    workspace.update();
    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::Battle).active);
    REQUIRE_FALSE(workspace.compatPanel().isVisible());
    REQUIRE_FALSE(workspace.savePanel().isVisible());
    REQUIRE_FALSE(workspace.eventAuthorityPanel().isVisible());
    REQUIRE_FALSE(workspace.messagePanel().isVisible());
    REQUIRE(workspace.battlePanel().isVisible());
    const auto battleJson = nlohmann::json::parse(facade.emitSnapshot());
    REQUIRE(battleJson["active_tab"] == "battle");
    REQUIRE(battleJson["active_tab_detail"]["tab"] == "battle");
    REQUIRE(battleJson["active_tab_detail"]["battle_summary"]["phase"] == "action");
    REQUIRE(battleJson["active_tab_detail"]["battle_summary"]["active"] == true);
    REQUIRE(battleJson["active_tab_detail"]["battle_summary"]["total_actions"] == 2);
    REQUIRE(battleJson["active_tab_detail"]["selected_subject_id"] == nullptr);
    REQUIRE(battleJson["active_tab_detail"]["visible_rows"].is_array());
    REQUIRE(battleJson["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(battleJson["active_tab_detail"]["visible_rows"][0]["subject_id"] == "actor_main");
    REQUIRE(battleJson["active_tab_detail"]["preview"]["phase"] == "action");
    REQUIRE(battleJson["active_tab_detail"]["preview"]["can_escape"] == true);
    REQUIRE(battleJson["active_tab_detail"]["preview"]["physical_damage"].get<int>() > 0);
    REQUIRE(battleJson["active_tab_detail"]["preview_issues"].is_array());

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Audio);
    workspace.render();
    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::Audio).active);
    REQUIRE_FALSE(workspace.compatPanel().isVisible());
    REQUIRE_FALSE(workspace.savePanel().isVisible());
    REQUIRE_FALSE(workspace.eventAuthorityPanel().isVisible());
    REQUIRE_FALSE(workspace.messagePanel().isVisible());
    REQUIRE_FALSE(workspace.battlePanel().isVisible());
    REQUIRE(workspace.audioPanel().isVisible());
    REQUIRE(workspace.audioPanel().hasRenderedFrame());
    REQUIRE(workspace.audioPanel().lastRenderSnapshot().master_volume == 1.0f);
    REQUIRE(workspace.audioPanel().lastRenderSnapshot().live_rows.size() == 1);
    REQUIRE(workspace.audioPanel().lastRenderSnapshot().live_rows[0].assetId == "workspace_test_se");
    const auto audioJson = nlohmann::json::parse(facade.emitSnapshot());
    REQUIRE(audioJson["active_tab"] == "audio");
    REQUIRE(audioJson["active_tab_detail"]["tab"] == "audio");
    REQUIRE(audioJson["active_tab_detail"]["master_volume"] == 1.0);
    REQUIRE(audioJson["active_tab_detail"]["active_count"] == 1);
    REQUIRE(audioJson["active_tab_detail"]["issue_count"] == 0);
    REQUIRE(audioJson["active_tab_detail"]["has_data"] == true);
    REQUIRE(audioJson["active_tab_detail"]["live_rows"].is_array());
    REQUIRE(audioJson["active_tab_detail"]["live_rows"].size() == 1);
    REQUIRE(audioJson["active_tab_detail"]["live_rows"][0]["assetId"] == "workspace_test_se");
    REQUIRE(audioJson["active_tab_detail"]["live_rows"][0]["category"] == "SE");

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();
    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::MigrationWizard).active);
    REQUIRE_FALSE(workspace.audioPanel().isVisible());
    REQUIRE(workspace.migrationWizardPanel().isVisible());
    REQUIRE(workspace.migrationWizardPanel().hasRenderedFrame());
    REQUIRE(workspace.migrationWizardPanel().lastRenderSnapshot().total_files_processed == 1);
    REQUIRE(workspace.migrationWizardPanel().lastRenderSnapshot().is_complete);
    REQUIRE(workspace.migrationWizardPanel().lastRenderSnapshot().summary_log_count == 2);
    REQUIRE(workspace.migrationWizardPanel().lastRenderSnapshot().headline == "Migration wizard complete.");
    REQUIRE(workspace.migrationWizardPanel().lastRenderSnapshot().summary_logs.size() == 2);
    REQUIRE(workspace.migrationWizardPanel().lastRenderSnapshot().summary_logs[0].find("Menu migration") != std::string::npos);
    REQUIRE(workspace.migrationWizardPanel().lastRenderSnapshot().subsystem_results.size() == 1);
    REQUIRE(workspace.migrationWizardPanel().lastRenderSnapshot().subsystem_results[0].subsystem_id == "menu");

    workspace.setVisible(false);
    workspace.update();
    const auto hiddenJson = nlohmann::json::parse(facade.emitSnapshot());
    REQUIRE(hiddenJson["active_tab"] == "migration_wizard");
    REQUIRE(hiddenJson["visible"] == false);
    REQUIRE_FALSE(workspace.compatPanel().isVisible());
    REQUIRE_FALSE(workspace.savePanel().isVisible());
    REQUIRE_FALSE(workspace.eventAuthorityPanel().isVisible());
    REQUIRE_FALSE(workspace.messagePanel().isVisible());
    REQUIRE_FALSE(workspace.battlePanel().isVisible());
    REQUIRE_FALSE(workspace.audioPanel().isVisible());
    REQUIRE_FALSE(workspace.migrationWizardPanel().isVisible());

    std::filesystem::remove_all(base);
    pluginManager.clearFailureDiagnostics();
    pluginManager.unloadAllPlugins();
}

TEST_CASE("DiagnosticsWorkspace - Menu runtime binding populates and clears menu diagnostics",
          "[editor][diagnostics][integration][menu]") {
    urpg::ui::MenuCommandRegistry registry;

    urpg::MenuCommandMeta itemCommand;
    itemCommand.id = "urpg.menu.item";
    itemCommand.label = "Item";
    itemCommand.route = urpg::MenuRouteTarget::Item;
    registry.registerCommand(itemCommand);

    urpg::MenuCommandMeta deadEndCommand;
    deadEndCommand.id = "urpg.menu.dead_end";
    deadEndCommand.label = "Dead End";
    deadEndCommand.route = urpg::MenuRouteTarget::Custom;

    auto menu = std::make_shared<urpg::ui::MenuScene>("MainMenu");

    urpg::ui::MenuPane pane;
    pane.id = "main_pane";
    pane.displayName = "Main Menu";
    pane.isVisible = true;
    pane.isActive = true;
    pane.commands = {itemCommand, deadEndCommand};

    menu->addPane(pane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("MainMenu");

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;
    graph.setCommandStateFromRegistry(registry, switches, variables);

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindMenuRuntime(graph, registry, switches, variables);

    const auto menuSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Menu);
    REQUIRE(menuSummary.item_count == 2);
    REQUIRE(menuSummary.issue_count == 2);
    REQUIRE(menuSummary.has_data);

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Menu);
    workspace.update();

    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::Menu).active);
    REQUIRE(workspace.menuPanel().IsVisible());
    REQUIRE(workspace.menuPreviewPanel().GetTitle() == "Menu Preview");
    REQUIRE(workspace.menuPreviewPanel().IsVisible());

    const auto menuSnapshot = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(menuSnapshot["active_tab"] == "menu");
    REQUIRE(menuSnapshot["active_tab_detail"]["tab"] == "menu");
    REQUIRE(menuSnapshot["active_tab_detail"]["summary"]["item_count"] == 2);
    REQUIRE(menuSnapshot["active_tab_detail"]["menu_summary"]["active_scene_id"] == "MainMenu");
    REQUIRE(menuSnapshot["active_tab_detail"]["menu_summary"]["total_commands"] == 2);
    REQUIRE(menuSnapshot["active_tab_detail"]["menu_summary"]["issue_count"] == 2);
    REQUIRE(menuSnapshot["active_tab_detail"]["selected_command_id"] == nullptr);
    REQUIRE(menuSnapshot["active_tab_detail"]["visible_rows"].is_array());
    REQUIRE(menuSnapshot["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(menuSnapshot["active_tab_detail"]["visible_rows"][0]["command_id"] == "urpg.menu.item");
    REQUIRE(menuSnapshot["active_tab_detail"]["visible_rows"][1]["command_id"] == "urpg.menu.dead_end");
    REQUIRE(menuSnapshot["active_tab_detail"]["issues"].is_array());
    REQUIRE(menuSnapshot["active_tab_detail"]["issues"].size() == 2);
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["title"] == "Menu Preview");
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["visible"] == true);

    workspace.clearMenuRuntime();

    const auto clearedSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Menu);
    REQUIRE(clearedSummary.item_count == 0);
    REQUIRE(clearedSummary.issue_count == 0);
    REQUIRE_FALSE(clearedSummary.has_data);

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Compat);
    workspace.update();
    REQUIRE_FALSE(workspace.menuPanel().IsVisible());
    REQUIRE_FALSE(workspace.menuPreviewPanel().IsVisible());
}

TEST_CASE("DiagnosticsWorkspace - Audio and ability runtimes clear and rebind cleanly",
          "[editor][diagnostics][integration][runtime_clear]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    urpg::audio::AudioCore firstAudioCore;
    firstAudioCore.playSound("first_audio", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(firstAudioCore);

    const auto firstAudioSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Audio);
    REQUIRE(firstAudioSummary.item_count == 1);
    REQUIRE(firstAudioSummary.has_data);

    workspace.clearAudioRuntime();

    const auto clearedAudioSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Audio);
    REQUIRE(clearedAudioSummary.item_count == 0);
    REQUIRE_FALSE(clearedAudioSummary.has_data);

    urpg::audio::AudioCore reboundAudioCore;
    reboundAudioCore.playSound("rebound_audio_a", urpg::audio::AudioCategory::SE);
    reboundAudioCore.playSound("rebound_audio_b", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(reboundAudioCore);

    const auto reboundAudioSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Audio);
    REQUIRE(reboundAudioSummary.item_count == 2);
    REQUIRE(reboundAudioSummary.has_data);

    urpg::ability::AbilitySystemComponent firstAsc;
    firstAsc.addTag(urpg::ability::GameplayTag("State.Empowered"));
    auto firstAbility = std::make_shared<WorkspaceAbility>("skill.first");
    firstAsc.grantAbility(firstAbility);
    workspace.bindAbilityRuntime(firstAsc);

    const auto firstAbilitySummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(firstAbilitySummary.item_count == 1);
    REQUIRE(firstAbilitySummary.has_data);
    REQUIRE(workspace.abilityPanel().getModel().getActiveTags().size() == 1);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.update();
    const auto firstAbilityJson = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(firstAbilityJson["active_tab"] == "abilities");
    REQUIRE(firstAbilityJson["active_tab_detail"]["tab"] == "abilities");
    REQUIRE(firstAbilityJson["active_tab_detail"]["abilities"].is_array());
    REQUIRE(firstAbilityJson["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(firstAbilityJson["active_tab_detail"]["abilities"][0]["name"] == "skill.first");
    REQUIRE(firstAbilityJson["active_tab_detail"]["abilities"][0]["can_activate"] == true);
    REQUIRE(firstAbilityJson["active_tab_detail"]["active_tags"].is_array());
    REQUIRE(firstAbilityJson["active_tab_detail"]["active_tags"].size() == 1);
    REQUIRE(firstAbilityJson["active_tab_detail"]["active_tags"][0]["tag"] == "State.Empowered");

    workspace.clearAbilityRuntime();

    const auto clearedAbilitySummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(clearedAbilitySummary.item_count == 0);
    REQUIRE_FALSE(clearedAbilitySummary.has_data);
    REQUIRE(workspace.abilityPanel().getModel().getActiveTags().empty());
    const auto clearedAbilityJson = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(clearedAbilityJson["active_tab"] == "abilities");
    REQUIRE(clearedAbilityJson["active_tab_detail"]["abilities"].is_array());
    REQUIRE(clearedAbilityJson["active_tab_detail"]["abilities"].empty());
    REQUIRE(clearedAbilityJson["active_tab_detail"]["active_tags"].is_array());
    REQUIRE(clearedAbilityJson["active_tab_detail"]["active_tags"].empty());

    urpg::ability::AbilitySystemComponent reboundAsc;
    reboundAsc.addTag(urpg::ability::GameplayTag("State.Charged"));
    auto reboundAbilityA = std::make_shared<WorkspaceAbility>("skill.rebound_a");
    auto reboundAbilityB = std::make_shared<WorkspaceAbility>("skill.rebound_b");
    reboundAsc.grantAbility(reboundAbilityA);
    reboundAsc.grantAbility(reboundAbilityB);
    workspace.bindAbilityRuntime(reboundAsc);

    const auto reboundAbilitySummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(reboundAbilitySummary.item_count == 2);
    REQUIRE(reboundAbilitySummary.has_data);
    REQUIRE(workspace.abilityPanel().getModel().getActiveTags().size() == 1);
    const auto reboundAbilityJson = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(reboundAbilityJson["active_tab"] == "abilities");
    REQUIRE(reboundAbilityJson["active_tab_detail"]["abilities"].is_array());
    REQUIRE(reboundAbilityJson["active_tab_detail"]["abilities"].size() == 2);
    REQUIRE(reboundAbilityJson["active_tab_detail"]["abilities"][0]["name"] == "skill.rebound_a");
    REQUIRE(reboundAbilityJson["active_tab_detail"]["abilities"][1]["name"] == "skill.rebound_b");
    REQUIRE(reboundAbilityJson["active_tab_detail"]["active_tags"].size() == 1);
    REQUIRE(reboundAbilityJson["active_tab_detail"]["active_tags"][0]["tag"] == "State.Charged");
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard state clears cleanly",
          "[editor][diagnostics][integration][wizard_clear]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    workspace.migrationWizardPanel().onProjectUpdateRequested({
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    });

    const auto populatedSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::MigrationWizard);
    REQUIRE(populatedSummary.item_count == 1);
    REQUIRE(populatedSummary.has_data);

    workspace.clearMigrationWizardRuntime();

    const auto clearedSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::MigrationWizard);
    REQUIRE(clearedSummary.item_count == 0);
    REQUIRE_FALSE(clearedSummary.has_data);
    REQUIRE(workspace.migrationWizardPanel().getModel()->getReport().summary_logs.empty());
    REQUIRE(workspace.migrationWizardPanel().getModel()->getReport().subsystem_results.empty());
    REQUIRE_FALSE(workspace.migrationWizardPanel().getModel()->getReport().is_complete);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard export carries selected subsystem detail",
          "[editor][diagnostics][integration][wizard_export]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    workspace.bindMigrationWizardRuntime({
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    });

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    const auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "migration_wizard");
    REQUIRE(exported["active_tab_detail"]["tab"] == "migration_wizard");
    REQUIRE(exported["active_tab_detail"]["summary"]["item_count"] == 2);
    REQUIRE(exported["active_tab_detail"]["summary_logs"].is_array());
    REQUIRE(exported["active_tab_detail"]["summary_logs"].size() == 3);
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].is_array());
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_display_name"] == "Message");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_processed_count"] == 1);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_completed"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_summary_line"].get<std::string>().find("Message migration") != std::string::npos);
    REQUIRE(exported["active_tab_detail"]["can_rerun_selected_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_clear_selected_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_select_next_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_select_previous_subsystem"] == false);
    REQUIRE(exported["active_tab_detail"]["exported_report_json"].is_string());
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard workflow actions are exposed at workspace level",
          "[editor][diagnostics][integration][wizard_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    nlohmann::json project_data = {
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    workspace.bindMigrationWizardRuntime(project_data);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["can_select_next_subsystem"] == true);

    REQUIRE(workspace.selectNextMigrationWizardSubsystemResult());
    workspace.render();

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["can_select_previous_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_select_next_subsystem"] == false);

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(workspace.rerunMigrationWizardSubsystem("menu", project_data));
    workspace.render();

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_summary_line"].get<std::string>().find("2 command(s)") != std::string::npos);

    const auto exported_report_json = workspace.exportMigrationWizardReportJson();
    REQUIRE_FALSE(exported_report_json.empty());
    const auto parsed_report = nlohmann::json::parse(exported_report_json);
    REQUIRE(parsed_report["selected_subsystem_id"] == "menu");

    REQUIRE(workspace.clearMigrationWizardSubsystemResult("menu"));
    workspace.render();

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 1);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard actions keep exported snapshot current without manual render",
          "[editor][diagnostics][integration][wizard_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    nlohmann::json project_data = {
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    workspace.bindMigrationWizardRuntime(project_data);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    REQUIRE(workspace.selectNextMigrationWizardSubsystemResult());
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(workspace.rerunMigrationWizardSubsystem("menu", project_data));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_summary_line"].get<std::string>().find("2 command(s)") != std::string::npos);

    REQUIRE(workspace.clearMigrationWizardSubsystemResult("menu"));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");

    workspace.clearMigrationWizardRuntime();
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].empty());
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard failed load clears exported snapshot state",
          "[editor][diagnostics][integration][wizard_file_failure]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    workspace.bindMigrationWizardRuntime({
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    });
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 1);

    const auto temp_path = (std::filesystem::temp_directory_path() / "urpg_workspace_migration_wizard_bad_load.json").string();
    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << "not valid json";
    }

    REQUIRE_FALSE(workspace.loadMigrationWizardReportFromFile(temp_path));

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);
    REQUIRE(exported["active_tab_detail"]["summary_logs"].empty());
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].empty());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"].is_null());

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard save/load round-trip preserves exported workflow state",
          "[editor][diagnostics][integration][wizard_file_roundtrip]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    nlohmann::json project_data = {
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    workspace.bindMigrationWizardRuntime(project_data);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();
    REQUIRE(workspace.selectNextMigrationWizardSubsystemResult());

    const auto original_report_json = workspace.exportMigrationWizardReportJson();
    const auto temp_path = (std::filesystem::temp_directory_path() / "urpg_workspace_migration_wizard_roundtrip.json").string();
    std::filesystem::remove(temp_path);

    REQUIRE(workspace.saveMigrationWizardReportToFile(temp_path));
    REQUIRE(std::filesystem::exists(temp_path));

    workspace.clearMigrationWizardRuntime();
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].empty());

    REQUIRE(workspace.loadMigrationWizardReportFromFile(temp_path));

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["can_save_report"] == true);
    REQUIRE(exported["active_tab_detail"]["can_load_report"] == true);

    const auto reloaded_report_json = workspace.exportMigrationWizardReportJson();
    REQUIRE(reloaded_report_json == original_report_json);

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - Audio runtime actions keep exported snapshot current without manual render",
          "[editor][diagnostics][integration][audio_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Audio);

    urpg::audio::AudioCore firstAudioCore;
    firstAudioCore.playSound("first_audio", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(firstAudioCore);
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "audio");
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["live_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["live_rows"][0]["assetId"] == "first_audio");

    workspace.clearAudioRuntime();
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);
    REQUIRE(exported["active_tab_detail"]["live_rows"].empty());

    urpg::audio::AudioCore secondAudioCore;
    secondAudioCore.playSound("second_audio_a", urpg::audio::AudioCategory::SE);
    secondAudioCore.playSound("second_audio_b", urpg::audio::AudioCategory::BGS);
    workspace.bindAudioRuntime(secondAudioCore);
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["live_rows"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["live_rows"][0]["assetId"] == "second_audio_a");
    REQUIRE(exported["active_tab_detail"]["live_rows"][1]["assetId"] == "second_audio_b");
}

TEST_CASE("DiagnosticsWorkspace - Event authority actions keep exported snapshot current without manual render",
          "[editor][diagnostics][integration][event_authority_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::EventAuthority);

    workspace.ingestEventAuthorityDiagnosticsJsonl(
        "{\"ts\":\"2026-03-04T00:00:02Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_alpha\",\"block_id\":\"blk_alpha\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"alpha\"}\n"
        "{\"ts\":\"2026-03-04T00:00:03Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_beta\",\"block_id\":\"blk_beta\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"beta\"}"
    );
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "event_authority");
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 2);
    REQUIRE(exported["active_tab_detail"]["visible_row_entries"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);

    workspace.clearEventAuthorityDiagnostics();
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 0);
    REQUIRE(exported["active_tab_detail"]["visible_row_entries"].empty());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);

    workspace.ingestEventAuthorityDiagnosticsJsonl(
        "{\"ts\":\"2026-03-04T00:00:04Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_gamma\",\"block_id\":\"blk_gamma\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"new_warning\",\"message\":\"gamma\"}"
    );
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 1);
    REQUIRE(exported["active_tab_detail"]["visible_row_entries"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_row_entries"][0]["event_id"] == "evt_gamma");
}

TEST_CASE("DiagnosticsWorkspace - Activating a snapshot-backed tab refreshes exported detail without manual render",
          "[editor][diagnostics][integration][tab_switch_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    urpg::audio::AudioCore audioCore;
    audioCore.playSound("tab_switch_audio", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(audioCore);

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "compat");

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Audio);
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "audio");
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["live_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["live_rows"][0]["assetId"] == "tab_switch_audio");
}

TEST_CASE("DiagnosticsWorkspace - Revealing a hidden snapshot-backed tab refreshes exported detail without manual render",
          "[editor][diagnostics][integration][visible_switch_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Audio);
    workspace.setVisible(false);

    urpg::audio::AudioCore audioCore;
    audioCore.playSound("visible_switch_audio", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(audioCore);

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "audio");
    REQUIRE(exported["visible"] == false);

    workspace.setVisible(true);
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["visible"] == true);
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["live_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["live_rows"][0]["assetId"] == "visible_switch_audio");
}

TEST_CASE("DiagnosticsWorkspace - Event authority workflow actions are exposed at workspace level",
          "[editor][diagnostics][integration][event_authority_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::EventAuthority);

    workspace.ingestEventAuthorityDiagnosticsJsonl(
        "{\"ts\":\"2026-03-04T00:00:02Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_alpha\",\"block_id\":\"blk_alpha\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"alpha\"}\n"
        "{\"ts\":\"2026-03-04T00:00:03Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_beta\",\"block_id\":\"blk_beta\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"beta\"}"
    );

    REQUIRE(workspace.setEventAuthorityEventIdFilter("evt_beta"));
    REQUIRE(workspace.selectEventAuthorityRow(0));

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["event_id_filter"] == "evt_beta");
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 1);
    REQUIRE(exported["active_tab_detail"]["selected_row"]["event_id"] == "evt_beta");

    REQUIRE(workspace.setEventAuthorityEventIdFilter(""));
    REQUIRE(workspace.selectNextEventAuthorityRow());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_row"]["event_id"] == "evt_alpha");
    REQUIRE(exported["active_tab_detail"]["can_select_next_row"] == true);

    REQUIRE(workspace.selectNextEventAuthorityRow());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_row"]["event_id"] == "evt_beta");
    REQUIRE(exported["active_tab_detail"]["can_select_next_row"] == false);
    REQUIRE(exported["active_tab_detail"]["can_select_previous_row"] == true);

    REQUIRE(workspace.setEventAuthorityLevelFilter("error"));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["level_filter"] == "error");
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 1);
    REQUIRE(exported["active_tab_detail"]["visible_row_entries"][0]["event_id"] == "evt_beta");

    REQUIRE(workspace.clearEventAuthorityFilters());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["event_id_filter"] == "");
    REQUIRE(exported["active_tab_detail"]["level_filter"] == "");
    REQUIRE(exported["active_tab_detail"]["mode_filter"] == "");
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 2);
}

TEST_CASE("DiagnosticsWorkspace - Message inspector workflow actions are exposed at workspace level",
          "[editor][diagnostics][integration][message_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {"speaker_a", "Welcome back.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
        {"narration_b", "", urpg::message::variantFromCompatRoute("narration", "Alicia", 3), true, {}, 0},
    });
    urpg::message::RichTextLayoutEngine layout;

    workspace.bindMessageRuntime(runner, layout);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MessageText);
    workspace.update();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "message_text");
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["selected_page_id"].is_null());

    REQUIRE(workspace.setMessageShowIssuesOnly(true));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["page_id"] == "narration_b");

    REQUIRE(workspace.setMessageShowIssuesOnly(false));
    REQUIRE(workspace.setMessageRouteFilter(urpg::message::MessagePresentationMode::Speaker));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["page_id"] == "speaker_a");

    REQUIRE(workspace.selectMessageRow(0));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_page_id"] == "speaker_a");

    REQUIRE(workspace.clearMessageRouteFilter());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["selected_page_id"] == "speaker_a");
}
