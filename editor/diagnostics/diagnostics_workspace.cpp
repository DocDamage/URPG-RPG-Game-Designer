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
    }
    return "compat";
}

} // namespace

DiagnosticsWorkspace::DiagnosticsWorkspace() {
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
        if (summary.issue_count == 0) {
            for (const auto& event : events) {
                if (event.severity != CompatEvent::Severity::INFO) {
                    ++summary.issue_count;
                }
            }
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
    } else {
        battle_panel_.render();
    }
}

void DiagnosticsWorkspace::refresh() {
    compat_panel_.refresh();
    save_panel_.refresh();
    event_authority_panel_.refresh();
    message_panel_.refresh();
    battle_panel_.refresh();
    syncPanelVisibility();
}

void DiagnosticsWorkspace::update() {
    compat_panel_.update();
    save_panel_.update();
    event_authority_panel_.update();
    message_panel_.update();
    battle_panel_.update();
    syncPanelVisibility();
}

void DiagnosticsWorkspace::syncPanelVisibility() {
    compat_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::Compat);
    save_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::Save);
    event_authority_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::EventAuthority);
    message_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::MessageText);
    battle_panel_.setVisible(visible_ && active_tab_ == DiagnosticsTab::Battle);
}

} // namespace urpg::editor
