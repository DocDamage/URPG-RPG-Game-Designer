#include "tests/unit/diagnostics_workspace_test_helpers.h"

TEST_CASE("DiagnosticsWorkspace - Compat live QuickJS plugin execution projects through diagnostics export",
          "[editor][diagnostics][compat][quickjs][live_plugin][parity]") {
    auto& pluginManager = urpg::compat::PluginManager::instance();
    pluginManager.unloadAllPlugins();
    pluginManager.clearFailureDiagnostics();
    pluginManager.clearExecutionDiagnostics();

    const auto tempRoot = std::filesystem::temp_directory_path() / "urpg_diagnostics_live_plugin_parity";
    std::filesystem::create_directories(tempRoot);
    const auto pluginPath = tempRoot / "DiagnosticsLivePlugin.js";
    writeWorkspaceTextFile(pluginPath,
                           R"JS(/*:
 * @target MZ
 * @plugindesc Diagnostics live plugin parity fixture
 * @author URPG Test
 */
PluginManager.registerCommand("DiagnosticsLivePlugin", "echo", function(args) {
  return {
    seen: args.message,
    enabled: PluginManager.parameters("DiagnosticsLivePlugin").enabled
  };
});
)JS");

    urpg::Value enabled;
    enabled.v = true;
    pluginManager.setParameter("DiagnosticsLivePlugin", "enabled", enabled);
    REQUIRE(pluginManager.loadPlugin(pluginPath.string()));

    urpg::Object payload;
    urpg::Value message;
    message.v = std::string("through_diagnostics");
    payload["message"] = message;
    const auto result = pluginManager.executeCommand("DiagnosticsLivePlugin", "echo", {urpg::Value::Obj(payload)});
    REQUIRE(std::holds_alternative<urpg::Object>(result.v));
    REQUIRE(std::get<std::string>(std::get<urpg::Object>(result.v).at("seen").v) == "through_diagnostics");
    REQUIRE_FALSE(pluginManager.exportExecutionDiagnosticsJsonl().empty());

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.refresh();
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Compat);
    workspace.render();

    const auto exportJson = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exportJson["active_tab"] == "compat");
    REQUIRE(exportJson["active_tab_detail"]["plugins"].is_array());
    REQUIRE(exportJson["active_tab_detail"]["plugins"].size() == 1);
    REQUIRE(exportJson["active_tab_detail"]["plugins"][0]["pluginId"] == "DiagnosticsLivePlugin");
    REQUIRE(exportJson["active_tab_detail"]["plugins"][0]["fullCount"] == 1);
    REQUIRE(exportJson["active_tab_detail"]["plugins"][0]["totalCalls"] == 1);
    REQUIRE(exportJson["active_tab_detail"]["recent_events"].is_array());
    REQUIRE(exportJson["active_tab_detail"]["recent_events"][0]["pluginId"] == "DiagnosticsLivePlugin");
    REQUIRE(exportJson["active_tab_detail"]["recent_events"][0]["severity"] == "INFO");
    REQUIRE(exportJson["active_tab_detail"]["recent_events"][0]["methodName"] == "execute_command");
    REQUIRE(pluginManager.exportExecutionDiagnosticsJsonl().empty());

    pluginManager.unloadAllPlugins();
    std::error_code ec;
    std::filesystem::remove_all(tempRoot, ec);
}

TEST_CASE("DiagnosticsWorkspace - Project audit derives blocker counts from issue flags",
          "[editor][diagnostics][integration][project_audit]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindProjectAuditReport(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"headline", "Fallback audit"},
        {"summary", "Derived blocker counts should come from issue flags."},
        {"templateContext", {{"id", "jrpg"}, {"status", "PARTIAL"}}},
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
           {"blocksExport", true}},
          {{"code", "missing_console_profile"},
           {"title", "Missing console profile"},
           {"detail", "console export profile is incomplete."},
           {"severity", "error"},
           {"blocksRelease", true},
           {"blocksExport", true}}}},
        {"governance",
         {{"localizationArtifacts",
           {{"path", "content/localization/en-US.json"}, {"available", true}, {"issueCount", 1}}},
          {"exportArtifacts",
           {{"path", "exports/project_audit/export_manifest.json"}, {"present", true}, {"issueCount", 2}}}}}});

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

