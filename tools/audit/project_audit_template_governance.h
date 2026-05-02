#pragma once

#include "tools/audit/project_audit_support.h"

#include <cstddef>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::tools::audit {

void addTemplateSpecArtifactGovernance(
    const TemplateContext& templateContext,
    std::vector<AuditIssue>& issues,
    std::size_t& templateSpecArtifactIssueCount,
    std::size_t& releaseBlockerCount,
    std::size_t& exportBlockerCount,
    nlohmann::json& governanceReport);
void addSignoffArtifactGovernance(
    const nlohmann::json& readiness,
    std::vector<AuditIssue>& issues,
    std::size_t& signoffArtifactIssueCount,
    std::size_t& releaseBlockerCount,
    nlohmann::json& governanceReport);

} // namespace urpg::tools::audit
