#pragma once

#include "engine/core/ai/ai_knowledge_base.h"
#include "engine/core/ai/wysiwyg_chatbot_coverage.h"
#include "engine/core/assets/asset_action_view.h"
#include "engine/core/assets/asset_library.h"
#include "engine/core/message/message_core.h"
#include "engine/core/message/world_knowledge_bridge.h"
#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace urpg::ai {

/**
 * @brief Represents a message in a chatbot conversation.
 */
struct ChatMessage {
    std::string role; // "user", "assistant", "system"
    std::string content;
};

class ChatRequestHandle {
  public:
    void cancel() { m_cancelled.store(true, std::memory_order_release); }
    bool cancelled() const { return m_cancelled.load(std::memory_order_acquire); }

  private:
    std::atomic_bool m_cancelled{false};
};

/**
 * @brief Interface for AI Chat Services.
 * This allows the game developer to plug in OpenAI, Anthropic, or a local model.
 */
class IChatService {
  public:
    virtual ~IChatService() = default;

    using ChatCallback = std::function<void(const std::string& response, const std::string& command)>;
    using StreamCallback = std::function<void(const std::string& chunk)>;

    /**
     * @brief Sends a prompt to the AI and gets a response.
     * @param history The conversation history.
     * @param callback Called when the AI responds.
     */
    virtual std::shared_ptr<ChatRequestHandle> requestResponse(const std::vector<ChatMessage>& history,
                                                               ChatCallback callback) = 0;

    /**
     * @brief Optional streaming request.
     */
    virtual std::shared_ptr<ChatRequestHandle> requestStream(const std::vector<ChatMessage>& history,
                                                             StreamCallback onChunk, ChatCallback onComplete) {
        (void)onChunk;
        // Fallback to non-streaming if not implemented
        return requestResponse(history, std::move(onComplete));
    }
};

/**
 * @brief A specialized Dialogue Node that acts as an entry point for AI Chat.
 */
class ChatbotComponent {
  public:
    ChatbotComponent(std::shared_ptr<IChatService> service) : m_service(std::move(service)) { rebuildAiKnowledge(); }
    ChatbotComponent(const ChatbotComponent& other)
        : m_service(other.m_service), m_history(other.m_history), m_systemPrompt(other.m_systemPrompt),
          m_projectData(other.m_projectData), m_assetLibrarySnapshot(other.m_assetLibrarySnapshot),
          m_aiKnowledge(other.m_aiKnowledge), m_currentAiTaskPlan(other.m_currentAiTaskPlan),
          m_lastAiToolSnapshot(other.m_lastAiToolSnapshot) {}
    ChatbotComponent& operator=(const ChatbotComponent& other) {
        if (this == &other) {
            return *this;
        }
        cancelPendingRequests();
        m_ownerToken = std::make_shared<ChatRequestHandle>();
        m_service = other.m_service;
        m_history = other.m_history;
        m_systemPrompt = other.m_systemPrompt;
        m_projectData = other.m_projectData;
        m_assetLibrarySnapshot = other.m_assetLibrarySnapshot;
        m_aiKnowledge = other.m_aiKnowledge;
        m_currentAiTaskPlan = other.m_currentAiTaskPlan;
        m_lastAiToolSnapshot = other.m_lastAiToolSnapshot;
        return *this;
    }
    ChatbotComponent(ChatbotComponent&&) noexcept = default;
    ChatbotComponent& operator=(ChatbotComponent&&) noexcept = default;
    ~ChatbotComponent() { cancelPendingRequests(); }

    void setSystemPrompt(const std::string& prompt) { m_systemPrompt = prompt; }
    void setProjectData(nlohmann::json projectData) {
        m_projectData = std::move(projectData);
        rebuildAiKnowledge();
    }
    void setAssetLibrarySnapshot(urpg::assets::AssetLibrarySnapshot assetLibrarySnapshot) {
        m_assetLibrarySnapshot = std::move(assetLibrarySnapshot);
    }

    const nlohmann::json& projectData() const { return m_projectData; }

