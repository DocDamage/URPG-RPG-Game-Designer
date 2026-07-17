#include "editor/ui/menu_authoring_workspace.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {

std::filesystem::path temporaryProject() {
    const auto token = std::chrono::steady_clock::now().time_since_epoch().count();
    auto path = std::filesystem::temp_directory_path() / ("urpg-menu-authoring-" + std::to_string(token));
    std::filesystem::create_directories(path);
    return path;
}

} // namespace

TEST_CASE("Menu authoring workspace persists one authoritative document and derives runtime output",
          "[menu][authoring_workspace][pcq480][pcq483]") {
    const auto project = temporaryProject();
    urpg::editor::MenuAuthoringWorkspace workspace;
    REQUIRE(workspace.bindProjectRoot(project).success);
    REQUIRE(workspace.templateId() == "title");
    REQUIRE(workspace.runtimeMaterialization().scene != nullptr);
    REQUIRE(workspace.audit().package_safe);
    REQUIRE_FALSE(workspace.document().nodes().empty());

    const auto node_id = workspace.document().nodes().front().id;
    REQUIRE(workspace.select({node_id}).success);
    REQUIRE(workspace.setSemanticProperty("accessible_label", "Main title region").applied);
    REQUIRE(workspace.dirty());
    const auto expected = workspace.document().toJson();
    REQUIRE(workspace.save().success);
    REQUIRE_FALSE(workspace.dirty());

    urpg::editor::MenuAuthoringWorkspace reopened;
    REQUIRE(reopened.bindProjectRoot(project).success);
    REQUIRE(reopened.document().toJson() == expected);
    REQUIRE(reopened.runtimeMaterialization().scene != nullptr);
    REQUIRE(reopened.snapshot().at("semantic_alternative").at("rows").size() == reopened.document().nodes().size());
    std::filesystem::remove_all(project);
}

TEST_CASE("Menu authoring workspace materializes and audits every original starter at target viewports",
          "[menu][authoring_workspace][templates][pcq481][pcq482][pcq484][pcq485]") {
    urpg::editor::MenuAuthoringWorkspace workspace;
    const auto starters = urpg::ui::MenuStarterTemplateLibrary::originalUrpgTemplates();
    const auto targets = urpg::ui::menuTargetResolutionPresets();
    REQUIRE(starters.templates().size() == 11);
    REQUIRE(targets.size() >= 5);
    for (const auto& starter : starters.templates()) {
        REQUIRE(workspace.resetFromTemplate(starter.id).success);
        for (const auto target : targets) {
            urpg::ui::MenuAuthoringAuditOptions options;
            options.target_canvas = target;
            options.input = urpg::ui::MenuInputPreview::Controller;
            workspace.setAuditOptions(options);
            INFO(starter.id << " at " << target.width << "x" << target.height);
            REQUIRE(workspace.runtimeMaterialization().scene != nullptr);
            REQUIRE(workspace.audit().package_safe);
        }
    }
}
