#include "engine/core/map/grid_part_catalog_loader.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <set>
#include <string>
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
        {"Terrain", GridPartLayer::Terrain},     {"Decoration", GridPartLayer::Decoration},
        {"Collision", GridPartLayer::Collision}, {"Object", GridPartLayer::Object},
        {"Actor", GridPartLayer::Actor},         {"Trigger", GridPartLayer::Trigger},
        {"Region", GridPartLayer::Region},       {"Overlay", GridPartLayer::Overlay},
    };

    const auto found = layers.find(value);
    return found == layers.end() ? GridPartLayer::Object : found->second;
}

GridPartCollisionPolicy gridPartCollisionPolicyFromString(const std::string& value) {
    static const std::unordered_map<std::string, GridPartCollisionPolicy> policies = {
        {"None", GridPartCollisionPolicy::None},     {"Solid", GridPartCollisionPolicy::Solid},
        {"Hazard", GridPartCollisionPolicy::Hazard}, {"TriggerOnly", GridPartCollisionPolicy::TriggerOnly},
        {"Custom", GridPartCollisionPolicy::Custom},
    };

    const auto found = policies.find(value);
    return found == policies.end() ? GridPartCollisionPolicy::None : found->second;
}

GridPartRuleset gridPartRulesetFromString(const std::string& value) {
    static const std::unordered_map<std::string, GridPartRuleset> rulesets = {
        {"TopDownJRPG", GridPartRuleset::TopDownJRPG},   {"SideScrollerAction", GridPartRuleset::SideScrollerAction},
        {"TacticalGrid", GridPartRuleset::TacticalGrid}, {"DungeonRoomBuilder", GridPartRuleset::DungeonRoomBuilder},
        {"WorldMap", GridPartRuleset::WorldMap},         {"TownHub", GridPartRuleset::TownHub},
        {"BattleArena", GridPartRuleset::BattleArena},   {"CutsceneStage", GridPartRuleset::CutsceneStage},
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

void copyStringProperty(const nlohmann::json& part, const char* json_key, const char* property_key,
                        GridPartDefinition& definition) {
    const auto found = part.find(json_key);
    if (found != part.end() && found->is_string()) {
        definition.default_properties[property_key] = found->get<std::string>();
    }
}

void copyAtlasRectProperties(const nlohmann::json& part, GridPartDefinition& definition) {
    const auto atlas_rect = part.find("atlasRect");
    if (atlas_rect == part.end() || !atlas_rect->is_object()) {
        return;
    }

    definition.default_properties["atlasRect.x"] = std::to_string(atlas_rect->value("x", 0));
    definition.default_properties["atlasRect.y"] = std::to_string(atlas_rect->value("y", 0));
    definition.default_properties["atlasRect.width"] = std::to_string(atlas_rect->value("width", 0));
    definition.default_properties["atlasRect.height"] = std::to_string(atlas_rect->value("height", 0));
}

bool addPayloadSmartPrefabs(const nlohmann::json& payload, GridPartCatalog& loaded, std::string* error_message) {
    const auto prefabs = payload.find("smartPrefabs");
    if (prefabs == payload.end()) {
        return true;
    }
    if (!prefabs->is_array()) {
        setError(error_message, "catalog_smart_prefabs_invalid");
        return false;
    }

    for (const auto& prefab_json : *prefabs) {
        if (!prefab_json.is_object() || !prefab_json.contains("prefabId") || !prefab_json["prefabId"].is_string() ||
            !prefab_json.contains("version") || !prefab_json["version"].is_string() ||
            !prefab_json.contains("operations") || !prefab_json["operations"].is_array()) {
            setError(error_message, "catalog_smart_prefab_incomplete");
            return false;
        }
        const auto has_invalid_optional_type = [&](const char* key, const nlohmann::json::value_t type) {
            const auto value = prefab_json.find(key);
            return value != prefab_json.end() && value->type() != type;
        };
        if (has_invalid_optional_type("displayName", nlohmann::json::value_t::string) ||
            has_invalid_optional_type("description", nlohmann::json::value_t::string) ||
            has_invalid_optional_type("dependencies", nlohmann::json::value_t::array) ||
            has_invalid_optional_type("conflictTags", nlohmann::json::value_t::array) ||
            has_invalid_optional_type("parameters", nlohmann::json::value_t::array)) {
            setError(error_message, "catalog_smart_prefab_field_type_invalid");
            return false;
        }
        GridPartSmartPrefab prefab;
        prefab.prefab_id = prefab_json["prefabId"].get<std::string>();
        prefab.version = prefab_json["version"].get<std::string>();
        prefab.display_name = prefab_json.value("displayName", prefab.prefab_id);
        prefab.description = prefab_json.value("description", "");
        if (prefab.prefab_id.empty() || prefab.version.empty()) {
            setError(error_message, "catalog_smart_prefab_identity_invalid");
            return false;
        }
        for (const auto& dependency : prefab_json.value("dependencies", nlohmann::json::array())) {
            if (!dependency.is_string() || dependency.get<std::string>().empty()) {
                setError(error_message, "catalog_smart_prefab_dependency_invalid");
                return false;
            }
            prefab.dependencies.push_back(dependency.get<std::string>());
        }
        for (const auto& tag : prefab_json.value("conflictTags", nlohmann::json::array())) {
            if (!tag.is_string() || tag.get<std::string>().empty()) {
                setError(error_message, "catalog_smart_prefab_conflict_tag_invalid");
                return false;
            }
            prefab.conflict_tags.push_back(tag.get<std::string>());
        }
        for (const auto& parameter_json : prefab_json.value("parameters", nlohmann::json::array())) {
            if (!parameter_json.is_object() || !parameter_json.contains("key") || !parameter_json["key"].is_string() ||
                !parameter_json.contains("defaultValue") || !parameter_json["defaultValue"].is_string()) {
                setError(error_message, "catalog_smart_prefab_parameter_invalid");
                return false;
            }
            const auto required = parameter_json.find("required");
            const auto allowed_values = parameter_json.find("allowedValues");
            if ((required != parameter_json.end() && !required->is_boolean()) ||
                (allowed_values != parameter_json.end() && !allowed_values->is_array())) {
                setError(error_message, "catalog_smart_prefab_parameter_type_invalid");
                return false;
            }
            GridPartPrefabParameter parameter;
            parameter.key = parameter_json["key"].get<std::string>();
            parameter.default_value = parameter_json["defaultValue"].get<std::string>();
            parameter.required = parameter_json.value("required", false);
            if (parameter.key.empty()) {
                setError(error_message, "catalog_smart_prefab_parameter_key_invalid");
                return false;
            }
            for (const auto& value : parameter_json.value("allowedValues", nlohmann::json::array())) {
                if (!value.is_string()) {
                    setError(error_message, "catalog_smart_prefab_parameter_value_invalid");
                    return false;
                }
                parameter.allowed_values.push_back(value.get<std::string>());
            }
            prefab.parameters.push_back(std::move(parameter));
        }
        for (const auto& operation_json : prefab_json["operations"]) {
            if (!operation_json.is_object() || !operation_json.contains("operationId") ||
                !operation_json["operationId"].is_string() || !operation_json.contains("partId") ||
                !operation_json["partId"].is_string()) {
                setError(error_message, "catalog_smart_prefab_operation_invalid");
                return false;
            }
            const auto offset_x = operation_json.find("offsetX");
            const auto offset_y = operation_json.find("offsetY");
            const auto offset_z = operation_json.find("offsetZ");
            const auto overrides_json = operation_json.find("propertyOverrides");
            if ((offset_x != operation_json.end() && !offset_x->is_number_integer()) ||
                (offset_y != operation_json.end() && !offset_y->is_number_integer()) ||
                (offset_z != operation_json.end() && !offset_z->is_number_integer()) ||
                (overrides_json != operation_json.end() && !overrides_json->is_object())) {
                setError(error_message, "catalog_smart_prefab_operation_type_invalid");
                return false;
            }
            GridPartPrefabOperation operation;
            operation.operation_id = operation_json["operationId"].get<std::string>();
            operation.part_id = operation_json["partId"].get<std::string>();
            operation.offset_x = operation_json.value("offsetX", 0);
            operation.offset_y = operation_json.value("offsetY", 0);
            operation.offset_z = operation_json.value("offsetZ", 0);
            if (operation.operation_id.empty() || operation.part_id.empty()) {
                setError(error_message, "catalog_smart_prefab_operation_identity_invalid");
                return false;
            }
            const auto overrides = operation_json.value("propertyOverrides", nlohmann::json::object());
            if (!overrides.is_object()) {
                setError(error_message, "catalog_smart_prefab_property_overrides_invalid");
                return false;
            }
            for (const auto& [key, value] : overrides.items()) {
                if (!value.is_string()) {
                    setError(error_message, "catalog_smart_prefab_property_override_invalid");
                    return false;
                }
                operation.property_overrides[key] = value.get<std::string>();
            }
            prefab.operations.push_back(std::move(operation));
        }
        if (!loaded.addSmartPrefab(std::move(prefab))) {
            setError(error_message, "catalog_smart_prefab_duplicate_or_empty");
            return false;
        }
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
        definition.tile_id = part.value("tileId", 0);

        const auto footprint = part.value("footprint", nlohmann::json::object());
        definition.footprint.width = footprint.value("width", 1);
        definition.footprint.height = footprint.value("height", 1);
        definition.footprint.allow_overlap = footprint.value("allowOverlap", false);
        definition.footprint.blocks_navigation = footprint.value("blocksNavigation", false);

        copyStringProperty(part, "previewPath", "previewPath", definition);
        copyStringProperty(part, "sourceImagePath", "sourceImagePath", definition);
        copyAtlasRectProperties(part, definition);

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

bool loadCatalogInto(const std::filesystem::path& catalog_path, GridPartCatalog& loaded,
                     std::set<std::filesystem::path>& active, std::string* error_message) {
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

    const bool added = addPayloadParts(payload, loaded, error_message) &&
                       addPayloadSmartPrefabs(payload, loaded, error_message);
    active.erase(absolute_path);
    return added;
}

bool pathLooksLikeFullLibraryScope(const std::filesystem::path& path) {
    const auto normalized = path.generic_string();
    return normalized.find("game_maker_all_parts") != std::string::npos ||
           normalized.find("cutesckr_all_parts") != std::string::npos || path.stem() == "full";
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

bool LoadGridPartCatalogScopeFromProject(const std::filesystem::path& project_root,
                                         const std::vector<std::filesystem::path>& relative_catalog_paths,
                                         GridPartCatalogScope& scope, std::string* error_message) {
    if (error_message != nullptr) {
        error_message->clear();
    }
    if (relative_catalog_paths.empty()) {
        setError(error_message, "catalog_scope_empty");
        return false;
    }

    GridPartCatalogScope loaded;
    std::set<std::filesystem::path> active;
    for (const auto& relative_path : relative_catalog_paths) {
        if (relative_path.empty()) {
            setError(error_message, "catalog_scope_path_empty");
            return false;
        }
        if (!loadCatalogInto(project_root / relative_path, loaded.catalog, active, error_message)) {
            return false;
        }
        loaded.active_catalog_paths.push_back(relative_path.generic_string());
        loaded.full_library_active = loaded.full_library_active || pathLooksLikeFullLibraryScope(relative_path);
    }
    loaded.scope_name = loaded.full_library_active ? "full_library" : "starter";
    loaded.active_catalog_count = loaded.active_catalog_paths.size();
    loaded.active_part_count = loaded.catalog.size();

    scope = std::move(loaded);
    return true;
}

} // namespace urpg::map
