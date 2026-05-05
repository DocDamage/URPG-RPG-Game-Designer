#include "apps/editor/editor_app_panels.h"
#include "editor/ability/ability_inspector_panel.h"
#include "editor/ability/pattern_field_panel.h"
#include "editor/analytics/analytics_panel.h"
#include "editor/assets/asset_library_panel.h"
#include "editor/diagnostics/diagnostics_workspace.h"
#include "editor/mod/mod_manager_panel.h"
#include "editor/spatial/level_builder_workspace.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/analytics/analytics_dispatcher.h"
#include "engine/core/analytics/analytics_privacy_controller.h"
#include "engine/core/analytics/analytics_uploader.h"
#include "engine/core/app_cli.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/diagnostics/startup_diagnostics.h"
#include "engine/core/editor/editor_panel_registry.h"
#include "engine/core/editor/editor_shell.h"
#include "engine/core/engine_context.h"
#include "engine/core/engine_shell.h"
#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/mod/mod_loader.h"
#include "engine/core/mod/mod_registry.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/presentation/presentation_schema.h"
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

#include <algorithm>
#include <chrono>
#include <cctype>
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
    urpg::editor::LevelBuilderWorkspace level_builder_workspace;
    urpg::ability::AbilitySystemComponent ability_runtime;
    urpg::map::GridPartDocument level_builder_document{"EditorPreview", 16, 12};
    urpg::map::GridPartCatalog level_builder_catalog;
    urpg::presentation::SpatialMapOverlay level_builder_overlay;
    urpg::mod::ModRegistry mod_registry;
    std::unique_ptr<urpg::mod::ModLoader> mod_loader;
    urpg::analytics::AnalyticsDispatcher analytics_dispatcher;
    urpg::analytics::AnalyticsUploader analytics_uploader;
    urpg::analytics::AnalyticsPrivacyController analytics_privacy_controller;
    std::filesystem::path project_root;
};

std::string abilityAssetFileName(const urpg::ability::AuthoredAbilityAsset& asset) {
    std::string stem;
    stem.reserve(asset.ability_id.size());
    for (const unsigned char ch : asset.ability_id) {
        if (std::isalnum(ch) || ch == '_' || ch == '-') {
            stem.push_back(static_cast<char>(ch));
        } else if (ch == '.' || ch == '/' || ch == '\\' || ch == ' ') {
            stem.push_back('_');
        }
    }

    if (stem.empty()) {
        stem = "draft";
    }
    return stem + ".json";
}

urpg::map::GridPartCategory gridPartCategoryFromString(const std::string& value) {
    using Category = urpg::map::GridPartCategory;
    static const std::unordered_map<std::string, Category> categories = {
        {"Tile", Category::Tile},
        {"Wall", Category::Wall},
        {"Platform", Category::Platform},
        {"Hazard", Category::Hazard},
        {"Door", Category::Door},
        {"Npc", Category::Npc},
        {"Enemy", Category::Enemy},
        {"TreasureChest", Category::TreasureChest},
        {"SavePoint", Category::SavePoint},
        {"Trigger", Category::Trigger},
        {"CutsceneZone", Category::CutsceneZone},
        {"Shop", Category::Shop},
        {"QuestItem", Category::QuestItem},
        {"Prop", Category::Prop},
        {"LevelBlock", Category::LevelBlock},
    };

    const auto found = categories.find(value);
    return found == categories.end() ? Category::Prop : found->second;
}

urpg::map::GridPartLayer gridPartLayerFromString(const std::string& value) {
    using Layer = urpg::map::GridPartLayer;
    static const std::unordered_map<std::string, Layer> layers = {
        {"Terrain", Layer::Terrain},     {"Decoration", Layer::Decoration}, {"Collision", Layer::Collision},
        {"Object", Layer::Object},       {"Actor", Layer::Actor},           {"Trigger", Layer::Trigger},
        {"Region", Layer::Region},       {"Overlay", Layer::Overlay},
    };

    const auto found = layers.find(value);
    return found == layers.end() ? Layer::Object : found->second;
}

urpg::map::GridPartCollisionPolicy gridPartCollisionPolicyFromString(const std::string& value) {
    using Policy = urpg::map::GridPartCollisionPolicy;
    static const std::unordered_map<std::string, Policy> policies = {
        {"None", Policy::None},
        {"Solid", Policy::Solid},
        {"Hazard", Policy::Hazard},
        {"TriggerOnly", Policy::TriggerOnly},
        {"Custom", Policy::Custom},
    };

    const auto found = policies.find(value);
    return found == policies.end() ? Policy::None : found->second;
}

