#include "engine/core/map/grid_part_serializer.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace urpg::map {

namespace {

constexpr int kSchemaVersion = 1;

const char* toString(GridPartCategory category) {
    switch (category) {
    case GridPartCategory::Tile:
        return "Tile";
    case GridPartCategory::Wall:
        return "Wall";
    case GridPartCategory::Platform:
        return "Platform";
    case GridPartCategory::Hazard:
        return "Hazard";
    case GridPartCategory::Door:
        return "Door";
    case GridPartCategory::Npc:
        return "Npc";
    case GridPartCategory::Enemy:
        return "Enemy";
    case GridPartCategory::TreasureChest:
        return "TreasureChest";
    case GridPartCategory::SavePoint:
        return "SavePoint";
    case GridPartCategory::Trigger:
        return "Trigger";
    case GridPartCategory::CutsceneZone:
        return "CutsceneZone";
    case GridPartCategory::Shop:
        return "Shop";
    case GridPartCategory::QuestItem:
        return "QuestItem";
    case GridPartCategory::Prop:
        return "Prop";
    case GridPartCategory::LevelBlock:
        return "LevelBlock";
    }
    return "Prop";
}

const char* toString(GridPartLayer layer) {
    switch (layer) {
    case GridPartLayer::Terrain:
        return "Terrain";
    case GridPartLayer::Decoration:
        return "Decoration";
    case GridPartLayer::Collision:
        return "Collision";
    case GridPartLayer::Object:
        return "Object";
    case GridPartLayer::Actor:
        return "Actor";
    case GridPartLayer::Trigger:
        return "Trigger";
    case GridPartLayer::Region:
        return "Region";
    case GridPartLayer::Overlay:
        return "Overlay";
    }
    return "Object";
}

std::optional<GridPartCategory> categoryFromString(const std::string& value) {
    static const std::vector<std::pair<std::string, GridPartCategory>> kValues = {
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
    const auto found =
        std::find_if(kValues.begin(), kValues.end(), [&](const auto& item) { return item.first == value; });
    return found == kValues.end() ? std::nullopt : std::make_optional(found->second);
}

std::optional<GridPartLayer> layerFromString(const std::string& value) {
    static const std::vector<std::pair<std::string, GridPartLayer>> kValues = {
        {"Terrain", GridPartLayer::Terrain},       {"Decoration", GridPartLayer::Decoration},
        {"Collision", GridPartLayer::Collision},   {"Object", GridPartLayer::Object},
        {"Actor", GridPartLayer::Actor},           {"Trigger", GridPartLayer::Trigger},
        {"Region", GridPartLayer::Region},         {"Overlay", GridPartLayer::Overlay},
    };
    const auto found =
        std::find_if(kValues.begin(), kValues.end(), [&](const auto& item) { return item.first == value; });
    return found == kValues.end() ? std::nullopt : std::make_optional(found->second);
}

std::vector<PlacedPartInstance> sortedParts(const GridPartDocument& document) {
    auto parts = document.parts();
    std::sort(parts.begin(), parts.end(), [](const PlacedPartInstance& left, const PlacedPartInstance& right) {
        if (left.layer != right.layer) {
            return static_cast<uint8_t>(left.layer) < static_cast<uint8_t>(right.layer);
        }
        if (left.grid_y != right.grid_y) {
            return left.grid_y < right.grid_y;
        }
        if (left.grid_x != right.grid_x) {
            return left.grid_x < right.grid_x;
        }
        return left.instance_id < right.instance_id;
    });
    return parts;
}

nlohmann::json propertiesToJson(const PlacedPartInstance& part) {
    std::vector<std::pair<std::string, std::string>> properties(part.properties.begin(), part.properties.end());
    std::sort(properties.begin(), properties.end(),
              [](const auto& left, const auto& right) { return left.first < right.first; });

    nlohmann::json json = nlohmann::json::object();
    for (const auto& [key, value] : properties) {
        json[key] = value;
    }
    return json;
}

nlohmann::json partToJson(const PlacedPartInstance& part) {
    return {
        {"instanceId", part.instance_id},
        {"partId", part.part_id},
        {"category", toString(part.category)},
        {"layer", toString(part.layer)},
        {"x", part.grid_x},
        {"y", part.grid_y},
        {"z", part.grid_z},
        {"width", part.width},
        {"height", part.height},
        {"rotY", part.rot_y},
        {"scale", part.scale},
        {"locked", part.locked},
        {"hidden", part.hidden},
        {"properties", propertiesToJson(part)},
    };
}

std::optional<PlacedPartInstance> partFromJson(const nlohmann::json& json) {
    if (!json.is_object()) {
        return std::nullopt;
    }

    auto category = categoryFromString(json.value("category", ""));
    auto layer = layerFromString(json.value("layer", ""));
    if (!category.has_value() || !layer.has_value()) {
        return std::nullopt;
    }

    PlacedPartInstance part;
    part.instance_id = json.value("instanceId", "");
    part.part_id = json.value("partId", "");
    part.category = *category;
    part.layer = *layer;
    part.grid_x = json.value("x", 0);
    part.grid_y = json.value("y", 0);
    part.grid_z = json.value("z", 0);
    part.width = json.value("width", 1);
    part.height = json.value("height", 1);
    part.rot_y = json.value("rotY", 0.0f);
    part.scale = json.value("scale", 1.0f);
    part.locked = json.value("locked", false);
    part.hidden = json.value("hidden", false);

    const auto properties = json.value("properties", nlohmann::json::object());
    if (!properties.is_object()) {
        return std::nullopt;
    }
    for (const auto& [key, value] : properties.items()) {
        if (!value.is_string()) {
            return std::nullopt;
        }
        part.properties[key] = value.get<std::string>();
    }
    return part;
}

} // namespace

nlohmann::json GridPartDocumentToJson(const GridPartDocument& document) {
    nlohmann::json json;
    json["schemaVersion"] = kSchemaVersion;
    json["mapId"] = document.mapId();
    json["width"] = document.width();
    json["height"] = document.height();
    json["chunkSize"] = document.chunkSize();
    json["parts"] = nlohmann::json::array();

    for (const auto& part : sortedParts(document)) {
        json["parts"].push_back(partToJson(part));
    }
    return json;
}

std::optional<GridPartDocument> GridPartDocumentFromJson(const nlohmann::json& json) {
    try {
        if (!json.is_object() || json.value("schemaVersion", 0) != kSchemaVersion) {
            return std::nullopt;
        }

        GridPartDocument document(json.value("mapId", ""), json.value("width", 0), json.value("height", 0),
                                  json.value("chunkSize", 16));
        if (document.mapId().empty() || document.width() <= 0 || document.height() <= 0) {
            return std::nullopt;
        }

        const auto parts = json.value("parts", nlohmann::json::array());
        if (!parts.is_array()) {
            return std::nullopt;
        }
        for (const auto& row : parts) {
            auto part = partFromJson(row);
            if (!part.has_value() || !document.placePart(*part)) {
                return std::nullopt;
            }
        }

        document.clearDirtyChunks();
        return document;
    } catch (const nlohmann::json::exception&) {
        return std::nullopt;
    }
}

} // namespace urpg::map
