#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::ai {

struct AiKnowledgeDiagnostic {
    std::string code;
    std::string message;
    std::string target;
};

struct AppCapability {
    std::string id;
    std::string title;
    std::string category;
    std::string wysiwyg_surface;
    std::vector<std::string> actions;
    std::vector<std::string> project_paths;
    std::vector<std::string> keywords;
    nlohmann::json toJson() const;
};

class AppCapabilityRegistry {
public:
    void registerCapability(AppCapability capability);
    const AppCapability* find(const std::string& id) const;
    std::vector<AppCapability> search(const std::string& query) const;
    const std::vector<AppCapability>& capabilities() const { return capabilities_; }
    nlohmann::json toJson() const;

    static AppCapabilityRegistry buildDefault();

private:
    std::vector<AppCapability> capabilities_;
};

struct KnowledgeEntry {
    std::string id;
    std::string type;
    std::string title;
    std::string path;
    std::string summary;
    std::vector<std::string> keywords;
    nlohmann::json metadata = nlohmann::json::object();
    nlohmann::json toJson() const;
};

class ProjectKnowledgeIndex {
public:
    void addEntry(KnowledgeEntry entry);
    std::vector<KnowledgeEntry> search(const std::string& query) const;
    const std::vector<KnowledgeEntry>& entries() const { return entries_; }
    nlohmann::json toJson() const;

    static ProjectKnowledgeIndex buildFromProjectData(const nlohmann::json& projectData);

private:
    std::vector<KnowledgeEntry> entries_;
};

class DocumentationKnowledgeIndex {
public:
    void addEntry(KnowledgeEntry entry);
    std::vector<KnowledgeEntry> search(const std::string& query) const;
    const std::vector<KnowledgeEntry>& entries() const { return entries_; }
    nlohmann::json toJson() const;

    static DocumentationKnowledgeIndex buildDefault();

private:
    std::vector<KnowledgeEntry> entries_;
};

struct AiToolDefinition {
    std::string id;
    std::string title;
    std::string capability_id;
    bool mutates_project = false;
    bool requires_approval = true;
    std::vector<std::string> required_fields;
    nlohmann::json toJson() const;
};

struct AiToolStep {
    std::string id;
    std::string tool_id;
    std::string summary;
    nlohmann::json arguments = nlohmann::json::object();
    bool approved = false;
    bool rejected = false;
    nlohmann::json toJson() const;
};

struct AiTaskPlan {
    std::string schema = "urpg.ai_task_plan.v1";
    std::string id;
    std::string user_request;
    std::vector<std::string> capability_ids;
    std::vector<AiToolStep> steps;
    std::vector<AiKnowledgeDiagnostic> diagnostics;
    bool ready_for_approval = false;
    nlohmann::json toJson() const;
};

struct AiToolApplyResult {
    bool applied = false;
    nlohmann::json project_data = nlohmann::json::object();
    nlohmann::json before_project_data = nlohmann::json::object();
    nlohmann::json project_patch = nlohmann::json::array();
    nlohmann::json revert_patch = nlohmann::json::array();
    std::vector<AiKnowledgeDiagnostic> diagnostics;
    nlohmann::json toJson() const;
};

struct AiToolApprovalSummary {
    std::string step_id;
    std::string tool_id;
    std::string tool_title;
    std::string capability_id;
    std::string summary;
    bool mutates_project = false;
    bool requires_approval = false;
    bool approved = false;
    bool rejected = false;
    std::vector<std::string> project_paths;
    nlohmann::json arguments = nlohmann::json::object();
    nlohmann::json toJson() const;
};

class AiToolRegistry {
public:
    void registerTool(AiToolDefinition tool);
    const AiToolDefinition* find(const std::string& id) const;
    const std::vector<AiToolDefinition>& tools() const { return tools_; }
    std::vector<AiToolDefinition> mutatingToolsRequiringApproval() const;
    std::vector<AiToolApprovalSummary> pendingApprovalSteps(const AiTaskPlan& plan,
                                                            const AppCapabilityRegistry& capabilities) const;
    nlohmann::json approvalManifest(const AiTaskPlan& plan,
                                    const AppCapabilityRegistry& capabilities) const;
    std::vector<AiKnowledgeDiagnostic> validatePlan(const AiTaskPlan& plan) const;
    AiToolApplyResult applyApprovedPlan(const AiTaskPlan& plan, const nlohmann::json& projectData) const;
    nlohmann::json toJson() const;

    static AiToolRegistry buildDefault();

private:
    std::vector<AiToolDefinition> tools_;
};

class AiTaskPlanner {
public:
    AiTaskPlan planTask(const std::string& userRequest,
                        const AppCapabilityRegistry& capabilities,
                        const ProjectKnowledgeIndex& projectIndex,
                        const DocumentationKnowledgeIndex& docs,
                        const AiToolRegistry& tools) const;
};

struct AiKnowledgeSnapshot {
    AppCapabilityRegistry capabilities;
    ProjectKnowledgeIndex project_index;
    DocumentationKnowledgeIndex docs_index;
    AiToolRegistry tools;
    nlohmann::json toJson() const;
};

AiKnowledgeSnapshot buildDefaultAiKnowledgeSnapshot(const nlohmann::json& projectData = nlohmann::json::object());

} // namespace urpg::ai
