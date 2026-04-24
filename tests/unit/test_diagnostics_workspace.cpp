#include "editor/diagnostics/diagnostics_workspace.h"
#include "editor/diagnostics/diagnostics_facade.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/battle/battle_core.h"
#include "engine/core/input/input_core.h"
#include "engine/core/message/message_core.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_scene_graph.h"

#include "runtimes/compat_js/data_manager.h"
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

class SceneBoundWorkspaceAbility final : public urpg::ability::GameplayAbility {
public:
    SceneBoundWorkspaceAbility() {
        id = "skill.live_scene";
        mpCost = 5.0f;
        cooldownTime = 3.0f;
    }

    const std::string& getId() const override { return id; }
    const ActivationInfo& getActivationInfo() const override { return info_; }

    void activate(urpg::ability::AbilitySystemComponent& source) override {
        commitAbility(source);
    }

private:
    ActivationInfo info_;
};

urpg::scene::BattleParticipant* findParticipant(std::vector<urpg::scene::BattleParticipant>& participants, bool is_enemy) {
    for (auto& participant : participants) {
        if (participant.isEnemy == is_enemy) {
            return &participant;
        }
    }
    return nullptr;
}

urpg::message::DialoguePage makeDialoguePage(std::string id,
                                             std::string body,
                                             urpg::message::MessagePresentationVariant variant,
                                             bool wait_for_advance = true,
                                             std::vector<urpg::message::ChoiceOption> choices = {},
                                             int32_t default_choice_index = 0) {
    urpg::message::DialoguePage page;
    page.id = std::move(id);
    page.body = std::move(body);
    page.variant = std::move(variant);
    page.wait_for_advance = wait_for_advance;
    page.choices = std::move(choices);
    page.default_choice_index = default_choice_index;
    return page;
}

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
        makeDialoguePage("speaker_a", "Welcome back.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3)),
        makeDialoguePage("narration_b", "", urpg::message::variantFromCompatRoute("narration", "Alicia", 3)),
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
    workspace.bindProjectAuditReport(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"headline", "Workspace audit"},
        {"summary", "One release blocker and one export blocker."},
        {"releaseBlockerCount", 1},
        {"exportBlockerCount", 1},
        {"assetGovernanceIssueCount", 2},
        {"schemaGovernanceIssueCount", 1},
        {"projectArtifactIssueCount", 3},
        {"localizationEvidenceIssueCount", 1},
        {"accessibilityArtifactIssueCount", 1},
        {"audioArtifactIssueCount", 2},
        {"performanceArtifactIssueCount", 3},
        {"releaseSignoffWorkflowIssueCount", 0},
        {"signoffArtifactIssueCount", 1},
        {"templateSpecArtifactIssueCount", 1},
        {"templateContext", {{"id", "jrpg"}, {"status", "PARTIAL"}}},
        {"governance", {
            {"assetReport", {
                {"path", "imports/reports/asset_audit.json"},
                {"available", true},
                {"usable", false},
                {"issueCount", 2},
                {"normalizedCount", 2},
                {"promotedCount", 2},
                {"promotedVisualLaneCount", 1},
                {"promotedAudioLaneCount", 1},
                {"wysiwygSmokeProofCount", 1}
            }},
            {"schema", {
                {"schemaExists", true},
                {"changelogExists", false},
                {"mentionsSchemaVersion", true},
                {"schemaVersion", "2.1.0"}
            }},
            {"projectSchema", {
                {"path", "schemas/project.schema.json"},
                {"available", true},
                {"hasLocalizationSection", true},
                {"hasInputSection", false},
                {"hasExportSection", true}
            }},
            {"localizationEvidence", {
                {"path", "imports/reports/localization/localization_consistency_report.json"},
                {"available", true},
                {"enabled", true},
                {"usable", true},
                {"dependency", "template localization coverage evidence"},
                {"summary", "Checking canonical localization consistency evidence for selected template jrpg."},
                {"status", "missing_keys"},
                {"issueCount", 1},
                {"hasBundles", true},
                {"bundleCount", 2},
                {"missingLocaleCount", 1},
                {"missingKeyCount", 2},
                {"extraKeyCount", 1},
                {"masterLocale", "en"},
                {"bundles", {
                    {
                        {"path", "content/localization/en.json"},
                        {"locale", "en"},
                        {"keyCount", 3}
                    },
                    {
                        {"path", "content/localization/fr.json"},
                        {"locale", "fr"},
                        {"keyCount", 2}
                    }
                }}
            }},
            {"inputArtifacts", {
                {"path", "content/input/input_bindings.json"},
                {"exists", true},
                {"issueCount", 2}
            }},
            {"accessibilityArtifacts", {
                {"path", "content/schemas/accessibility_report.schema.json"},
                {"available", true},
                {"issueCount", 1}
            }},
            {"audioArtifacts", {
                {"path", "content/schemas/audio_mix_presets.schema.json"},
                {"available", true},
                {"issueCount", 2}
            }},
            {"performanceArtifacts", {
                {"path", "docs/presentation/performance_budgets.md"},
                {"available", true},
                {"issueCount", 3}
            }},
            {"releaseSignoffWorkflow", {
                {"path", "docs/RELEASE_SIGNOFF_WORKFLOW.md"},
                {"available", true},
                {"issueCount", 0}
            }},
            {"signoffArtifacts", {
                {"enabled", true},
                {"dependency", "human-review-gated subsystem signoff artifacts"},
                {"summary", "Checking required subsystem signoff artifacts, conservative wording, and structured human-review signoff contracts for governed lanes."},
                {"available", true},
                {"issueCount", 1},
                {"expectedArtifacts", {
                    {
                        {"subsystemId", "battle_core"},
                        {"title", "Battle Core signoff"},
                        {"path", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"},
                        {"required", true},
                        {"exists", true},
                        {"isRegularFile", true},
                        {"status", "contract_mismatch"},
                        {"wordingOk", true},
                        {"signoffContract", {
                            {"required", true},
                            {"artifactPath", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"},
                            {"promotionRequiresHumanReview", false},
                            {"workflow", "docs/RELEASE_SIGNOFF_WORKFLOW.md"},
                            {"contractOk", false}
                        }}
                    }
                }}
            }},
            {"templateSpecArtifacts", {
                {"enabled", true},
                {"path", "docs/templates/jrpg_spec.md"},
                {"available", true},
                {"issueCount", 1},
                {"expectedArtifacts", {
                    {
                        {"path", "docs/templates/jrpg_spec.md"},
                        {"status", "parity_mismatch"},
                        {"templateIdMatches", true},
                        {"requiredSubsystemsMatch", false},
                        {"barsMatch", false},
                        {"missingRequiredSubsystems", {"save_data_core"}},
                        {"unexpectedRequiredSubsystems", {"2_5d_mode"}},
                        {"barMismatches", {
                            {
                                {"bar", "accessibility"},
                                {"label", "Accessibility"},
                                {"expectedStatus", "PARTIAL"},
                                {"specStatus", "PLANNED"}
                            }
                        }}
                    }
                }}
            }}
        }},
        {"issues", {
            {
                {"code", "missing_localization_key"},
                {"title", "Missing localization key"},
                {"detail", "menu.start is missing from one locale."},
                {"severity", "warning"},
                {"blocksRelease", true},
                {"blocksExport", false}
            },
            {
                {"code", "missing_input_binding"},
                {"title", "Missing input binding"},
                {"detail", "confirm action is not bound for controller."},
                {"severity", "error"},
                {"blocksRelease", false},
                {"blocksExport", true}
            }
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

    const auto auditSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::ProjectAudit);
    REQUIRE_FALSE(auditSummary.active);
    REQUIRE(auditSummary.item_count == 2);
    REQUIRE(auditSummary.issue_count == 2);
    REQUIRE(auditSummary.has_data);

    const auto allSummaries = workspace.allTabSummaries();
    REQUIRE(allSummaries.size() == 10);

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
    REQUIRE(exportedJson["tabs"].size() == 10);
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
    REQUIRE(exportedJson["tabs"][9]["name"] == "project_audit");

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
    REQUIRE(saveJson["active_tab_detail"]["autosave_policy"]["enabled"] == true);
    REQUIRE(saveJson["active_tab_detail"]["autosave_policy"]["slot_id"] == 0);
    REQUIRE(saveJson["active_tab_detail"]["retention_policy"]["max_manual_slots"] == 20);
    REQUIRE(saveJson["active_tab_detail"]["retention_policy"]["prune_excess_on_save"] == true);
    REQUIRE(saveJson["active_tab_detail"]["recovery_diagnostics"]["total_recovery_slots"] == 0);
    REQUIRE(saveJson["active_tab_detail"]["recovery_diagnostics"]["safe_mode_recovery_slots"] == 0);
    REQUIRE(saveJson["active_tab_detail"]["serialization_schema"]["format_magic"] == "URSV");
    REQUIRE(saveJson["active_tab_detail"]["serialization_schema"]["differential_supported"] == true);
    REQUIRE(saveJson["active_tab_detail"]["serialization_schema"]["compression_modes"].size() == 3);
    REQUIRE(saveJson["active_tab_detail"]["metadata_fields"].is_array());
    REQUIRE(saveJson["active_tab_detail"]["metadata_fields"].size() == 0);
    REQUIRE(saveJson["active_tab_detail"]["slot_descriptors"].is_array());
    REQUIRE(saveJson["active_tab_detail"]["slot_descriptors"].size() == 0);
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
    REQUIRE(workspace.messagePanel().hasRenderedFrame());
    REQUIRE(workspace.messagePanel().lastRenderSnapshot().has_data);
    REQUIRE(workspace.messagePanel().lastRenderSnapshot().total_pages == 2);
    REQUIRE(workspace.messagePanel().lastRenderSnapshot().visible_row_count == 2);
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

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::ProjectAudit);
    workspace.render();
    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::ProjectAudit).active);
    REQUIRE_FALSE(workspace.compatPanel().isVisible());
    REQUIRE_FALSE(workspace.savePanel().isVisible());
    REQUIRE_FALSE(workspace.eventAuthorityPanel().isVisible());
    REQUIRE_FALSE(workspace.messagePanel().isVisible());
    REQUIRE_FALSE(workspace.battlePanel().isVisible());
    REQUIRE(workspace.projectAuditPanel().isVisible());
    REQUIRE(workspace.projectAuditPanel().hasRenderedFrame());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().headline == "Workspace audit");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().issue_count == 2);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().release_blocker_count == 1);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().export_blocker_count == 1);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_id == "jrpg");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_status == "PARTIAL");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().asset_governance_issue_count == 2);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().schema_governance_issue_count == 1);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().project_artifact_issue_count == 3);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().localization_evidence_issue_count == 1);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().accessibility_artifact_issue_count == 1);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().audio_artifact_issue_count == 2);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().performance_artifact_issue_count == 3);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().release_signoff_workflow_issue_count == 0);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().signoff_artifact_issue_count == 1);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_spec_artifact_issue_count == 1);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().asset_report.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().schema_governance.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().project_schema_governance.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().input_artifacts.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().input_artifacts->path ==
            "content/input/input_bindings.json");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().localization_evidence.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().localization_evidence->path ==
            "imports/reports/localization/localization_consistency_report.json");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().localization_evidence->usable == true);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().localization_evidence->status == "missing_keys");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().localization_evidence->missing_key_count == 2);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().localization_evidence->bundles.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().accessibility_artifacts.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().accessibility_artifacts->path ==
            "content/schemas/accessibility_report.schema.json");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().audio_artifacts.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().audio_artifacts->path ==
            "content/schemas/audio_mix_presets.schema.json");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().performance_artifacts.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().performance_artifacts->path ==
            "docs/presentation/performance_budgets.md");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().release_signoff_workflow.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().release_signoff_workflow->path ==
            "docs/RELEASE_SIGNOFF_WORKFLOW.md");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().signoff_artifacts.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().signoff_artifacts->issue_count == 1);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().signoff_artifacts->expected_artifacts.size() == 1);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().signoff_artifacts->expected_artifacts[0].subsystem_id ==
            "battle_core");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().signoff_artifacts->expected_artifacts[0].signoff_contract.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().signoff_artifacts->expected_artifacts[0].signoff_contract->contract_ok ==
            false);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_spec_artifacts.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_spec_artifacts->path == "docs/templates/jrpg_spec.md");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_spec_artifacts->expected_artifacts.size() == 1);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_spec_artifacts->expected_artifacts[0].required_subsystems_match ==
            false);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_spec_artifacts->expected_artifacts[0].bars_match ==
            false);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().issues.size() == 2);
    const auto auditJson = nlohmann::json::parse(facade.emitSnapshot());
    REQUIRE(auditJson["active_tab"] == "project_audit");
    REQUIRE(auditJson["active_tab_detail"]["tab"] == "project_audit");
    REQUIRE(auditJson["active_tab_detail"]["headline"] == "Workspace audit");
    REQUIRE(auditJson["active_tab_detail"]["summary_text"] == "One release blocker and one export blocker.");
    REQUIRE(auditJson["active_tab_detail"]["issue_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["release_blocker_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["export_blocker_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["template_id"] == "jrpg");
    REQUIRE(auditJson["active_tab_detail"]["template_status"] == "PARTIAL");
    REQUIRE(auditJson["active_tab_detail"]["asset_governance_issue_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["schema_governance_issue_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["project_artifact_issue_count"] == 3);
    REQUIRE(auditJson["active_tab_detail"]["localization_evidence_issue_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["accessibility_artifact_issue_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["audio_artifact_issue_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["performance_artifact_issue_count"] == 3);
    REQUIRE(auditJson["active_tab_detail"]["release_signoff_workflow_issue_count"] == 0);
    REQUIRE(auditJson["active_tab_detail"]["signoff_artifact_issue_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["template_spec_artifact_issue_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["asset_report"]["path"] == "imports/reports/asset_audit.json");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["asset_report"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["asset_report"]["usable"] == false);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["asset_report"]["issue_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["asset_report"]["normalized_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["asset_report"]["promoted_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["asset_report"]["promoted_visual_lane_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["asset_report"]["promoted_audio_lane_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["asset_report"]["wysiwyg_smoke_proof_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["schema"]["schema_exists"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["schema"]["changelog_exists"] == false);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["schema"]["mentions_schema_version"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["schema"]["schema_version"] == "2.1.0");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["project_schema"]["path"] == "schemas/project.schema.json");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["project_schema"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["project_schema"]["has_localization_section"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["project_schema"]["has_input_section"] == false);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["project_schema"]["has_export_section"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["path"] ==
            "imports/reports/localization/localization_consistency_report.json");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["enabled"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["usable"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["status"] == "missing_keys");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["bundle_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["missing_locale_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["missing_key_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["extra_key_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["master_locale"] == "en");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["bundles"].is_array());
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_evidence"]["bundles"][0]["locale"] == "en");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["input_artifacts"]["path"] ==
            "content/input/input_bindings.json");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["input_artifacts"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["input_artifacts"]["issue_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["accessibility_artifacts"]["path"] ==
            "content/schemas/accessibility_report.schema.json");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["accessibility_artifacts"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["accessibility_artifacts"]["issue_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["audio_artifacts"]["path"] ==
            "content/schemas/audio_mix_presets.schema.json");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["audio_artifacts"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["audio_artifacts"]["issue_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["performance_artifacts"]["path"] ==
            "docs/presentation/performance_budgets.md");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["performance_artifacts"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["performance_artifacts"]["issue_count"] == 3);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["release_signoff_workflow"]["path"] ==
            "docs/RELEASE_SIGNOFF_WORKFLOW.md");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["release_signoff_workflow"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["release_signoff_workflow"]["issue_count"] == 0);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["issue_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["enabled"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["dependency"] ==
            "human-review-gated subsystem signoff artifacts");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["expected_artifacts"].is_array());
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["expected_artifacts"].size() == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["expected_artifacts"][0]["subsystem_id"] ==
            "battle_core");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["expected_artifacts"][0]["status"] ==
            "contract_mismatch");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["expected_artifacts"][0]["signoff_contract"]["artifact_path"] ==
            "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["expected_artifacts"][0]["signoff_contract"]["contract_ok"] ==
            false);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["path"] == "docs/templates/jrpg_spec.md");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["issue_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["enabled"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"].is_array());
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"].size() == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]["status"] ==
            "parity_mismatch");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]["required_subsystems_match"] ==
            false);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]["bars_match"] ==
            false);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]["missing_required_subsystems"][0] ==
            "save_data_core");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]["unexpected_required_subsystems"][0] ==
            "2_5d_mode");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]["bar_mismatches"][0]["bar"] ==
            "accessibility");
    REQUIRE(auditJson["active_tab_detail"]["issues"].is_array());
    REQUIRE(auditJson["active_tab_detail"]["issues"].size() == 2);
    REQUIRE(auditJson["active_tab_detail"]["issues"][0]["code"] == "missing_localization_key");
    REQUIRE(auditJson["active_tab_detail"]["issues"][0]["blocks_release"] == true);
    REQUIRE(auditJson["active_tab_detail"]["issues"][1]["code"] == "missing_input_binding");
    REQUIRE(auditJson["active_tab_detail"]["issues"][1]["blocks_export"] == true);
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

