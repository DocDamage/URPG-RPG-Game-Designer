#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::tools::audit {

struct TemplateContext {
    std::string id;
    std::string status;
    nlohmann::json data;
};

struct AuditIssue {
    std::string code;
    std::string title;
    std::string detail;
    std::string severity;
    bool blocksRelease = false;
    bool blocksExport = false;
};

struct AssetReportContext {
    std::filesystem::path path;
    bool explicitPath = false;
    std::optional<nlohmann::json> report;
    std::optional<std::string> loadWarning;
};

struct ProjectSchemaContext {
    std::filesystem::path path;
    std::optional<nlohmann::json> schema;
    std::optional<std::string> loadWarning;
};

struct LocalizationReportContext {
    std::filesystem::path path;
    bool explicitPath = false;
    std::optional<nlohmann::json> report;
    std::optional<std::string> loadWarning;
};

struct CanonicalArtifactSpec {
    std::string code;
    std::string title;
    std::string detailPrefix;
    std::filesystem::path path;
};

struct SignoffArtifactSpec {
    std::string subsystemId;
    std::string missingCode;
    std::string wordingCode;
    std::string contractCode;
    std::string title;
    std::string detailPrefix;
    std::filesystem::path path;
    std::vector<std::string> pendingPhrases;
    std::vector<std::string> readyPhrases;
};

std::string readFile(const std::filesystem::path& path);
std::string getString(const nlohmann::json& value, const std::string& key, const std::string& fallback = "unknown");
std::vector<std::string> getStringArray(const nlohmann::json& value, const std::string& key);
nlohmann::json makeIssue(const AuditIssue& issue);

AssetReportContext loadAssetReport(const std::filesystem::path& path, bool explicitPath);
ProjectSchemaContext loadProjectSchema(const std::filesystem::path& path);
LocalizationReportContext loadLocalizationReport(const std::filesystem::path& path, bool explicitPath);

const nlohmann::json* findTemplateById(const nlohmann::json& readiness, const std::string& templateId);
const nlohmann::json* findSubsystemById(const nlohmann::json& readiness, const std::string& subsystemId);
TemplateContext chooseTemplateContext(const nlohmann::json& readiness, const std::optional<std::string>& requestedTemplate);

bool isReadyStatus(const std::string& status);
bool isPartialStatus(const std::string& status);
bool isExperimentalStatus(const std::string& status);
bool isBlockedStatus(const std::string& status);
bool isPlannedStatus(const std::string& status);
std::string issueSeverityForSubsystem(bool required, const std::string& status);
bool templateBarNeedsProjectArtifact(const TemplateContext& templateContext, const char* barName);

} // namespace urpg::tools::audit
