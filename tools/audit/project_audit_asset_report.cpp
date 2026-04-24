#include "tools/audit/project_audit_asset_report.h"

namespace urpg::tools::audit {

std::optional<std::int64_t> getInteger(const nlohmann::json& value, const std::string& key) {
    if (!value.contains(key)) {
        return std::nullopt;
    }

    const nlohmann::json& entry = value.at(key);
    if (entry.is_number_integer()) {
        return entry.get<std::int64_t>();
    }

    if (entry.is_number_unsigned()) {
        return static_cast<std::int64_t>(entry.get<std::uint64_t>());
    }

    return std::nullopt;
}

AssetReportSummary readAssetReportSummary(const nlohmann::json& summary) {
    return {
        getInteger(summary, "normalized"),
        getInteger(summary, "promoted"),
        getInteger(summary, "promoted_visual_lanes"),
        getInteger(summary, "promoted_audio_lanes"),
        getInteger(summary, "wysiwyg_smoke_proofs"),
    };
}

} // namespace urpg::tools::audit
