#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::editor {

struct CreatorChecklistItem {
    std::string id;
    std::string label;
    bool complete = false;
    std::string route;
    std::string action_label;
    std::string coaching;
};

struct CreatorChecklistSnapshot {
    bool dismissed = false;
    bool completed = false;
    bool sample_project = false;
    bool replay_available = true;
    int replay_count = 0;
    std::string next_item_id;
    std::vector<CreatorChecklistItem> items;
};

class CreatorChecklist {
  public:
    CreatorChecklistSnapshot inspect(const std::filesystem::path& project_root) const;
    bool setDismissed(const std::filesystem::path& project_root, bool dismissed, std::string* error = nullptr) const;
    bool setCompleted(const std::filesystem::path& project_root, bool completed, std::string* error = nullptr) const;
    bool replay(const std::filesystem::path& project_root, std::string* error = nullptr) const;

  private:
    static std::filesystem::path statePath(const std::filesystem::path& project_root);
};

} // namespace urpg::editor
