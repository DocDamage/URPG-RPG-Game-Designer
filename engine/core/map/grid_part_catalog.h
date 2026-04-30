#pragma once

#include "engine/core/map/grid_part_types.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::map {

class GridPartCatalog {
  public:
    bool addDefinition(GridPartDefinition definition);
    const GridPartDefinition* find(const std::string& part_id) const;

    std::vector<GridPartDefinition> allDefinitions() const;
    std::vector<GridPartDefinition> filterByCategory(GridPartCategory category) const;
    std::vector<GridPartDefinition> filterByRuleset(GridPartRuleset ruleset) const;
    std::vector<GridPartDefinition> search(const std::string& query) const;

    size_t size() const;

  private:
    static void sortByPartId(std::vector<GridPartDefinition>& definitions);

    std::unordered_map<std::string, GridPartDefinition> definitions_;
};

} // namespace urpg::map
