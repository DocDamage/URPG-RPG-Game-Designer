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
#include "engine/core/diagnostics/startup_diagnostics.h"
#include "engine/core/editor/editor_panel_registry.h"
#include "engine/core/editor/editor_shell.h"
#include "engine/core/engine_shell.h"
#include "engine/core/mod/mod_loader.h"
#include "engine/core/mod/mod_registry.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/project/project_snapshot_store.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/scene_manager.h"
#include "engine/core/settings/app_settings_store.h"
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
#include <functional>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
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

    using PanelRenderFactory = std::function<urpg::editor::EditorShell::RenderCallback(EditorPanelRuntime&)>;
    const std::unordered_map<std::string, PanelRenderFactory> renderFactories = {
        {"diagnostics",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.diagnostics_workspace.update();
                 panelRuntime.diagnostics_workspace.render();
             };
         }},
        {"assets",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.asset_library_panel.render();
             };
         }},
        {"ability",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.ability_inspector_panel.update(panelRuntime.ability_runtime);
                 panelRuntime.ability_inspector_panel.render();
             };
         }},
        {"patterns",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.pattern_field_panel.render();
             };
         }},
        {"mod",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.mod_manager_panel.render();
             };
         }},
        {"analytics",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.analytics_panel.render();
             };
         }},
    };

    bool ok = true;
    for (const auto& panel : urpg::editor::topLevelEditorPanels()) {
        const auto factory = renderFactories.find(panel.id);
        if (factory == renderFactories.end()) {
            ok = false;
            continue;
        }

        ok = editor_shell.addPanel(
                 urpg::editor::EditorPanelDescriptor{panel.id, panel.title, panel.category}, factory->second(runtime)) &&
             ok;
    }

    return ok;
}

void printPanelList(const urpg::editor::EditorShell& editor_shell) {
    for (const auto& panel : editor_shell.panels()) {
        std::cout << panel.id << "\t" << panel.category << "\t" << panel.title << "\t"
                  << (panel.visible ? "visible" : "hidden") << "\n";
    }
}

void printStartupFailure(const urpg::diagnostics::StartupDiagnosticRecord& record,
                         const urpg::diagnostics::StartupDiagnosticWriteResult& writeResult) {
    std::cerr << "URPG editor startup " << urpg::diagnostics::toString(record.severity) << " [" << record.code
              << "]: " << record.message << "\n";
    if (!writeResult.log_path.empty()) {
        std::cerr << "URPG editor startup diagnostics log: " << writeResult.log_path.string() << "\n";
    }
    if (!writeResult.written && !writeResult.error.empty()) {
        std::cerr << "URPG editor startup diagnostics write failed: " << writeResult.error << "\n";
    }
}

urpg::analytics::ConsentState analyticsConsentFromSettings(const std::string& state) {
    if (state == "granted") {
        return urpg::analytics::ConsentState::Granted;
    }
    if (state == "denied") {
        return urpg::analytics::ConsentState::Denied;
    }
    return urpg::analytics::ConsentState::Unknown;
}

std::string analyticsConsentToSettings(urpg::analytics::ConsentState state) {
    switch (state) {
    case urpg::analytics::ConsentState::Granted:
        return "granted";
    case urpg::analytics::ConsentState::Denied:
        return "denied";
    case urpg::analytics::ConsentState::Unknown:
        return "unknown";
    }
    return "unknown";
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
    const auto requiredPanels = urpg::editor::smokeRequiredEditorPanelIds();

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

    for (const auto& panelId : requiredPanels) {
        if (!editorShell.hasPanel(panelId)) {
            report["errors"].push_back(std::string("missing_panel:") + panelId);
            continue;
        }
        if (!editorShell.openPanel(panelId)) {
            report["errors"].push_back(std::string("open_panel_failed:") + panelId);
            continue;
        }
        report["opened_panels"].push_back(panelId);
        if (!runEditorFrame(engineShell, editorShell, false)) {
            report["errors"].push_back(std::string("render_panel_failed:") + panelId);
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
        const auto settingsPaths = urpg::settings::appSettingsPaths(options.project_root);
        auto settingsLoad = urpg::settings::loadEditorSettings(settingsPaths.editor_settings, settingsPaths);
        for (const auto& warning : settingsLoad.report.warnings) {
            std::cerr << "URPG editor settings warning: " << warning << "\n";
        }
        if (options.width_provided) {
            settingsLoad.settings.window.width = options.width;
        }
        if (options.height_provided) {
            settingsLoad.settings.window.height = options.height;
        }

        if (const auto startupFailure = urpg::diagnostics::validateStartupInputs(
                "editor", options.project_root, settingsLoad.settings.window.width, settingsLoad.settings.window.height,
                options.headless)) {
            const auto writeResult = urpg::diagnostics::writeStartupDiagnostic(*startupFailure);
            printStartupFailure(*startupFailure, writeResult);
            return 1;
        }

        urpg::WindowConfig config;
        config.title = "URPG Editor";
        config.width = settingsLoad.settings.window.width;
        config.height = settingsLoad.settings.window.height;
        config.fullscreen = settingsLoad.settings.window.fullscreen;
        config.resizable = settingsLoad.settings.window.resizable;

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
        ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(config.width), static_cast<float>(config.height));
        std::filesystem::create_directories(settingsLoad.settings.imgui_ini_path.parent_path());
        const std::string imguiIniFilename = settingsLoad.settings.imgui_ini_path.string();
        ImGui::GetIO().IniFilename = imguiIniFilename.c_str();
        ImGui::GetIO().LogFilename = nullptr;
        unsigned char* fontPixels = nullptr;
        int fontWidth = 0;
        int fontHeight = 0;
        ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&fontPixels, &fontWidth, &fontHeight);
#endif

        EditorPanelRuntime panelRuntime;
        const auto analyticsConsent = analyticsConsentFromSettings(settingsLoad.settings.analytics_consent_state);
        panelRuntime.analytics_privacy_controller.recordConsentDecision(analyticsConsent);
        panelRuntime.analytics_dispatcher.setOptIn(analyticsConsent == urpg::analytics::ConsentState::Granted &&
                                                   settingsLoad.settings.analytics_upload_enabled);
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

        settingsLoad.settings.window.width = config.width;
        settingsLoad.settings.window.height = config.height;
        settingsLoad.settings.window.fullscreen = config.fullscreen;
        settingsLoad.settings.window.resizable = config.resizable;
        settingsLoad.settings.analytics_consent_state =
            analyticsConsentToSettings(panelRuntime.analytics_privacy_controller.getConsentState());
        settingsLoad.settings.analytics_upload_enabled = panelRuntime.analytics_dispatcher.isOptIn();
        std::string settingsError;
        if (!urpg::settings::saveEditorSettings(settingsPaths.editor_settings, settingsLoad.settings, &settingsError)) {
            std::cerr << "URPG editor failed to save settings: " << settingsError << "\n";
        }

        std::cout << "URPG editor exited after " << frame << " frame(s).\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "URPG editor exception: " << ex.what() << "\n";
        return 2;
    }
}