    /**
     * @brief Interacts with the AI.
     * @param userInput The text typed by the player.
     * @param onReady Callback providing a DialoguePage ready for the MessageFlowRunner.
     */
    std::shared_ptr<ChatRequestHandle> getResponse(const std::string& userInput,
                                                   std::function<void(urpg::message::DialoguePage)> onReady) {
        prepareHistory(userInput);

        auto requestHandle = registerPendingRequest();
        auto ownerToken = m_ownerToken;
        auto transportHandle =
            m_service->requestResponse(m_history, [this, ownerToken, requestHandle,
                                                   onReady = std::move(onReady)](const std::string& response,
                                                                                 const std::string& command) {
            if (ownerToken->cancelled() || requestHandle->cancelled()) {
                return;
            }
            m_history.push_back({"assistant", response});

            // Process Tool Calling (Function Calling)
            if (!command.empty()) {
                this->executeTool(command);
            }

            urpg::message::DialoguePage page;
            page.body = response;
            page.command = command; // Pass through to local handlers if needed
            page.variant.speaker = "Mysterious AI";

            completePendingRequest(requestHandle);
            onReady(page);
        });
        trackTransportHandle(std::move(transportHandle));
        return requestHandle;
    }

    /**
     * @brief Executes a tool/function requested by the AI.
     */
    nlohmann::json executeTool(const std::string& command) {
        if (command.rfind("AI_TASK:", 0) == 0) {
            return planAiTask(command.substr(std::string("AI_TASK:").size()));
        }
        if (command.rfind("AI_INGEST_FILESYSTEM_KNOWLEDGE:", 0) == 0) {
            return ingestFilesystemKnowledgeCommand(
                command.substr(std::string("AI_INGEST_FILESYSTEM_KNOWLEDGE:").size()));
        }
        if (command == "AI_REFRESH_FILESYSTEM_KNOWLEDGE" || command.rfind("AI_REFRESH_FILESYSTEM_KNOWLEDGE:", 0) == 0) {
            return refreshFilesystemKnowledgeCommand(command);
        }
        if (command == "AI_FILESYSTEM_KNOWLEDGE") {
            m_lastAiToolSnapshot = aiToolSnapshot();
            return m_lastAiToolSnapshot;
        }
        if (command.rfind("AI_APPROVE_STEP:", 0) == 0) {
            return approveAiToolStep(command.substr(std::string("AI_APPROVE_STEP:").size()));
        }
        if (command.rfind("AI_REJECT_STEP:", 0) == 0) {
            return rejectAiToolStep(command.substr(std::string("AI_REJECT_STEP:").size()));
        }
        if (command == "AI_APPROVE_ALL") {
            const auto approved = approveAllAiToolSteps();
            m_lastAiToolSnapshot = aiToolSnapshot();
            m_lastAiToolSnapshot["approved_count"] = approved;
            return m_lastAiToolSnapshot;
        }
        if (command == "AI_APPLY") {
            return applyApprovedAiToolPlan();
        }
        if (command == "AI_REVERT") {
            return revertLatestAiToolChange();
        }

        urpg::message::DialogueCommandProcessor processor;
        const auto result = processor.execute(command);
        m_lastAiToolSnapshot = {
            {"type", "dialogue_command"}, {"command", command},  {"handled", result.handled},
            {"success", result.success},  {"code", result.code}, {"message", result.message},
        };
        return m_lastAiToolSnapshot;
    }

    nlohmann::json refreshFilesystemKnowledgeCommand(const std::string& command) {
        std::string root = ".";
        std::string outputPath = ".urpg/ai/filesystem_knowledge.json";
        if (command.rfind("AI_REFRESH_FILESYSTEM_KNOWLEDGE:", 0) == 0) {
            try {
                const auto payload =
                    nlohmann::json::parse(command.substr(std::string("AI_REFRESH_FILESYSTEM_KNOWLEDGE:").size()));
                root = payload.value("root", root);
                outputPath = payload.value("output_path", outputPath);
            } catch (const nlohmann::json::exception&) {
                m_lastAiToolSnapshot = aiToolSnapshot();
                m_lastAiToolSnapshot["filesystem_knowledge_refresh"] = {
                    {"available", false},
                    {"status", "invalid_request"},
                    {"error", "invalid_json"},
                };
                return m_lastAiToolSnapshot;
            }
        }
        m_lastAiToolSnapshot = aiToolSnapshot();
        m_lastAiToolSnapshot["filesystem_knowledge_refresh"] =
            buildFilesystemCrawlerInvocation(m_projectData, root, outputPath);
        return m_lastAiToolSnapshot;
    }

