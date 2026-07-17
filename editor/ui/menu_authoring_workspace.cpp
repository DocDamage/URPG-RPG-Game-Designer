#include "editor/ui/menu_authoring_workspace.h"

#include <fstream>

namespace urpg::editor {
namespace {

bool atomicWriteJson(const std::filesystem::path& path, const nlohmann::json& value, std::string& error) {
    std::error_code filesystem_error;
    std::filesystem::create_directories(path.parent_path(), filesystem_error);
    if (filesystem_error) {
        error = "Unable to create the menu authoring directory: " + filesystem_error.message();
        return false;
    }
    auto temporary = path;
    temporary += ".tmp";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) {
            error = "Unable to open the temporary menu authoring document.";
            return false;
        }
        output << value.dump(2) << '\n';
        output.flush();
        if (!output) {
            error = "Unable to flush the temporary menu authoring document.";
            return false;
        }
    }
    auto backup = path;
    backup += ".bak";
    const bool replacing = std::filesystem::exists(path);
    if (replacing) {
        std::filesystem::remove(backup, filesystem_error);
        filesystem_error.clear();
        std::filesystem::rename(path, backup, filesystem_error);
        if (filesystem_error) {
            std::filesystem::remove(temporary);
            error = "Unable to stage the prior menu authoring document: " + filesystem_error.message();
            return false;
        }
    }
    std::filesystem::rename(temporary, path, filesystem_error);
    if (filesystem_error) {
        error = "Unable to publish the menu authoring document: " + filesystem_error.message();
        std::filesystem::remove(temporary);
        if (replacing) {
            std::error_code restore_error;
            std::filesystem::rename(backup, path, restore_error);
            if (restore_error) error += " Prior document restoration also failed: " + restore_error.message();
        }
        return false;
    }
    if (replacing) std::filesystem::remove(backup, filesystem_error);
    return true;
}

} // namespace