TEST_CASE("DiagnosticsWorkspace - Project audit derives blocker counts from issue flags",
          "[editor][diagnostics][integration][project_audit]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindProjectAuditReport(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"headline", "Fallback audit"},
        {"summary", "Derived blocker counts should come from issue flags."},
        {"templateContext", {{"id", "jrpg"}, {"status", "PARTIAL"}}},
        {"issues", {
            {
                {"code", "missing_localization_key"},
                {"title", "Missing localization key"},
                {"detail", "menu.start is missing from one locale."},
                {"severity", "warning"},
                {"blocksRelease", true},
                {"blocksExport", false}
            },
            {
                {"code", "missing_input_binding"},
                {"title", "Missing input binding"},
                {"detail", "confirm action is not bound for controller."},
                {"severity", "error"},
                {"blocksRelease", false},
                {"blocksExport", true}
            },
            {
                {"code", "missing_console_profile"},
                {"title", "Missing console profile"},
                {"detail", "console export profile is incomplete."},
                {"severity", "error"},
                {"blocksRelease", true},
                {"blocksExport", true}
            }
        }},
        {"governance", {
            {"localizationArtifacts", {
                {"path", "content/localization/en-US.json"},
                {"available", true},
                {"issueCount", 1}
            }},
            {"exportArtifacts", {
                {"path", "exports/project_audit/export_manifest.json"},
                {"present", true},
                {"issueCount", 2}
            }}
        }}
    });

    workspace.refresh();

    const auto auditSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::ProjectAudit);
    REQUIRE_FALSE(auditSummary.active);
    REQUIRE(auditSummary.item_count == 3);
    REQUIRE(auditSummary.issue_count == 4);
    REQUIRE(auditSummary.has_data);

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::ProjectAudit);
    workspace.render();

    REQUIRE(workspace.projectAuditPanel().isVisible());
    REQUIRE(workspace.projectAuditPanel().hasRenderedFrame());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().headline == "Fallback audit");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().issue_count == 3);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().release_blocker_count == 2);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().export_blocker_count == 2);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().issues.size() == 3);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().localization_artifacts.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().export_artifacts.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().localization_artifacts->path ==
            "content/localization/en-US.json");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().export_artifacts->path ==
            "exports/project_audit/export_manifest.json");

    urpg::editor::DiagnosticsFacade facade(workspace);
    const auto auditJson = nlohmann::json::parse(facade.emitSnapshot());
    REQUIRE(auditJson["active_tab"] == "project_audit");
    REQUIRE(auditJson["active_tab_detail"]["tab"] == "project_audit");
    REQUIRE(auditJson["active_tab_detail"]["headline"] == "Fallback audit");
    REQUIRE(auditJson["active_tab_detail"]["issue_count"] == 3);
    REQUIRE(auditJson["active_tab_detail"]["release_blocker_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["export_blocker_count"] == 2);
    REQUIRE(auditJson["active_tab_detail"]["issues"].is_array());
    REQUIRE(auditJson["active_tab_detail"]["issues"].size() == 3);
    REQUIRE(auditJson["active_tab_detail"]["issues"][0]["blocks_release"] == true);
    REQUIRE(auditJson["active_tab_detail"]["issues"][0]["blocks_export"] == false);
    REQUIRE(auditJson["active_tab_detail"]["issues"][1]["blocks_release"] == false);
    REQUIRE(auditJson["active_tab_detail"]["issues"][1]["blocks_export"] == true);
    REQUIRE(auditJson["active_tab_detail"]["issues"][2]["blocks_release"] == true);
    REQUIRE(auditJson["active_tab_detail"]["issues"][2]["blocks_export"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_artifacts"]["path"] ==
            "content/localization/en-US.json");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_artifacts"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["localization_artifacts"]["issue_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["export_artifacts"]["path"] ==
            "exports/project_audit/export_manifest.json");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["export_artifacts"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["export_artifacts"]["issue_count"] == 2);
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

    urpg::ui::MenuRouteResolver resolver;
    resolver.bindRoute(urpg::MenuRouteTarget::Item, [](const urpg::MenuCommandMeta&) {});
    graph.setRouteResolver(&resolver);

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
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["has_data"] == true);
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["active_scene_id"] == "MainMenu");
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["visible_panes"].is_array());
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["visible_panes"].size() == 1);
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["visible_panes"][0]["pane_id"] == "main_pane");
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["visible_panes"][0]["selected_command_id"] == "urpg.menu.item");

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

