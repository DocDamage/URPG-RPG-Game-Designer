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
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#endif

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
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
    urpg::editor::SpatialAuthoringWorkspace perspective_2d_workspace;
    urpg::ability::AbilitySystemComponent ability_runtime;
    urpg::map::GridPartDocument level_builder_document{"EditorPreview", 16, 12};
    urpg::map::GridPartCatalog level_builder_catalog;
    urpg::presentation::SpatialMapOverlay level_builder_overlay;
    urpg::scene::MapScene perspective_2d_scene{"EditorPreview", 16, 12};
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

urpg::editor::PropPlacementPanel::ScreenProjectionSettings
makeEditorPreviewProjection(const urpg::map::GridPartDocument& document) {
    urpg::editor::PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 1280.0f;
    projection.viewportHeight = 720.0f;
    projection.cameraCenterX = static_cast<float>(document.width()) * 0.5f;
    projection.cameraCenterZ = static_cast<float>(document.height()) * 0.5f;
    projection.worldUnitsPerPixel = 1.0f / 48.0f;
    return projection;
}

std::vector<urpg::map::MapRegionRule> makeEditorPreviewRegionRules() {
    return {
        {"editor_rain_path", 2, 1, 5, 4, "", "rain_loop", "rain", "", "normal", ""},
    };
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
                                               &runtime.level_builder_overlay,
                                               &runtime.perspective_2d_scene);
    runtime.perspective_2d_workspace.SetTargets(&runtime.perspective_2d_scene, &runtime.level_builder_overlay);
    runtime.perspective_2d_workspace.SetGridPartTargets(
        &runtime.level_builder_document, catalogLoaded ? &runtime.level_builder_catalog : nullptr);
    const auto projection = makeEditorPreviewProjection(runtime.level_builder_document);
    runtime.level_builder_workspace.SetProjectionSettings(projection);
    runtime.perspective_2d_workspace.SetProjectionSettings(projection);
    const auto previewRules = makeEditorPreviewRegionRules();
    runtime.perspective_2d_workspace.LoadRegionRules(previewRules);
    runtime.perspective_2d_workspace.LoadEnvironmentPreview(
        urpg::map::MapEnvironmentPreviewDocument::fromRegionRules(
            runtime.level_builder_document.mapId(),
            runtime.level_builder_document.width(),
            runtime.level_builder_document.height(),
            previewRules));
    runtime.perspective_2d_workspace.SelectEnvironmentTile(3, 2);
    urpg::map::TerrainBrush previewBrush;
    previewBrush.mode = urpg::map::TerrainBrushMode::Rectangle;
    previewBrush.width = 2;
    previewBrush.height = 2;
    previewBrush.tile_id = 1;
    runtime.perspective_2d_workspace.PreviewTerrainBrush(previewBrush, 3, 2, 1);
    runtime.perspective_2d_workspace.GenerateProceduralMap(
        {"editor_preview_seed",
         "dungeon",
         runtime.level_builder_document.width(),
         runtime.level_builder_document.height(),
         1,
         false,
         false,
         false});
    if (!runtime.project_root.empty()) {
        (void)runtime.perspective_2d_workspace.SetProjectRoot(runtime.project_root.string());
    }
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
        {"spatial_authoring",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext& context) {
                 panelRuntime.perspective_2d_workspace.Render(
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

void printRuntimeDiagnostics() {
    for (const auto& diagnostic : urpg::diagnostics::RuntimeDiagnostics::snapshot()) {
        std::cerr << "URPG editor runtime diagnostic [" << diagnostic.code << "]: " << diagnostic.message << "\n";
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

#ifdef URPG_IMGUI_ENABLED
void renderEditorChrome(urpg::editor::EditorShell& editorShell) {
    ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(340.0f, 520.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("URPG Editor")) {
        ImGui::End();
        return;
    }

    const auto snapshot = editorShell.snapshot();
    ImGui::Text("Project");
    ImGui::TextWrapped("%s", snapshot.project_root.string().c_str());
    ImGui::Separator();
    ImGui::Text("Active Panel");
    ImGui::TextWrapped("%s", snapshot.active_panel_id.c_str());
    ImGui::Separator();
    ImGui::Text("Release Panels");

    for (const auto& panel : snapshot.panels) {
        ImGui::BeginDisabled(!panel.enabled);
        const bool selected = panel.id == snapshot.active_panel_id;
        if (ImGui::Selectable((panel.title + "##" + panel.id).c_str(), selected)) {
            editorShell.openPanel(panel.id);
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s/%s", panel.category.c_str(), panel.id.c_str());
        }
    }

    ImGui::Separator();
    ImGui::Text("Preview");
    ImGui::TextWrapped("The map preview is rendered behind this editor shell.");
    ImGui::End();
}

const char* diagnosticsTabName(urpg::editor::DiagnosticsTab tab) {
    switch (tab) {
    case urpg::editor::DiagnosticsTab::Compat:
        return "Compat";
    case urpg::editor::DiagnosticsTab::Save:
        return "Save";
    case urpg::editor::DiagnosticsTab::EventAuthority:
        return "Events";
    case urpg::editor::DiagnosticsTab::MessageText:
        return "Messages";
    case urpg::editor::DiagnosticsTab::Battle:
        return "Battle";
    case urpg::editor::DiagnosticsTab::Menu:
        return "Menu";
    case urpg::editor::DiagnosticsTab::Audio:
        return "Audio";
    case urpg::editor::DiagnosticsTab::MigrationWizard:
        return "Migration";
    case urpg::editor::DiagnosticsTab::Abilities:
        return "Abilities";
    case urpg::editor::DiagnosticsTab::ProjectAudit:
        return "Audit";
    case urpg::editor::DiagnosticsTab::ProjectHealth:
        return "Health";
    }
    return "Diagnostics";
}

void renderDisabledReason(const std::string& reason) {
    if (!reason.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("%s", reason.c_str());
    }
}

void renderJsonLines(const nlohmann::json& value, int maxRows = 8) {
    if (value.is_null()) {
        ImGui::TextDisabled("None");
        return;
    }
    if (value.is_array()) {
        int row = 0;
        for (const auto& item : value) {
            if (row++ >= maxRows) {
                ImGui::TextDisabled("...");
                break;
            }
            ImGui::BulletText("%s", item.is_string() ? item.get<std::string>().c_str() : item.dump().c_str());
        }
        if (row == 0) {
            ImGui::TextDisabled("None");
        }
        return;
    }
    if (value.is_object()) {
        int row = 0;
        for (auto it = value.begin(); it != value.end(); ++it) {
            if (row++ >= maxRows) {
                ImGui::TextDisabled("...");
                break;
            }
            const auto rendered = it.value().is_string() ? it.value().get<std::string>() : it.value().dump();
            ImGui::BulletText("%s: %s", it.key().c_str(), rendered.c_str());
        }
        if (row == 0) {
            ImGui::TextDisabled("None");
        }
        return;
    }
    ImGui::TextWrapped("%s", value.dump().c_str());
}

void renderCompatDiagnostics(urpg::editor::CompatReportPanel& panel) {
    auto& model = panel.getModel();
    ImGui::Text("Project Compatibility Score: %d%%", model.getProjectCompatibilityScore());
    ImGui::Separator();

    std::string selected = panel.getSelectedPlugin();
    if (!selected.empty()) {
        if (ImGui::Button("< Back to Summary")) {
            panel.clearSelection();
        }
        ImGui::SameLine();
        ImGui::Text("Selected Plugin: %s", selected.c_str());
        ImGui::Separator();

        ImGui::Text("API Calls:");
        auto calls = model.getPluginCalls(selected);
        if (calls.empty()) {
            ImGui::TextDisabled("No calls logged.");
        } else {
            if (ImGui::BeginTable("CompatCallsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Class");
                ImGui::TableSetupColumn("Method");
                ImGui::TableSetupColumn("Status");
                ImGui::TableSetupColumn("Calls");
                ImGui::TableHeadersRow();
                for (const auto& call : calls) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(call.className.c_str());
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(call.methodName.c_str());
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(call.status == urpg::compat::CompatStatus::FULL ? "FULL" : (call.status == urpg::compat::CompatStatus::PARTIAL ? "PARTIAL" : (call.status == urpg::compat::CompatStatus::STUB ? "STUB" : "UNSUPPORTED")));
                    ImGui::TableNextColumn(); ImGui::Text("%u", call.callCount);
                }
                ImGui::EndTable();
            }
        }
    } else {
        ImGui::Text("Plugins:");
        auto summaries = model.getAllPluginSummaries();
        if (summaries.empty()) {
            ImGui::TextDisabled("No plugin summaries found.");
        } else {
            if (ImGui::BeginTable("CompatPluginsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Plugin ID");
                ImGui::TableSetupColumn("Score");
                ImGui::TableSetupColumn("Calls");
                ImGui::TableSetupColumn("Action");
                ImGui::TableHeadersRow();
                for (const auto& summary : summaries) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(summary.pluginId.c_str());
                    ImGui::TableNextColumn(); ImGui::Text("%d%%", summary.compatibilityScore);
                    ImGui::TableNextColumn(); ImGui::Text("%u", summary.totalCalls);
                    ImGui::TableNextColumn();
                    ImGui::PushID(summary.pluginId.c_str());
                    if (ImGui::Button("Inspect")) {
                        panel.selectPlugin(summary.pluginId);
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
    }
}

void renderSaveDiagnostics(urpg::editor::SaveInspectorPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Status: %s", snapshot.status.c_str());
    if (!snapshot.message.empty()) {
        ImGui::TextWrapped("%s", snapshot.message.c_str());
    }
    if (!snapshot.remediation.empty()) {
        ImGui::TextWrapped("%s", snapshot.remediation.c_str());
    }

    if (!snapshot.runtime_bound) {
        return;
    }

    auto& model = panel.getModel();

    bool problemOnly = panel.showProblemSlotsOnly();
    if (ImGui::Checkbox("Show Problem Slots Only", &problemOnly)) {
        panel.setShowProblemSlotsOnly(problemOnly);
        panel.refresh();
    }
    ImGui::SameLine();
    bool includeAutosave = panel.includeAutosave();
    if (ImGui::Checkbox("Include Autosave", &includeAutosave)) {
        panel.setIncludeAutosave(includeAutosave);
        panel.refresh();
    }

    ImGui::Separator();
    ImGui::Text("Save Slots:");
    auto rows = model.VisibleRows();
    if (rows.empty()) {
        ImGui::TextDisabled("No visible save slots.");
    } else {
        if (ImGui::BeginTable("SaveSlotsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Slot ID");
            ImGui::TableSetupColumn("Category");
            ImGui::TableSetupColumn("Label");
            ImGui::TableSetupColumn("Map");
            ImGui::TableSetupColumn("Diagnostics");
            ImGui::TableHeadersRow();
            for (const auto& row : rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%d", row.slot_id);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.category_label.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.slot_label.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.map_display_name.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.diagnostic.empty() ? "-" : row.diagnostic.c_str());
            }
            ImGui::EndTable();
        }
    }

    ImGui::Separator();
    ImGui::Text("Save Policy Draft:");
    auto draft = model.PolicyDraft();
    bool autosaveEnabled = draft.autosave_enabled;
    if (ImGui::Checkbox("Autosave Enabled", &autosaveEnabled)) {
        panel.setPolicyAutosaveEnabled(autosaveEnabled);
    }
    int autosaveSlot = draft.autosave_slot_id;
    if (ImGui::InputInt("Autosave Slot ID", &autosaveSlot)) {
        panel.setPolicyAutosaveSlotId(autosaveSlot);
    }

    int maxAutosave = static_cast<int>(draft.max_autosave_slots);
    int maxQuicksave = static_cast<int>(draft.max_quicksave_slots);
    int maxManual = static_cast<int>(draft.max_manual_slots);
    bool prune = draft.prune_excess_on_save;

    bool limitChanged = false;
    if (ImGui::SliderInt("Max Autosave Slots", &maxAutosave, 1, 10)) limitChanged = true;
    if (ImGui::SliderInt("Max Quicksave Slots", &maxQuicksave, 1, 10)) limitChanged = true;
    if (ImGui::SliderInt("Max Manual Slots", &maxManual, 1, 100)) limitChanged = true;
    if (ImGui::Checkbox("Prune Excess on Save", &prune)) limitChanged = true;

    if (limitChanged) {
        panel.setPolicyRetentionLimits(maxAutosave, maxQuicksave, maxManual, prune);
    }

    if (snapshot.can_apply_policy) {
        if (ImGui::Button("Apply Policy")) {
            panel.applyPolicyToRuntime();
        }
    }
}

void renderEventAuthorityDiagnostics(urpg::EventAuthorityPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Visible Logs: %zu (Warnings: %zu, Errors: %zu)", snapshot.visible_rows, snapshot.warning_count, snapshot.error_count);

    char eventFilter[64];
    strncpy(eventFilter, snapshot.event_id_filter.c_str(), sizeof(eventFilter));
    eventFilter[sizeof(eventFilter)-1] = '\0';
    if (ImGui::InputText("Event ID Filter", eventFilter, sizeof(eventFilter))) {
        panel.setFilter(eventFilter);
        panel.refresh();
    }

    char levelFilter[64];
    strncpy(levelFilter, snapshot.level_filter.c_str(), sizeof(levelFilter));
    levelFilter[sizeof(levelFilter)-1] = '\0';
    if (ImGui::InputText("Level Filter", levelFilter, sizeof(levelFilter))) {
        panel.setLevelFilter(levelFilter);
        panel.refresh();
    }

    char modeFilter[64];
    strncpy(modeFilter, snapshot.mode_filter.c_str(), sizeof(modeFilter));
    modeFilter[sizeof(modeFilter)-1] = '\0';
    if (ImGui::InputText("Mode Filter", modeFilter, sizeof(modeFilter))) {
        panel.setModeFilter(modeFilter);
        panel.refresh();
    }

    if (ImGui::Button("Clear Filters")) {
        panel.clearFilters();
        panel.refresh();
    }

    ImGui::Separator();
    ImGui::Text("Event Blocks:");

    if (snapshot.visible_row_entries.empty()) {
        ImGui::TextDisabled("No event authority rows found matching criteria.");
    } else {
        if (ImGui::BeginTable("EventAuthorityTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Time");
            ImGui::TableSetupColumn("Level");
            ImGui::TableSetupColumn("Event ID");
            ImGui::TableSetupColumn("Block ID");
            ImGui::TableSetupColumn("Message");
            ImGui::TableHeadersRow();
            for (const auto& row : snapshot.visible_row_entries) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.ts.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.level.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.event_id.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.block_id.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.message.c_str());
            }
            ImGui::EndTable();
        }
    }
}

void renderMessageDiagnostics(urpg::editor::MessageInspectorPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Total Pages: %zu (Issues: %zu)", snapshot.total_pages, snapshot.issue_count);

    bool showIssues = snapshot.show_issues_only;
    if (ImGui::Checkbox("Show Issues Only", &showIssues)) {
        panel.setShowIssuesOnly(showIssues);
        panel.refresh();
    }

    ImGui::Separator();
    ImGui::Text("Dialogue Pages:");
    if (snapshot.visible_rows.empty()) {
        ImGui::TextDisabled("No dialogue pages visible.");
    } else {
        if (ImGui::BeginTable("MessagePagesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Index");
            ImGui::TableSetupColumn("Speaker");
            ImGui::TableSetupColumn("Route");
            ImGui::TableSetupColumn("Body Preview");
            ImGui::TableSetupColumn("Issues");
            ImGui::TableHeadersRow();
            for (const auto& row : snapshot.visible_rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%zu", row.page_index);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.speaker.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.route.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.body_preview.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%zu", row.issue_count);
            }
            ImGui::EndTable();
        }
    }
}

void renderBattleDiagnostics(urpg::editor::BattleInspectorPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Status: %s (Phase: %s, Issues: %zu)", snapshot.status.c_str(), snapshot.phase.c_str(), snapshot.issue_count);
    if (!snapshot.message.empty()) {
        ImGui::TextWrapped("%s", snapshot.message.c_str());
    }
    if (!snapshot.remediation.empty()) {
        ImGui::TextWrapped("%s", snapshot.remediation.c_str());
    }

    if (!snapshot.runtime_bound) {
        return;
    }

    auto& model = panel.getModel();
    ImGui::Separator();
    ImGui::Text("Battle Turn Queue:");
    auto rows = model.VisibleRows();
    if (rows.empty()) {
        ImGui::TextDisabled("No active action rows in queue.");
    } else {
        if (ImGui::BeginTable("BattleActionQueueTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Order");
            ImGui::TableSetupColumn("Subject");
            ImGui::TableSetupColumn("Target");
            ImGui::TableSetupColumn("Command");
            ImGui::TableSetupColumn("Speed");
            ImGui::TableSetupColumn("Summary");
            ImGui::TableHeadersRow();
            for (const auto& row : rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%zu", row.action_order);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.subject_id.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.target_id.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.command.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%d", row.speed);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.summary.c_str());
            }
            ImGui::EndTable();
        }
    }
}

void renderAudioDiagnostics(urpg::editor::AudioInspectorPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Active Audio Sources: %zu (Issues: %zu)", snapshot.active_count, snapshot.issue_count);
    if (!snapshot.status_message.empty()) {
        ImGui::TextWrapped("%s", snapshot.status_message.c_str());
    }

    ImGui::Text("Master Volume: %.2f", snapshot.master_volume);

    ImGui::Separator();
    ImGui::Text("Live Audio Channels:");
    if (snapshot.live_rows.empty()) {
        ImGui::TextDisabled("No active audio channels.");
    } else {
        if (ImGui::BeginTable("AudioChannelsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Asset ID");
            ImGui::TableSetupColumn("Category");
            ImGui::TableSetupColumn("Volume");
            ImGui::TableSetupColumn("Pitch");
            ImGui::TableSetupColumn("Looping");
            ImGui::TableSetupColumn("Active");
            ImGui::TableHeadersRow();
            for (const auto& row : snapshot.live_rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.assetId.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%d", static_cast<int>(row.category));
                ImGui::TableNextColumn(); ImGui::Text("%.2f", row.volume);
                ImGui::TableNextColumn(); ImGui::Text("%.2f", row.pitch);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.isLooping ? "Yes" : "No");
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.isActive ? "Yes" : "No");
            }
            ImGui::EndTable();
        }
    }
}

