#pragma once

#include <string>
#include <vector>

namespace urpg::editor_app {

std::vector<std::string> editorAppRegisteredPanelFactoryIds();
std::vector<std::string> editorAppMissingReleasePanelFactoryIds();

} // namespace urpg::editor_app
