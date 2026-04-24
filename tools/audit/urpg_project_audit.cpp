#include "tools/audit/project_audit_asset_report.h"
#include "tools/audit/project_audit_artifact_governance.h"
#include "tools/audit/project_audit_issue_collectors.h"
#include "tools/audit/project_audit_support.h"
#include "tools/audit/project_audit_template_governance.h"

#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;
using urpg::tools::audit::AssetReportContext;
using urpg::tools::audit::AuditIssue;
using urpg::tools::audit::LocalizationReportContext;
using urpg::tools::audit::ProjectSchemaContext;
using urpg::tools::audit::TemplateContext;
using urpg::tools::audit::addAccessibilityArtifactGovernance;
using urpg::tools::audit::addAchievementArtifactGovernance;
using urpg::tools::audit::addAnalyticsArtifactGovernance;
using urpg::tools::audit::addAudioArtifactGovernance;
using urpg::tools::audit::addCharacterArtifactGovernance;
using urpg::tools::audit::addExportArtifactGovernance;
using urpg::tools::audit::addInputArtifactGovernance;
using urpg::tools::audit::addLocalizationArtifactGovernance;
using urpg::tools::audit::addAssetReportIssues;
using urpg::tools::audit::addModArtifactGovernance;
using urpg::tools::audit::addPerformanceArtifactGovernance;
using urpg::tools::audit::addLocalizationEvidenceIssues;
using urpg::tools::audit::addMainBlockerIssues;
using urpg::tools::audit::addProjectArtifactIssues;
using urpg::tools::audit::addReleaseSignoffWorkflowGovernance;
using urpg::tools::audit::addSchemaGovernanceIssues;
using urpg::tools::audit::addSignoffArtifactGovernance;
using urpg::tools::audit::addSubsystemIssues;
using urpg::tools::audit::addTemplateBarIssues;
using urpg::tools::audit::addTemplateSpecArtifactGovernance;
using urpg::tools::audit::chooseTemplateContext;
using urpg::tools::audit::getString;
using urpg::tools::audit::loadAssetReport;
using urpg::tools::audit::loadLocalizationReport;
using urpg::tools::audit::loadProjectSchema;
using urpg::tools::audit::makeIssue;
using urpg::tools::audit::readFile;
using urpg::tools::audit::readAssetReportSummary;

