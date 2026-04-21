#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

struct TemplateContext {
    std::string id;
    std::string status;
    json data;
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
    fs::path path;
    bool explicitPath = false;
    std::optional<json> report;
    std::optional<std::string> loadWarning;
};

struct ProjectSchemaContext {
    fs::path path;
    std::optional<json> schema;
    std::optional<std::string> loadWarning;
};

struct CanonicalArtifactSpec {
    std::string code;
    std::string title;
    std::string detailPrefix;
    fs::path path;
};

std::string readFile(const fs::path& path) {
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    if (!stream) {
        throw std::runtime_error("failed to open input file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

std::string getString(const json& value, const std::string& key, const std::string& fallback = "unknown") {
    if (value.contains(key) && value.at(key).is_string()) {
        return value.at(key).get<std::string>();
    }
    return fallback;
}

std::vector<std::string> getStringArray(const json& value, const std::string& key) {
    std::vector<std::string> items;
    if (!value.contains(key) || !value.at(key).is_array()) {
        return items;
    }

    for (const auto& entry : value.at(key)) {
        if (entry.is_string()) {
            items.push_back(entry.get<std::string>());
        }
    }

    return items;
}

std::optional<std::int64_t> getInteger(const json& value, const std::string& key) {
    if (!value.contains(key)) {
        return std::nullopt;
    }

    const json& entry = value.at(key);
    if (entry.is_number_integer()) {
        return entry.get<std::int64_t>();
    }

    if (entry.is_number_unsigned()) {
        return static_cast<std::int64_t>(entry.get<std::uint64_t>());
    }

    return std::nullopt;
}

json makeIssue(const AuditIssue& issue) {
    return json{
        {"code", issue.code},
        {"title", issue.title},
        {"detail", issue.detail},
        {"severity", issue.severity},
        {"blocksRelease", issue.blocksRelease},
        {"blocksExport", issue.blocksExport},
    };
}

AssetReportContext loadAssetReport(const fs::path& path, bool explicitPath) {
    AssetReportContext context;
    context.path = path;
    context.explicitPath = explicitPath;

    if (!fs::exists(path)) {
        if (explicitPath) {
            throw std::runtime_error("Asset report not found: " + path.string());
        }

        return context;
    }

    if (!fs::is_regular_file(path)) {
        if (explicitPath) {
            throw std::runtime_error("Asset report path is not a file: " + path.string());
        }

        context.loadWarning = "Asset report path is not a file: " + path.string();
        return context;
    }

    try {
        context.report = json::parse(readFile(path));
    } catch (const std::exception& ex) {
        if (explicitPath) {
            throw std::runtime_error("Failed to read asset report " + path.string() + ": " + ex.what());
        }

        context.loadWarning = "Failed to read asset report " + path.string() + ": " + ex.what();
    }

    return context;
}

ProjectSchemaContext loadProjectSchema(const fs::path& path) {
    ProjectSchemaContext context;
    context.path = path;

    if (!fs::exists(path)) {
        context.loadWarning = "Project schema not found: " + path.string();
        return context;
    }

    if (!fs::is_regular_file(path)) {
        context.loadWarning = "Project schema path is not a file: " + path.string();
        return context;
    }

    try {
        context.schema = json::parse(readFile(path));
    } catch (const std::exception& ex) {
        context.loadWarning = "Failed to read project schema " + path.string() + ": " + ex.what();
    }

    return context;
}

const json* findTemplateById(const json& readiness, const std::string& templateId) {
    if (!readiness.contains("templates") || !readiness.at("templates").is_array()) {
        return nullptr;
    }

    for (const auto& candidate : readiness.at("templates")) {
        if (candidate.is_object() && getString(candidate, "id") == templateId) {
            return &candidate;
        }
    }

    return nullptr;
}

TemplateContext chooseTemplateContext(const json& readiness, const std::optional<std::string>& requestedTemplate) {
    if (requestedTemplate.has_value()) {
        const json* selected = findTemplateById(readiness, *requestedTemplate);
        if (selected == nullptr) {
            throw std::runtime_error("unknown template id: " + *requestedTemplate);
        }

        return {getString(*selected, "id"), getString(*selected, "status", "PLANNED"), *selected};
    }

    if (readiness.contains("templates") && readiness.at("templates").is_array()) {
        for (const auto& candidate : readiness.at("templates")) {
            if (candidate.is_object() && getString(candidate, "id") == "jrpg") {
                return {getString(candidate, "id"), getString(candidate, "status", "PLANNED"), candidate};
            }
        }

        for (const auto& candidate : readiness.at("templates")) {
            if (candidate.is_object()) {
                return {getString(candidate, "id"), getString(candidate, "status", "PLANNED"), candidate};
            }
        }
    }

    return {"unknown", "PLANNED", json::object()};
}

bool isReadyStatus(const std::string& status) {
    return status == "READY";
}

bool isPartialStatus(const std::string& status) {
    return status == "PARTIAL";
}

bool isExperimentalStatus(const std::string& status) {
    return status == "EXPERIMENTAL";
}

bool isBlockedStatus(const std::string& status) {
    return status == "BLOCKED";
}

bool isPlannedStatus(const std::string& status) {
    return status == "PLANNED";
}

std::string issueSeverityForSubsystem(bool required, const std::string& status) {
    if (isReadyStatus(status)) {
        return "success";
    }

    if (required) {
        return "error";
    }

    if (isBlockedStatus(status) || isPlannedStatus(status)) {
        return "error";
    }

    return "warning";
}

void addSubsystemIssues(
    const json& readiness,
    const TemplateContext& templateContext,
    std::vector<AuditIssue>& issues,
    std::size_t& releaseBlockerCount,
    std::size_t& exportBlockerCount) {
    std::vector<std::string> requiredSubsystems;
    if (templateContext.data.contains("requiredSubsystems") && templateContext.data.at("requiredSubsystems").is_array()) {
        for (const auto& entry : templateContext.data.at("requiredSubsystems")) {
            if (entry.is_string()) {
                requiredSubsystems.push_back(entry.get<std::string>());
            }
        }
    }

    if (!readiness.contains("subsystems") || !readiness.at("subsystems").is_array()) {
        issues.push_back({
            "readiness.subsystems.missing",
            "Subsystem list missing",
            "The readiness document does not expose a subsystems array.",
            "error",
            true,
            true,
        });
        ++releaseBlockerCount;
        ++exportBlockerCount;
        return;
    }

    for (const auto& subsystem : readiness.at("subsystems")) {
        if (!subsystem.is_object()) {
            continue;
        }

        const std::string subsystemId = getString(subsystem, "id");
        const std::string status = getString(subsystem, "status", "UNKNOWN");
        const std::string summary = getString(subsystem, "summary", "No summary provided.");
        const bool required = std::find(requiredSubsystems.begin(), requiredSubsystems.end(), subsystemId) != requiredSubsystems.end();

        if (isReadyStatus(status)) {
            continue;
        }

        const bool blocksRelease = required && (isPartialStatus(status) || isBlockedStatus(status) || isPlannedStatus(status) || status != "READY");
        const bool blocksExport = required && (isBlockedStatus(status) || isPlannedStatus(status));
        const std::string severity = issueSeverityForSubsystem(required, status);
        std::string detail = summary;

        const auto gaps = getStringArray(subsystem, "mainGaps");
        if (!gaps.empty()) {
            detail += " Main gaps: ";
            for (std::size_t i = 0; i < gaps.size(); ++i) {
                if (i > 0) {
                    detail += "; ";
                }
                detail += gaps[i];
            }
            detail += ".";
        }

        issues.push_back({
            required ? "subsystem.required.not_ready" : "subsystem.not_ready",
            required ? "Required subsystem not ready" : "Subsystem not ready",
            subsystemId + ": " + detail,
            severity,
            blocksRelease,
            blocksExport,
        });

        if (blocksRelease) {
            ++releaseBlockerCount;
        }
        if (blocksExport) {
            ++exportBlockerCount;
        }
    }
}

void addTemplateBarIssues(
    const TemplateContext& templateContext,
    std::vector<AuditIssue>& issues,
    std::size_t& releaseBlockerCount,
    std::size_t& exportBlockerCount) {
    if (!templateContext.data.contains("bars") || !templateContext.data.at("bars").is_object()) {
        return;
    }

    const bool plannedTemplate = isPlannedStatus(templateContext.status);

    for (const auto& [barName, barValue] : templateContext.data.at("bars").items()) {
        if (!barValue.is_string()) {
            continue;
        }

        const std::string barStatus = barValue.get<std::string>();
        if (isReadyStatus(barStatus)) {
            continue;
        }

        const bool blocksRelease = plannedTemplate && (isPartialStatus(barStatus) || isPlannedStatus(barStatus) || isBlockedStatus(barStatus));
        const bool blocksExport = plannedTemplate && (isPartialStatus(barStatus) || isPlannedStatus(barStatus) || isBlockedStatus(barStatus));

        issues.push_back({
            "template.bar.not_ready",
            "Template bar not ready",
            barName + " is " + barStatus + " for template " + templateContext.id + ".",
            isPlannedStatus(barStatus) ? "error" : "warning",
            blocksRelease,
            blocksExport,
        });

        if (blocksRelease) {
            ++releaseBlockerCount;
        }
        if (blocksExport) {
            ++exportBlockerCount;
        }
    }
}

void addMainBlockerIssues(
    const TemplateContext& templateContext,
    std::vector<AuditIssue>& issues,
    std::size_t& releaseBlockerCount,
    std::size_t& exportBlockerCount) {
    if (!templateContext.data.contains("mainBlockers") || !templateContext.data.at("mainBlockers").is_array()) {
        return;
    }

    const bool plannedTemplate = isPlannedStatus(templateContext.status);

    for (const auto& blocker : templateContext.data.at("mainBlockers")) {
        if (!blocker.is_string()) {
            continue;
        }

        issues.push_back({
            "template.main_blocker",
            "Template main blocker",
            blocker.get<std::string>(),
            plannedTemplate ? "error" : "warning",
            plannedTemplate,
            plannedTemplate,
        });

        if (plannedTemplate) {
            ++releaseBlockerCount;
            ++exportBlockerCount;
        }
    }
}

void addAssetReportIssues(
    const TemplateContext& templateContext,
    const AssetReportContext& assetReportContext,
    std::vector<AuditIssue>& issues,
    std::size_t& assetGovernanceIssueCount) {
    if (assetReportContext.loadWarning.has_value()) {
        issues.push_back({
            "asset_report.unavailable",
            "Asset intake report unavailable",
            *assetReportContext.loadWarning,
            "warning",
            false,
            false,
        });
        ++assetGovernanceIssueCount;
        return;
    }

    if (!assetReportContext.report.has_value()) {
        return;
    }

    const json& report = *assetReportContext.report;
    if (!report.is_object()) {
        if (assetReportContext.explicitPath) {
            throw std::runtime_error("Asset report is not a JSON object: " + assetReportContext.path.string());
        }

        issues.push_back({
            "asset_report.malformed",
            "Asset intake report malformed",
            "Asset report is not a JSON object: " + assetReportContext.path.string(),
            "warning",
            false,
            false,
        });
        ++assetGovernanceIssueCount;
        return;
    }

    if (!report.contains("summary") || !report.at("summary").is_object()) {
        if (assetReportContext.explicitPath) {
            throw std::runtime_error("Asset report summary missing or malformed: " + assetReportContext.path.string());
        }

        issues.push_back({
            "asset_report.summary.malformed",
            "Asset intake summary malformed",
            "The asset report summary block is missing or not an object: " + assetReportContext.path.string(),
            "warning",
            false,
            false,
        });
        ++assetGovernanceIssueCount;
        return;
    }

    const json& summary = report.at("summary");
    const std::optional<std::int64_t> normalized = getInteger(summary, "normalized");
    const std::optional<std::int64_t> promoted = getInteger(summary, "promoted");

    if (!normalized.has_value() || !promoted.has_value()) {
        if (assetReportContext.explicitPath) {
            throw std::runtime_error("Asset report summary missing normalized/promoted counters: " + assetReportContext.path.string());
        }

        issues.push_back({
            "asset_report.summary.counters_missing",
            "Asset intake summary counters missing",
            "The asset report summary does not expose normalized/promoted counters: " + assetReportContext.path.string(),
            "warning",
            false,
            false,
        });
        ++assetGovernanceIssueCount;
        return;
    }

    if ((isPartialStatus(templateContext.status) || isExperimentalStatus(templateContext.status)) && *normalized == 0 && *promoted == 0) {
        issues.push_back({
            "asset_report.no_promoted_or_normalized_intake",
            "No promoted or normalized asset intake yet",
            "Selected template " + templateContext.id + " is " + templateContext.status +
                " and the asset intake report still shows normalized=0 and promoted=0.",
            "warning",
            false,
            false,
        });
        ++assetGovernanceIssueCount;
    }
}

void addSchemaGovernanceIssues(
    const json& readiness,
    std::vector<AuditIssue>& issues,
    std::size_t& schemaGovernanceIssueCount,
    json& governanceReport) {
    const fs::path schemaPath = fs::path("content") / "schemas" / "readiness_status.schema.json";
    const fs::path changelogPath = fs::path("docs") / "SCHEMA_CHANGELOG.md";
    const std::string schemaVersion = getString(readiness, "schemaVersion", "unknown");

    governanceReport["schema"] = {
        {"schemaPath", schemaPath.string()},
        {"schemaExists", fs::exists(schemaPath) && fs::is_regular_file(schemaPath)},
        {"changelogPath", changelogPath.string()},
        {"changelogExists", fs::exists(changelogPath) && fs::is_regular_file(changelogPath)},
        {"schemaVersion", schemaVersion},
    };

    if (!governanceReport["schema"]["schemaExists"].get<bool>()) {
        issues.push_back({
            "schema.file.missing",
            "Readiness schema file missing",
            "Expected schema file not found: " + schemaPath.string(),
            "warning",
            false,
            false,
        });
        ++schemaGovernanceIssueCount;
    }

    if (!governanceReport["schema"]["changelogExists"].get<bool>()) {
        issues.push_back({
            "schema.changelog.missing",
            "Schema changelog missing",
            "Expected schema changelog not found: " + changelogPath.string(),
            "warning",
            false,
            false,
        });
        ++schemaGovernanceIssueCount;
        return;
    }

    std::string changelogText;
    try {
        changelogText = readFile(changelogPath);
    } catch (const std::exception& ex) {
        issues.push_back({
            "schema.changelog.unreadable",
            "Schema changelog unreadable",
            ex.what(),
            "warning",
            false,
            false,
        });
        ++schemaGovernanceIssueCount;
        return;
    }

    const bool mentionsVersion = !schemaVersion.empty() && changelogText.find(schemaVersion) != std::string::npos;
    governanceReport["schema"]["mentionsSchemaVersion"] = mentionsVersion;

    if (!mentionsVersion) {
        issues.push_back({
            "schema.changelog.missing_version",
            "Schema changelog does not mention the readiness schema version",
            "The schema changelog does not mention schemaVersion " + schemaVersion + ".",
            "warning",
            false,
            false,
        });
        ++schemaGovernanceIssueCount;
    }
}

bool templateBarNeedsProjectArtifact(const TemplateContext& templateContext, const char* barName) {
    if (!templateContext.data.contains("bars") || !templateContext.data.at("bars").is_object()) {
        return false;
    }

    const auto it = templateContext.data.at("bars").find(barName);
    if (it == templateContext.data.at("bars").end() || !it->is_string()) {
        return false;
    }

    return it->get<std::string>() != "READY";
}

bool projectSchemaHasProperty(const json& schema, const char* propertyName) {
    if (!schema.is_object() || !schema.contains("properties") || !schema.at("properties").is_object()) {
        return false;
    }
    return schema.at("properties").contains(propertyName);
}

void addProjectArtifactIssues(const TemplateContext& templateContext,
                              const ProjectSchemaContext& projectSchemaContext,
                              std::vector<AuditIssue>& issues,
                              std::size_t& projectArtifactIssueCount,
                              json& governanceReport) {
    governanceReport["projectSchema"] = {
        {"path", projectSchemaContext.path.string()},
        {"available", projectSchemaContext.schema.has_value()},
    };

    if (projectSchemaContext.loadWarning.has_value()) {
        issues.push_back({
            "project_schema.unavailable",
            "Project schema unavailable",
            *projectSchemaContext.loadWarning,
            "warning",
            false,
            false,
        });
        ++projectArtifactIssueCount;
        return;
    }

    if (!projectSchemaContext.schema.has_value()) {
        return;
    }

    const json& schema = *projectSchemaContext.schema;
    governanceReport["projectSchema"]["hasLocalizationSection"] = projectSchemaHasProperty(schema, "localization");
    governanceReport["projectSchema"]["hasInputSection"] =
        projectSchemaHasProperty(schema, "input") || projectSchemaHasProperty(schema, "controllerBindings");
    governanceReport["projectSchema"]["hasExportSection"] =
        projectSchemaHasProperty(schema, "export") || projectSchemaHasProperty(schema, "exportProfiles");

    if (templateBarNeedsProjectArtifact(templateContext, "localization") &&
        !governanceReport["projectSchema"]["hasLocalizationSection"].get<bool>()) {
        issues.push_back({
            "project_schema.localization_missing",
            "Project schema has no localization governance section",
            "Selected template " + templateContext.id +
                " still depends on localization governance, but project.schema.json has no localization section.",
            "warning",
            false,
            false,
        });
        ++projectArtifactIssueCount;
    }

    if (templateBarNeedsProjectArtifact(templateContext, "input") &&
        !governanceReport["projectSchema"]["hasInputSection"].get<bool>()) {
        issues.push_back({
            "project_schema.input_governance_missing",
            "Project schema has no input governance section",
            "Selected template " + templateContext.id +
                " still depends on input/controller governance, but project.schema.json has no input or controllerBindings section.",
            "warning",
            false,
            false,
        });
        ++projectArtifactIssueCount;
    }

    if ((isPartialStatus(templateContext.status) || isExperimentalStatus(templateContext.status)) &&
        !governanceReport["projectSchema"]["hasExportSection"].get<bool>()) {
        issues.push_back({
            "project_schema.export_governance_missing",
            "Project schema has no export governance section",
            "Selected template " + templateContext.id +
                " is not fully ready and project.schema.json has no export or exportProfiles section for bounded export governance.",
            "warning",
            false,
            false,
        });
        ++projectArtifactIssueCount;
    }
}

void addCanonicalArtifactSection(const TemplateContext& templateContext,
                                 const std::string& sectionName,
                                 const std::string& dependencyLabel,
                                 bool enabled,
                                 const std::vector<CanonicalArtifactSpec>& artifacts,
                                 std::vector<AuditIssue>& issues,
                                 std::size_t& sectionIssueCount,
                                 json& governanceReport) {
    json section = json::object();
    section["enabled"] = enabled;
    section["dependency"] = dependencyLabel;
    section["issueCount"] = 0;
    section["expectedArtifacts"] = json::array();
    section["summary"] = enabled
        ? "Checking canonical " + dependencyLabel + " artifacts for selected template " + templateContext.id + "."
        : "Selected template " + templateContext.id + " does not currently depend on " + dependencyLabel + ".";

    for (const auto& artifact : artifacts) {
        const bool exists = fs::exists(artifact.path);
        const bool regularFile = exists && fs::is_regular_file(artifact.path);
        const bool required = enabled;
        const std::string status = !enabled ? "not_checked" : (regularFile ? "present" : (exists ? "invalid" : "missing"));

        section["expectedArtifacts"].push_back({
            {"code", artifact.code},
            {"title", artifact.title},
            {"path", artifact.path.string()},
            {"required", required},
            {"exists", exists},
            {"isRegularFile", regularFile},
            {"status", status},
        });

        if (!enabled || regularFile) {
            continue;
        }

        ++sectionIssueCount;
        issues.push_back({
            artifact.code,
            artifact.title,
            artifact.detailPrefix + " canonical artifact expected at " + artifact.path.string() +
                " is " + (exists ? "present but not a regular file" : "missing") +
                "; this is a governance gap, not proof the feature is absent.",
            "warning",
            false,
            false,
        });
    }

    section["issueCount"] = sectionIssueCount;
    governanceReport[sectionName] = section;
}

void addLocalizationArtifactGovernance(const TemplateContext& templateContext,
                                       std::vector<AuditIssue>& issues,
                                       std::size_t& localizationArtifactIssueCount,
                                       json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "localization");
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "localization_artifact.schema_missing",
            "Canonical localization catalog schema missing",
            "Localization",
            fs::path("content") / "schemas" / "localization_catalog.schema.json",
        },
        {
            "localization_artifact.extractor_missing",
            "Canonical localization extractor missing",
            "Localization",
            fs::path("tools") / "localization" / "extract_localization.cpp",
        },
        {
            "localization_artifact.writeback_missing",
            "Canonical localization writeback tool missing",
            "Localization",
            fs::path("tools") / "localization" / "writeback_localization.cpp",
        },
        {
            "localization_artifact.ci_gate_missing",
            "Canonical localization consistency gate missing",
            "Localization",
            fs::path("tools") / "ci" / "check_localization_consistency.ps1",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "localizationArtifacts",
        "localization governance",
        enabled,
        artifacts,
        issues,
        localizationArtifactIssueCount,
        governanceReport);
}

void addExportArtifactGovernance(const TemplateContext& templateContext,
                                 std::vector<AuditIssue>& issues,
                                 std::size_t& exportArtifactIssueCount,
                                 json& governanceReport) {
    const bool enabled = !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "export_artifact.packager_missing",
            "Canonical export packager missing",
            "Export",
            fs::path("engine") / "core" / "tools" / "export_packager.cpp",
        },
        {
            "export_artifact.cli_missing",
            "Canonical export packaging CLI missing",
            "Export",
            fs::path("tools") / "pack" / "pack_cli.cpp",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "exportArtifacts",
        "export governance",
        enabled,
        artifacts,
        issues,
        exportArtifactIssueCount,
        governanceReport);
}

