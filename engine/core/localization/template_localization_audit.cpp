#include "engine/core/localization/template_localization_audit.h"

#include "engine/core/localization/locale_catalog.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>

namespace urpg::localization {

namespace {

std::vector<std::string> readStringArray(const nlohmann::json& value) {
    std::vector<std::string> result;
    if (!value.is_array()) {
        return result;
    }
    for (const auto& item : value) {
        if (item.is_string()) {
            result.push_back(item.get<std::string>());
        }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

void appendDiagnostic(TemplateLocalizationAuditResult& result, std::string message) {
    result.diagnostics.push_back(std::move(message));
}

} // namespace

TemplateLocalizationAuditResult auditTemplateLocalization(const nlohmann::json& templateManifest) {
    TemplateLocalizationAuditResult result;
    result.template_id = templateManifest.value("template_id", "");

    if (result.template_id.empty()) {
        appendDiagnostic(result, "template_id_missing");
    }
    if (!templateManifest.contains("localization") || !templateManifest["localization"].is_object()) {
        appendDiagnostic(result, "localization_manifest_missing");
        return result;
    }

    const auto& localization = templateManifest["localization"];
    const auto requiredKeys = readStringArray(localization.value("required_keys", nlohmann::json::array()));
    result.required_key_count = requiredKeys.size();
    if (requiredKeys.empty()) {
        appendDiagnostic(result, "required_keys_missing");
    }

    if (!localization.contains("locale_bundles") || !localization["locale_bundles"].is_array() ||
        localization["locale_bundles"].empty()) {
        appendDiagnostic(result, "locale_bundles_missing");
        result.missing_key_count = result.required_key_count;
        result.missing_keys = requiredKeys;
        return result;
    }

    std::set<std::string> missingKeys;
    for (const auto& bundle : localization["locale_bundles"]) {
        LocaleCatalog catalog;
        try {
            catalog.loadFromJson(bundle);
        } catch (const std::invalid_argument&) {
            appendDiagnostic(result, "locale_bundle_invalid");
            for (const auto& key : requiredKeys) {
                missingKeys.insert(key);
            }
            continue;
        }

        const auto locale = catalog.getLocaleCode().empty() ? std::string("<unknown>") : catalog.getLocaleCode();
        if (!catalog.hasFontProfile()) {
            ++result.missing_font_profile_count;
            appendDiagnostic(result, "font_profile_missing:" + locale);
        }

        for (const auto& key : requiredKeys) {
            if (!catalog.hasKey(key)) {
                missingKeys.insert(key);
                appendDiagnostic(result, "missing_key:" + locale + ":" + key);
            }
        }
    }

    result.missing_keys.assign(missingKeys.begin(), missingKeys.end());
    result.missing_key_count = result.missing_keys.size();
    return result;
}

} // namespace urpg::localization
