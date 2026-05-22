#pragma once

#include "engine/core/map/grid_part_catalog.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::map {

struct GridPartCatalogScope {
    GridPartCatalog catalog;
    std::vector<std::filesystem::path> active_catalog_paths;
    std::string scope_name = "starter";
    size_t active_catalog_count = 0;
    size_t active_part_count = 0;
    bool full_library_active = false;
};

bool LoadGridPartCatalogFromFile(const std::filesystem::path& catalog_path, GridPartCatalog& catalog,
                                 std::string* error_message = nullptr);

bool LoadGridPartCatalogFromProject(const std::filesystem::path& project_root, GridPartCatalog& catalog,
                                    const std::filesystem::path& relative_catalog_path =
                                        std::filesystem::path("content") / "part_catalogs" / "base_jrpg_parts.json",
                                    std::string* error_message = nullptr);

bool LoadGridPartCatalogScopeFromProject(const std::filesystem::path& project_root,
                                         const std::vector<std::filesystem::path>& relative_catalog_paths,
                                         GridPartCatalogScope& scope, std::string* error_message = nullptr);

} // namespace urpg::map
