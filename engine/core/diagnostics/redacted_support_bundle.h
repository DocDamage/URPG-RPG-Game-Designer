#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace urpg::diagnostics {

struct RedactedSupportBundleInput {
    std::vector<std::string> logs;
    nlohmann::json diagnostics = nlohmann::json::array();
    nlohmann::json versions = nlohmann::json::object();
    nlohmann::json platform_capabilities = nlohmann::json::object();
    std::map<std::string, std::string> project_manifest_hashes;
    nlohmann::json replay = nullptr;
    nlohmann::json selected_project_data = nullptr;
    bool include_replay = true;
    bool include_selected_project_data = false;
};

struct SupportBundleRedaction {
    std::string path;
    std::string reason;
};

struct RedactedSupportBundlePreview {
    bool valid = false;
    std::string code;
    nlohmann::json bundle = nlohmann::json::object();
    std::vector<std::string> included_sections;
    std::vector<std::string> excluded_sections;
    std::vector<SupportBundleRedaction> redactions;
};

struct RedactedSupportBundleWriteResult {
    bool success = false;
    std::string code;
    std::string message;
    std::filesystem::path path;
};

class RedactedSupportBundleBuilder {
public:
    RedactedSupportBundlePreview preview(const RedactedSupportBundleInput& input) const;
    RedactedSupportBundleWriteResult writeApproved(const RedactedSupportBundlePreview& preview,
                                                   const std::filesystem::path& output_directory,
                                                   bool preview_approved) const;

private:
    static nlohmann::json redact(const nlohmann::json& value, const std::string& path,
                                 std::vector<SupportBundleRedaction>& redactions);
};

} // namespace urpg::diagnostics
