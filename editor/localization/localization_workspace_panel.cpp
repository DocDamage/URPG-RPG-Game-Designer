#include "editor/localization/localization_workspace_panel.h"

namespace urpg::editor::localization {

std::string LocalizationWorkspacePanel::snapshotLabel(const LocalizationWorkspaceModel& model) {
    const auto snapshot = model.snapshot();
    return snapshot.missingKeys.empty() && snapshot.glossaryIssues.empty() && snapshot.layoutIssues.empty()
        ? "localization:ready"
        : "localization:diagnostics";
}

} // namespace urpg::editor::localization