void renderMigrationWizardDiagnostics(urpg::editor::MigrationWizardPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Migration Status: %s", snapshot.headline.empty() ? "Idle" : snapshot.headline.c_str());
    ImGui::Text("Files Processed: %zu (Warnings: %zu, Errors: %zu)", snapshot.total_files_processed, snapshot.warning_count, snapshot.error_count);

    const auto buttonHelper = [&](const urpg::editor::MigrationWizardPanel::WorkflowActionState& action, const std::function<void()>& onClick) {
        if (!action.visible) return;
        if (!action.enabled) ImGui::BeginDisabled();
        if (ImGui::Button(action.label.c_str())) {
            onClick();
        }
        if (!action.enabled) ImGui::EndDisabled();
    };

    buttonHelper(snapshot.primary_actions.run_migration, [&]() {
        panel.rerunBoundProject();
    });
    ImGui::SameLine();
    buttonHelper(snapshot.primary_actions.rerun_selected_subsystem, [&]() {
        panel.rerunBoundSelectedSubsystem();
    });
    ImGui::SameLine();
    buttonHelper(snapshot.primary_actions.clear_selected_subsystem, [&]() {
        panel.clearSelectedSubsystemResult();
    });

    ImGui::NewLine();
    buttonHelper(snapshot.primary_actions.previous_subsystem, [&]() {
        panel.selectPreviousSubsystemResult();
    });
    ImGui::SameLine();
    buttonHelper(snapshot.primary_actions.next_subsystem, [&]() {
        panel.selectNextSubsystemResult();
    });
    ImGui::SameLine();
    buttonHelper(snapshot.primary_actions.previous_issue_subsystem, [&]() {
        panel.selectPreviousIssueSubsystemResult();
    });
    ImGui::SameLine();
    buttonHelper(snapshot.primary_actions.next_issue_subsystem, [&]() {
        panel.selectNextIssueSubsystemResult();
    });

    ImGui::Separator();
    ImGui::Text("Subsystems:");
    if (snapshot.subsystem_cards.empty()) {
        ImGui::TextDisabled("No subsystem migration cards.");
    } else {
        if (ImGui::BeginTable("MigrationSubsystemsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Subsystem");
            ImGui::TableSetupColumn("Completed");
            ImGui::TableSetupColumn("Processed");
            ImGui::TableSetupColumn("Warnings");
            ImGui::TableSetupColumn("Errors");
            ImGui::TableHeadersRow();
            for (const auto& card : snapshot.subsystem_cards) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                const bool is_selected = card.is_selected;
                if (ImGui::Selectable(card.display_name.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    panel.selectSubsystemResult(card.subsystem_id);
                }
                ImGui::TableNextColumn(); ImGui::TextUnformatted(card.completed ? "Yes" : "No");
                ImGui::TableNextColumn(); ImGui::Text("%zu", card.processed_count);
                ImGui::TableNextColumn(); ImGui::Text("%zu", card.warning_count);
                ImGui::TableNextColumn(); ImGui::Text("%zu", card.error_count);
            }
            ImGui::EndTable();
        }
    }

    if (!snapshot.summary_logs.empty()) {
        ImGui::Separator();
        ImGui::Text("Logs:");
        for (const auto& log : snapshot.summary_logs) {
            ImGui::TextWrapped("%s", log.c_str());
        }
    }
}

