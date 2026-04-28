#include "editor/diagnostics/project_audit_export_parity.h"

#include "editor/diagnostics/project_audit_panel.h"

#include <set>

namespace urpg::editor {
namespace {

template <typename SnapshotT>
nlohmann::json artifactGovernanceToJson(const SnapshotT& snapshot) {
    nlohmann::json artifact = nlohmann::json::object();
    if (snapshot.path.has_value()) {
        artifact["path"] = *snapshot.path;
    }
    if (snapshot.available.has_value()) {
        artifact["available"] = *snapshot.available;
    }
    if (snapshot.issue_count.has_value()) {
        artifact["issue_count"] = *snapshot.issue_count;
    }
    return artifact;
}

nlohmann::json signoffContractToJson(const ProjectAuditSignoffContractSnapshot& snapshot) {
    nlohmann::json contract = nlohmann::json::object();
    if (snapshot.required.has_value()) {
        contract["required"] = *snapshot.required;
    }
    if (snapshot.artifact_path.has_value()) {
        contract["artifact_path"] = *snapshot.artifact_path;
    }
    if (snapshot.promotion_requires_human_review.has_value()) {
        contract["promotion_requires_human_review"] = *snapshot.promotion_requires_human_review;
    }
    if (snapshot.workflow.has_value()) {
        contract["workflow"] = *snapshot.workflow;
    }
    if (snapshot.contract_ok.has_value()) {
        contract["contract_ok"] = *snapshot.contract_ok;
    }
    return contract;
}

nlohmann::json expectedArtifactToJson(const ProjectAuditExpectedArtifactSnapshot& snapshot) {
    nlohmann::json artifact = nlohmann::json::object();
    if (snapshot.subsystem_id.has_value()) {
        artifact["subsystem_id"] = *snapshot.subsystem_id;
    }
    if (snapshot.title.has_value()) {
        artifact["title"] = *snapshot.title;
    }
    if (snapshot.path.has_value()) {
        artifact["path"] = *snapshot.path;
    }
    if (snapshot.required.has_value()) {
        artifact["required"] = *snapshot.required;
    }
    if (snapshot.exists.has_value()) {
        artifact["exists"] = *snapshot.exists;
    }
    if (snapshot.is_regular_file.has_value()) {
        artifact["is_regular_file"] = *snapshot.is_regular_file;
    }
    if (snapshot.status.has_value()) {
        artifact["status"] = *snapshot.status;
    }
    if (snapshot.wording_ok.has_value()) {
        artifact["wording_ok"] = *snapshot.wording_ok;
    }
    if (snapshot.template_id_matches.has_value()) {
        artifact["template_id_matches"] = *snapshot.template_id_matches;
    }
    if (snapshot.required_subsystems_match.has_value()) {
        artifact["required_subsystems_match"] = *snapshot.required_subsystems_match;
    }
    if (snapshot.bars_match.has_value()) {
        artifact["bars_match"] = *snapshot.bars_match;
    }
    if (!snapshot.missing_phrases.empty()) {
        artifact["missing_phrases"] = snapshot.missing_phrases;
    }
    if (!snapshot.missing_required_subsystems.empty()) {
        artifact["missing_required_subsystems"] = snapshot.missing_required_subsystems;
    }
    if (!snapshot.unexpected_required_subsystems.empty()) {
        artifact["unexpected_required_subsystems"] = snapshot.unexpected_required_subsystems;
    }
    if (snapshot.bar_mismatches.has_value()) {
        artifact["bar_mismatches"] = *snapshot.bar_mismatches;
    }
    if (snapshot.signoff_contract.has_value()) {
        artifact["signoff_contract"] = signoffContractToJson(*snapshot.signoff_contract);
    }
    return artifact;
}

nlohmann::json richArtifactGovernanceToJson(const ProjectAuditRichArtifactGovernanceSnapshot& snapshot) {
    nlohmann::json artifact = artifactGovernanceToJson(snapshot);
    if (snapshot.enabled.has_value()) {
        artifact["enabled"] = *snapshot.enabled;
    }
    if (snapshot.dependency.has_value()) {
        artifact["dependency"] = *snapshot.dependency;
    }
    if (snapshot.summary.has_value()) {
        artifact["summary"] = *snapshot.summary;
    }
    if (!snapshot.expected_artifacts.empty()) {
        nlohmann::json expected_artifacts = nlohmann::json::array();
        for (const auto& expected_artifact : snapshot.expected_artifacts) {
            expected_artifacts.push_back(expectedArtifactToJson(expected_artifact));
        }
        artifact["expected_artifacts"] = std::move(expected_artifacts);
    }
    return artifact;
}

nlohmann::json localizationEvidenceToJson(const ProjectAuditLocalizationEvidenceSnapshot& snapshot) {
    nlohmann::json artifact = artifactGovernanceToJson(snapshot);
    if (snapshot.enabled.has_value()) {
        artifact["enabled"] = *snapshot.enabled;
    }
    if (snapshot.usable.has_value()) {
        artifact["usable"] = *snapshot.usable;
    }
    if (snapshot.dependency.has_value()) {
        artifact["dependency"] = *snapshot.dependency;
    }
    if (snapshot.summary.has_value()) {
        artifact["summary"] = *snapshot.summary;
    }
    if (snapshot.status.has_value()) {
        artifact["status"] = *snapshot.status;
    }
    if (snapshot.has_bundles.has_value()) {
        artifact["has_bundles"] = *snapshot.has_bundles;
    }
    if (snapshot.bundle_count.has_value()) {
        artifact["bundle_count"] = *snapshot.bundle_count;
    }
    if (snapshot.missing_locale_count.has_value()) {
        artifact["missing_locale_count"] = *snapshot.missing_locale_count;
    }
    if (snapshot.missing_key_count.has_value()) {
        artifact["missing_key_count"] = *snapshot.missing_key_count;
    }
    if (snapshot.extra_key_count.has_value()) {
        artifact["extra_key_count"] = *snapshot.extra_key_count;
    }
    if (snapshot.master_locale.has_value()) {
        artifact["master_locale"] = *snapshot.master_locale;
    }
    if (snapshot.bundles.has_value()) {
        artifact["bundles"] = *snapshot.bundles;
    }
    return artifact;
}

void addOptionalCount(nlohmann::json& root, const char* key, const std::optional<size_t>& value) {
    if (value.has_value()) {
        root[key] = *value;
    }
}

nlohmann::json snapshotToCanonical(const ProjectAuditPanel::RenderSnapshot& snapshot) {
    nlohmann::json canonical = {
        {"headline", snapshot.headline},
        {"summary_text", snapshot.summary},
        {"has_data", snapshot.has_data},
        {"issue_count", snapshot.issue_count},
        {"release_blocker_count", snapshot.release_blocker_count},
        {"export_blocker_count", snapshot.export_blocker_count},
        {"template_id", snapshot.template_id},
        {"template_status", snapshot.template_status},
    };

    addOptionalCount(canonical, "asset_governance_issue_count", snapshot.asset_governance_issue_count);
    addOptionalCount(canonical, "schema_governance_issue_count", snapshot.schema_governance_issue_count);
    addOptionalCount(canonical, "project_artifact_issue_count", snapshot.project_artifact_issue_count);
    addOptionalCount(canonical, "localization_evidence_issue_count", snapshot.localization_evidence_issue_count);
    addOptionalCount(canonical, "accessibility_artifact_issue_count", snapshot.accessibility_artifact_issue_count);
    addOptionalCount(canonical, "audio_artifact_issue_count", snapshot.audio_artifact_issue_count);
    addOptionalCount(canonical, "character_artifact_issue_count", snapshot.character_artifact_issue_count);
    addOptionalCount(canonical, "mod_artifact_issue_count", snapshot.mod_artifact_issue_count);
    addOptionalCount(canonical, "analytics_artifact_issue_count", snapshot.analytics_artifact_issue_count);
    addOptionalCount(canonical, "performance_artifact_issue_count", snapshot.performance_artifact_issue_count);
    addOptionalCount(canonical, "release_signoff_workflow_issue_count", snapshot.release_signoff_workflow_issue_count);
    addOptionalCount(canonical, "signoff_artifact_issue_count", snapshot.signoff_artifact_issue_count);
    addOptionalCount(canonical, "template_spec_artifact_issue_count", snapshot.template_spec_artifact_issue_count);

    nlohmann::json governance = nlohmann::json::object();
    if (snapshot.asset_report.has_value()) {
        nlohmann::json asset_report = artifactGovernanceToJson(*snapshot.asset_report);
        if (snapshot.asset_report->usable.has_value()) {
            asset_report["usable"] = *snapshot.asset_report->usable;
        }
        if (snapshot.asset_report->normalized_count.has_value()) {
            asset_report["normalized_count"] = *snapshot.asset_report->normalized_count;
        }
        if (snapshot.asset_report->promoted_count.has_value()) {
            asset_report["promoted_count"] = *snapshot.asset_report->promoted_count;
        }
        if (snapshot.asset_report->promoted_visual_lane_count.has_value()) {
            asset_report["promoted_visual_lane_count"] = *snapshot.asset_report->promoted_visual_lane_count;
        }
        if (snapshot.asset_report->promoted_audio_lane_count.has_value()) {
            asset_report["promoted_audio_lane_count"] = *snapshot.asset_report->promoted_audio_lane_count;
        }
        if (snapshot.asset_report->wysiwyg_smoke_proof_count.has_value()) {
            asset_report["wysiwyg_smoke_proof_count"] = *snapshot.asset_report->wysiwyg_smoke_proof_count;
        }
        governance["asset_report"] = std::move(asset_report);
    }
    if (snapshot.schema_governance.has_value()) {
        nlohmann::json schema = nlohmann::json::object();
        if (snapshot.schema_governance->schema_exists.has_value()) {
            schema["schema_exists"] = *snapshot.schema_governance->schema_exists;
        }
        if (snapshot.schema_governance->changelog_exists.has_value()) {
            schema["changelog_exists"] = *snapshot.schema_governance->changelog_exists;
        }
        if (snapshot.schema_governance->mentions_schema_version.has_value()) {
            schema["mentions_schema_version"] = *snapshot.schema_governance->mentions_schema_version;
        }
        if (snapshot.schema_governance->schema_version.has_value()) {
            schema["schema_version"] = *snapshot.schema_governance->schema_version;
        }
        governance["schema"] = std::move(schema);
    }
    if (snapshot.project_schema_governance.has_value()) {
        nlohmann::json project_schema = nlohmann::json::object();
        if (snapshot.project_schema_governance->path.has_value()) {
            project_schema["path"] = *snapshot.project_schema_governance->path;
        }
        if (snapshot.project_schema_governance->available.has_value()) {
            project_schema["available"] = *snapshot.project_schema_governance->available;
        }
        if (snapshot.project_schema_governance->has_localization_section.has_value()) {
            project_schema["has_localization_section"] =
                *snapshot.project_schema_governance->has_localization_section;
        }
        if (snapshot.project_schema_governance->has_input_section.has_value()) {
            project_schema["has_input_section"] = *snapshot.project_schema_governance->has_input_section;
        }
        if (snapshot.project_schema_governance->has_export_section.has_value()) {
            project_schema["has_export_section"] = *snapshot.project_schema_governance->has_export_section;
        }
        governance["project_schema"] = std::move(project_schema);
    }
    if (snapshot.localization_artifacts.has_value()) {
        governance["localization_artifacts"] = artifactGovernanceToJson(*snapshot.localization_artifacts);
    }
    if (snapshot.localization_evidence.has_value()) {
        governance["localization_evidence"] = localizationEvidenceToJson(*snapshot.localization_evidence);
    }
    if (snapshot.input_artifacts.has_value()) {
        governance["input_artifacts"] = artifactGovernanceToJson(*snapshot.input_artifacts);
    }
    if (snapshot.export_artifacts.has_value()) {
        governance["export_artifacts"] = artifactGovernanceToJson(*snapshot.export_artifacts);
    }
    if (snapshot.accessibility_artifacts.has_value()) {
        governance["accessibility_artifacts"] = artifactGovernanceToJson(*snapshot.accessibility_artifacts);
    }
    if (snapshot.audio_artifacts.has_value()) {
        governance["audio_artifacts"] = artifactGovernanceToJson(*snapshot.audio_artifacts);
    }
    if (snapshot.character_artifacts.has_value()) {
        governance["character_artifacts"] = artifactGovernanceToJson(*snapshot.character_artifacts);
    }
    if (snapshot.mod_artifacts.has_value()) {
        governance["mod_artifacts"] = artifactGovernanceToJson(*snapshot.mod_artifacts);
    }
    if (snapshot.analytics_artifacts.has_value()) {
        governance["analytics_artifacts"] = artifactGovernanceToJson(*snapshot.analytics_artifacts);
    }
    if (snapshot.performance_artifacts.has_value()) {
        governance["performance_artifacts"] = artifactGovernanceToJson(*snapshot.performance_artifacts);
    }
    if (snapshot.release_signoff_workflow.has_value()) {
        governance["release_signoff_workflow"] = artifactGovernanceToJson(*snapshot.release_signoff_workflow);
    }
    if (snapshot.signoff_artifacts.has_value()) {
        governance["signoff_artifacts"] = richArtifactGovernanceToJson(*snapshot.signoff_artifacts);
    }
    if (snapshot.template_spec_artifacts.has_value()) {
        governance["template_spec_artifacts"] = richArtifactGovernanceToJson(*snapshot.template_spec_artifacts);
    }
    if (!governance.empty()) {
        canonical["governance"] = std::move(governance);
    }

    nlohmann::json issues = nlohmann::json::array();
    for (const auto& issue : snapshot.issues) {
        const char* severity = "info";
        if (issue.severity == ProjectAuditSeverity::Warning) {
            severity = "warning";
        } else if (issue.severity == ProjectAuditSeverity::Error) {
            severity = "error";
        }
        issues.push_back({{"code", issue.code},
                          {"title", issue.title},
                          {"detail", issue.detail},
                          {"severity", severity},
                          {"blocks_release", issue.blocks_release},
                          {"blocks_export", issue.blocks_export}});
    }
    canonical["issues"] = std::move(issues);
    return canonical;
}

void compareJsonAtPath(const nlohmann::json& cli_value, const nlohmann::json& workspace_value,
                       std::string path, std::vector<ProjectAuditExportParityIssue>& issues) {
    if (cli_value.type() != workspace_value.type()) {
        issues.push_back({"type_mismatch", std::move(path), cli_value, workspace_value});
        return;
    }

    if (cli_value.is_object()) {
        std::set<std::string> keys;
        for (const auto& item : cli_value.items()) {
            keys.insert(item.key());
        }
        for (const auto& item : workspace_value.items()) {
            keys.insert(item.key());
        }
        for (const auto& key : keys) {
            const std::string child_path = path + "/" + key;
            const bool cli_has_key = cli_value.contains(key);
            const bool workspace_has_key = workspace_value.contains(key);
            if (!cli_has_key || !workspace_has_key) {
                issues.push_back({cli_has_key ? "missing_workspace_field" : "missing_cli_field",
                                  child_path,
                                  cli_has_key ? cli_value.at(key) : nlohmann::json(nullptr),
                                  workspace_has_key ? workspace_value.at(key) : nlohmann::json(nullptr)});
                continue;
            }
            compareJsonAtPath(cli_value.at(key), workspace_value.at(key), child_path, issues);
        }
        return;
    }

    if (cli_value.is_array()) {
        if (cli_value.size() != workspace_value.size()) {
            issues.push_back({"array_size_mismatch", std::move(path), cli_value, workspace_value});
            return;
        }
        for (size_t index = 0; index < cli_value.size(); ++index) {
            compareJsonAtPath(cli_value[index], workspace_value[index], path + "/" + std::to_string(index), issues);
        }
        return;
    }

    if (cli_value != workspace_value) {
        issues.push_back({"value_mismatch", std::move(path), cli_value, workspace_value});
    }
}

} // namespace

nlohmann::json buildProjectAuditCliCanonical(const nlohmann::json& cli_report) {
    ProjectAuditPanel panel;
    panel.setReportJson(cli_report);
    panel.render();
    return snapshotToCanonical(panel.lastRenderSnapshot());
}

nlohmann::json buildProjectAuditWorkspaceCanonical(const nlohmann::json& workspace_export) {
    nlohmann::json canonical = nlohmann::json::object();
    if (!workspace_export.is_object() || !workspace_export.contains("active_tab_detail") ||
        !workspace_export["active_tab_detail"].is_object()) {
        return canonical;
    }

    const auto& detail = workspace_export["active_tab_detail"];
    static constexpr const char* keys[] = {
        "headline",
        "summary_text",
        "has_data",
        "issue_count",
        "release_blocker_count",
        "export_blocker_count",
        "template_id",
        "template_status",
        "asset_governance_issue_count",
        "schema_governance_issue_count",
        "project_artifact_issue_count",
        "localization_evidence_issue_count",
        "accessibility_artifact_issue_count",
        "audio_artifact_issue_count",
        "character_artifact_issue_count",
        "mod_artifact_issue_count",
        "analytics_artifact_issue_count",
        "performance_artifact_issue_count",
        "release_signoff_workflow_issue_count",
        "signoff_artifact_issue_count",
        "template_spec_artifact_issue_count",
        "governance",
        "issues",
    };
    for (const char* key : keys) {
        if (detail.contains(key)) {
            canonical[key] = detail.at(key);
        }
    }
    return canonical;
}

ProjectAuditExportParityResult compareProjectAuditExportParity(const nlohmann::json& cli_report,
                                                               const nlohmann::json& workspace_export) {
    ProjectAuditExportParityResult result;
    result.cli_canonical = buildProjectAuditCliCanonical(cli_report);
    result.workspace_canonical = buildProjectAuditWorkspaceCanonical(workspace_export);

    if (!workspace_export.is_object() || workspace_export.value("active_tab", "") != "project_audit") {
        result.issues.push_back({"workspace_active_tab_mismatch",
                                 "/active_tab",
                                 "project_audit",
                                 workspace_export.is_object() && workspace_export.contains("active_tab")
                                     ? workspace_export.at("active_tab")
                                     : nlohmann::json(nullptr)});
    }
    compareJsonAtPath(result.cli_canonical, result.workspace_canonical, "", result.issues);
    result.matches = result.issues.empty();
    return result;
}

nlohmann::json projectAuditExportParityIssueToJson(const ProjectAuditExportParityIssue& issue) {
    return {{"code", issue.code},
            {"path", issue.path.empty() ? "/" : issue.path},
            {"cli_value", issue.cli_value},
            {"workspace_value", issue.workspace_value}};
}

nlohmann::json projectAuditExportParityResultToJson(const ProjectAuditExportParityResult& result) {
    nlohmann::json issues = nlohmann::json::array();
    for (const auto& issue : result.issues) {
        issues.push_back(projectAuditExportParityIssueToJson(issue));
    }
    return {{"matches", result.matches},
            {"issue_count", result.issues.size()},
            {"issues", std::move(issues)},
            {"cli_canonical", result.cli_canonical},
            {"workspace_canonical", result.workspace_canonical}};
}

} // namespace urpg::editor
