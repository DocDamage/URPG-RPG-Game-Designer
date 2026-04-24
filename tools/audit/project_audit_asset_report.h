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

struct ProjectSchemaGovernanceSections {
    bool hasLocalizationProperty = false;
    bool hasInputProperty = false;
    bool hasExportProfilesProperty = false;
    bool localizationSectionOk = false;
    bool inputSectionOk = false;
    bool exportProfilesSectionOk = false;
};

std::optional<std::int64_t> getInteger(const nlohmann::json& value, const std::string& key);
AssetReportSummary readAssetReportSummary(const nlohmann::json& summary);
ProjectSchemaGovernanceSections inspectProjectSchemaGovernance(const nlohmann::json& schema);

} // namespace urpg::tools::audit
