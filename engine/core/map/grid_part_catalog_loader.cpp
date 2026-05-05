#include "engine/core/map/grid_part_catalog_loader.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <set>
#include <unordered_map>
#include <utility>

namespace urpg::map {

namespace {

void setError(std::string* error_message, std::string message) {
    if (error_message != nullptr) {
        *error_message = std::move(message);
    }
}

GridPartCategory gridPartCategoryFromString(const std::string& value) {
    static const std::unordered_map<std::string, GridPartCategory> categories = {
        {"Tile", GridPartCategory::Tile},
        {"Wall", GridPartCategory::Wall},
        {"Platform", GridPartCategory::Platform},
        {"Hazard", GridPartCategory::Hazard},
        {"Door", GridPartCategory::Door},
        {"Npc", GridPartCategory::Npc},
        {"Enemy", GridPartCategory::Enemy},
        {"TreasureChest", GridPartCategory::TreasureChest},
        {"SavePoint", GridPartCategory::SavePoint},
        {"Trigger", GridPartCategory::Trigger},
        {"CutsceneZone", GridPartCategory::CutsceneZone},
        {"Shop", GridPartCategory::Shop},
        {"QuestItem", GridPartCategory::QuestItem},
        {"Prop", GridPartCategory::Prop},
        {"LevelBlock", GridPartCategory::LevelBlock},
    };

    const auto found = categories.find(value);
    return found == categories.end() ? GridPartCategory::Prop : found->second;
}

GridPartLayer gridPartLayerFromString(const std::string& value) {
    static const std::unordered_map<std::string, GridPartLayer> layers = {
        {"Terrain", GridPartLayer::Terrain}, {"Decoration", GridPartLayer::Decoration},
        {"Collision", GridPartLayer::Collision}, {"Object", GridPartLayer::Object},
        {"Actor", GridPartLayer::Actor}, {"Trigger", GridPartLayer::Trigger},
        {"Region", GridPartLayer::Region}, {"Overlay", GridPartLayer::Overlay},
    };

    const auto found = layers.find(value);
    return found == layers.end() ? GridPartLayer::Object : found->second;
}

GridPartCollisionPolicy gridPartCollisionPolicyFromString(const std::string& value) {
    static const std::unordered_map<std::string, GridPartCollisionPolicy> policies = {
        {"None", GridPartCollisionPolicy::None},
        {"Solid", GridPartCollisionPolicy::Solid},
        {"Hazard", GridPartCollisionPolicy::Hazard},
        {"TriggerOnly", GridPartCollisionPolicy::TriggerOnly},
        {"Custom", GridPartCollisionPolicy::Custom},
    };

    const auto found = policies.find(value);
    return found == policies.end() ? GridPartCollisionPolicy::None : found->second;
}

GridPartRuleset gridPartRulesetFromString(const std::string& value) {
    static const std::unordered_map<std::string, GridPartRuleset> rulesets = {
        {"TopDownJRPG", GridPartRuleset::TopDownJRPG},
        {"SideScrollerAction", GridPartRuleset::SideScrollerAction},
        {"TacticalGrid", GridPartRuleset::TacticalGrid},
        {"DungeonRoomBuilder", GridPartRuleset::DungeonRoomBuilder},
        {"WorldMap", GridPartRuleset::WorldMap},
        {"TownHub", GridPartRuleset::TownHub},
        {"BattleArena", GridPartRuleset::BattleArena},
        {"CutsceneStage", GridPartRuleset::CutsceneStage},
    };

    const auto found = rulesets.find(value);
    return found == rulesets.end() ? GridPartRuleset::TopDownJRPG : found->second;
}

bool loadJson(const std::filesystem::path& catalog_path, nlohmann::json& payload, std::string* error_message) {
    std::ifstream stream(catalog_path, std::ios::binary);
    if (!stream) {
        setError(error_message, "catalog_open_failed");
        return false;
    }

    try {
        payload = nlohmann::json::parse(stream);
    } catch (const nlohmann::json::exception&) {
        setError(error_message, "catalog_json_parse_failed");
        return false;
    }
    return true;
}

bool addPayloadParts(const nlohmann::json& payload, GridPartCatalog& loaded, std::string* error_message) {
    if (!payload.contains("parts") || !payload["parts"].is_array()) {
        setError(error_message, "catalog_parts_missing");
        return false;
    }

    for (const auto& part : payload["parts"]) {
        if (!part.is_object() || !part.contains("partId") || !part["partId"].is_string()) {
            setError(error_message, "catalog_part_id_missing");
            return false;
        }

        GridPartDefinition definition;
        definition.part_id = part["partId"].get<std::string>();
        definition.display_name = part.value("displayName", definition.part_id);
        definition.description = part.value("description", "");
        definition.category = gridPartCategoryFromString(part.value("category", "Prop"));
        definition.default_layer = gridPartLayerFromString(part.value("defaultLayer", "Object"));
        definition.collision_policy = gridPartCollisionPolicyFromString(part.value("collisionPolicy", "None"));
        definition.asset_id = part.value("assetId", "");
        definition.prefab_path = part.value("prefabPath", "");
        definition.preview_path = part.value("previewPath", "");
        definition.source_image_path = part.value("sourceImagePath", "");
        definition.tile_id = part.value("tileId", 0);

        const auto footprint = part.value("footprint", nlohmann::json::object());
        definition.footprint.width = footprint.value("width", 1);
        definition.footprint.height = footprint.value("height", 1);
        definition.footprint.allow_overlap = footprint.value("allowOverlap", false);
        definition.footprint.blocks_navigation = footprint.value("blocksNavigation", false);

        const auto atlas_rect = part.value("atlasRect", nlohmann::json::object());
        definition.atlas_rect.x = atlas_rect.value("x", 0);
        definition.atlas_rect.y = atlas_rect.value("y", 0);
        definition.atlas_rect.width = atlas_rect.value("width", 0);
        definition.atlas_rect.height = atlas_rect.value("height", 0);

        for (const auto& ruleset : part.value("supportedRulesets", nlohmann::json::array())) {
            if (ruleset.is_string()) {
                definition.supported_rulesets.push_back(gridPartRulesetFromString(ruleset.get<std::string>()));
            }
        }
        if (definition.supported_rulesets.empty()) {
            definition.supported_rulesets.push_back(GridPartRuleset::TopDownJRPG);
        }

        for (const auto& tag : part.value("tags", nlohmann::json::array())) {
            if (tag.is_string()) {
                definition.tags.push_back(tag.get<std::string>());
            }
        }

        const auto properties = part.value("defaultProperties", nlohmann::json::object());
        for (const auto& [key, value] : properties.items()) {
            if (value.is_string()) {
                definition.default_properties[key] = value.get<std::string>();
            }
        }

        if (!loaded.addDefinition(std::move(definition))) {
            setError(error_message, "catalog_duplicate_part_id");
            return false;
        }
    }
    return true;
}

bool loadCatalogInto(const std::filesystem::path& catalog_path, GridPartCatalog& loaded, std::set<std::filesystem::path>& active,
                     std::string* error_message) {
    const auto absolute_path = std::filesystem::absolute(catalog_path).lexically_normal();
    if (active.contains(absolute_path)) {
        setError(error_message, "catalog_include_cycle");
        return false;
    }

    nlohmann::json payload;
    if (!loadJson(absolute_path, payload, error_message)) {
        return false;
    }

    active.insert(absolute_path);
    for (const auto& include : payload.value("includes", nlohmann::json::array())) {
        if (!include.is_string()) {
            setError(error_message, "catalog_include_path_invalid");
            active.erase(absolute_path);
            return false;
        }

        const auto include_path = absolute_path.parent_path() / include.get<std::string>();
        if (!loadCatalogInto(include_path, loaded, active, error_message)) {
            active.erase(absolute_path);
            return false;
        }
    }

    const bool added = addPayloadParts(payload, loaded, error_message);
    active.erase(absolute_path);
    return added;
}

} // namespace

bool LoadGridPartCatalogFromFile(const std::filesystem::path& catalog_path, GridPartCatalog& catalog,
                                 std::string* error_message) {
    if (error_message != nullptr) {
        error_message->clear();
    }

    GridPartCatalog loaded;
    std::set<std::filesystem::path> active;
    if (!loadCatalogInto(catalog_path, loaded, active, error_message)) {
        return false;
    }

    catalog = std::move(loaded);
    return true;
}

bool LoadGridPartCatalogFromProject(const std::filesystem::path& project_root, GridPartCatalog& catalog,
                                    const std::filesystem::path& relative_catalog_path, std::string* error_message) {
    return LoadGridPartCatalogFromFile(project_root / relative_catalog_path, catalog, error_message);
}

} // namespace urpg::map