void renderProjectAuditDiagnostics(const urpg::editor::ProjectAuditPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Project Completeness Audit: %s", snapshot.headline.empty() ? "No Report Data" : snapshot.headline.c_str());
    ImGui::Text("Issues found: %zu (Release Blockers: %zu, Export Blockers: %zu)", snapshot.issue_count, snapshot.release_blocker_count, snapshot.export_blocker_count);
    if (!snapshot.summary.empty()) {
        ImGui::TextWrapped("%s", snapshot.summary.c_str());
    }

    ImGui::Separator();
    ImGui::Text("Audit Issues:");
    if (snapshot.issues.empty()) {
        ImGui::TextDisabled("All checks passed! No issues found.");
    } else {
        if (ImGui::BeginTable("AuditIssuesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Code");
            ImGui::TableSetupColumn("Severity");
            ImGui::TableSetupColumn("Title");
            ImGui::TableSetupColumn("Blocks Release");
            ImGui::TableSetupColumn("Blocks Export");
            ImGui::TableHeadersRow();
            for (const auto& issue : snapshot.issues) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(issue.code.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(issue.severity == urpg::editor::ProjectAuditSeverity::Error ? "Error" : (issue.severity == urpg::editor::ProjectAuditSeverity::Warning ? "Warning" : "Info"));
                ImGui::TableNextColumn(); ImGui::TextUnformatted(issue.title.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(issue.blocks_release ? "Yes" : "No");
                ImGui::TableNextColumn(); ImGui::TextUnformatted(issue.blocks_export ? "Yes" : "No");
            }
            ImGui::EndTable();
        }
    }
}

void renderProjectHealthDiagnostics(const urpg::editor::ProjectHealthPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Project Health Status: %s", snapshot.headline.empty() ? "No Health Data" : snapshot.headline.c_str());
    ImGui::Text("Total Issues: %zu (Blockers: %zu)", snapshot.issue_count, snapshot.release_blocker_count);
    if (!snapshot.summary.empty()) {
        ImGui::TextWrapped("%s", snapshot.summary.c_str());
    }

    ImGui::Separator();
    ImGui::Text("Health Groups:");
    if (snapshot.groups.empty()) {
        ImGui::TextDisabled("No health groups available.");
    } else {
        for (const auto& group : snapshot.groups) {
            if (ImGui::CollapsingHeader(group.title.c_str())) {
                ImGui::Text("Issues: %zu (Blockers: %zu)", group.issue_count, group.blocker_count);
                if (group.fixes.empty()) {
                    ImGui::TextDisabled("No issues in this group.");
                } else {
                    for (const auto& fix : group.fixes) {
                        ImGui::BulletText("[%s] %s: %s", fix.code.c_str(), fix.title.c_str(), fix.detail.c_str());
                    }
                }
            }
        }
    }
}

void renderDiagnosticsWorkspace(EditorPanelRuntime& runtime) {
    auto& diagnostics = runtime.diagnostics_workspace;
    diagnostics.update();
    for (const auto& summary : diagnostics.allTabSummaries()) {
        ImGui::PushID(static_cast<int>(summary.tab));
        if (ImGui::Selectable(diagnosticsTabName(summary.tab), summary.active)) {
            diagnostics.setActiveTab(summary.tab);
        }
        ImGui::SameLine(150.0f);
        ImGui::Text("items %zu", summary.item_count);
        ImGui::SameLine(235.0f);
        ImGui::Text("issues %zu", summary.issue_count);
        ImGui::PopID();
    }
    ImGui::Separator();
    ImGui::TextWrapped("Active diagnostics tab: %s", diagnosticsTabName(diagnostics.activeTab()));
    ImGui::Separator();

    auto activeTab = diagnostics.activeTab();
    if (activeTab == urpg::editor::DiagnosticsTab::Compat) {
        renderCompatDiagnostics(diagnostics.compatPanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::Save) {
        renderSaveDiagnostics(diagnostics.savePanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::EventAuthority) {
        renderEventAuthorityDiagnostics(diagnostics.eventAuthorityPanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::MessageText) {
        renderMessageDiagnostics(diagnostics.messagePanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::Battle) {
        renderBattleDiagnostics(diagnostics.battlePanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::Audio) {
        renderAudioDiagnostics(diagnostics.audioPanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::MigrationWizard) {
        renderMigrationWizardDiagnostics(diagnostics.migrationWizardPanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::ProjectAudit) {
        renderProjectAuditDiagnostics(diagnostics.projectAuditPanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::ProjectHealth) {
        renderProjectHealthDiagnostics(diagnostics.projectHealthPanel());
    }
}

void renderAssetWorkspace(EditorPanelRuntime& runtime) {
    auto& panel = runtime.asset_library_panel;
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Status: %s", snapshot.status.c_str());
    if (!snapshot.status_message.empty()) {
        ImGui::TextWrapped("%s", snapshot.status_message.c_str());
    }
    if (!snapshot.error_message.empty()) {
        ImGui::TextWrapped("%s", snapshot.error_message.c_str());
    }
    if (ImGui::Button("Load Reports")) {
        std::string error;
        (void)panel.model().loadReportsFromDirectory(runtime.project_root / "imports" / "reports", &error);
        panel.render();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Filters")) {
        (void)panel.model().applyQuickFilter("all_assets");
        panel.render();
    }
    ImGui::Separator();
    ImGui::Text("Assets: %zu", snapshot.asset_count);
    ImGui::Text("Runtime ready: %zu", snapshot.runtime_ready_count);
    ImGui::Text("Previewable: %zu", snapshot.previewable_count);
    ImGui::Text("Duplicates: %zu groups / %zu assets", snapshot.duplicate_group_count, snapshot.duplicate_asset_count);
    ImGui::Text("Import rows: %zu", snapshot.import_review_row_count);
    ImGui::Text("Project attached: %zu", snapshot.project_attached_count);
    ImGui::Separator();
    ImGui::Text("Last Action");
    renderJsonLines(snapshot.last_action, 6);
}

void renderModWorkspace(EditorPanelRuntime& runtime) {
    const auto snapshot = runtime.mod_manager_panel.lastRenderSnapshot();
    ImGui::Text("Status: %s", snapshot.value("status", "unknown").c_str());
    ImGui::Text("Registered: %zu", snapshot.value("registered_count", size_t{0}));
    ImGui::Text("Active: %zu", snapshot.value("active_count", size_t{0}));
    if (ImGui::Button("Clear Last Action")) {
        runtime.mod_manager_panel.clearLastAction();
        runtime.mod_manager_panel.render();
    }
    ImGui::Separator();
    ImGui::Text("Messages");
    renderJsonLines(snapshot.value("status_messages", nlohmann::json::array()), 6);
    ImGui::Separator();
    ImGui::Text("Mods");
    renderJsonLines(snapshot.value("mods", nlohmann::json::array()), 6);
}

void renderLevelBuilderWorkspace(EditorPanelRuntime& runtime) {
    auto& workspace = runtime.level_builder_workspace;
    const auto& snapshot = workspace.lastRenderSnapshot();
    ImGui::Text("Status: %s", snapshot.status.c_str());
    ImGui::TextWrapped("%s", snapshot.message.c_str());
    ImGui::Text("Mode: %s", snapshot.active_mode.c_str());
    ImGui::Text("Placed parts: %zu", snapshot.placement.placed_count);
    ImGui::Text("Palette parts: %zu", snapshot.palette.part_count);
    ImGui::Text("Diagnostics: %zu (%zu blocking)", snapshot.validation.diagnostic_count,
                snapshot.validation.blocking_count);
    ImGui::Separator();
    ImGui::Text("Actions");
    for (const auto& action : snapshot.actions) {
        ImGui::PushID(action.id.c_str());
        if (!action.enabled) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button(action.label.c_str(), ImVec2(118.0f, 0.0f))) {
            (void)workspace.ActivateToolbarAction(action.id);
        }
        if (!action.enabled) {
            ImGui::EndDisabled();
        }
        if (!action.enabled) {
            renderDisabledReason("Action is unavailable in the current level state.");
        }
        ImGui::SameLine();
        ImGui::PopID();
    }
    ImGui::NewLine();
    ImGui::Separator();
    ImGui::Text("Palette");
    int shown = 0;
    for (const auto& entry : snapshot.palette.entries) {
        if (shown++ >= 10) {
            ImGui::TextDisabled("...");
            break;
        }
        ImGui::PushID(entry.part_id.c_str());
        if (ImGui::Selectable(entry.display_name.c_str(), entry.selected)) {
            (void)workspace.SelectGridPart(entry.part_id);
        }
        ImGui::SameLine(220.0f);
        ImGui::TextDisabled("%s", entry.category.c_str());
        ImGui::PopID();
    }
}

void renderPerspectiveWorkspace(EditorPanelRuntime& runtime) {
    auto& workspace = runtime.perspective_2d_workspace;
    const auto& snapshot = workspace.lastRenderSnapshot();
    ImGui::Text("Status: %s", snapshot.status.c_str());
    ImGui::TextWrapped("%s", snapshot.message.c_str());
    ImGui::Text("Mode: %s", snapshot.toolbar.active_mode.c_str());
    ImGui::Text("Layers: %zu", snapshot.perspective_2d_layers.size());
    ImGui::Text("Events: %zu", snapshot.perspective_2d_events.size());
    ImGui::Text("Visible tile options: %zu", snapshot.perspective_2d_palette.visible_tile_option_count);
    ImGui::Separator();
    ImGui::Text("Tools");
    for (const auto& action : snapshot.toolbar.actions) {
        ImGui::PushID(action.id.c_str());
        if (!action.enabled) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button(action.label.c_str(), ImVec2(118.0f, 0.0f))) {
            (void)workspace.ActivateToolbarAction(action.id);
        }
        if (!action.enabled) {
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        ImGui::PopID();
    }
    ImGui::NewLine();
    ImGui::Separator();
    ImGui::Text("Layers");
    int shown = 0;
    for (const auto& layer : snapshot.perspective_2d_layers) {
        if (shown++ >= 10) {
            ImGui::TextDisabled("...");
            break;
        }
        ImGui::PushID(layer.id.c_str());
        if (ImGui::Selectable(layer.label.c_str(), layer.selected)) {
            (void)workspace.SelectPerspectiveLayer(layer.id);
        }
        ImGui::SameLine(220.0f);
        ImGui::TextDisabled("%s", layer.kind.c_str());
        ImGui::PopID();
    }
}

void renderAbilityWorkspaceInline(EditorPanelRuntime& runtime) {
    auto& panel = runtime.ability_inspector_panel;
    panel.update(runtime.ability_runtime);
    const auto& snapshot = panel.getRenderSnapshot();

    ImGui::Text("Active Tags");
    const auto& tags = panel.getModel().getActiveTags();
    if (tags.empty()) {
        ImGui::TextDisabled("None");
    } else {
        for (const auto& tagInfo : tags) {
            ImGui::BulletText("%s (%d stack%s)", tagInfo.tag.c_str(), tagInfo.count, tagInfo.count == 1 ? "" : "s");
        }
    }

    ImGui::Separator();
    ImGui::Text("Abilities");
    const auto& abilities = panel.getModel().getAbilities();
    if (abilities.empty()) {
        ImGui::TextDisabled("No abilities are bound to this runtime.");
    } else {
        if (ImGui::BeginTable("AbilityInspectorAbilitiesInline", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Ability");
            ImGui::TableSetupColumn("Status");
            ImGui::TableSetupColumn("Cooldown");
            ImGui::TableSetupColumn("Blocking Reason");
            ImGui::TableHeadersRow();

            const auto selected_index = panel.getModel().selectedAbilityIndex();
            for (size_t index = 0; index < abilities.size(); ++index) {
                const auto& info = abilities[index];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                const bool selected = selected_index.has_value() && *selected_index == index;
                if (ImGui::Selectable(info.name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    panel.selectAbility(index, runtime.ability_runtime);
                }
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(info.can_activate ? "Ready" : "Blocked");
                ImGui::TableNextColumn();
                ImGui::Text("%.2fs", info.cooldown_remaining);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(info.blocking_reason.empty() ? "-" : info.blocking_reason.c_str());
            }
            ImGui::EndTable();
        }
    }

    ImGui::Separator();
    ImGui::Text("Actions");
    if (ImGui::Button("Preview Selected")) {
        panel.previewSelectedAbility(runtime.ability_runtime);
    }
    ImGui::SameLine();
    if (ImGui::Button("Apply Draft")) {
        panel.applyDraftToRuntime(runtime.ability_runtime);
        panel.update(runtime.ability_runtime);
    }

    if (!snapshot.diagnostic_lines.empty()) {
        ImGui::Separator();
        ImGui::Text("Diagnostics");
        for (const auto& line : snapshot.diagnostic_lines) {
            ImGui::BulletText("%s", line.c_str());
        }
    }
}
void renderPatternsWorkspaceInline(EditorPanelRuntime& runtime) {
    auto& panel = runtime.pattern_field_panel;
    auto& currentModel = runtime.pattern_field_model;
    const auto& snapshot = panel.getRenderSnapshot();

    std::string name = snapshot.name;
    if (ImGui::InputText("Name", &name)) {
        panel.setPatternName(name);
    }

    int viewport = snapshot.viewport_size;
    if (ImGui::InputInt("Viewport", &viewport)) {
        panel.resizeViewport(viewport);
    }

    if (ImGui::Button("Clear")) {
        panel.clearPattern();
    }

    const auto presets = currentModel.availablePresets();
    if (!presets.empty()) {
        ImGui::Separator();
        ImGui::Text("Presets");
        for (const auto& preset : presets) {
            ImGui::PushID(preset.id.c_str());
            if (ImGui::Button(preset.display_name.c_str())) {
                panel.applyPreset(preset.id);
            }
            ImGui::PopID();
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }

    ImGui::Separator();
    ImGui::Text("Grid");
    const auto bounds = currentModel.getViewportBounds();
    for (int32_t y = bounds.minY; y <= bounds.maxY; ++y) {
        for (int32_t x = bounds.minX; x <= bounds.maxX; ++x) {
            ImGui::PushID(static_cast<int>((y - bounds.minY) * snapshot.viewport_size + (x - bounds.minX)));
            const bool selected = currentModel.isPointSelected(x, y);
            const char* label = (x == 0 && y == 0) ? (selected ? "[O]" : "[.]") : (selected ? "[X]" : "[ ]");
            if (ImGui::Button(label, ImVec2(36.0f, 28.0f))) {
                panel.togglePoint(x, y);
            }
            ImGui::PopID();
            if (x < bounds.maxX) {
                ImGui::SameLine();
            }
        }
    }
}

void renderAnalyticsWorkspaceInline(urpg::editor::AnalyticsPanel& panel) {
    const auto snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Session ID: %s", snapshot.value("session_id", "unknown").c_str());
    ImGui::Text("Consent State: %s", snapshot.value("consent_state", "unknown").c_str());

    bool optIn = snapshot.value("opt_in_checked", false);
    if (ImGui::Checkbox("Opt In", &optIn)) {
        panel.setOptIn(optIn);
        panel.render();
    }

    ImGui::Text("Queue size: %zu", snapshot.value("queue_size", size_t{0}));
    if (ImGui::Button("Clear Queue")) {
        panel.clearQueuedEvents();
        panel.render();
    }
    ImGui::SameLine();
    if (ImGui::Button("Flush Upload")) {
        panel.flushQueuedEvents();
        panel.render();
    }
}

void renderEditorWorkspace(urpg::editor::EditorShell& editorShell, EditorPanelRuntime& runtime) {
    const auto snapshot = editorShell.snapshot();
    ImGui::SetNextWindowPos(ImVec2(370.0f, 12.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(520.0f, 520.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("URPG Workspace")) {
        ImGui::End();
        return;
    }

    ImGui::Text("Panel: %s", snapshot.active_panel_id.c_str());
    ImGui::Separator();

    if (snapshot.active_panel_id == "diagnostics") {
        renderDiagnosticsWorkspace(runtime);
    } else if (snapshot.active_panel_id == "assets") {
        renderAssetWorkspace(runtime);
    } else if (snapshot.active_panel_id == "mod") {
        renderModWorkspace(runtime);
    } else if (snapshot.active_panel_id == "level_builder") {
        renderLevelBuilderWorkspace(runtime);
    } else if (snapshot.active_panel_id == "spatial_authoring") {
        renderPerspectiveWorkspace(runtime);
    } else if (snapshot.active_panel_id == "ability") {
        renderAbilityWorkspaceInline(runtime);
    } else if (snapshot.active_panel_id == "patterns") {
        renderPatternsWorkspaceInline(runtime);
    } else if (snapshot.active_panel_id == "analytics") {
        renderAnalyticsWorkspaceInline(runtime.analytics_panel);
    } else {
        ImGui::TextWrapped("This panel opens in its own tool window.");
    }

    ImGui::End();
}
#endif

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
                    EditorPanelRuntime* panelRuntime = nullptr, double deltaSeconds = 1.0 / 60.0) {
    engineShell.tick();
#ifdef URPG_IMGUI_ENABLED
#ifndef URPG_HEADLESS
    if (!editorShell.snapshot().headless) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
    }
#endif
    ImGui::NewFrame();
#endif
    bool rendered = false;
    if (editorShell.beginFrame(deltaSeconds)) {
#ifdef URPG_IMGUI_ENABLED
        if (!editorShell.snapshot().headless) {
            renderEditorChrome(editorShell);
        }
#endif
        if (renderAllPanels) {
            rendered = editorShell.renderVisiblePanels() > 0;
        } else {
            rendered = editorShell.renderActivePanel();
        }
#ifdef URPG_IMGUI_ENABLED
        if (panelRuntime != nullptr && !editorShell.snapshot().headless) {
            renderEditorWorkspace(editorShell, *panelRuntime);
        }
#endif
        rendered = editorShell.endFrame() && rendered;
    }
#ifdef URPG_IMGUI_ENABLED
    ImGui::Render();
#ifndef URPG_HEADLESS
    if (!editorShell.snapshot().headless) {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (auto* platform = engineShell.getPlatform()) {
            platform->present();
        }
    }
#endif
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
            printRuntimeDiagnostics();
            return 1;
        }

        auto& engineShell = urpg::EngineShell::getInstance();
        if (!engineShell.startup(std::move(surface), std::move(renderer),
                                 urpg::EngineShell::StartupOptions(options.project_root))) {
            std::cerr << "URPG editor startup failed.\n";
            return 1;
        }
        if (!options.headless) {
            engineShell.getRenderer()->setAutoPresent(false);
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
        if (options.headless) {
            unsigned char* fontPixels = nullptr;
            int fontWidth = 0;
            int fontHeight = 0;
            ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&fontPixels, &fontWidth, &fontHeight);
        }
#ifndef URPG_HEADLESS
        if (!options.headless && !ImGui_ImplOpenGL3_Init("#version 330")) {
            std::cerr << "URPG editor failed to initialize ImGui OpenGL renderer.\n";
            ImGui::DestroyContext();
            editorShell.shutdown();
            engineShell.shutdown();
            clearSceneStack();
            return 1;
        }
        if (!options.headless) {
            auto* sdlSurface = dynamic_cast<urpg::SDLSurface*>(engineShell.getPlatform());
            if (!sdlSurface || !ImGui_ImplSDL2_InitForOpenGL(sdlSurface->getNativeWindow(),
                                                             sdlSurface->getNativeGlContext())) {
                std::cerr << "URPG editor failed to initialize ImGui SDL renderer.\n";
                ImGui_ImplOpenGL3_Shutdown();
                ImGui::DestroyContext();
                editorShell.shutdown();
                engineShell.shutdown();
                clearSceneStack();
                return 1;
            }
            sdlSurface->setEventCallback([](const void* event) {
                ImGui_ImplSDL2_ProcessEvent(static_cast<const SDL_Event*>(event));
            });
        }
#endif
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
#ifndef URPG_HEADLESS
            if (!options.headless) {
                if (auto* sdlSurface = dynamic_cast<urpg::SDLSurface*>(engineShell.getPlatform())) {
                    sdlSurface->setEventCallback(nullptr);
                }
                ImGui_ImplSDL2_Shutdown();
                ImGui_ImplOpenGL3_Shutdown();
            }
#endif
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
#ifndef URPG_HEADLESS
            if (!options.headless) {
                if (auto* sdlSurface = dynamic_cast<urpg::SDLSurface*>(engineShell.getPlatform())) {
                    sdlSurface->setEventCallback(nullptr);
                }
                ImGui_ImplSDL2_Shutdown();
                ImGui_ImplOpenGL3_Shutdown();
            }
#endif
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
            (void)runEditorFrame(engineShell, editorShell, options.render_all_panels, &panelRuntime);
            ++frame;
            if (options.headless) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        editorShell.shutdown();
#ifdef URPG_IMGUI_ENABLED
#ifndef URPG_HEADLESS
        if (!options.headless) {
            if (auto* sdlSurface = dynamic_cast<urpg::SDLSurface*>(engineShell.getPlatform())) {
                sdlSurface->setEventCallback(nullptr);
            }
            ImGui_ImplSDL2_Shutdown();
            ImGui_ImplOpenGL3_Shutdown();
        }
#endif
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