    nlohmann::json planAiTask(const std::string& userRequest) {
        rebuildAiKnowledge();
        AiTaskPlanner planner;
        m_currentAiTaskPlan = planner.planTask(userRequest, m_aiKnowledge.capabilities, m_aiKnowledge.project_index,
                                               m_aiKnowledge.docs_index, m_aiKnowledge.tools);
        m_lastAiToolSnapshot = aiToolSnapshot();
        return m_lastAiToolSnapshot;
    }

    nlohmann::json ingestFilesystemKnowledgeCommand(const std::string& payload) {
        try {
            const auto filesystemKnowledge = nlohmann::json::parse(payload);
            m_projectData = mergeFilesystemKnowledgeIntoProjectData(std::move(m_projectData), filesystemKnowledge);
            rebuildAiKnowledge();
            m_lastAiToolSnapshot = aiToolSnapshot();
            m_lastAiToolSnapshot["ingested_filesystem_knowledge"] = {
                {"success", true},
                {"report", buildFilesystemKnowledgeReport(m_projectData)},
            };
        } catch (const nlohmann::json::exception& ex) {
            m_lastAiToolSnapshot = aiToolSnapshot();
            m_lastAiToolSnapshot["ingested_filesystem_knowledge"] = {
                {"success", false},
                {"error", "invalid_json"},
                {"message", ex.what()},
            };
        }
        return m_lastAiToolSnapshot;
    }

    bool approveAiToolStepById(const std::string& stepId) {
        if (m_currentAiTaskPlan.id.empty()) {
            return false;
        }
        for (auto& step : m_currentAiTaskPlan.steps) {
            if (step.id == stepId) {
                step.approved = true;
                step.rejected = false;
                return true;
            }
        }
        return false;
    }

    nlohmann::json approveAiToolStep(const std::string& stepId) {
        const bool approved = approveAiToolStepById(stepId);
        m_lastAiToolSnapshot = aiToolSnapshot();
        m_lastAiToolSnapshot["approved"] = approved;
        return m_lastAiToolSnapshot;
    }

    nlohmann::json rejectAiToolStep(const std::string& stepId) {
        bool rejected = false;
        if (!m_currentAiTaskPlan.id.empty()) {
            for (auto& step : m_currentAiTaskPlan.steps) {
                if (step.id == stepId) {
                    step.approved = false;
                    step.rejected = true;
                    rejected = true;
                    break;
                }
            }
        }
        m_lastAiToolSnapshot = aiToolSnapshot();
        m_lastAiToolSnapshot["rejected"] = rejected;
        return m_lastAiToolSnapshot;
    }

    std::size_t approveAllAiToolSteps() {
        if (m_currentAiTaskPlan.id.empty()) {
            return 0;
        }
        std::size_t approved = 0;
        for (auto& step : m_currentAiTaskPlan.steps) {
            const auto* tool = m_aiKnowledge.tools.find(step.tool_id);
            if (tool != nullptr && tool->requires_approval && !step.approved && !step.rejected) {
                step.approved = true;
                ++approved;
            }
        }
        return approved;
    }

    nlohmann::json applyApprovedAiToolPlan() {
        auto result = m_aiKnowledge.tools.applyApprovedPlan(m_currentAiTaskPlan, m_projectData);
        m_projectData = result.project_data;
        if (result.applied) {
            auto& history = m_projectData["_ai_change_history"];
            if (!history.is_array()) {
                history = nlohmann::json::array();
            }
            history.push_back(makeAiChangeRecord(result, history.size()));
            result.project_data = m_projectData;
            rebuildAiKnowledge();
        }
        m_lastAiToolSnapshot = aiToolSnapshot();
        m_lastAiToolSnapshot["last_apply"] = result.toJson();
        m_lastAiToolSnapshot["result_diff"] = buildAiToolResultDiff(result);
        return m_lastAiToolSnapshot;
    }

