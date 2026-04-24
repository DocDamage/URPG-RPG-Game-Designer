#include "editor/diagnostics/project_health_panel.h"

namespace urpg::editor {

void ProjectHealthPanel::setReportJson(const nlohmann::json& report) {
    model_.ingestAuditReport(report);
}

void ProjectHealthPanel::clear() {
    model_.clear();
    last_render_snapshot_ = {};
    has_rendered_frame_ = false;
}

void ProjectHealthPanel::render() {
    if (!visible_) {
        return;
    }

    last_render_snapshot_ = model_.snapshot();
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
