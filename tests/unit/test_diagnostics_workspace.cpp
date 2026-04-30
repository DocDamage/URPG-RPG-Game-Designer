#include "tests/unit/diagnostics_workspace_test_helpers.h"

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
    workspace.bindMigrationWizardRuntime(nlohmann::json{{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}});
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
        {"governance",
         {{"assetReport",
           {{"path", "imports/reports/asset_audit.json"},
            {"available", true},
            {"usable", false},
            {"issueCount", 2},
            {"normalizedCount", 2},
            {"promotedCount", 2},
            {"promotedVisualLaneCount", 1},
            {"promotedAudioLaneCount", 1},
            {"wysiwygSmokeProofCount", 1}}},
          {"schema",
           {{"schemaExists", true},
            {"changelogExists", false},
            {"mentionsSchemaVersion", true},
            {"schemaVersion", "2.1.0"}}},
          {"projectSchema",
           {{"path", "schemas/project.schema.json"},
            {"available", true},
            {"hasLocalizationSection", true},
            {"hasInputSection", false},
            {"hasExportSection", true}}},
          {"localizationEvidence",
           {{"path", "imports/reports/localization/localization_consistency_report.json"},
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
            {"bundles",
             {{{"path", "content/localization/en.json"}, {"locale", "en"}, {"keyCount", 3}},
              {{"path", "content/localization/fr.json"}, {"locale", "fr"}, {"keyCount", 2}}}}}},
          {"inputArtifacts", {{"path", "content/input/input_bindings.json"}, {"exists", true}, {"issueCount", 2}}},
          {"accessibilityArtifacts",
           {{"path", "content/schemas/accessibility_report.schema.json"}, {"available", true}, {"issueCount", 1}}},
          {"audioArtifacts",
           {{"path", "content/schemas/audio_mix_presets.schema.json"}, {"available", true}, {"issueCount", 2}}},
          {"performanceArtifacts",
           {{"path", "docs/presentation/performance_budgets.md"}, {"available", true}, {"issueCount", 3}}},
          {"releaseSignoffWorkflow",
           {{"path", "docs/RELEASE_SIGNOFF_WORKFLOW.md"}, {"available", true}, {"issueCount", 0}}},
          {"signoffArtifacts",
           {{"enabled", true},
            {"dependency", "human-review-gated subsystem signoff artifacts"},
            {"summary", "Checking required subsystem signoff artifacts, conservative wording, and structured "
                        "human-review signoff contracts for governed lanes."},
            {"available", true},
            {"issueCount", 1},
            {"expectedArtifacts",
             {{{"subsystemId", "battle_core"},
               {"title", "Battle Core signoff"},
               {"path", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"},
               {"required", true},
               {"exists", true},
               {"isRegularFile", true},
               {"status", "contract_mismatch"},
               {"wordingOk", true},
               {"signoffContract",
                {{"required", true},
                 {"artifactPath", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"},
                 {"promotionRequiresHumanReview", false},
                 {"workflow", "docs/RELEASE_SIGNOFF_WORKFLOW.md"},
                 {"contractOk", false}}}}}}}},
          {"templateSpecArtifacts",
           {{"enabled", true},
            {"path", "docs/templates/jrpg_spec.md"},
            {"available", true},
            {"issueCount", 1},
            {"expectedArtifacts",
             {{{"path", "docs/templates/jrpg_spec.md"},
               {"status", "parity_mismatch"},
               {"templateIdMatches", true},
               {"requiredSubsystemsMatch", false},
               {"barsMatch", false},
               {"missingRequiredSubsystems", {"save_data_core"}},
               {"unexpectedRequiredSubsystems", {"2_5d_mode"}},
               {"barMismatches",
                {{{"bar", "accessibility"},
                  {"label", "Accessibility"},
                  {"expectedStatus", "PARTIAL"},
                  {"specStatus", "PLANNED"}}}}}}}}}}},
        {"issues",
         {{{"code", "missing_localization_key"},
           {"title", "Missing localization key"},
           {"detail", "menu.start is missing from one locale."},
           {"severity", "warning"},
           {"blocksRelease", true},
           {"blocksExport", false}},
          {{"code", "missing_input_binding"},
           {"title", "Missing input binding"},
           {"detail", "confirm action is not bound for controller."},
           {"severity", "error"},
           {"blocksRelease", false},
           {"blocksExport", true}}}}});
    workspace.ingestEventAuthorityDiagnosticsJsonl(
        "{\"ts\":\"2026-03-04T00:00:02Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_"
        "rejected\",\"event_id\":\"evt_workspace\",\"block_id\":\"blk_workspace\",\"mode\":\"compat\",\"operation\":"
        "\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"workspace test\"}");

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
    REQUIRE(allSummaries.size() == 11);

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
    REQUIRE(exportedJson["tabs"].size() == 11);
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
    REQUIRE(workspace.eventAuthorityPanel().lastRenderSnapshot().selected_navigation_target->event_id ==
            "evt_workspace");
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
    REQUIRE(workspace.projectAuditPanel()
                .lastRenderSnapshot()
                .signoff_artifacts->expected_artifacts[0]
                .signoff_contract.has_value());
    REQUIRE(workspace.projectAuditPanel()
                .lastRenderSnapshot()
                .signoff_artifacts->expected_artifacts[0]
                .signoff_contract->contract_ok == false);
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_spec_artifacts.has_value());
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_spec_artifacts->path ==
            "docs/templates/jrpg_spec.md");
    REQUIRE(workspace.projectAuditPanel().lastRenderSnapshot().template_spec_artifacts->expected_artifacts.size() == 1);
    REQUIRE(workspace.projectAuditPanel()
                .lastRenderSnapshot()
                .template_spec_artifacts->expected_artifacts[0]
                .required_subsystems_match == false);
    REQUIRE(
        workspace.projectAuditPanel().lastRenderSnapshot().template_spec_artifacts->expected_artifacts[0].bars_match ==
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
    REQUIRE(
        auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["expected_artifacts"][0]["subsystem_id"] ==
        "battle_core");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["expected_artifacts"][0]["status"] ==
            "contract_mismatch");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["expected_artifacts"][0]
                     ["signoff_contract"]["artifact_path"] == "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["signoff_artifacts"]["expected_artifacts"][0]
                     ["signoff_contract"]["contract_ok"] == false);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["path"] ==
            "docs/templates/jrpg_spec.md");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["available"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["issue_count"] == 1);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["enabled"] == true);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"].is_array());
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"].size() == 1);
    REQUIRE(
        auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]["status"] ==
        "parity_mismatch");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]
                     ["required_subsystems_match"] == false);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]
                     ["bars_match"] == false);
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]
                     ["missing_required_subsystems"][0] == "save_data_core");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]
                     ["unexpected_required_subsystems"][0] == "2_5d_mode");
    REQUIRE(auditJson["active_tab_detail"]["governance"]["template_spec_artifacts"]["expected_artifacts"][0]
                     ["bar_mismatches"][0]["bar"] == "accessibility");
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
    REQUIRE(workspace.migrationWizardPanel().lastRenderSnapshot().summary_logs[0].find("Menu migration") !=
            std::string::npos);
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
