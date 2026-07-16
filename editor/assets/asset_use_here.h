#pragma once

#include <string>
#include <vector>

namespace urpg::editor {

enum class AssetUseTarget { MapTile, MapProp, MapEventSprite, BattleAsset, MenuImage, Portrait, Animation, Audio };

struct AssetUseHereRequest {
    std::string asset_id;
    std::string media_kind;
    std::vector<std::string> picker_targets;
    AssetUseTarget target = AssetUseTarget::MapTile;
    bool project_attached = false;
    bool owner_supports_undo = false;
};

struct AssetUseHereContract {
    bool compatible = false;
    std::string code;
    std::string target_id;
    std::string action_label;
    std::string owner_route;
    bool picker = false;
    bool drag_drop = false;
    bool keyboard = false;
    bool undo = false;
    std::string disabled_reason;
};

AssetUseHereContract resolveAssetUseHere(const AssetUseHereRequest& request);
const char* assetUseTargetId(AssetUseTarget target);

} // namespace urpg::editor
