#pragma once

#include "editor/diagnostics/project_health_model.h"

namespace urpg::editor {

class ProjectHealthPanel {
public:
    void setReportJson(const nlohmann::json& report);
    void clear();
    void render();

    const ProjectHealthModel& model() const { return model_; }
    const ProjectHealthSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }
    bool hasReportData() const { return model_.hasReportData(); }

    bool isVisible() const { return visible_; }
    void setVisible(bool visible) { visible_ = visible; }

private:
    ProjectHealthModel model_;
    ProjectHealthSnapshot last_render_snapshot_{};
    bool has_rendered_frame_ = false;
    bool visible_ = true;
};

} // namespace urpg::editor
