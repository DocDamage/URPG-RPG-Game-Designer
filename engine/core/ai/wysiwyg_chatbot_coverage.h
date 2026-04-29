#pragma once

#include "engine/core/ai/ai_knowledge_base.h"
#include "engine/core/assets/asset_library.h"

#include <nlohmann/json.hpp>

namespace urpg::ai {

struct WysiwygChatbotCoverageReport {
    bool passed = false;
    std::size_t release_panel_count = 0;
    std::size_t searchable_panel_count = 0;
    std::size_t capability_count = 0;
    std::size_t capability_with_tool_count = 0;
    std::size_t capability_with_wysiwyg_surface_count = 0;
    bool asset_panel_registered = false;
    bool asset_chatbot_tool_registered = false;
    bool asset_library_actions_available = false;
    nlohmann::json missing = nlohmann::json::array();
    nlohmann::json toJson() const;
};

WysiwygChatbotCoverageReport buildWysiwygChatbotCoverageReport(const AiKnowledgeSnapshot& knowledge,
                                                               const urpg::assets::AssetLibrarySnapshot& assets);

} // namespace urpg::ai