urpg::map::GridPartRuleset gridPartRulesetFromString(const std::string& value) {
    using Ruleset = urpg::map::GridPartRuleset;
    static const std::unordered_map<std::string, Ruleset> rulesets = {
        {"TopDownJRPG", Ruleset::TopDownJRPG},
        {"SideScrollerAction", Ruleset::SideScrollerAction},
        {"TacticalGrid", Ruleset::TacticalGrid},
        {"DungeonRoomBuilder", Ruleset::DungeonRoomBuilder},
        {"WorldMap", Ruleset::WorldMap},
        {"TownHub", Ruleset::TownHub},
        {"BattleArena", Ruleset::BattleArena},
        {"CutsceneStage", Ruleset::CutsceneStage},
    };

    const auto found = rulesets.find(value);
    return found == rulesets.end() ? Ruleset::TopDownJRPG : found->second;
}

bool loadGridPartCatalog(const std::filesystem::path& projectRoot, urpg::map::GridPartCatalog& catalog) {
    const auto catalogPath = projectRoot / "content" / "part_catalogs" / "base_jrpg_parts.json";
    std::ifstream stream(catalogPath, std::ios::binary);
    if (!stream) {
        return false;
    }

    nlohmann::json payload;
    try {
        payload = nlohmann::json::parse(stream);
    } catch (const nlohmann::json::exception&) {
        return false;
    }

    if (!payload.contains("parts") || !payload["parts"].is_array()) {
        return false;
    }

    urpg::map::GridPartCatalog loaded;
    for (const auto& part : payload["parts"]) {
        if (!part.is_object() || !part.contains("partId") || !part["partId"].is_string()) {
            return false;
        }

        urpg::map::GridPartDefinition definition;
        definition.part_id = part["partId"].get<std::string>();
        definition.display_name = part.value("displayName", definition.part_id);
        definition.description = part.value("description", "");
        definition.category = gridPartCategoryFromString(part.value("category", "Prop"));
        definition.default_layer = gridPartLayerFromString(part.value("defaultLayer", "Object"));
        definition.collision_policy = gridPartCollisionPolicyFromString(part.value("collisionPolicy", "None"));
        definition.asset_id = part.value("assetId", "");
        definition.prefab_path = part.value("prefabPath", "");
        definition.tile_id = part.value("tileId", 0);

        const auto footprint = part.value("footprint", nlohmann::json::object());
        definition.footprint.width = footprint.value("width", 1);
        definition.footprint.height = footprint.value("height", 1);
        definition.footprint.allow_overlap = footprint.value("allowOverlap", false);
        definition.footprint.blocks_navigation = footprint.value("blocksNavigation", false);

        for (const auto& ruleset : part.value("supportedRulesets", nlohmann::json::array())) {
            if (ruleset.is_string()) {
                definition.supported_rulesets.push_back(gridPartRulesetFromString(ruleset.get<std::string>()));
            }
        }
        if (definition.supported_rulesets.empty()) {
            definition.supported_rulesets.push_back(urpg::map::GridPartRuleset::TopDownJRPG);
        }

        for (const auto& tag : part.value("tags", nlohmann::json::array())) {
            if (tag.is_string()) {
                definition.tags.push_back(tag.get<std::string>());
            }
        }

        const auto properties = part.value("defaultProperties", nlohmann::json::object());
        for (const auto& [key, value] : properties.items()) {
            if (value.is_string()) {
                definition.default_properties[key] = value.get<std::string>();
            }
        }

        if (!loaded.addDefinition(std::move(definition))) {
            return false;
        }
    }

    catalog = std::move(loaded);
    return catalog.size() > 0;
}

void bindLevelBuilder(EditorPanelRuntime& runtime) {
    runtime.level_builder_overlay.mapId = runtime.level_builder_document.mapId();
    runtime.level_builder_overlay.elevation.width = static_cast<uint32_t>(runtime.level_builder_document.width());
    runtime.level_builder_overlay.elevation.height = static_cast<uint32_t>(runtime.level_builder_document.height());
    runtime.level_builder_overlay.elevation.levels.assign(
        static_cast<size_t>(runtime.level_builder_overlay.elevation.width) *
            static_cast<size_t>(runtime.level_builder_overlay.elevation.height),
        0);

    const bool catalogLoaded = loadGridPartCatalog(runtime.project_root, runtime.level_builder_catalog);
    runtime.level_builder_workspace.SetTargets(&runtime.level_builder_document,
                                               catalogLoaded ? &runtime.level_builder_catalog : nullptr,
                                               &runtime.level_builder_overlay);
}

