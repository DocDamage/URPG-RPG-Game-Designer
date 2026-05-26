#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace urpg::ai {

struct AiKnowledgeDiagnostic {
    std::string code;
    std::string message;
    std::string target;
};

struct AppCapability {
    AppCapability() = default;
    AppCapability(std::string idValue, std::string titleValue, std::string categoryValue,
                  std::string wysiwygSurfaceValue, std::vector<std::string> actionsValue,
                  std::vector<std::string> projectPathsValue, std::vector<std::string> keywordsValue,
                  std::string editorExposureValue = "release_top_level",
                  std::string accessLevelValue = "release_actionable", std::string panelIdValue = {},
                  std::string promotionGateValue = "review_gated_ai_tool")
        : id(std::move(idValue)), title(std::move(titleValue)), category(std::move(categoryValue)),
          wysiwyg_surface(std::move(wysiwygSurfaceValue)), actions(std::move(actionsValue)),
          project_paths(std::move(projectPathsValue)), keywords(std::move(keywordsValue)),
          editor_exposure(std::move(editorExposureValue)), access_level(std::move(accessLevelValue)),
          panel_id(std::move(panelIdValue)), promotion_gate(std::move(promotionGateValue)) {}

    std::string id;
    std::string title;
    std::string category;
    std::string wysiwyg_surface;
    std::vector<std::string> actions;
    std::vector<std::string> project_paths;
    std::vector<std::string> keywords;
    std::string editor_exposure = "release_top_level";
    std::string access_level = "release_actionable";
    std::string panel_id;
    std::string promotion_gate = "review_gated_ai_tool";
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
    AiToolDefinition() = default;
    AiToolDefinition(std::string idValue, std::string titleValue, std::string capabilityIdValue,
                     bool mutatesProjectValue, bool requiresApprovalValue,
                     std::vector<std::string> requiredFieldsValue,
                     std::string editorExposureValue = "release_top_level",
                     std::string accessLevelValue = "release_actionable", std::string panelIdValue = {},
                     std::string promotionGateValue = "review_gated_ai_tool")
        : id(std::move(idValue)), title(std::move(titleValue)), capability_id(std::move(capabilityIdValue)),
          mutates_project(mutatesProjectValue), requires_approval(requiresApprovalValue),
          required_fields(std::move(requiredFieldsValue)), editor_exposure(std::move(editorExposureValue)),
          access_level(std::move(accessLevelValue)), panel_id(std::move(panelIdValue)),
          promotion_gate(std::move(promotionGateValue)) {}

    std::string id;
    std::string title;
    std::string capability_id;
    bool mutates_project = false;
    bool requires_approval = true;
    std::vector<std::string> required_fields;
    std::string editor_exposure = "release_top_level";
    std::string access_level = "release_actionable";
    std::string panel_id;
    std::string promotion_gate = "review_gated_ai_tool";
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
    nlohmann::json approvalManifest(const AiTaskPlan& plan, const AppCapabilityRegistry& capabilities) const;
    std::vector<AiKnowledgeDiagnostic> validatePlan(const AiTaskPlan& plan) const;
    AiToolApplyResult applyApprovedPlan(const AiTaskPlan& plan, const nlohmann::json& projectData) const;
    nlohmann::json toJson() const;

    static AiToolRegistry buildDefault();

  private:
    std::vector<AiToolDefinition> tools_;
};

class AiTaskPlanner {
  public:
    AiTaskPlan planTask(const std::string& userRequest, const AppCapabilityRegistry& capabilities,
                        const ProjectKnowledgeIndex& projectIndex, const DocumentationKnowledgeIndex& docs,
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
nlohmann::json buildFilesystemKnowledgeReport(const nlohmann::json& projectData);
nlohmann::json buildFilesystemCrawlerInvocation(const nlohmann::json& projectData, const std::string& projectRoot,
                                                const std::string& outputPath);
nlohmann::json mergeFilesystemKnowledgeIntoProjectData(nlohmann::json projectData,
                                                       const nlohmann::json& filesystemKnowledge);
nlohmann::json buildAiToolResultDiff(const AiToolApplyResult& result);

} // namespace urpg::ai