void addInputArtifactGovernance(const TemplateContext& templateContext,
                                std::vector<AuditIssue>& issues,
                                std::size_t& inputArtifactIssueCount,
                                json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "input") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "input_artifact.runtime_header_missing",
            "Canonical controller binding runtime header missing",
            "Input",
            fs::path("engine") / "core" / "action" / "controller_binding_runtime.h",
        },
        {
            "input_artifact.runtime_source_missing",
            "Canonical controller binding runtime source missing",
            "Input",
            fs::path("engine") / "core" / "action" / "controller_binding_runtime.cpp",
        },
        {
            "input_artifact.panel_header_missing",
            "Canonical controller binding panel header missing",
            "Input",
            fs::path("editor") / "action" / "controller_binding_panel.h",
        },
        {
            "input_artifact.panel_source_missing",
            "Canonical controller binding panel source missing",
            "Input",
            fs::path("editor") / "action" / "controller_binding_panel.cpp",
        },
        {
            "input_artifact.schema_missing",
            "Canonical controller bindings schema missing",
            "Input",
            fs::path("content") / "schemas" / "controller_bindings.schema.json",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "inputArtifacts",
        "input governance",
        enabled,
        artifacts,
        issues,
        inputArtifactIssueCount,
        governanceReport);
}

void addAccessibilityArtifactGovernance(const TemplateContext& templateContext,
                                        std::vector<AuditIssue>& issues,
                                        std::size_t& accessibilityArtifactIssueCount,
                                        json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "accessibility") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "accessibility_artifact.schema_missing",
            "Canonical accessibility report schema missing",
            "Accessibility",
            fs::path("content") / "schemas" / "accessibility_report.schema.json",
        },
        {
            "accessibility_artifact.runtime_header_missing",
            "Canonical accessibility auditor header missing",
            "Accessibility",
            fs::path("engine") / "core" / "accessibility" / "accessibility_auditor.h",
        },
        {
            "accessibility_artifact.runtime_source_missing",
            "Canonical accessibility auditor source missing",
            "Accessibility",
            fs::path("engine") / "core" / "accessibility" / "accessibility_auditor.cpp",
        },
        {
            "accessibility_artifact.panel_header_missing",
            "Canonical accessibility panel header missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_panel.h",
        },
        {
            "accessibility_artifact.panel_source_missing",
            "Canonical accessibility panel source missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_panel.cpp",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "accessibilityArtifacts",
        "accessibility governance",
        enabled,
        artifacts,
        issues,
        accessibilityArtifactIssueCount,
        governanceReport);
}

