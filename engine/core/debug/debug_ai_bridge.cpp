#include "debug_ai_bridge.h"
#include <sstream>

namespace urpg::ai {

std::string DebugKnowledgeBridge::formatCallStack(const std::vector<urpg::DebugFrame>& frames) {
    std::stringstream ss;
    ss << "### Call Stack ###\n";
    if (frames.empty()) {
        ss << "(Empty)\n";
        return ss.str();
    }
    for (size_t i = 0; i < frames.size(); ++i) {
        const auto& f = frames[frames.size() - 1 - i]; // Show top-most first
        ss << " [" << i << "] Event: " << f.event_id << " | Block: " << f.block_id << "\n";
    }
    return ss.str();
}

std::string DebugKnowledgeBridge::formatWatchTable(const std::vector<urpg::WatchValue>& watches) {
    std::stringstream ss;
    ss << "### Active Watches ###\n";
    if (watches.empty()) {
        ss << "(No active watches)\n";
        return ss.str();
    }
    for (const auto& w : watches) {
        ss << " " << w.key << " = " << w.value << "\n";
    }
    return ss.str();
}

std::string DebugKnowledgeBridge::generateDebugPrompt(const urpg::DebugRuntimeSession& session,
                                                      const std::string& error_message) {
    std::stringstream ss;
    ss << "### Runtime Debug Analysis ###\n";
    if (!error_message.empty()) {
        ss << "CRITICAL ERROR: " << error_message << "\n\n";
    }

    // Capture state from the session
    ss << formatCallStack(session.SnapshotStack());
    ss << "\n";
    ss << formatWatchTable(session.SnapshotWatches());

    ss << "\n### AI Objective ###\n";
    ss << "Analyze the state above. Explain the most likely cause of the "
       << (error_message.empty() ? "pause" : "error");
    ss << " and suggest a fix for the event logic.";

    return ss.str();
}

} // namespace urpg::ai
