#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace urpg::assets {

struct AssetProvenance {
    std::string original_source;
    std::string license;
    std::string normalized_path;
    bool export_eligible = false;
};

inline nlohmann::json toJson(const AssetProvenance& provenance) {
    return {
        {"original_source", provenance.original_source},
        {"license", provenance.license},
        {"normalized_path", provenance.normalized_path},
        {"export_eligible", provenance.export_eligible},
    };
}

inline AssetProvenance assetProvenanceFromJson(const nlohmann::json& value) {
    AssetProvenance provenance{};
    if (!value.is_object()) {
        return provenance;
    }
    if (const auto it = value.find("original_source"); it != value.end() && it->is_string()) {
        provenance.original_source = it->get<std::string>();
    }
    if (const auto it = value.find("license"); it != value.end() && it->is_string()) {
        provenance.license = it->get<std::string>();
    }
    if (const auto it = value.find("normalized_path"); it != value.end() && it->is_string()) {
        provenance.normalized_path = it->get<std::string>();
    }
    if (const auto it = value.find("export_eligible"); it != value.end() && it->is_boolean()) {
        provenance.export_eligible = it->get<bool>();
    }
    return provenance;
}

} // namespace urpg::assets
