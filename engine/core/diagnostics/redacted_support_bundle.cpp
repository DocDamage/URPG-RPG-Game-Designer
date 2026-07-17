#include "engine/core/diagnostics/redacted_support_bundle.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>

namespace urpg::diagnostics {
namespace {

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

std::string sensitiveReason(const std::string& key) {
    const auto normalized = lower(key);
    for (const auto& token : {"password", "passwd", "token", "secret", "authorization", "api_key", "apikey",
                              "cookie", "credential"}) {
        if (normalized.find(token) != std::string::npos) return "secret";
    }
    for (const auto& token : {"email", "user_name", "username", "real_name", "display_name", "phone"}) {
        if (normalized.find(token) != std::string::npos) return "pii";
    }
    for (const auto& token : {"path", "directory", "project_root", "home"}) {
        if (normalized.find(token) != std::string::npos) return "filesystem_path";
    }
    return {};
}

std::string percentDecoded(const std::string& value) {
    const auto hex = [](const char character) -> int {
        if (character >= '0' && character <= '9') return character - '0';
        if (character >= 'a' && character <= 'f') return character - 'a' + 10;
        if (character >= 'A' && character <= 'F') return character - 'A' + 10;
        return -1;
    };
    std::string decoded;
    decoded.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] == '%' && index + 2 < value.size()) {
            const auto high = hex(value[index + 1]);
            const auto low = hex(value[index + 2]);
            if (high >= 0 && low >= 0) {
                decoded.push_back(static_cast<char>((high << 4) | low));
                index += 2;
                continue;
            }
        }
        decoded.push_back(value[index] == '+' ? ' ' : value[index]);
    }
    return decoded;
}

std::string freeTextReason(const std::string& value) {
    const auto inspected = percentDecoded(value);
    const auto normalized = lower(inspected);
    if ((normalized.starts_with("sk-") || normalized.starts_with("ghp_") ||
         normalized.starts_with("akia")) && normalized.size() >= 12) return "secret_text";
    for (const auto& marker : {"password=", "password:", "token=", "token:", "authorization:", "bearer ",
                               "api_key=", "secret="}) {
        if (normalized.find(marker) != std::string::npos) return "secret_text";
    }
    if (inspected.find('@') != std::string::npos &&
        inspected.find('.', inspected.find('@')) != std::string::npos) return "pii_text";
    const auto contains_windows_path = [&] {
        for (size_t index = 0; index + 2 < inspected.size(); ++index) {
            if (std::isalpha(static_cast<unsigned char>(inspected[index])) && inspected[index + 1] == ':' &&
                (inspected[index + 2] == '\\' || inspected[index + 2] == '/')) return true;
        }
        return false;
    }();
    if (contains_windows_path || inspected.starts_with("/") || inspected.starts_with("\\\\")) {
        return "filesystem_path_text";
    }
    return {};
}

void addSection(std::vector<std::string>& sections, const std::string& name) {
    if (std::find(sections.begin(), sections.end(), name) == sections.end()) sections.push_back(name);
}

} // namespace

RedactedSupportBundlePreview RedactedSupportBundleBuilder::preview(const RedactedSupportBundleInput& input) const {
    RedactedSupportBundlePreview preview;
    preview.bundle = {{"schema", "urpg.redacted_support_bundle.v2"}, {"upload_performed", false},
                      {"preview_required", true}};
    preview.bundle["logs"] = redact(input.logs, "logs", preview.redactions);
    preview.bundle["diagnostics"] = redact(input.diagnostics, "diagnostics", preview.redactions);
    preview.bundle["versions"] = redact(input.versions, "versions", preview.redactions);
    preview.bundle["platform_capabilities"] = redact(input.platform_capabilities, "platform_capabilities", preview.redactions);
    preview.bundle["project_manifest_hashes"] = redact(input.project_manifest_hashes, "project_manifest_hashes", preview.redactions);
    for (const auto& section : {"logs", "diagnostics", "versions", "platform_capabilities", "project_manifest_hashes"}) {
        addSection(preview.included_sections, section);
    }
    if (input.include_replay && !input.replay.is_null()) {
        preview.bundle["replay"] = redact(input.replay, "replay", preview.redactions);
        addSection(preview.included_sections, "replay");
    } else {
        preview.excluded_sections.push_back("replay");
    }
    if (input.include_selected_project_data && !input.selected_project_data.is_null()) {
        preview.bundle["selected_project_data"] =
            redact(input.selected_project_data, "selected_project_data", preview.redactions);
        addSection(preview.included_sections, "selected_project_data");
    } else {
        preview.excluded_sections.push_back("selected_project_data");
    }
    preview.bundle["redaction_count"] = preview.redactions.size();
    preview.bundle["included_sections"] = preview.included_sections;
    preview.bundle["excluded_sections"] = preview.excluded_sections;
    preview.valid = true;
    preview.code = "support_bundle_preview_ready";
    return preview;
}

