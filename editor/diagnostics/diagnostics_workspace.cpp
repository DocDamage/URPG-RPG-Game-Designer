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
}

void DiagnosticsWorkspace::clearAudioRuntime() {
    audio_panel_.clear();
}

void DiagnosticsWorkspace::bindMigrationWizardRuntime(const nlohmann::json& project_data) {
    migration_wizard_panel_.onProjectUpdateRequested(project_data);
}

void DiagnosticsWorkspace::clearMigrationWizardRuntime() {
    migration_wizard_panel_.clear();
}

void DiagnosticsWorkspace::bindAbilityRuntime(const urpg::ability::AbilitySystemComponent& asc) {
    ability_panel_.update(asc);
}

void DiagnosticsWorkspace::clearAbilityRuntime() {
    ability_panel_.clear();
}

void DiagnosticsWorkspace::ingestEventAuthorityDiagnosticsJsonl(std::string_view diagnostics_jsonl) {
    event_authority_panel_.ingestDiagnosticsJsonl(diagnostics_jsonl);
}

void DiagnosticsWorkspace::clearEventAuthorityDiagnostics() {
    event_authority_panel_.clearDiagnostics();
}

void DiagnosticsWorkspace::setActiveTab(DiagnosticsTab tab) {
    active_tab_ = tab;
    syncPanelVisibility();
}

DiagnosticsTab DiagnosticsWorkspace::activeTab() const {
    return active_tab_;
}

void DiagnosticsWorkspace::setVisible(bool visible) {
    visible_ = visible;
    syncPanelVisibility();
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
    root["tabs"] = nlohmann::json::array();

    for (const auto& summary : allTabSummaries()) {
        root["tabs"].push_back({
            {"name", TabName(summary.tab)},
            {"item_count", summary.item_count},
            {"issue_count", summary.issue_count},
            {"has_data", summary.has_data},
            {"active", summary.active},
        });
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

} // namespace urpg::editor
