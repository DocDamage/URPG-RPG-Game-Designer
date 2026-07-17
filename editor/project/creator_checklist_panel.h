#pragma once

#include "editor/project/creator_checklist.h"

#include <filesystem>
#include <functional>
#include <string>

namespace urpg::editor {

class CreatorChecklistPanel {
  public:
    using RouteHandler = std::function<bool(const CreatorChecklistItem&)>;

    void setProjectRoot(std::filesystem::path projectRoot);
    void setVisible(bool visible) { visible_ = visible; }
    bool dismiss(std::string* error = nullptr);
    bool restore(std::string* error = nullptr);
    bool complete(std::string* error = nullptr);
    bool replay(std::string* error = nullptr);
    void setRouteHandler(RouteHandler handler) { route_handler_ = std::move(handler); }
    bool activateNextAction();
    void render();

    const CreatorChecklistSnapshot& snapshot() const { return snapshot_; }
    const std::string& statusMessage() const { return status_message_; }
    bool isVisible() const { return visible_; }

  private:
    void refresh();

    CreatorChecklist checklist_;
    std::filesystem::path project_root_;
    CreatorChecklistSnapshot snapshot_;
    std::string status_message_ = "Choose or create a project to show the creator checklist.";
    bool visible_ = true;
    RouteHandler route_handler_;
};

} // namespace urpg::editor
