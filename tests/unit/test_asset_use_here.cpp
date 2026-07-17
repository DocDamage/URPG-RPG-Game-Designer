#include "editor/assets/asset_use_here.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

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

TEST_CASE("Asset use-here input paths commit through one persistent owner and undo boundary",
          "[assets][use_here][owner][roundtrip]") {
    using namespace urpg::editor;
    const std::vector targets{AssetUseTarget::MapTile, AssetUseTarget::MapProp, AssetUseTarget::MapEventSprite,
                              AssetUseTarget::BattleAsset, AssetUseTarget::MenuImage, AssetUseTarget::Portrait,
                              AssetUseTarget::Animation, AssetUseTarget::Audio};
    const std::vector<std::string> pickerTargets = {"map_tile_selector", "map_prop_selector", "event_sprite_selector",
        "battle_asset_selector", "menu_image_selector", "portrait_selector", "animation_selector", "audio_selector"};
    const auto root = std::filesystem::temp_directory_path() /
        ("urpg_asset_use_here_" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));

    for (const auto target : targets) {
        for (const auto inputPath : {AssetUseInputPath::Picker, AssetUseInputPath::DragDrop,
                                     AssetUseInputPath::Keyboard}) {
            const auto contract = resolveAssetUseHere({"asset.ready", target == AssetUseTarget::Audio ? "audio" : "image",
                                                       pickerTargets, target, true, true});
            const auto document = root / (std::string(assetUseTargetId(target)) + ".json");
            std::string value = "asset.old";
            std::string before;
            std::uint64_t revision = 1;
            const auto persist = [&] {
                std::filesystem::create_directories(document.parent_path());
                std::ofstream output(document, std::ios::binary | std::ios::trunc);
                output << nlohmann::json{{"asset_id", value}}.dump(2);
                return output.good();
            };
            REQUIRE(persist());
            AssetUseHereService service;
            urpg::project::ProjectOperationParticipant owner{
                std::string("owner.") + assetUseTargetId(target), revision, [&] { return revision; },
                [&](std::string&) { before = value; return true; },
                [&](std::string&) { value = "asset.ready"; ++revision; return persist(); },
                [&] { value = before; --revision; (void)persist(); },
                [&](std::string&) { value = before; ++revision; return persist(); },
                [&] { return nlohmann::json{{"asset_id", value}}.dump(); }, document};
            REQUIRE(service.assign(contract, inputPath,
                                   std::string("use.") + assetUseTargetId(target) + "." +
                                       std::to_string(static_cast<int>(inputPath)),
                                   std::move(owner)).success);
            REQUIRE(service.undoLabel() == contract.action_label);
            std::ifstream assignedInput(document, std::ios::binary);
            REQUIRE(nlohmann::json::parse(assignedInput)["asset_id"] == "asset.ready");
            assignedInput.close();
            REQUIRE(service.undoLast().success);
            std::ifstream undoneInput(document, std::ios::binary);
            REQUIRE(nlohmann::json::parse(undoneInput)["asset_id"] == "asset.old");
            undoneInput.close();
            REQUIRE(service.redoLast().success);
        }
    }
    std::filesystem::remove_all(root);
}