TEST_CASE("DiagnosticsWorkspace - Menu workflow actions are exposed at workspace level",
          "[editor][diagnostics][integration][menu_actions]") {
    urpg::ui::MenuCommandRegistry registry;

    urpg::MenuCommandMeta itemCommand;
    itemCommand.id = "urpg.menu.item";
    itemCommand.label = "Item";
    itemCommand.route = urpg::MenuRouteTarget::Item;
    registry.registerCommand(itemCommand);

    urpg::MenuCommandMeta hiddenCommand;
    hiddenCommand.id = "urpg.menu.hidden";
    hiddenCommand.label = "Hidden";
    hiddenCommand.route = urpg::MenuRouteTarget::Options;
    hiddenCommand.visibility_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "",
            .variable_id = "secret_level",
            .variable_threshold = 1,
            .invert = false,
        },
    };
    registry.registerCommand(hiddenCommand);

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
    pane.commands = {itemCommand, hiddenCommand, deadEndCommand};

    menu->addPane(pane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("MainMenu");

    urpg::ui::MenuRouteResolver resolver;
    resolver.bindRoute(urpg::MenuRouteTarget::Item, [](const urpg::MenuCommandMeta&) {});
    graph.setRouteResolver(&resolver);

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindMenuRuntime(graph, registry, switches, variables);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Menu);
    workspace.update();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "menu");
    REQUIRE(exported["active_tab_detail"]["selected_command_id"].is_null());
    REQUIRE(exported["active_tab_detail"]["command_id_filter"] == "");
    REQUIRE(exported["active_tab_detail"]["show_issues_only"] == false);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);

    REQUIRE(workspace.setMenuCommandIdFilter("hidden"));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["command_id_filter"] == "hidden");
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["command_id"] == "urpg.menu.hidden");

    REQUIRE(workspace.selectMenuRow(0));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_command_id"] == "urpg.menu.hidden");
    REQUIRE(exported["active_tab_detail"]["selected_row"]["command_id"] == "urpg.menu.hidden");
    REQUIRE(exported["active_tab_detail"]["selected_row"]["command_visible"] == false);
    REQUIRE(exported["active_tab_detail"]["selected_row"]["issue_count"].get<size_t>() >= 1);
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][0]["selected_command_id"] == "urpg.menu.hidden");

    REQUIRE(workspace.clearMenuCommandIdFilter());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["command_id_filter"] == "");
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["selected_command_id"] == "urpg.menu.hidden");

    REQUIRE(workspace.setMenuShowIssuesOnly(true));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["show_issues_only"] == true);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);

    REQUIRE(workspace.setMenuShowIssuesOnly(false));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["show_issues_only"] == false);
}

