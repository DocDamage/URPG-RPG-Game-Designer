#include "editor/diagnostics/diagnostics_workspace.h"
#include "engine/core/engine_context.h"
#include <nlohmann/json.hpp>
#include <iostream>

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
        {"reserved_slots", summary.reserved_slots},
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

DiagnosticsWorkspace::DiagnosticsWorkspace() {
    menu_model_ = std::make_shared<MenuInspectorModel>();
    menu_panel_ = std::make_unique<MenuInspectorPanel>(menu_model_);
    menu_preview_panel_ = std::make_unique<MenuPreviewPanel>();
    syncPanelVisibility();
}

CompatReportPanel& DiagnosticsWorkspace::compatPanel() {
    return compat_panel_;
}

const CompatReportPanel& DiagnosticsWorkspace::compatPanel() const {
    return compat_panel_;
}

SaveInspectorPanel& DiagnosticsWorkspace::savePanel() {
    return save_panel_;
}

const SaveInspectorPanel& DiagnosticsWorkspace::savePanel() const {
    return save_panel_;
}

urpg::EventAuthorityPanel& DiagnosticsWorkspace::eventAuthorityPanel() {
    return event_authority_panel_;
}

const urpg::EventAuthorityPanel& DiagnosticsWorkspace::eventAuthorityPanel() const {
    return event_authority_panel_;
}

MessageInspectorPanel& DiagnosticsWorkspace::messagePanel() {
    return message_panel_;
}

const MessageInspectorPanel& DiagnosticsWorkspace::messagePanel() const {
    return message_panel_;
}

BattleInspectorPanel& DiagnosticsWorkspace::battlePanel() {
    return battle_panel_;
}

const BattleInspectorPanel& DiagnosticsWorkspace::battlePanel() const {
    return battle_panel_;
}

MenuInspectorPanel& DiagnosticsWorkspace::menuPanel() {
    return *menu_panel_;
}

const MenuInspectorPanel& DiagnosticsWorkspace::menuPanel() const {
    return *menu_panel_;
}

MenuPreviewPanel& DiagnosticsWorkspace::menuPreviewPanel() {
    return *menu_preview_panel_;
}

const MenuPreviewPanel& DiagnosticsWorkspace::menuPreviewPanel() const {
    return *menu_preview_panel_;
}

AudioInspectorPanel& DiagnosticsWorkspace::audioPanel() {
    return audio_panel_;
}

const AudioInspectorPanel& DiagnosticsWorkspace::audioPanel() const {
    return audio_panel_;
}

MigrationWizardPanel& DiagnosticsWorkspace::migrationWizardPanel() {
    return migration_wizard_panel_;
}

const MigrationWizardPanel& DiagnosticsWorkspace::migrationWizardPanel() const {
    return migration_wizard_panel_;
}

AbilityInspectorPanel& DiagnosticsWorkspace::abilityPanel() {
    return ability_panel_;
}

const AbilityInspectorPanel& DiagnosticsWorkspace::abilityPanel() const {
    return ability_panel_;
}

void DiagnosticsWorkspace::bindSaveRuntime(const urpg::SaveCatalog& catalog,
                                           const urpg::SaveSessionCoordinator& coordinator) {
    save_panel_.bindRuntime(catalog, coordinator);
}

void DiagnosticsWorkspace::clearSaveRuntime() {
    save_panel_.clearRuntime();
}

void DiagnosticsWorkspace::bindMessageRuntime(const urpg::message::MessageFlowRunner& flow_runner,
                                              const urpg::message::RichTextLayoutEngine& layout_engine) {
    message_panel_.bindRuntime(flow_runner, layout_engine);
}

void DiagnosticsWorkspace::clearMessageRuntime() {
    message_panel_.clearRuntime();
}

bool DiagnosticsWorkspace::setMessageRouteFilter(std::optional<urpg::message::MessagePresentationMode> route_filter) {
    message_panel_.setRouteFilter(route_filter);
    message_panel_.update();
    return true;
}

bool DiagnosticsWorkspace::clearMessageRouteFilter() {
    return setMessageRouteFilter(std::nullopt);
}

bool DiagnosticsWorkspace::setMessageShowIssuesOnly(bool show_issues_only) {
    message_panel_.setShowIssuesOnly(show_issues_only);
    message_panel_.update();
    return true;
}

