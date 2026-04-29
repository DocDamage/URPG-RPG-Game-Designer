#pragma once

#include "engine/core/ai/ai_knowledge_base.h"
#include "engine/core/ai/wysiwyg_chatbot_coverage.h"
#include "engine/core/assets/asset_action_view.h"
#include "engine/core/assets/asset_library.h"
#include "engine/core/message/message_core.h"
#include "engine/core/message/world_knowledge_bridge.h"
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
    virtual void requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) = 0;

    /**
     * @brief Optional streaming request.
     */
    virtual void requestStream(const std::vector<ChatMessage>& history, StreamCallback onChunk,
                               ChatCallback onComplete) {
        (void)onChunk;
        // Fallback to non-streaming if not implemented
        requestResponse(history, onComplete);
    }
};

/**
 * @brief A specialized Dialogue Node that acts as an entry point for AI Chat.
 */
class ChatbotComponent {
  public:
    ChatbotComponent(std::shared_ptr<IChatService> service) : m_service(std::move(service)) { rebuildAiKnowledge(); }

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
    void getResponse(const std::string& userInput, std::function<void(urpg::message::DialoguePage)> onReady) {
        prepareHistory(userInput);

        m_service->requestResponse(m_history, [this, onReady](const std::string& response, const std::string& command) {
            m_history.push_back({"assistant", response});

            // Process Tool Calling (Function Calling)
            if (!command.empty()) {
                this->executeTool(command);
            }

            urpg::message::DialoguePage page;
            page.body = response;
            page.command = command; // Pass through to local handlers if needed
            page.variant.speaker = "Mysterious AI";

            onReady(page);
        });
    }

    /**
     * @brief Executes a tool/function requested by the AI.
     */
    nlohmann::json executeTool(const std::string& command) {
        if (command.rfind("AI_TASK:", 0) == 0) {
            return planAiTask(command.substr(std::string("AI_TASK:").size()));
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
            {"type", "dialogue_command"},
            {"command", command},
            {"handled", result.handled},
            {"success", result.success},
            {"code", result.code},
            {"message", result.message},
        };
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
    void streamResponse(const std::string& userInput, IChatService::StreamCallback onChunk,
                        std::function<void(urpg::message::DialoguePage)> onComplete) {
        prepareHistory(userInput);

        m_service->requestStream(m_history, onChunk,
                                 [this, onComplete](const std::string& response, const std::string& command) {
                                     m_history.push_back({"assistant", response});

                                     urpg::message::DialoguePage page;
                                     page.body = response;
                                     page.command = command;
                                     page.variant.speaker = "Mysterious AI";

                                     onComplete(page);
                                 });
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

  private:
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
        };
    }

    void rebuildAiKnowledge() {
        m_aiKnowledge = buildDefaultAiKnowledgeSnapshot(m_projectData);
    }

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
    std::vector<ChatMessage> m_history;
    std::string m_systemPrompt;
    nlohmann::json m_projectData = nlohmann::json::object();
    urpg::assets::AssetLibrarySnapshot m_assetLibrarySnapshot{};
    AiKnowledgeSnapshot m_aiKnowledge;
    AiTaskPlan m_currentAiTaskPlan;
    nlohmann::json m_lastAiToolSnapshot = nlohmann::json::object();
};

} // namespace urpg::ai
