#include "editor/ability/ability_inspector_panel.h"
#include "editor/ability/pattern_field_panel.h"
#include "editor/analytics/analytics_panel.h"
#include "editor/assets/asset_library_panel.h"
#include "editor/diagnostics/diagnostics_workspace.h"
#include "editor/mod/mod_manager_panel.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/analytics/analytics_dispatcher.h"
#include "engine/core/analytics/analytics_privacy_controller.h"
#include "engine/core/analytics/analytics_uploader.h"
#include "engine/core/app_cli.h"
#include "engine/core/editor/editor_shell.h"
#include "engine/core/engine_shell.h"
#include "engine/core/mod/mod_loader.h"
#include "engine/core/mod/mod_registry.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/project/project_snapshot_store.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/scene_manager.h"
#include "engine/core/version.h"
#include <nlohmann/json.hpp>

#ifndef URPG_HEADLESS
#include "engine/core/platform/opengl_renderer.h"
#include "engine/core/platform/sdl_surface.h"
#endif

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace {

bool defaultHeadless() {
#ifdef URPG_HEADLESS
    return true;
#else
    return false;
#endif
}

void printVersion() {
    std::cout << "URPG Editor " << urpg::versionString() << "\n";
}

void clearSceneStack() {
    auto& sceneManager = urpg::scene::SceneManager::getInstance();
    while (sceneManager.stackSize() > 0) {
        sceneManager.popScene();
    }
}

struct EditorPanelRuntime {
    urpg::editor::DiagnosticsWorkspace diagnostics_workspace;
    urpg::editor::AssetLibraryPanel asset_library_panel;
    urpg::editor::AbilityInspectorPanel ability_inspector_panel;
    urpg::editor::PatternFieldModel pattern_field_model;
    urpg::editor::PatternFieldPanel pattern_field_panel;
    urpg::editor::ModManagerPanel mod_manager_panel;
    urpg::editor::AnalyticsPanel analytics_panel;
    urpg::ability::AbilitySystemComponent ability_runtime;
    urpg::mod::ModRegistry mod_registry;
    std::unique_ptr<urpg::mod::ModLoader> mod_loader;
    urpg::analytics::AnalyticsDispatcher analytics_dispatcher;
    urpg::analytics::AnalyticsUploader analytics_uploader;
    urpg::analytics::AnalyticsPrivacyController analytics_privacy_controller;
};

bool registerEditorPanels(urpg::editor::EditorShell& editor_shell, EditorPanelRuntime& runtime) {
    runtime.ability_inspector_panel.update(runtime.ability_runtime);
    runtime.pattern_field_panel.bindModel(runtime.pattern_field_model);
    runtime.mod_loader = std::make_unique<urpg::mod::ModLoader>(runtime.mod_registry);
    runtime.mod_manager_panel.bindRegistry(&runtime.mod_registry);
    runtime.mod_manager_panel.bindLoader(runtime.mod_loader.get());
    runtime.analytics_panel.bindDispatcher(&runtime.analytics_dispatcher);
    runtime.analytics_panel.bindUploader(&runtime.analytics_uploader);
    runtime.analytics_panel.bindPrivacyController(&runtime.analytics_privacy_controller);
    runtime.diagnostics_workspace.bindAbilityRuntime(runtime.ability_runtime);

    using urpg::editor::EditorPanelDescriptor;

    bool ok = true;
    ok = editor_shell.addPanel(EditorPanelDescriptor{"diagnostics", "Diagnostics", "System"},
                               [&runtime](const urpg::editor::EditorFrameContext&) {
                                   runtime.diagnostics_workspace.update();
                                   runtime.diagnostics_workspace.render();
                               }) &&
         ok;
    ok = editor_shell.addPanel(
             EditorPanelDescriptor{"assets", "Assets", "Content"},
             [&runtime](const urpg::editor::EditorFrameContext&) { runtime.asset_library_panel.render(); }) &&
         ok;
    ok = editor_shell.addPanel(EditorPanelDescriptor{"ability", "Ability Inspector", "Gameplay"},
                               [&runtime](const urpg::editor::EditorFrameContext&) {
                                   runtime.ability_inspector_panel.update(runtime.ability_runtime);
                                   runtime.ability_inspector_panel.render();
                               }) &&
         ok;
    ok = editor_shell.addPanel(
             EditorPanelDescriptor{"patterns", "Pattern Field Editor", "Gameplay"},
             [&runtime](const urpg::editor::EditorFrameContext&) { runtime.pattern_field_panel.render(); }) &&
         ok;
    ok = editor_shell.addPanel(
             EditorPanelDescriptor{"mod", "Mod Manager", "Runtime"},
             [&runtime](const urpg::editor::EditorFrameContext&) { runtime.mod_manager_panel.render(); }) &&
         ok;
    ok = editor_shell.addPanel(
             EditorPanelDescriptor{"analytics", "Analytics", "Runtime"},
             [&runtime](const urpg::editor::EditorFrameContext&) { runtime.analytics_panel.render(); }) &&
         ok;

    return ok;
}

void printPanelList(const urpg::editor::EditorShell& editor_shell) {
    for (const auto& panel : editor_shell.panels()) {
        std::cout << panel.id << "\t" << panel.category << "\t" << panel.title << "\t"
                  << (panel.visible ? "visible" : "hidden") << "\n";
    }
}

bool runEditorFrame(urpg::EngineShell& engineShell, urpg::editor::EditorShell& editorShell, bool renderAllPanels,
                    double deltaSeconds = 1.0 / 60.0) {
    engineShell.tick();
#ifdef URPG_IMGUI_ENABLED
    ImGui::NewFrame();
#endif
    bool rendered = false;
    if (editorShell.beginFrame(deltaSeconds)) {
        if (renderAllPanels) {
            rendered = editorShell.renderVisiblePanels() > 0;
        } else {
            rendered = editorShell.renderActivePanel();
        }
        rendered = editorShell.endFrame() && rendered;
    }
#ifdef URPG_IMGUI_ENABLED
    ImGui::Render();
#endif
    return rendered;
}

nlohmann::json panelSnapshotJson(const urpg::editor::EditorShell& editorShell) {
    nlohmann::json panels = nlohmann::json::array();
    for (const auto& panel : editorShell.panels()) {
        panels.push_back({
            {"id", panel.id},
            {"title", panel.title},
            {"category", panel.category},
            {"visible", panel.visible},
            {"enabled", panel.enabled},
            {"rendered_last_frame", panel.rendered_last_frame},
            {"render_count", panel.render_count},
        });
    }
    return panels;
}

int runSmokeWorkflow(urpg::EngineShell& engineShell, urpg::editor::EditorShell& editorShell,
                     const urpg::cli::EditorCliOptions& options) {
    static constexpr std::string_view requiredPanels[] = {
        "diagnostics", "assets", "ability", "mod", "analytics",
    };

    nlohmann::json report = {
        {"schema", "urpg.editor_smoke.v1"},
        {"project_root", options.project_root.generic_string()},
        {"project_root_exists", std::filesystem::is_directory(options.project_root)},
        {"opened_panels", nlohmann::json::array()},
        {"rendered_panels", nlohmann::json::array()},
        {"errors", nlohmann::json::array()},
    };

    if (!std::filesystem::is_directory(options.project_root)) {
        report["errors"].push_back("project_root_missing");
    }

    for (const auto panelId : requiredPanels) {
        if (!editorShell.hasPanel(panelId)) {
            report["errors"].push_back(std::string("missing_panel:") + std::string(panelId));
            continue;
        }
        if (!editorShell.openPanel(panelId)) {
            report["errors"].push_back(std::string("open_panel_failed:") + std::string(panelId));
            continue;
        }
        report["opened_panels"].push_back(panelId);
        if (!runEditorFrame(engineShell, editorShell, false)) {
            report["errors"].push_back(std::string("render_panel_failed:") + std::string(panelId));
            continue;
        }
        report["rendered_panels"].push_back(panelId);
    }

    const urpg::project::ProjectSnapshotStore snapshotStore;
    std::filesystem::remove_all(options.smoke_snapshot_root / "editor_smoke_project_state");
    const auto snapshot =
        snapshotStore.createSnapshot(options.project_root, options.smoke_snapshot_root, "editor_smoke_project_state");
    report["project_snapshot"] = {
        {"success", snapshot.success},
        {"path", snapshot.snapshot_path.generic_string()},
        {"manifest", snapshot.manifest},
        {"errors", snapshot.errors},
    };
    if (!snapshot.success) {
        report["errors"].push_back("project_snapshot_failed");
    }

    const auto shellSnapshot = editorShell.snapshot();
    report["frame_index"] = shellSnapshot.frame_index;
    report["active_panel_id"] = shellSnapshot.active_panel_id;
    report["runtime_preview_id"] = shellSnapshot.runtime_preview_id;
    report["panels"] = panelSnapshotJson(editorShell);

    std::filesystem::create_directories(options.smoke_output.parent_path());
    std::ofstream out(options.smoke_output, std::ios::binary);
    if (!out) {
        std::cerr << "URPG editor smoke failed to open output '" << options.smoke_output.string() << "'.\n";
        return 1;
    }
    out << report.dump(2) << "\n";

    if (!report["errors"].empty()) {
        std::cerr << "URPG editor smoke failed: " << report["errors"].dump() << "\n";
        return 1;
    }

    std::cout << "URPG editor smoke wrote " << options.smoke_output.string() << "\n";
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto cli = urpg::cli::parseEditorCli(urpg::cli::argvToViews(argc, argv), defaultHeadless());
        if (!cli.ok()) {
            std::cerr << "URPG editor: " << cli.error << "\n" << urpg::cli::editorHelpText();
            return 2;
        }
        if (cli.action == urpg::cli::CliAction::Help) {
            std::cout << urpg::cli::editorHelpText();
            return 0;
        }
        if (cli.action == urpg::cli::CliAction::Version) {
            printVersion();
            return 0;
        }

        const urpg::cli::EditorCliOptions options = cli.options;

        urpg::WindowConfig config;
        config.title = "URPG Editor";
        config.width = options.width;
        config.height = options.height;

        std::unique_ptr<urpg::IPlatformSurface> surface;
        std::unique_ptr<urpg::RendererBackend> renderer;

        if (options.headless) {
            surface = std::make_unique<urpg::HeadlessSurface>();
            renderer = std::make_unique<urpg::HeadlessRenderer>();
        } else {
#ifdef URPG_HEADLESS
            std::cerr << "URPG editor was built headless; use --headless.\n";
            return 2;
#else
            surface = std::make_unique<urpg::SDLSurface>();
            renderer = std::make_unique<urpg::OpenGLRenderer>();
#endif
        }

        if (!surface->initialize(config)) {
            std::cerr << "URPG editor failed to initialize platform surface.\n";
            return 1;
        }

        auto& engineShell = urpg::EngineShell::getInstance();
        if (!engineShell.startup(std::move(surface), std::move(renderer),
                                 urpg::EngineShell::StartupOptions(options.project_root))) {
            std::cerr << "URPG editor startup failed.\n";
            return 1;
        }

        clearSceneStack();
        urpg::scene::SceneManager::getInstance().gotoScene(
            std::make_shared<urpg::scene::MapScene>("EditorPreview", 16, 12));

        urpg::editor::EditorShell editorShell;
        editorShell.setProjectRoot(options.project_root);
        editorShell.setRuntimePreviewId("EditorPreview");
        if (!editorShell.start(options.headless)) {
            std::cerr << "URPG editor shell startup failed.\n";
            return 1;
        }

#ifdef URPG_IMGUI_ENABLED
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(options.width), static_cast<float>(options.height));
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().LogFilename = nullptr;
        unsigned char* fontPixels = nullptr;
        int fontWidth = 0;
        int fontHeight = 0;
        ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&fontPixels, &fontWidth, &fontHeight);
