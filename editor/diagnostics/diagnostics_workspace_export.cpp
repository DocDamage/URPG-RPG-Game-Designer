#include "editor/diagnostics/diagnostics_workspace.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

namespace {
const char* TabName(DiagnosticsTab tab) {
    switch (tab) {
    case DiagnosticsTab::Compat:
        return "compat";
    case DiagnosticsTab::Save:
        return "save";
    case DiagnosticsTab::EventAuthority:
        return "event_authority";
    case DiagnosticsTab::MessageText:
        return "message_text";
    case DiagnosticsTab::Battle:
        return "battle";
    case DiagnosticsTab::Menu:
        return "menu";
    case DiagnosticsTab::Audio:
        return "audio";
    case DiagnosticsTab::MigrationWizard:
        return "migration_wizard";
    case DiagnosticsTab::Abilities:
        return "abilities";
    case DiagnosticsTab::ProjectAudit:
        return "project_audit";
    }
    return "compat";
}

nlohmann::json TabSummaryJson(const DiagnosticsTabSummary& summary) {
    return {
        {"name", TabName(summary.tab)},
        {"item_count", summary.item_count},
        {"issue_count", summary.issue_count},
        {"has_data", summary.has_data},
        {"active", summary.active},
    };
}

nlohmann::json CompatPluginSummaryJson(const PluginCompatSummary& summary) {
    nlohmann::json scoreHistory = nlohmann::json::array();
    for (const auto score : summary.scoreHistory) {
        scoreHistory.push_back(score);
    }
    return {
        {"pluginId", summary.pluginId},
        {"pluginName", summary.pluginName},
        {"version", summary.version},
        {"fullCount", summary.fullCount},
        {"partialCount", summary.partialCount},
        {"stubCount", summary.stubCount},
        {"unsupportedCount", summary.unsupportedCount},
        {"compatibilityScore", summary.compatibilityScore},
        {"warningCount", summary.warningCount},
        {"errorCount", summary.errorCount},
        {"totalCalls", summary.totalCalls},
        {"totalDurationUs", summary.totalDurationUs},
        {"scoreHistory", std::move(scoreHistory)},
        {"firstSeenTimestamp", summary.firstSeenTimestamp},
        {"lastUpdatedTimestamp", summary.lastUpdatedTimestamp},
    };
}

nlohmann::json CompatEventJson(const CompatEvent& event) {
    return {
        {"timestamp", event.timestamp},
        {"pluginId", event.pluginId},
        {"className", event.className},
        {"methodName", event.methodName},
        {"severity", event.severityToString()},
        {"message", event.message},
        {"sourceFile", event.sourceFile},
        {"sourceLine", event.sourceLine},
        {"navigationTarget", event.navigationTarget},
    };
}

const char* CompatStatusName(CompatStatus status) {
    switch (status) {
    case CompatStatus::FULL:
        return "FULL";
    case CompatStatus::PARTIAL:
        return "PARTIAL";
    case CompatStatus::STUB:
        return "STUB";
    case CompatStatus::UNSUPPORTED:
        return "UNSUPPORTED";
    default:
        return "UNKNOWN";
    }
}

nlohmann::json CompatCallRecordJson(const CompatCallRecord& record) {
    return {
        {"pluginId", record.pluginId},
        {"className", record.className},
        {"methodName", record.methodName},
        {"status", CompatStatusName(record.status)},
        {"deviationNote", record.deviationNote},
        {"callCount", record.callCount},
        {"totalDurationUs", record.totalDurationUs},
        {"lastCallTimestamp", record.lastCallTimestamp},
        {"hasWarning", record.hasWarning},
        {"hasError", record.hasError},
    };
}

nlohmann::json SaveRowJson(const SaveInspectorRow& row) {
    return {
        {"slot_id", row.slot_id},
        {"reserved_slot", row.reserved_slot},
        {"autosave", row.autosave},
        {"corrupted", row.corrupted},
        {"loaded_from_recovery", row.loaded_from_recovery},
        {"boot_safe_mode", row.boot_safe_mode},
        {"slot_label", row.slot_label},
        {"map_display_name", row.map_display_name},
        {"summary", row.summary},
        {"operation", row.operation},
        {"category_label", row.category_label},
        {"retention_label", row.retention_label},
        {"recovery_label", row.recovery_label},
        {"diagnostic", row.diagnostic},
    };
}

nlohmann::json SaveInspectorSummaryJson(const SaveInspectorSummary& summary) {
    return {
        {"total_slots", summary.total_slots},
        {"autosave_slots", summary.autosave_slots},
        {"quicksave_slots", summary.quicksave_slots},
        {"manual_slots", summary.manual_slots},
        {"corrupted_slots", summary.corrupted_slots},
        {"recovery_slots", summary.recovery_slots},
        {"safe_mode_slots", summary.safe_mode_slots},
        {"autosave_enabled", summary.autosave_enabled},
        {"autosave_slot_id", summary.autosave_slot_id},
        {"autosave_retention_limit", summary.autosave_retention_limit},
        {"quicksave_retention_limit", summary.quicksave_retention_limit},
        {"manual_retention_limit", summary.manual_retention_limit},
        {"prune_excess_on_save", summary.prune_excess_on_save},
        {"reserved_slots", summary.reserved_slots},
    };
}

nlohmann::json SaveMetadataFieldJson(const SaveInspectorMetadataFieldRow& field) {
    return {
        {"key", field.key},
        {"display_label", field.display_label},
        {"required", field.required},
        {"default_value", field.default_value},
    };
}

nlohmann::json SaveSlotDescriptorJson(const urpg::SaveSlotDescriptor& descriptor) {
    return {
        {"slot_id", descriptor.slot_id},
        {"category", ToString(descriptor.category)},
        {"label", descriptor.label},
        {"reserved", descriptor.reserved},
    };
}

nlohmann::json SaveRecoveryDiagnosticsJson(const SaveRecoveryDiagnosticsSummary& summary) {
    return {
        {"total_recovery_slots", summary.total_recovery_slots},
        {"autosave_recovery_slots", summary.autosave_recovery_slots},
        {"metadata_variables_recovery_slots", summary.metadata_variables_recovery_slots},
        {"safe_mode_recovery_slots", summary.safe_mode_recovery_slots},
        {"corrupted_slots", summary.corrupted_slots},
        {"diagnostic_rows", summary.diagnostic_rows},
    };
}

nlohmann::json SaveSerializationSchemaJson(const SaveSerializationSchemaSummary& summary) {
    return {
        {"format_magic", summary.format_magic},
        {"version_major", summary.version_major},
        {"version_minor", summary.version_minor},
        {"differential_supported", summary.differential_supported},
        {"compression_modes", summary.compression_modes},
    };
}

const char* SavePolicyIssueSeverityName(SavePolicyIssueSeverity severity) {
    switch (severity) {
    case SavePolicyIssueSeverity::Warning:
        return "warning";
    case SavePolicyIssueSeverity::Error:
        return "error";
    }
    return "warning";
}

nlohmann::json SavePolicyDraftJson(const SavePolicyDraft& draft) {
    return {
        {"autosave_enabled", draft.autosave_enabled},
        {"autosave_slot_id", draft.autosave_slot_id},
        {"max_autosave_slots", draft.max_autosave_slots},
        {"max_quicksave_slots", draft.max_quicksave_slots},
        {"max_manual_slots", draft.max_manual_slots},
        {"prune_excess_on_save", draft.prune_excess_on_save},
    };
}

nlohmann::json SavePolicyIssueJson(const SavePolicyIssue& issue) {
    return {
        {"severity", SavePolicyIssueSeverityName(issue.severity)},
        {"code", issue.code},
        {"message", issue.message},
    };
}

nlohmann::json SavePolicyValidationJson(const SavePolicyValidationSummary& summary) {
    return {
        {"issue_count", summary.issue_count},
        {"warning_count", summary.warning_count},
        {"error_count", summary.error_count},
        {"can_apply", summary.can_apply},
    };
}

nlohmann::json EventAuthorityTargetJson(const EventNavigationTarget& target) {
    return {
        {"event_id", target.event_id},
        {"block_id", target.block_id},
    };
}

nlohmann::json EventAuthorityRowJson(const EventAuthorityPanel::SelectedRowSnapshot& row) {
    return {
        {"ts", row.ts},
        {"level", row.level},
        {"event_id", row.event_id},
        {"block_id", row.block_id},
        {"mode", row.mode},
        {"operation", row.operation},
        {"error_code", row.error_code},
        {"message", row.message},
        {"summary", row.summary},
    };
}

nlohmann::json EventAuthorityRowJson(const EventAuthorityPanelRow& row) {
    return {
        {"ts", row.ts},
        {"level", row.level},
        {"event_id", row.event_id},
        {"block_id", row.block_id},
        {"mode", row.mode},
        {"operation", row.operation},
        {"error_code", row.error_code},
        {"message", row.message},
        {"summary", row.summary},
    };
}

nlohmann::json MigrationWizardSubsystemJson(const MigrationWizardModel::SubsystemResult& result) {
    return {
        {"subsystem_id", result.subsystem_id},
        {"display_name", result.display_name},
        {"processed_count", result.processed_count},
        {"warning_count", result.warning_count},
        {"error_count", result.error_count},
        {"completed", result.completed},
        {"summary_line", result.summary_line},
    };
}

nlohmann::json MigrationWizardWorkflowActionJson(const MigrationWizardPanel::WorkflowActionState& action) {
    return {
        {"id", action.id},
        {"label", action.label},
        {"visible", action.visible},
        {"enabled", action.enabled},
    };
}

nlohmann::json MigrationWizardWorkflowSubsystemCardJson(const MigrationWizardPanel::WorkflowSubsystemCard& card) {
    return {
        {"subsystem_id", card.subsystem_id},
        {"display_name", card.display_name},
        {"processed_count", card.processed_count},
        {"warning_count", card.warning_count},
        {"error_count", card.error_count},
        {"completed", card.completed},
        {"summary_line", card.summary_line},
        {"is_selected", card.is_selected},
        {"can_rerun", card.can_rerun},
        {"can_clear", card.can_clear},
    };
}

nlohmann::json MigrationWizardWorkflowPrimaryActionsJson(const MigrationWizardPanel::WorkflowPrimaryActions& actions) {
    return {
        {"run_migration", MigrationWizardWorkflowActionJson(actions.run_migration)},
        {"rerun_selected_subsystem", MigrationWizardWorkflowActionJson(actions.rerun_selected_subsystem)},
        {"clear_selected_subsystem", MigrationWizardWorkflowActionJson(actions.clear_selected_subsystem)},
        {"next_subsystem", MigrationWizardWorkflowActionJson(actions.next_subsystem)},
        {"previous_subsystem", MigrationWizardWorkflowActionJson(actions.previous_subsystem)},
        {"next_issue_subsystem", MigrationWizardWorkflowActionJson(actions.next_issue_subsystem)},
        {"previous_issue_subsystem", MigrationWizardWorkflowActionJson(actions.previous_issue_subsystem)},
    };
}

nlohmann::json MigrationWizardWorkflowReportIoJson(const MigrationWizardPanel::WorkflowReportIoState& report_io) {
    return {
        {"save", MigrationWizardWorkflowActionJson(report_io.save)},
        {"load", MigrationWizardWorkflowActionJson(report_io.load)},
        {"exported_report_json", report_io.exported_report_json},
    };
}

nlohmann::json MigrationWizardWorkflowBoundRuntimeJson(const MigrationWizardPanel::WorkflowBoundRuntimeActions& actions) {
    return {
        {"has_bound_project_data", actions.has_bound_project_data},
        {"rerun_migration", MigrationWizardWorkflowActionJson(actions.rerun_migration)},
        {"rerun_selected_subsystem", MigrationWizardWorkflowActionJson(actions.rerun_selected_subsystem)},
    };
}

nlohmann::json MigrationWizardSubsystemResultsJson(
    const std::vector<MigrationWizardModel::SubsystemResult>& subsystem_results) {
    nlohmann::json subsystemResults = nlohmann::json::array();
    for (const auto& result : subsystem_results) {
        subsystemResults.push_back(MigrationWizardSubsystemJson(result));
    }
    return subsystemResults;
}

nlohmann::json MigrationWizardSubsystemCardsJson(
    const std::vector<MigrationWizardPanel::WorkflowSubsystemCard>& subsystem_cards) {
    nlohmann::json subsystemCards = nlohmann::json::array();
    for (const auto& card : subsystem_cards) {
        subsystemCards.push_back(MigrationWizardWorkflowSubsystemCardJson(card));
    }
    return subsystemCards;
}

nlohmann::json MigrationWizardSelectedSubsystemCardJson(
    const std::optional<MigrationWizardPanel::WorkflowSubsystemCard>& selected_subsystem_card) {
    return selected_subsystem_card.has_value() ? MigrationWizardWorkflowSubsystemCardJson(*selected_subsystem_card)
                                               : nlohmann::json(nullptr);
}

const char* AudioCategoryName(urpg::audio::AudioCategory category) {
    switch (category) {
    case urpg::audio::AudioCategory::BGM:
        return "BGM";
    case urpg::audio::AudioCategory::BGS:
        return "BGS";
    case urpg::audio::AudioCategory::SE:
        return "SE";
    case urpg::audio::AudioCategory::ME:
        return "ME";
    case urpg::audio::AudioCategory::System:
        return "System";
    }
    return "System";
}

nlohmann::json AudioHandleRowJson(const AudioHandleRow& row) {
    return {
        {"handle", row.handle},
        {"assetId", row.assetId},
        {"category", AudioCategoryName(row.category)},
        {"volume", row.volume},
        {"pitch", row.pitch},
        {"isLooping", row.isLooping},
        {"isActive", row.isActive},
    };
}

const char* MessageInspectorIssueSeverityName(MessageInspectorIssueSeverity severity) {
    switch (severity) {
    case MessageInspectorIssueSeverity::Info:
        return "info";
    case MessageInspectorIssueSeverity::Warning:
        return "warning";
    case MessageInspectorIssueSeverity::Error:
        return "error";
    }
    return "info";
}

nlohmann::json MessageInspectorSummaryJson(const MessageInspectorSummary& summary) {
    return {
        {"total_pages", summary.total_pages},
        {"speaker_pages", summary.speaker_pages},
        {"narration_pages", summary.narration_pages},
        {"system_pages", summary.system_pages},
        {"choice_pages", summary.choice_pages},
        {"issue_count", summary.issue_count},
        {"max_preview_width", summary.max_preview_width},
        {"has_active_flow", summary.has_active_flow},
        {"current_page_index", summary.current_page_index},
    };
}

nlohmann::json MessageInspectorRowJson(const MessageInspectorRow& row) {
    return {
        {"page_index", row.page_index},
        {"page_id", row.page_id},
        {"route", row.route},
        {"tone", row.tone},
        {"speaker", row.speaker},
        {"face_actor_id", row.face_actor_id},
        {"body_preview", row.body_preview},
        {"line_count", row.line_count},
        {"preview_width", row.preview_width},
        {"preview_height", row.preview_height},
        {"has_choices", row.has_choices},
        {"choice_count", row.choice_count},
        {"issue_count", row.issue_count},
    };
}

nlohmann::json MessageInspectorIssueJson(const MessageInspectorIssue& issue) {
    return {
        {"severity", MessageInspectorIssueSeverityName(issue.severity)},
        {"code", issue.code},
        {"page_id", issue.page_id},
        {"message", issue.message},
    };
}

nlohmann::json AbilityInfoJson(const AbilityInfo& info) {
    return {
        {"name", info.name},
        {"can_activate", info.can_activate},
        {"cooldown_remaining", info.cooldown_remaining},
        {"blocking_reason", info.blocking_reason},
        {"has_pattern", static_cast<bool>(info.pattern)},
    };
}

nlohmann::json ActiveTagInfoJson(const ActiveTagInfo& info) {
    return {
        {"tag", info.tag},
        {"count", info.count},
    };
}

template <typename SnapshotT>
nlohmann::json ProjectAuditArtifactGovernanceJson(const SnapshotT& snapshot) {
    nlohmann::json artifact = nlohmann::json::object();
    if (snapshot.path.has_value()) {
        artifact["path"] = *snapshot.path;
    }
    if (snapshot.available.has_value()) {
        artifact["available"] = *snapshot.available;
    }
    if (snapshot.issue_count.has_value()) {
        artifact["issue_count"] = *snapshot.issue_count;
    }
    return artifact;
}

nlohmann::json ProjectAuditSignoffContractJson(const ProjectAuditSignoffContractSnapshot& snapshot) {
    nlohmann::json contract = nlohmann::json::object();
    if (snapshot.required.has_value()) {
        contract["required"] = *snapshot.required;
    }
    if (snapshot.artifact_path.has_value()) {
        contract["artifact_path"] = *snapshot.artifact_path;
    }
    if (snapshot.promotion_requires_human_review.has_value()) {
        contract["promotion_requires_human_review"] = *snapshot.promotion_requires_human_review;
    }
    if (snapshot.workflow.has_value()) {
        contract["workflow"] = *snapshot.workflow;
    }
    if (snapshot.contract_ok.has_value()) {
        contract["contract_ok"] = *snapshot.contract_ok;
    }
    return contract;
}

nlohmann::json ProjectAuditExpectedArtifactJson(const ProjectAuditExpectedArtifactSnapshot& snapshot) {
    nlohmann::json artifact = nlohmann::json::object();
    if (snapshot.subsystem_id.has_value()) {
        artifact["subsystem_id"] = *snapshot.subsystem_id;
    }
    if (snapshot.title.has_value()) {
        artifact["title"] = *snapshot.title;
    }
    if (snapshot.path.has_value()) {
        artifact["path"] = *snapshot.path;
    }
    if (snapshot.required.has_value()) {
        artifact["required"] = *snapshot.required;
    }
    if (snapshot.exists.has_value()) {
        artifact["exists"] = *snapshot.exists;
    }
    if (snapshot.is_regular_file.has_value()) {
        artifact["is_regular_file"] = *snapshot.is_regular_file;
    }
    if (snapshot.status.has_value()) {
        artifact["status"] = *snapshot.status;
    }
    if (snapshot.wording_ok.has_value()) {
        artifact["wording_ok"] = *snapshot.wording_ok;
    }
    if (snapshot.template_id_matches.has_value()) {
        artifact["template_id_matches"] = *snapshot.template_id_matches;
    }
    if (snapshot.required_subsystems_match.has_value()) {
        artifact["required_subsystems_match"] = *snapshot.required_subsystems_match;
    }
    if (snapshot.bars_match.has_value()) {
        artifact["bars_match"] = *snapshot.bars_match;
    }
    if (!snapshot.missing_phrases.empty()) {
        artifact["missing_phrases"] = snapshot.missing_phrases;
    }
    if (!snapshot.missing_required_subsystems.empty()) {
        artifact["missing_required_subsystems"] = snapshot.missing_required_subsystems;
    }
    if (!snapshot.unexpected_required_subsystems.empty()) {
        artifact["unexpected_required_subsystems"] = snapshot.unexpected_required_subsystems;
    }
    if (snapshot.bar_mismatches.has_value()) {
        artifact["bar_mismatches"] = *snapshot.bar_mismatches;
    }
    if (snapshot.signoff_contract.has_value()) {
        artifact["signoff_contract"] = ProjectAuditSignoffContractJson(*snapshot.signoff_contract);
    }
    return artifact;
}

nlohmann::json ProjectAuditRichArtifactGovernanceJson(const ProjectAuditRichArtifactGovernanceSnapshot& snapshot) {
    nlohmann::json artifact = ProjectAuditArtifactGovernanceJson(snapshot);
    if (snapshot.enabled.has_value()) {
        artifact["enabled"] = *snapshot.enabled;
    }
    if (snapshot.dependency.has_value()) {
        artifact["dependency"] = *snapshot.dependency;
    }
    if (snapshot.summary.has_value()) {
        artifact["summary"] = *snapshot.summary;
    }
    if (!snapshot.expected_artifacts.empty()) {
        nlohmann::json expected_artifacts = nlohmann::json::array();
        for (const auto& expected_artifact : snapshot.expected_artifacts) {
            expected_artifacts.push_back(ProjectAuditExpectedArtifactJson(expected_artifact));
        }
        artifact["expected_artifacts"] = std::move(expected_artifacts);
    }
    return artifact;
}

nlohmann::json ProjectAuditLocalizationEvidenceJson(const ProjectAuditLocalizationEvidenceSnapshot& snapshot) {
    nlohmann::json artifact = ProjectAuditArtifactGovernanceJson(snapshot);
    if (snapshot.enabled.has_value()) {
        artifact["enabled"] = *snapshot.enabled;
    }
    if (snapshot.usable.has_value()) {
        artifact["usable"] = *snapshot.usable;
    }
    if (snapshot.dependency.has_value()) {
        artifact["dependency"] = *snapshot.dependency;
    }
    if (snapshot.summary.has_value()) {
        artifact["summary"] = *snapshot.summary;
    }
    if (snapshot.status.has_value()) {
        artifact["status"] = *snapshot.status;
    }
    if (snapshot.has_bundles.has_value()) {
        artifact["has_bundles"] = *snapshot.has_bundles;
    }
    if (snapshot.bundle_count.has_value()) {
        artifact["bundle_count"] = *snapshot.bundle_count;
    }
    if (snapshot.missing_locale_count.has_value()) {
        artifact["missing_locale_count"] = *snapshot.missing_locale_count;
    }
    if (snapshot.missing_key_count.has_value()) {
        artifact["missing_key_count"] = *snapshot.missing_key_count;
    }
    if (snapshot.extra_key_count.has_value()) {
        artifact["extra_key_count"] = *snapshot.extra_key_count;
    }
    if (snapshot.master_locale.has_value()) {
        artifact["master_locale"] = *snapshot.master_locale;
    }
    if (snapshot.bundles.has_value()) {
        artifact["bundles"] = *snapshot.bundles;
    }
    return artifact;
}

const char* MenuInspectorIssueSeverityName(MenuInspectorIssueSeverity severity) {
    switch (severity) {
    case MenuInspectorIssueSeverity::Info:
        return "info";
    case MenuInspectorIssueSeverity::Warning:
        return "warning";
    case MenuInspectorIssueSeverity::Error:
        return "error";
    }
    return "info";
}

nlohmann::json MenuInspectorSummaryJson(const MenuInspectorSummary& summary) {
    return {
        {"active_scene_id", summary.active_scene_id},
        {"stack_depth", summary.stack_depth},
        {"total_panes", summary.total_panes},
        {"visible_panes", summary.visible_panes},
        {"active_panes", summary.active_panes},
        {"navigable_panes", summary.navigable_panes},
        {"total_commands", summary.total_commands},
        {"visible_commands", summary.visible_commands},
        {"enabled_commands", summary.enabled_commands},
        {"blocked_commands", summary.blocked_commands},
        {"issue_count", summary.issue_count},
        {"missing_registry_entries", summary.missing_registry_entries},
        {"route_binding_issues", summary.route_binding_issues},
        {"rule_validation_issues", summary.rule_validation_issues},
        {"duplicate_command_ids", summary.duplicate_command_ids},
    };
}

nlohmann::json MenuInspectorRowJson(const MenuInspectorRow& row) {
    return {
        {"scene_id", row.scene_id},
        {"pane_index", row.pane_index},
        {"pane_id", row.pane_id},
        {"pane_label", row.pane_label},
        {"command_index", row.command_index},
        {"command_id", row.command_id},
        {"command_label", row.command_label},
        {"icon_id", row.icon_id},
        {"route_label", row.route_label},
        {"custom_route_id", row.custom_route_id},
        {"fallback_route_label", row.fallback_route_label},
        {"fallback_custom_route_id", row.fallback_custom_route_id},
        {"priority", row.priority},
        {"pane_visible", row.pane_visible},
        {"pane_active", row.pane_active},
        {"command_registered", row.command_registered},
        {"command_visible", row.command_visible},
        {"command_enabled", row.command_enabled},
        {"row_navigable", row.row_navigable},
        {"issue_count", row.issue_count},
        {"summary", row.summary},
    };
}

nlohmann::json MenuInspectorIssueJson(const MenuInspectorIssue& issue) {
    nlohmann::json root{
        {"severity", MenuInspectorIssueSeverityName(issue.severity)},
        {"code", issue.code},
        {"message", issue.message},
        {"scene_id", issue.scene_id},
        {"pane_id", issue.pane_id},
        {"command_id", issue.command_id},
    };
    root["pane_index"] = issue.pane_index.has_value() ? nlohmann::json(*issue.pane_index) : nlohmann::json(nullptr);
    root["command_index"] = issue.command_index.has_value() ? nlohmann::json(*issue.command_index) : nlohmann::json(nullptr);
    return root;
}

nlohmann::json MenuPreviewPaneJson(const MenuPreviewPanel::PaneSnapshot& pane) {
    return {
        {"pane_id", pane.pane_id},
        {"pane_label", pane.pane_label},
        {"pane_active", pane.pane_active},
        {"selected_command_id", pane.selected_command_id.has_value() ? nlohmann::json(*pane.selected_command_id)
                                                                     : nlohmann::json(nullptr)},
        {"command_ids", pane.command_ids},
    };
}

const char* BattleInspectorIssueSeverityName(BattleInspectorIssueSeverity severity) {
    switch (severity) {
    case BattleInspectorIssueSeverity::Info:
        return "info";
    case BattleInspectorIssueSeverity::Warning:
        return "warning";
    case BattleInspectorIssueSeverity::Error:
        return "error";
    }
    return "info";
}

const char* BattlePreviewIssueSeverityName(BattlePreviewIssueSeverity severity) {
    switch (severity) {
    case BattlePreviewIssueSeverity::Info:
        return "info";
    case BattlePreviewIssueSeverity::Warning:
        return "warning";
    case BattlePreviewIssueSeverity::Error:
        return "error";
    }
    return "info";
}

nlohmann::json BattleInspectorSummaryJson(const BattleInspectorSummary& summary) {
    return {
        {"phase", summary.phase},
        {"active", summary.active},
        {"can_escape", summary.can_escape},
        {"turn_count", summary.turn_count},
        {"escape_failures", summary.escape_failures},
        {"total_actions", summary.total_actions},
        {"unique_subjects", summary.unique_subjects},
        {"fastest_speed", summary.fastest_speed},
        {"slowest_speed", summary.slowest_speed},
        {"issue_count", summary.issue_count},
    };
}

nlohmann::json BattleInspectorRowJson(const BattleInspectorActionRow& row) {
    return {
        {"action_order", row.action_order},
        {"subject_id", row.subject_id},
        {"target_id", row.target_id},
        {"command", row.command},
        {"speed", row.speed},
        {"priority", row.priority},
        {"issue_count", row.issue_count},
        {"summary", row.summary},
    };
}

nlohmann::json BattleInspectorIssueJson(const BattleInspectorIssue& issue) {
    nlohmann::json root{
        {"severity", BattleInspectorIssueSeverityName(issue.severity)},
        {"code", issue.code},
        {"message", issue.message},
    };
    root["action_order"] = issue.action_order.has_value() ? nlohmann::json(*issue.action_order) : nlohmann::json(nullptr);
    return root;
}

nlohmann::json BattlePreviewSnapshotJson(const BattlePreviewSnapshot& snapshot) {
    return {
        {"phase", snapshot.phase},
        {"can_escape", snapshot.can_escape},
        {"physical_damage", snapshot.physical_damage},
        {"guarded_damage", snapshot.guarded_damage},
        {"critical_damage", snapshot.critical_damage},
        {"magical_damage", snapshot.magical_damage},
        {"escape_ratio_now", snapshot.escape_ratio_now},
        {"escape_ratio_next_fail", snapshot.escape_ratio_next_fail},
        {"issue_count", snapshot.issue_count},
    };
}

nlohmann::json BattlePreviewIssueJson(const BattlePreviewIssue& issue) {
    return {
        {"severity", BattlePreviewIssueSeverityName(issue.severity)},
        {"code", issue.code},
        {"message", issue.message},
    };
}


} // namespace

