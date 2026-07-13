#pragma once

#include "editor/spatial/map_authoring_context.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::editor {

// A context route is the handoff contract between the unified Map workspace
// and a nested editor. It deliberately carries no panel pointers so routes
// remain serializable and safe to inspect in tests and diagnostics.
struct EditorContextAction {
    std::string route;
    std::string objectKind;
    std::string objectId;
    std::filesystem::path projectRoot;
    MapAuthoringSelection selection;
    std::string returnRoute = "map";
};

struct EditorContextActionResult {
    bool success = false;
    std::string code;
    std::string message;
    EditorContextAction action;
};

class EditorContextActionStack {
  public:
    EditorContextActionResult open(EditorContextAction action);
    EditorContextActionResult returnToPrevious();
    const EditorContextAction* active() const;
    size_t depth() const { return stack_.size(); }

  private:
    std::vector<EditorContextAction> stack_;
};

} // namespace urpg::editor
