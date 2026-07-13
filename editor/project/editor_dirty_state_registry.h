#pragma once

#include <functional>
#include <string>
#include <vector>

namespace urpg::editor {

enum class EditorNavigationDecision { Save, Discard, Cancel };

struct EditorDirtySaveResult {
    bool success = false;
    std::string code;
    std::string message;
};

struct EditorDirtySurface {
    std::string document_id;
    std::string focus_route;
    bool dirty = false;
    std::function<EditorDirtySaveResult()> save;
    std::function<void()> focus;
    EditorDirtySaveResult last_save_result;
};

struct EditorNavigationGuardResult {
    bool allowed = false;
    std::vector<std::string> dirty_document_ids;
    std::string failed_document_id;
    EditorDirtySaveResult diagnostic;
};

class EditorDirtyStateRegistry {
  public:
    bool registerSurface(EditorDirtySurface surface);
    bool markDirty(const std::string& document_id, bool dirty = true);
    bool isDirty(const std::string& document_id) const;
    std::vector<std::string> dirtyDocumentIds() const;
    EditorDirtySaveResult save(const std::string& document_id);
    EditorNavigationGuardResult resolveNavigation(EditorNavigationDecision decision);

  private:
    EditorDirtySurface* find(const std::string& document_id);
    const EditorDirtySurface* find(const std::string& document_id) const;
    std::vector<EditorDirtySurface> surfaces_;
};

} // namespace urpg::editor
