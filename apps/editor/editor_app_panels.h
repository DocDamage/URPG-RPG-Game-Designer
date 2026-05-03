#pragma once

#include <string>
#include <vector>

namespace urpg::editor_app {

std::vector<std::string> editorAppRegisteredPanelFactoryIds();
std::vector<std::string> editorAppRegisteredNestedPanelFactoryIds();
std::vector<std::string> editorAppRoutablePanelFactoryIds();
std::vector<std::string> editorAppMissingReleasePanelFactoryIds();
std::vector<std::string> editorAppMissingRoutablePanelFactoryIds();

} // namespace urpg::editor_app
