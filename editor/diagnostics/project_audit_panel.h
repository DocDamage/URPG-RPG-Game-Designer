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
        std::optional<ProjectAuditArtifactGovernanceSnapshot> input_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> export_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> accessibility_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> audio_artifacts;
        std::optional<ProjectAuditArtifactGovernanceSnapshot> performance_artifacts;
        std::optional<size_t> asset_governance_issue_count;
        std::optional<size_t> schema_governance_issue_count;
        std::optional<size_t> project_artifact_issue_count;
        std::optional<size_t> accessibility_artifact_issue_count;
        std::optional<size_t> audio_artifact_issue_count;
        std::optional<size_t> performance_artifact_issue_count;
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
