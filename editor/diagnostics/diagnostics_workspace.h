#pragma once

#include "editor/audio/audio_inspector_panel.h"
#include "editor/ability/ability_inspector_panel.h"
#include "editor/battle/battle_inspector_panel.h"
#include "editor/compat/compat_report_panel.h"
#include "editor/diagnostics/event_authority_panel.h"
#include "editor/diagnostics/migration_wizard_panel.h"
#include "editor/message/message_inspector_panel.h"
#include "editor/save/save_inspector_panel.h"
#include "editor/ui/menu_inspector_panel.h"
#include "editor/ui/menu_preview_panel.h"

namespace urpg::editor {

enum class DiagnosticsTab : uint8_t {
    Compat = 0,
    Save = 1,
    EventAuthority = 2,
    MessageText = 3,
    Battle = 4,
    Menu = 5,
    Audio = 6,
    MigrationWizard = 7,
    Abilities = 8,
};

struct DiagnosticsTabSummary {
    DiagnosticsTab tab = DiagnosticsTab::Compat;
    size_t item_count = 0;
    size_t issue_count = 0;
    bool has_data = false;
    bool active = false;
};

class DiagnosticsWorkspace {
public:
    DiagnosticsWorkspace();

    CompatReportPanel& compatPanel();
    const CompatReportPanel& compatPanel() const;

    SaveInspectorPanel& savePanel();
    const SaveInspectorPanel& savePanel() const;

    urpg::EventAuthorityPanel& eventAuthorityPanel();
    const urpg::EventAuthorityPanel& eventAuthorityPanel() const;
    MessageInspectorPanel& messagePanel();
    const MessageInspectorPanel& messagePanel() const;
    BattleInspectorPanel& battlePanel();
    const BattleInspectorPanel& battlePanel() const;
    MenuInspectorPanel& menuPanel();
    const MenuInspectorPanel& menuPanel() const;
    MenuPreviewPanel& menuPreviewPanel();
    const MenuPreviewPanel& menuPreviewPanel() const;
    AudioInspectorPanel& audioPanel();
    const AudioInspectorPanel& audioPanel() const;
    AbilityInspectorPanel& abilityPanel();
    const AbilityInspectorPanel& abilityPanel() const;
    MigrationWizardPanel& migrationWizardPanel();
    const MigrationWizardPanel& migrationWizardPanel() const;

    void bindSaveRuntime(const urpg::SaveCatalog& catalog,
                         const urpg::SaveSessionCoordinator& coordinator);
    void clearSaveRuntime();
    void bindMessageRuntime(const urpg::message::MessageFlowRunner& flow_runner,
                            const urpg::message::RichTextLayoutEngine& layout_engine);
    void clearMessageRuntime();
    void bindBattleRuntime(const urpg::battle::BattleFlowController& flow_controller,
                           const urpg::battle::BattleActionQueue& action_queue);
    void clearBattleRuntime();
    void bindMenuRuntime(const urpg::ui::MenuSceneGraph& scene_graph,
                         const urpg::ui::MenuCommandRegistry& registry,
                         const urpg::ui::MenuCommandRegistry::SwitchState& switches,
                         const urpg::ui::MenuCommandRegistry::VariableState& variables);
    void clearMenuRuntime();
    void bindAudioRuntime(const urpg::audio::AudioCore& core);
    void clearAudioRuntime();
    void clearMigrationWizardRuntime();
    void bindAbilityRuntime(const urpg::ability::AbilitySystemComponent& asc);
    void clearAbilityRuntime();
    void ingestEventAuthorityDiagnosticsJsonl(std::string_view diagnostics_jsonl);
    void clearEventAuthorityDiagnostics();

    void setActiveTab(DiagnosticsTab tab);
    DiagnosticsTab activeTab() const;

    void setVisible(bool visible);
    bool isVisible() const;

    DiagnosticsTabSummary tabSummary(DiagnosticsTab tab) const;
    std::vector<DiagnosticsTabSummary> allTabSummaries() const;
    std::string exportAsJson() const;

    void render();
    void refresh();
    void update();

private:
    void syncPanelVisibility();

    CompatReportPanel compat_panel_;
    SaveInspectorPanel save_panel_;
    urpg::EventAuthorityPanel event_authority_panel_;
    MessageInspectorPanel message_panel_;
    BattleInspectorPanel battle_panel_;
    
    std::shared_ptr<MenuInspectorModel> menu_model_;
    std::unique_ptr<MenuInspectorPanel> menu_panel_;
    std::unique_ptr<MenuPreviewPanel> menu_preview_panel_;

    AudioInspectorPanel audio_panel_;
    AbilityInspectorPanel ability_panel_;
    MigrationWizardPanel migration_wizard_panel_;
    DiagnosticsTab active_tab_ = DiagnosticsTab::Compat;
    bool visible_ = true;
};

} // namespace urpg::editor
