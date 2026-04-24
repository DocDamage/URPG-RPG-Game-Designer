#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace urpg::tools::audit {

struct AssetReportSummary {
    std::optional<std::int64_t> normalized;
    std::optional<std::int64_t> promoted;
    std::optional<std::int64_t> promotedVisualLanes;
    std::optional<std::int64_t> promotedAudioLanes;
    std::optional<std::int64_t> wysiwygSmokeProofs;
};

std::optional<std::int64_t> getInteger(const nlohmann::json& value, const std::string& key);
AssetReportSummary readAssetReportSummary(const nlohmann::json& summary);

} // namespace urpg::tools::audit
