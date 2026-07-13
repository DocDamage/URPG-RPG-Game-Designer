#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace urpg::editor {

struct EditorProjectIdentity {
    std::filesystem::path root;
    std::string project_id;
    std::string display_name;
    std::string schema_version;
};

struct EditorProjectSessionResult {
    bool success = false;
    std::string code;
    std::string message;
};

class EditorProjectSession {
  public:
    using SwitchListener = std::function<void(const EditorProjectIdentity&)>;

    EditorProjectSessionResult openProject(const std::filesystem::path& project_root);
    EditorProjectSessionResult closeProject();
    void addSwitchListener(SwitchListener listener);
    void setDirtySurfaceSummaries(std::vector<std::string> summaries);

    bool isOpen() const { return open_; }
    const EditorProjectIdentity& activeProject() const { return active_project_; }
    const EditorProjectSessionResult& lastDiagnostic() const { return last_diagnostic_; }
    const std::vector<std::string>& dirtySurfaceSummaries() const { return dirty_surface_summaries_; }

  private:
    EditorProjectSessionResult validateProject(const std::filesystem::path& project_root,
                                               EditorProjectIdentity& identity) const;

    bool open_ = false;
    EditorProjectIdentity active_project_;
    EditorProjectSessionResult last_diagnostic_;
    std::vector<std::string> dirty_surface_summaries_;
    std::vector<SwitchListener> switch_listeners_;
};

} // namespace urpg::editor
