#pragma once

#include "tools/audit/project_audit_support.h"

#include <cstddef>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::tools::audit {

void addSubsystemIssues(
    const nlohmann::json& readiness,
    const TemplateContext& templateContext,
    std::vector<AuditIssue>& issues,
    std::size_t& releaseBlockerCount,
    std::size_t& exportBlockerCount);
void addTemplateBarIssues(
    const TemplateContext& templateContext,
    std::vector<AuditIssue>& issues,
    std::size_t& releaseBlockerCount,
    std::size_t& exportBlockerCount);
void addMainBlockerIssues(
    const TemplateContext& templateContext,
    std::vector<AuditIssue>& issues,
    std::size_t& releaseBlockerCount,
    std::size_t& exportBlockerCount);
void addAssetReportIssues(
    const TemplateContext& templateContext,
    const AssetReportContext& assetReportContext,
    std::vector<AuditIssue>& issues,
    std::size_t& assetGovernanceIssueCount);
void addSchemaGovernanceIssues(
    const nlohmann::json& readiness,
    std::vector<AuditIssue>& issues,
    std::size_t& schemaGovernanceIssueCount,
    nlohmann::json& governanceReport);
void addProjectArtifactIssues(
    const TemplateContext& templateContext,
    const ProjectSchemaContext& projectSchemaContext,
    std::vector<AuditIssue>& issues,
    std::size_t& projectArtifactIssueCount,
    nlohmann::json& governanceReport);
void addLocalizationEvidenceIssues(
    const TemplateContext& templateContext,
    const LocalizationReportContext& localizationReportContext,
    std::vector<AuditIssue>& issues,
    std::size_t& localizationEvidenceIssueCount,
    nlohmann::json& governanceReport);

} // namespace urpg::tools::audit