TEST_CASE("DiagnosticsWorkspace - Save workflow actions are exposed at workspace level",
          "[editor][diagnostics][integration][save_actions]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_diagnostics_workspace_save_actions";
    std::filesystem::create_directories(base);

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);
    coordinator.setAutosavePolicy({true, 0});
    coordinator.setRetentionPolicy({1, 2, 20, true});
    coordinator.metadataRegistry().registerField({"difficulty", "Difficulty", false, "Normal"});

    urpg::SaveCatalogEntry autosave;
    autosave.meta.slot_id = 0;
    autosave.meta.flags.autosave = true;
    autosave.meta.category = urpg::SaveSlotCategory::Autosave;
    autosave.meta.retention_class = urpg::SaveRetentionClass::Autosave;
    autosave.meta.map_display_name = "Autosave Camp";
    autosave.last_operation = "save";
    REQUIRE(catalog.upsert(autosave));

    urpg::SaveCatalogEntry problem;
    problem.meta.slot_id = 6;
    problem.meta.category = urpg::SaveSlotCategory::Manual;
    problem.meta.retention_class = urpg::SaveRetentionClass::Manual;
    problem.meta.flags.corrupted = true;
    problem.meta.map_display_name = "Broken Tower";
    problem.last_operation = "load";
    problem.last_recovery_tier = urpg::SaveRecoveryTier::Level3SafeSkeleton;
    problem.diagnostic = "safe_mode_triggered";
    REQUIRE(catalog.upsert(problem));

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindSaveRuntime(catalog, coordinator);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Save);
    workspace.update();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "save");
    REQUIRE(exported["active_tab_detail"]["selected_slot_id"].is_null());
    REQUIRE(exported["active_tab_detail"]["show_problem_slots_only"] == false);
    REQUIRE(exported["active_tab_detail"]["include_autosave"] == true);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["autosave_enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["autosave_slot_id"] == 0);
    REQUIRE(exported["active_tab_detail"]["policy_validation"]["can_apply"] == true);
    REQUIRE(exported["active_tab_detail"]["policy_issues"].size() == 0);
    REQUIRE(exported["active_tab_detail"]["metadata_fields"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);

    REQUIRE(workspace.setSaveShowProblemSlotsOnly(true));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["show_problem_slots_only"] == true);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["slot_id"] == 6);

    REQUIRE(workspace.selectSaveRow(0));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_slot_id"] == 6);
    REQUIRE(exported["active_tab_detail"]["selected_row"]["slot_id"] == 6);
    REQUIRE(exported["active_tab_detail"]["selected_row"]["boot_safe_mode"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_row"]["diagnostic"] == "safe_mode_triggered");
    REQUIRE(exported["active_tab_detail"]["recovery_diagnostics"]["total_recovery_slots"] == 1);
    REQUIRE(exported["active_tab_detail"]["recovery_diagnostics"]["safe_mode_recovery_slots"] == 1);

    REQUIRE(workspace.setSaveIncludeAutosave(false));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["include_autosave"] == false);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["selected_slot_id"] == 6);

    REQUIRE(workspace.setSaveShowProblemSlotsOnly(false));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["show_problem_slots_only"] == false);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["slot_id"] == 6);

    REQUIRE(workspace.setSavePolicyAutosaveEnabled(false));
    REQUIRE(workspace.setSavePolicyAutosaveSlotId(3));
    REQUIRE(workspace.setSavePolicyRetentionLimits(2, 4, 18, false));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["autosave_enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["autosave_slot_id"] == 3);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["max_autosave_slots"] == 2);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["max_quicksave_slots"] == 4);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["max_manual_slots"] == 18);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["prune_excess_on_save"] == false);
    REQUIRE(exported["active_tab_detail"]["policy_validation"]["can_apply"] == true);
    REQUIRE(workspace.applySavePolicyChanges());
    REQUIRE(coordinator.autosavePolicy().enabled == false);
    REQUIRE(coordinator.autosavePolicy().slot_id == 3);
    REQUIRE(coordinator.retentionPolicy().max_autosave_slots == 2);
    REQUIRE(coordinator.retentionPolicy().max_quicksave_slots == 4);
    REQUIRE(coordinator.retentionPolicy().max_manual_slots == 18);
    REQUIRE(coordinator.retentionPolicy().prune_excess_on_save == false);

    REQUIRE(workspace.setSavePolicyAutosaveEnabled(true));
    REQUIRE(workspace.setSavePolicyAutosaveSlotId(-1));
    REQUIRE(workspace.setSavePolicyRetentionLimits(0, 4, 18, false));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["policy_validation"]["can_apply"] == false);
    REQUIRE(exported["active_tab_detail"]["policy_validation"]["error_count"] == 2);
    REQUIRE(exported["active_tab_detail"]["policy_issues"].size() == 2);
    REQUIRE_FALSE(workspace.applySavePolicyChanges());
    REQUIRE(coordinator.autosavePolicy().enabled == false);
    REQUIRE(coordinator.autosavePolicy().slot_id == 3);

    std::filesystem::remove_all(base);
}

