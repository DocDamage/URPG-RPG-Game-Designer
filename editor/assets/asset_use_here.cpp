#include "editor/assets/asset_use_here.h"

#include <algorithm>

namespace urpg::editor {
namespace {

const char* pickerTarget(const AssetUseTarget target) {
    switch (target) {
    case AssetUseTarget::MapTile: return "map_tile_selector";
    case AssetUseTarget::MapProp: return "map_prop_selector";
    case AssetUseTarget::MapEventSprite: return "event_sprite_selector";
    case AssetUseTarget::BattleAsset: return "battle_asset_selector";
    case AssetUseTarget::MenuImage: return "menu_image_selector";
    case AssetUseTarget::Portrait: return "portrait_selector";
    case AssetUseTarget::Animation: return "animation_selector";
    case AssetUseTarget::Audio: return "audio_selector";
    }
    return "asset_selector";
}

const char* route(const AssetUseTarget target) {
    switch (target) {
    case AssetUseTarget::MapTile:
    case AssetUseTarget::MapProp:
    case AssetUseTarget::MapEventSprite: return "map";
    case AssetUseTarget::BattleAsset: return "battle";
    case AssetUseTarget::MenuImage: return "menu_studio";
    case AssetUseTarget::Portrait: return "character_creator";
    case AssetUseTarget::Animation: return "sprite_animation";
    case AssetUseTarget::Audio: return "audio";
    }
    return "asset_library";
}

} // namespace

const char* assetUseTargetId(const AssetUseTarget target) {
    switch (target) {
    case AssetUseTarget::MapTile: return "map_tile";
    case AssetUseTarget::MapProp: return "map_prop";
    case AssetUseTarget::MapEventSprite: return "map_event_sprite";
    case AssetUseTarget::BattleAsset: return "battle_asset";
    case AssetUseTarget::MenuImage: return "menu_image";
    case AssetUseTarget::Portrait: return "portrait";
    case AssetUseTarget::Animation: return "animation";
    case AssetUseTarget::Audio: return "audio";
    }
    return "asset";
}

AssetUseHereContract resolveAssetUseHere(const AssetUseHereRequest& request) {
    AssetUseHereContract result;
    result.target_id = assetUseTargetId(request.target);
    result.action_label = std::string("Use for ") + result.target_id;
    result.owner_route = route(request.target);
    if (request.asset_id.empty()) {
        result.code = "asset_use_identity_missing";
        result.disabled_reason = "Choose an attached project asset first.";
        return result;
    }
    if (!request.project_attached) {
        result.code = "asset_use_requires_attachment";
        result.disabled_reason = "Attach the governed asset to this project before assigning it.";
        return result;
    }
    const auto requiredPicker = pickerTarget(request.target);
    if (std::find(request.picker_targets.begin(), request.picker_targets.end(), requiredPicker) ==
        request.picker_targets.end()) {
        result.code = "asset_use_target_incompatible";
        result.disabled_reason = "This asset is not compatible with the selected authoring target.";
        return result;
    }
    if ((request.target == AssetUseTarget::Audio) != (request.media_kind == "audio")) {
        result.code = "asset_use_media_kind_incompatible";
        result.disabled_reason = "The asset media kind does not match the selected target.";
        return result;
    }
    result.compatible = true;
    result.code = "asset_use_here_ready";
    result.picker = true;
    result.drag_drop = true;
    result.keyboard = true;
    result.undo = request.owner_supports_undo;
    if (!result.undo) {
        result.compatible = false;
        result.code = "asset_use_owner_undo_missing";
        result.disabled_reason = "The destination owner must expose undo before this assignment is enabled.";
    }
    return result;
}

} // namespace urpg::editor