std::string DiagnosticsWorkspace::exportAsJson() const {
    nlohmann::json root;
    root["active_tab"] = TabName(active_tab_);
    root["visible"] = visible_;
    root["active_tab_detail"] = {
        {"tab", TabName(active_tab_)},
        {"summary", TabSummaryJson(tabSummary(active_tab_))},
    };

    auto& activeTabDetail = root["active_tab_detail"];
    switch (active_tab_) {
    case DiagnosticsTab::Compat: {
        nlohmann::json plugins = nlohmann::json::array();
        for (const auto& pluginSummary : compat_panel_.getModel().getAllPluginSummaries()) {
            plugins.push_back(CompatPluginSummaryJson(pluginSummary));
        }
        activeTabDetail["plugins"] = std::move(plugins);
        activeTabDetail["project_compatibility_score"] = compat_panel_.getModel().getProjectCompatibilityScore();
        activeTabDetail["selected_plugin"] =
            compat_panel_.getSelectedPlugin().empty() ? nlohmann::json(nullptr) : nlohmann::json(compat_panel_.getSelectedPlugin());
        activeTabDetail["detail_view"] = compat_panel_.isDetailView();
        nlohmann::json recentEvents = nlohmann::json::array();
        for (const auto& event : compat_panel_.getModel().getRecentEvents(1000)) {
            recentEvents.push_back(CompatEventJson(event));
        }
        activeTabDetail["recent_events"] = std::move(recentEvents);
        activeTabDetail["recent_event_count"] = activeTabDetail["recent_events"].size();
        if (!compat_panel_.getSelectedPlugin().empty()) {
            const auto selectedPlugin = compat_panel_.getSelectedPlugin();
            activeTabDetail["selected_plugin_summary"] =
                CompatPluginSummaryJson(compat_panel_.getModel().getPluginSummary(selectedPlugin));

            nlohmann::json selectedCalls = nlohmann::json::array();
            for (const auto& call : compat_panel_.getModel().getPluginCalls(selectedPlugin)) {
                selectedCalls.push_back(CompatCallRecordJson(call));
            }
            activeTabDetail["selected_plugin_calls"] = std::move(selectedCalls);

            nlohmann::json selectedEvents = nlohmann::json::array();
            for (const auto& event : compat_panel_.getModel().getPluginEvents(selectedPlugin)) {
                selectedEvents.push_back(CompatEventJson(event));
            }
            activeTabDetail["selected_plugin_events"] = std::move(selectedEvents);
        }
        break;
    }
    case DiagnosticsTab::Save: {
        activeTabDetail["save_summary"] = SaveInspectorSummaryJson(save_panel_.getModel().Summary());
        activeTabDetail["show_problem_slots_only"] = save_panel_.showProblemSlotsOnly();
        activeTabDetail["include_autosave"] = save_panel_.includeAutosave();
        activeTabDetail["autosave_policy"] = {
            {"enabled", save_panel_.getModel().Summary().autosave_enabled},
            {"slot_id", save_panel_.getModel().Summary().autosave_slot_id},
        };
        activeTabDetail["retention_policy"] = {
            {"max_autosave_slots", save_panel_.getModel().Summary().autosave_retention_limit},
            {"max_quicksave_slots", save_panel_.getModel().Summary().quicksave_retention_limit},
            {"max_manual_slots", save_panel_.getModel().Summary().manual_retention_limit},
            {"prune_excess_on_save", save_panel_.getModel().Summary().prune_excess_on_save},
        };
        activeTabDetail["policy_draft"] = SavePolicyDraftJson(save_panel_.getModel().PolicyDraft());
        activeTabDetail["policy_validation"] = SavePolicyValidationJson(save_panel_.getModel().PolicyValidation());
        activeTabDetail["selected_slot_id"] =
            save_panel_.getModel().SelectedSlotId().has_value() ? nlohmann::json(*save_panel_.getModel().SelectedSlotId())
                                                                : nlohmann::json(nullptr);
        activeTabDetail["selected_row"] = nullptr;
        nlohmann::json policyIssues = nlohmann::json::array();
        for (const auto& issue : save_panel_.getModel().PolicyIssues()) {
            policyIssues.push_back(SavePolicyIssueJson(issue));
        }
        activeTabDetail["policy_issues"] = std::move(policyIssues);
        nlohmann::json metadataFields = nlohmann::json::array();
        for (const auto& field : save_panel_.getModel().MetadataFields()) {
            metadataFields.push_back(SaveMetadataFieldJson(field));
        }
        activeTabDetail["metadata_fields"] = std::move(metadataFields);
        nlohmann::json slotDescriptors = nlohmann::json::array();
        for (const auto& descriptor : save_panel_.getModel().SlotDescriptors()) {
            slotDescriptors.push_back(SaveSlotDescriptorJson(descriptor));
        }
        activeTabDetail["slot_descriptors"] = std::move(slotDescriptors);
        activeTabDetail["recovery_diagnostics"] =
            SaveRecoveryDiagnosticsJson(save_panel_.getModel().RecoveryDiagnostics());
        activeTabDetail["serialization_schema"] =
            SaveSerializationSchemaJson(save_panel_.getModel().SerializationSchema());
        nlohmann::json rows = nlohmann::json::array();
        for (const auto& row : save_panel_.getModel().VisibleRows()) {
            rows.push_back(SaveRowJson(row));
            if (save_panel_.getModel().SelectedSlotId().has_value() &&
                row.slot_id == *save_panel_.getModel().SelectedSlotId()) {
                activeTabDetail["selected_row"] = SaveRowJson(row);
            }
        }
        activeTabDetail["visible_rows"] = std::move(rows);
        break;
    }
    case DiagnosticsTab::EventAuthority: {
        const auto& snapshot = event_authority_panel_.lastRenderSnapshot();
        activeTabDetail["event_id_filter"] = snapshot.event_id_filter;
        activeTabDetail["level_filter"] = snapshot.level_filter;
        activeTabDetail["mode_filter"] = snapshot.mode_filter;
        activeTabDetail["visible_rows"] = snapshot.visible_rows;
        activeTabDetail["warning_count"] = snapshot.warning_count;
        activeTabDetail["error_count"] = snapshot.error_count;
        activeTabDetail["has_data"] = snapshot.has_data;
        activeTabDetail["has_selection"] = snapshot.has_selection;
        activeTabDetail["can_select_next_row"] = snapshot.can_select_next_row;
        activeTabDetail["can_select_previous_row"] = snapshot.can_select_previous_row;
        nlohmann::json visibleRows = nlohmann::json::array();
        for (const auto& row : snapshot.visible_row_entries) {
            visibleRows.push_back(EventAuthorityRowJson(row));
        }
        activeTabDetail["visible_row_entries"] = std::move(visibleRows);
        if (snapshot.selected_row_index.has_value()) {
            activeTabDetail["selected_row_index"] = snapshot.selected_row_index.value();
        }
        if (snapshot.selected_row.has_value()) {
            activeTabDetail["selected_row"] = EventAuthorityRowJson(snapshot.selected_row.value());
        }
        if (snapshot.selected_navigation_target.has_value()) {
            activeTabDetail["selected_navigation_target"] =
                EventAuthorityTargetJson(snapshot.selected_navigation_target.value());
        }
        break;
    }
    case DiagnosticsTab::MessageText: {
        if (message_panel_.hasRenderedFrame()) {
            const auto& snapshot = message_panel_.lastRenderSnapshot();
            activeTabDetail["message_summary"] = MessageInspectorSummaryJson(snapshot.summary);
            activeTabDetail["selected_page_id"] = !snapshot.selected_page_id.empty()
                                                      ? nlohmann::json(snapshot.selected_page_id)
                                                      : nlohmann::json(nullptr);
            activeTabDetail["has_data"] = snapshot.has_data;
            activeTabDetail["route_filter"] = snapshot.route_filter.has_value() ? nlohmann::json("set") : nlohmann::json(nullptr);
            activeTabDetail["show_issues_only"] = snapshot.show_issues_only;

            nlohmann::json visibleRows = nlohmann::json::array();
            for (const auto& row : snapshot.visible_rows) {
                visibleRows.push_back(MessageInspectorRowJson(row));
            }
            activeTabDetail["visible_rows"] = std::move(visibleRows);

            nlohmann::json issues = nlohmann::json::array();
            for (const auto& issue : snapshot.issues) {
                issues.push_back(MessageInspectorIssueJson(issue));
            }
            activeTabDetail["issues"] = std::move(issues);
        } else {
            activeTabDetail["message_summary"] = MessageInspectorSummaryJson(message_panel_.getModel().Summary());
            activeTabDetail["selected_page_id"] = message_panel_.getModel().SelectedPageId().has_value()
                                                      ? nlohmann::json(*message_panel_.getModel().SelectedPageId())
                                                      : nlohmann::json(nullptr);

            nlohmann::json visibleRows = nlohmann::json::array();
            for (const auto& row : message_panel_.getModel().VisibleRows()) {
                visibleRows.push_back(MessageInspectorRowJson(row));
            }
            activeTabDetail["visible_rows"] = std::move(visibleRows);

            nlohmann::json issues = nlohmann::json::array();
            for (const auto& issue : message_panel_.getModel().Issues()) {
                issues.push_back(MessageInspectorIssueJson(issue));
            }
            activeTabDetail["issues"] = std::move(issues);
        }
        break;
    }
    case DiagnosticsTab::MigrationWizard: {
        const auto& snapshot = migration_wizard_panel_.lastRenderSnapshot();
        activeTabDetail["total_files_processed"] = snapshot.total_files_processed;
        activeTabDetail["warning_count"] = snapshot.warning_count;
        activeTabDetail["error_count"] = snapshot.error_count;
        activeTabDetail["headline"] = snapshot.headline;
        activeTabDetail["has_data"] = snapshot.has_data;
        activeTabDetail["is_complete"] = snapshot.is_complete;
        activeTabDetail["summary_log_count"] = snapshot.summary_log_count;
        activeTabDetail["summary_logs"] = snapshot.summary_logs;
        activeTabDetail["selected_subsystem_id"] =
            snapshot.selected_subsystem_id.has_value() ? nlohmann::json(*snapshot.selected_subsystem_id) : nlohmann::json(nullptr);
        activeTabDetail["selected_subsystem_display_name"] = snapshot.selected_subsystem_display_name;
        activeTabDetail["selected_subsystem_processed_count"] = snapshot.selected_subsystem_processed_count;
        activeTabDetail["selected_subsystem_warning_count"] = snapshot.selected_subsystem_warning_count;
        activeTabDetail["selected_subsystem_error_count"] = snapshot.selected_subsystem_error_count;
        activeTabDetail["selected_subsystem_completed"] = snapshot.selected_subsystem_completed;
        activeTabDetail["selected_subsystem_summary_line"] = snapshot.selected_subsystem_summary_line;
        activeTabDetail["can_rerun_selected_subsystem"] = snapshot.can_rerun_selected_subsystem;
        activeTabDetail["can_clear_selected_subsystem"] = snapshot.can_clear_selected_subsystem;
        activeTabDetail["can_select_next_subsystem"] = snapshot.can_select_next_subsystem;
        activeTabDetail["can_select_previous_subsystem"] = snapshot.can_select_previous_subsystem;
        activeTabDetail["can_select_next_issue_subsystem"] = snapshot.can_select_next_issue_subsystem;
        activeTabDetail["can_select_previous_issue_subsystem"] = snapshot.can_select_previous_issue_subsystem;
        activeTabDetail["has_bound_project_data"] = snapshot.has_bound_project_data;
        activeTabDetail["can_rerun_bound_migration"] = snapshot.can_rerun_bound_migration;
        activeTabDetail["can_rerun_bound_selected_subsystem"] = snapshot.can_rerun_bound_selected_subsystem;
        activeTabDetail["can_save_report"] = snapshot.can_save_report;
        activeTabDetail["can_load_report"] = snapshot.can_load_report;
        activeTabDetail["exported_report_json"] = snapshot.exported_report_json;
        activeTabDetail["workflow_sections"] = snapshot.workflow_sections;
        activeTabDetail["primary_actions"] = MigrationWizardWorkflowPrimaryActionsJson(snapshot.primary_actions);
        activeTabDetail["report_io"] = MigrationWizardWorkflowReportIoJson(snapshot.report_io);
        activeTabDetail["bound_runtime_actions"] = MigrationWizardWorkflowBoundRuntimeJson(snapshot.bound_runtime_actions);
        activeTabDetail["subsystem_results"] = MigrationWizardSubsystemResultsJson(snapshot.subsystem_results);
        activeTabDetail["subsystem_cards"] = MigrationWizardSubsystemCardsJson(snapshot.subsystem_cards);
        activeTabDetail["selected_subsystem_card"] = MigrationWizardSelectedSubsystemCardJson(snapshot.selected_subsystem_card);
        break;
    }
    case DiagnosticsTab::Battle: {
        activeTabDetail["battle_summary"] = BattleInspectorSummaryJson(battle_panel_.getModel().Summary());
        activeTabDetail["selected_subject_id"] =
            battle_panel_.getModel().SelectedSubjectId().has_value() ? nlohmann::json(*battle_panel_.getModel().SelectedSubjectId()) : nlohmann::json(nullptr);

        nlohmann::json visibleRows = nlohmann::json::array();
        for (const auto& row : battle_panel_.getModel().VisibleRows()) {
            visibleRows.push_back(BattleInspectorRowJson(row));
        }
        activeTabDetail["visible_rows"] = std::move(visibleRows);

        nlohmann::json issues = nlohmann::json::array();
        for (const auto& issue : battle_panel_.getModel().Issues()) {
            issues.push_back(BattleInspectorIssueJson(issue));
        }
        activeTabDetail["issues"] = std::move(issues);

        activeTabDetail["preview"] = BattlePreviewSnapshotJson(battle_panel_.previewPanel().snapshot());
        nlohmann::json previewIssues = nlohmann::json::array();
        for (const auto& issue : battle_panel_.previewPanel().issues()) {
            previewIssues.push_back(BattlePreviewIssueJson(issue));
        }
        activeTabDetail["preview_issues"] = std::move(previewIssues);
        break;
    }
    case DiagnosticsTab::Audio: {
        const auto& snapshot = audio_panel_.lastRenderSnapshot();
        activeTabDetail["active_count"] = snapshot.active_count;
        activeTabDetail["issue_count"] = snapshot.issue_count;
        activeTabDetail["master_volume"] = snapshot.master_volume;
        activeTabDetail["has_data"] = snapshot.has_data;
        activeTabDetail["selected_handle"] =
            snapshot.selected_handle.has_value() ? nlohmann::json(*snapshot.selected_handle) : nlohmann::json(nullptr);
        activeTabDetail["can_select_next_row"] = snapshot.can_select_next_row;
        activeTabDetail["can_select_previous_row"] = snapshot.can_select_previous_row;
        nlohmann::json liveRows = nlohmann::json::array();
        for (const auto& row : snapshot.live_rows) {
            liveRows.push_back(AudioHandleRowJson(row));
        }
        activeTabDetail["live_rows"] = std::move(liveRows);
        activeTabDetail["selected_row"] =
            snapshot.selected_row.has_value() ? AudioHandleRowJson(*snapshot.selected_row) : nlohmann::json(nullptr);
        break;
    }
    case DiagnosticsTab::Menu: {
        if (menu_panel_ && menu_panel_->hasRenderedFrame()) {
            const auto& snapshot = menu_panel_->lastRenderSnapshot();
            activeTabDetail["menu_summary"] = MenuInspectorSummaryJson(snapshot.summary);
            activeTabDetail["command_id_filter"] = snapshot.command_id_filter;
            activeTabDetail["show_issues_only"] = snapshot.show_issues_only;
            activeTabDetail["has_data"] = snapshot.has_data;
            activeTabDetail["selected_command_id"] =
                snapshot.selected_command_id.has_value() ? nlohmann::json(*snapshot.selected_command_id) : nlohmann::json(nullptr);

            nlohmann::json visibleRows = nlohmann::json::array();
            for (const auto& row : snapshot.visible_rows) {
                visibleRows.push_back(MenuInspectorRowJson(row));
            }
            activeTabDetail["visible_rows"] = std::move(visibleRows);

            if (snapshot.selected_row.has_value()) {
                activeTabDetail["selected_row"] = MenuInspectorRowJson(*snapshot.selected_row);
            }

            nlohmann::json issues = nlohmann::json::array();
            for (const auto& issue : snapshot.issues) {
                issues.push_back(MenuInspectorIssueJson(issue));
            }
            activeTabDetail["issues"] = std::move(issues);
        } else if (menu_model_) {
            activeTabDetail["menu_summary"] = MenuInspectorSummaryJson(menu_model_->Summary());
        }
        if (menu_preview_panel_) {
            nlohmann::json preview{
                {"title", menu_preview_panel_->GetTitle()},
                {"visible", menu_preview_panel_->IsVisible()},
            };
            if (menu_preview_panel_->hasRenderedFrame()) {
                const auto& snapshot = menu_preview_panel_->lastRenderSnapshot();
                preview["has_data"] = snapshot.has_data;
                preview["active_scene_id"] = snapshot.active_scene_id;
                preview["last_blocked_command_id"] = snapshot.last_blocked_command_id;
                preview["last_blocked_reason"] = snapshot.last_blocked_reason;
                nlohmann::json visiblePanes = nlohmann::json::array();
                for (const auto& pane : snapshot.visible_panes) {
                    visiblePanes.push_back(MenuPreviewPaneJson(pane));
                }
                preview["visible_panes"] = std::move(visiblePanes);
            }
            activeTabDetail["preview"] = std::move(preview);
        }
        break;
    }
    case DiagnosticsTab::Abilities: {
        nlohmann::json abilities = nlohmann::json::array();
        for (const auto& ability : ability_panel_.getModel().getAbilities()) {
            abilities.push_back(AbilityInfoJson(ability));
        }
        activeTabDetail["abilities"] = std::move(abilities);

        nlohmann::json activeTags = nlohmann::json::array();
        for (const auto& tag : ability_panel_.getModel().getActiveTags()) {
            activeTags.push_back(ActiveTagInfoJson(tag));
        }
        activeTabDetail["active_tags"] = std::move(activeTags);
        const auto& snapshot = ability_panel_.getRenderSnapshot();
        activeTabDetail["selected_ability_id"] = snapshot.selected_ability_id;
        activeTabDetail["selected_ability_can_activate"] = snapshot.selected_ability_can_activate;
        activeTabDetail["selected_ability_blocking_reason"] = snapshot.selected_ability_blocking_reason;
        activeTabDetail["diagnostic_count"] = snapshot.diagnostic_count;
        activeTabDetail["latest_ability_id"] = snapshot.latest_ability_id;
        activeTabDetail["latest_outcome"] = snapshot.latest_outcome;
        activeTabDetail["can_preview_selected_ability"] =
            ability_runtime_ != nullptr && ability_runtime_mutable_ && !snapshot.selected_ability_id.empty();
        nlohmann::json diagnosticLines = nlohmann::json::array();
        for (const auto& line : snapshot.diagnostic_lines) {
            diagnosticLines.push_back(line);
        }
        activeTabDetail["diagnostic_lines"] = std::move(diagnosticLines);
        nlohmann::json draft = nlohmann::json::object();
        draft["has_draft"] = snapshot.draft_preview.has_draft;
        draft["ability_id"] = snapshot.draft_preview.ability_id;
        draft["cooldown_seconds"] = snapshot.draft_preview.cooldown_seconds;
        draft["mp_cost"] = snapshot.draft_preview.mp_cost;
        draft["effect_id"] = snapshot.draft_preview.effect_id;
        draft["effect_attribute"] = snapshot.draft_preview.effect_attribute;
        draft["effect_operation"] = snapshot.draft_preview.effect_operation;
        draft["effect_value"] = snapshot.draft_preview.effect_value;
        draft["effect_duration"] = snapshot.draft_preview.effect_duration;
        draft["preview_mp_before"] = snapshot.draft_preview.preview_mp_before;
        draft["preview_mp_after"] = snapshot.draft_preview.preview_mp_after;
        draft["preview_attribute_before"] = snapshot.draft_preview.preview_attribute_before;
        draft["preview_attribute_after"] = snapshot.draft_preview.preview_attribute_after;
        nlohmann::json pattern = nlohmann::json::object();
        pattern["name"] = snapshot.draft_preview.pattern_preview.name;
        pattern["is_valid"] = snapshot.draft_preview.pattern_preview.is_valid;
        pattern["issues"] = snapshot.draft_preview.pattern_preview.issues;
        pattern["grid_rows"] = snapshot.draft_preview.pattern_preview.grid_rows;
        draft["pattern_preview"] = std::move(pattern);
        activeTabDetail["draft"] = std::move(draft);
        nlohmann::json projectContent = nlohmann::json::object();
        projectContent["project_root"] = ability_project_root_.empty() ? "" : ability_project_root_.generic_string();
        projectContent["canonical_directory"] =
            ability_project_root_.empty()
                ? ""
                : urpg::ability::canonicalAbilityContentDirectory(ability_project_root_).generic_string();
        projectContent["asset_count"] = ability_project_assets_.size();
        projectContent["selected_asset_index"] = ability_selected_project_asset_index_.has_value()
                                                     ? static_cast<int64_t>(*ability_selected_project_asset_index_)
                                                     : -1;
        projectContent["can_load_selected"] = ability_selected_project_asset_index_.has_value();
        projectContent["can_apply_selected"] =
            ability_runtime_ != nullptr && ability_runtime_mutable_ && ability_selected_project_asset_index_.has_value();
        nlohmann::json assets = nlohmann::json::array();
        for (size_t i = 0; i < ability_project_assets_.size(); ++i) {
            const auto& asset = ability_project_assets_[i];
            assets.push_back({
                {"index", i},
                {"relative_path", asset.relative_path},
                {"absolute_path", asset.absolute_path.generic_string()},
                {"ability_id", asset.ability_id},
                {"selected", ability_selected_project_asset_index_.has_value() &&
                                 *ability_selected_project_asset_index_ == i},
            });
        }
        projectContent["assets"] = std::move(assets);
        activeTabDetail["project_content"] = std::move(projectContent);
        break;
    }
    case DiagnosticsTab::ProjectAudit: {
        const auto& snapshot = project_audit_panel_.lastRenderSnapshot();
        activeTabDetail["headline"] = snapshot.headline;
        activeTabDetail["summary_text"] = snapshot.summary;
        activeTabDetail["has_data"] = snapshot.has_data;
        activeTabDetail["issue_count"] = snapshot.issue_count;
        activeTabDetail["release_blocker_count"] = snapshot.release_blocker_count;
        activeTabDetail["export_blocker_count"] = snapshot.export_blocker_count;
        activeTabDetail["template_id"] = snapshot.template_id;
        activeTabDetail["template_status"] = snapshot.template_status;
        if (snapshot.asset_governance_issue_count.has_value()) {
            activeTabDetail["asset_governance_issue_count"] = *snapshot.asset_governance_issue_count;
        }
        if (snapshot.schema_governance_issue_count.has_value()) {
            activeTabDetail["schema_governance_issue_count"] = *snapshot.schema_governance_issue_count;
        }
        if (snapshot.project_artifact_issue_count.has_value()) {
            activeTabDetail["project_artifact_issue_count"] = *snapshot.project_artifact_issue_count;
        }
        if (snapshot.localization_evidence_issue_count.has_value()) {
            activeTabDetail["localization_evidence_issue_count"] = *snapshot.localization_evidence_issue_count;
        }
        if (snapshot.release_signoff_workflow_issue_count.has_value()) {
            activeTabDetail["release_signoff_workflow_issue_count"] =
                *snapshot.release_signoff_workflow_issue_count;
        }
        if (snapshot.signoff_artifact_issue_count.has_value()) {
            activeTabDetail["signoff_artifact_issue_count"] = *snapshot.signoff_artifact_issue_count;
        }
        if (snapshot.template_spec_artifact_issue_count.has_value()) {
            activeTabDetail["template_spec_artifact_issue_count"] = *snapshot.template_spec_artifact_issue_count;
        }
        if (snapshot.accessibility_artifact_issue_count.has_value()) {
            activeTabDetail["accessibility_artifact_issue_count"] = *snapshot.accessibility_artifact_issue_count;
        }
        if (snapshot.audio_artifact_issue_count.has_value()) {
            activeTabDetail["audio_artifact_issue_count"] = *snapshot.audio_artifact_issue_count;
        }
        if (snapshot.performance_artifact_issue_count.has_value()) {
            activeTabDetail["performance_artifact_issue_count"] = *snapshot.performance_artifact_issue_count;
        }
        nlohmann::json governance = nlohmann::json::object();
        if (snapshot.asset_report.has_value()) {
            nlohmann::json assetReport = nlohmann::json::object();
            if (snapshot.asset_report->path.has_value()) {
                assetReport["path"] = *snapshot.asset_report->path;
            }
            if (snapshot.asset_report->available.has_value()) {
                assetReport["available"] = *snapshot.asset_report->available;
            }
            if (snapshot.asset_report->usable.has_value()) {
                assetReport["usable"] = *snapshot.asset_report->usable;
            }
            if (snapshot.asset_report->issue_count.has_value()) {
                assetReport["issue_count"] = *snapshot.asset_report->issue_count;
            }
            if (snapshot.asset_report->normalized_count.has_value()) {
                assetReport["normalized_count"] = *snapshot.asset_report->normalized_count;
            }
            if (snapshot.asset_report->promoted_count.has_value()) {
                assetReport["promoted_count"] = *snapshot.asset_report->promoted_count;
            }
            if (snapshot.asset_report->promoted_visual_lane_count.has_value()) {
                assetReport["promoted_visual_lane_count"] = *snapshot.asset_report->promoted_visual_lane_count;
            }
            if (snapshot.asset_report->promoted_audio_lane_count.has_value()) {
                assetReport["promoted_audio_lane_count"] = *snapshot.asset_report->promoted_audio_lane_count;
            }
            if (snapshot.asset_report->wysiwyg_smoke_proof_count.has_value()) {
                assetReport["wysiwyg_smoke_proof_count"] = *snapshot.asset_report->wysiwyg_smoke_proof_count;
            }
            governance["asset_report"] = std::move(assetReport);
        }
        if (snapshot.schema_governance.has_value()) {
            nlohmann::json schemaGovernance = nlohmann::json::object();
            if (snapshot.schema_governance->schema_exists.has_value()) {
                schemaGovernance["schema_exists"] = *snapshot.schema_governance->schema_exists;
            }
            if (snapshot.schema_governance->changelog_exists.has_value()) {
                schemaGovernance["changelog_exists"] = *snapshot.schema_governance->changelog_exists;
            }
            if (snapshot.schema_governance->mentions_schema_version.has_value()) {
                schemaGovernance["mentions_schema_version"] = *snapshot.schema_governance->mentions_schema_version;
            }
            if (snapshot.schema_governance->schema_version.has_value()) {
                schemaGovernance["schema_version"] = *snapshot.schema_governance->schema_version;
            }
            governance["schema"] = std::move(schemaGovernance);
        }
        if (snapshot.project_schema_governance.has_value()) {
            nlohmann::json projectSchemaGovernance = nlohmann::json::object();
            if (snapshot.project_schema_governance->path.has_value()) {
                projectSchemaGovernance["path"] = *snapshot.project_schema_governance->path;
            }
            if (snapshot.project_schema_governance->available.has_value()) {
                projectSchemaGovernance["available"] = *snapshot.project_schema_governance->available;
            }
            if (snapshot.project_schema_governance->has_localization_section.has_value()) {
                projectSchemaGovernance["has_localization_section"] =
                    *snapshot.project_schema_governance->has_localization_section;
            }
            if (snapshot.project_schema_governance->has_input_section.has_value()) {
                projectSchemaGovernance["has_input_section"] =
                    *snapshot.project_schema_governance->has_input_section;
            }
            if (snapshot.project_schema_governance->has_export_section.has_value()) {
                projectSchemaGovernance["has_export_section"] =
                    *snapshot.project_schema_governance->has_export_section;
            }
            governance["project_schema"] = std::move(projectSchemaGovernance);
        }
        if (snapshot.localization_artifacts.has_value()) {
            governance["localization_artifacts"] =
                ProjectAuditArtifactGovernanceJson(*snapshot.localization_artifacts);
        }
        if (snapshot.localization_evidence.has_value()) {
            governance["localization_evidence"] =
                ProjectAuditLocalizationEvidenceJson(*snapshot.localization_evidence);
        }
        if (snapshot.input_artifacts.has_value()) {
            governance["input_artifacts"] = ProjectAuditArtifactGovernanceJson(*snapshot.input_artifacts);
        }
        if (snapshot.export_artifacts.has_value()) {
            governance["export_artifacts"] = ProjectAuditArtifactGovernanceJson(*snapshot.export_artifacts);
        }
        if (snapshot.accessibility_artifacts.has_value()) {
            governance["accessibility_artifacts"] =
                ProjectAuditArtifactGovernanceJson(*snapshot.accessibility_artifacts);
        }
        if (snapshot.audio_artifacts.has_value()) {
            governance["audio_artifacts"] = ProjectAuditArtifactGovernanceJson(*snapshot.audio_artifacts);
        }
        if (snapshot.performance_artifacts.has_value()) {
            governance["performance_artifacts"] =
                ProjectAuditArtifactGovernanceJson(*snapshot.performance_artifacts);
        }
        if (snapshot.release_signoff_workflow.has_value()) {
            governance["release_signoff_workflow"] =
                ProjectAuditArtifactGovernanceJson(*snapshot.release_signoff_workflow);
        }
        if (snapshot.signoff_artifacts.has_value()) {
            governance["signoff_artifacts"] =
                ProjectAuditRichArtifactGovernanceJson(*snapshot.signoff_artifacts);
        }
        if (snapshot.template_spec_artifacts.has_value()) {
            governance["template_spec_artifacts"] =
                ProjectAuditRichArtifactGovernanceJson(*snapshot.template_spec_artifacts);
        }
        if (!governance.empty()) {
            activeTabDetail["governance"] = std::move(governance);
        }
        nlohmann::json issues = nlohmann::json::array();
        for (const auto& issue : snapshot.issues) {
            const char* severity = "info";
            switch (issue.severity) {
            case ProjectAuditSeverity::Warning:
                severity = "warning";
                break;
            case ProjectAuditSeverity::Error:
                severity = "error";
                break;
            case ProjectAuditSeverity::Info:
            default:
                break;
            }
            issues.push_back({
                {"code", issue.code},
                {"title", issue.title},
                {"detail", issue.detail},
                {"severity", severity},
                {"blocks_release", issue.blocks_release},
                {"blocks_export", issue.blocks_export},
            });
        }
        activeTabDetail["issues"] = std::move(issues);
        break;
    }
    default:
        break;
    }

    root["tabs"] = nlohmann::json::array();

    for (const auto& summary : allTabSummaries()) {
        root["tabs"].push_back(TabSummaryJson(summary));
    }

    return root.dump();
}


} // namespace urpg::editor
