#include "apps/editor/editor_app_panels.h"

#include "engine/core/editor/editor_panel_registry.h"

#include <algorithm>

namespace urpg::editor_app {

std::vector<std::string> editorAppRegisteredPanelFactoryIds() {
    return {"diagnostics", "assets", "ability", "patterns", "mod", "analytics", "level_builder"};
}

std::vector<std::string> editorAppMissingReleasePanelFactoryIds() {
    auto factoryIds = editorAppRegisteredPanelFactoryIds();
    std::sort(factoryIds.begin(), factoryIds.end());

    std::vector<std::string> missing;
    for (const auto& panelId : editor::requiredTopLevelPanelIds()) {
        if (!std::binary_search(factoryIds.begin(), factoryIds.end(), panelId)) {
            missing.push_back(panelId);
        }
    }
    return missing;
}

} // namespace urpg::editor_app
