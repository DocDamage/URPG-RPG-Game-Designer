#pragma once

#include "runtimes/compat_js/data_manager.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::compat {

using DataManagerJson = nlohmann::json;

Value jsonToValue(const DataManagerJson& j);
std::optional<DataManagerJson> loadJsonFile(const std::string& path);
std::optional<DataManagerJson> loadJsonArrayFile(const std::string& filename);
std::string buildMapFilename(int32_t mapId);
std::vector<std::vector<int32_t>> convertMapLayers(const DataManagerJson& data, int32_t width, int32_t height);
Object mapInfoToObject(const MapInfo& info);
Object tilesetToObject(const TilesetData& tileset);
void hydrateActorParamsFromClasses(std::vector<ActorData>& actors, const std::vector<ClassData>& classes);
void seedActorResourceState(std::vector<ActorData>& actors);

} // namespace urpg::compat
