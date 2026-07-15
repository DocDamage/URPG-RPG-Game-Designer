#include "engine/core/map/grid_part_catalog.h"

#include <algorithm>
#include <cctype>
#include <set>

namespace urpg::map {

namespace {

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool containsCaseInsensitive(const std::string& haystack, const std::string& needle) {
    return toLower(haystack).find(toLower(needle)) != std::string::npos;
}

bool supportsRuleset(const GridPartDefinition& definition, GridPartRuleset ruleset) {
    return std::find(definition.supported_rulesets.begin(), definition.supported_rulesets.end(), ruleset) !=
           definition.supported_rulesets.end();
}

bool matchesSearch(const GridPartDefinition& definition, const std::string& query) {
    if (query.empty()) {
        return true;
    }

    if (containsCaseInsensitive(definition.part_id, query) || containsCaseInsensitive(definition.display_name, query) ||
        containsCaseInsensitive(definition.description, query)) {
        return true;
    }

    return std::any_of(definition.tags.begin(), definition.tags.end(),
                       [&](const std::string& tag) { return containsCaseInsensitive(tag, query); });
}

} // namespace

bool GridPartCatalog::addDefinition(GridPartDefinition definition) {
    if (definition.part_id.empty() || definitions_.contains(definition.part_id)) {
        return false;
    }

    auto partId = definition.part_id;
    definitions_.emplace(std::move(partId), std::move(definition));
    return true;
}

const GridPartDefinition* GridPartCatalog::find(const std::string& part_id) const {
    const auto found = definitions_.find(part_id);
    return found == definitions_.end() ? nullptr : &found->second;
}

bool GridPartCatalog::addSmartPrefab(GridPartSmartPrefab prefab) {
    if (prefab.prefab_id.empty() || prefab.version.empty() || prefab.operations.empty() ||
        smart_prefabs_.contains(prefab.prefab_id)) {
        return false;
    }

    std::set<std::string> parameter_keys;
    for (const auto& parameter : prefab.parameters) {
        if (parameter.key.empty() || !parameter_keys.insert(parameter.key).second) {
            return false;
        }
    }

    std::set<std::string> operation_ids;
    for (const auto& operation : prefab.operations) {
        if (operation.operation_id.empty() || operation.part_id.empty() ||
            !operation_ids.insert(operation.operation_id).second) {
            return false;
        }
    }

    auto prefabId = prefab.prefab_id;
    smart_prefabs_.emplace(std::move(prefabId), std::move(prefab));
    return true;
}

const GridPartSmartPrefab* GridPartCatalog::findSmartPrefab(const std::string& prefab_id) const {
    const auto found = smart_prefabs_.find(prefab_id);
    return found == smart_prefabs_.end() ? nullptr : &found->second;
}

std::vector<GridPartDefinition> GridPartCatalog::allDefinitions() const {
    std::vector<GridPartDefinition> definitions;
    definitions.reserve(definitions_.size());
    for (const auto& [partId, definition] : definitions_) {
        (void)partId;
        definitions.push_back(definition);
    }
    sortByPartId(definitions);
    return definitions;
}

std::vector<GridPartDefinition> GridPartCatalog::filterByCategory(GridPartCategory category) const {
    std::vector<GridPartDefinition> definitions;
    for (const auto& [partId, definition] : definitions_) {
        (void)partId;
        if (definition.category == category) {
            definitions.push_back(definition);
        }
    }
    sortByPartId(definitions);
    return definitions;
}

std::vector<GridPartDefinition> GridPartCatalog::filterByRuleset(GridPartRuleset ruleset) const {
    std::vector<GridPartDefinition> definitions;
    for (const auto& [partId, definition] : definitions_) {
        (void)partId;
        if (supportsRuleset(definition, ruleset)) {
            definitions.push_back(definition);
        }
    }
    sortByPartId(definitions);
    return definitions;
}

std::vector<GridPartDefinition> GridPartCatalog::search(const std::string& query) const {
    std::vector<GridPartDefinition> definitions;
    for (const auto& [partId, definition] : definitions_) {
        (void)partId;
        if (matchesSearch(definition, query)) {
            definitions.push_back(definition);
        }
    }
    sortByPartId(definitions);
    return definitions;
}

std::vector<GridPartSmartPrefab> GridPartCatalog::allSmartPrefabs() const {
    std::vector<GridPartSmartPrefab> prefabs;
    prefabs.reserve(smart_prefabs_.size());
    for (const auto& [prefabId, prefab] : smart_prefabs_) {
        (void)prefabId;
        prefabs.push_back(prefab);
    }
    std::sort(prefabs.begin(), prefabs.end(), [](const GridPartSmartPrefab& left, const GridPartSmartPrefab& right) {
        return left.prefab_id < right.prefab_id;
    });
    return prefabs;
}

size_t GridPartCatalog::size() const {
    return definitions_.size();
}

void GridPartCatalog::sortByPartId(std::vector<GridPartDefinition>& definitions) {
    std::sort(definitions.begin(), definitions.end(),
              [](const GridPartDefinition& lhs, const GridPartDefinition& rhs) { return lhs.part_id < rhs.part_id; });
}

} // namespace urpg::map
