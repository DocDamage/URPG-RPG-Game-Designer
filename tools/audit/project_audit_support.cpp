#include "tools/audit/project_audit_support.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace urpg::tools::audit {

std::string readFile(const fs::path& path) {
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    if (!stream) {
        throw std::runtime_error("failed to open input file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

std::string getString(const json& value, const std::string& key, const std::string& fallback) {
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

LocalizationReportContext loadLocalizationReport(const fs::path& path, bool explicitPath) {
    LocalizationReportContext context;
    context.path = path;
    context.explicitPath = explicitPath;

    if (!fs::exists(path)) {
        if (explicitPath) {
            throw std::runtime_error("Localization report not found: " + path.string());
        }

        return context;
    }

    if (!fs::is_regular_file(path)) {
        if (explicitPath) {
            throw std::runtime_error("Localization report path is not a file: " + path.string());
        }

        context.loadWarning = "Localization report path is not a file: " + path.string();
        return context;
    }

    try {
        context.report = json::parse(readFile(path));
    } catch (const std::exception& ex) {
        if (explicitPath) {
            throw std::runtime_error("Failed to read localization report " + path.string() + ": " + ex.what());
        }

        context.loadWarning = "Failed to read localization report " + path.string() + ": " + ex.what();
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

const json* findSubsystemById(const json& readiness, const std::string& subsystemId) {
    if (!readiness.contains("subsystems") || !readiness.at("subsystems").is_array()) {
        return nullptr;
    }

    for (const auto& candidate : readiness.at("subsystems")) {
        if (candidate.is_object() && getString(candidate, "id") == subsystemId) {
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

} // namespace urpg::tools::audit