    nlohmann::json revertLatestAiToolChange() {
        const auto changeIndex = latestUnrevertedAiChangeIndex();
        if (!changeIndex.has_value()) {
            m_lastAiToolSnapshot = aiToolSnapshot();
            m_lastAiToolSnapshot["last_revert"] = {{"reverted", false}, {"reason", "no_applied_ai_change"}};
            return m_lastAiToolSnapshot;
        }

        const auto change = m_projectData["_ai_change_history"][*changeIndex];
        try {
            m_projectData = m_projectData.patch(change.value("revert_patch", nlohmann::json::array()));
        } catch (const nlohmann::json::exception&) {
            m_lastAiToolSnapshot = aiToolSnapshot();
            m_lastAiToolSnapshot["last_revert"] = {{"reverted", false}, {"reason", "patch_apply_failed"}};
            return m_lastAiToolSnapshot;
        }

        auto& history = m_projectData["_ai_change_history"];
        if (!history.is_array()) {
            history = nlohmann::json::array();
        }
        if (*changeIndex < history.size()) {
            history[*changeIndex]["reverted"] = true;
        } else {
            auto restoredChange = change;
            restoredChange["reverted"] = true;
            history.push_back(std::move(restoredChange));
        }
        rebuildAiKnowledge();
        m_lastAiToolSnapshot = aiToolSnapshot();
        m_lastAiToolSnapshot["last_revert"] = {
            {"reverted", true},
            {"change_id", change.value("change_id", "")},
            {"revert_patch", change.value("revert_patch", nlohmann::json::array())},
            {"project_data", m_projectData},
        };
        return m_lastAiToolSnapshot;
    }

    nlohmann::json aiToolSnapshot() const {
        return {
            {"project_index_count", m_aiKnowledge.project_index.entries().size()},
            {"capability_count", m_aiKnowledge.capabilities.capabilities().size()},
            {"tool_count", m_aiKnowledge.tools.tools().size()},
            {"wysiwyg_chatbot_coverage",
             buildWysiwygChatbotCoverageReport(m_aiKnowledge, m_assetLibrarySnapshot).toJson()},
            {"asset_action_rows", urpg::assets::buildAssetActionRows(m_assetLibrarySnapshot)},
            {"asset_preview_rows", urpg::assets::buildAssetPreviewRows(m_assetLibrarySnapshot)},
            {"filesystem_knowledge", buildFilesystemKnowledgeReport(m_projectData)},
            {"task_plan", m_currentAiTaskPlan.toJson()},
            {"approval", m_aiKnowledge.tools.approvalManifest(m_currentAiTaskPlan, m_aiKnowledge.capabilities)},
            {"controls", buildAiToolControls()},
            {"apply_history", buildApplyHistorySnapshot()},
        };
    }

    const nlohmann::json& lastAiToolSnapshot() const { return m_lastAiToolSnapshot; }

    /**
     * @brief Streams the AI response in real-time.
     */
    std::shared_ptr<ChatRequestHandle> streamResponse(const std::string& userInput, IChatService::StreamCallback onChunk,
                                                      std::function<void(urpg::message::DialoguePage)> onComplete) {
        prepareHistory(userInput);

        auto requestHandle = registerPendingRequest();
        auto ownerToken = m_ownerToken;
        auto guardedChunk = [ownerToken, requestHandle, onChunk = std::move(onChunk)](const std::string& chunk) {
            if (ownerToken->cancelled() || requestHandle->cancelled()) {
                return;
            }
            onChunk(chunk);
        };
        auto transportHandle =
            m_service->requestStream(m_history, std::move(guardedChunk),
                                     [this, ownerToken, requestHandle,
                                      onComplete = std::move(onComplete)](const std::string& response,
                                                                          const std::string& command) {
                                         if (ownerToken->cancelled() || requestHandle->cancelled()) {
                                             return;
                                         }
                                     m_history.push_back({"assistant", response});

                                     urpg::message::DialoguePage page;
                                     page.body = response;
                                     page.command = command;
                                     page.variant.speaker = "Mysterious AI";

                                     completePendingRequest(requestHandle);
                                     onComplete(page);
                                 });
        trackTransportHandle(std::move(transportHandle));
        return requestHandle;
    }

