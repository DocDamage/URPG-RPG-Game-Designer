#pragma once

#include "engine/core/debug/debug_session.h"
#include <string>
#include <vector>

namespace urpg::ai {

/**
 * @brief Bridge between the DebugRuntimeSession and the AI Advisor.
 * This allows the AI to analyze call-stacks and watch-tables to explain bugs.
 */
class DebugKnowledgeBridge {
public:
    /**
     * @brief Generates a prompt describing the current PAUSED debug state.
     */
    static std::string generateDebugPrompt(
        const urpg::DebugRuntimeSession& session,
        const std::string& error_message = ""
    );

    /**
     * @brief Formats the call stack for the LLM.
     */
    static std::string formatCallStack(const std::vector<urpg::DebugFrame>& frames);

    /**
     * @brief Formats the watch table for the LLM.
     */
    static std::string formatWatchTable(const std::vector<urpg::WatchValue>& watches);
};

} // namespace urpg::ai