bool DiagnosticsWorkspace::selectMessageRow(size_t row_index) {
    return message_panel_.getModel().SelectRow(row_index);
}

void DiagnosticsWorkspace::bindBattleRuntime(const urpg::battle::BattleFlowController& flow_controller,
                                             const urpg::battle::BattleActionQueue& action_queue) {
    battle_panel_.bindRuntime(flow_controller, action_queue);
}

void DiagnosticsWorkspace::clearBattleRuntime() {
    battle_panel_.clearRuntime();
}

void DiagnosticsWorkspace::bindMenuRuntime(const urpg::ui::MenuSceneGraph& scene_graph,
                                           const urpg::ui::MenuCommandRegistry& registry,
                                           const urpg::ui::MenuCommandRegistry::SwitchState& switches,
                                           const urpg::ui::MenuCommandRegistry::VariableState& variables) {
    if (menu_model_) {
        menu_model_->LoadFromRuntime(scene_graph, registry, switches, variables);
    }
    if (menu_preview_panel_) {
        menu_preview_panel_->bindRuntime(scene_graph);
    }
}

void DiagnosticsWorkspace::clearMenuRuntime() {
    if (menu_model_) {
        menu_model_->Clear();
    }
    if (menu_preview_panel_) {
        menu_preview_panel_->clearRuntime();
    }
}

void DiagnosticsWorkspace::bindAudioRuntime(const urpg::audio::AudioCore& core) {
    audio_panel_.onRefreshRequested(core);
    refreshAudioSnapshotIfActive();
}

void DiagnosticsWorkspace::clearAudioRuntime() {
    audio_panel_.clear();
    refreshAudioSnapshotIfActive();
}

void DiagnosticsWorkspace::bindMigrationWizardRuntime(const nlohmann::json& project_data) {
    migration_wizard_panel_.onProjectUpdateRequested(project_data);
    refreshMigrationWizardSnapshotIfActive();
}

void DiagnosticsWorkspace::clearMigrationWizardRuntime() {
    migration_wizard_panel_.clear();
    refreshMigrationWizardSnapshotIfActive();
}

