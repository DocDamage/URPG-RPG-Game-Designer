#pragma once

#include "engine/core/map/grid_part_catalog.h"

#include <filesystem>
#include <string>

namespace urpg::map {

bool LoadGridPartCatalogFromFile(const std::filesystem::path& catalog_path, GridPartCatalog& catalog,
                                 std::string* error_message = nullptr);

bool LoadGridPartCatalogFromProject(const std::filesystem::path& project_root, GridPartCatalog& catalog,
                                    const std::filesystem::path& relative_catalog_path =
                                        std::filesystem::path("content") / "part_catalogs" / "base_jrpg_parts.json",
                                    std::string* error_message = nullptr);

} // namespace urpg::map
