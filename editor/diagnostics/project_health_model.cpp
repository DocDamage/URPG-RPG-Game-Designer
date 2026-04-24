#include "editor/diagnostics/project_health_model.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace urpg::editor {

namespace {

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

std::vector<std::string> readStringArray(const nlohmann::json& value, const char* key) {
    std::vector<std::string> strings;
    const auto it = value.find(key);
    if (it == value.end()) {
        return strings;
    }
    if (it->is_string()) {
        strings.push_back(it->get<std::string>());
        return strings;
    }
    if (!it->is_array()) {
        return strings;
    }
    for (const auto& entry : *it) {
        if (entry.is_string()) {
            strings.push_back(entry.get<std::string>());
        }
    }
    return strings;
}

ProjectHealthSeverity parseSeverity(const nlohmann::json& issue) {
    const auto severity = readString(issue, "severity").value_or("info");
    if (severity == "error") {
        return ProjectHealthSeverity::Error;
    }
    if (severity == "warning") {
        return ProjectHealthSeverity::Warning;
    }
    return ProjectHealthSeverity::Info;
}

int severityRank(ProjectHealthSeverity severity) {
    switch (severity) {
    case ProjectHealthSeverity::Error:
        return 0;
    case ProjectHealthSeverity::Warning:
        return 1;
    case ProjectHealthSeverity::Info:
    default:
        return 2;
    }
}

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool containsToken(const ProjectHealthFixCard& card, const char* token) {
    const auto needle = std::string(token);
    return lowercase(card.code).find(needle) != std::string::npos ||
           lowercase(card.title).find(needle) != std::string::npos ||
           lowercase(card.detail).find(needle) != std::string::npos;
}

ProjectHealthGroup classifyGroup(const ProjectHealthFixCard& card) {
    if (card.blocks_release) {
        return ProjectHealthGroup::ReleaseBlockers;
    }
    if (card.blocks_export) {
        return ProjectHealthGroup::ExportBlockers;
    }
    if (containsToken(card, "asset")) {
        return ProjectHealthGroup::AssetIssues;
    }
    if (containsToken(card, "schema")) {
        return ProjectHealthGroup::SchemaIssues;
    }
    if (containsToken(card, "governance") || containsToken(card, "artifact") ||
        containsToken(card, "signoff") || containsToken(card, "template")) {
        return ProjectHealthGroup::GovernanceIssues;
    }
    return ProjectHealthGroup::Warnings;
}

size_t countIssuesWithFlag(const nlohmann::json& report, const char* flag_key) {
    const auto it = report.find("issues");
    if (it == report.end() || !it->is_array()) {
        return 0;
    }

    size_t count = 0;
    for (const auto& issue : *it) {
        if (issue.is_object() && readBool(issue, flag_key).value_or(false)) {
            ++count;
        }
    }
    return count;
}

std::string readSubsystem(const nlohmann::json& issue) {
    if (const auto subsystem = readString(issue, "owningSubsystem")) {
        return *subsystem;
    }
    if (const auto subsystem = readString(issue, "subsystem")) {
        return *subsystem;
    }
    return {};
}

ProjectHealthFixCard makeFixCard(const nlohmann::json& issue) {
    ProjectHealthFixCard card{};
    card.code = readString(issue, "code").value_or("");
    card.title = readString(issue, "title").value_or(card.code.empty() ? "Project health issue" : card.code);
    card.detail = readString(issue, "detail").value_or("");
    card.owning_subsystem = readSubsystem(issue);
    card.severity = parseSeverity(issue);
    card.blocks_release = readBool(issue, "blocksRelease").value_or(false);
    card.blocks_export = readBool(issue, "blocksExport").value_or(false);
    card.affected_paths = readStringArray(issue, "affectedPaths");
    if (card.affected_paths.empty()) {
        card.affected_paths = readStringArray(issue, "paths");
    }
    if (card.affected_paths.empty()) {
        card.affected_paths = readStringArray(issue, "path");
    }
    card.validation_commands = readStringArray(issue, "validationCommands");
    if (card.validation_commands.empty()) {
        card.validation_commands = readStringArray(issue, "validationCommand");
    }
    card.acceptance_criteria = readStringArray(issue, "acceptanceCriteria");
    card.group = classifyGroup(card);
    return card;
}

void appendSyntheticCountCard(std::vector<ProjectHealthFixCard>& cards,
                              ProjectHealthGroup group,
                              ProjectHealthSeverity severity,
                              std::string code,
                              std::string title,
                              size_t count) {
    if (count == 0) {
        return;
    }

    ProjectHealthFixCard card{};
    card.code = std::move(code);
    card.title = std::move(title);
    card.detail = "Audit reported " + std::to_string(count) + " issue(s), but did not include expanded issue records.";
    card.severity = severity;
    card.group = group;
    card.blocks_release = group == ProjectHealthGroup::ReleaseBlockers;
    card.blocks_export = group == ProjectHealthGroup::ExportBlockers;
    cards.push_back(std::move(card));
}

bool fixCardLess(const ProjectHealthFixCard& lhs, const ProjectHealthFixCard& rhs) {
    if (severityRank(lhs.severity) != severityRank(rhs.severity)) {
        return severityRank(lhs.severity) < severityRank(rhs.severity);
    }
    if (lhs.blocks_release != rhs.blocks_release) {
        return lhs.blocks_release;
    }
    if (lhs.blocks_export != rhs.blocks_export) {
        return lhs.blocks_export;
    }
    if (lhs.owning_subsystem != rhs.owning_subsystem) {
        return lhs.owning_subsystem < rhs.owning_subsystem;
    }
    if (lhs.code != rhs.code) {
        return lhs.code < rhs.code;
    }
    return lhs.title < rhs.title;
}

ProjectHealthGroupCard buildGroup(ProjectHealthGroup group, const std::vector<ProjectHealthFixCard>& cards) {
    ProjectHealthGroupCard group_card{};
    group_card.group = group;
    group_card.title = toString(group);
    for (const auto& card : cards) {
        if (card.group != group) {
            continue;
        }
        group_card.fixes.push_back(card);
        ++group_card.issue_count;
        if (card.blocks_release || card.blocks_export) {
            ++group_card.blocker_count;
        }
    }
    std::sort(group_card.fixes.begin(), group_card.fixes.end(), fixCardLess);
    return group_card;
}

} // namespace

