#include "engine/core/editor/editor_panel_registry.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <set>
#include <string>

namespace {

bool ContainsId(const std::vector<std::string>& ids, const std::string& id) {
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

} // namespace

TEST_CASE("Editor panel registry exposes canonical top-level panels", "[editor][panel][registry]") {
    const auto ids = urpg::editor::requiredTopLevelPanelIds();

    REQUIRE(ids.size() == 6);
    REQUIRE(ContainsId(ids, "diagnostics"));
    REQUIRE(ContainsId(ids, "assets"));
    REQUIRE(ContainsId(ids, "ability"));
    REQUIRE(ContainsId(ids, "patterns"));
    REQUIRE(ContainsId(ids, "mod"));
    REQUIRE(ContainsId(ids, "analytics"));

    const auto* patterns = urpg::editor::findEditorPanelRegistryEntry("patterns");
    REQUIRE(patterns != nullptr);
    REQUIRE(patterns->exposure == urpg::editor::EditorPanelExposure::TopLevel);
    REQUIRE(patterns->title == "Pattern Field Editor");
}

TEST_CASE("Editor panel registry documents every hidden compiled panel", "[editor][panel][registry]") {
    REQUIRE(urpg::editor::hiddenEditorPanelEntriesHaveReasons());

    std::set<std::string> seen;
    for (const auto& entry : urpg::editor::editorPanelRegistry()) {
        REQUIRE_FALSE(entry.id.empty());
        REQUIRE_FALSE(entry.title.empty());
        REQUIRE_FALSE(entry.category.empty());
        REQUIRE_FALSE(entry.owner.empty());
        REQUIRE(seen.insert(entry.id).second);

        if (entry.exposure != urpg::editor::EditorPanelExposure::TopLevel) {
            REQUIRE_FALSE(entry.reason.empty());
        }
    }
}

TEST_CASE("Editor panel registry classifies diagnostics and incubating workspaces", "[editor][panel][registry]") {
    const auto* saveInspector = urpg::editor::findEditorPanelRegistryEntry("save_inspector");
    REQUIRE(saveInspector != nullptr);
    REQUIRE(saveInspector->exposure == urpg::editor::EditorPanelExposure::NestedWorkspace);

    const auto* spatialAuthoring = urpg::editor::findEditorPanelRegistryEntry("spatial_authoring");
    REQUIRE(spatialAuthoring != nullptr);
    REQUIRE(spatialAuthoring->exposure == urpg::editor::EditorPanelExposure::Internal);

    const auto topLevel = urpg::editor::topLevelEditorPanels();
    REQUIRE(topLevel.size() == urpg::editor::requiredTopLevelPanelIds().size());
}

TEST_CASE("Editor smoke coverage follows every registered top-level panel", "[editor][panel][registry][smoke]") {
    const auto topLevelIds = urpg::editor::requiredTopLevelPanelIds();
    const auto smokeIds = urpg::editor::smokeRequiredEditorPanelIds();

    REQUIRE(smokeIds == topLevelIds);
    REQUIRE(ContainsId(smokeIds, "patterns"));
}
