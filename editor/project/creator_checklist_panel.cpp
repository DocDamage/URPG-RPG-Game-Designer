#include "editor/project/creator_checklist_panel.h"

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor {

void CreatorChecklistPanel::setProjectRoot(std::filesystem::path projectRoot) {
    project_root_ = std::move(projectRoot);
    refresh();
}

void CreatorChecklistPanel::refresh() {
    if (project_root_.empty()) {
        snapshot_ = {};
        status_message_ = "Choose or create a project to show the creator checklist.";
        return;
    }
    snapshot_ = checklist_.inspect(project_root_);
    status_message_ = snapshot_.dismissed ? "Checklist dismissed. Restore it from Help when needed."
                                        : "Complete steps from durable project state.";
}

bool CreatorChecklistPanel::dismiss(std::string* error) {
    if (project_root_.empty()) {
        if (error) *error = "No project is open.";
        return false;
    }
    const bool saved = checklist_.setDismissed(project_root_, true, error);
    if (saved) refresh();
    return saved;
}

bool CreatorChecklistPanel::restore(std::string* error) {
    if (project_root_.empty()) {
        if (error) *error = "No project is open.";
        return false;
    }
    const bool saved = checklist_.setDismissed(project_root_, false, error);
    if (saved) refresh();
    return saved;
}

void CreatorChecklistPanel::render() {
    if (!visible_) return;
    refresh();
#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() != nullptr) {
        if (ImGui::Begin("Creator Checklist")) {
            ImGui::TextWrapped("%s", status_message_.c_str());
            if (!project_root_.empty() && !snapshot_.dismissed) {
                ImGui::Separator();
                for (const auto& item : snapshot_.items) {
                    ImGui::BulletText("%s %s", item.complete ? "[done]" : "[ ]", item.label.c_str());
                }
                if (ImGui::Button("Dismiss Checklist")) {
                    std::string ignored;
                    (void)dismiss(&ignored);
                }
            } else if (!project_root_.empty() && ImGui::Button("Restore Checklist")) {
                std::string ignored;
                (void)restore(&ignored);
            }
        }
        ImGui::End();
    }
#endif
}

} // namespace urpg::editor
