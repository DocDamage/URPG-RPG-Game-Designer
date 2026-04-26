#pragma once

#include "editor/battle/battle_inspector_model.h"
#include "editor/battle/battle_preview_panel.h"

namespace urpg::editor {

class BattleInspectorPanel {
public:
    struct RenderSnapshot {
        std::string status = "disabled";
        std::string message = "No battle runtime is bound.";
        std::string remediation = "Bind BattleFlowController and BattleActionQueue before rendering battle diagnostics.";
        bool runtime_bound = false;
        bool visible = true;
        bool can_refresh = false;
        size_t visible_row_count = 0;
        size_t issue_count = 0;
        std::string phase = "none";
    };

    BattleInspectorPanel() = default;

    void bindRuntime(const urpg::battle::BattleFlowController& flow_controller,
                     const urpg::battle::BattleActionQueue& action_queue);

    template <typename SceneLike>
    requires requires(const SceneLike& battle_scene) {
        battle_scene.flowController();
        battle_scene.nativeActionQueue();
        battle_scene.buildDiagnosticsPreview();
    }
    void bindRuntime(const SceneLike& battle_scene) {
        bindRuntime(battle_scene.flowController(), battle_scene.nativeActionQueue());
        setPreviewOverride(battle_scene.buildDiagnosticsPreview());
    }

    void clearRuntime();

    BattleInspectorModel& getModel();
    const BattleInspectorModel& getModel() const;

    BattlePreviewPanel& previewPanel();
    const BattlePreviewPanel& previewPanel() const;

    void setVisible(bool visible);
    bool isVisible() const;

    void setShowIssuesOnly(bool show_issues_only);
    void setSubjectFilter(std::optional<std::string> subject_filter);

    void render();
    void refresh();
    void update();
    const RenderSnapshot& lastRenderSnapshot() const;

private:
    struct PreviewOverride {
        urpg::battle::BattleDamageContext physical_preview;
        urpg::battle::BattleDamageContext magical_preview;
        int32_t party_agi = 100;
        int32_t troop_agi = 100;
    };

    void applyDefaultPreviewBinding();
    void applyPreviewBinding();
    void captureRenderSnapshot();

    template <typename OptionalPreview>
    void setPreviewOverride(OptionalPreview&& preview) {
        if (preview.has_value()) {
            preview_override_ = PreviewOverride{
                preview->physical_preview,
                preview->magical_preview,
                preview->party_agi,
                preview->troop_agi,
            };
            return;
        }
        preview_override_.reset();
    }

    const urpg::battle::BattleFlowController* flow_controller_ = nullptr;
    const urpg::battle::BattleActionQueue* action_queue_ = nullptr;
    std::optional<PreviewOverride> preview_override_;
    BattleInspectorModel model_;
    BattlePreviewPanel preview_panel_;
    RenderSnapshot last_render_snapshot_;
    bool visible_ = true;
    bool show_issues_only_ = false;
    std::optional<std::string> subject_filter_;
};

} // namespace urpg::editor