TEST_CASE("DiagnosticsWorkspace - Menu preview actions drive runtime selection and blocked-command export",
          "[editor][diagnostics][integration][menu_preview_actions]") {
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

    urpg::ui::MenuPane mainPane;
    mainPane.id = "main_pane";
    mainPane.displayName = "Main Pane";
    mainPane.isVisible = true;
    mainPane.isActive = true;
    mainPane.commands = {itemCommand, deadEndCommand};

    urpg::ui::MenuPane sidePane;
    sidePane.id = "side_pane";
    sidePane.displayName = "Side Pane";
    sidePane.isVisible = true;
    sidePane.isActive = false;
    sidePane.commands = {itemCommand};

    menu->addPane(mainPane);
    menu->addPane(sidePane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("MainMenu");

    urpg::ui::MenuRouteResolver resolver;
    resolver.bindRoute(urpg::MenuRouteTarget::Item, [](const urpg::MenuCommandMeta&) {});
    graph.setRouteResolver(&resolver);

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindMenuRuntime(graph, registry, switches, variables);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Menu);
    workspace.update();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][0]["selected_command_id"] == "urpg.menu.item");
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][0]["pane_active"] == true);
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][1]["pane_active"] == false);
    REQUIRE(exported["active_tab_detail"]["preview"]["last_blocked_command_id"] == "");
    REQUIRE(exported["active_tab_detail"]["preview"]["last_blocked_reason"] == "");

    REQUIRE(workspace.dispatchMenuPreviewAction(urpg::input::InputAction::MoveDown));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][0]["selected_command_id"] == "urpg.menu.dead_end");
    REQUIRE(exported["active_tab_detail"]["selected_command_id"] == "urpg.menu.dead_end");
    REQUIRE(exported["active_tab_detail"]["selected_row"]["command_id"] == "urpg.menu.dead_end");

    REQUIRE(workspace.dispatchMenuPreviewAction(urpg::input::InputAction::Confirm));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["preview"]["last_blocked_command_id"] == "urpg.menu.dead_end");
    REQUIRE(exported["active_tab_detail"]["preview"]["last_blocked_reason"] == "No route resolved for command.");

    REQUIRE(workspace.dispatchMenuPreviewAction(urpg::input::InputAction::MoveRight));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][0]["pane_active"] == false);
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][1]["pane_active"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_command_id"] == "urpg.menu.item");
    REQUIRE(exported["active_tab_detail"]["selected_row"]["pane_id"] == "side_pane");
    REQUIRE(exported["active_tab_detail"]["selected_row"]["command_id"] == "urpg.menu.item");
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

TEST_CASE("DiagnosticsWorkspace - ability tab follows live scene-owned ASC updates",
          "[editor][diagnostics][integration][abilities][scene]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    urpg::scene::BattleScene battle({"1"});
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<urpg::scene::BattleParticipant>&>(battle.getParticipants());
    auto* hero = findParticipant(participants, false);
    REQUIRE(hero != nullptr);

    hero->abilitySystem.setAttribute("MP", 20.0f);
    auto liveAbility = std::make_shared<SceneBoundWorkspaceAbility>();
    hero->abilitySystem.grantAbility(liveAbility);

    workspace.bindAbilityRuntime(hero->abilitySystem);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.update();

    const auto before = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(before["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(before["active_tab_detail"]["abilities"][0]["name"] == "skill.live_scene");
    REQUIRE(before["active_tab_detail"]["abilities"][0]["can_activate"] == true);

    REQUIRE(hero->abilitySystem.tryActivateAbility(*liveAbility));
    workspace.update();
    workspace.render();

    const auto after = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(after["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(after["active_tab_detail"]["abilities"][0]["name"] == "skill.live_scene");
    REQUIRE(after["active_tab_detail"]["abilities"][0]["can_activate"] == false);
    REQUIRE(after["active_tab_detail"]["abilities"][0]["cooldown_remaining"].get<float>() > 0.0f);

    const auto snapshot = workspace.abilityPanel().getRenderSnapshot();
    REQUIRE(snapshot.latest_ability_id == "skill.live_scene");
    REQUIRE(snapshot.diagnostic_count == 1);
}

namespace {

class WorkspacePreviewAbility final : public urpg::ability::GameplayAbility {
public:
    WorkspacePreviewAbility(std::string ability_id, float cooldown_seconds, float mp_cost)
        : id_(std::move(ability_id)) {
        info_.cooldownSeconds = cooldown_seconds;
        info_.mpCost = static_cast<int32_t>(mp_cost);
    }

    const std::string& getId() const override { return id_; }
    const ActivationInfo& getActivationInfo() const override { return info_; }

    void activate(urpg::ability::AbilitySystemComponent& source) override {
        commitAbility(source);
    }

private:
    std::string id_;
    ActivationInfo info_;
};

} // namespace

TEST_CASE("DiagnosticsWorkspace - ability workflow actions expose selection and live preview activation",
          "[editor][diagnostics][integration][abilities][actions]") {
    urpg::ability::AbilitySystemComponent asc;
    asc.setAttribute("MP", 20.0f);
    asc.grantAbility(std::make_shared<WorkspacePreviewAbility>("skill.preview_fire", 3.0f, 5.0f));
    asc.grantAbility(std::make_shared<WorkspacePreviewAbility>("skill.preview_ice", 0.0f, 2.0f));

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindAbilityRuntime(asc);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.update();

    REQUIRE(workspace.selectAbilityRow(0));
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "abilities");
    REQUIRE(exported["active_tab_detail"]["selected_ability_id"] == "skill.preview_fire");
    REQUIRE(exported["active_tab_detail"]["selected_ability_can_activate"] == true);
    REQUIRE(exported["active_tab_detail"]["can_preview_selected_ability"] == true);

    REQUIRE(workspace.previewSelectedAbility());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["latest_ability_id"] == "skill.preview_fire");
    REQUIRE(exported["active_tab_detail"]["latest_outcome"] == "executed");
    REQUIRE(exported["active_tab_detail"]["diagnostic_count"] == 1);
    REQUIRE(exported["active_tab_detail"]["diagnostic_lines"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["selected_ability_can_activate"] == false);
    REQUIRE(exported["active_tab_detail"]["selected_ability_blocking_reason"].get<std::string>().find("Cooldown") != std::string::npos);
    REQUIRE(asc.getAttribute("MP", 0.0f) == 15.0f);

    REQUIRE(workspace.selectAbilityRow(1));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_ability_id"] == "skill.preview_ice");
    REQUIRE(exported["active_tab_detail"]["selected_ability_can_activate"] == true);
}

TEST_CASE("DiagnosticsWorkspace - ability draft editing drives owned preview runtime and pattern/effect export",
          "[editor][diagnostics][integration][abilities][draft]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.beginAbilityDraftPreview();

    REQUIRE(workspace.setAbilityDraftId("skill.editor_authored"));
    REQUIRE(workspace.setAbilityDraftCooldownSeconds(4.0f));
    REQUIRE(workspace.setAbilityDraftMpCost(7.0f));
    REQUIRE(workspace.setAbilityDraftEffectId("effect.editor_guard"));
    REQUIRE(workspace.setAbilityDraftEffectAttribute("Defense"));
    REQUIRE(workspace.setAbilityDraftEffectValue(18.0f));
    REQUIRE(workspace.setAbilityDraftEffectDuration(9.0f));
    REQUIRE(workspace.applyAbilityDraftPatternPreset("skill_cross_small"));
    REQUIRE(workspace.setAbilityDraftPatternName("Editor Guard Pattern"));
    REQUIRE(workspace.toggleAbilityDraftPatternPoint(1, 1));

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "abilities");
    REQUIRE(exported["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["name"] == "skill.editor_authored");
    REQUIRE(exported["active_tab_detail"]["selected_ability_id"] == "skill.editor_authored");
    REQUIRE(exported["active_tab_detail"]["draft"]["ability_id"] == "skill.editor_authored");
    REQUIRE(exported["active_tab_detail"]["draft"]["effect_attribute"] == "Defense");
    REQUIRE(exported["active_tab_detail"]["draft"]["effect_value"] == 18.0);
    REQUIRE(exported["active_tab_detail"]["draft"]["preview_mp_before"] == 30.0);
    REQUIRE(exported["active_tab_detail"]["draft"]["preview_mp_after"] == 23.0);
    REQUIRE(exported["active_tab_detail"]["draft"]["preview_attribute_before"] == 100.0);
    REQUIRE(exported["active_tab_detail"]["draft"]["preview_attribute_after"] == 118.0);
    REQUIRE(exported["active_tab_detail"]["draft"]["pattern_preview"]["name"] == "Editor Guard Pattern");
    REQUIRE(exported["active_tab_detail"]["draft"]["pattern_preview"]["is_valid"] == true);
    REQUIRE(exported["active_tab_detail"]["draft"]["pattern_preview"]["grid_rows"][2].get<std::string>().find("[O]") != std::string::npos);

    REQUIRE(workspace.previewSelectedAbility());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["latest_ability_id"] == "skill.editor_authored");
    REQUIRE(exported["active_tab_detail"]["latest_outcome"] == "executed");
    REQUIRE(exported["active_tab_detail"]["diagnostic_count"] == 1);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["can_activate"] == false);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["blocking_reason"].get<std::string>().find("Cooldown") != std::string::npos);
}

TEST_CASE("DiagnosticsWorkspace - ability draft state save and load round-trip",
          "[editor][diagnostics][integration][abilities][draft_io]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.beginAbilityDraftPreview();

    REQUIRE(workspace.setAbilityDraftId("skill.saved_authored"));
    REQUIRE(workspace.setAbilityDraftCooldownSeconds(6.0f));
    REQUIRE(workspace.setAbilityDraftMpCost(9.0f));
    REQUIRE(workspace.setAbilityDraftEffectId("effect.saved_guard"));
    REQUIRE(workspace.setAbilityDraftEffectAttribute("MagicDefense"));
    REQUIRE(workspace.setAbilityDraftEffectOperation(urpg::ModifierOp::Override));
    REQUIRE(workspace.setAbilityDraftEffectValue(42.0f));
    REQUIRE(workspace.setAbilityDraftEffectDuration(11.0f));
    REQUIRE(workspace.applyAbilityDraftPatternPreset("skill_cross_small"));
    REQUIRE(workspace.setAbilityDraftPatternName("Saved Guard Pattern"));

    const auto exportedDraft = nlohmann::json::parse(workspace.exportAbilityDraftStateJson());
    REQUIRE(exportedDraft["ability_id"] == "skill.saved_authored");
    REQUIRE(exportedDraft["effect_attribute"] == "MagicDefense");
    REQUIRE(exportedDraft["effect_operation"] == "Override");
    REQUIRE(exportedDraft["pattern"]["name"] == "Saved Guard Pattern");

    const auto temp_path = (std::filesystem::temp_directory_path() / "urpg_workspace_ability_draft.json").string();
    std::filesystem::remove(temp_path);
    REQUIRE(workspace.saveAbilityDraftStateToFile(temp_path));
    REQUIRE(std::filesystem::exists(temp_path));

    urpg::editor::DiagnosticsWorkspace loadWorkspace;
    loadWorkspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(loadWorkspace.loadAbilityDraftStateFromFile(temp_path));

    const auto loaded = nlohmann::json::parse(loadWorkspace.exportAsJson());
    REQUIRE(loaded["active_tab_detail"]["draft"]["ability_id"] == "skill.saved_authored");
    REQUIRE(loaded["active_tab_detail"]["draft"]["effect_attribute"] == "MagicDefense");
    REQUIRE(loaded["active_tab_detail"]["draft"]["effect_operation"] == "Override");
    REQUIRE(loaded["active_tab_detail"]["draft"]["effect_value"] == 42.0);
    REQUIRE(loaded["active_tab_detail"]["draft"]["pattern_preview"]["name"] == "Saved Guard Pattern");
    REQUIRE(loaded["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(loaded["active_tab_detail"]["abilities"][0]["name"] == "skill.saved_authored");

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - authored ability can be applied to a live battle participant runtime",
          "[editor][diagnostics][integration][abilities][battle_apply]") {
    urpg::scene::BattleScene battle({"1"});
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<urpg::scene::BattleParticipant>&>(battle.getParticipants());
    auto* hero = findParticipant(participants, false);
    REQUIRE(hero != nullptr);

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.bindAbilityRuntime(hero->abilitySystem);

    workspace.setAbilityDraftId("skill.battle_authored");
    workspace.setAbilityDraftCooldownSeconds(5.0f);
    workspace.setAbilityDraftMpCost(4.0f);
    workspace.setAbilityDraftEffectId("effect.battle_focus");
    workspace.setAbilityDraftEffectValue(12.0f);
    workspace.setAbilityDraftEffectDuration(7.0f);

    REQUIRE(workspace.applyAbilityDraftToRuntime());
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["name"] == "skill.battle_authored");
    REQUIRE(exported["active_tab_detail"]["selected_ability_id"] == "skill.battle_authored");

    hero->abilitySystem.setAttribute("MP", 20.0f);
    hero->abilitySystem.setAttribute("Attack", 100.0f);
    REQUIRE(workspace.previewSelectedAbility());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["latest_ability_id"] == "skill.battle_authored");
    REQUIRE(hero->abilitySystem.getAttribute("MP", 0.0f) == 16.0f);
    REQUIRE(hero->abilitySystem.getAttribute("Attack", 0.0f) == 112.0f);
}

TEST_CASE("DiagnosticsWorkspace - authored ability can be applied to a live map scene runtime",
          "[editor][diagnostics][integration][abilities][map_apply]") {
    urpg::scene::MapScene map("001", 10, 10);

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.bindAbilityRuntime(map.playerAbilitySystem());

    workspace.setAbilityDraftId("skill.map_authored");
    workspace.setAbilityDraftCooldownSeconds(4.0f);
    workspace.setAbilityDraftMpCost(6.0f);
    workspace.setAbilityDraftEffectId("effect.map_guard");
    workspace.setAbilityDraftEffectAttribute("Defense");
    workspace.setAbilityDraftEffectValue(15.0f);
    workspace.setAbilityDraftEffectDuration(8.0f);
    workspace.applyAbilityDraftPatternPreset("skill_cross_small");
    workspace.setAbilityDraftPatternName("Map Guard Pattern");

    REQUIRE(workspace.applyAbilityDraftToRuntime());
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["name"] == "skill.map_authored");
    REQUIRE(exported["active_tab_detail"]["draft"]["pattern_preview"]["name"] == "Map Guard Pattern");

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("Defense", 100.0f);
    REQUIRE(workspace.previewSelectedAbility());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["latest_ability_id"] == "skill.map_authored");
    REQUIRE(map.playerAbilitySystem().getAttribute("MP", 0.0f) == 24.0f);
    REQUIRE(map.playerAbilitySystem().getAttribute("Defense", 0.0f) == 115.0f);
}

