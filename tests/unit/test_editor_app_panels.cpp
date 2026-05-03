#include "apps/editor/editor_app_panels.h"
#include "engine/core/editor/editor_panel_registry.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <set>
#include <string>
#include <vector>

namespace {

const std::vector<std::string>& CanonicalReleasePanelIds() {
    static const std::vector<std::string> ids = {"diagnostics", "assets",  "ability", "patterns",
                                                 "mod",         "analytics", "level_builder"};
    return ids;
}

} // namespace

TEST_CASE("editor app panels have a render factory for every release top-level panel", "[editor][app][panel]") {
    REQUIRE(urpg::editor_app::editorAppMissingReleasePanelFactoryIds().empty());

    const auto factoryIds = urpg::editor_app::editorAppRegisteredPanelFactoryIds();
    REQUIRE(std::find(factoryIds.begin(), factoryIds.end(), "level_builder") != factoryIds.end());
}

TEST_CASE("editor app panels expose exactly the canonical release factories", "[editor][app][panel]") {
    auto factoryIds = urpg::editor_app::editorAppRegisteredPanelFactoryIds();
    std::sort(factoryIds.begin(), factoryIds.end());

    auto releaseIds = urpg::editor::requiredTopLevelPanelIds();
    std::sort(releaseIds.begin(), releaseIds.end());

    auto canonicalIds = CanonicalReleasePanelIds();
    std::sort(canonicalIds.begin(), canonicalIds.end());

    REQUIRE(factoryIds == releaseIds);
    REQUIRE(factoryIds == canonicalIds);
    REQUIRE(std::set<std::string>(factoryIds.begin(), factoryIds.end()).size() == factoryIds.size());
}

TEST_CASE("editor app panels have route factories for release and nested showcase panels", "[editor][app][panel]") {
    REQUIRE(urpg::editor_app::editorAppMissingRoutablePanelFactoryIds().empty());

    const auto routableFactoryIds = urpg::editor_app::editorAppRoutablePanelFactoryIds();
    REQUIRE(std::find(routableFactoryIds.begin(), routableFactoryIds.end(), "message_inspector") !=
            routableFactoryIds.end());
    REQUIRE(std::find(routableFactoryIds.begin(), routableFactoryIds.end(), "event_authoring") !=
            routableFactoryIds.end());
    REQUIRE(std::find(routableFactoryIds.begin(), routableFactoryIds.end(), "showcase_crew_management") !=
            routableFactoryIds.end());
}