void addAudioArtifactGovernance(const TemplateContext& templateContext,
                                std::vector<AuditIssue>& issues,
                                std::size_t& audioArtifactIssueCount,
                                json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "audio") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "audio_artifact.schema_missing",
            "Canonical audio mix schema missing",
            "Audio",
            fs::path("content") / "schemas" / "audio_mix_presets.schema.json",
        },
        {
            "audio_artifact.runtime_header_missing",
            "Canonical audio mix preset runtime header missing",
            "Audio",
            fs::path("engine") / "core" / "audio" / "audio_mix_presets.h",
        },
        {
            "audio_artifact.runtime_source_missing",
            "Canonical audio mix preset runtime source missing",
            "Audio",
            fs::path("engine") / "core" / "audio" / "audio_mix_presets.cpp",
        },
        {
            "audio_artifact.panel_header_missing",
            "Canonical audio mix panel header missing",
            "Audio",
            fs::path("editor") / "audio" / "audio_mix_panel.h",
        },
        {
            "audio_artifact.panel_source_missing",
            "Canonical audio mix panel source missing",
            "Audio",
            fs::path("editor") / "audio" / "audio_mix_panel.cpp",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "audioArtifacts",
        "audio governance",
        enabled,
        artifacts,
        issues,
        audioArtifactIssueCount,
        governanceReport);
}

