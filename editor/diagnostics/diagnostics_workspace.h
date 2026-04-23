#pragma once

#include "editor/audio/audio_inspector_panel.h"
#include "editor/ability/ability_inspector_panel.h"
#include "editor/battle/battle_inspector_panel.h"
#include "editor/compat/compat_report_panel.h"
#include "editor/diagnostics/event_authority_panel.h"
#include "editor/diagnostics/migration_wizard_panel.h"
#include "editor/diagnostics/project_audit_panel.h"
#include "editor/message/message_inspector_panel.h"
#include "editor/save/save_inspector_panel.h"
#include "editor/ui/menu_inspector_panel.h"
#include "editor/ui/menu_preview_panel.h"
#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_effect.h"
#include "engine/core/input/input_core.h"

#include <filesystem>

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
    ProjectAudit = 9,
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
    ProjectAuditPanel& projectAuditPanel();
    const ProjectAuditPanel& projectAuditPanel() const;

    void bindSaveRuntime(const urpg::SaveCatalog& catalog,
                         urpg::SaveSessionCoordinator& coordinator);
    void clearSaveRuntime();
    bool setSaveShowProblemSlotsOnly(bool show_problem_slots_only);
    bool setSaveIncludeAutosave(bool include_autosave);
    bool selectSaveRow(size_t row_index);
    bool setSavePolicyAutosaveEnabled(bool autosave_enabled);
    bool setSavePolicyAutosaveSlotId(int32_t autosave_slot_id);
    bool setSavePolicyRetentionLimits(size_t max_autosave_slots,
                                      size_t max_quicksave_slots,
                                      size_t max_manual_slots,
                                      bool prune_excess_on_save);
    bool applySavePolicyChanges();
    void bindMessageRuntime(const urpg::message::MessageFlowRunner& flow_runner,
                            const urpg::message::RichTextLayoutEngine& layout_engine);
    void clearMessageRuntime();
    bool setMessageRouteFilter(std::optional<urpg::message::MessagePresentationMode> route_filter);
    bool clearMessageRouteFilter();
    bool setMessageShowIssuesOnly(bool show_issues_only);
    bool selectMessageRow(size_t row_index);
    bool updateMessagePageBody(size_t row_index, const std::string& new_body);
    bool updateMessagePageSpeaker(size_t row_index, const std::string& new_speaker);
    bool removeMessagePage(size_t row_index);
    bool addMessagePage(const urpg::message::DialoguePage& page);
    bool applyMessageChangesToRuntime(urpg::message::MessageFlowRunner& runner);
    std::string exportMessageStateJson() const;
    bool saveMessageStateToFile(const std::string& path) const;
    bool loadMessageStateFromFile(const std::string& path, urpg::message::MessageFlowRunner& runner);
    void bindBattleRuntime(const urpg::battle::BattleFlowController& flow_controller,
                           const urpg::battle::BattleActionQueue& action_queue);

    template <typename SceneLike>
    requires requires(const SceneLike& battle_scene) {
        battle_scene.flowController();
        battle_scene.nativeActionQueue();
        battle_scene.buildDiagnosticsPreview();
    }
    void bindBattleRuntime(const SceneLike& battle_scene) {
        battle_panel_.bindRuntime(battle_scene);
    }

    void clearBattleRuntime();
    void bindMenuRuntime(urpg::ui::MenuSceneGraph& scene_graph,
                         const urpg::ui::MenuCommandRegistry& registry,
                         const urpg::ui::MenuCommandRegistry::SwitchState& switches,
                         const urpg::ui::MenuCommandRegistry::VariableState& variables);
    void clearMenuRuntime();
    bool setMenuCommandIdFilter(std::string_view command_id_filter);
    bool clearMenuCommandIdFilter();
    bool setMenuShowIssuesOnly(bool show_issues_only);
    bool selectMenuRow(size_t row_index);
    bool dispatchMenuPreviewAction(urpg::input::InputAction action);
    bool updateMenuCommandLabel(size_t row_index, std::string_view label);
    bool updateMenuCommandRoute(size_t row_index, urpg::MenuRouteTarget route, std::string_view custom_route_id);
    bool removeMenuCommand(size_t row_index);
    bool addMenuCommand(size_t pane_index, const urpg::MenuCommandMeta& command);
    bool applyMenuChangesToRuntime();
    std::string exportMenuStateJson() const;
    bool saveMenuStateToFile(const std::string& path);
    bool loadMenuStateFromFile(const std::string& path);
    void bindAudioRuntime(const urpg::audio::AudioCore& core);
    void clearAudioRuntime();
    bool selectNextAudioRow();
    bool selectPreviousAudioRow();
    void bindMigrationWizardRuntime(const nlohmann::json& project_data);
    void clearMigrationWizardRuntime();
    bool selectMigrationWizardSubsystemResult(std::string_view subsystem_id);
    bool selectNextMigrationWizardSubsystemResult();
    bool selectPreviousMigrationWizardSubsystemResult();
    bool selectNextMigrationWizardIssueSubsystemResult();
    bool selectPreviousMigrationWizardIssueSubsystemResult();
    bool rerunBoundMigrationWizard();
    bool rerunMigrationWizardSubsystem(std::string_view subsystem_id, const nlohmann::json& project_data);
    bool rerunBoundSelectedMigrationWizardSubsystem();
    bool rerunSelectedMigrationWizardSubsystem(const nlohmann::json& project_data);
    bool clearMigrationWizardSubsystemResult(std::string_view subsystem_id);
    bool clearSelectedMigrationWizardSubsystemResult();
    std::string exportMigrationWizardReportJson() const;
    bool saveMigrationWizardReportToFile(const std::string& path);
    bool loadMigrationWizardReportFromFile(const std::string& path);
    void beginAbilityDraftPreview();
    void bindAbilityRuntime(urpg::ability::AbilitySystemComponent& asc);
    void bindAbilityRuntime(const urpg::ability::AbilitySystemComponent& asc);
    void clearAbilityRuntime();
    bool selectAbilityRow(size_t row_index);
    bool previewSelectedAbility();
    bool applyAbilityDraftToRuntime();
    bool setAbilityDraftId(const std::string& ability_id);
    bool setAbilityDraftCooldownSeconds(float cooldown_seconds);
    bool setAbilityDraftMpCost(float mp_cost);
    bool setAbilityDraftEffectId(const std::string& effect_id);
    bool setAbilityDraftEffectAttribute(const std::string& effect_attribute);
    bool setAbilityDraftEffectOperation(urpg::ModifierOp effect_operation);
    bool setAbilityDraftEffectValue(float effect_value);
    bool setAbilityDraftEffectDuration(float effect_duration);
    bool setAbilityDraftPatternName(const std::string& pattern_name);
    bool applyAbilityDraftPatternPreset(const std::string& preset_id);
    bool toggleAbilityDraftPatternPoint(int32_t x, int32_t y);
    bool clearAbilityDraftPattern();
    std::string exportAbilityDraftStateJson() const;
    bool saveAbilityDraftStateToFile(const std::string& path) const;
    bool loadAbilityDraftStateFromFile(const std::string& path);
    bool setAbilityProjectRoot(const std::string& root_path);
    bool refreshAbilityProjectAssets();
    bool selectAbilityProjectAsset(size_t index);
    bool loadSelectedAbilityProjectAsset();
    bool applySelectedAbilityProjectAssetToRuntime();
    bool saveAbilityDraftToProjectContent(const std::string& file_name);
    void bindProjectAuditReport(const nlohmann::json& report);
    void clearProjectAuditReport();
    void ingestEventAuthorityDiagnosticsJsonl(std::string_view diagnostics_jsonl);
    void clearEventAuthorityDiagnostics();
    bool setEventAuthorityEventIdFilter(std::string_view event_id_filter);
    bool setEventAuthorityLevelFilter(std::string_view level_filter);
    bool setEventAuthorityModeFilter(std::string_view mode_filter);
    bool clearEventAuthorityFilters();
    bool selectEventAuthorityRow(size_t row_index);
    bool selectNextEventAuthorityRow();
    bool selectPreviousEventAuthorityRow();

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
    void refreshActiveSnapshotBackedTabIfVisible();
    void refreshEventAuthoritySnapshotIfActive();
    void renderEventAuthoritySnapshotIfActive();
    void refreshMenuSnapshotIfActive();
    void refreshAudioSnapshotIfActive();
    void refreshMessageInspectorSnapshotIfActive();
    void refreshMigrationWizardSnapshotIfActive();
    void refreshAbilityDraftAuthoringState();
    void rebuildAbilityDraftPreviewRuntime();
    void refreshAbilityProjectAssetCatalog();

    CompatReportPanel compat_panel_;
    SaveInspectorPanel save_panel_;
    urpg::EventAuthorityPanel event_authority_panel_;
    MessageInspectorPanel message_panel_;
    BattleInspectorPanel battle_panel_;
    
    std::shared_ptr<MenuInspectorModel> menu_model_;
    std::unique_ptr<MenuInspectorPanel> menu_panel_;
    std::unique_ptr<MenuPreviewPanel> menu_preview_panel_;
    urpg::ui::MenuSceneGraph* menu_scene_graph_ = nullptr;
    const urpg::ui::MenuCommandRegistry* menu_registry_ = nullptr;
    urpg::ui::MenuCommandRegistry::SwitchState menu_switches_;
    urpg::ui::MenuCommandRegistry::VariableState menu_variables_;

    AudioInspectorPanel audio_panel_;
    AbilityInspectorPanel ability_panel_;
    urpg::ability::AbilitySystemComponent* ability_runtime_ = nullptr;
    bool ability_runtime_mutable_ = false;
    std::optional<urpg::ability::AbilitySystemComponent> owned_ability_preview_runtime_;
    std::filesystem::path ability_project_root_;
    std::vector<urpg::ability::AuthoredAbilityAssetRecord> ability_project_assets_;
    std::optional<size_t> ability_selected_project_asset_index_;
    MigrationWizardPanel migration_wizard_panel_;
    ProjectAuditPanel project_audit_panel_;
    DiagnosticsTab active_tab_ = DiagnosticsTab::Compat;
    bool visible_ = true;
};

} // namespace urpg::editor
