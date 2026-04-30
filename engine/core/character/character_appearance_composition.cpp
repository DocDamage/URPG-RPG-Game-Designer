#include "engine/core/character/character_appearance_composition.h"

#include <algorithm>
#include <unordered_map>

namespace urpg::character {

namespace {

std::string surfaceToString(CharacterCompositionSurface surface) {
    switch (surface) {
    case CharacterCompositionSurface::Portrait:
        return "portrait";
    case CharacterCompositionSurface::Field:
        return "field";
    case CharacterCompositionSurface::Battle:
        return "battle";
    }
    return "unknown";
}

CharacterAppearanceLayer makeLayer(const std::string& token,
                                   CharacterCompositionSurface surface,
                                   int32_t order,
                                   const std::string& slot) {
    const std::string surfaceName = surfaceToString(surface);
    return {
        surfaceName + "." + token,
        surfaceName + "/" + token,
        slot,
        order,
        false,
    };
}

std::string slotForToken(const std::string& token) {
    static const std::unordered_map<std::string, std::string> kSlots = {
        {"hair_short", "hair"},
        {"hair_long", "hair"},
        {"beard_short", "facial_hair"},
        {"armor_steel", "outfit"},
        {"cloak_travel", "outerwear"},
        {"hat_wizard", "headwear"},
    };
    const auto it = kSlots.find(token);
    return it == kSlots.end() ? "accessory" : it->second;
}

std::string slotForAssetId(const std::string& assetId) {
    const auto lastDot = assetId.find_last_of('.');
    if (lastDot == std::string::npos || lastDot + 1 >= assetId.size()) {
        return "promoted_part";
    }
    return assetId.substr(lastDot + 1);
}

void addBaseDiagnostics(const CharacterIdentity& identity, std::vector<std::string>& diagnostics) {
    if (identity.getPortraitId().empty() && identity.getPortraitAssetId().empty()) {
        diagnostics.push_back("missing_portrait_base_asset");
    }
    if (identity.getBodySpriteId().empty() && identity.getFieldSpriteAssetId().empty()) {
        diagnostics.push_back("missing_field_body_base_asset");
    }
    if (identity.getBodySpriteId().empty() && identity.getBattleSpriteAssetId().empty()) {
        diagnostics.push_back("missing_battle_body_base_asset");
    }
}

CharacterSurfaceComposition composeSurface(const CharacterIdentity& identity,
                                           CharacterCompositionSurface surface,
                                           const std::string& baseAssetId) {
    CharacterSurfaceComposition composition;
    composition.surface = surface;
    composition.base_asset_id = baseAssetId;
    if (!baseAssetId.empty()) {
        composition.layers.push_back({
            surfaceToString(surface) + ".base",
            baseAssetId,
            "base",
            0,
            true,
        });
    }

    int32_t order = 10;
    for (const auto& token : identity.getAppearanceTokens()) {
        composition.layers.push_back(makeLayer(token, surface, order, slotForToken(token)));
        order += 10;
    }
    for (const auto& assetId : identity.getLayeredPartAssetIds()) {
        composition.layers.push_back({
            surfaceToString(surface) + ".promoted." + std::to_string(order),
            assetId,
            slotForAssetId(assetId),
            order,
            false,
        });
        order += 10;
    }

    std::stable_sort(composition.layers.begin(), composition.layers.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.order < rhs.order;
    });
    return composition;
}

nlohmann::json surfaceToJson(const CharacterSurfaceComposition& surface) {
    nlohmann::json layers = nlohmann::json::array();
    for (const auto& layer : surface.layers) {
        layers.push_back({
            {"id", layer.id},
            {"source_asset_id", layer.source_asset_id},
            {"slot", layer.slot},
            {"order", layer.order},
            {"required", layer.required},
        });
    }

    return {
        {"surface", surfaceToString(surface.surface)},
        {"base_asset_id", surface.base_asset_id},
        {"layer_count", layers.size()},
        {"layers", layers},
    };
}

} // namespace

CharacterAppearanceComposition composeCharacterAppearance(const CharacterIdentity& identity) {
    CharacterAppearanceComposition composition;
    const auto portraitBase = identity.getPortraitAssetId().empty() ? identity.getPortraitId() : identity.getPortraitAssetId();
    const auto fieldBase =
        identity.getFieldSpriteAssetId().empty() ? identity.getBodySpriteId() : identity.getFieldSpriteAssetId();
    const auto battleBase =
        identity.getBattleSpriteAssetId().empty() ? identity.getBodySpriteId() : identity.getBattleSpriteAssetId();
    composition.portrait = composeSurface(identity, CharacterCompositionSurface::Portrait, portraitBase);
    composition.field = composeSurface(identity, CharacterCompositionSurface::Field, fieldBase);
    composition.battle = composeSurface(identity, CharacterCompositionSurface::Battle, battleBase);
    addBaseDiagnostics(identity, composition.diagnostics);
    composition.complete = composition.diagnostics.empty() && !composition.portrait.layers.empty() &&
                           !composition.field.layers.empty() && !composition.battle.layers.empty();
    return composition;
}

nlohmann::json characterAppearanceCompositionToJson(const CharacterAppearanceComposition& composition) {
    return {
        {"complete", composition.complete},
        {"diagnostic_count", composition.diagnostics.size()},
        {"diagnostics", composition.diagnostics},
        {"portrait", surfaceToJson(composition.portrait)},
        {"field", surfaceToJson(composition.field)},
        {"battle", surfaceToJson(composition.battle)},
    };
}

} // namespace urpg::character