MenuAuthoringWorkspace::MenuAuthoringWorkspace() {
    const auto templates = ui::MenuStarterTemplateLibrary::originalUrpgTemplates();
    if (const auto* starter = templates.find(template_id_)) document_ = starter->document;
    bindSemanticSurface();
    refreshDerived();
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::bindProjectRoot(std::filesystem::path project_root) {
    project_root_ = std::move(project_root);
    project_path_ = project_root_ / "content" / "ui" / "menu_authoring.json";
    return load();
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::load() {
    if (project_root_.empty()) return {false, "menu_project_missing", "Open a project before loading a menu."};
    if (!std::filesystem::exists(project_path_)) {
        const auto result = resetFromTemplate("title");
        dirty_ = false;
        return {result.success, "menu_default_loaded", "Loaded the original URPG title starter."};
    }
    std::ifstream input(project_path_, std::ios::binary);
    const auto json = nlohmann::json::parse(input, nullptr, false);
    if (!json.is_object() || json.value("schema", "") != "urpg.menu_authoring_workspace.v1" ||
        !json.contains("document")) {
        return {false, "menu_document_invalid", "The menu authoring document is malformed."};
    }
    std::vector<std::string> diagnostics;
    auto restored = ui::MenuAuthoringDocument::fromJson(json.at("document"), &diagnostics);
    if (!restored) {
        return {false, "menu_document_rejected", diagnostics.empty() ? "The menu authoring document is invalid."
                                                                      : diagnostics.front()};
    }
    document_ = std::move(*restored);
    template_id_ = json.value("template_id", "custom");
    dirty_ = false;
    bindSemanticSurface();
    refreshDerived();
    return {true, "menu_document_loaded", "Loaded the project menu authoring document."};
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::save() {
    if (project_root_.empty()) return {false, "menu_project_missing", "Open a project before saving a menu."};
    refreshDerived();
    if (!audit_.package_safe) {
        return {false, "menu_package_audit_failed", "Resolve blocking menu audit findings before saving."};
    }
    std::string error;
    const nlohmann::json payload{{"schema", "urpg.menu_authoring_workspace.v1"},
                                 {"template_id", template_id_},
                                 {"document", document_.toJson()}};
    if (!atomicWriteJson(project_path_, payload, error)) return {false, "menu_save_failed", std::move(error)};
    dirty_ = false;
    return {true, "menu_saved", "Saved the authoritative menu document and refreshed its runtime projection."};
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::resetFromTemplate(const std::string_view template_id) {
    const auto templates = ui::MenuStarterTemplateLibrary::originalUrpgTemplates();
    const auto* starter = templates.find(template_id);
    if (starter == nullptr) return {false, "menu_template_missing", "The requested starter template does not exist."};
    document_ = starter->document;
    template_id_ = starter->id;
    dirty_ = true;
    bindSemanticSurface();
    refreshDerived();
    return {true, "menu_template_applied", "Applied an original editable URPG starter template."};
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::mutationResult(const bool changed, std::string code,
                                                                    std::string message,
                                                                    std::vector<std::string> diagnostics) {
    if (!changed) {
        if (!diagnostics.empty()) message += " " + diagnostics.front();
        return {false, std::move(code), std::move(message)};
    }
    dirty_ = true;
    bindSemanticSurface();
    refreshDerived();
    return {true, std::move(code), std::move(message)};
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::select(std::vector<std::string> ids) {
    return mutationResult(document_.select(std::move(ids)), "menu_selection_rejected", "Menu selection changed.");
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::moveSelection(const int dx, const int dy, const bool snap) {
    auto result = document_.moveSelection(dx, dy, snap);
    return mutationResult(result.changed, "menu_move_rejected", "Moved the menu selection.", result.diagnostics);
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::resizeNode(const std::string_view id, const int width,
                                                                const int height, const bool snap) {
    auto result = document_.resizeNode(id, width, height, snap);
    return mutationResult(result.changed, "menu_resize_rejected", "Resized the menu node.", result.diagnostics);
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::alignSelection(const ui::MenuCanvasAlignment alignment) {
    auto result = document_.alignSelection(alignment);
    return mutationResult(result.changed, "menu_align_rejected", "Aligned the menu selection.", result.diagnostics);
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::distributeSelection(const ui::MenuCanvasDistribution distribution) {
    auto result = document_.distributeSelection(distribution);
    return mutationResult(result.changed, "menu_distribute_rejected", "Distributed the menu selection.", result.diagnostics);
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::undo() {
    return mutationResult(document_.undo(), "menu_undo_unavailable", "Undid the menu edit.");
}

MenuAuthoringWorkspaceResult MenuAuthoringWorkspace::redo() {
    return mutationResult(document_.redo(), "menu_redo_unavailable", "Redid the menu edit.");
}

SemanticEditorCommandResult MenuAuthoringWorkspace::navigateSemantic(const SemanticEditorNavigation navigation) {
    return semantic_surface_.navigate(navigation);
}

SemanticEditorCommandResult MenuAuthoringWorkspace::setSemanticProperty(const std::string_view key,
                                                                        const std::string_view value) {
    auto result = semantic_surface_.setSelectedProperty(key, value);
    if (result.applied) {
        dirty_ = true;
        refreshDerived();
    }
    return result;
}

SemanticEditorCommandResult MenuAuthoringWorkspace::connectSemanticSelectionTo(const std::string_view target_id) {
    auto result = semantic_surface_.connectSelectedTo(target_id);
    if (result.applied) {
        dirty_ = true;
        refreshDerived();
    }
    return result;
}

SemanticEditorCommandResult MenuAuthoringWorkspace::focusSemanticDiagnostic(const std::size_t diagnostic_index) {
    return semantic_surface_.focusDiagnostic(diagnostic_index);
}

void MenuAuthoringWorkspace::setBindingContext(ui::MenuBindingContext context) {
    binding_context_ = std::move(context);
    refreshDerived();
}

void MenuAuthoringWorkspace::setAuditOptions(ui::MenuAuthoringAuditOptions options) {
    audit_options_ = std::move(options);
    refreshDerived();
}

void MenuAuthoringWorkspace::bindSemanticSurface() {
    semantic_surface_ = semanticCommandSurfaceForMenu(document_);
}

void MenuAuthoringWorkspace::refreshDerived() {
    runtime_ = ui::materializeMenuAuthoringDocument(document_, "menu_authoring_preview", &binding_context_);
    audit_ = ui::auditMenuAuthoringDocument(document_, audit_options_);
    semantic_surface_.refresh();
}

nlohmann::json MenuAuthoringWorkspace::snapshot() {
    refreshDerived();
    nlohmann::json issues = nlohmann::json::array();
    for (const auto& issue : audit_.issues) {
        issues.push_back({{"code", issue.code}, {"node_id", issue.node_id}, {"message", issue.message},
                          {"blocking", issue.blocking}});
    }
    return {{"project_path", project_path_.generic_string()},
            {"template_id", template_id_},
            {"dirty", dirty_},
            {"document", document_.toJson()},
            {"runtime_scene_ready", runtime_.scene != nullptr},
            {"runtime_diagnostics", runtime_.diagnostics},
            {"package_safe", audit_.package_safe},
            {"audit_issues", std::move(issues)},
            {"semantic_alternative", semantic_surface_.renderSnapshot()}};
}

} // namespace urpg::editor
