#include "tools/audit/project_audit_issue_collectors.h"

#include "tools/audit/project_audit_asset_report.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace urpg::tools::audit {
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
    const auto assetSummary = readAssetReportSummary(summary);
    const auto& normalized = assetSummary.normalized;
    const auto& promoted = assetSummary.promoted;
    const auto& promotedVisualLanes = assetSummary.promotedVisualLanes;
    const auto& promotedAudioLanes = assetSummary.promotedAudioLanes;
    const auto& wysiwygSmokeProofs = assetSummary.wysiwygSmokeProofs;

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

    if (isPartialStatus(templateContext.status) || isExperimentalStatus(templateContext.status)) {
        const auto addCounterIssue = [&](const std::string& code,
                                         const std::string& title,
                                         const std::string& detail) {
            issues.push_back({
                code,
                title,
                "Selected template " + templateContext.id + " is " + templateContext.status + " and " + detail,
                "warning",
                false,
                false,
            });
            ++assetGovernanceIssueCount;
        };

        if (*normalized <= 0) {
            addCounterIssue(
                "asset_report.no_normalized_intake",
                "No normalized asset intake yet",
                "the asset intake report shows normalized=0.");
        }
        if (*promoted <= 0) {
            addCounterIssue(
                "asset_report.no_promoted_intake",
                "No promoted asset intake yet",
                "the asset intake report shows promoted=0.");
        }
        if (!promotedVisualLanes.has_value() || *promotedVisualLanes <= 0) {
            addCounterIssue(
                "asset_report.no_promoted_visual_lane",
                "No promoted visual asset lane yet",
                "the asset intake report does not show a promoted visual lane.");
        }
        if (!promotedAudioLanes.has_value() || *promotedAudioLanes <= 0) {
            addCounterIssue(
                "asset_report.no_promoted_audio_lane",
                "No promoted audio asset lane yet",
                "the asset intake report does not show a promoted audio lane.");
        }
        if (!wysiwygSmokeProofs.has_value() || *wysiwygSmokeProofs <= 0) {
            addCounterIssue(
                "asset_report.no_wysiwyg_smoke_proof",
                "No WYSIWYG asset smoke proof yet",
                "the asset intake report does not show a WYSIWYG smoke proof.");
        }
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

    const auto schemaGovernance = inspectProjectSchemaGovernance(*projectSchemaContext.schema);

    governanceReport["projectSchema"]["canonicalLocalizationProperty"] = "localization";
    governanceReport["projectSchema"]["canonicalInputProperty"] = "input";
    governanceReport["projectSchema"]["canonicalExportProperty"] = "exportProfiles";
    governanceReport["projectSchema"]["hasLocalizationSection"] = schemaGovernance.localizationSectionOk;
    governanceReport["projectSchema"]["hasInputSection"] = schemaGovernance.inputSectionOk;
    governanceReport["projectSchema"]["hasExportSection"] = schemaGovernance.exportProfilesSectionOk;

    if (templateBarNeedsProjectArtifact(templateContext, "localization") &&
        !schemaGovernance.hasLocalizationProperty) {
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

    if (templateBarNeedsProjectArtifact(templateContext, "localization") &&
        schemaGovernance.hasLocalizationProperty && !schemaGovernance.localizationSectionOk) {
        issues.push_back({
            "project_schema.localization_malformed",
            "Project schema localization governance section is malformed",
            "Selected template " + templateContext.id +
                " still depends on localization governance, but project.schema.json localization must be an object with "
                "bundleRoot, schemaPath, reportPath, and requiredLocales.",
            "warning",
            false,
            false,
        });
        ++projectArtifactIssueCount;
    }

    if (templateBarNeedsProjectArtifact(templateContext, "input") && !schemaGovernance.hasInputProperty) {
        issues.push_back({
            "project_schema.input_governance_missing",
            "Project schema has no input governance section",
            "Selected template " + templateContext.id +
                " still depends on input/controller governance, but project.schema.json has no input section.",
            "warning",
            false,
            false,
        });
        ++projectArtifactIssueCount;
    }

    if (templateBarNeedsProjectArtifact(templateContext, "input") && schemaGovernance.hasInputProperty &&
        !schemaGovernance.inputSectionOk) {
        issues.push_back({
            "project_schema.input_governance_malformed",
            "Project schema input governance section is malformed",
            "Selected template " + templateContext.id +
                " still depends on input/controller governance, but project.schema.json input must be an object with "
                "bindingSchemaPath, controllerBindingSchemaPath, and fixturePath.",
            "warning",
            false,
            false,
        });
        ++projectArtifactIssueCount;
    }

    if ((isPartialStatus(templateContext.status) || isExperimentalStatus(templateContext.status)) &&
        !schemaGovernance.hasExportProfilesProperty) {
        issues.push_back({
            "project_schema.export_governance_missing",
            "Project schema has no export governance section",
            "Selected template " + templateContext.id +
                " is not fully ready and project.schema.json has no exportProfiles section for bounded export governance.",
            "warning",
            false,
            false,
        });
        ++projectArtifactIssueCount;
    }

    if ((isPartialStatus(templateContext.status) || isExperimentalStatus(templateContext.status)) &&
        schemaGovernance.hasExportProfilesProperty && !schemaGovernance.exportProfilesSectionOk) {
        issues.push_back({
            "project_schema.export_governance_malformed",
            "Project schema export governance section is malformed",
            "Selected template " + templateContext.id +
                " is not fully ready and project.schema.json exportProfiles must be an array of objects with id, target, "
                "configSchemaPath, and validationReportSchemaPath.",
            "warning",
            false,
            false,
        });
        ++projectArtifactIssueCount;
    }
}

void addLocalizationEvidenceIssues(const TemplateContext& templateContext,
                                   const LocalizationReportContext& localizationReportContext,
                                   std::vector<AuditIssue>& issues,
                                   std::size_t& localizationEvidenceIssueCount,
                                   json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "localization");
    json section = {
        {"enabled", enabled},
        {"dependency", "localization completeness evidence"},
        {"path", localizationReportContext.path.string()},
        {"explicit", localizationReportContext.explicitPath},
        {"available", localizationReportContext.report.has_value() || localizationReportContext.loadWarning.has_value()},
        {"usable", false},
        {"issueCount", 0},
        {"summary", enabled
                ? "Checking localization consistency evidence for selected template " + templateContext.id + "."
                : "Selected template " + templateContext.id + " does not currently depend on localization evidence."},
    };

    if (!enabled) {
        governanceReport["localizationEvidence"] = std::move(section);
        return;
    }

    if (localizationReportContext.loadWarning.has_value()) {
        issues.push_back({
            "localization_report.unavailable",
            "Localization consistency report unavailable",
            *localizationReportContext.loadWarning,
            "warning",
            false,
            false,
        });
        ++localizationEvidenceIssueCount;
        section["issueCount"] = localizationEvidenceIssueCount;
        governanceReport["localizationEvidence"] = std::move(section);
        return;
    }

    if (!localizationReportContext.report.has_value()) {
        issues.push_back({
            "localization_report.missing",
            "Localization consistency report missing",
            "Expected localization consistency evidence at " + localizationReportContext.path.string() +
                " was not found. Run tools/ci/check_localization_consistency.ps1 to refresh the canonical report.",
            "warning",
            false,
            false,
        });
        ++localizationEvidenceIssueCount;
        section["issueCount"] = localizationEvidenceIssueCount;
        governanceReport["localizationEvidence"] = std::move(section);
        return;
    }

    const json& report = *localizationReportContext.report;
    if (!report.is_object()) {
        if (localizationReportContext.explicitPath) {
            throw std::runtime_error("Localization report is not a JSON object: " + localizationReportContext.path.string());
        }

        issues.push_back({
            "localization_report.malformed",
            "Localization consistency report malformed",
            "Localization report is not a JSON object: " + localizationReportContext.path.string(),
            "warning",
            false,
            false,
        });
        ++localizationEvidenceIssueCount;
        section["issueCount"] = localizationEvidenceIssueCount;
        governanceReport["localizationEvidence"] = std::move(section);
        return;
    }

    if (!report.contains("summary") || !report.at("summary").is_object()) {
        if (localizationReportContext.explicitPath) {
            throw std::runtime_error("Localization report summary missing or malformed: " +
                                     localizationReportContext.path.string());
        }

        issues.push_back({
            "localization_report.summary.malformed",
            "Localization consistency summary malformed",
            "The localization report summary block is missing or not an object: " +
                localizationReportContext.path.string(),
            "warning",
            false,
            false,
        });
        ++localizationEvidenceIssueCount;
        section["issueCount"] = localizationEvidenceIssueCount;
        governanceReport["localizationEvidence"] = std::move(section);
        return;
    }

    const json& summary = report.at("summary");
    const std::optional<std::int64_t> bundleCount = getInteger(summary, "bundleCount");
    const std::optional<std::int64_t> missingLocaleCount = getInteger(summary, "missingLocaleCount");
    const std::optional<std::int64_t> missingKeyCount = getInteger(summary, "missingKeyCount");
    const std::optional<std::int64_t> extraKeyCount = getInteger(summary, "extraKeyCount");
    const bool summaryHasBundles = summary.contains("hasBundles") && summary.at("hasBundles").is_boolean();

    if (!bundleCount.has_value() || !missingLocaleCount.has_value() || !missingKeyCount.has_value() ||
        !extraKeyCount.has_value() || !summaryHasBundles) {
        if (localizationReportContext.explicitPath) {
            throw std::runtime_error("Localization report summary counters missing or malformed: " +
                                     localizationReportContext.path.string());
        }

        issues.push_back({
            "localization_report.summary.counters_missing",
            "Localization consistency summary counters missing",
            "The localization report summary does not expose the expected bundle and completeness counters: " +
                localizationReportContext.path.string(),
            "warning",
            false,
            false,
        });
        ++localizationEvidenceIssueCount;
        section["issueCount"] = localizationEvidenceIssueCount;
        governanceReport["localizationEvidence"] = std::move(section);
        return;
    }

    section["usable"] = true;
    section["status"] = getString(report, "status", "unknown");
    section["hasBundles"] = summary.at("hasBundles");
    section["bundleCount"] = *bundleCount;
    section["missingLocaleCount"] = *missingLocaleCount;
    section["missingKeyCount"] = *missingKeyCount;
    section["extraKeyCount"] = *extraKeyCount;
    if (summary.contains("masterLocale")) {
        section["masterLocale"] = summary.at("masterLocale");
    }

    if (report.contains("bundles") && report.at("bundles").is_array()) {
        section["bundles"] = report.at("bundles");
    }

    if (*missingKeyCount > 0) {
        issues.push_back({
            "localization_report.missing_keys",
            "Localization bundles are missing master keys",
            "Localization consistency report " + localizationReportContext.path.string() +
                " records " + std::to_string(*missingKeyCount) + " missing keys across " +
                std::to_string(*missingLocaleCount) + " locale bundles.",
            "warning",
            false,
            false,
        });
        ++localizationEvidenceIssueCount;
    }

    section["issueCount"] = localizationEvidenceIssueCount;
    governanceReport["localizationEvidence"] = std::move(section);
}

} // namespace urpg::tools::audit
