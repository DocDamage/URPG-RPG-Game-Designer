#include "editor/assets/asset_use_here.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Asset use-here contract covers every golden-slice target and input path", "[assets][use_here]") {
    using namespace urpg::editor;
    const std::vector targets{AssetUseTarget::MapTile, AssetUseTarget::MapProp, AssetUseTarget::MapEventSprite,
                              AssetUseTarget::BattleAsset, AssetUseTarget::MenuImage, AssetUseTarget::Portrait,
                              AssetUseTarget::Animation, AssetUseTarget::Audio};
    const std::vector<std::string> pickerTargets = {"map_tile_selector", "map_prop_selector", "event_sprite_selector",
        "battle_asset_selector", "menu_image_selector", "portrait_selector", "animation_selector", "audio_selector"};
    for (const auto target : targets) {
        const auto contract = resolveAssetUseHere({"asset.ready", target == AssetUseTarget::Audio ? "audio" : "image",
                                                   pickerTargets, target, true, true});
        REQUIRE(contract.compatible);
        REQUIRE(contract.code == "asset_use_here_ready");
        REQUIRE_FALSE(contract.target_id.empty());
        REQUIRE_FALSE(contract.owner_route.empty());
        REQUIRE(contract.picker);
        REQUIRE(contract.drag_drop);
        REQUIRE(contract.keyboard);
        REQUIRE(contract.undo);
    }
}

TEST_CASE("Asset use-here contract refuses raw incompatible and non-undoable assignments", "[assets][use_here]") {
    using namespace urpg::editor;
    const auto raw = resolveAssetUseHere({"asset.raw", "image", {"map_tile_selector"},
                                          AssetUseTarget::MapTile, false, true});
    REQUIRE_FALSE(raw.compatible);
    REQUIRE(raw.code == "asset_use_requires_attachment");
    const auto incompatible = resolveAssetUseHere({"asset.image", "image", {"portrait_selector"},
                                                   AssetUseTarget::Audio, true, true});
    REQUIRE_FALSE(incompatible.compatible);
    REQUIRE(incompatible.code == "asset_use_target_incompatible");
    const auto noUndo = resolveAssetUseHere({"asset.image", "image", {"menu_image_selector"},
                                             AssetUseTarget::MenuImage, true, false});
    REQUIRE_FALSE(noUndo.compatible);
    REQUIRE(noUndo.code == "asset_use_owner_undo_missing");
}
