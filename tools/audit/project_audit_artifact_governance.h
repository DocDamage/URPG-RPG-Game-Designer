#pragma once

#include "tools/audit/project_audit_support.h"

#include <cstddef>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::tools::audit {

void addLocalizationArtifactGovernance(const TemplateContext& templateContext,
                                       std::vector<AuditIssue>& issues,
                                       std::size_t& localizationArtifactIssueCount,
                                       nlohmann::json& governanceReport);
void addExportArtifactGovernance(const TemplateContext& templateContext,
                                 std::vector<AuditIssue>& issues,
                                 std::size_t& exportArtifactIssueCount,
                                 nlohmann::json& governanceReport);
void addInputArtifactGovernance(const TemplateContext& templateContext,
                                std::vector<AuditIssue>& issues,
                                std::size_t& inputArtifactIssueCount,
                                nlohmann::json& governanceReport);
void addAccessibilityArtifactGovernance(const TemplateContext& templateContext,
                                        std::vector<AuditIssue>& issues,
                                        std::size_t& accessibilityArtifactIssueCount,
                                        nlohmann::json& governanceReport);
void addAudioArtifactGovernance(const TemplateContext& templateContext,
                                std::vector<AuditIssue>& issues,
                                std::size_t& audioArtifactIssueCount,
                                nlohmann::json& governanceReport);
void addAchievementArtifactGovernance(const TemplateContext& templateContext,
                                      std::vector<AuditIssue>& issues,
                                      std::size_t& achievementArtifactIssueCount,
                                      nlohmann::json& governanceReport);
void addCharacterArtifactGovernance(const TemplateContext& templateContext,
                                    std::vector<AuditIssue>& issues,
                                    std::size_t& characterArtifactIssueCount,
                                    nlohmann::json& governanceReport);
void addModArtifactGovernance(const TemplateContext& templateContext,
                              std::vector<AuditIssue>& issues,
                              std::size_t& modArtifactIssueCount,
                              nlohmann::json& governanceReport);
void addAnalyticsArtifactGovernance(const TemplateContext& templateContext,
                                    std::vector<AuditIssue>& issues,
                                    std::size_t& analyticsArtifactIssueCount,
                                    nlohmann::json& governanceReport);
void addPerformanceArtifactGovernance(const TemplateContext& templateContext,
                                      std::vector<AuditIssue>& issues,
                                      std::size_t& performanceArtifactIssueCount,
                                      nlohmann::json& governanceReport);
void addReleaseSignoffWorkflowGovernance(const TemplateContext& templateContext,
                                         std::vector<AuditIssue>& issues,
                                         std::size_t& releaseSignoffWorkflowIssueCount,
                                         nlohmann::json& governanceReport);

} // namespace urpg::tools::audit
