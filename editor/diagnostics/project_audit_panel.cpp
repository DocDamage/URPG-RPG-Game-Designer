#include "editor/diagnostics/project_audit_panel.h"

namespace urpg::editor {

namespace {

ProjectAuditSeverity parseSeverity(const std::string& value) {
    if (value == "warning") {
        return ProjectAuditSeverity::Warning;
    }
    if (value == "error") {
        return ProjectAuditSeverity::Error;
    }
    return ProjectAuditSeverity::Info;
}

std::optional<std::string> readString(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    if (it == value.end() || !it->is_string()) {
        return std::nullopt;
    }
    return it->get<std::string>();
}

std::optional<bool> readBool(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    if (it == value.end() || !it->is_boolean()) {
        return std::nullopt;
    }
    return it->get<bool>();
}

std::optional<size_t> readCount(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    if (it == value.end()) {
        return std::nullopt;
    }
    if (it->is_number_unsigned()) {
        return it->get<size_t>();
    }
    if (it->is_number_integer()) {
        const auto count = it->get<long long>();
        if (count >= 0) {
            return static_cast<size_t>(count);
        }
    }
    return std::nullopt;
}

std::optional<bool> readPresence(const nlohmann::json& value) {
    if (const auto available = readBool(value, "available")) {
        return available;
    }
    if (const auto present = readBool(value, "present")) {
        return present;
    }
    if (const auto exists = readBool(value, "exists")) {
        return exists;
    }
    return std::nullopt;
}

std::optional<size_t> readIssueCount(const nlohmann::json& value) {
    if (const auto issue_count = readCount(value, "issueCount")) {
        return issue_count;
    }
    return readCount(value, "issue_count");
}

size_t countIssuesWithFlag(const nlohmann::json& report, const char* flag_key) {
    if (!report.is_object()) {
        return 0;
    }

    const auto it = report.find("issues");
    if (it == report.end() || !it->is_array()) {
        return 0;
    }

    size_t count = 0;
    for (const auto& issue_json : *it) {
        if (!issue_json.is_object()) {
            continue;
        }
        if (readBool(issue_json, flag_key).value_or(false)) {
            ++count;
        }
    }
    return count;
}

void appendIssueSnapshot(const nlohmann::json& issue_json, std::vector<ProjectAuditIssue>& issues) {
    if (!issue_json.is_object()) {
        return;
    }

    ProjectAuditIssue issue{};
    if (const auto code = readString(issue_json, "code")) {
        issue.code = *code;
    }
    if (const auto title = readString(issue_json, "title")) {
        issue.title = *title;
    }
    if (const auto detail = readString(issue_json, "detail")) {
        issue.detail = *detail;
    }
    if (const auto severity = readString(issue_json, "severity")) {
        issue.severity = parseSeverity(*severity);
    }
    if (const auto blocks_release = readBool(issue_json, "blocksRelease")) {
        issue.blocks_release = *blocks_release;
    }
    if (const auto blocks_export = readBool(issue_json, "blocksExport")) {
        issue.blocks_export = *blocks_export;
    }

    issues.push_back(std::move(issue));
}

template <typename SnapshotT>
void populateArtifactSectionSnapshot(const nlohmann::json& section, SnapshotT& snapshot) {
    if (const auto path = readString(section, "path")) {
        snapshot.path = *path;
    }
    if (const auto available = readPresence(section)) {
        snapshot.available = *available;
    }
    if (const auto issue_count = readIssueCount(section)) {
        snapshot.issue_count = *issue_count;
    }
}

const nlohmann::json* findObjectMember(const nlohmann::json& value,
                                       const char* primary,
                                       const char* alternate = nullptr) {
    const auto primary_it = value.find(primary);
    if (primary_it != value.end() && primary_it->is_object()) {
        return &(*primary_it);
    }
    if (alternate != nullptr) {
        const auto alternate_it = value.find(alternate);
        if (alternate_it != value.end() && alternate_it->is_object()) {
            return &(*alternate_it);
        }
    }
    return nullptr;
}

void populateCountsFromReport(const nlohmann::json& report,
                              size_t& release_blocker_count,
                              size_t& export_blocker_count) {
    release_blocker_count = readCount(report, "releaseBlockerCount").value_or(
        countIssuesWithFlag(report, "blocksRelease"));
    export_blocker_count = readCount(report, "exportBlockerCount").value_or(
        countIssuesWithFlag(report, "blocksExport"));
}

template <typename SnapshotT>
void populateOptionalCount(const nlohmann::json& value, const char* key, std::optional<SnapshotT>& out) {
    if (const auto count = readCount(value, key)) {
        out = static_cast<SnapshotT>(*count);
    }
}

void populateGovernanceSnapshot(const nlohmann::json& report, ProjectAuditPanel::RenderSnapshot& snapshot) {
    if (!report.contains("governance") || !report["governance"].is_object()) {
        return;
    }

    const auto& governance = report["governance"];

    if (governance.contains("assetReport") && governance["assetReport"].is_object()) {
        ProjectAuditAssetReportSnapshot asset_report{};
        const auto& asset = governance["assetReport"];
        if (const auto path = readString(asset, "path")) {
            asset_report.path = *path;
        }
        if (const auto available = readPresence(asset)) {
            asset_report.available = *available;
        }
        if (const auto usable = readBool(asset, "usable")) {
            asset_report.usable = *usable;
        }
        if (const auto issue_count = readIssueCount(asset)) {
            asset_report.issue_count = *issue_count;
        }
        snapshot.asset_report = std::move(asset_report);
    }

    if (governance.contains("schema") && governance["schema"].is_object()) {
        ProjectAuditSchemaGovernanceSnapshot schema_governance{};
        const auto& schema = governance["schema"];
        if (const auto schema_exists = readBool(schema, "schemaExists")) {
            schema_governance.schema_exists = *schema_exists;
        }
        if (const auto changelog_exists = readBool(schema, "changelogExists")) {
            schema_governance.changelog_exists = *changelog_exists;
        }
        if (const auto mentions_schema_version = readBool(schema, "mentionsSchemaVersion")) {
            schema_governance.mentions_schema_version = *mentions_schema_version;
        }
        if (const auto schema_version = readString(schema, "schemaVersion")) {
            schema_governance.schema_version = *schema_version;
        }
        snapshot.schema_governance = std::move(schema_governance);
    }

    if (governance.contains("projectSchema") && governance["projectSchema"].is_object()) {
        ProjectAuditProjectSchemaGovernanceSnapshot project_schema{};
        const auto& project = governance["projectSchema"];
        if (const auto path = readString(project, "path")) {
            project_schema.path = *path;
        }
        if (const auto available = readBool(project, "available")) {
            project_schema.available = *available;
        }
        if (const auto has_localization_section = readBool(project, "hasLocalizationSection")) {
            project_schema.has_localization_section = *has_localization_section;
        }
        if (const auto has_input_section = readBool(project, "hasInputSection")) {
            project_schema.has_input_section = *has_input_section;
        }
        if (const auto has_export_section = readBool(project, "hasExportSection")) {
            project_schema.has_export_section = *has_export_section;
        }
        snapshot.project_schema_governance = std::move(project_schema);
    }

    if (const auto* localization_artifacts = findObjectMember(governance, "localizationArtifacts", "localization_artifacts")) {
        ProjectAuditArtifactGovernanceSnapshot localization{};
        populateArtifactSectionSnapshot(*localization_artifacts, localization);
        snapshot.localization_artifacts = std::move(localization);
    }

    if (const auto* input_artifacts = findObjectMember(governance, "inputArtifacts", "input_artifacts")) {
        ProjectAuditArtifactGovernanceSnapshot input{};
        populateArtifactSectionSnapshot(*input_artifacts, input);
        snapshot.input_artifacts = std::move(input);
    }

    if (const auto* export_artifacts = findObjectMember(governance, "exportArtifacts", "export_artifacts")) {
        ProjectAuditArtifactGovernanceSnapshot export_section{};
        populateArtifactSectionSnapshot(*export_artifacts, export_section);
        snapshot.export_artifacts = std::move(export_section);
    }

    if (const auto* accessibility_artifacts = findObjectMember(governance, "accessibilityArtifacts", "accessibility_artifacts")) {
        ProjectAuditArtifactGovernanceSnapshot accessibility{};
        populateArtifactSectionSnapshot(*accessibility_artifacts, accessibility);
        snapshot.accessibility_artifacts = std::move(accessibility);
    }

    if (const auto* audio_artifacts = findObjectMember(governance, "audioArtifacts", "audio_artifacts")) {
        ProjectAuditArtifactGovernanceSnapshot audio{};
        populateArtifactSectionSnapshot(*audio_artifacts, audio);
        snapshot.audio_artifacts = std::move(audio);
    }

    if (const auto* performance_artifacts = findObjectMember(governance, "performanceArtifacts", "performance_artifacts")) {
        ProjectAuditArtifactGovernanceSnapshot performance{};
        populateArtifactSectionSnapshot(*performance_artifacts, performance);
        snapshot.performance_artifacts = std::move(performance);
    }

    if (const auto* release_signoff_workflow = findObjectMember(governance, "releaseSignoffWorkflow", "release_signoff_workflow")) {
        ProjectAuditArtifactGovernanceSnapshot workflow{};
        populateArtifactSectionSnapshot(*release_signoff_workflow, workflow);
        snapshot.release_signoff_workflow = std::move(workflow);
    }

    populateOptionalCount<size_t>(report, "assetGovernanceIssueCount", snapshot.asset_governance_issue_count);
    populateOptionalCount<size_t>(report, "schemaGovernanceIssueCount", snapshot.schema_governance_issue_count);
    populateOptionalCount<size_t>(report, "projectArtifactIssueCount", snapshot.project_artifact_issue_count);
    populateOptionalCount<size_t>(report, "accessibilityArtifactIssueCount", snapshot.accessibility_artifact_issue_count);
    populateOptionalCount<size_t>(report, "audioArtifactIssueCount", snapshot.audio_artifact_issue_count);
    populateOptionalCount<size_t>(report, "performanceArtifactIssueCount", snapshot.performance_artifact_issue_count);
    populateOptionalCount<size_t>(report, "releaseSignoffWorkflowIssueCount", snapshot.release_signoff_workflow_issue_count);
}

} // namespace

void ProjectAuditPanel::setReportJson(const nlohmann::json& report) {
    report_json_ = report;
}

void ProjectAuditPanel::clear() {
    report_json_.reset();
    last_render_snapshot_ = {};
    has_rendered_frame_ = false;
}

void ProjectAuditPanel::render() {
    if (!visible_) {
        return;
    }

    RenderSnapshot snapshot{};
    if (report_json_.has_value() && report_json_->is_object()) {
        const auto& report = *report_json_;
        snapshot.has_data = true;
        if (const auto headline = readString(report, "headline")) {
            snapshot.headline = *headline;
        }
        if (const auto summary = readString(report, "summary")) {
            snapshot.summary = *summary;
        }
        populateCountsFromReport(report, snapshot.release_blocker_count, snapshot.export_blocker_count);

        if (report.contains("templateContext") && report["templateContext"].is_object()) {
            const auto& template_context = report["templateContext"];
            if (const auto id = readString(template_context, "id")) {
                snapshot.template_id = *id;
            }
            if (const auto status = readString(template_context, "status")) {
                snapshot.template_status = *status;
            }
        }

        if (report.contains("issues") && report["issues"].is_array()) {
            for (const auto& issue_json : report["issues"]) {
                appendIssueSnapshot(issue_json, snapshot.issues);
            }
        }

        populateGovernanceSnapshot(report, snapshot);

        snapshot.issue_count = snapshot.issues.size();
    }

    last_render_snapshot_ = std::move(snapshot);
    has_rendered_frame_ = true;
}

size_t ProjectAuditPanel::currentIssueCount() const {
    if (!report_json_.has_value() || !report_json_->is_object()) {
        return 0;
    }
    const auto& report = *report_json_;
    if (!report.contains("issues") || !report["issues"].is_array()) {
        return 0;
    }
    size_t count = 0;
    for (const auto& issue_json : report["issues"]) {
        if (issue_json.is_object()) {
            ++count;
        }
    }
    return count;
}

size_t ProjectAuditPanel::currentReleaseBlockerCount() const {
    if (!report_json_.has_value() || !report_json_->is_object()) {
        return 0;
    }
    return readCount(*report_json_, "releaseBlockerCount").value_or(
        countIssuesWithFlag(*report_json_, "blocksRelease"));
}

size_t ProjectAuditPanel::currentExportBlockerCount() const {
    if (!report_json_.has_value() || !report_json_->is_object()) {
        return 0;
    }
    return readCount(*report_json_, "exportBlockerCount").value_or(
        countIssuesWithFlag(*report_json_, "blocksExport"));
}

} // namespace urpg::editor