void addPerformanceArtifactGovernance(const TemplateContext& templateContext,
                                      std::vector<AuditIssue>& issues,
                                      std::size_t& performanceArtifactIssueCount,
                                      json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "performance") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "performance_artifact.runtime_header_missing",
            "Canonical performance profiler header missing",
            "Performance",
            fs::path("engine") / "core" / "perf" / "perf_profiler.h",
        },
        {
            "performance_artifact.runtime_source_missing",
            "Canonical performance profiler source missing",
            "Performance",
            fs::path("engine") / "core" / "perf" / "perf_profiler.cpp",
        },
        {
            "performance_artifact.panel_header_missing",
            "Canonical performance diagnostics panel header missing",
            "Performance",
            fs::path("editor") / "perf" / "perf_diagnostics_panel.h",
        },
        {
            "performance_artifact.panel_source_missing",
            "Canonical performance diagnostics panel source missing",
            "Performance",
            fs::path("editor") / "perf" / "perf_diagnostics_panel.cpp",
        },
        {
            "performance_artifact.budget_doc_missing",
            "Canonical performance budget doc missing",
            "Performance",
            fs::path("docs") / "presentation" / "performance_budgets.md",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "performanceArtifacts",
        "performance governance",
        enabled,
        artifacts,
        issues,
        performanceArtifactIssueCount,
        governanceReport);
}