#endif

        EditorPanelRuntime panelRuntime;
        if (!registerEditorPanels(editorShell, panelRuntime)) {
            std::cerr << "URPG editor failed to register required panels.\n";
#ifdef URPG_IMGUI_ENABLED
            ImGui::DestroyContext();
#endif
            editorShell.shutdown();
            engineShell.shutdown();
            clearSceneStack();
            return 1;
        }

        if (options.open_panel_id.has_value() && !editorShell.openPanel(*options.open_panel_id)) {
            std::cerr << "URPG editor has no reachable panel with id '" << *options.open_panel_id << "'.\n";
#ifdef URPG_IMGUI_ENABLED
            ImGui::DestroyContext();
#endif
            editorShell.shutdown();
            engineShell.shutdown();
            clearSceneStack();
            return 2;
        }

        if (options.list_panels) {
            printPanelList(editorShell);
        }

        if (options.smoke) {
            const int smokeResult = runSmokeWorkflow(engineShell, editorShell, options);
            editorShell.shutdown();
#ifdef URPG_IMGUI_ENABLED
            ImGui::DestroyContext();
#endif
            engineShell.shutdown();
            clearSceneStack();
            return smokeResult;
        }

        int frame = 0;
        while (engineShell.isRunning() && editorShell.isRunning() && (options.frames < 0 || frame < options.frames)) {
            (void)runEditorFrame(engineShell, editorShell, options.render_all_panels);
            ++frame;
            if (options.headless) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        editorShell.shutdown();
#ifdef URPG_IMGUI_ENABLED
        ImGui::DestroyContext();
#endif
        engineShell.shutdown();
        clearSceneStack();

        std::cout << "URPG editor exited after " << frame << " frame(s).\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "URPG editor exception: " << ex.what() << "\n";
        return 2;
    }
}