    void clearHistory() { m_history.clear(); }

    /**
     * @brief Internal access for sync services.
     */
    const std::vector<ChatMessage>& getHistory() const { return m_history; }

    /**
     * @brief Restore a conversation history from serialized persistence state.
     * The in-tree sync path currently proves process-local restore only; callers
     * may also inject histories loaded by out-of-tree persistence backends.
     */
    void restoreHistory(const std::vector<ChatMessage>& history) { m_history = history; }
    void cancelPendingRequests() {
        if (m_ownerToken) {
            m_ownerToken->cancel();
        }
        for (const auto& handle : m_pendingRequests) {
            if (handle) {
                handle->cancel();
            }
        }
        for (const auto& handle : m_transportRequests) {
            if (handle) {
                handle->cancel();
            }
        }
        m_pendingRequests.clear();
        m_transportRequests.clear();
        m_ownerToken = std::make_shared<ChatRequestHandle>();
    }

  private:
    std::shared_ptr<ChatRequestHandle> registerPendingRequest() {
        auto handle = std::make_shared<ChatRequestHandle>();
        m_pendingRequests.push_back(handle);
        return handle;
    }

    void completePendingRequest(const std::shared_ptr<ChatRequestHandle>& handle) {
        if (handle) {
            handle->cancel();
        }
        m_pendingRequests.erase(std::remove(m_pendingRequests.begin(), m_pendingRequests.end(), handle),
                                m_pendingRequests.end());
    }

    void trackTransportHandle(std::shared_ptr<ChatRequestHandle> handle) {
        if (!handle || handle->cancelled()) {
            return;
        }
        m_transportRequests.push_back(std::move(handle));
    }

    nlohmann::json makeAiChangeRecord(const AiToolApplyResult& result, std::size_t index) const {
        return {
            {"change_id", "ai_change_" + std::to_string(index + 1)},
            {"plan_id", m_currentAiTaskPlan.id},
            {"user_request", m_currentAiTaskPlan.user_request},
            {"step_count", m_currentAiTaskPlan.steps.size()},
            {"forward_patch", result.project_patch},
            {"revert_patch", result.revert_patch},
            {"before_project_data", result.before_project_data},
            {"after_project_data", result.project_data},
            {"reverted", false},
        };
    }

    std::optional<std::size_t> latestUnrevertedAiChangeIndex() const {
        if (!m_projectData.is_object() || !m_projectData.contains("_ai_change_history") ||
            !m_projectData["_ai_change_history"].is_array()) {
            return std::nullopt;
        }
        const auto& history = m_projectData["_ai_change_history"];
        for (std::size_t offset = 0; offset < history.size(); ++offset) {
            const auto index = history.size() - 1U - offset;
            if (!history[index].value("reverted", false)) {
                return index;
            }
        }
        return std::nullopt;
    }

