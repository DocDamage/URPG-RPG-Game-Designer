#pragma once

#include "editor/battle/battle_inspector_model.h"
#include "editor/battle/battle_preview_panel.h"

namespace urpg::editor {

class BattleInspectorPanel {
public:
    BattleInspectorPanel() = default;

    void bindRuntime(const urpg::battle::BattleFlowController& flow_controller,
                     const urpg::battle::BattleActionQueue& action_queue);
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

private:
    const urpg::battle::BattleFlowController* flow_controller_ = nullptr;
    const urpg::battle::BattleActionQueue* action_queue_ = nullptr;
    BattleInspectorModel model_;
    BattlePreviewPanel preview_panel_;
    bool visible_ = true;
    bool show_issues_only_ = false;
    std::optional<std::string> subject_filter_;
};

} // namespace urpg::editor
