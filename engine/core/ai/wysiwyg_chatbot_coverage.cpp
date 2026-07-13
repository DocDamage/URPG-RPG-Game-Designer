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

bool hasReadonlyPanelTools(const AiToolRegistry& tools) {
    const auto* describe = tools.find("describe_panel");
    const auto* list = tools.find("list_panel_actions");
    const auto* route = tools.find("route_to_panel");
    return describe != nullptr && list != nullptr && route != nullptr && !describe->mutates_project &&
           !list->mutates_project && !route->mutates_project;
}

bool hasCapabilityForPanel(const AiKnowledgeSnapshot& knowledge, const std::string& panelId) {
    return std::any_of(knowledge.capabilities.capabilities().begin(), knowledge.capabilities.capabilities().end(),
                       [&](const auto& capability) {
                           return (capability.panel_id == panelId || capability.panel_id == "*") &&
                                  hasToolForCapability(knowledge.tools, capability.id);
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
        {"editor_panel_count", editor_panel_count},
        {"searchable_editor_panel_count", searchable_editor_panel_count},
        {"release_panel_with_chatbot_coverage_count", release_panel_with_chatbot_coverage_count},
        {"non_release_panel_count", non_release_panel_count},
        {"non_release_panel_discoverable_count", non_release_panel_discoverable_count},
        {"mutating_tool_count", mutating_tool_count},
        {"mutating_tool_requires_approval_count", mutating_tool_requires_approval_count},
        {"unsafe_mutating_tool_count", unsafe_mutating_tool_count},
        {"asset_panel_registered", asset_panel_registered},
        {"asset_chatbot_tool_registered", asset_chatbot_tool_registered},
        {"asset_library_actions_available", asset_library_actions_available},
        {"missing", missing},
    };
}

WysiwygChatbotCoverageReport buildWysiwygChatbotCoverageReport(const AiKnowledgeSnapshot& knowledge,
                                                               const urpg::assets::AssetLibrarySnapshot& assets) {
    WysiwygChatbotCoverageReport report;
    const bool panelToolsAvailable = hasReadonlyPanelTools(knowledge.tools);
    for (const auto& panel : urpg::editor::editorPanelRegistry()) {
        ++report.editor_panel_count;
        const auto matches = knowledge.docs_index.search(panel.id + " " + panel.title);
        const bool searchable = !matches.empty();
        if (searchable) {
            ++report.searchable_editor_panel_count;
        } else {
            addMissing(report.missing, "editor_panel_chatbot_index", panel.id,
                       "Editor panel is not searchable by chatbot knowledge.");
        }
        if (panel.exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel) {
            ++report.release_panel_count;
            if (searchable) {
                ++report.searchable_panel_count;
            }
            if (searchable && (hasCapabilityForPanel(knowledge, panel.id) || panelToolsAvailable)) {
                ++report.release_panel_with_chatbot_coverage_count;
            } else {
                addMissing(report.missing, "release_panel_chatbot_coverage", panel.id,
                           "Release top-level WYSIWYG panel lacks actionable or readonly chatbot coverage.");
            }
        } else {
            ++report.non_release_panel_count;
            if (searchable && panelToolsAvailable) {
                ++report.non_release_panel_discoverable_count;
            } else {
                addMissing(report.missing, "non_release_panel_discoverability", panel.id,
                           "Non-release editor panel is not discoverable through readonly chatbot panel tools.");
            }
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

    for (const auto& tool : knowledge.tools.tools()) {
        if (!tool.mutates_project) {
            continue;
        }
        ++report.mutating_tool_count;
        if (tool.requires_approval) {
            ++report.mutating_tool_requires_approval_count;
        } else {
            ++report.unsafe_mutating_tool_count;
            addMissing(report.missing, "mutating_tool_approval", tool.id,
                       "Mutating chatbot tool does not require approval.");
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
                    report.editor_panel_count == report.searchable_editor_panel_count &&
                    report.release_panel_count == report.release_panel_with_chatbot_coverage_count &&
                    report.non_release_panel_count == report.non_release_panel_discoverable_count &&
                    report.capability_count == report.capability_with_tool_count &&
                    report.capability_count == report.capability_with_wysiwyg_surface_count &&
                    report.unsafe_mutating_tool_count == 0 &&
                    report.asset_panel_registered &&
                    report.asset_chatbot_tool_registered;
    return report;
}

} // namespace urpg::ai
