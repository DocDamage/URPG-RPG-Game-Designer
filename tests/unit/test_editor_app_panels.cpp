#include "apps/editor/editor_app_panels.h"
#include "engine/core/editor/editor_panel_registry.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

TEST_CASE("editor app panels have a render factory for every release top-level panel", "[editor][app][panel]") {
    REQUIRE(urpg::editor_app::editorAppMissingReleasePanelFactoryIds().empty());

    const auto factoryIds = urpg::editor_app::editorAppRegisteredPanelFactoryIds();
    REQUIRE(std::find(factoryIds.begin(), factoryIds.end(), "level_builder") != factoryIds.end());
}
