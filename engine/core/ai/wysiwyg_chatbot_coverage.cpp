#include "engine/core/ai/wysiwyg_chatbot_coverage.h"

#include "engine/core/editor/editor_panel_registry.h"

#include <algorithm>

namespace urpg::ai {

namespace {

bool hasToolForCapability(const AiToolRegistry& tools, const std::string& capabilityId) {
    return std::any_of(tools.tools().begin(), tools.tools().end(), [&](const auto& tool) {
        return tool.capability_id == capabilityId;
    });
}

void addMissing(nlohmann::json& missing, std::string kind, std::string id, std::string message) {
    missing.push_back({{"kind", std::move(kind)}, {"id", std::move(id)}, {"message", std::move(message)}});
}

} // namespace

nlohmann::json WysiwygChatbotCoverageReport::toJson() const {
    return {
        {"passed", passed},
        {"release_panel_count", release_panel_count},
        {"searchable_panel_count", searchable_panel_count},
        {"capability_count", capability_count},
        {"capability_with_tool_count", capability_with_tool_count},
        {"capability_with_wysiwyg_surface_count", capability_with_wysiwyg_surface_count},
        {"asset_panel_registered", asset_panel_registered},
        {"asset_chatbot_tool_registered", asset_chatbot_tool_registered},
        {"asset_library_actions_available", asset_library_actions_available},
        {"missing", missing},
    };
}

WysiwygChatbotCoverageReport buildWysiwygChatbotCoverageReport(const AiKnowledgeSnapshot& knowledge,
                                                               const urpg::assets::AssetLibrarySnapshot& assets) {
    WysiwygChatbotCoverageReport report;
    for (const auto& panel : urpg::editor::editorPanelRegistry()) {
        if (panel.exposure != urpg::editor::EditorPanelExposure::ReleaseTopLevel) {
            continue;
        }
        ++report.release_panel_count;
        const auto matches = knowledge.docs_index.search(panel.id + " " + panel.title);
        if (!matches.empty()) {
            ++report.searchable_panel_count;
        } else {
            addMissing(report.missing, "editor_panel_chatbot_index", panel.id,
                       "Release top-level WYSIWYG panel is not searchable by chatbot knowledge.");
        }
    }

    report.capability_count = knowledge.capabilities.capabilities().size();
    for (const auto& capability : knowledge.capabilities.capabilities()) {
        if (!capability.wysiwyg_surface.empty()) {
            ++report.capability_with_wysiwyg_surface_count;
        } else {
            addMissing(report.missing, "capability_wysiwyg_surface", capability.id,
                       "AI capability does not declare a WYSIWYG surface.");
        }
        if (hasToolForCapability(knowledge.tools, capability.id)) {
            ++report.capability_with_tool_count;
        } else {
            addMissing(report.missing, "capability_tool", capability.id,
                       "AI capability has no chatbot tool registered.");
        }
    }

    report.asset_panel_registered = urpg::editor::findEditorPanelRegistryEntry("assets") != nullptr;
    report.asset_chatbot_tool_registered = knowledge.tools.find("import_asset_record") != nullptr;
    report.asset_library_actions_available = assets.promoted_count > 0 || assets.archived_count > 0 ||
                                             assets.runtime_ready_count > 0 || assets.previewable_count > 0;
    if (!report.asset_panel_registered) {
        addMissing(report.missing, "asset_wysiwyg_panel", "assets", "Asset library panel is not registered.");
    }
    if (!report.asset_chatbot_tool_registered) {
        addMissing(report.missing, "asset_chatbot_tool", "import_asset_record",
                   "Asset import/promotion chatbot tool is not registered.");
    }

    report.passed = report.missing.empty() &&
                    report.release_panel_count > 0 &&
                    report.release_panel_count == report.searchable_panel_count &&
                    report.capability_count == report.capability_with_tool_count &&
                    report.capability_count == report.capability_with_wysiwyg_surface_count &&
                    report.asset_panel_registered &&
                    report.asset_chatbot_tool_registered;
    return report;
}

} // namespace urpg::ai
