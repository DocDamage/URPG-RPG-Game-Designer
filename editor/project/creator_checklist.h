#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::editor {

struct CreatorChecklistItem {
    std::string id;
    std::string label;
    bool complete = false;
};

struct CreatorChecklistSnapshot {
    bool dismissed = false;
    std::vector<CreatorChecklistItem> items;
};

class CreatorChecklist {
  public:
    CreatorChecklistSnapshot inspect(const std::filesystem::path& project_root) const;
    bool setDismissed(const std::filesystem::path& project_root, bool dismissed, std::string* error = nullptr) const;

  private:
    static std::filesystem::path statePath(const std::filesystem::path& project_root);
};

} // namespace urpg::editor
