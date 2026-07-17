#pragma once

#include "editor/accessibility/semantic_editor_command_surface.h"
#include "engine/core/ui/menu_authoring_document.h"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace urpg::editor {

struct MenuAuthoringWorkspaceResult {
    bool success = false;
    std::string code;
    std::string message;

    MenuAuthoringWorkspaceResult() = default;
    MenuAuthoringWorkspaceResult(bool value, std::string result_code, std::string result_message)
        : success(value), code(std::move(result_code)), message(std::move(result_message)) {}
};

class MenuAuthoringWorkspace {
public:
    MenuAuthoringWorkspace();

    MenuAuthoringWorkspaceResult bindProjectRoot(std::filesystem::path project_root);
    MenuAuthoringWorkspaceResult save();
    MenuAuthoringWorkspaceResult resetFromTemplate(std::string_view template_id);

    ui::MenuAuthoringDocument& document() { return document_; }
    const ui::MenuAuthoringDocument& document() const { return document_; }
    const std::filesystem::path& projectPath() const { return project_path_; }
    const std::string& templateId() const { return template_id_; }
    bool dirty() const { return dirty_; }

    MenuAuthoringWorkspaceResult select(std::vector<std::string> ids);
    MenuAuthoringWorkspaceResult moveSelection(int dx, int dy, bool snap);
    MenuAuthoringWorkspaceResult resizeNode(std::string_view id, int width, int height, bool snap);
    MenuAuthoringWorkspaceResult alignSelection(ui::MenuCanvasAlignment alignment);
    MenuAuthoringWorkspaceResult distributeSelection(ui::MenuCanvasDistribution distribution);
    MenuAuthoringWorkspaceResult undo();
    MenuAuthoringWorkspaceResult redo();
    SemanticEditorCommandResult navigateSemantic(SemanticEditorNavigation navigation);
    SemanticEditorCommandResult setSemanticProperty(std::string_view key, std::string_view value);
    SemanticEditorCommandResult connectSemanticSelectionTo(std::string_view target_id);
    SemanticEditorCommandResult focusSemanticDiagnostic(std::size_t diagnostic_index);

    void setBindingContext(ui::MenuBindingContext context);
    void setAuditOptions(ui::MenuAuthoringAuditOptions options);
    const ui::MenuRuntimeMaterialization& runtimeMaterialization() const { return runtime_; }
    const ui::MenuAuthoringAuditResult& audit() const { return audit_; }
    nlohmann::json snapshot();

private:
    MenuAuthoringWorkspaceResult load();
    void refreshDerived();
    void bindSemanticSurface();
    MenuAuthoringWorkspaceResult mutationResult(bool changed, std::string code, std::string message,
                                                std::vector<std::string> diagnostics = {});

    std::filesystem::path project_root_;
    std::filesystem::path project_path_;
    ui::MenuAuthoringDocument document_;
    ui::MenuBindingContext binding_context_;
    ui::MenuAuthoringAuditOptions audit_options_;
    ui::MenuRuntimeMaterialization runtime_;
    ui::MenuAuthoringAuditResult audit_;
    SemanticEditorCommandSurface semantic_surface_;
    std::string template_id_ = "title";
    bool dirty_ = false;
};

} // namespace urpg::editor