json buildReport(const json& readiness,
                 const TemplateContext& templateContext,
                 const AssetReportContext& assetReportContext,
                 const ProjectSchemaContext& projectSchemaContext) {
    std::vector<AuditIssue> issues;
    std::size_t releaseBlockerCount = 0;
    std::size_t exportBlockerCount = 0;
    std::size_t assetGovernanceIssueCount = 0;
    std::size_t schemaGovernanceIssueCount = 0;
    std::size_t projectArtifactIssueCount = 0;
    std::size_t localizationArtifactIssueCount = 0;
    std::size_t exportArtifactIssueCount = 0;
    std::size_t inputArtifactIssueCount = 0;
    std::size_t accessibilityArtifactIssueCount = 0;
    std::size_t audioArtifactIssueCount = 0;
    std::size_t performanceArtifactIssueCount = 0;
    json governanceReport = json::object();

    addSubsystemIssues(readiness, templateContext, issues, releaseBlockerCount, exportBlockerCount);
    addTemplateBarIssues(templateContext, issues, releaseBlockerCount, exportBlockerCount);
    addMainBlockerIssues(templateContext, issues, releaseBlockerCount, exportBlockerCount);
    addAssetReportIssues(templateContext, assetReportContext, issues, assetGovernanceIssueCount);
    addSchemaGovernanceIssues(readiness, issues, schemaGovernanceIssueCount, governanceReport);
    addProjectArtifactIssues(templateContext, projectSchemaContext, issues, projectArtifactIssueCount, governanceReport);
    addLocalizationArtifactGovernance(templateContext, issues, localizationArtifactIssueCount, governanceReport);
    addExportArtifactGovernance(templateContext, issues, exportArtifactIssueCount, governanceReport);
    addInputArtifactGovernance(templateContext, issues, inputArtifactIssueCount, governanceReport);
    addAccessibilityArtifactGovernance(templateContext, issues, accessibilityArtifactIssueCount, governanceReport);
    addAudioArtifactGovernance(templateContext, issues, audioArtifactIssueCount, governanceReport);
    addPerformanceArtifactGovernance(templateContext, issues, performanceArtifactIssueCount, governanceReport);

    json issueArray = json::array();
    for (const auto& issue : issues) {
        issueArray.push_back(makeIssue(issue));
    }

    const std::string templateStatus = templateContext.status;
    const std::string headline = "Readiness audit for " + templateContext.id + " (" + templateStatus + ")";
    std::ostringstream summary;
    summary << "Selected template " << templateContext.id << " is " << templateStatus << ". "
            << releaseBlockerCount << " release blockers and " << exportBlockerCount << " export blockers were found.";
    if (assetGovernanceIssueCount > 0 || schemaGovernanceIssueCount > 0 || projectArtifactIssueCount > 0) {
        summary << " Asset governance issues: " << assetGovernanceIssueCount
                << ". Schema governance issues: " << schemaGovernanceIssueCount
                << ". Project artifact issues: " << projectArtifactIssueCount << ".";
    }
    if (localizationArtifactIssueCount > 0 || exportArtifactIssueCount > 0) {
        summary << " Localization artifact issues: " << localizationArtifactIssueCount
                << ". Export artifact issues: " << exportArtifactIssueCount << ".";
    }
    if (inputArtifactIssueCount > 0) {
        summary << " Input artifact issues: " << inputArtifactIssueCount << ".";
    }
    if (accessibilityArtifactIssueCount > 0) {
        summary << " Accessibility artifact issues: " << accessibilityArtifactIssueCount << ".";
    }
    if (audioArtifactIssueCount > 0) {
        summary << " Audio artifact issues: " << audioArtifactIssueCount << ".";
    }
    if (performanceArtifactIssueCount > 0) {
        summary << " Performance artifact issues: " << performanceArtifactIssueCount << ".";
    }

    return json{
        {"schemaVersion", getString(readiness, "schemaVersion", "1.0.0")},
        {"statusDate", getString(readiness, "statusDate")},
        {"headline", headline},
        {"summary", summary.str()},
        {"releaseBlockerCount", releaseBlockerCount},
        {"exportBlockerCount", exportBlockerCount},
        {"templateContext",
            {
                {"id", templateContext.id},
                {"status", templateContext.status},
            }},
        {"governance",
            {
                {"assetReport",
                    {
                        {"path", assetReportContext.path.string()},
                        {"explicit", assetReportContext.explicitPath},
                        {"available", assetReportContext.report.has_value() || assetReportContext.loadWarning.has_value()},
                        {"usable", assetReportContext.report.has_value()},
                        {"issueCount", assetGovernanceIssueCount},
                    }},
                {"schema", governanceReport["schema"]},
                {"projectSchema", governanceReport["projectSchema"]},
                {"localizationArtifacts", governanceReport["localizationArtifacts"]},
                {"exportArtifacts", governanceReport["exportArtifacts"]},
                {"inputArtifacts", governanceReport["inputArtifacts"]},
                {"accessibilityArtifacts", governanceReport["accessibilityArtifacts"]},
                {"audioArtifacts", governanceReport["audioArtifacts"]},
                {"performanceArtifacts", governanceReport["performanceArtifacts"]},
            }},
        {"assetGovernanceIssueCount", assetGovernanceIssueCount},
        {"schemaGovernanceIssueCount", schemaGovernanceIssueCount},
        {"projectArtifactIssueCount", projectArtifactIssueCount},
        {"inputArtifactIssueCount", inputArtifactIssueCount},
        {"accessibilityArtifactIssueCount", accessibilityArtifactIssueCount},
        {"audioArtifactIssueCount", audioArtifactIssueCount},
        {"performanceArtifactIssueCount", performanceArtifactIssueCount},
        {"issues", issueArray},
    };
}

