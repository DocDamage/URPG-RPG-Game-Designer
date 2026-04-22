#pragma once

#include <nlohmann/json.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

enum class ProjectAuditSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
};

struct ProjectAuditIssue {
    std::string code;
    std::string title;
    std::string detail;
    ProjectAuditSeverity severity = ProjectAuditSeverity::Info;
    bool blocks_release = false;
    bool blocks_export = false;
};

struct ProjectAuditAssetReportSnapshot {
    std::optional<std::string> path;
    std::optional<bool> available;
    std::optional<bool> usable;
    std::optional<size_t> issue_count;
};

struct ProjectAuditSchemaGovernanceSnapshot {
    std::optional<bool> schema_exists;
    std::optional<bool> changelog_exists;
    std::optional<bool> mentions_schema_version;
    std::optional<std::string> schema_version;
};

struct ProjectAuditProjectSchemaGovernanceSnapshot {
    std::optional<std::string> path;
    std::optional<bool> available;
    std::optional<bool> has_localization_section;
    std::optional<bool> has_input_section;
    std::optional<bool> has_export_section;
};

struct ProjectAuditArtifactGovernanceSnapshot {
    std::optional<std::string> path;
    std::optional<bool> available;
    std::optional<size_t> issue_count;
};

struct ProjectAuditSignoffContractSnapshot {
    std::optional<bool> required;
    std::optional<std::string> artifact_path;
    std::optional<bool> promotion_requires_human_review;
    std::optional<std::string> workflow;
    std::optional<bool> contract_ok;
};

struct ProjectAuditExpectedArtifactSnapshot {
    std::optional<std::string> subsystem_id;
    std::optional<std::string> title;
    std::optional<std::string> path;
    std::optional<bool> required;
    std::optional<bool> exists;
    std::optional<bool> is_regular_file;
    std::optional<std::string> status;
    std::optional<bool> wording_ok;
    std::optional<bool> template_id_matches;
    std::optional<bool> required_subsystems_match;
    std::optional<bool> bars_match;
    std::vector<std::string> missing_phrases;
    std::vector<std::string> missing_required_subsystems;
    std::vector<std::string> unexpected_required_subsystems;
    std::optional<nlohmann::json> bar_mismatches;
    std::optional<ProjectAuditSignoffContractSnapshot> signoff_contract;
};

struct ProjectAuditRichArtifactGovernanceSnapshot {
    std::optional<std::string> path;
    std::optional<bool> available;
    std::optional<bool> enabled;
    std::optional<std::string> dependency;
    std::optional<std::string> summary;
    std::optional<size_t> issue_count;
    std::vector<ProjectAuditExpectedArtifactSnapshot> expected_artifacts;
};

struct ProjectAuditLocalizationEvidenceSnapshot {
    std::optional<std::string> path;
    std::optional<bool> available;
    std::optional<bool> enabled;
    std::optional<bool> usable;
    std::optional<std::string> dependency;
    std::optional<std::string> summary;
    std::optional<size_t> issue_count;
    std::optional<std::string> status;
    std::optional<bool> has_bundles;
    std::optional<size_t> bundle_count;
    std::optional<size_t> missing_locale_count;
    std::optional<size_t> missing_key_count;
    std::optional<size_t> extra_key_count;
    std::optional<std::string> master_locale;
    std::optional<nlohmann::json> bundles;
};

class ProjectAuditPanel {
public:
    struct RenderSnapshot {
        bool has_data = false;
        std::string headline;
        std::string summary;
        size_t issue_count = 0;
        size_t release_blocker_count = 0;
        size_t export_blocker_count = 0;
        std::string template_id;
        std::string template_status;
        std::optional<ProjectAuditAssetReportSnapshot> asset_report;
        std::optional<ProjectAuditSchemaGovernanceSnapshot> schema_governance;
        std::optional<ProjectAuditProjectSchemaGovernanceSnapshot> project_schema_governance;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> localization_artifacts;
        std::optional<ProjectAuditLocalizationEvidenceSnapshot> localization_evidence;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> input_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> export_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> accessibility_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> audio_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> performance_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> character_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> mod_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> analytics_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> release_signoff_workflow;
        std::optional<ProjectAuditRichArtifactGovernanceSnapshot> signoff_artifacts;
        std::optional<ProjectAuditRichArtifactGovernanceSnapshot> template_spec_artifacts;
        std::optional<size_t> asset_governance_issue_count;
        std::optional<size_t> schema_governance_issue_count;
        std::optional<size_t> project_artifact_issue_count;
        std::optional<size_t> localization_evidence_issue_count;
        std::optional<size_t> accessibility_artifact_issue_count;
        std::optional<size_t> audio_artifact_issue_count;
        std::optional<size_t> character_artifact_issue_count;
        std::optional<size_t> mod_artifact_issue_count;
        std::optional<size_t> analytics_artifact_issue_count;
        std::optional<size_t> performance_artifact_issue_count;
        std::optional<size_t> release_signoff_workflow_issue_count;
        std::optional<size_t> signoff_artifact_issue_count;
        std::optional<size_t> template_spec_artifact_issue_count;
        std::vector<ProjectAuditIssue> issues;
    };

    void setReportJson(const nlohmann::json& report);
    void clear();
    void render();

    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }
    bool hasReportData() const { return report_json_.has_value(); }
    size_t currentIssueCount() const;
    size_t currentReleaseBlockerCount() const;
    size_t currentExportBlockerCount() const;

    bool isVisible() const { return visible_; }
    void setVisible(bool visible) { visible_ = visible; }

private:
    std::optional<nlohmann::json> report_json_;
    RenderSnapshot last_render_snapshot_{};
    bool has_rendered_frame_ = false;
    bool visible_ = true;
};

} // namespace urpg::editor