TEST_CASE("DiagnosticsWorkspace - project ability content is discoverable and loadable through the picker",
          "[editor][diagnostics][integration][abilities][content_picker]") {
    const auto project_root = std::filesystem::temp_directory_path() / "urpg_workspace_ability_content_picker";
    std::filesystem::remove_all(project_root);
    std::filesystem::create_directories(project_root / "content" / "abilities");

    urpg::ability::AuthoredAbilityAsset fire;
    fire.ability_id = "skill.content_fire";
    fire.effect_id = "effect.content_fire";
    fire.effect_attribute = "Attack";
    fire.effect_value = 18.0f;
    fire.pattern.setName("Fire Pattern");
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(
        fire,
        project_root / "content" / "abilities" / "fire.json"));

    urpg::ability::AuthoredAbilityAsset guard;
    guard.ability_id = "skill.content_guard";
    guard.effect_id = "effect.content_guard";
    guard.effect_attribute = "Defense";
    guard.effect_operation = urpg::ModifierOp::Override;
    guard.effect_value = 55.0f;
    guard.pattern.setName("Guard Pattern");
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(
        guard,
        project_root / "content" / "abilities" / "guard.json"));

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(workspace.setAbilityProjectRoot(project_root.string()));

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["project_content"]["canonical_directory"] ==
            (project_root / "content" / "abilities").generic_string());
    REQUIRE(exported["active_tab_detail"]["project_content"]["asset_count"] == 2);
    REQUIRE(exported["active_tab_detail"]["project_content"]["assets"][0]["ability_id"] == "skill.content_fire");
    REQUIRE(exported["active_tab_detail"]["project_content"]["assets"][1]["ability_id"] == "skill.content_guard");

    REQUIRE(workspace.selectAbilityProjectAsset(1));
    REQUIRE(workspace.loadSelectedAbilityProjectAsset());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["draft"]["ability_id"] == "skill.content_guard");
    REQUIRE(exported["active_tab_detail"]["draft"]["effect_attribute"] == "Defense");
    REQUIRE(exported["active_tab_detail"]["project_content"]["assets"][1]["selected"] == true);

    std::filesystem::remove_all(project_root);
}

