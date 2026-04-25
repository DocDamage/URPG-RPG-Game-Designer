#pragma once

#include "editor/localization/localization_workspace_model.h"

#include <string>

namespace urpg::editor::localization {

class LocalizationWorkspacePanel {
public:
    [[nodiscard]] static std::string snapshotLabel(const LocalizationWorkspaceModel& model);
};

} // namespace urpg::editor::localization