bool registerEditorPanels(urpg::editor::EditorShell& editor_shell, EditorPanelRuntime& runtime) {
    runtime.ability_inspector_panel.update(runtime.ability_runtime);
    runtime.ability_inspector_panel.setCommandCallbacks({
        [&runtime] {
            const bool ok = runtime.ability_inspector_panel.previewSelectedAbility(runtime.ability_runtime);
            runtime.ability_inspector_panel.recordCommandResult(
                "preview_selected", ok,
                ok ? "Previewed selected ability against the editor runtime."
                   : "Select a runtime ability before previewing.");
            return ok;
        },
        [&runtime] {
            runtime.ability_inspector_panel.applyDraftToRuntime(runtime.ability_runtime);
            runtime.ability_inspector_panel.update(runtime.ability_runtime);
            const bool selected = runtime.ability_inspector_panel.selectDraftAbility(runtime.ability_runtime);
            const auto asset = runtime.ability_inspector_panel.getDraftAsset();
            runtime.ability_inspector_panel.recordCommandResult(
                "apply_draft", selected,
                selected ? "Applied draft ability '" + asset.ability_id + "' to the editor runtime."
                         : "Draft ability was applied, but the runtime selection could not be refreshed.");
            return selected;
        },
        [&runtime] {
            if (runtime.project_root.empty()) {
                runtime.ability_inspector_panel.recordCommandResult("save_draft", false,
                                                                    "Project root is not configured.");
                return false;
            }

            const auto asset = runtime.ability_inspector_panel.getDraftAsset();
            const auto target_path =
                urpg::ability::canonicalAbilityContentDirectory(runtime.project_root) / abilityAssetFileName(asset);
            const bool ok = urpg::ability::saveAuthoredAbilityAssetToFile(asset, target_path);
            runtime.ability_inspector_panel.recordCommandResult(
                "save_draft", ok,
                ok ? "Saved draft ability to " +
                         std::filesystem::relative(target_path, runtime.project_root).generic_string()
                   : "Failed to save draft ability to " + target_path.generic_string());
            return ok;
        },
        [&runtime] {
            if (runtime.project_root.empty()) {
                runtime.ability_inspector_panel.recordCommandResult("load_draft", false,
                                                                    "Project root is not configured.");
                return false;
            }

            const auto records = urpg::ability::discoverAuthoredAbilityAssets(runtime.project_root);
            if (records.empty()) {
                runtime.ability_inspector_panel.recordCommandResult(
                    "load_draft", false, "No authored ability assets found under content/abilities.");
                return false;
            }

            const auto current_asset = runtime.ability_inspector_panel.getDraftAsset();
            auto selected = records.begin();
            const auto matching = std::find_if(records.begin(), records.end(), [&current_asset](const auto& record) {
                return record.ability_id == current_asset.ability_id;
            });
            if (matching != records.end()) {
                selected = matching;
            }

            const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(selected->absolute_path);
            if (!asset.has_value()) {
                runtime.ability_inspector_panel.recordCommandResult(
                    "load_draft", false, "Failed to load " + selected->relative_path + ".");
                return false;
            }

            runtime.ability_inspector_panel.setDraftFromAsset(*asset);
            runtime.ability_inspector_panel.update(runtime.ability_runtime);
            runtime.ability_inspector_panel.recordCommandResult(
                "load_draft", true, "Loaded draft ability from " + selected->relative_path + ".");
            return true;
        },
    });
    runtime.pattern_field_panel.bindModel(runtime.pattern_field_model);
    runtime.mod_loader = std::make_unique<urpg::mod::ModLoader>(runtime.mod_registry);
    runtime.mod_manager_panel.bindRegistry(&runtime.mod_registry);
    runtime.mod_manager_panel.bindLoader(runtime.mod_loader.get());
    runtime.analytics_uploader.setLocalJsonlExportPath(runtime.project_root / "reports" / "analytics" /
                                                       "editor_analytics.jsonl");
    runtime.analytics_panel.bindDispatcher(&runtime.analytics_dispatcher);
    runtime.analytics_panel.bindUploader(&runtime.analytics_uploader);
    runtime.analytics_panel.bindPrivacyController(&runtime.analytics_privacy_controller);
    runtime.diagnostics_workspace.bindAbilityRuntime(runtime.ability_runtime);
    bindLevelBuilder(runtime);

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
        {"level_builder",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext& context) {
                 panelRuntime.level_builder_workspace.Render(
                     urpg::FrameContext{static_cast<float>(context.delta_seconds),
                                        static_cast<uint32_t>(context.frame_index)});
             };
         }},
    };

    for (const auto& factoryId : urpg::editor_app::editorAppRegisteredPanelFactoryIds()) {
        if (renderFactories.find(factoryId) == renderFactories.end()) {
            return false;
        }
    }

    if (!urpg::editor_app::editorAppMissingReleasePanelFactoryIds().empty()) {
        return false;
    }

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