TEST_CASE("DiagnosticsWorkspace - selected project ability asset can bind into a live map runtime and save back to content",
          "[editor][diagnostics][integration][abilities][content_binding][map]") {
    const auto project_root = std::filesystem::temp_directory_path() / "urpg_workspace_ability_content_binding";
    std::filesystem::remove_all(project_root);
    std::filesystem::create_directories(project_root / "content" / "abilities");

    urpg::ability::AuthoredAbilityAsset mapAsset;
    mapAsset.ability_id = "skill.content_map";
    mapAsset.mp_cost = 7.0f;
    mapAsset.effect_id = "effect.content_map";
    mapAsset.effect_attribute = "Defense";
    mapAsset.effect_value = 21.0f;
    mapAsset.pattern.setName("Map Content Pattern");
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(
        mapAsset,
        project_root / "content" / "abilities" / "map_guard.json"));

    urpg::scene::MapScene map("001", 10, 10);
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.bindAbilityRuntime(map.playerAbilitySystem());
    REQUIRE(workspace.setAbilityProjectRoot(project_root.string()));
    REQUIRE(workspace.applySelectedAbilityProjectAssetToRuntime());

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["name"] == "skill.content_map");
    REQUIRE(exported["active_tab_detail"]["project_content"]["can_apply_selected"] == true);

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("Defense", 100.0f);
    REQUIRE(workspace.previewSelectedAbility());
    REQUIRE(map.playerAbilitySystem().getAttribute("MP", 0.0f) == 23.0f);
    REQUIRE(map.playerAbilitySystem().getAttribute("Defense", 0.0f) == 121.0f);

    REQUIRE(workspace.setAbilityDraftId("skill.content_saved"));
    REQUIRE(workspace.setAbilityDraftEffectId("effect.content_saved"));
    REQUIRE(workspace.setAbilityDraftEffectAttribute("MagicDefense"));
    REQUIRE(workspace.setAbilityDraftEffectOperation(urpg::ModifierOp::Override));
    REQUIRE(workspace.setAbilityDraftEffectValue(64.0f));
    REQUIRE(workspace.saveAbilityDraftToProjectContent("saved/content_saved.json"));
    REQUIRE(std::filesystem::exists(project_root / "content" / "abilities" / "saved" / "content_saved.json"));

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["project_content"]["asset_count"] == 2);
    REQUIRE(exported["active_tab_detail"]["project_content"]["assets"][1]["relative_path"] ==
            "content/abilities/saved/content_saved.json");
    REQUIRE(exported["active_tab_detail"]["project_content"]["assets"][1]["selected"] == true);

    std::filesystem::remove_all(project_root);
}

