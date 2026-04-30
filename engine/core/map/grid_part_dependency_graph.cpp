#include "engine/core/map/grid_part_dependency_graph.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>

namespace urpg::map {

namespace {

struct PropertyDependencyRule {
    const char* key = "";
    GridPartDependencyType type = GridPartDependencyType::Asset;
};

constexpr PropertyDependencyRule kPropertyRules[] = {
    {"assetId", GridPartDependencyType::Asset},
    {"enemyId", GridPartDependencyType::Enemy},
    {"npcId", GridPartDependencyType::Npc},
    {"dialogueId", GridPartDependencyType::Dialogue},
    {"shopTableId", GridPartDependencyType::ShopTable},
    {"lootTableId", GridPartDependencyType::LootTable},
    {"questId", GridPartDependencyType::Quest},
    {"questItemId", GridPartDependencyType::QuestItem},
    {"cutsceneId", GridPartDependencyType::Cutscene},
    {"timelineId", GridPartDependencyType::Timeline},
    {"abilityId", GridPartDependencyType::Ability},
    {"audioId", GridPartDependencyType::Audio},
    {"animationId", GridPartDependencyType::Animation},
    {"scriptId", GridPartDependencyType::Script},
    {"tilesetId", GridPartDependencyType::Tileset},
    {"prefabPath", GridPartDependencyType::Prefab},
};

std::unordered_map<std::string, std::string> mergedProperties(const GridPartDefinition& definition,
                                                              const PlacedPartInstance& instance) {
    auto properties = definition.default_properties;
    for (const auto& [key, value] : instance.properties) {
        properties[key] = value;
    }
    return properties;
}

void addDependency(std::vector<GridPartDependency>& dependencies, GridPartDependencyType type, std::string id,
                   const std::string& sourceInstanceId, bool required = true) {
    if (id.empty()) {
        return;
    }

    const auto duplicate = std::find_if(dependencies.begin(), dependencies.end(), [&](const auto& dependency) {
        return dependency.type == type && dependency.id == id && dependency.source_instance_id == sourceInstanceId;
    });
    if (duplicate != dependencies.end()) {
        return;
    }

    dependencies.push_back({type, std::move(id), sourceInstanceId, required});
}

void sortDependencies(std::vector<GridPartDependency>& dependencies) {
    std::sort(dependencies.begin(), dependencies.end(), [](const auto& left, const auto& right) {
        if (left.type != right.type) {
            return static_cast<uint8_t>(left.type) < static_cast<uint8_t>(right.type);
        }
        if (left.id != right.id) {
            return left.id < right.id;
        }
        if (left.source_instance_id != right.source_instance_id) {
            return left.source_instance_id < right.source_instance_id;
        }
        return left.required > right.required;
    });
}

} // namespace

std::vector<GridPartDependency> CollectGridPartDependencies(const GridPartDocument& document,
                                                            const GridPartCatalog& catalog) {
    std::vector<GridPartDependency> dependencies;

    for (const auto& instance : document.parts()) {
        const auto* definition = catalog.find(instance.part_id);
        if (definition == nullptr) {
            continue;
        }

        addDependency(dependencies, GridPartDependencyType::Asset, definition->asset_id, instance.instance_id);
        addDependency(dependencies, GridPartDependencyType::Prefab, definition->prefab_path, instance.instance_id);
        if (definition->tile_id > 0) {
            addDependency(dependencies, GridPartDependencyType::Tileset, "tile:" + std::to_string(definition->tile_id),
                          instance.instance_id);
        }

        const auto properties = mergedProperties(*definition, instance);
        for (const auto& rule : kPropertyRules) {
            const auto found = properties.find(rule.key);
            if (found != properties.end()) {
                addDependency(dependencies, rule.type, found->second, instance.instance_id);
            }
        }
    }

    sortDependencies(dependencies);
    return dependencies;
}

} // namespace urpg::map
