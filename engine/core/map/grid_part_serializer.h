#pragma once

#include "engine/core/map/grid_part_document.h"

#include <nlohmann/json.hpp>

#include <optional>

namespace urpg::map {

nlohmann::json GridPartDocumentToJson(const GridPartDocument& document);
std::optional<GridPartDocument> GridPartDocumentFromJson(const nlohmann::json& json);

} // namespace urpg::map