TEST_CASE("DiagnosticsWorkspace - Project audit CLI and export parity contract",
          "[editor][diagnostics][project_audit][parity]") {
    const nlohmann::json cliReport{
        {"schemaVersion", "1.0.0"},
        {"headline", "Parity audit"},
        {"summary", "CLI and editor export must agree on governed audit evidence."},
        {"releaseBlockerCount", 1},
        {"exportBlockerCount", 1},
        {"assetGovernanceIssueCount", 2},
        {"schemaGovernanceIssueCount", 1},
        {"projectArtifactIssueCount", 0},
        {"localizationEvidenceIssueCount", 1},
        {"accessibilityArtifactIssueCount", 1},
        {"audioArtifactIssueCount", 2},
        {"characterArtifactIssueCount", 3},
        {"modArtifactIssueCount", 4},
        {"analyticsArtifactIssueCount", 5},
        {"performanceArtifactIssueCount", 6},
        {"releaseSignoffWorkflowIssueCount", 0},
        {"signoffArtifactIssueCount", 1},
        {"templateSpecArtifactIssueCount", 1},
        {"templateContext", {{"id", "jrpg"}, {"status", "PARTIAL"}}},
        {"governance",
         {{"assetReport", {{"path", "imports/reports/asset_audit.json"}, {"available", true}, {"issueCount", 2}}},
          {"schema", {{"schemaExists", true}, {"changelogExists", true}, {"mentionsSchemaVersion", true}}},
          {"projectSchema",
           {{"path", "content/schemas/project.schema.json"},
            {"available", true},
            {"hasLocalizationSection", true},
            {"hasInputSection", true},
            {"hasExportSection", true}}},
          {"localizationEvidence",
           {{"path", "imports/reports/localization.json"},
            {"available", true},
            {"enabled", true},
            {"usable", true},
            {"issueCount", 1},
            {"status", "missing_keys"},
            {"hasBundles", true},
            {"bundleCount", 2}}},
          {"accessibilityArtifacts",
           {{"path", "content/schemas/accessibility_report.schema.json"}, {"available", true}, {"issueCount", 1}}},
          {"audioArtifacts",
           {{"path", "content/schemas/audio_mix_presets.schema.json"}, {"available", true}, {"issueCount", 2}}},
          {"characterArtifacts",
           {{"path", "content/schemas/character_identity.schema.json"}, {"available", true}, {"issueCount", 3}}},
          {"modArtifacts",
           {{"path", "content/schemas/mod_manifest.schema.json"}, {"available", true}, {"issueCount", 4}}},
          {"analyticsArtifacts",
           {{"path", "content/schemas/analytics_config.schema.json"}, {"available", true}, {"issueCount", 5}}},
          {"performanceArtifacts",
           {{"path", "docs/presentation/performance_budgets.md"}, {"available", true}, {"issueCount", 6}}},
          {"releaseSignoffWorkflow",
           {{"path", "docs/RELEASE_SIGNOFF_WORKFLOW.md"}, {"available", true}, {"issueCount", 0}}},
          {"signoffArtifacts",
           {{"path", "docs/signoff"},
            {"available", true},
            {"enabled", true},
            {"issueCount", 1},
            {"expectedArtifacts",
             {{{"subsystemId", "battle_core"},
               {"title", "Battle Core"},
               {"path", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"},
               {"required", true},
               {"exists", true},
               {"isRegularFile", true},
               {"status", "contract_mismatch"},
               {"signoffContract",
                {{"required", true},
                 {"artifactPath", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"},
                 {"promotionRequiresHumanReview", true},
                 {"workflow", "docs/RELEASE_SIGNOFF_WORKFLOW.md"},
                 {"contractOk", false}}}}}}}},
          {"templateSpecArtifacts",
           {{"path", "docs/templates/jrpg_spec.md"},
            {"available", true},
            {"enabled", true},
            {"issueCount", 1},
            {"expectedArtifacts",
             {{{"path", "docs/templates/jrpg_spec.md"},
               {"required", true},
               {"exists", true},
               {"status", "parity_mismatch"},
               {"templateIdMatches", true},
               {"requiredSubsystemsMatch", false},
               {"barsMatch", false},
               {"missingRequiredSubsystems", {"save_data_core"}},
               {"barMismatches",
                {{{"bar", "accessibility"},
                  {"label", "Accessibility"},
                  {"expectedStatus", "DONE"},
                  {"specStatus", "PARTIAL"}}}}}}}}}}},
        {"issues",
         {{{"code", "missing_localization_key"},
           {"title", "Missing localization key"},
           {"detail", "menu.start is missing from one locale."},
           {"severity", "warning"},
           {"blocksRelease", true},
           {"blocksExport", false}},
          {{"code", "missing_export_contract"},
           {"title", "Missing export contract"},
           {"detail", "Export manifest evidence is stale."},
           {"severity", "error"},
           {"blocksRelease", false},
           {"blocksExport", true}}}}};

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindProjectAuditReport(cliReport);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::ProjectAudit);
    workspace.render();

    const auto parity = workspace.compareProjectAuditExportParityReport(cliReport);
    REQUIRE(parity.matches);
    REQUIRE(parity.issues.empty());
    REQUIRE(parity.workspace_canonical["character_artifact_issue_count"] == 3);
    REQUIRE(parity.workspace_canonical["mod_artifact_issue_count"] == 4);
    REQUIRE(parity.workspace_canonical["analytics_artifact_issue_count"] == 5);
    REQUIRE(parity.workspace_canonical["governance"]["character_artifacts"]["issue_count"] == 3);
    REQUIRE(parity.workspace_canonical["governance"]["mod_artifacts"]["issue_count"] == 4);
    REQUIRE(parity.workspace_canonical["governance"]["analytics_artifacts"]["issue_count"] == 5);

    auto driftedWorkspaceExport = nlohmann::json::parse(workspace.exportAsJson());
    driftedWorkspaceExport["active_tab_detail"]["release_blocker_count"] = static_cast<size_t>(99);
    const auto drift = urpg::editor::compareProjectAuditExportParity(cliReport, driftedWorkspaceExport);
    REQUIRE_FALSE(drift.matches);
    REQUIRE(drift.issues.size() == 1);
    REQUIRE(drift.issues[0].code == "value_mismatch");
    REQUIRE(drift.issues[0].path == "/release_blocker_count");

    const auto parityJson = nlohmann::json::parse(workspace.exportProjectAuditParityJson(cliReport));
    REQUIRE(parityJson["matches"] == true);
    REQUIRE(parityJson["issue_count"] == 0);
    REQUIRE(parityJson["cli_canonical"]["headline"] == "Parity audit");
}