const char* runtimeDiagnosticSeverity(urpg::diagnostics::DiagnosticSeverity severity) {
    switch (severity) {
    case urpg::diagnostics::DiagnosticSeverity::Info:
        return "info";
    case urpg::diagnostics::DiagnosticSeverity::Warning:
        return "warning";
    case urpg::diagnostics::DiagnosticSeverity::Error:
        return "error";
    case urpg::diagnostics::DiagnosticSeverity::Fatal:
        return "fatal";
    }
    return "unknown";
}

void printRuntimeDiagnostics() {
    const auto diagnostics = urpg::diagnostics::RuntimeDiagnostics::snapshot();
    for (const auto& diagnostic : diagnostics) {
        std::cerr << "URPG editor runtime diagnostic " << runtimeDiagnosticSeverity(diagnostic.severity)
                  << " [" << diagnostic.subsystem << "/" << diagnostic.code << "]: "
                  << diagnostic.message << "\n";
    }
}

#ifndef URPG_HEADLESS
int runPlatformProbe(const urpg::WindowConfig& config, bool probeOpenGl, bool probeRender) {
    const auto result = urpg::SDLSurface::probe(config, probeOpenGl);
    std::cout << "URPG editor platform probe\n";
    std::cout << "SDL video: " << (result.videoInitialized ? "ok" : "failed") << "\n";
    if (!result.videoError.empty()) {
        std::cout << "SDL video error: " << result.videoError << "\n";
    }
    std::cout << "SDL game controller: " << (result.controllerInitialized ? "ok" : "unavailable") << "\n";
    if (!result.controllerError.empty()) {
        std::cout << "SDL game controller warning: " << result.controllerError << "\n";
    }
    if (probeOpenGl) {
        std::cout << "Hidden OpenGL window: " << (result.windowCreated ? "ok" : "failed") << "\n";
        if (!result.windowError.empty()) {
            std::cout << "Hidden OpenGL window error: " << result.windowError << "\n";
        }
        std::cout << "OpenGL context: " << (result.glContextCreated ? "ok" : "failed") << "\n";
        if (!result.glContextError.empty()) {
            std::cout << "OpenGL context error: " << result.glContextError << "\n";
        }
    } else {
        std::cout << "OpenGL context: not probed; pass --probe-opengl to create a hidden GL context.\n";
    }
    if (!result.videoInitialized || (probeOpenGl && (!result.windowCreated || !result.glContextCreated))) {
        return 1;
    }

    if (probeRender) {
        urpg::WindowConfig hiddenConfig = config;
        hiddenConfig.hidden = true;
        hiddenConfig.resizable = false;
        hiddenConfig.title = "URPG Editor Render Probe";

        urpg::SDLSurface surface;
        if (!surface.initialize(hiddenConfig)) {
            std::cout << "Renderer hidden surface: failed\n";
            printRuntimeDiagnostics();
            return 1;
        }

        urpg::OpenGLRenderer renderer;
        const bool rendererInitialized = renderer.initialize(&surface);
        std::cout << "OpenGL renderer: " << (rendererInitialized ? "ok" : "failed") << "\n";
        if (!rendererInitialized) {
            printRuntimeDiagnostics();
            renderer.shutdown();
            surface.shutdown();
            return 1;
        }
        renderer.beginFrame();
        renderer.endFrame();
        renderer.shutdown();
        surface.shutdown();
        std::cout << "Hidden render frame: ok\n";
    }

    return 0;
}
#endif

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

        if (options.probe_platform) {
#ifdef URPG_HEADLESS
            std::cerr << "URPG editor was built headless; platform probing is unavailable.\n";
            return 2;
#else
            return runPlatformProbe(config, options.probe_opengl, options.probe_render);
#endif
        }

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
            printRuntimeDiagnostics();
            std::cerr << "Try 'urpg_editor --safe-mode' to start with the non-OpenGL diagnostic backend.\n";
            return 1;
        }

        auto& engineShell = urpg::EngineShell::getInstance();
        if (!engineShell.startup(std::move(surface), std::move(renderer),
                                 urpg::EngineShell::StartupOptions(options.project_root))) {
            std::cerr << "URPG editor startup failed.\n";
            return 1;
        }

        clearSceneStack();
        auto editorPreview = std::make_shared<urpg::scene::MapScene>("EditorPreview", 16, 12);
        editorPreview->setAssetReferences(
            urpg::scene::loadRuntimeMapAssetReferences(options.project_root, "EditorPreview"));
        urpg::scene::SceneManager::getInstance().gotoScene(editorPreview);

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
        panelRuntime.project_root = options.project_root;
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
