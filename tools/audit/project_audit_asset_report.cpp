#include "tools/audit/project_audit_asset_report.h"

#include <vector>

namespace urpg::tools::audit {

namespace {

std::string getString(const nlohmann::json& value, const std::string& key, const std::string& fallback = "unknown") {
    if (value.contains(key) && value.at(key).is_string()) {
        return value.at(key).get<std::string>();
    }
    return fallback;
}

const nlohmann::json* projectSchemaProperty(const nlohmann::json& schema, const char* propertyName) {
    if (!schema.is_object() || !schema.contains("properties") || !schema.at("properties").is_object()) {
        return nullptr;
    }
    const auto it = schema.at("properties").find(propertyName);
    if (it == schema.at("properties").end() || !it->is_object()) {
        return nullptr;
    }
    return &(*it);
}

bool jsonStringArrayContains(const nlohmann::json& value, const char* expected) {
    if (!value.is_array()) {
        return false;
    }

    for (const auto& entry : value) {
        if (entry.is_string() && entry.get<std::string>() == expected) {
            return true;
        }
    }

    return false;
}

bool projectSchemaObjectSectionHasRequiredProperties(const nlohmann::json& schema,
                                                     const char* propertyName,
                                                     const std::vector<const char*>& requiredProperties) {
    const nlohmann::json* section = projectSchemaProperty(schema, propertyName);
    if (section == nullptr || getString(*section, "type") != "object") {
        return false;
    }

    if (!section->contains("properties") || !section->at("properties").is_object() ||
        !section->contains("required") || !section->at("required").is_array()) {
        return false;
    }

    for (const char* requiredProperty : requiredProperties) {
        if (!section->at("properties").contains(requiredProperty) ||
            !jsonStringArrayContains(section->at("required"), requiredProperty)) {
            return false;
        }
    }

    return true;
}

bool projectSchemaHasExportProfilesSection(const nlohmann::json& schema) {
    const nlohmann::json* section = projectSchemaProperty(schema, "exportProfiles");
    if (section == nullptr || getString(*section, "type") != "array") {
        return false;
    }

    if (!section->contains("items") || !section->at("items").is_object()) {
        return false;
    }

    const nlohmann::json& items = section->at("items");
    if (getString(items, "type") != "object" || !items.contains("properties") || !items.at("properties").is_object() ||
        !items.contains("required") || !items.at("required").is_array()) {
        return false;
    }

    const std::vector<const char*> requiredProperties = {
        "id",
        "target",
        "configSchemaPath",
        "validationReportSchemaPath",
    };
    for (const char* requiredProperty : requiredProperties) {
        if (!items.at("properties").contains(requiredProperty) ||
            !jsonStringArrayContains(items.at("required"), requiredProperty)) {
            return false;
        }
    }

    return true;
}

} // namespace

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

ProjectSchemaGovernanceSections inspectProjectSchemaGovernance(const nlohmann::json& schema) {
    return {
        projectSchemaProperty(schema, "localization") != nullptr,
        projectSchemaProperty(schema, "input") != nullptr,
        projectSchemaProperty(schema, "exportProfiles") != nullptr,
        projectSchemaObjectSectionHasRequiredProperties(
            schema,
            "localization",
            {"bundleRoot", "schemaPath", "reportPath", "requiredLocales"}),
        projectSchemaObjectSectionHasRequiredProperties(
            schema,
            "input",
            {"bindingSchemaPath", "controllerBindingSchemaPath", "fixturePath"}),
        projectSchemaHasExportProfilesSection(schema),
    };
}

} // namespace urpg::tools::audit