    nlohmann::json buildApplyHistorySnapshot() const {
        nlohmann::json entries = nlohmann::json::array();
        const auto latestIndex = latestUnrevertedAiChangeIndex();
        if (m_projectData.is_object() && m_projectData.contains("_ai_change_history") &&
            m_projectData["_ai_change_history"].is_array()) {
            const auto& history = m_projectData["_ai_change_history"];
            for (std::size_t index = 0; index < history.size(); ++index) {
                const auto& record = history[index];
                if (record.value("reverted", false)) {
                    continue;
                }
                const auto forwardPatch = record.value("forward_patch", nlohmann::json::array());
                const auto revertPatch = record.value("revert_patch", nlohmann::json::array());
                entries.push_back({
                    {"index", index},
                    {"change_id", record.value("change_id", "")},
                    {"plan_id", record.value("plan_id", "")},
                    {"can_revert", latestIndex.value_or(index) == index},
                    {"project_patch_count", forwardPatch.size()},
                    {"revert_patch_count", revertPatch.size()},
                    {"persisted_record", record},
                });
            }
        }
        return {
            {"count", entries.size()},
            {"can_revert_latest", latestIndex.has_value()},
            {"latest_change_id",
             latestIndex.has_value() && m_projectData["_ai_change_history"].is_array()
                 ? nlohmann::json(m_projectData["_ai_change_history"][*latestIndex].value("change_id", ""))
                 : nlohmann::json(nullptr)},
            {"entries", entries},
        };
    }

    nlohmann::json buildAiToolControls() const {
        const auto history = buildApplyHistorySnapshot();
        const bool canRevert = history.value("can_revert_latest", false);
        const auto filesystemReport = buildFilesystemKnowledgeReport(m_projectData);
        return {
            {"revert_button",
             {
                 {"visible", true},
                 {"enabled", canRevert},
                 {"label", "Revert AI Change"},
                 {"action", "AI_REVERT"},
             }},
            {"undo_stack",
             {
                 {"available", canRevert},
                 {"count", history.value("count", std::size_t{0})},
                 {"latest_change_id", history.value("latest_change_id", nlohmann::json(nullptr))},
             }},
            {"filesystem_knowledge",
             {
                 {"refresh_button",
                  {
                      {"visible", true},
                      {"enabled", true},
                      {"label", "Refresh Project Knowledge"},
                      {"action", "AI_REFRESH_FILESYSTEM_KNOWLEDGE"},
                      {"invocation",
                       buildFilesystemCrawlerInvocation(m_projectData, ".", ".urpg/ai/filesystem_knowledge.json")},
                  }},
                 {"report_button",
                  {
                      {"visible", true},
                      {"enabled", filesystemReport.value("available", false)},
                      {"label", "Review Project Knowledge"},
                      {"action", "AI_FILESYSTEM_KNOWLEDGE"},
                  }},
                 {"document_count", filesystemReport.value("document_count", std::size_t{0})},
                 {"skipped_count", filesystemReport.value("skipped_count", 0)},
                 {"diagnostic_count", filesystemReport.value("diagnostic_count", std::size_t{0})},
                 {"diagnostic_rows", filesystemReport.value("diagnostics", nlohmann::json::array())},
             }},
        };
    }

    void rebuildAiKnowledge() { m_aiKnowledge = buildDefaultAiKnowledgeSnapshot(m_projectData); }

    void prepareHistory(const std::string& userInput) {
        // ALWAYS refresh the dynamic world state context for every request
        // This ensures the AI knows if a quest progressed or a switch flipped since the last message.
        std::string dynamicContext = WorldKnowledgeBridge::generateContext();

        if (m_history.empty()) {
            m_history.push_back({"system", m_systemPrompt + "\n\n" + dynamicContext});
        } else {
            // Update the system prompt or inject as a turn-based context hint
            m_history[0].content = m_systemPrompt + "\n\n" + dynamicContext;
        }

        m_history.push_back({"user", userInput});
    }

    std::shared_ptr<IChatService> m_service;
    std::shared_ptr<ChatRequestHandle> m_ownerToken = std::make_shared<ChatRequestHandle>();
    std::vector<std::shared_ptr<ChatRequestHandle>> m_pendingRequests;
    std::vector<std::shared_ptr<ChatRequestHandle>> m_transportRequests;
    std::vector<ChatMessage> m_history;
    std::string m_systemPrompt;
    nlohmann::json m_projectData = nlohmann::json::object();
    urpg::assets::AssetLibrarySnapshot m_assetLibrarySnapshot{};
    AiKnowledgeSnapshot m_aiKnowledge;
    AiTaskPlan m_currentAiTaskPlan;
    nlohmann::json m_lastAiToolSnapshot = nlohmann::json::object();
};

} // namespace urpg::ai