namespace {

json buildReport(const json& readiness,
                 const TemplateContext& templateContext,
                 const AssetReportContext& assetReportContext,
                 const ProjectSchemaContext& projectSchemaContext,
                 const LocalizationReportContext& localizationReportContext) {
    std::vector<AuditIssue> issues;
    std::size_t releaseBlockerCount = 0;
    std::size_t exportBlockerCount = 0;
    std::size_t assetGovernanceIssueCount = 0;
    std::size_t schemaGovernanceIssueCount = 0;
    std::size_t projectArtifactIssueCount = 0;
    std::size_t localizationArtifactIssueCount = 0;
    std::size_t localizationEvidenceIssueCount = 0;
    std::size_t exportArtifactIssueCount = 0;
    std::size_t inputArtifactIssueCount = 0;
    std::size_t accessibilityArtifactIssueCount = 0;
    std::size_t audioArtifactIssueCount = 0;
    std::size_t achievementArtifactIssueCount = 0;
    std::size_t characterArtifactIssueCount = 0;
    std::size_t modArtifactIssueCount = 0;
    std::size_t analyticsArtifactIssueCount = 0;
    std::size_t performanceArtifactIssueCount = 0;
    std::size_t releaseSignoffWorkflowIssueCount = 0;
    std::size_t signoffArtifactIssueCount = 0;
    std::size_t templateSpecArtifactIssueCount = 0;
    json governanceReport = json::object();

    addSubsystemIssues(readiness, templateContext, issues, releaseBlockerCount, exportBlockerCount);
    addTemplateBarIssues(templateContext, issues, releaseBlockerCount, exportBlockerCount);
    addMainBlockerIssues(templateContext, issues, releaseBlockerCount, exportBlockerCount);
    addAssetReportIssues(templateContext, assetReportContext, issues, assetGovernanceIssueCount);
    addSchemaGovernanceIssues(readiness, issues, schemaGovernanceIssueCount, governanceReport);
    addProjectArtifactIssues(templateContext, projectSchemaContext, issues, projectArtifactIssueCount, governanceReport);
    addLocalizationArtifactGovernance(templateContext, issues, localizationArtifactIssueCount, governanceReport);
    addLocalizationEvidenceIssues(templateContext, localizationReportContext, issues, localizationEvidenceIssueCount, governanceReport);
    addExportArtifactGovernance(templateContext, issues, exportArtifactIssueCount, governanceReport);
    addInputArtifactGovernance(templateContext, issues, inputArtifactIssueCount, governanceReport);
    addAccessibilityArtifactGovernance(templateContext, issues, accessibilityArtifactIssueCount, governanceReport);
    addAudioArtifactGovernance(templateContext, issues, audioArtifactIssueCount, governanceReport);
    addAchievementArtifactGovernance(templateContext, issues, achievementArtifactIssueCount, governanceReport);
    addCharacterArtifactGovernance(templateContext, issues, characterArtifactIssueCount, governanceReport);
    addModArtifactGovernance(templateContext, issues, modArtifactIssueCount, governanceReport);
    addAnalyticsArtifactGovernance(templateContext, issues, analyticsArtifactIssueCount, governanceReport);
    addPerformanceArtifactGovernance(templateContext, issues, performanceArtifactIssueCount, governanceReport);
    addReleaseSignoffWorkflowGovernance(templateContext, issues, releaseSignoffWorkflowIssueCount, governanceReport);
    addSignoffArtifactGovernance(readiness, issues, signoffArtifactIssueCount, governanceReport);
    addTemplateSpecArtifactGovernance(templateContext, issues, templateSpecArtifactIssueCount, releaseBlockerCount, exportBlockerCount, governanceReport);

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
    if (localizationArtifactIssueCount > 0 || localizationEvidenceIssueCount > 0 || exportArtifactIssueCount > 0) {
        summary << " Localization artifact issues: " << localizationArtifactIssueCount
                << ". Localization evidence issues: " << localizationEvidenceIssueCount
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
    if (achievementArtifactIssueCount > 0) {
        summary << " Achievement artifact issues: " << achievementArtifactIssueCount << ".";
    }
    if (characterArtifactIssueCount > 0) {
        summary << " Character artifact issues: " << characterArtifactIssueCount << ".";
    }
    if (modArtifactIssueCount > 0) {
        summary << " Mod artifact issues: " << modArtifactIssueCount << ".";
    }
    if (analyticsArtifactIssueCount > 0) {
        summary << " Analytics artifact issues: " << analyticsArtifactIssueCount << ".";
    }
    if (performanceArtifactIssueCount > 0) {
        summary << " Performance artifact issues: " << performanceArtifactIssueCount << ".";
    }
    if (releaseSignoffWorkflowIssueCount > 0) {
        summary << " Release-signoff workflow issues: " << releaseSignoffWorkflowIssueCount << ".";
    }
    if (signoffArtifactIssueCount > 0) {
        summary << " Signoff artifact issues: " << signoffArtifactIssueCount << ".";
    }
    if (templateSpecArtifactIssueCount > 0) {
        summary << " Template-spec artifact issues: " << templateSpecArtifactIssueCount << ".";
    }

    json assetReportGovernance = {
        {"path", assetReportContext.path.string()},
        {"explicit", assetReportContext.explicitPath},
        {"available", assetReportContext.report.has_value() || assetReportContext.loadWarning.has_value()},
        {"usable", assetReportContext.report.has_value()},
        {"issueCount", assetGovernanceIssueCount},
    };
    if (assetReportContext.report.has_value() && assetReportContext.report->is_object() &&
        assetReportContext.report->contains("summary") && assetReportContext.report->at("summary").is_object()) {
        const auto assetSummary = readAssetReportSummary(assetReportContext.report->at("summary"));
        if (assetSummary.normalized.has_value()) {
            assetReportGovernance["normalizedCount"] = *assetSummary.normalized;
        }
        if (assetSummary.promoted.has_value()) {
            assetReportGovernance["promotedCount"] = *assetSummary.promoted;
        }
        if (assetSummary.promotedVisualLanes.has_value()) {
            assetReportGovernance["promotedVisualLaneCount"] = *assetSummary.promotedVisualLanes;
        }
        if (assetSummary.promotedAudioLanes.has_value()) {
            assetReportGovernance["promotedAudioLaneCount"] = *assetSummary.promotedAudioLanes;
        }
        if (assetSummary.wysiwygSmokeProofs.has_value()) {
            assetReportGovernance["wysiwygSmokeProofCount"] = *assetSummary.wysiwygSmokeProofs;
        }
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
                {"assetReport", assetReportGovernance},
                {"schema", governanceReport["schema"]},
                {"projectSchema", governanceReport["projectSchema"]},
                {"localizationArtifacts", governanceReport["localizationArtifacts"]},
                {"localizationEvidence", governanceReport["localizationEvidence"]},
                {"exportArtifacts", governanceReport["exportArtifacts"]},
                {"inputArtifacts", governanceReport["inputArtifacts"]},
                {"accessibilityArtifacts", governanceReport["accessibilityArtifacts"]},
                {"audioArtifacts", governanceReport["audioArtifacts"]},
                {"achievementArtifacts", governanceReport["achievementArtifacts"]},
                {"characterArtifacts", governanceReport["characterArtifacts"]},
                {"modArtifacts", governanceReport["modArtifacts"]},
                {"analyticsArtifacts", governanceReport["analyticsArtifacts"]},
                {"performanceArtifacts", governanceReport["performanceArtifacts"]},
                {"releaseSignoffWorkflow", governanceReport["releaseSignoffWorkflow"]},
                {"signoffArtifacts", governanceReport["signoffArtifacts"]},
                {"templateSpecArtifacts", governanceReport["templateSpecArtifacts"]},
            }},
        {"assetGovernanceIssueCount", assetGovernanceIssueCount},
        {"schemaGovernanceIssueCount", schemaGovernanceIssueCount},
        {"projectArtifactIssueCount", projectArtifactIssueCount},
        {"localizationEvidenceIssueCount", localizationEvidenceIssueCount},
        {"inputArtifactIssueCount", inputArtifactIssueCount},
        {"accessibilityArtifactIssueCount", accessibilityArtifactIssueCount},
        {"audioArtifactIssueCount", audioArtifactIssueCount},
        {"achievementArtifactIssueCount", achievementArtifactIssueCount},
        {"characterArtifactIssueCount", characterArtifactIssueCount},
        {"modArtifactIssueCount", modArtifactIssueCount},
        {"analyticsArtifactIssueCount", analyticsArtifactIssueCount},
        {"performanceArtifactIssueCount", performanceArtifactIssueCount},
        {"releaseSignoffWorkflowIssueCount", releaseSignoffWorkflowIssueCount},
        {"signoffArtifactIssueCount", signoffArtifactIssueCount},
        {"templateSpecArtifactIssueCount", templateSpecArtifactIssueCount},
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
        << "    --localization-report <path>\n"
        << "                      Read the localization consistency report from the given file.\n"
        << "    --template <id>   Select a template context by id.\n"
        << "    --help, -h        Show this help.\n";
}

} // namespace

int main(int argc, char** argv) {
    bool outputJson = false;
    fs::path inputPath = fs::path("content") / "readiness" / "readiness_status.json";
    fs::path assetReportPath = fs::path("imports") / "reports" / "asset_intake" / "source_capture_status.json";
    fs::path localizationReportPath =
        fs::path("imports") / "reports" / "localization" / "localization_consistency_report.json";
    bool assetReportExplicit = false;
    bool localizationReportExplicit = false;
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

        if (arg == "--localization-report") {
            if (i + 1 >= argc) {
                std::cerr << "Missing path after --localization-report\n";
                return 1;
            }

            localizationReportPath = fs::path(argv[++i]);
            localizationReportExplicit = true;
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
        const LocalizationReportContext localizationReportContext =
            loadLocalizationReport(localizationReportPath, localizationReportExplicit);
        const json report =
            buildReport(readiness, templateContext, assetReportContext, projectSchemaContext, localizationReportContext);

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