bool DiagnosticsWorkspace::selectMigrationWizardSubsystemResult(std::string_view subsystem_id) {
    const bool changed = migration_wizard_panel_.selectSubsystemResult(subsystem_id);
    if (changed) {
        refreshMigrationWizardSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::selectNextMigrationWizardSubsystemResult() {
    const bool changed = migration_wizard_panel_.selectNextSubsystemResult();
    if (changed) {
        refreshMigrationWizardSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::selectPreviousMigrationWizardSubsystemResult() {
    const bool changed = migration_wizard_panel_.selectPreviousSubsystemResult();
    if (changed) {
        refreshMigrationWizardSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::rerunMigrationWizardSubsystem(std::string_view subsystem_id, const nlohmann::json& project_data) {
    const bool changed = migration_wizard_panel_.rerunSubsystem(subsystem_id, project_data);
    if (changed) {
        refreshMigrationWizardSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::clearMigrationWizardSubsystemResult(std::string_view subsystem_id) {
    const bool changed = migration_wizard_panel_.clearSubsystemResult(subsystem_id);
    if (changed) {
        refreshMigrationWizardSnapshotIfActive();
    }
    return changed;
}

std::string DiagnosticsWorkspace::exportMigrationWizardReportJson() const {
    return migration_wizard_panel_.getModel()->getReportJson();
}

bool DiagnosticsWorkspace::saveMigrationWizardReportToFile(const std::string& path) {
    return migration_wizard_panel_.saveReportToFile(path);
}

bool DiagnosticsWorkspace::loadMigrationWizardReportFromFile(const std::string& path) {
    const bool loaded = migration_wizard_panel_.loadReportFromFile(path);
    refreshMigrationWizardSnapshotIfActive();
    return loaded;
}

void DiagnosticsWorkspace::bindAbilityRuntime(const urpg::ability::AbilitySystemComponent& asc) {
    ability_panel_.update(asc);
}

void DiagnosticsWorkspace::clearAbilityRuntime() {
    ability_panel_.clear();
}

void DiagnosticsWorkspace::ingestEventAuthorityDiagnosticsJsonl(std::string_view diagnostics_jsonl) {
    event_authority_panel_.ingestDiagnosticsJsonl(diagnostics_jsonl);
    refreshEventAuthoritySnapshotIfActive();
}

void DiagnosticsWorkspace::clearEventAuthorityDiagnostics() {
    event_authority_panel_.clearDiagnostics();
    refreshEventAuthoritySnapshotIfActive();
}

bool DiagnosticsWorkspace::setEventAuthorityEventIdFilter(std::string_view event_id_filter) {
    event_authority_panel_.setFilter(event_id_filter);
    refreshEventAuthoritySnapshotIfActive();
    return true;
}

bool DiagnosticsWorkspace::setEventAuthorityLevelFilter(std::string_view level_filter) {
    event_authority_panel_.setLevelFilter(level_filter);
    refreshEventAuthoritySnapshotIfActive();
    return true;
}

bool DiagnosticsWorkspace::setEventAuthorityModeFilter(std::string_view mode_filter) {
    event_authority_panel_.setModeFilter(mode_filter);
    refreshEventAuthoritySnapshotIfActive();
    return true;
}

bool DiagnosticsWorkspace::clearEventAuthorityFilters() {
    event_authority_panel_.setFilter({});
    event_authority_panel_.setLevelFilter({});
    event_authority_panel_.setModeFilter({});
    refreshEventAuthoritySnapshotIfActive();
    return true;
}

bool DiagnosticsWorkspace::selectEventAuthorityRow(size_t row_index) {
    const bool changed = event_authority_panel_.selectRow(row_index);
    if (changed) {
        renderEventAuthoritySnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::selectNextEventAuthorityRow() {
    const bool changed = event_authority_panel_.selectNextRow();
    if (changed) {
        renderEventAuthoritySnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::selectPreviousEventAuthorityRow() {
    const bool changed = event_authority_panel_.selectPreviousRow();
    if (changed) {
        renderEventAuthoritySnapshotIfActive();
    }
    return changed;
}

void DiagnosticsWorkspace::setActiveTab(DiagnosticsTab tab) {
    active_tab_ = tab;
    syncPanelVisibility();
    refreshActiveSnapshotBackedTabIfVisible();
}

DiagnosticsTab DiagnosticsWorkspace::activeTab() const {
    return active_tab_;
}

void DiagnosticsWorkspace::setVisible(bool visible) {
    visible_ = visible;
    syncPanelVisibility();
    refreshActiveSnapshotBackedTabIfVisible();
}

bool DiagnosticsWorkspace::isVisible() const {
    return visible_;
}

DiagnosticsTabSummary DiagnosticsWorkspace::tabSummary(DiagnosticsTab tab) const {
    DiagnosticsTabSummary summary;
    summary.tab = tab;
    summary.active = (active_tab_ == tab);

    switch (tab) {
    case DiagnosticsTab::Compat: {
        const auto pluginSummaries = compat_panel_.getModel().getAllPluginSummaries();
        const auto events = compat_panel_.getModel().getRecentEvents(1000);
        summary.item_count = pluginSummaries.size();
        summary.issue_count = 0;
        for (const auto& pluginSummary : pluginSummaries) {
            summary.issue_count += static_cast<size_t>(pluginSummary.warningCount);
            summary.issue_count += static_cast<size_t>(pluginSummary.errorCount);
        }
        summary.has_data = summary.item_count > 0 || !events.empty();
        break;
    }
    case DiagnosticsTab::Save: {
        const auto& saveSummary = save_panel_.getModel().Summary();
        summary.item_count = saveSummary.total_slots;
        summary.issue_count = saveSummary.corrupted_slots + saveSummary.recovery_slots;
        summary.has_data = summary.item_count > 0;
        break;
    }
    case DiagnosticsTab::EventAuthority: {
        const auto& rows = event_authority_panel_.getModel().VisibleRows();
        summary.item_count = rows.size();
        summary.issue_count = rows.size();
        summary.has_data = !rows.empty();
        break;
    }
    case DiagnosticsTab::MessageText: {
        const auto& model_summary = message_panel_.getModel().Summary();
        summary.item_count = model_summary.total_pages;
        summary.issue_count = model_summary.issue_count;
        summary.has_data = model_summary.total_pages > 0;
        break;
    }
    case DiagnosticsTab::Battle: {
        const auto& model_summary = battle_panel_.getModel().Summary();
        summary.item_count = model_summary.total_actions;
        summary.issue_count = model_summary.issue_count + battle_panel_.previewPanel().issues().size();
        summary.has_data = model_summary.active || model_summary.total_actions > 0 || model_summary.turn_count > 0;
        break;
    }
    case DiagnosticsTab::Menu: {
        if (menu_model_) {
            const auto& model_summary = menu_model_->Summary();
            summary.item_count = model_summary.total_commands;
            summary.issue_count = model_summary.issue_count;
            summary.has_data = !model_summary.active_scene_id.empty() || model_summary.total_panes > 0;
        }
        break;
    }
    case DiagnosticsTab::Audio: {
        const auto& model_summary = audio_panel_.getModel()->getSummary();
        summary.item_count = model_summary.activeCount;
        summary.issue_count = model_summary.issueCount;
        summary.has_data = model_summary.activeCount > 0;
        break;
    }
    case DiagnosticsTab::MigrationWizard: {
        const auto& report = migration_wizard_panel_.getModel()->getReport();
        summary.item_count = report.total_files_processed;
        summary.issue_count = report.warning_count + report.error_count;
        summary.has_data = report.is_complete || report.total_files_processed > 0;
        break;
    }
    case DiagnosticsTab::Abilities: {
        summary.item_count = ability_panel_.getModel().getAbilities().size();
        summary.issue_count = 0;
        summary.has_data = !ability_panel_.getModel().getAbilities().empty() || !ability_panel_.getModel().getActiveTags().empty();
        break;
    }
    }

    return summary;
}

std::vector<DiagnosticsTabSummary> DiagnosticsWorkspace::allTabSummaries() const {
    return {
        tabSummary(DiagnosticsTab::Compat),
        tabSummary(DiagnosticsTab::Save),
        tabSummary(DiagnosticsTab::EventAuthority),
        tabSummary(DiagnosticsTab::MessageText),
        tabSummary(DiagnosticsTab::Battle),
        tabSummary(DiagnosticsTab::Menu),
        tabSummary(DiagnosticsTab::Audio),
        tabSummary(DiagnosticsTab::MigrationWizard),
        tabSummary(DiagnosticsTab::Abilities),
    };
}

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
        activeTabDetail["selected_slot_id"] =
            save_panel_.getModel().SelectedSlotId().has_value() ? nlohmann::json(*save_panel_.getModel().SelectedSlotId())
                                                                : nlohmann::json(nullptr);
        nlohmann::json rows = nlohmann::json::array();
        for (const auto& row : save_panel_.getModel().VisibleRows()) {
            rows.push_back(SaveRowJson(row));
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
        break;
    }
    case DiagnosticsTab::MigrationWizard: {
        const auto& snapshot = migration_wizard_panel_.lastRenderSnapshot();
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
        activeTabDetail["can_save_report"] = snapshot.can_save_report;
        activeTabDetail["can_load_report"] = snapshot.can_load_report;
        activeTabDetail["exported_report_json"] = snapshot.exported_report_json;
        nlohmann::json subsystemResults = nlohmann::json::array();
        for (const auto& result : snapshot.subsystem_results) {
            subsystemResults.push_back(MigrationWizardSubsystemJson(result));
        }
        activeTabDetail["subsystem_results"] = std::move(subsystemResults);
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
        nlohmann::json liveRows = nlohmann::json::array();
        for (const auto& row : snapshot.live_rows) {
            liveRows.push_back(AudioHandleRowJson(row));
        }
        activeTabDetail["live_rows"] = std::move(liveRows);
        break;
    }
    case DiagnosticsTab::Menu: {
        if (menu_model_) {
            activeTabDetail["menu_summary"] = MenuInspectorSummaryJson(menu_model_->Summary());
            activeTabDetail["selected_command_id"] =
                menu_model_->SelectedCommandId().has_value() ? nlohmann::json(*menu_model_->SelectedCommandId()) : nlohmann::json(nullptr);

            nlohmann::json visibleRows = nlohmann::json::array();
            for (const auto& row : menu_model_->VisibleRows()) {
                visibleRows.push_back(MenuInspectorRowJson(row));
            }
            activeTabDetail["visible_rows"] = std::move(visibleRows);

            nlohmann::json issues = nlohmann::json::array();
            for (const auto& issue : menu_model_->Issues()) {
                issues.push_back(MenuInspectorIssueJson(issue));
            }
            activeTabDetail["issues"] = std::move(issues);
        }
        if (menu_preview_panel_) {
            activeTabDetail["preview"] = {
                {"title", menu_preview_panel_->GetTitle()},
                {"visible", menu_preview_panel_->IsVisible()},
            };
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

void DiagnosticsWorkspace::render() {
    if (!visible_) {
        return;
    }

    if (active_tab_ == DiagnosticsTab::Compat) {
        compat_panel_.render();
    } else if (active_tab_ == DiagnosticsTab::Save) {
        save_panel_.render();
    } else if (active_tab_ == DiagnosticsTab::EventAuthority) {
        event_authority_panel_.render();
    } else if (active_tab_ == DiagnosticsTab::MessageText) {
        message_panel_.render();
    } else if (active_tab_ == DiagnosticsTab::Battle) {
        battle_panel_.render();
    } else if (active_tab_ == DiagnosticsTab::Menu) {
        if (menu_panel_) {
            urpg::FrameContext context;
            context.dt = 0.016f;
            menu_panel_->Render(context);
            if (menu_preview_panel_) {
                menu_preview_panel_->Render(context);
            }
        }
    } else if (active_tab_ == DiagnosticsTab::Audio) {
        audio_panel_.render();
    } else if (active_tab_ == DiagnosticsTab::MigrationWizard) {
        migration_wizard_panel_.render();
    } else if (active_tab_ == DiagnosticsTab::Abilities) {
        ability_panel_.render();
    }
}

void DiagnosticsWorkspace::refresh() {
    compat_panel_.refresh();
    save_panel_.refresh();
    event_authority_panel_.refresh();
    message_panel_.refresh();
    battle_panel_.refresh();
    if (menu_preview_panel_) {
        menu_preview_panel_->refresh();
    }
    syncPanelVisibility();
}

void DiagnosticsWorkspace::update() {
    compat_panel_.update();
    save_panel_.update();
    event_authority_panel_.update();
    message_panel_.update();
    battle_panel_.update();
    if (menu_preview_panel_) {
        menu_preview_panel_->update();
    }
    syncPanelVisibility();
}

void DiagnosticsWorkspace::syncPanelVisibility() {
    compat_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::Compat);
    save_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::Save);
    event_authority_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::EventAuthority);
    message_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::MessageText);
    battle_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::Battle);
    if (menu_panel_) {
        menu_panel_->SetVisible(visible_ && active_tab_ == DiagnosticsTab::Menu);
    }
    if (menu_preview_panel_) {
        menu_preview_panel_->SetVisible(visible_ && active_tab_ == DiagnosticsTab::Menu);
    }
    audio_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::Audio);
    migration_wizard_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::MigrationWizard);
    ability_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::Abilities);
}

void DiagnosticsWorkspace::refreshActiveSnapshotBackedTabIfVisible() {
    if (!visible_) {
        return;
    }

    switch (active_tab_) {
    case DiagnosticsTab::EventAuthority:
        refreshEventAuthoritySnapshotIfActive();
        break;
    case DiagnosticsTab::Audio:
        refreshAudioSnapshotIfActive();
        break;
    case DiagnosticsTab::MigrationWizard:
        refreshMigrationWizardSnapshotIfActive();
        break;
    default:
        break;
    }
}

void DiagnosticsWorkspace::refreshEventAuthoritySnapshotIfActive() {
    if (visible_ && active_tab_ == DiagnosticsTab::EventAuthority) {
        event_authority_panel_.refresh();
        event_authority_panel_.render();
    }
}

void DiagnosticsWorkspace::renderEventAuthoritySnapshotIfActive() {
    if (visible_ && active_tab_ == DiagnosticsTab::EventAuthority) {
        event_authority_panel_.render();
    }
}

void DiagnosticsWorkspace::refreshAudioSnapshotIfActive() {
    if (visible_ && active_tab_ == DiagnosticsTab::Audio) {
        audio_panel_.render();
    }
}

void DiagnosticsWorkspace::refreshMigrationWizardSnapshotIfActive() {
    if (visible_ && active_tab_ == DiagnosticsTab::MigrationWizard) {
        migration_wizard_panel_.render();
    }
}

} // namespace urpg::editor