const char* toString(ProjectHealthSeverity severity) {
    switch (severity) {
    case ProjectHealthSeverity::Error:
        return "error";
    case ProjectHealthSeverity::Warning:
        return "warning";
    case ProjectHealthSeverity::Info:
    default:
        return "info";
    }
}

const char* toString(ProjectHealthGroup group) {
    switch (group) {
    case ProjectHealthGroup::ReleaseBlockers:
        return "release_blockers";
    case ProjectHealthGroup::ExportBlockers:
        return "export_blockers";
    case ProjectHealthGroup::Warnings:
        return "warnings";
    case ProjectHealthGroup::GovernanceIssues:
        return "governance_issues";
    case ProjectHealthGroup::AssetIssues:
        return "asset_issues";
    case ProjectHealthGroup::SchemaIssues:
        return "schema_issues";
    }
    return "warnings";
}

void ProjectHealthModel::ingestAuditReport(const nlohmann::json& report) {
    snapshot_ = {};
    if (!report.is_object()) {
        return;
    }

    snapshot_.has_data = true;
    snapshot_.headline = readString(report, "headline").value_or("Project health");
    snapshot_.summary = readString(report, "summary").value_or("");
    snapshot_.status_date = readString(report, "statusDate").value_or("");
    snapshot_.readiness_date = readString(report, "readinessDate").value_or(
        readString(report, "readinessStatusDate").value_or(""));
    snapshot_.stale = !snapshot_.status_date.empty() && !snapshot_.readiness_date.empty() &&
                      snapshot_.status_date != snapshot_.readiness_date;

    snapshot_.release_blocker_count = readCount(report, "releaseBlockerCount").value_or(
        countIssuesWithFlag(report, "blocksRelease"));
    snapshot_.export_blocker_count = readCount(report, "exportBlockerCount").value_or(
        countIssuesWithFlag(report, "blocksExport"));

    std::vector<ProjectHealthFixCard> cards;
    const auto issues = report.find("issues");
    if (issues != report.end() && issues->is_array()) {
        for (const auto& issue : *issues) {
            if (issue.is_object()) {
                cards.push_back(makeFixCard(issue));
            }
        }
    }

    appendSyntheticCountCard(cards,
                             ProjectHealthGroup::ReleaseBlockers,
                             ProjectHealthSeverity::Error,
                             "release_blocker_count",
                             "Release blockers need review",
                             snapshot_.release_blocker_count > countIssuesWithFlag(report, "blocksRelease")
                                 ? snapshot_.release_blocker_count - countIssuesWithFlag(report, "blocksRelease")
                                 : 0);
    appendSyntheticCountCard(cards,
                             ProjectHealthGroup::ExportBlockers,
                             ProjectHealthSeverity::Error,
                             "export_blocker_count",
                             "Export blockers need review",
                             snapshot_.export_blocker_count > countIssuesWithFlag(report, "blocksExport")
                                 ? snapshot_.export_blocker_count - countIssuesWithFlag(report, "blocksExport")
                                 : 0);
    appendSyntheticCountCard(cards,
                             ProjectHealthGroup::AssetIssues,
                             ProjectHealthSeverity::Warning,
                             "asset_governance_count",
                             "Asset governance issues need review",
                             readCount(report, "assetGovernanceIssueCount").value_or(0));
    appendSyntheticCountCard(cards,
                             ProjectHealthGroup::SchemaIssues,
                             ProjectHealthSeverity::Warning,
                             "schema_governance_count",
                             "Schema governance issues need review",
                             readCount(report, "schemaGovernanceIssueCount").value_or(0));
    appendSyntheticCountCard(cards,
                             ProjectHealthGroup::GovernanceIssues,
                             ProjectHealthSeverity::Warning,
                             "project_artifact_count",
                             "Project artifact governance issues need review",
                             readCount(report, "projectArtifactIssueCount").value_or(0));

    std::sort(cards.begin(), cards.end(), fixCardLess);
    snapshot_.fix_next = cards;
    snapshot_.issue_count = cards.size();

    constexpr std::array<ProjectHealthGroup, 6> groups = {
        ProjectHealthGroup::ReleaseBlockers,
        ProjectHealthGroup::ExportBlockers,
        ProjectHealthGroup::Warnings,
        ProjectHealthGroup::GovernanceIssues,
        ProjectHealthGroup::AssetIssues,
        ProjectHealthGroup::SchemaIssues,
    };
    for (const auto group : groups) {
        auto group_card = buildGroup(group, cards);
        if (group_card.issue_count > 0) {
            snapshot_.groups.push_back(std::move(group_card));
        }
    }
}

void ProjectHealthModel::clear() {
    snapshot_ = {};
}

} // namespace urpg::editor
