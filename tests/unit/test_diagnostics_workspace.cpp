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
    workspace.migrationWizardPanel().onProjectUpdateRequested({
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

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::EventAuthority);
    workspace.render();
    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::EventAuthority).active);
    REQUIRE_FALSE(workspace.compatPanel().isVisible());
    REQUIRE_FALSE(workspace.savePanel().isVisible());
    REQUIRE(workspace.eventAuthorityPanel().isVisible());
    REQUIRE(workspace.eventAuthorityPanel().hasRenderedFrame());
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().visible_rows == 1);
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().warning_count == 1);
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().error_count == 0);
    REQUIRE_FALSE(workspace.messagePanel().isVisible());
    REQUIRE_FALSE(workspace.battlePanel().isVisible());

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MessageText);
    workspace.update();
    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::MessageText).active);
    REQUIRE_FALSE(workspace.compatPanel().isVisible());
    REQUIRE_FALSE(workspace.savePanel().isVisible());
    REQUIRE_FALSE(workspace.eventAuthorityPanel().isVisible());
    REQUIRE(workspace.messagePanel().isVisible());
    REQUIRE_FALSE(workspace.battlePanel().isVisible());

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Battle);
    workspace.update();
    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::Battle).active);
    REQUIRE_FALSE(workspace.compatPanel().isVisible());
    REQUIRE_FALSE(workspace.savePanel().isVisible());
    REQUIRE_FALSE(workspace.eventAuthorityPanel().isVisible());
    REQUIRE_FALSE(workspace.messagePanel().isVisible());
    REQUIRE(workspace.battlePanel().isVisible());

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

    workspace.clearAbilityRuntime();

    const auto clearedAbilitySummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(clearedAbilitySummary.item_count == 0);
    REQUIRE_FALSE(clearedAbilitySummary.has_data);
    REQUIRE(workspace.abilityPanel().getModel().getActiveTags().empty());

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
