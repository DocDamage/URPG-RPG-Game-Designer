#include "editor/assets/editor_asset_drag_payload.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Editor asset drag payload preserves provenance and rejects raw external durable drops", "[assets][drag_drop]") {
    urpg::editor::EditorAssetDragPayload payload{
        "local:hero", "", "image", "", 48, 64, urpg::editor::EditorAssetProvenanceState::RawExternal};
    urpg::editor::EditorAssetDragPayload parsed;
    REQUIRE(urpg::editor::deserializeEditorAssetDragPayload(urpg::editor::serializeEditorAssetDragPayload(payload), &parsed).accepted);
    REQUIRE(parsed.assetId == "local:hero");
    const auto rejected = urpg::editor::assessEditorAssetDrop(parsed, true);
    REQUIRE_FALSE(rejected.accepted);
    REQUIRE(rejected.code == "asset_drop_requires_attachment");
    REQUIRE(rejected.remediation.find("Attach To Project") != std::string::npos);
    parsed.provenance = urpg::editor::EditorAssetProvenanceState::Attached;
    parsed.projectPath = "content/assets/hero.png";
    REQUIRE(urpg::editor::assessEditorAssetDrop(parsed, true).accepted);
}
