#pragma once

#include "engine/core/assets/asset_library.h"

#include <nlohmann/json.hpp>

namespace urpg::assets {

nlohmann::json buildAssetActionRows(const AssetLibrarySnapshot& snapshot);

} // namespace urpg::assets