RedactedSupportBundleWriteResult RedactedSupportBundleBuilder::writeApproved(
    const RedactedSupportBundlePreview& preview, const std::filesystem::path& output_directory,
    const bool preview_approved) const {
    if (!preview.valid) return {false, "support_bundle_preview_invalid", "Support bundle preview is invalid.", {}};
    if (!preview_approved) {
        return {false, "support_bundle_preview_not_approved",
                "Review and explicitly approve the support bundle preview before writing.", {}};
    }
    std::error_code error;
    std::filesystem::create_directories(output_directory, error);
    if (error) return {false, "support_bundle_directory_failed", error.message(), {}};
    const auto path = output_directory / "redacted_support_bundle.json";
    const auto temporary = output_directory / "redacted_support_bundle.json.tmp";
    const auto backup = output_directory / "redacted_support_bundle.json.bak";
    {
        std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
        if (!stream) return {false, "support_bundle_write_failed", "Could not open support bundle output.", {}};
        stream << preview.bundle.dump(2) << '\n';
        if (!stream.good()) return {false, "support_bundle_write_failed", "Could not write support bundle output.", {}};
    }
    const bool replacing = std::filesystem::exists(path);
    if (replacing) {
        std::filesystem::remove(backup, error);
        error.clear();
        std::filesystem::rename(path, backup, error);
        if (error) {
            std::filesystem::remove(temporary, error);
            return {false, "support_bundle_publish_failed", "Could not stage the previous support bundle.", {}};
        }
    }
    std::filesystem::rename(temporary, path, error);
    if (error) {
        std::filesystem::remove(temporary, error);
        if (replacing) {
            std::error_code restore_error;
            std::filesystem::rename(backup, path, restore_error);
        }
        return {false, "support_bundle_publish_failed", "Could not publish support bundle atomically.", {}};
    }
    if (replacing) std::filesystem::remove(backup, error);
    return {true, "support_bundle_written", "Redacted support bundle written locally; nothing was uploaded.", path};
}

nlohmann::json RedactedSupportBundleBuilder::redact(
    const nlohmann::json& value, const std::string& path, std::vector<SupportBundleRedaction>& redactions) {
    if (value.is_object()) {
        auto result = nlohmann::json::object();
        for (const auto& [key, child] : value.items()) {
            const auto child_path = path + "." + key;
            const auto reason = sensitiveReason(key);
            if (!reason.empty()) {
                result[key] = "[REDACTED]";
                redactions.push_back({child_path, reason});
            } else {
                result[key] = redact(child, child_path, redactions);
            }
        }
        return result;
    }
    if (value.is_array()) {
        auto result = nlohmann::json::array();
        for (size_t index = 0; index < value.size(); ++index) {
            result.push_back(redact(value[index], path + "[" + std::to_string(index) + "]", redactions));
        }
        return result;
    }
    if (value.is_string()) {
        const auto reason = freeTextReason(value.get<std::string>());
        if (!reason.empty()) {
            redactions.push_back({path, reason});
            return "[REDACTED]";
        }
    }
    return value;
}

} // namespace urpg::diagnostics
