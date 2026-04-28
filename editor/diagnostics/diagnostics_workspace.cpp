#include "editor/diagnostics/diagnostics_workspace.h"
#include "engine/core/engine_context.h"
#include <nlohmann/json.hpp>
#include <iostream>

namespace urpg::editor {

namespace {

std::optional<std::pair<std::string, std::string>> ActiveMenuPreviewSelection(const urpg::ui::MenuSceneGraph* scene_graph) {
    if (!scene_graph) {
        return std::nullopt;
    }

    const auto active_scene = scene_graph->getActiveScene();
    if (!active_scene) {
        return std::nullopt;
    }

    for (const auto& pane : active_scene->getPanes()) {
        if (!pane.isActive) {
            continue;
        }

        const auto* selected_command = pane.getSelectedCommand();
        if (!selected_command) {
            return std::nullopt;
        }

        return std::make_pair(pane.id, selected_command->id);
    }

    return std::nullopt;
}

bool ApplyMenuInspectorSelectionToGraph(urpg::ui::MenuSceneGraph* scene_graph,
                                        const std::optional<urpg::editor::MenuInspectorRow>& selected_row) {
    if (!scene_graph || !selected_row.has_value()) {
        return false;
    }

    const auto active_scene = scene_graph->getActiveScene();
    if (!active_scene) {
        return false;
    }

    auto pane_handle = active_scene->getPane(selected_row->pane_id);
    if (!pane_handle.has_value() || !(*pane_handle)) {
        return false;
    }

    auto& panes = const_cast<std::vector<urpg::ui::MenuPane>&>(active_scene->getPanes());
    for (auto& pane : panes) {
        pane.isActive = (pane.id == selected_row->pane_id);
    }

    auto* pane = *pane_handle;
    for (size_t command_index = 0; command_index < pane->commands.size(); ++command_index) {
        if (pane->commands[command_index].id == selected_row->command_id) {
            pane->selectedCommandIndex = static_cast<int>(command_index);
            scene_graph->clearLastBlockedCommand();
            return true;
        }
    }

    return false;
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

ProjectAuditPanel& DiagnosticsWorkspace::projectAuditPanel() {
    return project_audit_panel_;
}

const ProjectAuditPanel& DiagnosticsWorkspace::projectAuditPanel() const {
    return project_audit_panel_;
}

ProjectHealthPanel& DiagnosticsWorkspace::projectHealthPanel() {
    return project_health_panel_;
}

const ProjectHealthPanel& DiagnosticsWorkspace::projectHealthPanel() const {
    return project_health_panel_;
}

void DiagnosticsWorkspace::bindSaveRuntime(const urpg::SaveCatalog& catalog,
                                           urpg::SaveSessionCoordinator& coordinator) {
    save_panel_.bindRuntime(catalog, coordinator);
}

void DiagnosticsWorkspace::clearSaveRuntime() {
    save_panel_.clearRuntime();
}

bool DiagnosticsWorkspace::setSaveShowProblemSlotsOnly(bool show_problem_slots_only) {
    save_panel_.setShowProblemSlotsOnly(show_problem_slots_only);
    save_panel_.update();
    return true;
}

bool DiagnosticsWorkspace::setSaveIncludeAutosave(bool include_autosave) {
    save_panel_.setIncludeAutosave(include_autosave);
    save_panel_.update();
    return true;
}

bool DiagnosticsWorkspace::selectSaveRow(size_t row_index) {
    return save_panel_.getModel().SelectRow(row_index);
}

bool DiagnosticsWorkspace::setSavePolicyAutosaveEnabled(bool autosave_enabled) {
    return save_panel_.setPolicyAutosaveEnabled(autosave_enabled);
}

bool DiagnosticsWorkspace::setSavePolicyAutosaveSlotId(int32_t autosave_slot_id) {
    return save_panel_.setPolicyAutosaveSlotId(autosave_slot_id);
}

bool DiagnosticsWorkspace::setSavePolicyRetentionLimits(size_t max_autosave_slots,
                                                        size_t max_quicksave_slots,
                                                        size_t max_manual_slots,
                                                        bool prune_excess_on_save) {
    return save_panel_.setPolicyRetentionLimits(
        max_autosave_slots, max_quicksave_slots, max_manual_slots, prune_excess_on_save);
}

bool DiagnosticsWorkspace::applySavePolicyChanges() {
    const bool applied = save_panel_.applyPolicyToRuntime();
    if (applied) {
        save_panel_.update();
    }
    return applied;
}

void DiagnosticsWorkspace::bindMessageRuntime(const urpg::message::MessageFlowRunner& flow_runner,
                                              const urpg::message::RichTextLayoutEngine& layout_engine) {
    message_panel_.bindRuntime(flow_runner, layout_engine);
    message_panel_.update();
    refreshMessageInspectorSnapshotIfActive();
}

void DiagnosticsWorkspace::clearMessageRuntime() {
    message_panel_.clearRuntime();
    message_panel_.clear();
    refreshMessageInspectorSnapshotIfActive();
}

bool DiagnosticsWorkspace::setMessageRouteFilter(std::optional<urpg::message::MessagePresentationMode> route_filter) {
    message_panel_.setRouteFilter(route_filter);
    message_panel_.update();
    refreshMessageInspectorSnapshotIfActive();
    return true;
}

bool DiagnosticsWorkspace::clearMessageRouteFilter() {
    return setMessageRouteFilter(std::nullopt);
}

bool DiagnosticsWorkspace::setMessageShowIssuesOnly(bool show_issues_only) {
    message_panel_.setShowIssuesOnly(show_issues_only);
    message_panel_.update();
    refreshMessageInspectorSnapshotIfActive();
    return true;
}

bool DiagnosticsWorkspace::selectMessageRow(size_t row_index) {
    const bool changed = message_panel_.getModel().SelectRow(row_index);
    if (changed) {
        refreshMessageInspectorSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::updateMessagePageBody(size_t row_index, const std::string& new_body) {
    const bool changed = message_panel_.updatePageBody(row_index, new_body);
    if (changed) {
        refreshMessageInspectorSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::updateMessagePageSpeaker(size_t row_index, const std::string& new_speaker) {
    const bool changed = message_panel_.getModel().updatePageSpeaker(row_index, new_speaker);
    if (changed) {
        refreshMessageInspectorSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::removeMessagePage(size_t row_index) {
    const bool changed = message_panel_.removePage(row_index);
    if (changed) {
        refreshMessageInspectorSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::addMessagePage(const urpg::message::DialoguePage& page) {
    const bool changed = message_panel_.addPage(page);
    if (changed) {
        refreshMessageInspectorSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::applyMessageChangesToRuntime(urpg::message::MessageFlowRunner& runner) {
    const bool applied = message_panel_.applyToRuntime(runner);
    if (applied) {
        refreshMessageInspectorSnapshotIfActive();
    }
    return applied;
}

void DiagnosticsWorkspace::bindBattleRuntime(const urpg::battle::BattleFlowController& flow_controller,
                                             const urpg::battle::BattleActionQueue& action_queue) {
    battle_panel_.bindRuntime(flow_controller, action_queue);
}

void DiagnosticsWorkspace::clearBattleRuntime() {
    battle_panel_.clearRuntime();
}

void DiagnosticsWorkspace::bindMenuRuntime(urpg::ui::MenuSceneGraph& scene_graph,
                                           const urpg::ui::MenuCommandRegistry& registry,
                                           const urpg::ui::MenuCommandRegistry::SwitchState& switches,
                                           const urpg::ui::MenuCommandRegistry::VariableState& variables) {
    menu_scene_graph_ = &scene_graph;
    menu_registry_ = &registry;
    menu_switches_ = switches;
    menu_variables_ = variables;
    if (menu_model_) {
        menu_model_->LoadFromRuntime(scene_graph, registry, switches, variables);
    }
    if (menu_preview_panel_) {
        menu_preview_panel_->bindRuntime(*menu_scene_graph_);
    }
    refreshMenuSnapshotIfActive();
}

void DiagnosticsWorkspace::clearMenuRuntime() {
    menu_scene_graph_ = nullptr;
    menu_registry_ = nullptr;
    menu_switches_.clear();
    menu_variables_.clear();
    if (menu_model_) {
        menu_model_->Clear();
    }
    if (menu_preview_panel_) {
        menu_preview_panel_->clearRuntime();
    }
    refreshMenuSnapshotIfActive();
}

bool DiagnosticsWorkspace::setMenuCommandIdFilter(std::string_view command_id_filter) {
    if (!menu_model_) {
        return false;
    }

    menu_model_->SetCommandIdFilter(std::string(command_id_filter));
    refreshMenuSnapshotIfActive();
    return true;
}

bool DiagnosticsWorkspace::clearMenuCommandIdFilter() {
    if (!menu_model_) {
        return false;
    }

    menu_model_->SetCommandIdFilter(std::nullopt);
    refreshMenuSnapshotIfActive();
    return true;
}

bool DiagnosticsWorkspace::setMenuShowIssuesOnly(bool show_issues_only) {
    if (!menu_model_) {
        return false;
    }

    menu_model_->SetShowIssuesOnly(show_issues_only);
    refreshMenuSnapshotIfActive();
    return true;
}

bool DiagnosticsWorkspace::selectMenuRow(size_t row_index) {
    if (!menu_model_) {
        return false;
    }

    const bool changed = menu_model_->SelectRow(row_index);
    if (changed) {
        ApplyMenuInspectorSelectionToGraph(menu_scene_graph_, menu_model_->SelectedRow());
        refreshMenuSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::dispatchMenuPreviewAction(urpg::input::InputAction action) {
    if (!menu_scene_graph_) {
        return false;
    }

    menu_scene_graph_->handleInput(action, urpg::input::ActionState::Pressed);
    if (menu_model_ && menu_registry_) {
        menu_model_->LoadFromRuntime(*menu_scene_graph_, *menu_registry_, menu_switches_, menu_variables_);
        if (const auto selected_row = ActiveMenuPreviewSelection(menu_scene_graph_); selected_row.has_value()) {
            menu_model_->SelectCommandRow(selected_row->first, selected_row->second);
        }
    }
    refreshMenuSnapshotIfActive();
    return true;
}

bool DiagnosticsWorkspace::updateMenuCommandLabel(size_t row_index, std::string_view label) {
    if (!menu_model_) {
        return false;
    }
    const bool changed = menu_model_->UpdateCommandLabel(row_index, std::string(label));
    if (changed) {
        refreshMenuSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::updateMenuCommandRoute(size_t row_index, urpg::MenuRouteTarget route, std::string_view custom_route_id) {
    if (!menu_model_) {
        return false;
    }
    const bool changed = menu_model_->UpdateCommandRoute(row_index, route, std::string(custom_route_id));
    if (changed) {
        refreshMenuSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::removeMenuCommand(size_t row_index) {
    if (!menu_model_) {
        return false;
    }
    const bool changed = menu_model_->RemoveCommand(row_index);
    if (changed) {
        refreshMenuSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::addMenuCommand(size_t pane_index, const urpg::MenuCommandMeta& command) {
    if (!menu_model_) {
        return false;
    }
    const bool changed = menu_model_->AddCommand(pane_index, command);
    if (changed) {
        refreshMenuSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::applyMenuChangesToRuntime() {
    if (!menu_model_ || !menu_scene_graph_) {
        return false;
    }
    const bool applied = menu_model_->ApplyToRuntime(*menu_scene_graph_);
    if (applied) {
        refreshMenuSnapshotIfActive();
    }
    return applied;
}

void DiagnosticsWorkspace::bindAudioRuntime(const urpg::audio::AudioCore& core) {
    audio_panel_.onRefreshRequested(core);
    refreshAudioSnapshotIfActive();
}

void DiagnosticsWorkspace::clearAudioRuntime() {
    audio_panel_.clear();
    refreshAudioSnapshotIfActive();
}

bool DiagnosticsWorkspace::selectNextAudioRow() {
    const bool changed = audio_panel_.getModel()->selectNextRow();
    if (changed) {
        refreshAudioSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::selectPreviousAudioRow() {
    const bool changed = audio_panel_.getModel()->selectPreviousRow();
    if (changed) {
        refreshAudioSnapshotIfActive();
    }
    return changed;
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

bool DiagnosticsWorkspace::selectNextMigrationWizardIssueSubsystemResult() {
    const bool changed = migration_wizard_panel_.selectNextIssueSubsystemResult();
    if (changed) {
        refreshMigrationWizardSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::selectPreviousMigrationWizardIssueSubsystemResult() {
    const bool changed = migration_wizard_panel_.selectPreviousIssueSubsystemResult();
    if (changed) {
        refreshMigrationWizardSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::rerunBoundMigrationWizard() {
    const bool changed = migration_wizard_panel_.rerunBoundProject();
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

bool DiagnosticsWorkspace::rerunBoundSelectedMigrationWizardSubsystem() {
    const bool changed = migration_wizard_panel_.rerunBoundSelectedSubsystem();
    if (changed) {
        refreshMigrationWizardSnapshotIfActive();
    }
    return changed;
}

bool DiagnosticsWorkspace::rerunSelectedMigrationWizardSubsystem(const nlohmann::json& project_data) {
    const bool changed = migration_wizard_panel_.rerunSelectedSubsystem(project_data);
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

bool DiagnosticsWorkspace::clearSelectedMigrationWizardSubsystemResult() {
    const bool changed = migration_wizard_panel_.clearSelectedSubsystemResult();
    if (changed) {
        refreshMigrationWizardSnapshotIfActive();
    }
    return changed;
}

void DiagnosticsWorkspace::beginAbilityDraftPreview() {
    rebuildAbilityDraftPreviewRuntime();
    if (ability_runtime_ != nullptr) {
        ability_panel_.selectDraftAbility(*ability_runtime_);
    }
    refreshActiveSnapshotBackedTabIfVisible();
}

void DiagnosticsWorkspace::bindAbilityRuntime(urpg::ability::AbilitySystemComponent& asc) {
    owned_ability_preview_runtime_.reset();
    ability_runtime_ = &asc;
    ability_runtime_mutable_ = true;
    ability_panel_.update(asc);
    refreshActiveSnapshotBackedTabIfVisible();
}

void DiagnosticsWorkspace::bindAbilityRuntime(const urpg::ability::AbilitySystemComponent& asc) {
    owned_ability_preview_runtime_.reset();
    ability_runtime_ = const_cast<urpg::ability::AbilitySystemComponent*>(&asc);
    ability_runtime_mutable_ = false;
    ability_panel_.update(asc);
    refreshActiveSnapshotBackedTabIfVisible();
}

void DiagnosticsWorkspace::clearAbilityRuntime() {
    owned_ability_preview_runtime_.reset();
    ability_runtime_ = nullptr;
    ability_runtime_mutable_ = false;
    ability_panel_.clear();
    refreshActiveSnapshotBackedTabIfVisible();
}

bool DiagnosticsWorkspace::selectAbilityRow(size_t row_index) {
    if (ability_runtime_ == nullptr) {
        return false;
    }

    const bool changed = ability_panel_.selectAbility(row_index, *ability_runtime_);
    if (changed) {
        refreshActiveSnapshotBackedTabIfVisible();
    }
    return changed;
}

bool DiagnosticsWorkspace::previewSelectedAbility() {
    if (ability_runtime_ == nullptr || !ability_runtime_mutable_) {
        return false;
    }

    const bool attempted = ability_panel_.previewSelectedAbility(*ability_runtime_);
    if (attempted) {
        refreshActiveSnapshotBackedTabIfVisible();
    }
    return attempted;
}

bool DiagnosticsWorkspace::applyAbilityDraftToRuntime() {
    if (ability_runtime_ == nullptr || !ability_runtime_mutable_) {
        return false;
    }

    ability_panel_.applyDraftToRuntime(*ability_runtime_);
    ability_panel_.update(*ability_runtime_);
    const bool selected = ability_panel_.selectDraftAbility(*ability_runtime_);
    refreshActiveSnapshotBackedTabIfVisible();
    return selected;
}

bool DiagnosticsWorkspace::setAbilityProjectRoot(const std::string& root_path) {
    const std::filesystem::path new_root = root_path;
    if (ability_project_root_ == new_root) {
        return refreshAbilityProjectAssets();
    }

    ability_project_root_ = new_root;
    refreshAbilityProjectAssetCatalog();
    refreshActiveSnapshotBackedTabIfVisible();
    return true;
}

bool DiagnosticsWorkspace::refreshAbilityProjectAssets() {
    refreshAbilityProjectAssetCatalog();
    refreshActiveSnapshotBackedTabIfVisible();
    return true;
}

bool DiagnosticsWorkspace::selectAbilityProjectAsset(size_t index) {
    if (index >= ability_project_assets_.size()) {
        return false;
    }
    if (ability_selected_project_asset_index_.has_value() && *ability_selected_project_asset_index_ == index) {
        return false;
    }
    ability_selected_project_asset_index_ = index;
    refreshActiveSnapshotBackedTabIfVisible();
    return true;
}

bool DiagnosticsWorkspace::loadSelectedAbilityProjectAsset() {
    if (!ability_selected_project_asset_index_.has_value()) {
        return false;
    }

    const auto& record = ability_project_assets_[*ability_selected_project_asset_index_];
    const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(record.absolute_path);
    if (!asset.has_value()) {
        return false;
    }

    ability_panel_.setDraftFromAsset(*asset);
    rebuildAbilityDraftPreviewRuntime();
    return true;
}

bool DiagnosticsWorkspace::applySelectedAbilityProjectAssetToRuntime() {
    if (ability_runtime_ == nullptr || !ability_runtime_mutable_ || !ability_selected_project_asset_index_.has_value()) {
        return false;
    }

    const auto& record = ability_project_assets_[*ability_selected_project_asset_index_];
    const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(record.absolute_path);
    if (!asset.has_value()) {
        return false;
    }

    ability_panel_.setDraftFromAsset(*asset);
    ability_panel_.applyDraftToRuntime(*ability_runtime_);
    ability_panel_.update(*ability_runtime_);
    const bool selected = ability_panel_.selectDraftAbility(*ability_runtime_);
    refreshActiveSnapshotBackedTabIfVisible();
    return selected;
}

bool DiagnosticsWorkspace::saveAbilityDraftToProjectContent(const std::string& file_name) {
    if (ability_project_root_.empty() || file_name.empty()) {
        return false;
    }

    const auto target_path = urpg::ability::canonicalAbilityContentDirectory(ability_project_root_) / file_name;
    if (!urpg::ability::saveAuthoredAbilityAssetToFile(ability_panel_.getDraftAsset(), target_path)) {
        return false;
    }

    refreshAbilityProjectAssetCatalog();
    const auto relative_path = std::filesystem::relative(target_path, ability_project_root_).generic_string();
    for (size_t i = 0; i < ability_project_assets_.size(); ++i) {
        if (ability_project_assets_[i].relative_path == relative_path) {
            ability_selected_project_asset_index_ = i;
            break;
        }
    }
    refreshActiveSnapshotBackedTabIfVisible();
    return true;
}

bool DiagnosticsWorkspace::setAbilityDraftId(const std::string& ability_id) {
    const bool changed = ability_panel_.setDraftAbilityId(ability_id);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::setAbilityDraftCooldownSeconds(float cooldown_seconds) {
    const bool changed = ability_panel_.setDraftCooldownSeconds(cooldown_seconds);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::setAbilityDraftMpCost(float mp_cost) {
    const bool changed = ability_panel_.setDraftMpCost(mp_cost);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::setAbilityDraftEffectId(const std::string& effect_id) {
    const bool changed = ability_panel_.setDraftEffectId(effect_id);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::setAbilityDraftEffectAttribute(const std::string& effect_attribute) {
    const bool changed = ability_panel_.setDraftEffectAttribute(effect_attribute);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::setAbilityDraftEffectOperation(urpg::ModifierOp effect_operation) {
    const bool changed = ability_panel_.setDraftEffectOperation(effect_operation);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::setAbilityDraftEffectValue(float effect_value) {
    const bool changed = ability_panel_.setDraftEffectValue(effect_value);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::setAbilityDraftEffectDuration(float effect_duration) {
    const bool changed = ability_panel_.setDraftEffectDuration(effect_duration);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::setAbilityDraftPatternName(const std::string& pattern_name) {
    const bool changed = ability_panel_.setDraftPatternName(pattern_name);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::applyAbilityDraftPatternPreset(const std::string& preset_id) {
    const bool changed = ability_panel_.applyDraftPatternPreset(preset_id);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::toggleAbilityDraftPatternPoint(int32_t x, int32_t y) {
    const bool changed = ability_panel_.toggleDraftPatternPoint(x, y);
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

bool DiagnosticsWorkspace::clearAbilityDraftPattern() {
    const bool changed = ability_panel_.clearDraftPattern();
    if (changed) {
        refreshAbilityDraftAuthoringState();
    }
    return changed;
}

void DiagnosticsWorkspace::bindProjectAuditReport(const nlohmann::json& report) {
    project_audit_panel_.setReportJson(report);
    project_health_panel_.setReportJson(report);
    refreshActiveSnapshotBackedTabIfVisible();
}

void DiagnosticsWorkspace::clearProjectAuditReport() {
    project_audit_panel_.clear();
    project_health_panel_.clear();
    refreshActiveSnapshotBackedTabIfVisible();
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
        summary.issue_count = saveSummary.corrupted_slots + saveSummary.recovery_slots +
                              save_panel_.getModel().PolicyValidation().error_count;
        summary.has_data = summary.item_count > 0 || save_panel_.getModel().PolicyValidation().issue_count > 0;
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
    case DiagnosticsTab::ProjectAudit: {
        summary.item_count = project_audit_panel_.currentIssueCount();
        summary.issue_count = project_audit_panel_.currentReleaseBlockerCount() + project_audit_panel_.currentExportBlockerCount();
        summary.has_data = project_audit_panel_.hasReportData();
        break;
    }
    case DiagnosticsTab::ProjectHealth: {
        const auto& snapshot = project_health_panel_.model().snapshot();
        summary.item_count = snapshot.fix_next.size();
        summary.issue_count = snapshot.release_blocker_count + snapshot.export_blocker_count;
        summary.has_data = project_health_panel_.hasReportData();
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
        tabSummary(DiagnosticsTab::ProjectAudit),
        tabSummary(DiagnosticsTab::ProjectHealth),
    };
}

ProjectAuditExportParityResult DiagnosticsWorkspace::compareProjectAuditExportParityReport(
    const nlohmann::json& cli_report) const {
    return compareProjectAuditExportParity(cli_report, nlohmann::json::parse(exportAsJson()));
}

std::string DiagnosticsWorkspace::exportProjectAuditParityJson(const nlohmann::json& cli_report) const {
    return projectAuditExportParityResultToJson(compareProjectAuditExportParityReport(cli_report)).dump(2);
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
    } else if (active_tab_ == DiagnosticsTab::ProjectAudit) {
        project_audit_panel_.render();
    } else if (active_tab_ == DiagnosticsTab::ProjectHealth) {
        project_health_panel_.render();
    }
}

void DiagnosticsWorkspace::refresh() {
    compat_panel_.refresh();
    save_panel_.refresh();
    event_authority_panel_.refresh();
    message_panel_.refresh();
    battle_panel_.refresh();
    if (menu_panel_) {
        menu_panel_->refresh();
    }
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
    if (ability_runtime_ != nullptr) {
        ability_panel_.update(*ability_runtime_);
    }
    if (menu_panel_) {
        menu_panel_->update();
    }
    if (menu_preview_panel_) {
        menu_preview_panel_->update();
    }
    syncPanelVisibility();
}

void DiagnosticsWorkspace::refreshAbilityDraftAuthoringState() {
    if (owned_ability_preview_runtime_.has_value()) {
        rebuildAbilityDraftPreviewRuntime();
        return;
    }

    if (ability_runtime_ != nullptr) {
        ability_panel_.update(*ability_runtime_);
    }
    refreshActiveSnapshotBackedTabIfVisible();
}

void DiagnosticsWorkspace::rebuildAbilityDraftPreviewRuntime() {
    owned_ability_preview_runtime_.emplace();
    ability_panel_.populateDraftPreviewRuntime(*owned_ability_preview_runtime_);
    ability_runtime_ = &*owned_ability_preview_runtime_;
    ability_runtime_mutable_ = true;
    ability_panel_.update(*ability_runtime_);
    ability_panel_.selectDraftAbility(*ability_runtime_);
    refreshActiveSnapshotBackedTabIfVisible();
}

void DiagnosticsWorkspace::refreshAbilityProjectAssetCatalog() {
    const auto previously_selected_path =
        ability_selected_project_asset_index_.has_value() && *ability_selected_project_asset_index_ < ability_project_assets_.size()
            ? std::optional<std::string>(ability_project_assets_[*ability_selected_project_asset_index_].relative_path)
            : std::nullopt;

    ability_project_assets_ = ability_project_root_.empty()
                                  ? std::vector<urpg::ability::AuthoredAbilityAssetRecord>{}
                                  : urpg::ability::discoverAuthoredAbilityAssets(ability_project_root_);
    ability_selected_project_asset_index_.reset();

    if (previously_selected_path.has_value()) {
        for (size_t i = 0; i < ability_project_assets_.size(); ++i) {
            if (ability_project_assets_[i].relative_path == *previously_selected_path) {
                ability_selected_project_asset_index_ = i;
                return;
            }
        }
    }

    if (!ability_project_assets_.empty()) {
        ability_selected_project_asset_index_ = 0;
    }
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
    project_audit_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::ProjectAudit);
    project_health_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::ProjectHealth);
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
    case DiagnosticsTab::Menu:
        refreshMenuSnapshotIfActive();
        break;
    case DiagnosticsTab::MigrationWizard:
        refreshMigrationWizardSnapshotIfActive();
        break;
    case DiagnosticsTab::Abilities:
        if (visible_ && active_tab_ == DiagnosticsTab::Abilities) {
            ability_panel_.render();
        }
        break;
    case DiagnosticsTab::ProjectAudit:
        if (visible_ && active_tab_ == DiagnosticsTab::ProjectAudit) {
            project_audit_panel_.render();
        }
        break;
    case DiagnosticsTab::ProjectHealth:
        if (visible_ && active_tab_ == DiagnosticsTab::ProjectHealth) {
            project_health_panel_.render();
        }
        break;
    case DiagnosticsTab::MessageText:
        refreshMessageInspectorSnapshotIfActive();
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

void DiagnosticsWorkspace::refreshMenuSnapshotIfActive() {
    if (visible_ && active_tab_ == DiagnosticsTab::Menu && menu_panel_) {
        urpg::FrameContext context{0.016f, 0};
        menu_panel_->Render(context);
        if (menu_preview_panel_) {
            menu_preview_panel_->Render(context);
        }
    }
}

void DiagnosticsWorkspace::refreshMessageInspectorSnapshotIfActive() {
    if (visible_ && active_tab_ == DiagnosticsTab::MessageText) {
        message_panel_.render();
    }
}

void DiagnosticsWorkspace::refreshMigrationWizardSnapshotIfActive() {
    if (visible_ && active_tab_ == DiagnosticsTab::MigrationWizard) {
        migration_wizard_panel_.render();
    }
}

} // namespace urpg::editor