void printHelp() {
    std::cout
        << "urpg_project_audit\n"
        << "  Conservative scanner for content/readiness/readiness_status.json.\n"
        << "  Options:\n"
        << "    --json            Emit a JSON audit report.\n"
        << "    --input <path>    Read readiness data from the given file.\n"
        << "    --asset-report <path>\n"
        << "                      Read the asset intake report from the given file.\n"
        << "    --template <id>   Select a template context by id.\n"
        << "    --help, -h        Show this help.\n";
}

} // namespace

int main(int argc, char** argv) {
    bool outputJson = false;
    fs::path inputPath = fs::path("content") / "readiness" / "readiness_status.json";
    fs::path assetReportPath = fs::path("imports") / "reports" / "asset_intake" / "source_capture_status.json";
    bool assetReportExplicit = false;
    std::optional<std::string> requestedTemplate;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            printHelp();
            return 0;
        }

        if (arg == "--json") {
            outputJson = true;
            continue;
        }

        if (arg == "--input") {
            if (i + 1 >= argc) {
                std::cerr << "Missing path after --input\n";
                return 1;
            }

            inputPath = fs::path(argv[++i]);
            continue;
        }

        if (arg == "--asset-report") {
            if (i + 1 >= argc) {
                std::cerr << "Missing path after --asset-report\n";
                return 1;
            }

            assetReportPath = fs::path(argv[++i]);
            assetReportExplicit = true;
            continue;
        }

        if (arg == "--template") {
            if (i + 1 >= argc) {
                std::cerr << "Missing template id after --template\n";
                return 1;
            }

            requestedTemplate = std::string(argv[++i]);
            continue;
        }

        std::cerr << "Unknown argument: " << arg << "\n";
        return 1;
    }

    try {
        if (!fs::exists(inputPath)) {
            std::cerr << "Input file not found: " << inputPath.string() << "\n";
            return 1;
        }

        if (!fs::is_regular_file(inputPath)) {
            std::cerr << "Input path is not a file: " << inputPath.string() << "\n";
            return 1;
        }

        const json readiness = json::parse(readFile(inputPath));
        const TemplateContext templateContext = chooseTemplateContext(readiness, requestedTemplate);
        const AssetReportContext assetReportContext = loadAssetReport(assetReportPath, assetReportExplicit);
        const ProjectSchemaContext projectSchemaContext =
            loadProjectSchema(fs::path("content") / "schemas" / "project.schema.json");
        const json report = buildReport(readiness, templateContext, assetReportContext, projectSchemaContext);

        if (outputJson) {
            std::cout << report.dump(2) << "\n";
            return 0;
        }

        std::cout << report["headline"].get<std::string>() << "\n"
                  << report["summary"].get<std::string>() << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "urpg_project_audit failed: " << ex.what() << "\n";
        return 1;
    }
}
